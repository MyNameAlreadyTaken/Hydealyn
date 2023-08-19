#include <stdio.h>
#include "chr.h"

struct MOUSE_DEC {
	unsigned char buf[3], phase;
	int x, y, btn;
};
struct TSS32 {
	int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
	int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
	int es, cs, ss, ds, fs, gs;
	int ldtr, iomap;
};
struct SHEET {
	struct SHTCTL *ctl;
	unsigned char title[16];
	unsigned char *buf;
	int bxsize, bysize, vx0, vy0, col_inv, height, flags;
	struct PROCESS *process;
};
struct mess1 {
	int m1i1;
	int m1i2;
	int m1i3;
	int m1i4;
};
struct mess2 {
	void *m2p1;
	void *m2p2;
	void *m2p3;
	void *m2p4;
};
struct mess3 {
	int m3i1;
	int m3i2;
	int m3i3;
	int m3i4;
	long long m3l1;
	long long m3l2;
	void *m3p1;
	void *m3p2;
};
union messes {
	struct mess1 m1;
	struct mess2 m2;
	struct mess3 m3;
};
struct MESSAGE {
	int source;
	int type;
	union messes u;
};

/*bootpack*/

void _io_hlt(void);
void _write_mem8(int addr, int data);
void _io_delay(void);
void _io_cli(void);
void _io_sti(void);
void _io_stihlt(void);
int _io_in8(int port);
int _io_in16(int port);
int _io_in32(int port);
void _io_out8(int port, int data);
void _io_out16(int port, int data);
void _io_out32(int port, int data);
void _port_read(unsigned short port, void *buf, int n);
void _port_write(unsigned short port, void *buf, int n);
int _io_load_eflags(void);
void _io_store_eflags(int eflags);
void _asm_inthandler20(void);
void _asm_inthandler21(void);
void _asm_inthandler27(void);
void _asm_inthandler2c(void);
void _asm_inthandler2e(void);
int _load_cr0(void);
void _store_cr0(int cr0);
int _load_tr(int tr);
unsigned int _memtest_sub(unsigned int start, unsigned int end);
void _farjmp(int eip, int cs);
void _disable_irq(int irq, void (*_inthandle)(int *esp));
void _enable_irq(int irq);

/*ui*/

#define COL8_000000		0
#define COL8_FF0000		1
#define COL8_00FF00		2
#define COL8_FFFF00		3
#define COL8_0000FF		4
#define COL8_FF00FF		5
#define COL8_00FFFF		6
#define COL8_FFFFFF		7
#define COL8_C6C6C6		8
#define COL8_840000		9
#define COL8_008400		10
#define COL8_848400		11
#define COL8_000084		12
#define COL8_840084		13
#define COL8_008484		14
#define COL8_848484		15

#define	ADR_BOOTINFO	0x0ff0

struct BOOTINFO {
	char cyls, leds, vmode, reserve;
	short scrnx, scrny;
	unsigned char *vram;
};
void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void init_screen(unsigned char *vram, int xsize, int ysize);
void fillbox8(unsigned char *vram, int xsize, unsigned char color, int x0, int y0, int x1, int y1);
void putblock8_8(unsigned char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char *buf, int bxsize);
void putFont8(unsigned char *vram, int xsize, int x, int y, char color, const unsigned char *font);
void putFonts8(unsigned char *vram, int xsize, int x, int y, char color, unsigned char *fonts);
void changeFont8(struct SHEET *sht, int x, int y, int color0, int color1, unsigned char *font);
void changeFonts8(struct SHEET *sht, int x0, int y0, int x1, int y1, int color0, int color1, unsigned char *fonts);

/*mem*/

#define MEMMAN_ADDR 0x004C0000
#define MEMMAN_FREES 4090

struct FREEINFO {
	unsigned int addr, size;
};
struct MEMMAN {
	int frees, maxfrees, lostsize, losts;
	struct FREEINFO free[MEMMAN_FREES];
};
unsigned int memtest(unsigned int start, unsigned int end);
void memman_init(struct MEMMAN *man);
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size);
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size);
unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size);
int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size);

/*fifo*/

struct FIFO8 {
	unsigned char *buf;
	int p, q, size, free, flags;
	struct PROCESS *process;
};
void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf, struct PROCESS *process);
int fifo8_put(struct FIFO8 *fifo, unsigned char data);
int fifo8_get(struct FIFO8 *fifo);
int fifo8_status(struct FIFO8 *fifo);

struct FIFO16 {
	unsigned short *buf;
	int p, q, size, free, flags;
	struct PROCESS *process;
};
void fifo16_init(struct FIFO16 *fifo, int size, unsigned short *buf, struct PROCESS *process);
int fifo16_put(struct FIFO16 *fifo, unsigned short data);
int fifo16_get(struct FIFO16 *fifo);
int fifo16_status(struct FIFO16 *fifo);

