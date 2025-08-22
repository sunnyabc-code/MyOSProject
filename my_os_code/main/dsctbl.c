// dsctbl.c - GDT / IDT 初始化实现 (Haribote-OS)
#include "bootpack.h"
extern void asm_inthandler21(void);
extern void asm_inthandler2c(void);
extern void asm_inthandler20(void);

// 全局变量：GDT 和 IDT 表空间
struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) 0x00270000;
struct GATE_DESCRIPTOR *idt = (struct GATE_DESCRIPTOR *) 0x0026f800;

void init_gdtidt(void) {
    int i;
    // GDT 初始化 (8192 个)
    for (i = 0; i < 8192; i++) {
        set_segmdesc(gdt + i, 0, 0, 0);
    }
    // 设置数据段和代码段
    set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, 0x4092);
    set_segmdesc(gdt + 2, 0x0007ffff, 0x00280000, 0x409a);
    load_gdtr(0xffff, 0x00270000);

    // IDT 初始化 (256 个)
    for (i = 0; i < 256; i++) {
        set_gatedesc(idt + i, 0, 0, 0);
    }
    load_idtr(0x07ff, 0x0026f800);
    // 设置键盘中断处理函数到 INT 0x21
    set_gatedesc(idt + 0x21, (int)asm_inthandler21, 2 * 8, 0x008e);
	set_gatedesc(idt + 0x2c, (int)asm_inthandler2c, 2 * 8, 0x008e);
	set_gatedesc(idt + 0x20, (int)asm_inthandler20, 2 * 8, 0x008e);
}

void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar) {
    if (limit > 0xfffff) {
        ar |= 0x8000;  // G_bit = 1 (4KB粒度)
        limit /= 0x1000;
    }
    sd->limit_low = limit & 0xffff;
    sd->base_low = base & 0xffff;
    sd->base_mid = (base >> 16) & 0xff;
    sd->access_right = ar & 0xff;
    sd->limit_high = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
    sd->base_high = (base >> 24) & 0xff;
}

void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar) {
    gd->offset_low = offset & 0xffff;
    gd->selector = selector;
    gd->dw_count = (ar >> 8) & 0xff;
    gd->access_right = ar & 0xff;
    gd->offset_high = (offset >> 16) & 0xffff;
}

void init_pic(void)
/* PIC偺弶婜壔 */
{
	io_out8(PIC0_IMR,  0xff  ); /* 慡偰偺妱傝崬傒傪庴偗晅偗側偄 */
	io_out8(PIC1_IMR,  0xff  ); /* 慡偰偺妱傝崬傒傪庴偗晅偗側偄 */

	io_out8(PIC0_ICW1, 0x11  ); /* 僄僢僕僩儕僈儌乕僪 */
	io_out8(PIC0_ICW2, 0x20  ); /* IRQ0-7偼丄INT20-27偱庴偗傞 */
	io_out8(PIC0_ICW3, 1 << 2); /* PIC1偼IRQ2偵偰愙懕 */
	io_out8(PIC0_ICW4, 0x01  ); /* 僲儞僶僢僼傽儌乕僪 */

	io_out8(PIC1_ICW1, 0x11  ); /* 僄僢僕僩儕僈儌乕僪 */
	io_out8(PIC1_ICW2, 0x28  ); /* IRQ8-15偼丄INT28-2f偱庴偗傞 */
	io_out8(PIC1_ICW3, 2     ); /* PIC1偼IRQ2偵偰愙懕 */
	io_out8(PIC1_ICW4, 0x01  ); /* 僲儞僶僢僼傽儌乕僪 */

	io_out8(PIC0_IMR,  0xfb  ); /* 11111011 PIC1埲奜偼慡偰嬛巭 */
	io_out8(PIC1_IMR,  0xff  ); /* 11111111 慡偰偺妱傝崬傒傪庴偗晅偗側偄 */

	return;
}


