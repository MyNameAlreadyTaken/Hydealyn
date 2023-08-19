#include "head.h"

#define PIT_CTRL 0x0043
#define PIT_CNT0 0x0040

#define TIMER_FLAGS_ALLOC 1
#define TIMER_FLAGS_USING 2

struct TIMERCTL timerctl;
extern struct TIMER *processTimer;

void init_pit(void) {
	_io_out8(PIT_CTRL, 0x34);
	_io_out8(PIT_CNT0, 0x9C);	/*11932=0x2E9C*/
	_io_out8(PIT_CNT0, 0x2E);
	timerctl.count = 0;
	int i;
	for (i = 0; i < MAX_TIMER; ++i) {
		timerctl.timers0[i].flags = 0;
	}
	struct TIMER *t = timer_alloc();
	t->timeout = 0xFFFFFFFF;
	t->flags = TIMER_FLAGS_USING;
	t->next = 0;
	timerctl.t0 = t;
	timerctl.next = 0xFFFFFFFF;
	//_io_out8(PIC0_IMR, 0xfe);
	return;
}

void _inthandler20(int *esp) {
	_io_out8(PIC0_OCW2, 0x60);
	timerctl.count++;
	if (timerctl.next > timerctl.count) {
		return;
	}
	struct TIMER *timer = timerctl.t0;
	char ts = 0;
	for ( ; ; ) {
		if (timer->timeout > timerctl.count) {
			break;
		}
		timer->flags = TIMER_FLAGS_ALLOC;
		if (timer != processTimer) {
			fifo8_put(timer->fifo, timer->data);
		}
		else {
			ts = 1;
		}
		timer = timer->next;
	}
	timerctl.t0 = timer;
	timerctl.next = timerctl.t0->timeout;
	if (ts != 0) {
		process_switch();
	}
	return;
}

struct TIMER *timer_alloc(void) {
	int i;
	for (i = 0; i < MAX_TIMER; ++i) {
		if (timerctl.timers0[i].flags == 0) {
			timerctl.timers0[i].flags = TIMER_FLAGS_ALLOC;
			return &timerctl.timers0[i];
		}
	}
	return 0;
}

void timer_free(struct TIMER *timer) {
	timer->flags = 0;
	return;
}

void timer_parse(struct TIMER *timer) {
	if (timer->flags != TIMER_FLAGS_USING)
		return;
	int e = _io_load_eflags();
	_io_cli();
	timer->flags = TIMER_FLAGS_ALLOC;
	if (timerctl.t0 == timer) {
		timerctl.t0 = timer->next;
		timerctl.next = timer->next->timeout;
	}
	else {
		struct TIMER *pre = timerctl.t0;
		for ( ; ; ) {
			if (pre->next != timer)
				pre = pre->next;
			else
				break;
		}
		pre->next = timer->next;
	}
	_io_store_eflags(e);
	_io_sti();
	return;
/*
	int e;
	struct TIMER *t;
	e = _io_load_eflags();
	if (timer->flags == TIMER_FLAGS_USING) {	
		if (timer == timerctl.t0) {
			t = timer->next;
			timerctl.t0 = t;
			timerctl.next = t->timeout;
		} else {
			t = timerctl.t0;
			for (;;) {
				if (t->next == timer) {
					break;
				}
				t = t->next;
			}
			t->next = timer->next;
		}
		timer->flags = TIMER_FLAGS_ALLOC;
		_io_store_eflags(e);
		return;	
	}
	_io_store_eflags(e);
	_io_sti();
	return;*/
}

void timer_init(struct TIMER *timer, struct FIFO8 *fifo, unsigned char data) {
	timer->fifo = fifo;
	timer->data = data;
	return;
}

void timer_settime(struct TIMER *timer, unsigned int timeout) {
	timer->timeout = timeout + timerctl.count;
	timer->flags = TIMER_FLAGS_USING;
	int e = _io_load_eflags();
	_io_cli();
	struct TIMER *t = timerctl.t0;
	if (timer->timeout <= t->timeout) {
		timerctl.t0 = timer;
		timer->next = t;
		timerctl.next = timer->timeout;
		_io_store_eflags(e);
		_io_sti();
		return;
	}
	struct TIMER *s;
	for ( ; ; ) {
		s = t;
		t = t->next;
		if (timer->timeout <= t->timeout) {
			s->next = timer;
			timer->next = t;
			_io_store_eflags(e);
			_io_sti();
			return;
		}
	}
}
