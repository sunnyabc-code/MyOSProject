#include "bootpack.h"

#define PORT_KEYDAT 0x0060
#define PORT_KEYSTA 0x0064
#define PORT_KEYCMD 0x0064

#define KEYSTA_SEND_NOTREADY 0x02
#define KEYCMD_SENDTO_MOUSE 0xd4
#define MOUSECMD_ENABLE 0xf4

struct FIFO32 *mousefifo;
int mousedata0;
struct FIFO32 *keyfifo;
int keydata0;

// �ȴ����̿�����׼����
void wait_KBC_sendready(void)
{
    for (;;) {
        if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
            break;
        }
    }
}

void enable_mouse(struct FIFO32 *fifo, int data0, struct MOUSE_DEC *mdec)
{
	/* �������ݐ��FIFO�o�b�t�@���L�� */
	mousefifo = fifo;
	mousedata0 = data0;
	/* �}�E�X�L�� */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	/* ���܂�������ACK(0xfa)�����M����Ă��� */
	mdec->phase = 0; /* �}�E�X��0xfa��҂��Ă���i�K */
	return;
}

// ������ݽ���
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat)
{
   	if (mdec->phase == 0) {
		/* �}�E�X��0xfa��҂��Ă���i�K */
		if (dat == 0xfa) {
			mdec->phase = 1;
		}
		return 0;
	}
	if (mdec->phase == 1) {
		/* �}�E�X��1�o�C�g�ڂ�҂��Ă���i�K */
		if ((dat & 0xc8) == 0x08) {
			/* ������1�o�C�g�ڂ����� */
			mdec->buf[0] = dat;
			mdec->phase = 2;
		}
		return 0;
	}
	if (mdec->phase == 2) {
		/* �}�E�X��2�o�C�g�ڂ�҂��Ă���i�K */
		mdec->buf[1] = dat;
		mdec->phase = 3;
		return 0;
	}
	if (mdec->phase == 3) {
		/* �}�E�X��3�o�C�g�ڂ�҂��Ă���i�K */
		mdec->buf[2] = dat;
		mdec->phase = 1;
		mdec->btn = mdec->buf[0] & 0x07;
		mdec->x = mdec->buf[1];
		mdec->y = mdec->buf[2];
		if ((mdec->buf[0] & 0x10) != 0) {
			mdec->x |= 0xffffff00;
		}
		if ((mdec->buf[0] & 0x20) != 0) {
			mdec->y |= 0xffffff00;
		}
		mdec->y = - mdec->y; /* �}�E�X�ł�y�����̕�������ʂƔ��� */
		return 1;
	}
		
	return -1; /* �����ɗ��邱�Ƃ͂Ȃ��͂� */
}

#define KEYCMD_WRITE_MODE		0x60
#define KBC_MODE				0x47

void init_keyboard(struct FIFO32 *fifo, int data0)
{
	/* �������ݐ��FIFO�o�b�t�@���L�� */
	keyfifo = fifo;
	keydata0 = data0;
	/* �L�[�{�[�h�R���g���[���̏����� */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
	return;
}

