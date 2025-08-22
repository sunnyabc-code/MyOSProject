// font.h
#ifndef FONT_H
#define FONT_H

extern unsigned char hankaku[4096];

// 绘制单字符
void putfont8(char *vram, int xsize, int x, int y, char color, unsigned char *font);

// 根据字符获得对应字体数据指针
unsigned char *get_fontdata(char c);

// 绘制字符串
void putfonts8_asc(char *vram, int xsize, int x, int y, char color, const char *s);

#endif
