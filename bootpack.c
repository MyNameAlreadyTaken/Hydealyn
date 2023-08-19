#include "head.h"

#define KEYCMD_LED 0xED

int main(void) {
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	unsigned short fifo16buf[128];
	unsigned char fifo8buf[8];
	struct FIFO16 fifo16;
	struct FIFO8 fifo8;
	
	unsigned int test = memtest(0x00500000, 0xbfffffff);
	unsigned char tst[40];
	memman_init(memman);
	memman_free(memman, 0x00500000, test - 0x00500000);
	//sprintf((char *)tst, "Memory: %dMB", test >> 20);
	
	short key_shift = 0;
	char keycmd_wait = -1;
	struct FIFO8 keycmd;
	unsigned char keycmd_buf[32];
	fifo8_init(&keycmd, 32, keycmd_buf, 0);
	char key_leds = (binfo->leds >> 4) & 7;
	
	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse;
	unsigned char *buf_back;
	unsigned char buf_mouse[256];

	struct MOUSE_DEC mouseDec;
	
	init_gdtidt();
	init_pic();
	_io_sti(); 
	init_pit();

	fifo16_init(&fifo16, 128, fifo16buf, 0);
	enable_keyboard(&fifo16);
	enable_mouse(&fifo16, &mouseDec);
	//_io_out8(PIC0_IMR, 0xf8); 
	//_io_out8(PIC1_IMR, 0xef);
	
	fifo8_init(&fifo8, 8, fifo8buf, 0);
	
	init_palette();
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	sht_back = sheet_alloc(shtctl);
	sht_mouse = sheet_alloc(shtctl);
	buf_back = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, COL8_0000FF);
	init_screen(buf_back, binfo->scrnx, binfo->scrny);
	init_mouse((char *)buf_mouse, COL8_0000FF);
	sheet_slide(sht_back, 0, 0);
	int mouseX = 0;
	int mouseY = 25;
	sheet_slide(sht_mouse, mouseX, mouseY);
	sheet_updown(sht_back, 0);
	sheet_updown(sht_mouse, 1);
	
	key_leds ^= 2;
	fifo8_put(&keycmd, KEYCMD_LED);
	fifo8_put(&keycmd, key_leds);
	
	//putFonts8(buf_back, binfo->scrnx, 128, 0, COL8_000000, tst);
	
	struct PROCESS *main_process = process_init(memman);
	init_main_process(&fifo8, &fifo16);
	
	//struct SHEET *sht_console = sheet_alloc(shtctl);
	//struct PROCESS * console = create_console(memman, shtctl, sht_console);	//!!
	//sheet_slide(sht_console, 32, 40);
	//sheet_updown(sht_console, 1);
	
	//struct TIMER *clockTimer = timer_alloc();
	//timer_init(clockTimer, &fifo8, 1);
	//timer_settime(clockTimer, 100);
	//struct tm now_time;
	//read_time(&now_time);
	//unsigned char clock[8];
	//sprintf(clock, "%02d:%02d:%02d", now_time.tm_hour, now_time.tm_min, now_time.tm_sec);
	//putFonts8(buf_back, binfo->scrnx, binfo->scrnx - 64, binfo->scrny - 17, COL8_000000, clock);
	//sheet_refresh(sht_back, 0, 0, binfo->scrnx, binfo->scrny);
	
	unsigned char *diskNum = init_hdisk();
	
	//fifo16_put(&fifo16, 256);
	//if (fifo16_status(&fifo16) != 0) {
	//	fillbox8(buf_back, binfo->scrnx, COL8_000000, 0, binfo->scrny / 2 - 5, binfo->scrnx, binfo->scrny / 2 + 5);
	//	sheet_refresh(sht_back, 0, 0, binfo->scrnx, binfo->scrny);
	//}
	int mmx = -1, mmy = -1;
	struct SHEET *sht = 0;
	unsigned short i;
	for ( ; ; ) {
		if (fifo8_status(&keycmd) > 0 && keycmd_wait < 0) {
			//fillbox8(buf_back, binfo->scrnx, COL8_FFFFFF, 0, binfo->scrny / 2 - 5, binfo->scrnx, binfo->scrny / 2 + 5);
			//sheet_refresh(sht_back, 0, binfo->scrny / 2 - 5, binfo->scrnx, binfo->scrny / 2 + 5);
			keycmd_wait = fifo8_get(&keycmd);
			wait_KBC_sendready();
			_io_out8(PORT_KEYDAT, keycmd_wait);
		}
		_io_cli();
		if (fifo16_status(&fifo16) + fifo8_status(&fifo8) == 0) {
			process_sleep(main_process);
			_io_sti();
		}
		else if (fifo8_status(&fifo8) != 0) {	//
			//i = fifo8_get(&fifo8);
			//_io_sti();
			//if (i == 1) {
			//	++now_time.tm_sec;
			//	if (now_time.tm_sec == 60) {
			//		++now_time.tm_min;
			//		now_time.tm_sec = 0;
			//		if (now_time.tm_min == 60) {
			//			++now_time.tm_hour;
			//			now_time.tm_min = 0;
			//			if (now_time.tm_hour == 24) {
			//				now_time.tm_hour = 0;
			//			}
			//		}
			//	}
				//sprintf(clock, "%02d:%02d:%02d", now_time.tm_hour, now_time.tm_min, now_time.tm_sec);
				//fillbox8(buf_back, binfo->scrnx, COL8_00FFFF, binfo->scrnx - 64, binfo->scrny - 17, binfo->scrnx, binfo->scrny);
				//putFonts8(buf_back, binfo->scrnx, binfo->scrnx - 64, binfo->scrny - 17, COL8_000000, clock);
				//sheet_refresh(sht_back, binfo->scrnx - 64, binfo->scrny - 17, binfo->scrnx, binfo->scrny);
			//	timer_settime(clockTimer, 100);
			//}
		}
		else {
			i = fifo16_get(&fifo16);
			_io_sti();
			if (i < 256) {
				shtctl->focus_on->process->close(shtctl->focus_on);
				/*close_console(shtctl->sheets0 + i);*/
			}
			if (256 <= i && i <= 511) {		//keyboard
				if (i == 256) {
				fillbox8(buf_back, binfo->scrnx, COL8_FFFFFF, 0, binfo->scrny / 2 - 5, binfo->scrnx, binfo->scrny / 2 + 5);
				sheet_refresh(sht_back, 0, binfo->scrny / 2 - 5, binfo->scrnx, binfo->scrny / 2 + 5);
				}
				i -= 256;
				//unsigned char s[40];
				//sprintf(s, "%02X", i);
				//fillbox8(buf_back, binfo->scrnx, COL8_00FFFF, 0, binfo->scrny - 17, 16, binfo->scrny);
				//putFonts8(buf_back, binfo->scrnx, 0, binfo->scrny - 17, COL8_000000, s);
				sheet_refresh(sht_back, 0, binfo->scrny - 17, 16, binfo->scrny);
				if (i == 0x2A || i == 0x36) {
					key_shift = 1;
				}
				else if (i == 0xAA || i == 0xB6) {
					key_shift = 0;
				}
				else if (i == 0 || i > 108) {}
				else if (keytable[i] != 0) {
					fillbox8(buf_back, binfo->scrnx, COL8_00FFFF, 16, binfo->scrny - 17, 24, binfo->scrny);
					if (((key_leds & 4) == 0 && key_shift == 0) || ((key_leds & 4) != 0 && key_shift == 1)) {
						//putFont8(buf_back, binfo->scrnx, 16, binfo->scrny - 17, COL8_000000, ascii + ((keytable[i] - 32) << 4));
						if (shtctl->focus_on != 0) {
							fifo16_put(&shtctl->focus_on->process->fifo16, keytable[i]);
						}
					}
					else {
						//putFont8(buf_back, binfo->scrnx, 16, binfo->scrny - 17, COL8_000000, ascii + ((shift_keytable[i] - 32) << 4));
						if (shtctl->focus_on != 0) {
							fifo16_put(&shtctl->focus_on->process->fifo16, shift_keytable[i]);
						}
					}
					sheet_refresh(sht_back, 16, binfo->scrny - 17, 24, binfo->scrny);
				}
				else if (i == 0x0E || i == 0x1C) {
					if (shtctl->focus_on != 0) {
						fifo16_put(&shtctl->focus_on->process->fifo16, i + 128);
					}
				}
				else if (i == 0x3A) {		/* CapsLock*/
					key_leds ^= 4;
					fifo8_put(&keycmd, KEYCMD_LED);
					fifo8_put(&keycmd, key_leds);
				}
				else if (i == 0x45) {		/*NumLock*/
					key_leds ^= 2;
					fifo8_put(&keycmd, KEYCMD_LED);
					fifo8_put(&keycmd, key_leds);
				}
				else if (i == 0x46) {		/*ScrollLock*/
					key_leds ^= 1;
					fifo8_put(&keycmd, KEYCMD_LED);
					fifo8_put(&keycmd, key_leds);
				}
				if (i == 0xFA) {	/*键盘成功接收数据*/
					keycmd_wait = -1;
				}
				else if (i == 0xFE) {	/*键盘未能成功接收数据*/
					wait_KBC_sendready();
					_io_out8(PORT_KEYDAT, keycmd_wait);
				}
			}
			else if (512 <= i && i <= 767) {
				
			}
		}
	}
	return 0;
}