/*int*/

#define PORT_KEYDAT 0x0060
#define PORT_KEYSTA 0x0064
#define PORT_KEYCMD 0x0064
#define KEYSTA_SEND_NOTREADY 0x02
#define KEYCMD_WRITE_MODE 0x60
#define KBC_MODE 0x47

#define KEYCMD_SENDTO_MOUSE 0xd4
#define MOUSECMD_ENABLE 0xf4

#define PIC0_ICW1		0x0020
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1

void init_pic(void);
void wait_KBC_sendready(void);
void enable_keyboard(struct FIFO16 *fifo);
void enable_mouse(struct FIFO16 *fifo, struct MOUSE_DEC *mdec);
void _inthandler21(int *esp);
void _inthandler27(int *esp);
void _inthandler2c(int *esp);

/*timer*/

#define MAX_TIMER 512

struct TIMER {
	struct TIMER *next;
	unsigned int timeout, flags;
	struct FIFO8* fifo;
	unsigned char data;
};
struct TIMERCTL {
	unsigned int count, next;
	struct TIMER *t0;
	struct TIMER timers0[MAX_TIMER];
};
void init_pit(void);
void _inthandler20(int *esp);
struct TIMER *timer_alloc(void);
void timer_free(struct TIMER *timer);
void timer_parse(struct TIMER *timer);
void timer_init(struct TIMER *timer, struct FIFO8 *fifo, unsigned char data);
void timer_settime(struct TIMER *timer, unsigned int timeout);

/*dsctbl*/

#define ADR_IDT		0x0026f800
#define LIMIT_IDT		0x000007ff
#define ADR_GDT		0x00270000
#define LIMIT_GDT		0x0000ffff
#define ADR_BOTPAK	0x00300000
#define LIMIT_BOTPAK	0x0007ffff
#define AR_DATA32_RW	0x4092
#define AR_CODE32_ER	0x409a
#define AR_TSS32		0x0089
#define AR_INTGATE32	0x008e

void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);

void init_gdtidt(void);

struct SEGMENT_DESCRIPTOR {
	unsigned short limit_low, base_low;
	unsigned char base_mid, access_right;
	unsigned char limit_high, base_high;
};
struct GATE_DESCRIPTOR {
	unsigned short offset_low, selector;
	unsigned char dw_count, access_right;
	unsigned short offset_high;
};
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);

/*process*/

#define MAX_PROCESSES 1024
#define PROCESS_GDT0 3
#define MAX_PROCESSES_LV 64
#define MAX_PROCESSLEVELS 16

struct STACKFRAME {
	unsigned int gs;
	unsigned int fs;
	unsigned int es;
	unsigned int ds;
	unsigned int edi;
	unsigned int esi;
	unsigned int ebp;
	unsigned int kernel_esp;
	unsigned int ebx;
	unsigned int edx;
	unsigned int ecx;
	unsigned int eax;
	unsigned int retaddr;
	unsigned int eip;		/*指向代码*/
	unsigned int cs;
	unsigned int eflags;
	unsigned int esp;		/*指向堆栈*/
	unsigned int ss;
};
struct PROCESS {
	struct STACKFRAME regs;
	int pid;
	int sel, flags;	/*sel: GDT编号*/
	int level, index0, priority;
	struct SEGMENT_DESCRIPTOR seg_descriptor0;
	struct SEGMENT_DESCRIPTOR seg_descriptor1;
	int p_recvfrom;
	int p_sendto;
	int has_int_msg;
	struct MESSAGE p_msg;
	struct PROCESS *q_sending;
	struct PROCESS *next_sending;
	struct TIMER *timer;
	struct FIFO8 fifo8;
	struct FIFO16 fifo16;
	struct TSS32 tss;
	int ds_base, cons_stack;
	void (*close)(struct SHEET *sheet);
};
struct PROCESSLEVEL {
	int running;
	int now;
	struct PROCESS *processes[MAX_PROCESSES_LV];
};
struct PROCESSCTL {
	int now_lv;
	int lv_change;
	struct PROCESS *main_process;
	struct PROCESSLEVEL level[MAX_PROCESSLEVELS];
	struct PROCESS processes0[MAX_PROCESSES];
};
struct PROCESS *process_init(struct MEMMAN *memman);
void init_main_process(struct FIFO8 *fifo8, struct FIFO16 *fifo16);
struct PROCESS *process_alloc(void);
void process_run(struct PROCESS *process, int level, int priority);
void process_switch(void);
void process_sleep(struct PROCESS *process);
struct PROCESS *process_now(void);
char process_add(struct PROCESS *process);
void process_remove(struct PROCESS *process);
void process_switchsub(void);
void process_idle(void);
int process_get_index0(struct PROCESS *process);

