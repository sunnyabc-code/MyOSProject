#include "bootpack.h"

static char keytable[0x80] = {
  0,  0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
  'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0, 'a', 's',
  'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',
  'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static char keytable_shift[0x80] = {
  0,  0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t',
  'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0, 'A', 'S',
  'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|', 'Z', 'X', 'C', 'V',
  'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void task_console_main(struct CONSOLE *cons) {
    char line[128];
    int linepos = 0;
    int i;
    int shift_flag = 0; // 记录Shift键状态

    console_init(cons, cons->sheet);
    console_putstr(cons, "Welcome to MyOS Shell\n> ", COL8_LIGHT_GRAY);

    for (;;) {
        io_cli();
        if (fifo32_status(&cons->fifo) == 0) {
            task_sleep(task_now());
            io_sti();
        } else {
            i = fifo32_get(&cons->fifo);
            io_sti();
            int raw = i - 256;
            if (i >= 256 && i <= 511) {
                // 监测Shift按键按下和抬起
                if (raw == 0x2A || raw == 0x36) { // Shift按下
                    shift_flag = 1;
                    continue;
                }
                if (raw == 0xAA || raw == 0xB6) { // Shift松开
                    shift_flag = 0;
                    continue;
                }

                if (raw < 0x80) {
                    char c;
                    if (shift_flag) {
                        c = keytable_shift[raw];
                    } else {
                        c = keytable[raw];
                    }
                    if (c == '\b') {
                        if (linepos > 0) {
                            linepos--;
                            line[linepos] = 0;
                            console_backspace(cons);
                        }
                    } else if (c == '\n') {
                        console_putchar(cons, '\n', COL8_LIGHT_GRAY);
                        line[linepos] = 0;
                        if (linepos > 0) {
                            command_exec(cons, line);
                        } else {
                            console_putstr(cons, "> ", COL8_LIGHT_GRAY);
                        }
                        linepos = 0;
                    } else if (c >= ' ' && c <= '~') {
                        if (linepos < sizeof(line) - 1) {
                            line[linepos++] = c;
                            line[linepos] = 0;
                            console_putchar(cons, c, COL8_LIGHT_GRAY);
                        }
                    }
                }
            }
        }
    }
}

