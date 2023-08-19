#include "head.h"
#include <string.h>

#define CONSOLE_BXSIZE 512
#define CONSOLE_BYSIZE 330

#define outb_p(value,port) \
( {_io_out8(port, value); \
		_io_hlt(); \
		_io_hlt();} )

#define inb_p(port) ({ \
unsigned char _v; \
 _v = _io_in8(port); \
	_io_hlt(); \
	_io_hlt(); \
_v; \
})

#define CMOS_READ(addr) ({ \
outb_p(0x80|addr,0x70); \
inb_p(0x71); \
})

unsigned char cmos_read(addr) {
	outb_p(0x80 | addr, 0x70);
	return inb_p(0x71);
}

extern struct PROCESSCTL *processCtl;
extern struct FIFO16 *keyfifo;
extern struct hdisk_info hd_info[2];
extern unsigned char hdbuf[512 << 1];

extern struct HDISK_REQUEST hdisk_request;

struct PROCESS *create_console(struct MEMMAN *memman, struct SHTCTL *shtctl, struct SHEET *sht) {
	sprintf(sht->title, "%s", "console");
	unsigned char *buf = (unsigned char *) memman_alloc_4k(memman, CONSOLE_BXSIZE * CONSOLE_BYSIZE);
	sheet_setbuf(sht, buf, CONSOLE_BXSIZE, CONSOLE_BYSIZE, -1);
	draw_title(sht);
	draw_textbox(sht, 8, 28, sht->bxsize - 16, sht->bysize - 36, COL8_000000);

	struct PROCESS *process = process_alloc();
	sht->process = process;
	sht->flags |= SHEET_HAVE_CURSOR;
	process->close = close_console;
	process->cons_stack = memman_alloc_4k(memman, 1024 << 6);
	process->tss.esp = process->cons_stack + (1024 << 6) - 8;
	process->tss.eip = (int) &console_process;
	process->tss.es = 8;
	process->tss.cs = 16;
	process->tss.ss = 8;
	process->tss.ds = 8;
	process->tss.fs = 8;
	process->tss.gs = 8;
	*((int *) (process->tss.esp + 4)) = (int) sht;
	
	process_run(process, 2, 2);
	
	return process;
}