/*sheet*/

#define MAX_SHEETS 256
#define SHEET_HAVE_HEAD 0x10
#define CLOSEBTN_CLICKED 0x100
#define SHEET_HAVE_CURSOR 0x1000

struct SHTCTL {
	unsigned char *vram, *map;
	int xsize, ysize, top;
	struct SHEET *focus_on;
	struct SHEET *sheets[MAX_SHEETS];
	struct SHEET sheets0[MAX_SHEETS];
};
struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize);
struct SHEET *sheet_alloc(struct SHTCTL *ctl);
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv);
void sheet_updown(struct SHEET *sht, int height);
void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0, int h1);
void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1);
void sheet_slide(struct SHEET *sht, int vx0, int vy0);
void sheet_free(struct SHEET *sht);
void sheet_refreshmap(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0);

/*win*/

void draw_title(struct SHEET *sht);
void draw_closeBtnClicked(struct SHEET *sht);
void draw_textbox(struct SHEET *sht, int x0, int y0, int sx, int sy, int c);

/*mouse*/

void init_mouse(char *mouse, char bgColor);
int mouse_decode(struct MOUSE_DEC *mouseDec, unsigned char dat, struct SHEET *sht_back);

/*console*/

#define BCD_TO_BIN(val) ((val)=((val)&15) + ((val)>>4)*10)

unsigned char cmos_read(int addr);
struct PROCESS *create_console(struct MEMMAN *memman, struct SHTCTL *shtctl, struct SHEET *sht);
void console_process(struct SHEET *sheet);
void close_console(struct SHEET *sheet);
void console_newline(struct SHEET *sheet, int *cursor_x, int *cursor_y);
void console_println(struct SHEET *sheet, int *cursor_x, int *cursor_y, unsigned char *buf);
void command(struct SHEET *sheet, unsigned char *cmd, int *cursor_x, int *cursor_y);
void command_clear(struct SHEET *sheet, int *cursor_y);
void command_console(struct SHEET *sheet);
void command_exit(struct SHEET *sheet);
void command_hdisk(struct SHEET *sheet, int *cursor_x, int *cursor_y);
void command_hdisk_info(struct SHEET *sheet, int *cursor_x, int *cursor_y);
void command_time(struct SHEET *sheet, int *cursor_x, int *cursor_y);
void command_direct_hdisk_read(struct SHEET *sheet, int *cursor_x, int *cursor_y);
void command_direct_hdisk_write(struct SHEET *sheet, int *cursor_x, int *cursor_y);
void command_test(struct SHEET *sheet, int *cursor_x, int *cursor_y);

/*hdisk.c*/

#define MAX_DRIVES	2					/*最大磁盘数*/
#define READ 0
#define WRITE 1
#define READ_AHEAD 2
#define WRITE_AHEAD 3
/* seperate major and minor numbers from device number*/
#define MAJOR(a) (((unsigned)(a)) >> 8)
#define MINOR(a) ((a) & 0xFF)
#define SECTOR_SIZE	512

struct hdisk_info {
	int head;		/*磁头数*/
	int sector;		/*每磁道扇区数*/
	int cyl;		/*柱面数*/
	int wpcom;	/*写前预补偿柱面号*/
	int lzone;		/*磁头着陆区柱面号*/
	int ctl;		/*控制字节*/
};
struct HDISK_STRUCT {
	unsigned int start_sector;
	unsigned int nr_sectors;
};
struct PARTITION {
	unsigned char boot_ind;		/**/
	unsigned char head;
	unsigned char sector;
	unsigned char cyl;
	unsigned char sys_ind;
	unsigned char end_head;
	unsigned char end_sector;
	unsigned char end_cyl;
	unsigned int start_sector;
	unsigned int nr_sectors;
};
unsigned char init_hdisk();
void get_partition_table(int drive, struct PARTITION partition, int sector_nr);
void hdisk_process();
void read_hdisk_info();
unsigned char *hdisk_identify(unsigned char drive);
void do_hdisk_request(void);
void _inthandler2e(int *esp);

/*time.c*/

struct tm {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;
};
long mktime(struct tm *tm);
void read_time(struct tm *time);

/*hdisk_request*/

struct HDISK_REQUEST {
	unsigned int dev;
	unsigned int cmd;
	unsigned int error;
	unsigned int sector;
	unsigned int nr_sector;
	char *buffer;
	struct PROCESS *process;
	struct HDISK_REQUEST *next;
};
struct HDISK_REQUESTS {
	struct HDISK_REQUEST *first_request;
	struct HDISK_REQUEST *last_request;
};
void direct_read_hdisk(int sector);
void direct_write_hdisk(int sector);
void add_hdisk_request(struct HDISK_REQUEST *request);
void end_hdisk_request();

/*file_system.c*/
