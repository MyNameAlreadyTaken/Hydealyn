#include "head.h"

#define FLAGS_OVERRUN		0x0001

void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf, struct PROCESS *process) {
	fifo->size = size;
	fifo->buf = buf;
	fifo->free = size; 
	fifo->flags = 0;
	fifo->p = 0;
	fifo->q = 0;
	fifo->process = process;
	return;
}

int fifo8_put(struct FIFO8 *fifo, unsigned char data) {
	if (fifo->free == 0) {
		fifo->flags |= FLAGS_OVERRUN;
		return -1;
	}
	fifo->buf[fifo->p] = data;
	fifo->p++;
	if (fifo->p == fifo->size) {
		fifo->p = 0;
	}
	fifo->free--;
	if (fifo->process != 0) {
		if (fifo->process->flags != 2) {
			process_run(fifo->process, -1, 0);
		}
	}
	return 0;
}

int fifo8_get(struct FIFO8 *fifo) {
	unsigned char data;
	if (fifo->free == fifo->size) {
		return -1;
	}
	data = fifo->buf[fifo->q];
	fifo->q++;
	if (fifo->q == fifo->size) {
		fifo->q = 0;
	}
	fifo->free++;
	return data;
}

int fifo8_status(struct FIFO8 *fifo) {
	return fifo->size - fifo->free;
}

void fifo16_init(struct FIFO16 *fifo, int size, unsigned short *buf, struct PROCESS *process) {
	fifo->size = size;
	fifo->buf = buf;
	fifo->free = size;
	fifo->flags = 0;
	fifo->p = 0;
	fifo->q = 0;
	fifo->process = process;
	return;
}

int fifo16_put(struct FIFO16 *fifo, unsigned short data) {
	if (fifo->free == 0) {
		fifo->flags |= FLAGS_OVERRUN;
		return -1;
	}
	fifo->buf[fifo->p] = data;
	++(fifo->p);
	if (fifo->p == fifo->size) {
		fifo->p = 0;
	}
	--(fifo->free);
	if (fifo->process != 0) {
		if (fifo->process->flags != 2) {
			process_run(fifo->process, -1, 0);
		}
	}
	return 0;
}

int fifo16_get(struct FIFO16 *fifo) {
	unsigned short data;
	if (fifo->free == fifo->size) {
		return -1;
	}
	data = fifo->buf[fifo->q];
	++(fifo->q);
	if (fifo->q == fifo->size) {
		fifo->q = 0;
	}
	++(fifo->free);
	return data;
}

int fifo16_status(struct FIFO16 *fifo) {
	return fifo->size - fifo->free;
}
