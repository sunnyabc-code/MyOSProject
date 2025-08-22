#include "bootpack.h"
#include "font.h"
void HariMain(void);

// 鼠标解码结构体
struct MOUSE_DEC mdec;

// 内存管理结构体指针
struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
int task_number =0;


/* ---------- 显示十进制数字的函数 ---------- */
void print_dec(unsigned char *vram, int scrnx, int x, int y, unsigned int val, unsigned char color) {
    if (val == 0) {
        putfonts8_asc(vram, scrnx, x, y, color, "0");
        return;
    }
    char buf[10];
    int i = 0;
    while (val > 0) {
        buf[i++] = '0' + (val % 10);
        val /= 10;
    }
    int j; 
    for (j = i - 1; j >= 0; j--) {
        char s[2] = {buf[j], 0};
        putfonts8_asc(vram, scrnx, x, y, color, s);
        x += 8;
    }
}

/* ---------- 显示字符串的函数 ---------- */
void print_str(unsigned char *vram, int scrnx, int x, int y, const char *s, unsigned char color) {
    putfonts8_asc(vram, scrnx, x, y, color, s);
}

// 简单数字转字符串函数，返回字符串长度
int int_to_str(int val, char *buf) {
    if (val == 0) {
        buf[0] = '0';
        buf[1] = 0;
        return 1;
    }
    int i = 0;
    int v = val;
    while (v > 0) {
        buf[i++] = '0' + (v % 10);
        v /= 10;
    }
    // 反转字符串
    int j;
    for (j = 0; j < i / 2; j++) {
        char t = buf[j];
        buf[j] = buf[i - 1 - j];
        buf[i - 1 - j] = t;
    }
    buf[i] = 0;
    return i;
}
 unsigned int memtotal;

void task_b_main(struct SHEET *sht_win_b)
{
    struct FIFO32 fifo_b;
    struct TIMER *timer_1s;
    int i, fifobuf_b[128], count = 0, count0 = 0, uptime = 0;

    fifo32_init(&fifo_b, 128, fifobuf_b, 0);
    timer_1s = timer_alloc();
    timer_init(timer_1s, &fifo_b, 100);
    timer_settime(timer_1s, 100);

    for (;;) {
        count++;
        io_cli();
        if (fifo32_status(&fifo_b) == 0) {
            io_sti();
        } else {
            i = fifo32_get(&fifo_b);
            io_sti();
            if (i == 100) {
                char s[64], num[16];

                int val = count - count0;
                uptime++;

                // Loops/sec
                mstrcpy(s, "Loops/sec: ");
                int_to_str(val, num);
                mstrcpy(s + mstrlen(s), num);

                putfonts8_asc_sht(sht_win_b, 24, 28, COL8_BLACK, COL8_LIGHT_GRAY, s, mstrlen(s));


              // 内存占用情况
				struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
				unsigned int total = memtotal / 1024;              // KB
				unsigned int free  = memman_total(memman) / 1024;  // KB
				unsigned int used  = total - free;
				
				// Total Mem
				mstrcpy(s, "Total Mem: ");
				int_to_str(total, num);
				mstrcpy(s + mstrlen(s), num);
				mstrcpy(s + mstrlen(s), " KB");
				putfonts8_asc_sht(sht_win_b, 24, 44, COL8_BLACK, COL8_LIGHT_GRAY, s, mstrlen(s));
				
				// Used Mem
				mstrcpy(s, "Used Mem:  ");
				int_to_str(used, num);
				mstrcpy(s + mstrlen(s), num);
				mstrcpy(s + mstrlen(s), " KB");
				putfonts8_asc_sht(sht_win_b, 24, 60, COL8_BLACK, COL8_LIGHT_GRAY, s, mstrlen(s));
				
				// Free Mem
				mstrcpy(s, "Free Mem:  ");
				int_to_str(free, num);
				mstrcpy(s + mstrlen(s), num);
				mstrcpy(s + mstrlen(s), " KB");
				putfonts8_asc_sht(sht_win_b, 24, 76, COL8_BLACK, COL8_LIGHT_GRAY, s, mstrlen(s));
				
				// 任务信息 (正在运行的任务数)
				int total_tasks = 0;
				int lv;
				for (lv = 0; lv < MAX_TASKLEVELS; lv++) {
				    total_tasks += taskctl->level[lv].running;
				}
				
				mstrcpy(s, "Running Tasks: ");
				int_to_str(total_tasks, num);
				mstrcpy(s + mstrlen(s), num);
				
				putfonts8_asc_sht(sht_win_b, 24, 108, COL8_BLACK, COL8_LIGHT_GRAY, s, mstrlen(s));
								
            }
        }
    }
}