void console_process(struct SHEET *sheet) {
	struct PROCESS *process = process_now();

	int cursor_x = 8, cursor_y = 28, cursor_c = COL8_000000;
	unsigned char fifo8buf[4];
	unsigned short fifo16buf[64];
	
	fifo8_init(&process->fifo8, 4, fifo8buf, process);
	fifo16_init(&process->fifo16, 64, fifo16buf, process);
	process->timer = timer_alloc();
	timer_init(process->timer, &process->fifo8, 1);
	timer_settime(process->timer, 50);
	
	changeFont8(sheet, cursor_x, cursor_y, COL8_000000, COL8_FFFFFF, ">");
	cursor_x += 8;
	
	unsigned char cmd[128];
	unsigned short i;
	for ( ; ; ) {
		_io_cli();
		if (fifo8_status(&process->fifo8) == 0 && fifo16_status(&process->fifo16) == 0) {
			process_sleep(process);
			_io_sti();
		}
		else if (fifo16_status(&process->fifo16) != 0) {
			i = fifo16_get(&process->fifo16);
			_io_sti();
			if (i > 128) {
				i -= 128;
				if (i == 0x0E && cursor_x > 16) {
					fillbox8(sheet->buf, sheet->bxsize, COL8_FFFFFF, cursor_x - 8, cursor_y, cursor_x, cursor_y + 16);
					fillbox8(sheet->buf, sheet->bxsize, COL8_000000, cursor_x, cursor_y, cursor_x + 8, cursor_y + 16);
					sheet_refresh(sheet, cursor_x - 8, cursor_y, cursor_x + 18, cursor_y + 16);
					cmd[(cursor_x >> 3) - 2] = 0;
					cursor_x -= 8;
				}
				else if (i == 0x1C) {
					fillbox8(sheet->buf, sheet->bxsize, COL8_000000, cursor_x, cursor_y, cursor_x + 8, cursor_y + 16);
					sheet_refresh(sheet, cursor_x, cursor_y, cursor_x + 8, cursor_y + 16);
					cmd[(cursor_x >> 3) - 2] = 0;
					if (cursor_y < sheet->bysize - 36) {
						cursor_y += 16;
					}
					else {
						int x, y;
						for (y = 28; y < 28 + sheet->bysize - 36 - 16; ++y) {
							int temp = y * sheet->bxsize;
							for (x = 8; x < 8 + sheet->bxsize - 16; ++x) {
								sheet->buf[x + temp] = sheet->buf[x + temp + (sheet->bxsize << 4)];
							}
						}
						for (y = 28 + sheet->bysize - 36 - 16; y < 28 + sheet->bysize - 36; ++y) {
							int temp = y * sheet->bxsize;
							for (x = 8; x < 8 + sheet->bxsize - 16; ++x) {
								sheet->buf[x + temp] = COL8_000000;
							}
						}
						sheet_refresh(sheet, 8, 28, 8 + sheet->bxsize - 16, 28 + sheet->bysize - 36);
					}
					command(sheet, cmd, &cursor_x, &cursor_y);
					cursor_x = 8;
					changeFont8(sheet, cursor_x, cursor_y, COL8_000000, COL8_FFFFFF, ">");
					cursor_x += 8;
				}
			}
			else {
				unsigned char s[2];
				s[0] = i;
				s[1] = 0;
				if (cursor_x < sheet->bxsize - 18) {
					changeFonts8(sheet, cursor_x, cursor_y, cursor_x + 8, cursor_y + 16, COL8_000000, COL8_FFFFFF, s);
					cmd[(cursor_x >> 3) - 2] = i;
					cursor_x += 8;
				}
			}
			
		}
		else {
			i = fifo8_get(&process->fifo8);
			_io_sti();
			if (i <= 1) {				
				if (i != 0) {
					timer_init(process->timer, &process->fifo8, 0);
					cursor_c = COL8_FFFFFF;
				}
				else {
					timer_init(process->timer, &process->fifo8, 1);
					cursor_c = COL8_000000;
				}
				timer_settime(process->timer, 50);
				fillbox8(sheet->buf, sheet->bxsize, cursor_c, cursor_x, cursor_y, cursor_x + 8, cursor_y + 16);
				sheet_refresh(sheet, cursor_x, cursor_y, cursor_x + 8, cursor_y + 16);
			}
			else if (i == 2) {
				fillbox8(sheet->buf, sheet->bxsize, COL8_000000, cursor_x, cursor_y, cursor_x + 8, cursor_y + 16);
				sheet_refresh(sheet, cursor_x, cursor_y, cursor_x + 8, cursor_y + 16);
				timer_parse(process->timer);
			}
		}
	}
}

void close_console(struct SHEET *sheet) {
	struct PROCESS *process = sheet->process;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	timer_free(process->timer);
	memman_free_4k(memman, (int) sheet->buf, sheet->bxsize * sheet->bysize);
	sheet_free(sheet);
	process_sleep(process);
	memman_free_4k(memman, process->cons_stack, 1024 << 6);
	memman_free_4k(memman, (int) process->fifo8.buf, 4);
	memman_free_4k(memman, (int) process->fifo16.buf, 64 << 1);
	process->flags = 0;
	return;
}

void console_newline(struct SHEET *sheet, int *cursor_x, int *cursor_y) {
	if (*cursor_y < CONSOLE_BYSIZE - 36) {
		*cursor_y += 16;
	}
	else {
		int x, y;
		for (y = 28; y < 28 + sheet->bysize - 36 - 16; ++y) {
			int temp = y * sheet->bxsize;
			for (x = 8; x < 8 + sheet->bxsize - 16; ++x) {
				sheet->buf[x + temp] = sheet->buf[x + temp + (sheet->bxsize << 4)];
			}
		}
		for (y = 28 + sheet->bysize - 36 - 16; y < 28 + sheet->bysize - 36; ++y) {
			int temp = y * sheet->bxsize;
			for (x = 8; x < 8 + sheet->bxsize - 16; ++x) {
				sheet->buf[x + temp] = COL8_000000;
			}
		}
		sheet_refresh(sheet, 8, 28, 8 + sheet->bxsize - 16, 28 + sheet->bysize - 36);
	}
	*cursor_x = 8;
	return;
}

void console_println(struct SHEET *sheet, int *cursor_x, int *cursor_y, unsigned char *buf) {
	changeFonts8(sheet, *cursor_x, *cursor_y, sheet->bxsize - 18, *cursor_y + 16, COL8_000000, COL8_FFFFFF, buf);
	console_newline(sheet, cursor_x, cursor_y);
	return;
}

