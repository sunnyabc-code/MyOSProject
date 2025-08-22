// window.c
#include "bootpack.h"
extern char font[16];

void make_window8(unsigned char *buf, int xsize, int ysize, char *title, char act) {
    static char closebtn[14][16] = {
        "OOOOOOOOOOOOOO",
        "OQQQQQQQQQQQQO",
        "OQQQ@@QQ@@QQQO",
        "OQQQQ@@@ @QQQO",
        "OQQQQQ@ @QQQQO",
        "OQQQQ@@@ @QQQO",
        "OQQQ@@QQ@@QQQO",
        "OQQQQQQQQQQQQO",
        "OQQQQQQQQQQQQO",
        "OQQQ@@@@@@QQQO",
        "OQQ@@@@@@@QQQO",
        "OQ@@@@O@@@@QQO",
        "OQ@@@@@@@O@@QO",
        "OOOOOOOOOOOOOO"
    };
    int x, y;
    char c, tc, tbc;
    if (act) {
        tc = COL8_WHITE;
        tbc = COL8_LIGHT_BLUE;
    } else {
        tc = COL8_LIGHT_GRAY;
        tbc = COL8_LIGHT_BLUE;
    }
    boxfill8(buf, xsize, COL8_BLACK, 0, 0, xsize - 1, 0);
    boxfill8(buf, xsize, COL8_WHITE, 1, 1, xsize - 2, 1);
    boxfill8(buf, xsize, COL8_BLACK, 0, 0, 0, ysize - 1);
    boxfill8(buf, xsize, COL8_WHITE, 1, 1, 1, ysize - 2);
    boxfill8(buf, xsize, COL8_LIGHT_GRAY, xsize - 2, 1, xsize - 2, ysize - 2);
    boxfill8(buf, xsize, COL8_BLACK, xsize - 1, 0, xsize - 1, ysize - 1);
    boxfill8(buf, xsize, COL8_LIGHT_GRAY, 2, 2, xsize - 3, ysize - 3);
    boxfill8(buf, xsize, tbc, 3, 3, xsize - 4, 20);
    putfonts8_asc(buf, xsize, 24, 4, tc, title);
    for (y = 0; y < 14; y++) {
        for (x = 0; x < 16; x++) {
            c = closebtn[y][x];
            if (c == '@') c = COL8_BLACK;
            else if (c == '$') c = COL8_LIGHT_BLUE;
            else if (c == 'Q') c = COL8_LIGHT_GRAY;
            else c = COL8_WHITE;
            buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
        }
    }
}

void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l) {
    boxfill8(sht->buf, sht->bxsize, b, x, y, x + l * 8 - 1, y + 15);
    putfonts8_asc(sht->buf, sht->bxsize, x, y, c, s);
    sheet_refresh(sht, x, y, x + l * 8, y + 16);
}