//cmd
// 全局变量，控制台窗口和任务指针
struct SHEET *sht_console = NULL;
struct TASK *task_console = NULL;
static struct CONSOLE cons; // 全局或静态变量，保证生命周期

struct SHTCTL *shtctl;

void open_console_shell(struct SHTCTL *shtctl, struct MEMMAN *memman) {
    if (sht_console != NULL) {
        sheet_updown(sht_console, shtctl->top);
        return;
    }
    int width_pixel = CONSOLE_WIDTH * 8;
	int height_pixel = CONSOLE_HEIGHT * 16;
	
	unsigned char *buf_console = (unsigned char *) memman_alloc(memman, width_pixel * height_pixel);
	sht_console = sheet_alloc(shtctl);
	sheet_setbuf(sht_console, buf_console, width_pixel, height_pixel, -1);
	make_window8(buf_console, width_pixel, height_pixel, "Console", 1);
	sheet_slide(sht_console, 100, 100);
	sheet_updown(sht_console, shtctl->top);

    // 初始化 cons
    cons.sheet = sht_console;
    mmemset(cons.buf, ' ', CONSOLE_WIDTH * CONSOLE_HEIGHT);
    mmemset(cons.color_buf, COL8_BLACK, CONSOLE_WIDTH * CONSOLE_HEIGHT);
    cons.cursor_x = 0;
    cons.cursor_y = 0;
    
    int close_btn_x = width_pixel - 21;
	int close_btn_y = 5;
	boxfill8(buf_console, width_pixel, COL8_LIGHT_GRAY, close_btn_x, close_btn_y, close_btn_x + 15, close_btn_y + 15);
	putfonts8_asc(buf_console, width_pixel, close_btn_x + 4, close_btn_y + 2, COL8_WHITE, "×");
	sheet_refresh(sht_console, close_btn_x, close_btn_y, close_btn_x + 16, close_btn_y + 16);

    // 启动任务，传入 cons 地址
    task_console = task_alloc();
    task_console->tss.esp = memman_alloc(memman, 64 * 1024) + 64 * 1024 - 8;
    task_console->tss.eip = (int) &task_console_main;
    task_console->tss.es = 1 * 8;
    task_console->tss.cs = 2 * 8;
    task_console->tss.ss = 1 * 8;
    task_console->tss.ds = 1 * 8;
    task_console->tss.fs = 1 * 8;
    task_console->tss.gs = 1 * 8;

    *((int *)(task_console->tss.esp + 4)) = (int) &cons;
    fifo32_init(&cons.fifo, 128, cons.fifobuf, task_console);
    task_run(task_console, 2, 2);
}

void close_console_shell(struct MEMMAN *memman) {
    if (task_console != NULL) {
        task_sleep(task_console);  // 让任务停止
        task_free(task_console);   // 释放任务结构
        task_console = NULL;
    }
    if (sht_console != NULL) {
        memman_free(memman, sht_console->buf, sht_console->bxsize * sht_console->bysize);
        sheet_free(sht_console);
        sht_console = NULL;
    }
}

