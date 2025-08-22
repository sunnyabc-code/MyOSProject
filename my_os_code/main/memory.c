#include "bootpack.h"

#define EFLAGS_AC_BIT		0x00040000
#define CR0_CACHE_DISABLE	0x60000000

unsigned int memtest(unsigned int start, unsigned int end)
{
	char flg486 = 0;
	unsigned int eflg, cr0, i;

	/* 386偐丄486埲崀側偺偐偺妋擣 */
	eflg = io_load_eflags();
	eflg |= EFLAGS_AC_BIT; /* AC-bit = 1 */
	io_store_eflags(eflg);
	eflg = io_load_eflags();
	if ((eflg & EFLAGS_AC_BIT) != 0) { /* 386偱偼AC=1偵偟偰傕帺摦偱0偵栠偭偰偟傑偆 */
		flg486 = 1;
	}
	eflg &= ~EFLAGS_AC_BIT; /* AC-bit = 0 */
	io_store_eflags(eflg);

	if (flg486 != 0) {
		cr0 = load_cr0();
		cr0 |= CR0_CACHE_DISABLE; /* 僉儍僢僔儏嬛巭 */
		store_cr0(cr0);
	}

	i = memtest_sub(start, end);

	if (flg486 != 0) {
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE; /* 僉儍僢僔儏嫋壜 */
		store_cr0(cr0);
	}

	return i;
}

void memman_init(struct MEMMAN *man)
{
    man->frees = 0;      // 空闲块数量清零
    man->maxfrees = 0;   // 历史最大空闲块数量清零
    man->lostsize = 0;   // 释放失败大小清零
    man->losts = 0;      // 释放失败次数清零
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
                // 删除这个空闲块，整体向前移位
                man->frees--;
                for (; i < man->frees; i++) {
                    man->free[i] = man->free[i + 1];
                }
            }
            return addr;
        }
    }
    return 0;  // 分配失败，返回0
}

int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
    int i, j;
    // 找到合适的位置插入空闲块（按addr升序排列）
    for (i = 0; i < man->frees; i++) {
        if (man->free[i].addr > addr) {
            break;
        }
    }

    // 前后合并
    if (i > 0 && man->free[i - 1].addr + man->free[i - 1].size == addr) {
        // 和前面的空闲块合并
        man->free[i - 1].size += size;
        // 继续尝试和后面的空闲块合并
        if (i < man->frees && addr + size == man->free[i].addr) {
            man->free[i - 1].size += man->free[i].size;
            // 删除第 i 个空闲块
            man->frees--;
            for (j = i; j < man->frees; j++) {
                man->free[j] = man->free[j + 1];
            }
        }
        return 0;  // 成功释放
    }

    if (i < man->frees && addr + size == man->free[i].addr) {
        // 和后面的空闲块合并
        man->free[i].addr = addr;
        man->free[i].size += size;
        return 0;
    }

    // 插入新的空闲块
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

    // 空闲块表满了，无法释放
    man->losts++;
    man->lostsize += size;
    return -1;
}

