#include "head.h"

#define SHEET_USE 1

void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0, int h1) {
	int h, bx, by, vx, vy, bx0, by0, bx1, by1;
	unsigned char *buf, sid, *vram = ctl->vram, *map = ctl->map;
	struct SHEET *sht;
	if (vx0 < 0) { vx0 = 0; }
	if (vy0 < 0) { vy0 = 0; }
	if (vx1 > ctl->xsize) { vx1 = ctl->xsize; }
	if (vy1 > ctl->ysize) { vy1 = ctl->ysize; }
	for (h = h0; h <= h1; ++h) {
		sht = ctl->sheets[h];
		buf = sht->buf;
		sid = sht - ctl->sheets0;
		bx0 = vx0 - sht->vx0;
		by0 = vy0 - sht->vy0;
		bx1 = vx1 - sht->vx0;
		by1 = vy1 - sht->vy0;
		if (bx0 < 0) { bx0 = 0; }
		if (by0 < 0) { by0 =0; }
		if (bx1 > sht->bxsize) { bx1 = sht->bxsize; }
		if (by1 > sht->bysize) { by1 = sht->bysize; }
		
		if ((sht->vx0 & 3) == 0) {
			int i = (bx0 + 3) >> 2;
			int i1 = (bx1 >> 2) - i;
			int sid4 = sid | sid << 8 | sid << 16 | sid << 24;
			int temp2 = (by0 - 1) * sht->bxsize;
			for (by = by0; by < by1; ++by) {
				vy = sht->vy0 + by;
				int temp1 = vy * ctl->xsize;
				temp2 += sht->bxsize;
				for (bx = bx0; bx < bx1 && (bx & 3) != 0; ++bx) {
					vx = sht->vx0 + bx;
					if (map[temp1 + vx] == sid) {
						vram[temp1 + vx] = buf[temp2 + bx];
					}
				}
				vx = sht->vx0 + bx;
				int *p = (int *) &map[temp1 + vx];
				int *q = (int *) &vram[temp1 + vx];
				int *r = (int *) &buf[temp2 + bx];
				for (i = 0; i < i1; ++i) {
					if (p[i] == sid4) {
						q[i] = r[i];
					}
					else {
						int bx2 = bx + (i << 2);
						vx = sht->vx0 + bx2;
						if (map[temp1 + vx] == sid) {
							vram[temp1 + vx] = buf[temp2 + bx2];
						}
						if (map[temp1 + vx + 1] == sid) {
							vram[temp1 + vx + 1] = buf[temp2 + bx2 + 1];
						}
						if (map[temp1 + vx + 2] == sid) {
							vram[temp1 + vx + 2] = buf[temp2 + bx2 + 2];
						}
						if (map[temp1 + vx + 3] == sid) {
							vram[temp1 + vx + 3] = buf[temp2 + bx2 + 3];
						}
					}
				}
				for (bx += (i1 << 2); bx < bx1; ++bx) {
					vx = sht->vx0 + bx;
					if (map[temp1 + vx] == sid) {
						vram[temp1 + vx] = buf[temp2 + bx];
					}
				}
			}
		}
		else {
			int temp2 = (by0 - 1) * sht->bxsize;
			for (by = by0; by < by1; ++by) {
				vy = sht->vy0 + by;
				int temp1 = vy * ctl->xsize;
				temp2 += sht->bxsize;
				for (bx = bx0; bx < bx1; ++bx) {
					vx = sht->vx0 + bx;
					if (map[temp1 + vx] == sid) {
						vram[temp1 + vx] = buf[temp2 + bx];
					}
				}
			}
		}
	}
	return;
}

struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize) {
	struct SHTCTL *ctl = (struct SHTCTL *)memman_alloc_4k(memman, sizeof(struct SHTCTL));
	if (ctl == 0) {
		goto err;
	}
	ctl->map = (unsigned char *) memman_alloc_4k(memman, xsize * ysize);
	if (ctl->map == 0) {
		memman_free_4k(memman, (int) ctl, sizeof(struct SHTCTL));
		goto err;
	}
	ctl->vram = vram;
	ctl->xsize = xsize;
	ctl->ysize = ysize;
	ctl->top = -1;
	ctl->focus_on = 0;
	int i;
	for (i = 0; i < MAX_SHEETS; ++i) {
		ctl->sheets0[i].flags = 0;
	}
err:
	return ctl;
}

struct SHEET *sheet_alloc(struct SHTCTL *ctl) {
	struct SHEET *sht;
	int i;
	for (i = 0; i < MAX_SHEETS; ++i) {
		if (ctl->sheets0[i].flags == 0) {
			sht = &(ctl->sheets0[i]);
			sht->flags = SHEET_USE;
			sht->height = -1;
			sht->ctl = ctl;
			sht->process = 0;
			return sht;
		}
	}
	return 0;
}

void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv) {
	sht->buf = buf;
	sht->bxsize = xsize;
	sht->bysize = ysize;
	sht->col_inv = col_inv;
	return;
}

