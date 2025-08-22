#include "bootpack.h"

struct SHEET *sht_calc;
static char input[64];
static int input_len = 0;
extern struct SHTCTL *shtctl;
extern struct MEMMAN *memman;

// --- 工具函数 ---

void mstrcat_char(char *s, char c) {
    int len = mstrlen(s);
    if (len < 63) {
        s[len] = c;
        s[len + 1] = '\0';
    }
}

// 支持 "a+b" "a-b" "a*b" "a/b"
int parse_expr(char *s, int *a, int *b, char *op) {
    int i = 0, val = 0;
    *a = *b = 0; *op = 0;

    // 读第一个数
    while (s[i] >= '0' && s[i] <= '9') {
        val = val * 10 + (s[i] - '0');
        i++;
    }
    *a = val;

    // 读运算符
    if (s[i] == '+' || s[i] == '-' || s[i] == '*' || s[i] == '/') {
        *op = s[i];
        i++;
    } else return 0;

    // 读第二个数
    val = 0;
    while (s[i] >= '0' && s[i] <= '9') {
        val = val * 10 + (s[i] - '0');
        i++;
    }
    *b = val;

    return 1;
}

// --- 计算器窗口 ---

#define CALC_W  160
#define CALC_H  200
#define BTN_W   30
#define BTN_H   30

void draw_button(struct SHEET *sht, int x, int y, char *label) {
    boxfill8(sht->buf, sht->bxsize, COL8_LIGHT_GRAY, x, y, x+BTN_W-2, y+BTN_H-2);
    putfonts8_asc_sht(sht, x+10, y+10, COL8_BLACK, COL8_LIGHT_GRAY, label, mstrlen(label));
}

void draw_calc_window() {
    make_window8(sht_calc->buf, CALC_W, CALC_H, "Calculator", 1);

    // 显示区域
    boxfill8(sht_calc->buf, sht_calc->bxsize, COL8_WHITE, 10, 24, CALC_W-10, 50);
    putfonts8_asc_sht(sht_calc, 12, 28, COL8_BLACK, COL8_WHITE, input, mstrlen(input));

    // 数字按钮
    int x0 = 10, y0 = 60, num = 1,row,col;
    for (row = 0; row < 3; row++) {
        for (col = 0; col < 3; col++) {
            char s[2] = { '0' + num, 0 };
            draw_button(sht_calc, x0 + col*(BTN_W+5), y0 + row*(BTN_H+5), s);
            num++;
        }
    }
    draw_button(sht_calc, x0 + BTN_W + 5, y0 + 3*(BTN_H+5), "0");

    // 运算符按钮
    draw_button(sht_calc, 110, 60, "+");
    draw_button(sht_calc, 110, 95, "-");
    draw_button(sht_calc, 110, 130, "*");
    draw_button(sht_calc, 110, 165, "/");

    // 等号和清除
    draw_button(sht_calc, 10, 165, "=");
    draw_button(sht_calc, 50, 165, "C");

    sheet_refresh(sht_calc, 0, 0, CALC_W, CALC_H);
}

void open_calc_window() {
    unsigned char *buf = (unsigned char *) memman_alloc(memman, CALC_W * CALC_H);
    sht_calc = sheet_alloc(shtctl);
    sheet_setbuf(sht_calc, buf, CALC_W, CALC_H, -1);
    sheet_slide(sht_calc, 200, 100);
    sheet_updown(sht_calc, 3);

    input[0] = '\0';
    input_len = 0;

    draw_calc_window();
}

// --- 处理点击 ---

void calc_update_display() {
   // 清空显示区（保持和初始化一致）
    boxfill8(sht_calc->buf, sht_calc->bxsize,
             COL8_WHITE, 10, 24, CALC_W-10, 50);

    // 重绘输入内容
    putfonts8_asc_sht(sht_calc, 12, 28,
                      COL8_BLACK, COL8_WHITE,
                      input, mstrlen(input));

    sheet_refresh(sht_calc, 10, 24, CALC_W-10, 50);
}

void calc_on_click(int mx, int my) {
    if (!sht_calc) return;

    // 转换成窗口内部坐标
    int x = mx - sht_calc->vx0;
    int y = my - sht_calc->vy0;

    // 检查数字 1-9
    int x0 = 10, y0 = 60, num = 1,row,col;
    for (row = 0; row < 3; row++) {
        for (col = 0; col < 3; col++) {
            int bx = x0 + col*(BTN_W+5), by = y0 + row*(BTN_H+5);
            if (x >= bx && x <= bx+BTN_W && y >= by && y <= by+BTN_H) {
                mstrcat_char(input, '0' + num);
                calc_update_display();
                return;
            }
            num++;
        }
    }

    // 运算符
    if (x >= 110 && x <= 110+BTN_W) {
        if (y >= 60 && y <= 60+BTN_H) { mstrcat_char(input, '+'); calc_update_display(); return; }
        if (y >= 95 && y <= 95+BTN_H) { mstrcat_char(input, '-'); calc_update_display(); return; }
        if (y >= 130 && y <= 130+BTN_H) { mstrcat_char(input, '*'); calc_update_display(); return; }
        if (y >= 165 && y <= 165+BTN_H) { mstrcat_char(input, '/'); calc_update_display(); return; }
    }

    // =
    if (x >= 10 && x <= 10+BTN_W && y >= 165 && y <= 165+BTN_H) {
        int a, b; char op;
        if (parse_expr(input, &a, &b, &op)) {
            int res = 0;
            if (op == '+') res = a + b;
            else if (op == '-') res = a - b;
            else if (op == '*') res = a * b;
            else if (op == '/' && b != 0) res = a / b;

            char buf[32];
            int_to_str(res, buf);
            mstrcpy(input, buf);
        } else {
            mstrcpy(input, "Err");
        }
        calc_update_display();
        return;
    }

    // C 清空
    if (x >= 50 && x <= 50+BTN_W && y >= 165 && y <= 165+BTN_H) {
    	 // 清空逻辑缓冲区
    input[0] = '\0';
    input_len = 0;
    	boxfill8(sht_calc->buf, sht_calc->bxsize,
         COL8_WHITE, 10, 24, CALC_W-10, 50);
        sheet_refresh(sht_calc, 10, 24, CALC_W-10, 50);
        return;
    }
}

