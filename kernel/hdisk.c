#include "hdisk.h"

struct hdisk_info hd_info[MAX_DRIVES];
struct HDISK_STRUCT hdisk_struct[5 * MAX_DRIVES];
unsigned char hdbuf[SECTOR_SIZE << 1];

static int recalibrate = 1;
static int reset = 1;

extern struct HDISK_REQUESTS hdisk_requests;
extern struct HDISK_REQUEST hdisk_request;

unsigned char init_hdisk() {
	/*int k;
	for (k = 0; k < 32; ++k) {
		hdisk_request.buffer[k] = 1;
	}
	for (k = 0; k < (SECTOR_SIZE << 1); ++k) {
		hdbuf[k] = 0;
	}*/
	
	unsigned char *diskNum = (unsigned char *) (0x475);
	if (*diskNum == 0)
		return 0;
	
	_disable_irq(14, _inthandler2e);
	_enable_irq(2);
	_enable_irq(14);
	/*hdisk_identify(*diskNum - 1);*/
	struct DRIVE_INFO drive_info = DRIVE_INFO_ADDR;
	void *BIOS = (void *) &drive_info;
	int drive;
	for (drive = 0 ; drive < 2 ; drive++) {
		hd_info[drive].cyl = *(unsigned short *) BIOS;
		hd_info[drive].head = *(unsigned char *) (2+BIOS);
		hd_info[drive].wpcom = *(unsigned short *) (5+BIOS);
		hd_info[drive].ctl = *(unsigned char *) (8+BIOS);
		hd_info[drive].lzone = *(unsigned short *) (12+BIOS);
		hd_info[drive].sector = *(unsigned char *) (14+BIOS);
		BIOS += 16;
	}
	int i;
	for (i = 0; i < *diskNum; ++i) {
		hdisk_struct[i * 5].start_sector = 0;
		hdisk_struct[i * 5].nr_sectors = hd_info[i].head * hd_info[i].sector * hd_info[i].cyl;
	}
	for (i = *diskNum; i < MAX_DRIVES; ++i) {
		hdisk_struct[i * 5].start_sector = 0;
		hdisk_struct[i * 5].nr_sectors = 0;
	}
	hdisk_struct[1].start_sector = 1;
	hdisk_struct[1].nr_sectors = hdisk_struct[0].nr_sectors - 1;
	return *diskNum;
}

void get_partition_table(int drive, struct PARTITION partition, int sector_nr) {
	struct CMD_BLK_REG cmd;
	cmd.features = 0;
	cmd.count = 1;
	cmd.lba_low = sector_nr & 0xFF;
	cmd.lba_mid = (sector_nr >> 8) & 0xFF;
	cmd.lba_high = (sector_nr >> 16) & 0xFF;
	cmd.device = MAKE_DEVICE_REG(1, drive, (sector_nr >> 24) & 0xF);
	cmd.command = ATA_READ;
	hdisk_sendCmd(&cmd);
	return;
}

struct PROCESS *create_hdprocess() {
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct PROCESS *process = process_alloc();
	process->cons_stack = memman_alloc_4k(memman, 4096);
	process->tss.esp = process->cons_stack + 4096;
	process->tss.eip = (int) &hdisk_process;
	process->tss.es = 8;
	process->tss.cs = 16;
	process->tss.ss = 8;
	process->tss.ds = 8;
	process->tss.fs = 8;
	process->tss.gs = 8;

	process_run(process, 2, 2);

	return process;
}

void hdisk_process() {
	struct PROCESS *process = process_now();
	unsigned char hdFifo8buf[16];
	unsigned short hdFifo16buf[64];

	fifo8_init(&process->fifo8, 16, hdFifo8buf, process);
	fifo16_init(&process->fifo16, 64, hdFifo16buf, process);

	for ( ; ; ) {
		hdisk_identify(0);
	}
}

unsigned char *hdisk_identify(unsigned char drive) {
	struct CMD_BLK_REG cmd_blk_reg;
	cmd_blk_reg.device = MAKE_DEVICE_REG(0, drive, 0);
	cmd_blk_reg.command = ATA_IDENTIFY;
	
	hdisk_sendCmd(&cmd_blk_reg);
	int i;
	int j;
	int k;
	for (k = 0; k < (2 >> 30); ++k) {
	for (j = 0; j < (2 >> 30); ++j) {
		for (i = 0; i < (2 >>30); ++i) { _io_hlt(); }
	}
	}
	_port_read(PRI_DATA, hdbuf, SECTOR_SIZE);	/*0x1F0*/
	return hdbuf;
}

