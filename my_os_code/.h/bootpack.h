// bootpack.h

#ifndef BOOTPACK_H
#define BOOTPACK_H

#define COL8_BLACK         0
#define COL8_LIGHT_GRAY    1
#define COL8_WHITE         2
#define COL8_LIGHT_BLUE    3
#define COL8_LIGHT_GREEN   4
#define COL8_LIGHT_YELLOW  5
#define COL8_LIGHT_PINK    6

#define PIC0_ICW1		0x0020
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1
#define PORT_KEYDAT     0x0060
#define MOUSEBUF_SIZE 128
#define MEMMAN_ADDR 0x003c0000  
#define MAX_SHEETS 256
#define NULL 0
#define MAX_TIMER 500
#define TIMER_FLAGS_ALLOC 1   // 定时器已分配
#define TIMER_FLAGS_USING  2   // 定时器正在运行
#define ADR_GDT         0x00270000
#define AR_TSS32        0x0089
#define TASK_GDT0       3
#define MAX_TASKS       1000
#define MAX_TASKLEVELS  10
struct FIFO32 {
	int *buf;
	int p, q, size, free, flags;
	struct TASK *task;
};



// 鼠标解码结构体
struct MOUSE_DEC {
    unsigned char buf[3];   // 存储3字节数据
    unsigned char phase;    // 当前接收阶段
    int x, y;               // 坐标增量
    int btn;                // 按键状态
};

void enable_mouse(struct FIFO32 *fifo, int data0, struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);
void init_mouse_cursor8(char *mouse, char bc);
void putblock8_8(char *vram, int vxsize, int pxsize,
	int pysize, int px0, int py0, char *buf, int bxsize);
void init_keyboard(struct FIFO32 *fifo, int data0);
struct BOOTINFO {
    char cyls, leds, vmode, reserve;
    short scrnx, scrny;
    char *vram;
};
extern struct BOOTINFO *binfo;

extern void load_tr(int tr);
extern void farjmp(int eip, int cs);

extern int io_in8(int port);
void io_out8(int port, int data);
extern int io_in16(int port);
extern int io_in32(int port);
extern void io_cli(void);
extern void io_sti(void);
extern void io_stihlt(void);
extern int io_load_eflags(void);
extern void io_store_eflags(int eflags);
extern void load_gdtr(int limit, int addr);
extern void load_idtr(int limit, int addr);

void inthandler21_main(int *esp);
void inthandler2c(int *esp);
void inthandler20(int *esp);

void set_palette(unsigned char *rgb);
void init_palette(void);
void boxfill8(char *vram, int xsize, unsigned char color,
              int x0, int y0, int x1, int y1);
void init_screen(char *vram, int xsize, int ysize);

void init_gdtidt(void);

struct SEGMENT_DESCRIPTOR {
    short limit_low, base_low;
    char base_mid, access_right;
    char limit_high, base_high;
};

struct GATE_DESCRIPTOR {
    short offset_low, selector;
    char dw_count, access_right;
    short offset_high;
};

void fifo32_init(struct FIFO32 *fifo, int size, int *buf, struct TASK *task);
int fifo32_put(struct FIFO32 *fifo, int data);
int fifo32_get(struct FIFO32 *fifo);
int fifo32_status(struct FIFO32 *fifo);



void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);


void init_pic(void);

//内存管理相关
typedef unsigned int uint;

struct FREEINFO {
    uint addr;   // 空闲内存起始地址
    uint size;   // 空闲内存大小（字节）
};

#define MEMMAN_FREES 4090

struct MEMMAN {
    int frees;                     // 空闲块数目
    int maxfrees;                  // 观察最大空闲数
    int lostsize;                  // 释放失败的总大小
    int losts;                     // 释放失败次数
    struct FREEINFO free[MEMMAN_FREES];
};

unsigned int memtest(unsigned int start, unsigned int end);

void memman_init(struct MEMMAN *man);
unsigned int memman_total(struct MEMMAN *man);
int memman_alloc(struct MEMMAN *man, unsigned int size);
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size);

//分层
struct SHEET {
    unsigned char *buf;
    int bxsize, bysize, vx0, vy0, col_inv, height, flags;
    struct SHTCTL *ctl;
};

struct SHTCTL {
    unsigned char *vram, *map;
    int xsize, ysize, top;
    struct SHEET *sheets[MAX_SHEETS];
    struct SHEET sheets0[MAX_SHEETS];
};

#define SHEET_USE 1

struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize);
struct SHEET *sheet_alloc(struct SHTCTL *ctl);
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv);
void sheet_updown(struct SHEET *sht, int height);
void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1);
void sheet_slide(struct SHEET *sht, int vx0, int vy0);
void sheet_refreshmap(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0);
void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0, int h1);
void sheet_free(struct SHEET *sht);

//窗口相关
void make_window8(unsigned char *buf, int xsize, int ysize, char *title, char act);
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l);

//time相关
struct TIMER {
    struct TIMER *next;
    unsigned int timeout;
    char flags;
    struct FIFO32 *fifo;
    int data;
};

struct TIMERCTL {
    unsigned int count;
    unsigned int next;
    struct TIMER *t0;
    struct TIMER timers0[MAX_TIMER];
};
extern struct TIMERCTL timerctl;

void init_pit(void);
struct TIMER *timer_alloc(void);
void timer_free(struct TIMER *timer);
void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data);
void timer_settime(struct TIMER *timer, unsigned int timeout);

//多任务相关

struct TSS32 {
    int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
    int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
    int es, cs, ss, ds, fs, gs;
    int ldtr, iomap;
};

