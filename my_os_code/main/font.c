// font.c
#include "font.h"

extern unsigned char hankaku[4096];

unsigned char *get_fontdata(char c) {
    return hankaku + ((unsigned char)c) * 16;
}

void putfont8(char *vram, int xsize, int x, int y, char color, unsigned char *font) {
	int i;
    for (i = 0; i < 16; i++) {
        unsigned char d = font[i];
        char *p = vram + (y + i) * xsize + x;
        if (d & 0x80) p[0] = color;
        if (d & 0x40) p[1] = color;
        if (d & 0x20) p[2] = color;
        if (d & 0x10) p[3] = color;
        if (d & 0x08) p[4] = color;
        if (d & 0x04) p[5] = color;
        if (d & 0x02) p[6] = color;
        if (d & 0x01) p[7] = color;
    }
}

void putfonts8_asc(char *vram, int xsize, int x, int y, char color, const char *s) {
    for (; *s; s++) {
        putfont8(vram, xsize, x, y, color, get_fontdata(*s));
        x += 8;
    }
}

