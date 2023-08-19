#include "string.h"

void sprintf(unsigned char *out, ) {

}

char strcmp(unsigned char *str1, unsigned char *str2) {
	unsigned int i;
	while (str1[i] != 0 && str2[i] != 0) {
		if (str1[i] == str2[i])
			continue;
		else if (str1[i] < str2[i])
			return 1;
		else
			return -1;
	}
	if (str1[i] == str2[i])
		return 0;
	else if (str1[i] == 0)
		return 1;
	else
		return -1;
}
