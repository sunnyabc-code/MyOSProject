#include "bootpack.h"
void inthandler20(int *esp) __asm__("_inthandler20");
void inthandler2c(int *esp) __asm__("_inthandler2c");
void inthandler21_main(int *esp) __asm__("_inthandler21_main");

// 定义键盘和鼠标数据端口
#define PORT_KEYDAT 0x0060

extern struct FIFO32 *mousefifo;
extern struct FIFO32 *keyfifo;

// 键盘中断处理函数（IRQ1）
void inthandler21_main(int *esp)
{
    int data;
    io_out8(PIC0_OCW2, 0x61);  // 通知PIC IRQ-01中断处理完成
    data = io_in8(PORT_KEYDAT);
    fifo32_put(keyfifo, data + 256); // 键盘数据加256偏移，区分键盘和其他输入
    return;
}

// 鼠标中断处理函数（IRQ12）
void inthandler2c(int *esp)
{
    int data;
    io_out8(PIC1_OCW2, 0x64);  // 通知从PIC IRQ-12中断处理完成
    io_out8(PIC0_OCW2, 0x62);  // 通知主PIC IRQ-02中断处理完成
    data = io_in8(PORT_KEYDAT);
    fifo32_put(mousefifo, data + 512); // 鼠标数据加512偏移，区分鼠标和键盘输入
    return;
}

// 定时器中断处理函数（IRQ0）
void inthandler20(int *esp)
{
	struct TIMER *timer;
	char ts = 0;
	io_out8(PIC0_OCW2, 0x60);	/* IRQ-00庴晅姰椆傪PIC偵捠抦 */
	timerctl.count++;
	if (timerctl.next > timerctl.count) {
		return;
	}
	timer = timerctl.t0; /* 偲傝偁偊偢愭摢偺斣抧傪timer偵戙擖 */
	for (;;) {
		/* timers偺僞僀儅偼慡偰摦嶌拞偺傕偺側偺偱丄flags傪妋擣偟側偄 */
		if (timer->timeout > timerctl.count) {
			break;
		}
		/* 僞僀儉傾僂僩 */
		timer->flags = TIMER_FLAGS_ALLOC;
		if (timer != task_timer) {
			fifo32_put(timer->fifo, timer->data);
		} else {
			ts = 1; /* task_timer偑僞僀儉傾僂僩偟偨 */
		}
		timer = timer->next; /* 師偺僞僀儅偺斣抧傪timer偵戙擖 */
	}
	timerctl.t0 = timer;
	timerctl.next = timer->timeout;
	if (ts != 0) {
		task_switch();
	}
	return;
}


