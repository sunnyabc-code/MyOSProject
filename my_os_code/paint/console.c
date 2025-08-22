#include "bootpack.h"
#include "font.h"

void *mmemset(void *dest, int val, unsigned int len) {
    unsigned char *ptr = (unsigned char *)dest;
    while (len-- > 0) {
        *ptr++ = (unsigned char)val;
    }
    return dest;
}

void console_init(struct CONSOLE *cons, struct SHEET *sheet) {
    cons->sheet = sheet;
    cons->cursor_x = 0;
    cons->cursor_y = 0;
    // ����ַ��������ɫ����
    mmemset(cons->buf, ' ', CONSOLE_WIDTH * CONSOLE_HEIGHT);
    mmemset(cons->color_buf, COL8_BLACK, CONSOLE_WIDTH * CONSOLE_HEIGHT);
    fifo32_init(&cons->fifo, 128, cons->fifobuf, 0);
    // ����ˢ��
    console_refresh(cons);
}

void console_refresh(struct CONSOLE *cons) {
    int x, y;
    //�������ͼ��Ϊ�ڵף���ֹ��������
    boxfill8(cons->sheet->buf, cons->sheet->bxsize,
             COL8_BLACK, 0, 0,
             cons->sheet->bxsize - 1, cons->sheet->bysize - 1);
    for (y = 0; y < CONSOLE_HEIGHT; y++) {
        for (x = 0; x < CONSOLE_WIDTH; x++) {
            int idx = y * CONSOLE_WIDTH + x;
            char c = cons->buf[idx];
            unsigned char color = cons->color_buf[idx];

            // ֻ���ƿɴ�ӡ�ַ�
            if (c >= ' ' && c <= '~') {
                putfonts8_asc_sht(cons->sheet, x * 8, y * 16,
                                  color, COL8_BLACK, &c, 1);
            }
        }
    }
    // ��귴ɫ��ʾ
    int cur_idx = cons->cursor_y * CONSOLE_WIDTH + cons->cursor_x;
    char cur_c = cons->buf[cur_idx];
    putfonts8_asc_sht(cons->sheet, cons->cursor_x * 8, cons->cursor_y * 16,
                      COL8_WHITE, COL8_BLACK, &cur_c, 1);
    sheet_refresh(cons->sheet, 0, 0, cons->sheet->bxsize, cons->sheet->bysize);
}

void console_scroll(struct CONSOLE *cons) {
	int x, y;
	unsigned char *buf = cons->sheet->buf;
	int bxsize = cons->sheet->bxsize;
	int font_height = 16;

	/* 1. �����ַ����壺����һ�� */
	for (y = 1; y < CONSOLE_HEIGHT; y++) {
		for (x = 0; x < CONSOLE_WIDTH; x++) {
			int dst = (y - 1) * CONSOLE_WIDTH + x;
			int src = y * CONSOLE_WIDTH + x;
			cons->buf[dst] = cons->buf[src];
			cons->color_buf[dst] = cons->color_buf[src];
		}
	}

	/* 2. ������һ���ַ� */
	for (x = 0; x < CONSOLE_WIDTH; x++) {
		int idx = (CONSOLE_HEIGHT - 1) * CONSOLE_WIDTH + x;
		cons->buf[idx] = ' ';
		cons->color_buf[idx] = COL8_LIGHT_GRAY;
	}

	/* 3. ����ͼ�����Ϲ���������һ�и����أ� */
	for (y = font_height; y < font_height * CONSOLE_HEIGHT; y++) {
		for (x = 0; x < bxsize; x++) {
			buf[x + (y - font_height) * bxsize] = buf[x + y * bxsize];
		}
	}

	/* 4. ������һ������ */
	for (y = font_height * (CONSOLE_HEIGHT - 1); y < font_height * CONSOLE_HEIGHT; y++) {
		for (x = 0; x < bxsize; x++) {
			buf[x + y * bxsize] = COL8_BLACK;
		}
	}

	/* 5. ˢ�¸����� */
	sheet_refresh(cons->sheet, 0, 0, bxsize, font_height * CONSOLE_HEIGHT);

	/* 6. ���ص����һ����� */
	cons->cursor_y = CONSOLE_HEIGHT - 1;
	cons->cursor_x = 0;
}


void console_newline(struct CONSOLE *cons) {
	cons->cursor_x = 0;
	cons->cursor_y++;
	if (cons->cursor_y >= CONSOLE_HEIGHT) {
		console_scroll(cons);
	}
}



void console_putchar(struct CONSOLE *cons, char c, unsigned char color) {
    if (c == '\n') {
        console_newline(cons);
    } else if (c == '\r') {
        cons->cursor_x = 0;
    } else {
        int idx = cons->cursor_y * CONSOLE_WIDTH + cons->cursor_x;
        cons->buf[idx] = c;
        cons->color_buf[idx] = color;

        // ֻ���Ƶ�ǰ�ַ�
        putfonts8_asc_sht(cons->sheet, cons->cursor_x * 8, cons->cursor_y * 16,
                          color, COL8_BLACK, &c, 1);
        sheet_refresh(cons->sheet, cons->cursor_x * 8, cons->cursor_y * 16,
                      cons->cursor_x * 8 + 8, cons->cursor_y * 16 + 16);

        cons->cursor_x++;
        if (cons->cursor_x >= CONSOLE_WIDTH) {
            console_newline(cons);
        }
    }
}

void console_putstr(struct CONSOLE *cons, const char *s, unsigned char color) {
    while (*s) {
        console_putchar(cons, *s, color);
        s++;
    }
    // ���ﲻ��ȫ��ˢ���ˣ�console_putchar���Ѿֲ�ˢ��
}

void console_backspace(struct CONSOLE *cons) {
    if (cons->cursor_x > 0) {
        cons->cursor_x--;
        int idx = cons->cursor_y * CONSOLE_WIDTH + cons->cursor_x;
        cons->buf[idx] = ' ';
        cons->color_buf[idx] = COL8_LIGHT_GRAY;
        // ֻ������굱ǰλ��
        putfonts8_asc_sht(cons->sheet, cons->cursor_x * 8, cons->cursor_y * 16,
                          COL8_LIGHT_GRAY, COL8_BLACK, " ", 1);
        sheet_refresh(cons->sheet, cons->cursor_x * 8, cons->cursor_y * 16,
                      cons->cursor_x * 8 + 8, cons->cursor_y * 16 + 16);
    }
}

void console_clear(struct CONSOLE *cons) {
    int i;
    for (i = 0; i < CONSOLE_WIDTH * CONSOLE_HEIGHT; i++) {
        cons->buf[i] = ' ';
        cons->color_buf[i] = COL8_LIGHT_GRAY;
    }
    cons->cursor_x = 0;
    cons->cursor_y = 0;
    console_refresh(cons);
}

