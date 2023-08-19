#include "head.h"

char closeBtn[14][16] = {
	"000000000000000@",
	"0qqqqqqqqqqqqq$@",
	"0qqqqqqqqqqqqq$@",
	"0qqq@@qqqq@@qq$@",
	"0qqqq@@qq@@qqq$@",
	"0qqqqq@@@@qqqq$@",
	"0qqqqqq@@qqqqq$@",
	"0qqqqq@@@@qqqq$@",
	"0qqqq@@qq@@qqq$@",
	"0qqq@@qqqq@@qq$@",
	"0qqqqqqqqqqqqq$@",
	"0qqqqqqqqqqqqq$@",
	"0$$$$$$$$$$$$$$@",
	"@@@@@@@@@@@@@@@@"
};
char closeBtnClicked[14][16] = {
	"@@@@@@@@@@@@@@@@",
	"@$$$$$$$$$$$$$$0",
	"@$qqqqqqqqqqqqq0",
	"@$qqqqqqqqqqqqq0",
	"@$qq@@qqqq@@qqq0",
	"@$qqq@@qq@@qqqq0",
	"@$qqqq@@@@qqqqq0",
	"@$qqqqq@@qqqqqq0",
	"@$qqqq@@@@qqqqq0",
	"@$qqq@@qq@@qqqq0",
	"@$qq@@qqqq@@qqq0",
	"@$qqqqqqqqqqqqq0",
	"@$qqqqqqqqqqqqq0",
	"@000000000000000"
};

void draw_title(struct SHEET *sht) {
	sht->flags |= SHEET_HAVE_HEAD;
	fillbox8(sht->buf, sht->bxsize, COL8_C6C6C6, 0, 0, sht->bxsize - 1, 0);
	fillbox8(sht->buf, sht->bxsize, COL8_FFFFFF, 1, 1, sht->bxsize - 2, 1);
	fillbox8(sht->buf, sht->bxsize, COL8_C6C6C6, 0, 0, 0, sht->bysize - 1);
	fillbox8(sht->buf, sht->bxsize, COL8_FFFFFF, 1, 1, 1, sht->bysize - 2);
	fillbox8(sht->buf, sht->bxsize, COL8_848484, sht->bxsize - 2, 1, sht->bxsize - 2, sht->bysize - 2);
	fillbox8(sht->buf, sht->bxsize, COL8_000000, sht->bxsize - 1, 0, sht->bxsize - 1, sht->bysize - 1);
	fillbox8(sht->buf, sht->bxsize, COL8_C6C6C6, 2, 2, sht->bxsize - 3, sht->bysize - 3);
	fillbox8(sht->buf, sht->bxsize, COL8_000084, 3, 3, sht->bxsize - 4, 20);
	fillbox8(sht->buf, sht->bxsize, COL8_848484, 1, sht->bysize - 2, sht->bxsize - 2, sht->bysize - 2);
	fillbox8(sht->buf, sht->bxsize, COL8_000000, 0, sht->bysize - 1, sht->bxsize - 1, sht->bysize - 1);
	putFonts8(sht->buf, sht->bxsize, 24, 2, COL8_FFFFFF, sht->title);
	draw_closeBtn(sht);
	/*int x, y;
	char c;
	for (y = 0; y < 14; ++y) {
		int temp = (5 + y) * sht->bxsize + sht->bxsize - 21;
		for (x = 0; x < 16; ++x) {
			c = closeBtn[y][x];
			if (c == '@') { c = COL8_000000; }
			else if (c == '$') { c = COL8_848484; }
			else if (c == 'q') { c = COL8_FFFFFF; }
			else { c = COL8_FFFFFF; }
			sht->buf[temp + x] = c;
		}
	}*/
	return;
}

void draw_closeBtn(struct SHEET *sht) {
	int x, y;
	char c;
	for (y = 0; y < 14; ++y) {
		int temp = (5 + y) * sht->bxsize + sht->bxsize - 21;
		for (x = 0; x < 16; ++x) {
			c = closeBtn[y][x];
			if (c == '@') { c = COL8_000000; }
			else if (c == '$') { c = COL8_848484; }
			else if (c == 'q') { c = COL8_FFFFFF; }
			else { c = COL8_FFFFFF; }
			sht->buf[temp + x] = c;
		}
	}
	return;
}

void draw_closeBtnClicked(struct SHEET *sht) {
	int x, y;
	char c;
	for (y = 0; y < 14; ++y) {
		int temp = (5 + y) * sht->bxsize + sht->bxsize - 21;
		for (x = 0; x < 16; ++x) {
			c = closeBtnClicked[y][x];
			if (c == '@') { c = COL8_000000; }
			else if (c == '$') { c = COL8_848484; }
			else if (c == 'q') { c = COL8_FFFFFF; }
			else { c = COL8_FFFFFF; }
			sht->buf[temp + x] = c;
		}
	}
	return;
}

void draw_textbox(struct SHEET *sht, int x0, int y0, int sx, int sy, int c) {
	int x1 = x0 + sx, y1 = y0 + sy;
	fillbox8(sht->buf, sht->bxsize, COL8_848484, x0 - 2, y0 - 3, x1 + 1, y0 - 3);
	fillbox8(sht->buf, sht->bxsize, COL8_848484, x0 - 3, y0 - 3, x0 - 3, y1 + 1);
	fillbox8(sht->buf, sht->bxsize, COL8_FFFFFF, x0 - 3, y1 + 2, x1 + 1, y1 + 2);
	fillbox8(sht->buf, sht->bxsize, COL8_FFFFFF, x1 + 2, y0 - 3, x1 + 2, y1 + 2);
	fillbox8(sht->buf, sht->bxsize, COL8_000000, x0 - 1, y0 - 2, x1, y0 - 2);
	fillbox8(sht->buf, sht->bxsize, COL8_000000, x0 - 2, y0 - 2, x0 - 2, y1);
	fillbox8(sht->buf, sht->bxsize, COL8_C6C6C6, x0 - 2, y1 + 1, x1, y1 + 1);
	fillbox8(sht->buf, sht->bxsize, COL8_C6C6C6, x1 + 1, y0 - 2, x1 + 1, y1 + 1);
	fillbox8(sht->buf, sht->bxsize, c, x0 - 1, y0 - 1, x1, y1);
	return;
}