struct TASK {
    int sel, flags;         // sel = GDT 号码
    int level, priority;    // 优先级
    struct TSS32 tss;
};

struct TASKLEVEL {
    int running;            // 正在运行的任务数
    int now;                // 当前运行到哪个任务
    struct TASK *tasks[ MAX_TASKS ];
};

struct TASKCTL {
    int now_lv;             // 当前运行的层
    int lv_change;          // 下次任务切换时是否改变层
    struct TASKLEVEL level[ MAX_TASKLEVELS ];
    struct TASK tasks0[ MAX_TASKS ];
};

// 声明全局变量和函数
extern struct TASKCTL *taskctl;
extern struct TIMER *task_timer;


struct TASK *task_now(void);
void task_add(struct TASK *task);
void task_remove(struct TASK *task);
void task_switchsub(void);
struct TASK *task_init(struct MEMMAN *memman);
struct TASK *task_alloc(void);
void task_run(struct TASK *task, int level, int priority);
void task_sleep(struct TASK *task);
void task_switch(void);
void task_free(struct TASK *task);

//控制台相关
#define CONSOLE_WIDTH 80
#define CONSOLE_HEIGHT 25

struct CONSOLE {
    struct SHEET *sheet;
    unsigned char buf[CONSOLE_WIDTH * CONSOLE_HEIGHT];        // 字符缓冲区
    unsigned char color_buf[CONSOLE_WIDTH * CONSOLE_HEIGHT];  // 颜色缓冲区
    int cursor_x, cursor_y;
    struct FIFO32 fifo;          
    int fifobuf[128];            
};

void console_init(struct CONSOLE *cons, struct SHEET *sheet);
void console_putchar(struct CONSOLE *cons, char c, unsigned char color);
void console_newline(struct CONSOLE *cons);
void console_putstr(struct CONSOLE *cons, const char *s, unsigned char color);
void console_scroll(struct CONSOLE *cons);
void console_refresh(struct CONSOLE *cons);
void console_backspace(struct CONSOLE *cons);
void console_clear(struct CONSOLE *cons);
int int_to_str(int val, char *buf);
void command_exec(struct CONSOLE *cons, char *line);
void *mmemset(void *dest, int val, unsigned int len);

void task_console_main(struct CONSOLE *cons);

int mstrcmp(const char *a, const char *b) ;
int mstrlen(const char *s) ;
void mstrcpy(char *dst, const char *src);

#define FILE_MAX 32
#define DISK_SIZE (1440 * 1024)   // 1.44MB，模拟软盘大小
#define SECTOR_SIZE 512
#define FILENAME_LEN 11           // 8 + 3 格式 (8位文件名 + 3位扩展名)
#define FILE_CONTENT_MAX 512      // 每个文件最大512字节 (1扇区大小)

struct FILEINFO {
    char name[FILENAME_LEN];      // 文件名，8+3格式，不包含终止符
    unsigned char type;           // 文件类型，0无效，1文件等
    unsigned int size;            // 文件大小（字节）
    unsigned short clustno;       // 文件数据块号，索引disk数组中的扇区
};

// 声明全局变量
extern unsigned char *disk;         // 指向模拟磁盘内存
extern struct FILEINFO fileinfo[FILE_MAX];

// 文件系统函数声明
void fs_init(struct MEMMAN *memman);
void fs_format(void);
void fs_mock_write(const char *name, const char *content);
struct FILEINFO *fs_find(const char *name);
int fs_read(const struct FILEINFO *finfo, char *buf, int maxlen);
int fs_create(const char *name, const char *text);
int fs_delete(const char *name);
int mstrncmp(const char *a, const char *b, int len);
void fs_normalize_filename(const char *src, char *dest);
void open_file_window(char *filename);

//文件系统可视化

#define MAX_FILES 32
struct FileEntry {
    char name[16];    // 文件名
    int x, y;         // 图标坐标
};


void draw_icon8(char *vram, int vxsize, int px0, int py0, char *icon);
void draw_filename(char *vram, int vxsize, int x, int y, char *name, char color);
void create_file_and_draw(struct SHEET *sht_files, int xsize, char *filename);
struct SHEET *open_file_manager();
void refresh_file_manager(struct SHEET *sht);
void delete_file_icon(struct SHEET *sht_files, int xsize, char *filename);


//snake_game
#define MAP_W 40
#define MAP_H 40
#define CELL 10   // 每个格子像素大小

// 方向定义
#define DIR_UP    0
#define DIR_RIGHT 1
#define DIR_DOWN  2
#define DIR_LEFT  3

// 键盘方向键（根据你的 keycode 定义修改）
#define KEY_UP     0x48   
#define KEY_DOWN   0x50
#define KEY_LEFT   0x4B
#define KEY_RIGHT  0x4D

// 蛇节点
struct SnakeNode {
    int x, y;
};

// 全局变量（如果别的文件需要用，就 extern）
extern struct SnakeNode snake[MAP_W * MAP_H];
extern int snake_len, dir;
extern struct SHEET *sht_snake;

// 游戏函数接口
void snake_init();               // 初始化窗口 + 游戏
void snake_draw();               // 绘制地图
void snake_step();               // 蛇前进一步
void snake_input(int key);       // 处理键盘输入

//计算器
void open_calc_window(); 
void draw_calc_window(void);
void calc_on_click(int mx, int my);
void calc_update_display(void);
void mstrcat_char(char *s, char c);
int parse_expr(char *s, int *a, int *b, char *op);

#endif