void command(struct SHEET *sheet, unsigned char *cmd, int *cursor_x, int *cursor_y) {
	int temp_x = *cursor_x;
	*cursor_x = 8;
	int i;
	for (i = ((temp_x - 8) >> 3) - 2; i >= 0; --i) {
		if (cmd[i] == ' ')
			cmd[i] = 0;
		else
			break;
	}
	if (strcmp(cmd, "clear") == 0) {
		command_clear(sheet, cursor_y);
	}
	else if (strcmp(cmd, "console") == 0) {
		command_console(sheet);
	}
	else if (strcmp(cmd, "exit") == 0) {
		command_exit(sheet);
	}
	else if (strcmp(cmd, "hdisk") == 0) {
		command_hdisk(sheet, cursor_x, cursor_y);
	}
	else if (strcmp(cmd, "hdisk_info") == 0) {
		command_hdisk_info(sheet, cursor_x, cursor_y);
	}
	else if (strcmp(cmd, "time") == 0) {
		command_time(sheet, cursor_x, cursor_y);
	}
	else if (strcmp(cmd, "hdisk_read") == 0) {
		command_direct_hdisk_read(sheet, cursor_x, cursor_y);
	}
	else if (strcmp(cmd, "hdisk_write") == 0) {
		command_direct_hdisk_write(sheet, cursor_x, cursor_y);
	}
	else if (strcmp(cmd, "test") == 0) {
		command_test(sheet, cursor_x, cursor_y);
	}
	for ( ; i >= 0; --i) {
		cmd[i] = 0;
	}
	return;
}

void command_clear(struct SHEET *sheet, int *cursor_y) {
	int x, y;
	int temp = 27 * sheet->bxsize;
	for (y = 28; y < 28 + sheet->bysize - 36; ++y) {
		temp += sheet->bxsize;
		for (x = 8; x < 8 + sheet->bxsize - 16; ++x) {
			sheet->buf[temp + x] = COL8_000000;
		}
	}
	sheet_refresh(sheet, 8, 28, 8 + sheet->bxsize - 16, 28 + sheet->bysize - 36);
	*cursor_y = 28;
	return;
}

void command_console(struct SHEET *sheet) {
	struct SHEET *new_sheet = sheet_alloc(sheet->ctl);
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct PROCESS *new_console = create_console(memman, new_sheet->ctl, new_sheet);
	fifo8_put(&sheet->process->fifo8, 2);
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	sheet_slide(new_sheet, (sheet->vx0 + 40) % binfo->scrnx, sheet->vy0);
	sheet_updown(new_sheet, new_sheet->ctl->top);
	return;
}

void command_exit(struct SHEET *sheet) {
	_io_cli();
	fifo16_put(keyfifo, sheet - sheet->ctl->sheets0);
	/*fifo16_put(&processCtl->main_process->fifo16, sheet - sheet->ctl->sheets0);*/
	_io_sti();
	for ( ; ; )
		process_sleep(sheet->process);
	return;
}