void sheet_updown(struct SHEET *sht, int height) {
	struct SHTCTL *ctl = sht->ctl;
	int h, old = sht->height;
	if (height > ctl->top + 1) {
		height = ctl->top + 1;
	}
	if (height < -1) {
		height = -1;
	}
	sht->height = height;
	if (old > height) {
		if (height >= 0) {
			for (h = old; h > height; --h) {
				ctl->sheets[h] = ctl->sheets[h - 1];
				ctl->sheets[h]->height = h;
			}
			ctl->sheets[height] = sht;
			sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height + 1);
			sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height + 1, old);
		}
		else {
			if (ctl->top > old) {
				for (h = old; h < ctl->top; ++h) {
					ctl->sheets[h] = ctl->sheets[h + 1];
					ctl->sheets[h]->height = h;
				}
			}
			ctl->top--;
			sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 0);
			sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 0, old - 1);
		}
	}
	else if (old < height) {
		if (old >= 0) {
			for (h = old; h < height; ++h) {
				ctl->sheets[h] = ctl->sheets[h + 1];
				ctl->sheets[h]->height = h;
			}
			ctl->sheets[height] = sht;
		}
		else {
			for (h = ctl->top; h >= height; --h) {
				ctl->sheets[h + 1] = ctl->sheets[h];
				ctl->sheets[h + 1]->height = h + 1;
			}
			ctl->sheets[height] = sht;
			ctl->top++;
		}
		sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height);
		sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height, height);
	}
	if (ctl->top > 1 && height == ctl->top - 1 && ctl->focus_on != sht) {
		if ((ctl->focus_on->flags & SHEET_HAVE_CURSOR) != 0) {
			fifo8_put(&ctl->focus_on->process->fifo8, 2);
		}
		ctl->focus_on = sht;
		if ((ctl->focus_on->flags & SHEET_HAVE_CURSOR) != 0) {
			fifo8_put(&ctl->focus_on->process->fifo8, 0);
		}
	}
	return;
}

void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1) {
	if (sht->height >= 0) {
		sheet_refreshsub(sht->ctl, sht->vx0 + bx0, sht->vy0 + by0, sht->vx0 + bx1, sht->vy0 + by1, sht->height, sht->height);
	}
	return;
}

void sheet_slide(struct SHEET *sht, int vx0, int vy0) {
	struct SHTCTL *ctl = sht->ctl;
	int old_vx0 = sht->vx0;
	int old_vy0 = sht->vy0;
	sht->vx0 = vx0;
	sht->vy0 = vy0;
	if (sht->height >= 0) {
		sheet_refreshmap(ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize, 0);
		sheet_refreshmap(ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize, sht->height);
		sheet_refreshsub(ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize, 0, sht->height - 1);
		sheet_refreshsub(ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize, sht->height, sht->height);
	}
	return;
}

void sheet_free(struct SHEET *sht) {
	if (sht->height >= 0) {
		sheet_updown(sht, -1);
	}
	sht->flags = 0;
	return;
}

void sheet_refreshmap(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0) {
	int h, bx, by, vx, vy, bx0, by0, bx1, by1;
	unsigned char *buf, sid, *map = ctl->map;
	struct SHEET *sht;
	if (vx0 < 0) { vx0 = 0; }
	if (vy0 < 0) { vy0 = 0; }
	if (vx1 > ctl->xsize) { vx1 = ctl->xsize; }
	if (vy1 > ctl->ysize) { vy1 = ctl->ysize; }
	for (h = h0; h <= ctl->top; ++h) {
		sht = ctl->sheets[h];
		sid = sht - ctl->sheets0;
		buf = sht->buf;
		bx0 = vx0 - sht->vx0;
		by0 = vy0 - sht->vy0;
		bx1 = vx1 - sht->vx0;
		by1 = vy1 - sht->vy0;
		if (bx0 < 0) { bx0 = 0; }
		if (by0 < 0) { by0 = 0; }
		if (bx1 > sht->bxsize) { bx1 = sht->bxsize; }
		if (by1 > sht->bysize) { by1 = sht->bysize; }
		int temp1 = (by0 - 1) * sht->bxsize;
		if (sht->col_inv == -1) {
			if ((sht->vx0 & 3) == 0 && (bx0 & 3) == 0 && (bx1 & 3) == 0) {
				bx1 = (bx1 - bx0) >> 2;
				int sid4 = sid | sid << 8 | sid << 16 | sid << 24;
				vx = sht->vx0 + bx0;
				for (by = by0; by < by1; ++by) {
					vy = sht->vy0 + by;
					int *p = (int *) &map[vy * ctl->xsize + vx];
					for (bx = 0; bx < bx1; ++bx) {
						p[bx] = sid4;
					}
				}
			}
			for (by = by0; by < by1; ++by) {
				vy = sht->vy0 + by;
				int temp2 = vy * ctl->xsize;
				for (bx = bx0; bx < bx1; ++bx) {
					vx = sht->vx0 + bx;
					map[temp2 + vx] = sid;
				}
			}
		}
		else {
			for (by = by0; by < by1; ++by) {
				vy = sht->vy0 + by;
				temp1 += sht->bxsize;
				int temp2 = vy * ctl->xsize;
				for (bx = bx0; bx < bx1; ++bx) {
					vx = sht->vx0 + bx;
					if (buf[temp1 + bx] != sht->col_inv) {
						map[temp2 + vx] = sid;
					}
				}
			}
		}
	}
	return;
}
