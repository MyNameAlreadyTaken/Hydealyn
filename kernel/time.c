#include "head.h"
#include "time.h"

long mktime(struct tm *tm) {
	int year = tm->tm_year - 70;
	long res = YEAR * year  + DAY * ((year + 1) / 4);
	res += month[tm->tm_mon];
	if (tm->tm_mon > 1 && ((year + 2) % 4)) {
		res -= DAY;
	}
	res += DAY * (tm->tm_mday - 1);
	res += HOUR * tm->tm_hour;
	res += MINUTE * tm->tm_min;
	res += tm->tm_sec;
	return res;
}

void read_time(struct tm *time) {
	time->tm_sec = cmos_read(0);
	time->tm_min = cmos_read(2);
	time->tm_hour = cmos_read(4);
	time->tm_mday = cmos_read(7);
	time->tm_mon = cmos_read(8);
	time->tm_year = cmos_read(9);
	BCD_TO_BIN(time->tm_sec);
	BCD_TO_BIN(time->tm_min);
	BCD_TO_BIN(time->tm_hour);
	BCD_TO_BIN(time->tm_mday);
	BCD_TO_BIN(time->tm_mon);
	BCD_TO_BIN(time->tm_year);
	return;
}
