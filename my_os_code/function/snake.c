// === ̰������Ϸ ===
#include "bootpack.h"

extern struct SHTCTL *shtctl;
extern struct MEMMAN *memman;
struct SnakeNode snake[MAP_W * MAP_H];
int snake_len, dir;  // dir: 0=��,1=��,2=��,3=��
unsigned char *buf_snake;
struct SHEET *sht_snake;
int food_x, food_y;

#define OBST_MAX 5
struct SnakeNode obstacles[OBST_MAX];

// ��ʼ����Ϸ
void snake_init() {
    int win_w = MAP_W * CELL, win_h = MAP_H * CELL + 20;
    sht_snake = sheet_alloc(shtctl);
    buf_snake = (unsigned char *)memman_alloc(memman, win_w * win_h);
    sheet_setbuf(sht_snake, buf_snake, win_w, win_h, -1);

    make_window8(buf_snake, win_w, win_h, "Snake", 0);

    // ��ʼ�ߣ��м�λ�ã����� 3��
    snake_len = 3;
    snake[0].x = 5; snake[0].y = 10;
    snake[1].x = 4; snake[1].y = 10;
    snake[2].x = 3; snake[2].y = 10;
    dir = 1; // ����

    // ʳ�����
    food_x = 10; food_y = 5;
    
                 
	// ��ʼ���ϰ���
	int i; 
	for (i = 0; i < OBST_MAX; i++) {
	    obstacles[i].x = (i * 7 + 3) % MAP_W;
	    obstacles[i].y = (i * 5 + 11) % MAP_H;
	}

    sheet_slide(sht_snake, 50, 50);
    sheet_updown(sht_snake, shtctl->top);
    snake_draw();
}

// ����������ͼ
void snake_draw() {
    int i, x, y;
    // �������
    boxfill8(buf_snake, sht_snake->bxsize, COL8_WHITE, 
             0, 20, sht_snake->bxsize, sht_snake->bysize);

    // ��
    for (i = 0; i < snake_len; i++) {
        x = snake[i].x * CELL;
        y = snake[i].y * CELL + 20;
        boxfill8(buf_snake, sht_snake->bxsize, COL8_LIGHT_GREEN,
                 x, y, x + CELL - 1, y + CELL - 1);
    }
    
    //�ϰ� 
	for (i = 0; i < OBST_MAX; i++) {
	    x = obstacles[i].x * CELL;
	    y = obstacles[i].y * CELL + 20;
	    boxfill8(buf_snake, sht_snake->bxsize, COL8_BLACK,
	             x, y, x + CELL - 1, y + CELL - 1);
	}

    // ʳ��
    x = food_x * CELL;
    y = food_y * CELL + 20;
    boxfill8(buf_snake, sht_snake->bxsize,COL8_LIGHT_PINK,
             x, y, x + CELL - 1, y + CELL - 1);



    sheet_refresh(sht_snake, 0, 20, sht_snake->bxsize, sht_snake->bysize);
}

// �ƶ�һ��
void snake_step() {
    int i;
    // ����ͷ
    int nx = snake[0].x;
    int ny = snake[0].y;
    if (dir == 0) ny--;
    if (dir == 1) nx++;
    if (dir == 2) ny++;
    if (dir == 3) nx--;

    // ײǽ�ж�
    if (nx < 0 || nx >= MAP_W || ny < 0 || ny >= MAP_H) {
        putfonts8_asc_sht(sht_snake, 40, 40, COL8_LIGHT_PINK, COL8_WHITE, "GAME OVER", 9);
        return;
    }
	// ײ���Լ��ж�
	for (i = 0; i < snake_len; i++) {
	    if (snake[i].x == nx && snake[i].y == ny) {
	        putfonts8_asc_sht(sht_snake, 40, 60, COL8_LIGHT_PINK, COL8_WHITE, "GAME OVER", 9);
	        return;
	    }
	} 
	// ײ���ϰ���
	for (i = 0; i < OBST_MAX; i++) {
	    if (nx == obstacles[i].x && ny == obstacles[i].y) {
	        putfonts8_asc_sht(sht_snake, 40, 60, COL8_LIGHT_PINK, COL8_WHITE, "GAME OVER", 9);
	        return;
	    }
	}

    // �������
    for (i = snake_len; i > 0; i--) {
        snake[i] = snake[i-1];
    }
    snake[0].x = nx; snake[0].y = ny;

    // ��ʳ��
    if (nx == food_x && ny == food_y) {
        snake_len++;
        food_x = (nx * 3 + ny * 7) % MAP_W; // ��������λ��
        food_y = (nx * 5 + ny * 11) % MAP_H;
    }

    snake_draw();
}

// �����������
void snake_input(int key) {
    if (key == KEY_UP && dir != 2) dir = 0;
    if (key == KEY_RIGHT && dir != 3) dir = 1;
    if (key == KEY_DOWN && dir != 0) dir = 2;
    if (key == KEY_LEFT && dir != 1) dir = 3;
}