void read_hdisk_info() {
	struct DRIVE_INFO drive_info = DRIVE_INFO_ADDR;
	void *BIOS = (void *) &drive_info;
	int drive;
	for (drive = 0 ; drive < 2 ; drive++) {
		hd_info[drive].cyl = *(unsigned short *) BIOS;
		hd_info[drive].head = *(unsigned char *) (2+BIOS);
		hd_info[drive].wpcom = *(unsigned short *) (5+BIOS);
		hd_info[drive].ctl = *(unsigned char *) (8+BIOS);
		hd_info[drive].lzone = *(unsigned short *) (12+BIOS);
		hd_info[drive].sector = *(unsigned char *) (14+BIOS);
		BIOS += 16;
	}
	return;
}

/*void print_hdisk_identify(unsigned short *hdbuf) {
	
}*/

void hdisk_sendCmd(struct CMD_BLK_REG *cmd_blk_reg) {
	controller_ready();
	
	_io_out8(CTL_BLK_REG, hd_info[0].ctl);				/*0x3F6*/
	_io_out8(PRI_FEATURES, cmd_blk_reg->features);		/*0x1F1*/
	_io_out8(PRI_SECTORCOUNT, cmd_blk_reg->count);	/*0x1F2*/
	_io_out8(PRI_LBA_L, cmd_blk_reg->lba_low);		/*0x1F3*/
	_io_out8(PRI_LBA_M, cmd_blk_reg->lba_mid);		/*0x1F4*/
	_io_out8(PRI_LBA_H, cmd_blk_reg->lba_high);		/*0x1F5*/
	_io_out8(PRI_DEVICE, cmd_blk_reg->device);		/*0x1F6*/
	_io_out8(PRI_CMD, cmd_blk_reg->command);		/*0x1F7*/
	
	return;
}

void hdisk_out(unsigned int drive, unsigned int nr_sect, unsigned int sect, unsigned int head, unsigned int cyl, unsigned int cmd, void (* intr_addr)(void)) {
	sect = 1;
	head = 0;
	cyl = 1;
	if (drive > 1) {
		while (1) { _io_hlt(); }
	}
	controller_ready();
	do_hdisk = intr_addr;
	/*_io_out8(CTL_BLK_REG, 0);*/
	_io_out8(CTL_BLK_REG, hd_info[drive].ctl);
	_io_out8(PRI_FEATURES, hd_info[drive].wpcom >> 2);
	_io_out8(PRI_SECTORCOUNT, nr_sect);
	_io_out8(PRI_LBA_L, sect);
	_io_out8(PRI_LBA_M, cyl);
	_io_out8(PRI_LBA_H, cyl >> 8);
	_io_out8(PRI_DEVICE, 0xA0 | (drive << 4) | head);
	_io_out8(PRI_CMD, cmd);
	return;
}

/*判断并循环等待硬盘控制器就绪*/
int controller_ready(void) {
	int retry = 100000;
	while (--retry && (_io_in8(PRI_STATUS) & 0xc0) != 0x40) {  }
	return retry;
}

/*检测硬盘执行命令后的状态*/
int winchester_result(void) {
	int i = _io_in8(PRI_STATUS);
	if ((i & (STATUS_BSY | STATUS_READY | STATUS_DFSE | STATUS_DSC | STATUS_ERR)) == (STATUS_READY | STATUS_DSC))
		return 0;
	if (i & 1)
		i = _io_in8(PRI_ERROR);
	return i;
}

/*等待硬盘就绪*/
int drive_busy(void) {
	unsigned int i;
	for (i = 0; i < 100000; ++i) {
		if (STATUS_READY == (_io_in8(PRI_STATUS) & (STATUS_BSY | STATUS_READY)))
			break;
	}
	i = _io_in8(PRI_STATUS);
	i &= STATUS_BSY | STATUS_READY | STATUS_DSC;
	if (i == (STATUS_READY | STATUS_DSC))
		return 0;
	return 1;
}

/*诊断复位硬盘控制器*/
void reset_controller(void) {
	_io_out8(CTL_BLK_REG, 4);
	int i;
	for (i = 0; i < 1000; ++i) { _io_hlt(); }
	_io_out8(CTL_BLK_REG, hd_info[0].ctl & 0x0F);
	if (drive_busy()) { while (1) { _io_hlt(); }}
	if ((i = _io_in8(PRI_ERROR)) != 1) { while (1) { _io_hlt(); } }
}