void command_hdisk(struct SHEET *sheet, int *cursor_x, int *cursor_y) {
	/*unsigned char *buf = */
	hdisk_identify(0);
	int i, k;
	unsigned char s[64];
	unsigned char out[CONSOLE_BXSIZE - 26];
	unsigned char *tst = "hello";
	unsigned char tst1[10];

	/*sprintf(tst, "%s", "hello");*/
	sprintf(tst1, "%s!", "ccp");
	sprintf(out, "%s! %s", tst, tst1);
	console_println(sheet, cursor_x, cursor_y, out);

	struct iden_info_ascii {
		int idx;
		int len;
		unsigned char *desc;
	}; 
	/*iinfo[] = {{10, 20, "HD SN"}, {27, 40, "HD Model"} };*/
		 /*Serial number in ASCII */ /* Model number in ASCII */
	
	struct iden_info_ascii iinfo0;
	iinfo0.idx = 10;
	iinfo0.len = 20;
	iinfo0.desc = "HD SN";
	struct iden_info_ascii iinfo1;
	iinfo1.idx = 27;
	iinfo1.len = 40;
	iinfo1.desc = "HD Model";
	struct iden_info_ascii iinfo[2];
	iinfo[0] = iinfo0;
	iinfo[1] = iinfo1;

	for (k = 0; k < sizeof(iinfo) / sizeof(iinfo[0]); k++) {
		unsigned char *p = (unsigned char*)&hdbuf[iinfo[k].idx];
		for (i = 0; i < iinfo[k].len / 2; i++) {
			s[i * 2 + 1] = *p++;
			s[i * 2] = *p++;
		}
		s[i * 2] = 0;
		sprintf(out, "%s: %s", iinfo[k].desc, s);
		console_println(sheet, cursor_x, cursor_y, out);
	}
	
	int capabilities = hdbuf[49];
	sprintf(out, "%d", capabilities);
	console_println(sheet, cursor_x, cursor_y, out);
	sprintf(out, "LBA supported: %s", ((capabilities & 0x0200) != 0) ? "Yes" : "No");
	console_println(sheet, cursor_x, cursor_y, out);

	int cmd_set_supported = hdbuf[83];
	sprintf(out, "%d", cmd_set_supported);
	console_println(sheet, cursor_x, cursor_y, out);
	sprintf(out, "LBA48 supported: %s", ((cmd_set_supported & 0x0400) != 0) ? "Yes" : "No");
	console_println(sheet, cursor_x, cursor_y, out);

	unsigned int sectors = ((int)hdbuf[61] << 16) + hdbuf[60];
	sprintf(out, "%d", sectors);
	console_println(sheet, cursor_x, cursor_y, out);
	sprintf(out, "HD size: %dMB", sectors * 512 / 1048576);
	console_println(sheet, cursor_x, cursor_y, out);
	
	return;
}

void command_hdisk_info(struct SHEET *sheet, int *cursor_x, int *cursor_y) {
	/*struct hdisk_info hd_info[2];
	//read_hdisk_info();*/
	unsigned char out[64];
	if (hd_info[0].cyl != 0) {
		sprintf(out, "cyl: %d, head: %d, wpcom: %d", hd_info[0].cyl, hd_info[0].head, hd_info[0].wpcom);
		console_println(sheet, cursor_x, cursor_y, out);
		sprintf(out, "ctl: %d, lzone: %d, sector: %d", hd_info[0].ctl, hd_info[0].lzone, hd_info[0].sector);
		console_println(sheet, cursor_x, cursor_y, out);
	}
	if (hd_info[1].cyl != 0) {
		sprintf(out, "%s", "=========================================");
		console_println(sheet, cursor_x, cursor_y, out);
		sprintf(out, "cyl: %d, head: %d, wpcom: %d", hd_info[1].cyl, hd_info[1].head, hd_info[1].wpcom);
		console_println(sheet, cursor_x, cursor_y, out);
		sprintf(out, "ctl: %d, lzone: %d, sector: %d", hd_info[1].ctl, hd_info[1].lzone, hd_info[1].sector);
		console_println(sheet, cursor_x, cursor_y, out);
	}
	return;
}

void command_time(struct SHEET *sheet, int *cursor_x, int *cursor_y) {
	struct tm time;
	read_time(&time);
	unsigned char out[32];
	sprintf(out, "%d.%d.%d %d:%d:%d", time.tm_year, time.tm_mon, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec);
	console_println(sheet, cursor_x, cursor_y, out);
	return;
}

void command_direct_hdisk_read(struct SHEET *sheet, int *cursor_x, int *cursor_y) {
	direct_read_hdisk(64);
	int i;
	unsigned char out[2];
	for (i = 0; i < 32; ++i) {
		sprintf(out, "%d", hdisk_request.buffer[i]);
		console_println(sheet, cursor_x, cursor_y, out);
	}
}

void command_direct_hdisk_write(struct SHEET *sheet, int *cursor_x, int *cursor_y) {
	direct_write_hdisk(64);
}

void command_test(struct SHEET *sheet, int *cursor_x, int *cursor_y) {
	unsigned char lst[16] = { 0x01 };
	unsigned char out[4];
	sprintf(out, "%d", lst[0]);
	console_println(sheet, cursor_x, cursor_y, out);
	sprintf(out, "%d", lst[1]);
	console_println(sheet, cursor_x, cursor_y, out);
	sprintf(out, "%d", lst[2]);
	console_println(sheet, cursor_x, cursor_y, out);
	sprintf(out, "%d", lst[3]);
	console_println(sheet, cursor_x, cursor_y, out);
	return;
}
