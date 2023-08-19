#include "head.h"


void init_mouse(char *mouse, char bgColor) {
	const short cursor1[256] = {
		1,1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
		1,2,1,3,3,3,3,3,3,3,3,3,3,3,3,3,
		1,2,2,1,3,3,3,3,3,3,3,3,3,3,3,3,
		1,2,2,2,1,3,3,3,3,3,3,3,3,3,3,3,
		1,2,2,2,2,1,3,3,3,3,3,3,3,3,3,3,
		1,2,2,2,2,2,1,3,3,3,3,3,3,3,3,3,
		1,2,2,2,2,2,2,1,3,3,3,3,3,3,3,3,
		1,2,2,2,2,2,2,2,1,3,3,3,3,3,3,3,
		1,2,2,2,2,2,2,2,2,1,3,3,3,3,3,3,
		1,2,2,2,2,2,2,2,2,2,1,3,3,3,3,3,
		1,2,2,2,2,2,1,2,2,2,2,1,3,3,3,3,
		1,2,2,2,2,1,3,1,2,2,2,2,1,3,3,3,
		1,2,2,2,1,3,3,3,3,1,2,2,2,1,3,3,
		1,2,2,1,3,3,3,3,3,3,3,1,2,2,1,3,
		1,2,1,3,3,3,3,3,3,3,3,3,3,1,2,1,
		1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,1
	};

	int x, y;
	int temp = 0;
	int count = 0;
	for (x = 0; x < 256; ++x) {
		
		if (cursor[x] == 1) {
			count++;
			mouse[x] = 0;
		}
		else {
			if (cursor[x] == 2) {
				mouse[x] = 7;
			}
			else {
				mouse[x] = bgColor;
			}
		}
	}
	//for (y = 0; y < 16; ++y) {
	//	for (x = 0; x < 16; ++x) {
	//	int t = temp + x;
	//	count += cursor[t];
	//		if (cursor[t] == 1) {
	//			*(mouse + temp + x) = bgColor;
	//		}
	//		else if (cursor[t] == 3) {
	//			*(mouse + temp + x) = 7;
	//		}
	//		else if (cursor[t] == 2) {
	//			*(mouse + temp + x) = 2;
	//		}
	//		else {
	//			*(mouse + temp + x) = 0;
	//		}
	//	}
	//	temp += 16;
	//}
	
	//count = 0;
	//for (x = 0; x < 1536; ++x) {
	//	count += *(ascii + x);
	//}
	unsigned int why = 13;
	int t1;
	short t2 = cursor[t1 = why];
	const short test1[2] = {1, 2};
	//int t3 = 
	
	//if (cursor[&cursor[why] - cursor] == cursor[13]) {
	//if (cursor1[13] == 0) {
	const short *p = cursor;
	//if (p == cursor) {
	if (test1[0] == 0) {
	for (y = 5; y < 11; ++y) {
		temp = y << 4;
		for (x = 5; x < 11; ++x) {
			mouse[temp + x] = 7;
		}
	}
	}
	return;
}

int mouse_decode(struct MOUSE_DEC *mouseDec, unsigned char dat, struct SHEET *sht_back) {
	if (mouseDec->phase == 0) {
		if (dat == 0xfa) {
			mouseDec->phase = 1;
		}
		return 0;
	}
	else if (mouseDec->phase == 1) {
		if ((dat & 0xc8) == 0x08) {
			mouseDec->buf[0] = dat;
			mouseDec->phase = 2;
		}
		return 0;
	}
	else if (mouseDec->phase == 2) {
		mouseDec->buf[1] = dat;
		mouseDec->phase = 3;
		return 0;
	}
	else if (mouseDec->phase == 3) {
		mouseDec->buf[2] = dat;
		mouseDec->phase = 1;
		mouseDec->btn = mouseDec->buf[0] & 0x07;
		mouseDec->x = mouseDec->buf[1];
		mouseDec->y = mouseDec->buf[2];
		if ((mouseDec->buf[0] & 0x10) != 0) {
			mouseDec->x |= 0xffffff00;
		}
		if ((mouseDec->buf[0] & 0x20) != 0) {
			mouseDec->y |= 0xffffff00;
		}
		mouseDec->y = - mouseDec->y;
		unsigned char s[40];
		struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
		//sprintf(s, "%02X %02X %02X", mouseDec->buf[0], mouseDec->buf[1], mouseDec->buf[2]);
		fillbox8(sht_back->buf, binfo->scrnx, COL8_00FFFF, 0, 0, 64, 16);
		//putFonts8(sht_back->buf, binfo->scrnx, 0, 0, COL8_000000, s);
		sheet_refresh(sht_back, 0, 0, 64, 16);
		return 1;
	}
	return -1;
}

