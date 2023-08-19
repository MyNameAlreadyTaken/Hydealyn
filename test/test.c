#include <stdio.h>

void init_mouse(char *mouse, char bgColor) {
	char cursor[16][16] = {
		"**--------------",
		"*0*-------------",
		"*00*------------",
		"*000*-----------",
		"*0000*----------",
		"*00000*---------",
		"*000000*--------",
		"*0000000*-------",
		"*00000000*------",
		"*000000000*-----",
		"*00000*0000*----",
		"*0000*-*0000*---",
		"*000*----*000*--",
		"*00*-------*00*-",
		"*0*----------*0*",
		"*--------------*"
	};
	int x, y;
	int temp = -16;
	for (y = 0; y < 16; ++y) {
		temp += 16;
		for (x = 0; x < 16; ++x) { 
			if (cursor[y][x] == '0')
				mouse[temp + x] = 0;
			else if (cursor[y][x] == '*')
				mouse[temp + x] = 1;
			else
				mouse[temp + x] = 2;
		}
	}
	return;
}

int main() {
	unsigned char buf_mouse[256];
	init_mouse(buf_mouse, 2);
	int x, y;
	int temp = -16;
	for (y = 0; y < 16; ++y) {
		temp += 16;
		for (x = 0; x < 16; ++x) {
			printf("%d", buf_mouse[temp + x]);
		}
		printf("\n");
	}
	int count = 0;
	for (x = 0; x < 256; ++x) {
		count += x;
	}
	printf("%d", count);
	return 0;
}

