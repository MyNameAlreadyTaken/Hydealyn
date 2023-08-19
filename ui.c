#include "head.h"

void init_palette(void) {
	static unsigned char table_rgb[16 * 3] = {
		0x00, 0x00, 0x00,	/*  0: 黑色*/
		0xff, 0x00, 0x00,	/*  1: 亮红*/
		0x00, 0xff, 0x00,	/*  2: 亮绿*/
		0xff, 0xff, 0x00,	/*  3: 亮黄*/
		0x00, 0x00, 0xff,	/*  4: 亮蓝*/
		0xff, 0x00, 0xff,	/*  5: 亮紫*/
		0x00, 0xff, 0xff,	/*  6: 浅亮蓝*/
		0xff, 0xff, 0xff,	/*  7: 白色*/
		0xc6, 0xc6, 0xc6,	/*  8: 亮灰*/
		0x84, 0x00, 0x00,	/*  9: 暗红*/
		0x00, 0x84, 0x00,	/* 10: 暗绿*/
		0x84, 0x84, 0x00,	/* 11: 暗黄*/
		0x00, 0x00, 0x84,	/* 12: 暗蓝*/
		0x84, 0x00, 0x84,	/* 13: 暗紫*/
		0x00, 0x84, 0x84,	/* 14: 浅暗蓝*/
		0x84, 0x84, 0x84	/* 15: 暗灰*/
	};
	set_palette(0, 15, table_rgb);
	return;
}

void set_palette(int start, int end, unsigned char *rgb)
{
	// int i; 
	int eflags;
	eflags = _io_load_eflags();	
	_io_cli(); 					
	_io_out8(0x03c8, start);
	
	_io_out8(0x03c9, 0x00 / 4);
	_io_out8(0x03c9, 0x00 / 4);
	_io_out8(0x03c9, 0x00 / 4);
	
	_io_out8(0x03c9, 0xff / 4);
	_io_out8(0x03c9, 0x00 / 4);
	_io_out8(0x03c9, 0x00 / 4);
	
	_io_out8(0x03c9, 0x00 / 4);
	_io_out8(0x03c9, 0xff / 4);
	_io_out8(0x03c9, 0x00 / 4);
	
	_io_out8(0x03c9, 0xff / 4);
	_io_out8(0x03c9, 0xff / 4);
	_io_out8(0x03c9, 0x00 / 4);
	
	_io_out8(0x03c9, 0x00 / 4);
	_io_out8(0x03c9, 0x00 / 4);
	_io_out8(0x03c9, 0xff / 4);
	
	_io_out8(0x03c9, 0xff / 4);
	_io_out8(0x03c9, 0x00 / 4);
	_io_out8(0x03c9, 0xff / 4);
	
	_io_out8(0x03c9, 0x00 / 4);
	_io_out8(0x03c9, 0xff / 4);
	_io_out8(0x03c9, 0xff / 4);
	
	_io_out8(0x03c9, 0xff / 4);
	_io_out8(0x03c9, 0xff / 4);
	_io_out8(0x03c9, 0xff / 4);
	
	_io_out8(0x03c9, 0xc6 / 4);
	_io_out8(0x03c9, 0xc6 / 4);
	_io_out8(0x03c9, 0xc6 / 4);
	
	_io_out8(0x03c9, 0x84 / 4);
	_io_out8(0x03c9, 0x00 / 4);
	_io_out8(0x03c9, 0x00 / 4);
	
	_io_out8(0x03c9, 0x00 / 4);
	_io_out8(0x03c9, 0x84 / 4);
	_io_out8(0x03c9, 0x00 / 4);
	
	_io_out8(0x03c9, 0x84 / 4);
	_io_out8(0x03c9, 0x84 / 4);
	_io_out8(0x03c9, 0x00 / 4);
	
	_io_out8(0x03c9, 0x00 / 4);
	_io_out8(0x03c9, 0x00 / 4);
	_io_out8(0x03c9, 0x84 / 4);
	
	_io_out8(0x03c9, 0x84 / 4);
	_io_out8(0x03c9, 0x00 / 4);
	_io_out8(0x03c9, 0x84 / 4);
	
	_io_out8(0x03c9, 0x00 / 4);
	_io_out8(0x03c9, 0x84 / 4);
	_io_out8(0x03c9, 0x84 / 4);
	
	_io_out8(0x03c9, 0x84 / 4);
	_io_out8(0x03c9, 0x84 / 4);
	_io_out8(0x03c9, 0x84 / 4);
//	for (i = start; i <= end; i++) {
		//_io_out8(0x03c9, 0xff / 4);
		//_io_out8(0x03c9, 0xff / 4);
		//_io_out8(0x03c9, 0xff / 4);
//		_io_out8(0x03c9, rgb[0] / 4);
//		_io_out8(0x03c9, rgb[1] / 4);
//		_io_out8(0x03c9, rgb[2] / 4);
//		rgb += 3;
//	}
	_io_store_eflags(eflags);	
	return;
}

