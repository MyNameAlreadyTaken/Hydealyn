#include "head.h"

extern struct PROCESSCTL *processCtl;

int sys_sendrec(int function, int src_dest, struct MESSAGE *message, struct PROCESS* p) {
	int ret = 0;
	int caller = process_get_index0(p);
	struct MESSAGE *mla = (MESSAGE *)va2la(caller, message);
	mla->source = caller;
	
	if (function == SEND) {
		ret = msg_send(p, src_dest, message);
		if (ret != 0)
			return ret;
	}
	else if (function == RECEIVE) {
		ret = msg_send(p, src_dest, message);
		if (ret != 0)
			return ret;
	}
	else {
		
	}
	return 0;
}

int ldt_seg_linear(struct PROCESS* p, int idx) {
	struct SEGMENT_DESCRIPTOR *d = (idx == 0) ? &p->seg_descriptor0 : &p->seg_descriptor1;
	return (d->base_high << 24) | (d->base_mid << 16) | d->base_low;
}

void *va2la(int pid, void *va) {
	struct PROCESS *p;
	unsigned int msg_base = ldt_seg_linear(p, INDEX_LDT_RW);
	unsigned int la = seg_base + (unsigned int)va;
	
	if (pid < NR_TASKS + NR_PROCESSS) {
		
	}
	return (void *)la;
}

void reset_msg(struct MESSAGE *message) {
	memset(message, 0, size(struct MESSAGE));
	return;
}

void block(struct PROCESS *p) {
	/*assert(p->p_flags);*/
	schedule();
	return;
}

void unblock(struct PROCESS *p) {
	/*assert(p->p_flags == 0);*/
}

int deadlock(int src, int dest) {
	struct PROCESS *p = &processCtl->processes0[dest];
	
}

int msg_send(struct PROCESS *current, int dest, struct MESSAGE *message) {
	struct PROCESS *sender = current;
	struct PROCESS *p_dest = &processCtl->processes0[dest];

	if (deadlock(process_get_index0(sender), dest)) {
		
	}
	
	if ((p_dest->p_flags & RECEIVING) && (p_dest->p_recvfrom == process_get_index0(sender) || p_dest->p_recvfrom == ANY)) {
		phys_copy(va2la(dest, p_dest->p_msg), va2la(process_get_index0(sender), message), sizeof(struct MESSAGE));
		p_dest->p_msg = 0;
		p_dest->p_flags &= ~RECEIVING;
		p_dest->p_recvfrom = NO_TASK;
		unblock(p_dest);
		
	}
	else {
		sender->p_flags |= SENDING;
		sender->p_sendto = dest;
		sender->p_msg = message;
		
		struct PROCESS *p;
		if (p_dest->q_sending) {
			p = p_dest->q_sending;
			while (p->next_sending) {
				p = p->next_sending;
			}
			p->next_sending = sender;
		}
		else {
			p_dest->q_sending = sender;
		}
		sender->next_sending = 0;

		block(sender);
		
	}
	return 0;
}

int msg_receive(struct PROCESS *current, int src, struct MESSAGE *message) {
	struct PROCESS *p_who_wanna_recv = current;
	struct PROCESS *p_from = 0;
	struct PROCESS *prev = 0;
	int copyok = 0;
	
	if ((p_who_wanna_recv->has_int_msg) && ((src == ANY) || (src == INTERRUPT))) {
		struct MESSAGE msg;
		reset_msg(&msg);
		msg.source = INTERRUPT;
		msg.type = HARD_INT;
		
		phys_copy(va2la(process_get_index0(p_who_wanna_recv), message), &msg, sizeof(struct MESSAGE));
		p_who_wanna_recv->has_int_msg = 0;
		
		return 0;
	}
	
	if (src = ANY) {
		if (p_who_wanna_recv->q_sending) {
			p_from = p_who_wanna_recv->q_sending;
			copyok = 1;
			
		}
	}
	else {
		p_from = &processCtl->processes0[src];

		if ((p_from->p_flags & SENDING) && (p_from->p_sendto == process_get_index0(p_who_wanna_recv))) {
			copyok = 1;
			struct PROCESS *p = p_who_wanna_recv->q_sending;
			
			while (p) {
				
				if (process_get_index0(p) == src) {
					p_from = p;
					break;
				}
				prev = p;
				p = p->next_sending;
				
			}
		}
	}

	if (copyok) {
		if (p_from == p_who_wanna_recv->q_sending) {
			p_who_wanna_recv->q_sending = p_from->next_sending;
			p_from->next_sending = 0;
		}

		phys_copy(va2la(process_get_index0(p_who_wanna_recv), message), va2la(process_get_index0(p_from), p_from->p_msg), sizeof(struct MESSAGE));
		p_from->p_msg = 0;
		p_from->p_sendto = NO_TASK;
		p_from->p_flags &= ~SENDING;
		unblock(p_from);
	}
	else {
		p_who_wanna_recv->p_flags |= RECEIVING;
		p_who_wanna_recv->p_msg = message;
		if (src == ANY) {
			p_who_wanna_recv->p_recvfrom = ANY;
		}
		else {
			p_who_wanna_recv->p_recvfrom = process_get_index0(p_from);
		}
		block(p_who_wanna_recv);
		
	}
	return 0;
}

int send_recv(int function, int src_dest, struct MESSAGE *message) {
	int ret = 0;
	if (function == RECEIVE) {
		memset(message, 0, sizeof(struct MESSAGE));
	}

	switch(function) {
	case BOTH:
		ret = sendrec(SEND, src_dest, message);
		if (ret == 0) {
			ret = sendrec(RECEIVE, src_dest, message);
		}
		break;
	case SEND:
	case RECEIVE:
		ret = sendrec(function, src_dest, message);
		break;
	default:
		
	}
	return ret;
}

void inform_int(int tast_nr) {
	struct PROCESS *p;
	
	if ((p->flags & RECEIVING) && ((p->p_recvfrom == INTERRUPT) || (p->p_recvfrom == ANY))) {
		p->msg->source = INTERRUPT;
		p->msg->type = HARD_INT;
		p->msg = 0;
		p->has_int_msg = 0;
		p->flags &= ~RECEIVING;
		p->p_recvfrom = NO_TASK;
		
		unblock(p);

		
	}
	else {
		p->has_int_msg = 1;
	}
}
