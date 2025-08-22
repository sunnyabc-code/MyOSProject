#include "bootpack.h"
void inthandler20(int *esp) __asm__("_inthandler20");
void inthandler2c(int *esp) __asm__("_inthandler2c");
void inthandler21_main(int *esp) __asm__("_inthandler21_main");

// ������̺�������ݶ˿�
#define PORT_KEYDAT 0x0060

extern struct FIFO32 *mousefifo;
extern struct FIFO32 *keyfifo;

// �����жϴ�������IRQ1��
void inthandler21_main(int *esp)
{
    int data;
    io_out8(PIC0_OCW2, 0x61);  // ֪ͨPIC IRQ-01�жϴ������
    data = io_in8(PORT_KEYDAT);
    fifo32_put(keyfifo, data + 256); // �������ݼ�256ƫ�ƣ����ּ��̺���������
    return;
}

// ����жϴ�������IRQ12��
void inthandler2c(int *esp)
{
    int data;
    io_out8(PIC1_OCW2, 0x64);  // ֪ͨ��PIC IRQ-12�жϴ������
    io_out8(PIC0_OCW2, 0x62);  // ֪ͨ��PIC IRQ-02�жϴ������
    data = io_in8(PORT_KEYDAT);
    fifo32_put(mousefifo, data + 512); // ������ݼ�512ƫ�ƣ��������ͼ�������
    return;
}

// ��ʱ���жϴ�������IRQ0��
void inthandler20(int *esp)
{
	struct TIMER *timer;
	char ts = 0;
	io_out8(PIC0_OCW2, 0x60);	/* IRQ-00��t������PIC�ɒʒm */
	timerctl.count++;
	if (timerctl.next > timerctl.count) {
		return;
	}
	timer = timerctl.t0; /* �Ƃ肠�����擪�̔Ԓn��timer�ɑ�� */
	for (;;) {
		/* timers�̃^�C�}�͑S�ē��쒆�̂��̂Ȃ̂ŁAflags���m�F���Ȃ� */
		if (timer->timeout > timerctl.count) {
			break;
		}
		/* �^�C���A�E�g */
		timer->flags = TIMER_FLAGS_ALLOC;
		if (timer != task_timer) {
			fifo32_put(timer->fifo, timer->data);
		} else {
			ts = 1; /* task_timer���^�C���A�E�g���� */
		}
		timer = timer->next; /* ���̃^�C�}�̔Ԓn��timer�ɑ�� */
	}
	timerctl.t0 = timer;
	timerctl.next = timer->timeout;
	if (ts != 0) {
		task_switch();
	}
	return;
}


