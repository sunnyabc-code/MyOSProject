#include "bootpack.h"

struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize) {
    struct SHTCTL *ctl;
    int i;
    ctl = (struct SHTCTL *) memman_alloc(memman, sizeof(struct SHTCTL));
    ctl->map = (unsigned char *) memman_alloc(memman, xsize * ysize);
    ctl->vram = vram;
    ctl->xsize = xsize;
    ctl->ysize = ysize;
    ctl->top = -1; // 无图层
    for (i = 0; i < MAX_SHEETS; i++) {
        ctl->sheets0[i].flags = 0; // 未使用
        ctl->sheets0[i].ctl = ctl;
    }
    return ctl;
}

struct SHEET *sheet_alloc(struct SHTCTL *ctl) {
    int i;
    for (i = 0; i < MAX_SHEETS; i++) {
        if (ctl->sheets0[i].flags == 0) {
            struct SHEET *sht = &ctl->sheets0[i];
            sht->flags = SHEET_USE;
            sht->height = -1; // 隐藏中
            return sht;
        }
    }
    return 0; // 分配失败
}

void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv) {
    sht->buf = buf;
    sht->bxsize = xsize;
    sht->bysize = ysize;
    sht->col_inv = col_inv;
}

void sheet_refreshmap(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0) {
    int h, bx, by, vx, vy;
    unsigned char *buf, sid, *map = ctl->map;
    struct SHEET *sht;
    if (vx0 < 0) vx0 = 0;
    if (vy0 < 0) vy0 = 0;
    if (vx1 > ctl->xsize) vx1 = ctl->xsize;
    if (vy1 > ctl->ysize) vy1 = ctl->ysize;
    for (h = h0; h <= ctl->top; h++) {
        sht = ctl->sheets[h];
        sid = h; // 直接使用高度作为标识
        buf = sht->buf;
        for (by = 0; by < sht->bysize; by++) {
            vy = sht->vy0 + by;
            if (vy < vy0 || vy >= vy1) continue;
            for (bx = 0; bx < sht->bxsize; bx++) {
                vx = sht->vx0 + bx;
                if (vx < vx0 || vx >= vx1) continue;
                if (buf[by * sht->bxsize + bx] != sht->col_inv) {
                    map[vy * ctl->xsize + vx] = sid;
                }
            }
        }
    }
}

void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0, int h1) {
    int h, bx, by, vx, vy;
    unsigned char *buf, c, sid, *vram = ctl->vram, *map = ctl->map;
    struct SHEET *sht;
    if (vx0 < 0) vx0 = 0;
    if (vy0 < 0) vy0 = 0;
    if (vx1 > ctl->xsize) vx1 = ctl->xsize;
    if (vy1 > ctl->ysize) vy1 = ctl->ysize;
    for (h = h0; h <= h1; h++) {
        sht = ctl->sheets[h];
        buf = sht->buf;
        sid = h; // 高度做sid
        for (by = 0; by < sht->bysize; by++) {
            vy = sht->vy0 + by;
            if (vy < vy0 || vy >= vy1) continue;
            for (bx = 0; bx < sht->bxsize; bx++) {
                vx = sht->vx0 + bx;
                if (vx < vx0 || vx >= vx1) continue;
                if (map[vy * ctl->xsize + vx] == sid) {
                    c = buf[by * sht->bxsize + bx];
                    vram[vy * ctl->xsize + vx] = c;
                }
            }
        }
    }
}

void sheet_updown(struct SHEET *sht, int height) {
    struct SHTCTL *ctl = sht->ctl;
    int h, old = sht->height;
    if (height > ctl->top + 1) height = ctl->top + 1;
    if (height < -1) height = -1;
    sht->height = height;

    if (old > height) {
        if (height >= 0) {
            for (h = old; h > height; h--) {
                ctl->sheets[h] = ctl->sheets[h - 1];
                ctl->sheets[h]->height = h;
            }
            ctl->sheets[height] = sht;
        } else {
            if (ctl->top > old) {
                for (h = old; h < ctl->top; h++) {
                    ctl->sheets[h] = ctl->sheets[h + 1];
                    ctl->sheets[h]->height = h;
                }
            }
            ctl->top--;
        }
        sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 0);
        sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 0, old - 1);
    } else if (old < height) {
        if (old >= 0) {
            for (h = old; h < height; h++) {
                ctl->sheets[h] = ctl->sheets[h + 1];
                ctl->sheets[h]->height = h;
            }
            ctl->sheets[height] = sht;
        } else {
            for (h = ctl->top; h >= height; h--) {
                ctl->sheets[h + 1] = ctl->sheets[h];
                ctl->sheets[h + 1]->height = h + 1;
            }
            ctl->sheets[height] = sht;
            ctl->top++;
        }
        sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height);
        sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height, height);
    }
}

void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1) {
    if (sht->height >= 0) {
        sheet_refreshmap(sht->ctl, sht->vx0 + bx0, sht->vy0 + by0, sht->vx0 + bx1, sht->vy0 + by1, sht->height);
        sheet_refreshsub(sht->ctl, sht->vx0 + bx0, sht->vy0 + by0, sht->vx0 + bx1, sht->vy0 + by1, sht->height, sht->height);
    }
}

void sheet_slide(struct SHEET *sht, int vx0, int vy0) {
    int old_vx0 = sht->vx0, old_vy0 = sht->vy0;
    sht->vx0 = vx0;
    sht->vy0 = vy0;
    if (sht->height >= 0) {
        sheet_refreshmap(sht->ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize, 0);
        sheet_refreshmap(sht->ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize, sht->height);
        sheet_refreshsub(sht->ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize, 0, sht->height - 1);
        sheet_refreshsub(sht->ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize, sht->height, sht->height);
    }
}

void sheet_free(struct SHEET *sht) {
    if (sht->height >= 0) {
        sheet_updown(sht, -1);  // 隐藏图层
    }
    sht->flags = 0;  // 标记图层空闲
}

