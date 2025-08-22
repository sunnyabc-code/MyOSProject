#include "bootpack.h"
#include "font.h"
void HariMain(void);

// ������ṹ��
struct MOUSE_DEC mdec;

// �ڴ����ṹ��ָ��
struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
int task_number =0;


/* ---------- ��ʾʮ�������ֵĺ��� ---------- */
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

/* ---------- ��ʾ�ַ����ĺ��� ---------- */
void print_str(unsigned char *vram, int scrnx, int x, int y, const char *s, unsigned char color) {
    putfonts8_asc(vram, scrnx, x, y, color, s);
}

// ������ת�ַ��������������ַ�������
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
    // ��ת�ַ���
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


              // �ڴ�ռ�����
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
				
				// ������Ϣ (�������е�������)
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
// ȫ�ֱ���������̨���ں�����ָ��
struct SHEET *sht_console = NULL;
struct TASK *task_console = NULL;
static struct CONSOLE cons; // ȫ�ֻ�̬��������֤��������

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

    // ��ʼ�� cons
    cons.sheet = sht_console;
    mmemset(cons.buf, ' ', CONSOLE_WIDTH * CONSOLE_HEIGHT);
    mmemset(cons.color_buf, COL8_BLACK, CONSOLE_WIDTH * CONSOLE_HEIGHT);
    cons.cursor_x = 0;
    cons.cursor_y = 0;
    
    int close_btn_x = width_pixel - 21;
	int close_btn_y = 5;
	boxfill8(buf_console, width_pixel, COL8_LIGHT_GRAY, close_btn_x, close_btn_y, close_btn_x + 15, close_btn_y + 15);
	putfonts8_asc(buf_console, width_pixel, close_btn_x + 4, close_btn_y + 2, COL8_WHITE, "��");
	sheet_refresh(sht_console, close_btn_x, close_btn_y, close_btn_x + 16, close_btn_y + 16);

    // �������񣬴��� cons ��ַ
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
        task_sleep(task_console);  // ������ֹͣ
        task_free(task_console);   // �ͷ�����ṹ
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
/* ---------- ��������� ---------- */
void HariMain(void) {
    // --- �������� ---

	struct FIFO32 fifo;
	int fifobuf[128];
    char mcursor[256];

    int mx, my, i;
    struct MOUSE_DEC mdec;
    
    // --- ϵͳ��ʼ�� ---
    init_gdtidt();               // ��ʼ�� GDT �� IDT
    init_pic();                  // ��ʼ�� PIC �жϿ�����
    io_sti();                   // ���� CPU �ж�
	                    
    // --- ��ʼ�� FIFO ������ ---
    fifo32_init(&fifo, 128, fifobuf,0);

    // --- ��ʼ�������豸 ---
    init_keyboard(&fifo, 256);
	enable_mouse(&fifo, 512, &mdec);
	
	io_out8(PIC0_IMR, 0xf8);  
    io_out8(PIC1_IMR, 0xef);
 
    // --- ��ʼ�� PIT ��ʱ�� ---
    init_pit();

    // --- �ڴ�������� ---
    memtotal = memtest(0x00400000, 0xbfffffff);
    memman_init(memman);
    memman_free(memman, 0x00001000, 0x0009e000);
    memman_free(memman, 0x00400000, memtotal - 0x00400000);

    shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
    // --- ��ɫ���ʼ�� ---
    init_palette();
    	
	struct SHEET *sht_win_b[3];
	struct TASK *task_a, *task_b[3];
    task_a = task_init(memman);
	fifo.task = task_a;
	task_run(task_a, 1, 2);


    // --- ͼ������ʼ�� ---
    struct SHEET *sht_back = sheet_alloc(shtctl);
    struct SHEET *sht_mouse = sheet_alloc(shtctl);
    struct SHEET *sht_win1 = sheet_alloc(shtctl);

    unsigned char *buf_back = (unsigned char *) memman_alloc(memman, binfo->scrnx * binfo->scrny);
    unsigned char *buf_win1 = (unsigned char *) memman_alloc(memman, 300*128);


    sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);
    sheet_setbuf(sht_win1, buf_win1, 300, 128, -1);

    // --- ��ʼ����Ļ�����ͼ�� ---
    sht_mouse = sheet_alloc(shtctl);
	sheet_setbuf(sht_mouse, mcursor, 16, 16, 99);
    init_screen(buf_back, binfo->scrnx, binfo->scrny);
    init_mouse_cursor8(mcursor, 99);

    // --- ����ʼλ�� ---
	mx = (binfo->scrnx - 16) / 2; /* ��ʒ����ɂȂ�悤�ɍ��W�v�Z */
	my = (binfo->scrny - 28 - 16) / 2;

    // --- �������� ---
    make_window8(buf_win1, 300, 128, "Window 1", 1);
    //putfonts8_asc_sht(sht_win1, 24, 28, COL8_BLACK, COL8_LIGHT_GRAY, "Hello from Win1", 15);

    // --- ����ͼ��λ�� ---
    //�ļ�ͼ�� 
    sht_files = sheet_alloc(shtctl);
	unsigned char *buf_files = (unsigned char *)memman_alloc(memman, binfo->scrnx * binfo->scrny);
	sheet_setbuf(sht_files, buf_files, binfo->scrnx, binfo->scrny - binfo->scrny / 8, -1);
	// ��ʼ���ļ��㱳��Ϊ͸�������汳��ɫ
	boxfill8(buf_files, binfo->scrnx,COL8_LIGHT_PINK, 0, 0, binfo->scrnx - 1, binfo->scrny - binfo->scrny / 8);

    int win1_x = binfo->scrnx - sht_win1->bxsize;  // �����Ҷ���
    int win1_y = binfo->scrny - sht_win1->bysize;  // �����¶���
    sheet_slide(sht_win1, win1_x, win1_y);
    sheet_slide(sht_files, 0, 0);
    sheet_slide(sht_mouse, mx, my);

    // --- ����ͼ�����˳�� ---
    sheet_updown(sht_back, 0);
    sheet_updown(sht_files, 1);
    sheet_updown(sht_win1, 2);
    sheet_updown(sht_mouse, 10);
    
    // --- ��ť---
	int bar_height = 24;
    int bar_y = binfo->scrny - bar_height;

    // ��ť����
    int btn_x0 = 10;
    int btn_y0 = bar_y - 20 ;
    int btn_x1 = btn_x0 + 80;
    int btn_y1 = btn_y0 + 20;

    // ��ť��ɫ�ͱ߿�
    boxfill8(buf_back, binfo->scrnx, COL8_LIGHT_GRAY, btn_x0, btn_y0, btn_x1, btn_y1);
    boxfill8(buf_back, binfo->scrnx, COL8_BLACK, btn_x0, btn_y0, btn_x1, btn_y0);
    boxfill8(buf_back, binfo->scrnx, COL8_BLACK, btn_x0, btn_y0, btn_x0, btn_y1);
    boxfill8(buf_back, binfo->scrnx, COL8_WHITE, btn_x0, btn_y1, btn_x1, btn_y1);
    boxfill8(buf_back, binfo->scrnx, COL8_WHITE, btn_x1, btn_y0, btn_x1, btn_y1);

    putfonts8_asc(buf_back, binfo->scrnx, btn_x0 + 10, btn_y0 + 4, COL8_WHITE, "Terminal");
    sheet_refresh(sht_back, btn_x0, btn_y0, btn_x1, btn_y1);

    // --- ��ʼ����ʱ�� ---
    struct TIMER *timer = timer_alloc();
    timer_init(timer, &fifo, 1);      // ��ʱ���¼�����Ϊ 1
    timer_settime(timer, 100);        // Լ 1 ��󴥷�
    
	//task_b
	unsigned char *buf_win_b;  // ֻ����һ������
	fs_init(memman);
	
	// ���䴰��ͼ��
	sht_win_b[0] = sheet_alloc(shtctl);
	
	// ���仺�������Ĵ�һ�㣬���� 320 x 128
	buf_win_b = (unsigned char *) memman_alloc(memman, 320 * 128);
	sheet_setbuf(sht_win_b[0], buf_win_b, 320, 128, -1);
	
	// �������ڣ������Ϊ System Info
	make_window8(buf_win_b, 320, 128, "System Info", 1);
	
	// ��������
	task_b[0] = task_alloc();
	task_b[0]->tss.esp = memman_alloc(memman, 64 * 1024) + 64 * 1024 - 8;
	task_b[0]->tss.eip = (int) &task_b_main;  // ����ԭ task_b_main
	task_b[0]->tss.es = 1 * 8;
	task_b[0]->tss.cs = 2 * 8;
	task_b[0]->tss.ss = 1 * 8;
	task_b[0]->tss.ds = 1 * 8;
	task_b[0]->tss.fs = 1 * 8;
	task_b[0]->tss.gs = 1 * 8;
	
	// �����ڵ�ַ��������
	*((int *)(task_b[0]->tss.esp + 4)) = (int) sht_win_b[0];
	
	// ��������
	task_run(task_b[0], 2, 1);
	
	//task_b 
	int icon_x = 120;  // ��ťλ��
	int icon_y = bar_y - 20;
	int icon_w = 32;
	int icon_h = 32;
	
	// ����ɫ + �߿�ģ�°�ť��
	boxfill8(buf_back, binfo->scrnx, COL8_LIGHT_GRAY, icon_x, icon_y, icon_x + icon_w, icon_y + icon_h);
	boxfill8(buf_back, binfo->scrnx, COL8_BLACK, icon_x, icon_y, icon_x + icon_w, icon_y); // �ϱ�
	boxfill8(buf_back, binfo->scrnx, COL8_BLACK, icon_x, icon_y, icon_x, icon_y + icon_h); // ���
	boxfill8(buf_back, binfo->scrnx, COL8_WHITE, icon_x, icon_y + icon_h, icon_x + icon_w, icon_y + icon_h); // �±�
	boxfill8(buf_back, binfo->scrnx, COL8_WHITE, icon_x + icon_w, icon_y, icon_x + icon_w, icon_y + icon_h); // �ұ�
	
	// ���ͼ����ɫ����ɫ��
	int x, y;
	for (y = 2; y < icon_h - 2; y++) {
	    for (x = 2; x < icon_w - 2; x++) {
	        buf_back[(icon_y + y) * binfo->scrnx + (icon_x + x)] = COL8_LIGHT_BLUE;
	    }
	}
	
	// ˢ�°�ť����
	sheet_refresh(sht_back, icon_x, icon_y, icon_x + icon_w, icon_y + icon_h);
	
	//�ļ�ϵͳͼ��
	// ����ɫ + �߿�ģ�°�ť��
	int icon_x_file = 170;  // ��ťλ��
	boxfill8(buf_back, binfo->scrnx, COL8_LIGHT_GRAY, icon_x_file, icon_y, icon_x_file + icon_w, icon_y + icon_h);
	boxfill8(buf_back, binfo->scrnx, COL8_BLACK, icon_x_file, icon_y, icon_x_file + icon_w, icon_y); // �ϱ�
	boxfill8(buf_back, binfo->scrnx, COL8_BLACK, icon_x_file, icon_y, icon_x_file, icon_y + icon_h); // ���
	boxfill8(buf_back, binfo->scrnx, COL8_WHITE, icon_x_file, icon_y + icon_h, icon_x_file + icon_w, icon_y + icon_h); // �±�
	boxfill8(buf_back, binfo->scrnx, COL8_WHITE, icon_x_file + icon_w, icon_y, icon_x_file + icon_w, icon_y + icon_h); // �ұ�
	
		// ���ͼ����ɫ
	for (y = 2; y < icon_h - 2; y++) {
	    for (x = 2; x < icon_w - 2; x++) {
	        buf_back[(icon_y + y) * binfo->scrnx + (icon_x_file + x)] = COL8_LIGHT_GREEN;
	    }
	}
	
	// ˢ�°�ť����
	sheet_refresh(sht_back, icon_x_file, icon_y, icon_x_file + icon_w, icon_y + icon_h);
	 
	//̰����
	int icon_x_snake = 220;  // ��ťλ��
	boxfill8(buf_back, binfo->scrnx, COL8_LIGHT_GRAY, icon_x_snake, icon_y, icon_x_snake + icon_w, icon_y + icon_h);
	boxfill8(buf_back, binfo->scrnx, COL8_BLACK, icon_x_snake, icon_y, icon_x_snake + icon_w, icon_y); // �ϱ�
	boxfill8(buf_back, binfo->scrnx, COL8_BLACK, icon_x_snake, icon_y, icon_x_snake, icon_y + icon_h); // ���
	boxfill8(buf_back, binfo->scrnx, COL8_WHITE, icon_x_snake, icon_y + icon_h, icon_x_snake + icon_w, icon_y + icon_h); // �±�
	boxfill8(buf_back, binfo->scrnx, COL8_WHITE, icon_x_snake + icon_w, icon_y, icon_x_snake + icon_w, icon_y + icon_h); // �ұ�
	
		// ���ͼ����ɫ
	for (y = 2; y < icon_h - 2; y++) {
	    for (x = 2; x < icon_w - 2; x++) {
	        buf_back[(icon_y + y) * binfo->scrnx + (icon_x_snake + x)] = COL8_LIGHT_PINK;
	    }
	}
	
	// ˢ�°�ť����
	sheet_refresh(sht_back, icon_x_snake, icon_y, icon_x_snake + icon_w, icon_y + icon_h);
	
	//������
	int icon_x_cal = 270;  // ��ťλ��
	boxfill8(buf_back, binfo->scrnx, COL8_LIGHT_GRAY, icon_x_cal, icon_y, icon_x_snake + icon_w, icon_y + icon_h);
	boxfill8(buf_back, binfo->scrnx, COL8_BLACK, icon_x_cal, icon_y, icon_x_cal + icon_w, icon_y); // �ϱ�
	boxfill8(buf_back, binfo->scrnx, COL8_BLACK, icon_x_cal, icon_y, icon_x_cal, icon_y + icon_h); // ���
	boxfill8(buf_back, binfo->scrnx, COL8_WHITE, icon_x_cal, icon_y + icon_h, icon_x_cal + icon_w, icon_y + icon_h); // �±�
	boxfill8(buf_back, binfo->scrnx, COL8_WHITE, icon_x_cal + icon_w, icon_y, icon_x_cal + icon_w, icon_y + icon_h); // �ұ�
	
		// ���ͼ����ɫ
	for (y = 2; y < icon_h - 2; y++) {
	    for (x = 2; x < icon_w - 2; x++) {
	        buf_back[(icon_y + y) * binfo->scrnx + (icon_x_cal + x)] = COL8_LIGHT_YELLOW;
	    }
	}
	
	// ˢ�°�ť����
	sheet_refresh(sht_back, icon_x_snake, icon_y, icon_x_cal + icon_w, icon_y + icon_h); 
	 
    // --- �϶�������ر��� ---
    int dragging = 0;
    struct SHEET *drag_sht = NULL;
    int drag_x = 0, drag_y = 0;

    /* ---------- ���¼�ѭ�� ---------- */
    for (;;) {
    io_cli();
    if (fifo32_status(&fifo) == 0) {
        task_sleep(task_a);
        io_sti();
    } else {
        i = fifo32_get(&fifo);
        io_sti();

        if (i == 1) {
            // ��ʱ���¼�
            static int count = 0;
            int sec;
            char s[12];
            count++;
            boxfill8(buf_win1, 160, COL8_LIGHT_GRAY, 24, 48, 24 + 8 * 20, 48 + 16);
            putfonts8_asc_sht(sht_win1, 24, 48, COL8_BLACK, COL8_LIGHT_GRAY, "Timer Tick: ", 12);
            int len = int_to_str(count, s);
            putfonts8_asc_sht(sht_win1, 24 + 8 * 12, 48, COL8_BLACK, COL8_LIGHT_GRAY, s, len);
            sheet_refresh(sht_win1, 24, 48, 24 + 8 * (12 + len), 48 + 16);
            // ÿ100��tick��1�루����timer_settime(timer, 50) = 20ms��
			if (count % 2 == 0) {
			    sec++;
			    int h = (sec / 3600) % 24;
			    int m = (sec / 60) % 60;
			    int s2 = sec % 60;
			
			    // ���������Ԥ���㹻�ռ䣨���ַ�����һ�㣬�����Ӱ��
			    boxfill8(buf_win1, 160, COL8_LIGHT_GRAY, 24, 70, 24 + 8 * 12, 70 + 16);
			
			    // ת��Сʱ
			    char s[4];
			    int len = int_to_str(h, s);
			    if (len == 1) { putfonts8_asc_sht(sht_win1, 24, 70, COL8_BLACK, COL8_LIGHT_GRAY, "0", 1); }
			    putfonts8_asc_sht(sht_win1, 24 + 8 * (2 - len), 70, COL8_BLACK, COL8_LIGHT_GRAY, s, len);
			
			    // �ָ��� :
			    putfonts8_asc_sht(sht_win1, 24 + 8 * 2, 70, COL8_BLACK, COL8_LIGHT_GRAY, ":", 1);
			
			    // ת������
			    len = int_to_str(m, s);
			    if (len == 1) { putfonts8_asc_sht(sht_win1, 24 + 8 * 3, 70, COL8_BLACK, COL8_LIGHT_GRAY, "0", 1); }
			    putfonts8_asc_sht(sht_win1, 24 + 8 * (3 + (2 - len)), 70, COL8_BLACK, COL8_LIGHT_GRAY, s, len);
			
			    // �ָ��� :
			    putfonts8_asc_sht(sht_win1, 24 + 8 * 5, 70, COL8_BLACK, COL8_LIGHT_GRAY, ":", 1);
			
			    // ת����
			    len = int_to_str(s2, s);
			    if (len == 1) { putfonts8_asc_sht(sht_win1, 24 + 8 * 6, 70, COL8_BLACK, COL8_LIGHT_GRAY, "0", 1); }
			    putfonts8_asc_sht(sht_win1, 24 + 8 * (6 + (2 - len)), 70, COL8_BLACK, COL8_LIGHT_GRAY, s, len);
			
			    // ˢ��ʱ����ʾ����
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
		    // �������Ϸ�Ѿ�����
		    if (sht_snake != NULL) {
		        if (key == KEY_LEFT || key == KEY_RIGHT || key == KEY_UP || key == KEY_DOWN) {
		            snake_input(key);  // ��������Ϸ������
		        }
		    }
        	
            // �������봦��Ԥ����
            if (task_console != NULL) {
		        fifo32_put(&cons.fifo, i);
		        task_run(task_console, -1, 0); // ���ѿ���̨����
		    }
            
        } else if (512 <= i && i <= 767) {
            // ����¼�
            if (mouse_decode(&mdec, i - 512) != 0) {
                mx += mdec.x;
                my += mdec.y;
                if (mx < 0) mx = 0;
                if (my < 0) my = 0;
                if (mx > binfo->scrnx - 1) mx = binfo->scrnx - 1;
                if (my > binfo->scrny - 1) my = binfo->scrny - 1;
                sheet_slide(sht_mouse, mx, my);

                if ((mdec.btn & 0x01) != 0) {
                    // ����������
                    
                    // ����Ƿ����ˡ�Terminal����ť
                    if (mx >= btn_x0 && mx <= btn_x1 && my >= btn_y0 && my <= btn_y1) {
                        open_console_shell(shtctl, memman);
                    }
                    //����Ƿ����ļ� 
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
					        break; // ��ֹ��������϶�
					    }
					}
					
					//���b������
					if (mx >= icon_x && mx < icon_x + 32 &&
		                my >= icon_y && my < icon_y + 32) {
		                // �����ͼ�� �� ��������
		                sheet_updown(sht_win_b[0], 3);  // �ᵽ�ļ�ͼ��֮��
		            } 
		            
		            //�ļ�����ϵͳ���
					if (mx >= icon_x_file && mx <= icon_x_file + icon_w &&
				        my >= icon_y && my <= icon_y + icon_h) {
				        open_file_manager(); // ���ļ�����������
				    } 

                    //̰���ߵ��
					if (mx >= icon_x_snake && mx <= icon_x_snake + icon_w &&
				        my >= icon_y && my <= icon_y + icon_h) {
				        snake_init(); // ���ļ�����������
				    }
					
					//��������� 
					if (mx >= icon_x_cal && mx <= icon_x_cal + icon_w &&
				        my >= icon_y && my <= icon_y + icon_h) {
				        open_calc_window(); // ���ļ�����������
				    }
				    // ������� 
                    calc_on_click(mx, my);
				    
                    if (dragging == 0) {
                        struct SHEET *shts[] = {sht_win1, sht_win_b[0], sht_win_b[1], sht_win_b[2],sht_console,sht_snake,sht_file_manager,sht_calc};
                        int si;
                        for (si = 0; si < 8; si++) {
                            struct SHEET *sht = shts[si];
                            int x = mx - sht->vx0;
                            int y = my - sht->vy0;
                            if (0 <= x && x < sht->bxsize && 0 <= y && y < sht->bysize) {
                                // �ж��Ƿ���ڴ��ڹرհ�ťλ��
                                if (x >= sht->bxsize - 21 && y < 21) {
                                    if (sht == sht_console) {
								        close_console_shell(memman);
						 
								    } else {
								        sheet_updown(sht, -1);
								    }
                                } else if (y < 21) {
                                    // ��ʼ�϶�����
                                    dragging = 1;
                                    drag_sht = sht;
                                    drag_x = x;
                                    drag_y = y;
                                    sheet_updown(sht, shtctl->top - 1);  // �ᵽ����
                                    sheet_updown(sht_mouse, shtctl->top);
                                }
                                break;
                            }
                        }
                    } else {
                        // �����϶�����
                        sheet_slide(drag_sht, mx - drag_x, my - drag_y);
                    }

                } else {
                    // �������ͷ�
                    dragging = 0;
                }
            }
        }
    }
}

}

