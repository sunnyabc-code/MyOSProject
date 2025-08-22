#include "bootpack.h"

#define EFLAGS_AC_BIT		0x00040000
#define CR0_CACHE_DISABLE	0x60000000

unsigned int memtest(unsigned int start, unsigned int end)
{
	char flg486 = 0;
	unsigned int eflg, cr0, i;

	/* 386���A486�ȍ~�Ȃ̂��̊m�F */
	eflg = io_load_eflags();
	eflg |= EFLAGS_AC_BIT; /* AC-bit = 1 */
	io_store_eflags(eflg);
	eflg = io_load_eflags();
	if ((eflg & EFLAGS_AC_BIT) != 0) { /* 386�ł�AC=1�ɂ��Ă�������0�ɖ߂��Ă��܂� */
		flg486 = 1;
	}
	eflg &= ~EFLAGS_AC_BIT; /* AC-bit = 0 */
	io_store_eflags(eflg);

	if (flg486 != 0) {
		cr0 = load_cr0();
		cr0 |= CR0_CACHE_DISABLE; /* �L���b�V���֎~ */
		store_cr0(cr0);
	}

	i = memtest_sub(start, end);

	if (flg486 != 0) {
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE; /* �L���b�V������ */
		store_cr0(cr0);
	}

	return i;
}

void memman_init(struct MEMMAN *man)
{
    man->frees = 0;      // ���п���������
    man->maxfrees = 0;   // ��ʷ�����п���������
    man->lostsize = 0;   // �ͷ�ʧ�ܴ�С����
    man->losts = 0;      // �ͷ�ʧ�ܴ�������
}

unsigned int memman_total(struct MEMMAN *man)
{
    unsigned int total = 0;
    int i; 
    for (i = 0; i < man->frees; i++) {
        total += man->free[i].size;
    }
    return total;
}

int memman_alloc(struct MEMMAN *man, unsigned int size)
{
	int i;
    for (i = 0; i < man->frees; i++) {
        if (man->free[i].size >= size) {
            unsigned int addr = man->free[i].addr;
            man->free[i].addr += size;
            man->free[i].size -= size;
            if (man->free[i].size == 0) {
                // ɾ��������п飬������ǰ��λ
                man->frees--;
                for (; i < man->frees; i++) {
                    man->free[i] = man->free[i + 1];
                }
            }
            return addr;
        }
    }
    return 0;  // ����ʧ�ܣ�����0
}

int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
    int i, j;
    // �ҵ����ʵ�λ�ò�����п飨��addr�������У�
    for (i = 0; i < man->frees; i++) {
        if (man->free[i].addr > addr) {
            break;
        }
    }

    // ǰ��ϲ�
    if (i > 0 && man->free[i - 1].addr + man->free[i - 1].size == addr) {
        // ��ǰ��Ŀ��п�ϲ�
        man->free[i - 1].size += size;
        // �������Ժͺ���Ŀ��п�ϲ�
        if (i < man->frees && addr + size == man->free[i].addr) {
            man->free[i - 1].size += man->free[i].size;
            // ɾ���� i �����п�
            man->frees--;
            for (j = i; j < man->frees; j++) {
                man->free[j] = man->free[j + 1];
            }
        }
        return 0;  // �ɹ��ͷ�
    }

    if (i < man->frees && addr + size == man->free[i].addr) {
        // �ͺ���Ŀ��п�ϲ�
        man->free[i].addr = addr;
        man->free[i].size += size;
        return 0;
    }

    // �����µĿ��п�
    if (man->frees < MEMMAN_FREES) {
        for (j = man->frees; j > i; j--) {
            man->free[j] = man->free[j - 1];
        }
        man->frees++;
        if (man->maxfrees < man->frees) {
            man->maxfrees = man->frees;
        }
        man->free[i].addr = addr;
        man->free[i].size = size;
        return 0;
    }

    // ���п�����ˣ��޷��ͷ�
    man->losts++;
    man->lostsize += size;
    return -1;
}