/*复位硬盘nr*/
void reset_hdisk(int nr) {
	reset_controller();
	hdisk_out(nr, hd_info[nr].sector, hd_info[nr].sector, hd_info[nr].head - 1, hd_info[nr].cyl, ATA_SPECIFY, &recal_intr);
	return;
}

void unexpected_hdisk_interrupt(void) {
	/*while (1) { _io_hlt(); }*/
}

/*读写硬盘失败处理调用函数*/
void bad_rw_intr(void) {
	if (++hdisk_requests.first_request->error >= MAX_ERROR) {
		end_hdisk_request();
	}
	if (hdisk_requests.first_request->error > MAX_ERROR / 2) {
		reset = 1;
	}
}

void read_intr(void) {
	if (winchester_result()) {
		while (1) { _io_hlt(); }
		bad_rw_intr();
		do_hdisk_request();
		return;
	}
	_port_read(PRI_DATA, hdisk_requests.first_request->buffer, 256);
	hdisk_requests.first_request->error = 0;
	/*hdisk_requests.first_request->buffer += 512;*/
	hdisk_requests.first_request->sector++;
	if (--hdisk_requests.first_request->nr_sector) {
		do_hdisk = &read_intr;
		return;
	}
	end_hdisk_request();
	do_hdisk_request();
}

void write_intr(void) {
	if (winchester_result()) {
		bad_rw_intr();
		do_hdisk_request();
		return;
	}
	if (--hdisk_requests.first_request->nr_sector) {
		hdisk_requests.first_request->sector++;
		/*hdisk_requests.first_request->buffer += 512;*/
		do_hdisk = &write_intr;
		_port_write(PRI_DATA, hdisk_requests.first_request->buffer, 512);
		return;
	}
	end_hdisk_request();
	do_hdisk_request();
}

void recal_intr(void) {
	if (winchester_result())
		bad_rw_intr();
	do_hdisk_request();
	return;
}

void do_hdisk_request() {
	if (hdisk_requests.first_request == 0) {
		return;
	}
	int i, r = 0;
	unsigned int block, dev;
	unsigned int sector, head, cyl;
	unsigned int nr_sect = 1;
	
	dev = MINOR(hdisk_requests.first_request->dev);
	block = hdisk_requests.first_request->sector;
	
	block += hdisk_struct[dev].start_sector;
	dev /= 5;
	cyl = block / (hd_info[dev].head * hd_info[dev].sector);
	block %= hd_info[dev].head * hd_info[dev].sector;
	if (block == 0) {
		--cyl;
		head = hd_info[dev].head - 1;
		sector = hd_info[dev].sector;
	}
	else {
		head = block / hd_info[dev].sector;
		if (block % hd_info[dev].sector == 0) {
			--head;
			sector = hd_info[dev].sector;
		}
		else {
			sector = block % hd_info[dev].sector;
		}
	}
	
	if (reset) {
		reset = 0;
		recalibrate = 1;
		reset_hdisk((hdisk_requests.first_request->dev & 0xFF) / 5);
		return;
	}
	if (recalibrate) {
		recalibrate = 0;
		hdisk_out(dev, hd_info[(hdisk_requests.first_request->dev & 0xFF) / 5].sector, 0, 0, 0, ATA_RESTORE, &recal_intr);
		return;
	}
	if (hdisk_requests.first_request->cmd == WRITE) {
		hdisk_out(dev, nr_sect, sector, head, cyl, ATA_WRITE, &write_intr);
		for (i = 0; i < 30000 && !(r = (_io_in8(PRI_STATUS) & STATUS_DRQ)); ++i) { }
		if (!r) {
			bad_rw_intr();
		}
		_port_write(PRI_DATA, hdisk_requests.first_request->buffer, 512);
	}
	else if (hdisk_requests.first_request->cmd == READ) {
		hdisk_out(dev, nr_sect, sector, head, cyl, ATA_READ, &read_intr);
	}
	else {
		while (1) { _io_hlt(); }
	}
}

void _inthandler2e(int *esp) {
	_io_out8(0x20, 0xA0);
	_io_out8(PIC1_OCW2, 0x66);
	_io_out8(PIC0_OCW2, 0x62);
	/*while (1) { _io_hlt(); }*/
	if (do_hdisk == 0) {
		unexpected_hdisk_interrupt();
	}
	else {
		do_hdisk();
	}
	_io_out8(0x20, 0x20);
	/*STATUS_REG hd_status = _io_in8(PRI_STATUS);*/
	/*fifo16_put*/
	return;
}