void init_screen(unsigned char *vram, int xsize, int ysize) {
	fillbox8(vram, xsize, COL8_00FFFF, 0, 0, xsize, 17);
	fillbox8(vram, xsize, COL8_0000FF, 0, 18, xsize, ysize - 18);	
	fillbox8(vram, xsize, COL8_00FFFF, 0, ysize - 17, xsize, ysize);
	//fillbox8(vram, xsize, COL8_FFFFFF, 0, 0, xsize, ysize);
}

void putblock8_8(unsigned char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char *buf, int bxsize) {	/*(px0,py0)左上*/
	int x, y;									/*pxsize横长 pysize竖长*/
	for (y = 0; y < pysize; ++y) {							/*vxsize屏幕横长 bxsize每一行像素数*/
		int temp1 = (py0 + y) * vxsize + px0;
		int temp2 = y * bxsize;
		for (x = 0; x < pxsize; ++x) {
			vram[temp1 + x] = buf[temp2 + x];
		}
	}
	return;
}

void fillbox8(unsigned char *vram, int xsize, unsigned char color, int x0, int y0, int x1, int y1) {	/*(x0, y0)左上 (x1, y1)右下*/
	int x, y;
	int temp = (y0 - 1) * xsize;
	for (y = y0; y < y1; ++y) {
		temp += xsize;
		for (x = x0; x < x1; ++x) {
			vram[temp + x] = color;
		}
	}
	return;
}

void putFont8(unsigned char *vram, int xsize, int x, int y, char color, const unsigned char *font) {
	char data;
	int i;
	int temp = y * xsize + x;
	for (i = 0; i < 16; ++i) {
		temp += xsize;
		data = font[i];
		if ((data & 0x80) != 0) { vram[temp + 0] = color; }
		if ((data & 0x40) != 0) { vram[temp + 1] = color; }
		if ((data & 0x20) != 0) { vram[temp + 2] = color; }
		if ((data & 0x10) != 0) { vram[temp + 3] = color; }
		if ((data & 0x08) != 0) { vram[temp + 4] = color; }
		if ((data & 0x04) != 0) { vram[temp + 5] = color; }
		if ((data & 0x02) != 0) { vram[temp + 6] = color; }
		if ((data & 0x01) != 0) { vram[temp + 7] = color; }
	}
	return;
}

void putFonts8(unsigned char *vram, int xsize, int x, int y, char color, unsigned char *fonts) {
	int p = 0;
	while (*(fonts + p) != 0x00) {
		if (*(fonts + p) == '\\')
			putFont8(vram, xsize, x, y, color, ascii + 960);
		else
			putFont8(vram, xsize, x, y, color, ascii + ((*(fonts + p) - 32) << 4));
		++p;
		x += 8;	
	}
	return;
}

void changeFont8(struct SHEET *sht, int x, int y, int color0, int color1, unsigned char *font) {
	fillbox8(sht->buf, sht->bxsize, color0, x, y, x + 8, y + 16);
	/*if (*font == '\\')
		putFont8(sht->buf, sht->bxsize, x, y, color1, ascii + 960);
	else
		putFont8(sht->buf, sht->bxsize, x, y, color1, ascii + ((*font - 32) << 4));*/
	putFonts8(sht->buf, sht->bxsize, x, y, color1, font);
	sheet_refresh(sht, x, y, x + 8, y + 16);
	return;
}

void changeFonts8(struct SHEET *sht, int x0, int y0, int x1, int y1, int color0, int color1, unsigned char *fonts) {
	fillbox8(sht->buf, sht->bxsize, color0, x0, y0, x1, y1);
	putFonts8(sht->buf, sht->bxsize, x0, y0, color1, fonts);
	sheet_refresh(sht, x0, y0, x1, y1);
	return;
}

