// graphic.c

#include "bootpack.h"
#include "font.h"


void init_mouse_cursor8(char *mouse, char bc)
/* �}�E�X�J�[�\���������i16x16�j */
{
	static char cursor[16][16] = {
		"**************..",
		"*OOOOOOOOOOO*...",
		"*OOOOOOOOOO*....",
		"*OOOOOOOOO*.....",
		"*OOOOOOOO*......",
		"*OOOOOOO*.......",
		"*OOOOOOO*.......",
		"*OOOOOOOO*......",
		"*OOOO**OOO*.....",
		"*OOO*..*OOO*....",
		"*OO*....*OOO*...",
		"*O*......*OOO*..",
		"**........*OOO*.",
		"*..........*OOO*",
		"............*OO*",
		".............***"
	};
	int x, y;

	for (y = 0; y < 16; y++) {
		for (x = 0; x < 16; x++) {
			if (cursor[y][x] == '*') {
				mouse[y * 16 + x] =COL8_LIGHT_PINK;
			}
			if (cursor[y][x] == 'O') {
				mouse[y * 16 + x] = COL8_WHITE;
			}
			if (cursor[y][x] == '.') {
				mouse[y * 16 + x] = bc;
			}
		}
	}
	return;
}

void putblock8_8(char *vram, int vxsize, int pxsize,
	int pysize, int px0, int py0, char *buf, int bxsize)
{
	int x, y;
	for (y = 0; y < pysize; y++) {
		for (x = 0; x < pxsize; x++) {
			vram[(py0 + y) * vxsize + (px0 + x)] = buf[y * bxsize + x];
		}
	}
	return;
}
static unsigned char table_rgb[7 * 3] = {
    0x00, 0x00, 0x00,   // ��ɫ
    0xc6, 0xc6, 0xc6,   // ǳ��
    0xff, 0xff, 0xff,   // ��ɫ
    0x66, 0xcc, 0xff,   // ǳ��
    0x66, 0xff, 0x66,   // ǳ��
    0xff, 0xff, 0x99,   // ǳ��
    0xff, 0xcc, 0xff    // ǳ��
};

void set_palette(unsigned char *rgb) {
    int i;
    io_out8(0x03c8, 0);  // ��ɫ����ʼ����0
    for (i = 0; i < 7 * 3; i++) {
        io_out8(0x03c9, rgb[i] / 4);  // VGA��ɫ����Ҫ��4
    }
}

void init_palette(void) {
    set_palette(table_rgb);
}

void boxfill8(char *vram, int xsize, unsigned char color,
              int x0, int y0, int x1, int y1) {
    int x, y;
    for (y = y0; y <= y1; y++) {
        for (x = x0; x <= x1; x++) {
            vram[y * xsize + x] = color;
        }
    }
}

void init_screen(char *vram, int xsize, int ysize) {
    boxfill8(vram, xsize, COL8_LIGHT_PINK, 0, 0, xsize - 1, ysize - 1);
    boxfill8(vram, xsize, COL8_LIGHT_GRAY, 0, ysize - ysize / 8, xsize - 1, ysize - 1);
}

//�ļ�ͼ��
// �ļ�ͼ��16x16���أ����װ棬0=͸����1=��ɫ��2=ǳ�ң�
static char file_icon[16 * 16] = {
    0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,
    0,0,0,0,0,0,1,2,2,2,1,0,0,0,0,0,
    0,0,0,0,0,1,2,2,2,2,2,1,0,0,0,0,
    0,0,0,0,1,2,2,2,2,2,2,2,1,0,0,0,
    0,0,0,1,2,2,2,2,2,2,2,2,2,1,0,0,
    0,0,1,2,2,2,2,2,2,2,2,2,2,2,1,0,
    0,1,2,2,2,2,2,2,2,2,2,2,2,2,2,1,
    0,1,2,2,2,2,2,2,2,2,2,2,2,2,2,1,
    0,1,2,2,2,2,2,2,2,2,2,2,2,2,2,1,
    0,0,1,2,2,2,2,2,2,2,2,2,2,2,1,0,
    0,0,0,1,2,2,2,2,2,2,2,2,2,1,0,0,
    0,0,0,0,1,2,2,2,2,2,2,2,1,0,0,0,
    0,0,0,0,0,1,2,2,2,2,2,1,0,0,0,0,
    0,0,0,0,0,0,1,2,2,2,1,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

// ����16x16ͼ�꣬color����Ϊ0=͸��, 1=��ɫ, 2=ǳ��
void draw_icon8(char *vram, int vxsize, int px0, int py0, char *icon) {
    int x, y;
    char c;
    for (y = 0; y < 16; y++) {
        for (x = 0; x < 16; x++) {
            c = icon[y * 16 + x];
            if (c == 1) {
                vram[(py0 + y) * vxsize + (px0 + x)] = COL8_WHITE;
            } else if (c == 2) {
                vram[(py0 + y) * vxsize + (px0 + x)] = COL8_LIGHT_GRAY;
            }
            // 0Ϊ͸����������
        }
    }
}

// ���ļ����ַ�����x,y�����Ͻ�����
void draw_filename(char *vram, int vxsize, int x, int y, char *name, char color) {
    putfonts8_asc(vram, vxsize, x, y, color, name);
}

extern struct FileEntry files[MAX_FILES];
extern int file_count;

void create_file_and_draw(struct SHEET *sht_files, int xsize, char *filename) {
    if (file_count >= MAX_FILES) {
        // �ļ�̫�࣬������
        return;
    }

    // �򵥴���ͼ�����У��������У�ÿ���ļ�ռ 80x60 ��������
    int x = 32 + (file_count % 5) * 80;
    int y = 32 + (file_count / 5) * 60;

    // �����ļ���¼
    struct FileEntry *fe = &files[file_count];
    int i; 
    for (i = 0; i < 15 && filename[i]; i++) {
        fe->name[i] = filename[i];
        fe->name[i+1] = 0;
    }
    fe->x = x;
    fe->y = y;
    	char s[16];


    file_count++;


    // ����ͼ�������
    draw_icon8(sht_files->buf, xsize, x, y, file_icon);
    draw_filename(sht_files->buf, xsize, x, y + 20, fe->name, COL8_BLACK);
     sheet_refresh(sht_files, x, y, x + 80, y + 60);
}

void delete_file_icon(struct SHEET *sht_files, int xsize, char *filename) {
    int i,j;
    for (i = 0; i < file_count; i++) {
        if (strcmp(files[i].name, filename) == 0) {
            int x = files[i].x;
            int y = files[i].y;

            // 1. �����ͼ�����򣨱���ɫ��
            boxfill8(sht_files->buf, xsize, COL8_LIGHT_PINK, 
                     x, y, x + 80, y + 60);

            // 2. ˢ����һ��
            sheet_refresh(sht_files, x, y, x + 80, y + 60);

            // 3. ɾ�������������ļ���������������
            for (j = i; j < file_count - 1; j++) {
                files[j] = files[j + 1];
            }
            file_count--;

            return;
        }
    }
}


