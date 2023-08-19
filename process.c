#include "head.h"

struct PROCESSCTL *processCtl;
struct TIMER *processTimer;

struct PROCESS *process_init(struct MEMMAN *memman) {
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	processCtl = (struct PROCESSCTL *) memman_alloc_4k(memman, sizeof(struct PROCESSCTL));
	int i;
	for (i = 0; i < MAX_PROCESSES; ++i) {
		processCtl->processes0[i].flags = 0;
		processCtl->processes0[i].sel = (PROCESS_GDT0 + i) << 3;
		set_segmdesc(gdt + PROCESS_GDT0 + i, 103, (int) &processCtl->processes0[i].tss, AR_TSS32);
	}
	for (i = 0; i < MAX_PROCESSLEVELS; ++i) {
		processCtl->level[i].running = 0;
		processCtl->level[i].now = 0;
	}
	struct PROCESS *process = process_alloc();
	process->flags = 2;
	process->priority = 2;
	process->level = 0;
	processCtl->main_process = process;
	process_add(process);
	process_switchsub();
	_load_tr(process->sel);
	processTimer = timer_alloc();
	timer_settime(processTimer, process->priority);

	struct PROCESS *idle = process_alloc();
	idle->tss.esp = memman_alloc_4k(memman, 1024) + 1024;
	idle->tss.eip = (int) &process_idle;
	idle->tss.es = 8;
	idle->tss.cs = 16;
	idle->tss.ss = 8;
	idle->tss.ds = 8;
	idle->tss.fs = 8;
	idle->tss.gs = 8;
	process_run(idle, MAX_PROCESSLEVELS - 1, 1);
	
	return process;
}

void init_main_process(struct FIFO8 *fifo8, struct FIFO16 *fifo16) {
	fifo8->process = processCtl->main_process;
	fifo16->process = processCtl->main_process;
	process_run(processCtl->main_process, 1, 2);
	return;
}

struct PROCESS *process_alloc(void) {
	struct PROCESS *process;
	int i;
	for (i = 0; i < MAX_PROCESSES; ++i) {
		if (processCtl->processes0[i].flags == 0) {
			process = &processCtl->processes0[i];
			process->flags = 1;
			process->index0 = i;
			process->tss.eflags = 0x00000202;
			process->tss.eax = 0;
			process->tss.ecx = 0;
			process->tss.edx = 0;
			process->tss.ebx = 0;
			process->tss.ebp = 0;
			process->tss.esi = 0;
			process->tss.edi = 0;
			process->tss.es = 0;
			process->tss.ds = 0;
			process->tss.fs = 0;
			process->tss.gs = 0;
			process->tss.ldtr = 0;
			process->tss.iomap = 0x40000000;
			return process;
		}
	}
	return 0;
}

void process_run(struct PROCESS *process, int level, int priority) {
	if (level < 0) {
		level = process->level;
	}
	if (priority > 0) {
		process->priority = priority;
	}
	if (process->flags == 2 && process->level != level) {
		process_remove(process);
	}
	if (process->flags != 2) {
		process->level = level;	
		process_add(process);
	}
	processCtl->lv_change = 1;
	return;
}

void process_switch(void) {
	struct PROCESSLEVEL *pl = &processCtl->level[processCtl->now_lv];
	struct PROCESS *new_process, *now_process = pl->processes[pl->now];
	++pl->now;
	if (pl->now == pl->running) {
		pl->now = 0;
	}
	if (processCtl->lv_change != 0) {
		process_switchsub();
		pl = &processCtl->level[processCtl->now_lv];
	}
	new_process = pl->processes[pl->now];
	timer_settime(processTimer, new_process->priority);
	if (new_process != now_process) {
		_farjmp(0, new_process->sel);
	}
	return;
}

void process_sleep(struct PROCESS *process) {
	if (process->flags == 2) {
		struct PROCESS *now_process = process_now();
		process_remove(process);
		if (process == now_process) {
			process_switchsub();
			now_process = process_now();
			_farjmp(0, now_process->sel);
		}
	}
	return;
}

struct PROCESS *process_now(void) {
	struct PROCESSLEVEL *pl = &processCtl->level[processCtl->now_lv];
	return pl->processes[pl->now];
}

char process_add(struct PROCESS *process) {
	struct PROCESSLEVEL *pl = &processCtl->level[process->level];
	if (pl->running >= MAX_PROCESSES_LV) {
		return 0;
	}
	pl->processes[pl->running] = process;
	++pl->running;
	process->flags = 2;
	return 1;
}

void process_remove(struct PROCESS *process) {
	struct PROCESSLEVEL *pl = &processCtl->level[process->level];
	int i;
	for (i = 0; i < pl->running; ++i) {
		if (pl->processes[i] == process) {
			break;
		}
	}
	--pl->running;
	if (i < pl->now) {
		--pl->now;
	}
	if (pl->now >= pl->running) {
		pl->now = 0;
	}
	process->flags = 1;
	for ( ; i < pl->running; ++i) {
		pl->processes[i] = pl->processes[i + 1];
	}
	return;
}

void process_switchsub(void) {
	int i;
	for (i = 0; i < MAX_PROCESSLEVELS; ++i) {
		if (processCtl->level[i].running > 0) {
			break;
		}
	}
	processCtl->now_lv = i;
	processCtl->lv_change = 0;
	return;
}

void process_idle(void) {
	for ( ; ; ) {
		_io_hlt();
	}
}

int process_get_index0(struct PROCESS *process) {
	return process->index0;
}