struct SHEET *sht_files; 
struct FileEntry files[MAX_FILES];
int file_count = 0;
extern struct SHEET *sht_snake; 
extern struct SHEET *sht_file_manager;
extern struct SHEET *sht_calc;
/* ---------- 主程序入口 ---------- */
void HariMain(void) {
    // --- 变量声明 ---

	struct FIFO32 fifo;
	int fifobuf[128];
    char mcursor[256];

    int mx, my, i;
    struct MOUSE_DEC mdec;
    
    // --- 系统初始化 ---
    init_gdtidt();               // 初始化 GDT 和 IDT
    init_pic();                  // 初始化 PIC 中断控制器
    io_sti();                   // 开启 CPU 中断
	                    
    // --- 初始化 FIFO 缓冲区 ---
    fifo32_init(&fifo, 128, fifobuf,0);

    // --- 初始化输入设备 ---
    init_keyboard(&fifo, 256);
	enable_mouse(&fifo, 512, &mdec);
	
	io_out8(PIC0_IMR, 0xf8);  
    io_out8(PIC1_IMR, 0xef);
 
    // --- 初始化 PIT 定时器 ---
    init_pit();

    // --- 内存检测与管理 ---
    memtotal = memtest(0x00400000, 0xbfffffff);
    memman_init(memman);
    memman_free(memman, 0x00001000, 0x0009e000);
    memman_free(memman, 0x00400000, memtotal - 0x00400000);

    shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
    // --- 调色板初始化 ---
    init_palette();
    	
	struct SHEET *sht_win_b[3];
	struct TASK *task_a, *task_b[3];
    task_a = task_init(memman);
	fifo.task = task_a;
	task_run(task_a, 1, 2);


    // --- 图层管理初始化 ---
    struct SHEET *sht_back = sheet_alloc(shtctl);
    struct SHEET *sht_mouse = sheet_alloc(shtctl);
    struct SHEET *sht_win1 = sheet_alloc(shtctl);

    unsigned char *buf_back = (unsigned char *) memman_alloc(memman, binfo->scrnx * binfo->scrny);
    unsigned char *buf_win1 = (unsigned char *) memman_alloc(memman, 300*128);


    sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);
    sheet_setbuf(sht_win1, buf_win1, 300, 128, -1);

    // --- 初始化屏幕与鼠标图形 ---
    sht_mouse = sheet_alloc(shtctl);
	sheet_setbuf(sht_mouse, mcursor, 16, 16, 99);
    init_screen(buf_back, binfo->scrnx, binfo->scrny);
    init_mouse_cursor8(mcursor, 99);

    // --- 鼠标初始位置 ---
	mx = (binfo->scrnx - 16) / 2; /* 夋柺拞墰偵側傞傛偆偵嵗昗寁嶼 */
	my = (binfo->scrny - 28 - 16) / 2;

    // --- 创建窗口 ---
    make_window8(buf_win1, 300, 128, "Window 1", 1);
    //putfonts8_asc_sht(sht_win1, 24, 28, COL8_BLACK, COL8_LIGHT_GRAY, "Hello from Win1", 15);

    // --- 设置图层位置 ---
    //文件图层 
    sht_files = sheet_alloc(shtctl);
	unsigned char *buf_files = (unsigned char *)memman_alloc(memman, binfo->scrnx * binfo->scrny);
	sheet_setbuf(sht_files, buf_files, binfo->scrnx, binfo->scrny - binfo->scrny / 8, -1);
	// 初始化文件层背景为透明或桌面背景色
	boxfill8(buf_files, binfo->scrnx,COL8_LIGHT_PINK, 0, 0, binfo->scrnx - 1, binfo->scrny - binfo->scrny / 8);

    int win1_x = binfo->scrnx - sht_win1->bxsize;  // 窗口右对齐
    int win1_y = binfo->scrny - sht_win1->bysize;  // 窗口下对齐
    sheet_slide(sht_win1, win1_x, win1_y);
    sheet_slide(sht_files, 0, 0);
    sheet_slide(sht_mouse, mx, my);

    // --- 设置图层叠加顺序 ---
    sheet_updown(sht_back, 0);
    sheet_updown(sht_files, 1);
    sheet_updown(sht_win1, 2);
    sheet_updown(sht_mouse, 10);
    
    // --- 按钮---
	int bar_height = 24;
    int bar_y = binfo->scrny - bar_height;

    // 按钮参数
    int btn_x0 = 10;
    int btn_y0 = bar_y - 20 ;
    int btn_x1 = btn_x0 + 80;
    int btn_y1 = btn_y0 + 20;

    // 按钮底色和边框
    boxfill8(buf_back, binfo->scrnx, COL8_LIGHT_GRAY, btn_x0, btn_y0, btn_x1, btn_y1);
    boxfill8(buf_back, binfo->scrnx, COL8_BLACK, btn_x0, btn_y0, btn_x1, btn_y0);
    boxfill8(buf_back, binfo->scrnx, COL8_BLACK, btn_x0, btn_y0, btn_x0, btn_y1);
    boxfill8(buf_back, binfo->scrnx, COL8_WHITE, btn_x0, btn_y1, btn_x1, btn_y1);
    boxfill8(buf_back, binfo->scrnx, COL8_WHITE, btn_x1, btn_y0, btn_x1, btn_y1);

    putfonts8_asc(buf_back, binfo->scrnx, btn_x0 + 10, btn_y0 + 4, COL8_WHITE, "Terminal");
    sheet_refresh(sht_back, btn_x0, btn_y0, btn_x1, btn_y1);

    // --- 初始化定时器 ---
    struct TIMER *timer = timer_alloc();
    timer_init(timer, &fifo, 1);      // 定时器事件数据为 1
    timer_settime(timer, 100);        // 约 1 秒后触发
    
	//task_b
	unsigned char *buf_win_b;  // 只保留一个窗口
	fs_init(memman);
	
	// 分配窗口图层
	sht_win_b[0] = sheet_alloc(shtctl);
	
	// 分配缓冲区，改大一点，比如 320 x 128
	buf_win_b = (unsigned char *) memman_alloc(memman, 320 * 128);
	sheet_setbuf(sht_win_b[0], buf_win_b, 320, 128, -1);
	
	// 创建窗口，标题改为 System Info
	make_window8(buf_win_b, 320, 128, "System Info", 1);
	
	// 创建任务
	task_b[0] = task_alloc();
	task_b[0]->tss.esp = memman_alloc(memman, 64 * 1024) + 64 * 1024 - 8;
	task_b[0]->tss.eip = (int) &task_b_main;  // 保留原 task_b_main
	task_b[0]->tss.es = 1 * 8;
	task_b[0]->tss.cs = 2 * 8;
	task_b[0]->tss.ss = 1 * 8;
	task_b[0]->tss.ds = 1 * 8;
	task_b[0]->tss.fs = 1 * 8;
	task_b[0]->tss.gs = 1 * 8;
	
	// 将窗口地址传入任务
	*((int *)(task_b[0]->tss.esp + 4)) = (int) sht_win_b[0];
	
	// 启动任务
	task_run(task_b[0], 2, 1);
	
	//task_b 
	int icon_x = 120;  // 按钮位置
	int icon_y = bar_y - 20;
	int icon_w = 32;
	int icon_h = 32;
	
	// 背景色 + 边框（模仿按钮）
	boxfill8(buf_back, binfo->scrnx, COL8_LIGHT_GRAY, icon_x, icon_y, icon_x + icon_w, icon_y + icon_h);
	boxfill8(buf_back, binfo->scrnx, COL8_BLACK, icon_x, icon_y, icon_x + icon_w, icon_y); // 上边
	boxfill8(buf_back, binfo->scrnx, COL8_BLACK, icon_x, icon_y, icon_x, icon_y + icon_h); // 左边
	boxfill8(buf_back, binfo->scrnx, COL8_WHITE, icon_x, icon_y + icon_h, icon_x + icon_w, icon_y + icon_h); // 下边
	boxfill8(buf_back, binfo->scrnx, COL8_WHITE, icon_x + icon_w, icon_y, icon_x + icon_w, icon_y + icon_h); // 右边
	
	// 填充图标颜色（蓝色）
	int x, y;
	for (y = 2; y < icon_h - 2; y++) {
	    for (x = 2; x < icon_w - 2; x++) {
	        buf_back[(icon_y + y) * binfo->scrnx + (icon_x + x)] = COL8_LIGHT_BLUE;
	    }
	}
	
	// 刷新按钮区域
	sheet_refresh(sht_back, icon_x, icon_y, icon_x + icon_w, icon_y + icon_h);
	
	//文件系统图标
	// 背景色 + 边框（模仿按钮）
	int icon_x_file = 170;  // 按钮位置
	boxfill8(buf_back, binfo->scrnx, COL8_LIGHT_GRAY, icon_x_file, icon_y, icon_x_file + icon_w, icon_y + icon_h);
	boxfill8(buf_back, binfo->scrnx, COL8_BLACK, icon_x_file, icon_y, icon_x_file + icon_w, icon_y); // 上边
	boxfill8(buf_back, binfo->scrnx, COL8_BLACK, icon_x_file, icon_y, icon_x_file, icon_y + icon_h); // 左边
	boxfill8(buf_back, binfo->scrnx, COL8_WHITE, icon_x_file, icon_y + icon_h, icon_x_file + icon_w, icon_y + icon_h); // 下边
	boxfill8(buf_back, binfo->scrnx, COL8_WHITE, icon_x_file + icon_w, icon_y, icon_x_file + icon_w, icon_y + icon_h); // 右边
	
		// 填充图标颜色
	for (y = 2; y < icon_h - 2; y++) {
	    for (x = 2; x < icon_w - 2; x++) {
	        buf_back[(icon_y + y) * binfo->scrnx + (icon_x_file + x)] = COL8_LIGHT_GREEN;
	    }
	}
	
	// 刷新按钮区域
	sheet_refresh(sht_back, icon_x_file, icon_y, icon_x_file + icon_w, icon_y + icon_h);
	 
	//贪吃蛇
	int icon_x_snake = 220;  // 按钮位置
	boxfill8(buf_back, binfo->scrnx, COL8_LIGHT_GRAY, icon_x_snake, icon_y, icon_x_snake + icon_w, icon_y + icon_h);
	boxfill8(buf_back, binfo->scrnx, COL8_BLACK, icon_x_snake, icon_y, icon_x_snake + icon_w, icon_y); // 上边
	boxfill8(buf_back, binfo->scrnx, COL8_BLACK, icon_x_snake, icon_y, icon_x_snake, icon_y + icon_h); // 左边
	boxfill8(buf_back, binfo->scrnx, COL8_WHITE, icon_x_snake, icon_y + icon_h, icon_x_snake + icon_w, icon_y + icon_h); // 下边
	boxfill8(buf_back, binfo->scrnx, COL8_WHITE, icon_x_snake + icon_w, icon_y, icon_x_snake + icon_w, icon_y + icon_h); // 右边
	
		// 填充图标颜色
	for (y = 2; y < icon_h - 2; y++) {
	    for (x = 2; x < icon_w - 2; x++) {
	        buf_back[(icon_y + y) * binfo->scrnx + (icon_x_snake + x)] = COL8_LIGHT_PINK;
	    }
	}
	
	// 刷新按钮区域
	sheet_refresh(sht_back, icon_x_snake, icon_y, icon_x_snake + icon_w, icon_y + icon_h);
	
	//计算器
	int icon_x_cal = 270;  // 按钮位置
	boxfill8(buf_back, binfo->scrnx, COL8_LIGHT_GRAY, icon_x_cal, icon_y, icon_x_snake + icon_w, icon_y + icon_h);
	boxfill8(buf_back, binfo->scrnx, COL8_BLACK, icon_x_cal, icon_y, icon_x_cal + icon_w, icon_y); // 上边
	boxfill8(buf_back, binfo->scrnx, COL8_BLACK, icon_x_cal, icon_y, icon_x_cal, icon_y + icon_h); // 左边
	boxfill8(buf_back, binfo->scrnx, COL8_WHITE, icon_x_cal, icon_y + icon_h, icon_x_cal + icon_w, icon_y + icon_h); // 下边
	boxfill8(buf_back, binfo->scrnx, COL8_WHITE, icon_x_cal + icon_w, icon_y, icon_x_cal + icon_w, icon_y + icon_h); // 右边
	
		// 填充图标颜色
	for (y = 2; y < icon_h - 2; y++) {
	    for (x = 2; x < icon_w - 2; x++) {
	        buf_back[(icon_y + y) * binfo->scrnx + (icon_x_cal + x)] = COL8_LIGHT_YELLOW;
	    }
	}
	
	// 刷新按钮区域
	sheet_refresh(sht_back, icon_x_snake, icon_y, icon_x_cal + icon_w, icon_y + icon_h); 
	 
    // --- 拖动窗口相关变量 ---
    int dragging = 0;
    struct SHEET *drag_sht = NULL;
    int drag_x = 0, drag_y = 0;

    /* ---------- 主事件循环 ---------- */
    for (;;) {
    io_cli();
    if (fifo32_status(&fifo) == 0) {
        task_sleep(task_a);
        io_sti();
    } else {
        i = fifo32_get(&fifo);
        io_sti();

        if (i == 1) {
            // 定时器事件
            static int count = 0;
            int sec;
            char s[12];
            count++;
            boxfill8(buf_win1, 160, COL8_LIGHT_GRAY, 24, 48, 24 + 8 * 20, 48 + 16);
            putfonts8_asc_sht(sht_win1, 24, 48, COL8_BLACK, COL8_LIGHT_GRAY, "Timer Tick: ", 12);
            int len = int_to_str(count, s);
            putfonts8_asc_sht(sht_win1, 24 + 8 * 12, 48, COL8_BLACK, COL8_LIGHT_GRAY, s, len);
            sheet_refresh(sht_win1, 24, 48, 24 + 8 * (12 + len), 48 + 16);
            // 每100个tick算1秒（假设timer_settime(timer, 50) = 20ms）
			if (count % 2 == 0) {
			    sec++;
			    int h = (sec / 3600) % 24;
			    int m = (sec / 60) % 60;
			    int s2 = sec % 60;
			
			    // 先清除区域，预留足够空间（比字符串宽一点，避免残影）
			    boxfill8(buf_win1, 160, COL8_LIGHT_GRAY, 24, 70, 24 + 8 * 12, 70 + 16);
			
			    // 转换小时
			    char s[4];
			    int len = int_to_str(h, s);
			    if (len == 1) { putfonts8_asc_sht(sht_win1, 24, 70, COL8_BLACK, COL8_LIGHT_GRAY, "0", 1); }
			    putfonts8_asc_sht(sht_win1, 24 + 8 * (2 - len), 70, COL8_BLACK, COL8_LIGHT_GRAY, s, len);
			
			    // 分隔符 :
			    putfonts8_asc_sht(sht_win1, 24 + 8 * 2, 70, COL8_BLACK, COL8_LIGHT_GRAY, ":", 1);
			
			    // 转换分钟
			    len = int_to_str(m, s);
			    if (len == 1) { putfonts8_asc_sht(sht_win1, 24 + 8 * 3, 70, COL8_BLACK, COL8_LIGHT_GRAY, "0", 1); }
			    putfonts8_asc_sht(sht_win1, 24 + 8 * (3 + (2 - len)), 70, COL8_BLACK, COL8_LIGHT_GRAY, s, len);
			
			    // 分隔符 :
			    putfonts8_asc_sht(sht_win1, 24 + 8 * 5, 70, COL8_BLACK, COL8_LIGHT_GRAY, ":", 1);
			
			    // 转换秒
			    len = int_to_str(s2, s);
			    if (len == 1) { putfonts8_asc_sht(sht_win1, 24 + 8 * 6, 70, COL8_BLACK, COL8_LIGHT_GRAY, "0", 1); }
			    putfonts8_asc_sht(sht_win1, 24 + 8 * (6 + (2 - len)), 70, COL8_BLACK, COL8_LIGHT_GRAY, s, len);
			
			    // 刷新时钟显示区域
			    sheet_refresh(sht_win1, 24, 70, 24 + 8 * 12, 70 + 16);
			}

            timer_settime(timer, 50);
            if (count % 1 == 0) {  
                if (sht_snake != NULL) {
                    snake_step();
                }
            }
            
        } else if (256 <= i && i <= 511) {
        	  int key = i - 256;
		    // 如果蛇游戏已经启动
		    if (sht_snake != NULL) {
		        if (key == KEY_LEFT || key == KEY_RIGHT || key == KEY_UP || key == KEY_DOWN) {
		            snake_input(key);  // 调用蛇游戏的输入
		        }
		    }
        	
            // 键盘输入处理（预留）
            if (task_console != NULL) {
		        fifo32_put(&cons.fifo, i);
		        task_run(task_console, -1, 0); // 唤醒控制台任务
		    }
            
        } else if (512 <= i && i <= 767) {
            // 鼠标事件
            if (mouse_decode(&mdec, i - 512) != 0) {
                mx += mdec.x;
                my += mdec.y;
                if (mx < 0) mx = 0;
                if (my < 0) my = 0;
                if (mx > binfo->scrnx - 1) mx = binfo->scrnx - 1;
                if (my > binfo->scrny - 1) my = binfo->scrny - 1;
                sheet_slide(sht_mouse, mx, my);

                if ((mdec.btn & 0x01) != 0) {
                    // 鼠标左键按下
                    
                    // 检查是否点击了“Terminal”按钮
                    if (mx >= btn_x0 && mx <= btn_x1 && my >= btn_y0 && my <= btn_y1) {
                        open_console_shell(shtctl, memman);
                    }
                    //检查是否点击文件 
                    int fx = mx;
					int fy = my;
					char s[16];
				
					int num;
					for (num = 0; num < file_count; num++) {
					    struct FileEntry *fe = &files[num];
					    if (fx >= fe->x && fx < fe->x + 80 &&
					        fy >= fe->y && fy < fe->y + 60) {
					        putfonts8_asc_sht(sht_win1, 24, 60, COL8_BLACK, COL8_LIGHT_GRAY, fe->name, 15);
					        open_file_window(fe->name);
					        break; // 防止继续检测拖动
					    }
					}
					
					//检查b任务点击
					if (mx >= icon_x && mx < icon_x + 32 &&
		                my >= icon_y && my < icon_y + 32) {
		                // 点击到图标 → 弹出窗口
		                sheet_updown(sht_win_b[0], 3);  // 提到文件图层之上
		            } 
		            
		            //文件管理系统点击
					if (mx >= icon_x_file && mx <= icon_x_file + icon_w &&
				        my >= icon_y && my <= icon_y + icon_h) {
				        open_file_manager(); // 打开文件管理器窗口
				    } 

                    //贪吃蛇点击
					if (mx >= icon_x_snake && mx <= icon_x_snake + icon_w &&
				        my >= icon_y && my <= icon_y + icon_h) {
				        snake_init(); // 打开文件管理器窗口
				    }
					
					//计算器点击 
					if (mx >= icon_x_cal && mx <= icon_x_cal + icon_w &&
				        my >= icon_y && my <= icon_y + icon_h) {
				        open_calc_window(); // 打开文件管理器窗口
				    }
				    // 处理计算 
                    calc_on_click(mx, my);
				    
                    if (dragging == 0) {
                        struct SHEET *shts[] = {sht_win1, sht_win_b[0], sht_win_b[1], sht_win_b[2],sht_console,sht_snake,sht_file_manager,sht_calc};
                        int si;
                        for (si = 0; si < 8; si++) {
                            struct SHEET *sht = shts[si];
                            int x = mx - sht->vx0;
                            int y = my - sht->vy0;
                            if (0 <= x && x < sht->bxsize && 0 <= y && y < sht->bysize) {
                                // 判断是否点在窗口关闭按钮位置
                                if (x >= sht->bxsize - 21 && y < 21) {
                                    if (sht == sht_console) {
								        close_console_shell(memman);
						 
								    } else {
								        sheet_updown(sht, -1);
								    }
                                } else if (y < 21) {
                                    // 开始拖动窗口
                                    dragging = 1;
                                    drag_sht = sht;
                                    drag_x = x;
                                    drag_y = y;
                                    sheet_updown(sht, shtctl->top - 1);  // 提到顶层
                                    sheet_updown(sht_mouse, shtctl->top);
                                }
                                break;
                            }
                        }
                    } else {
                        // 正在拖动窗口
                        sheet_slide(drag_sht, mx - drag_x, my - drag_y);
                    }

                } else {
                    // 鼠标左键释放
                    dragging = 0;
                }
            }
        }
    }
}

}

