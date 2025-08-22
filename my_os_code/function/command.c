#include "bootpack.h"

extern struct MEMMAN *memman;
extern struct SHEET *sht_files;
 
// 字符串比较，返回0表示相等
int mstrcmp(const char *a, const char *b) {
    while (*a && *b) {
        if (*a != *b) return *a - *b;
        a++;
        b++;
    }
    return *a - *b;
}

// 字符串长度
int mstrlen(const char *s) {
    int len = 0;
    while (s[len]) len++;
    return len;
}

// 字符串拷贝
void mstrcpy(char *dst, const char *src) {
    while (*src) {
        *dst++ = *src++;
    }
    *dst = 0;
}

// mem 命令
void cmd_mem(struct CONSOLE *cons, int argc, char **argv) {
    char buf[64];
    int mem_total_kb = memman_total(memman) / 1024;
    char prefix[] = "Memory Total: ";
    int i = 0;
    while(prefix[i] != 0){
        buf[i] = prefix[i];
        i++;
    }
    i += int_to_str(mem_total_kb, buf + i);
    char suffix[] = " KB\n";
    int j = 0;
    while(suffix[j] != 0){
        buf[i++] = suffix[j++];
    }
    buf[i] = 0;

    console_putstr(cons, buf, COL8_LIGHT_GRAY);
}

// cls 命令
void cmd_cls(struct CONSOLE *cons, int argc, char **argv) {
    console_clear(cons);
}

// ver 命令
void cmd_ver(struct CONSOLE *cons, int argc, char **argv) {
    console_putstr(cons, "MyOS version 0.1 (build 2025-08-07)\n", COL8_LIGHT_GRAY);
}

// help 命令
void cmd_help(struct CONSOLE *cons, int argc, char **argv) {
    console_putstr(cons, "Available commands:\n", COL8_LIGHT_GRAY);
    console_putstr(cons, "  mem  - Show memory usage\n", COL8_LIGHT_GRAY);
    console_putstr(cons, "  cls  - Clear screen\n", COL8_LIGHT_GRAY);
    console_putstr(cons, "  ver  - Show OS version\n", COL8_LIGHT_GRAY);
    console_putstr(cons, "  help - Show this help\n", COL8_LIGHT_GRAY);
    console_putstr(cons, "  echo - Print text to screen\n", COL8_LIGHT_GRAY);
}

// echo 命令
void cmd_echo(struct CONSOLE *cons, int argc, char **argv) {
    int i;
    for (i = 1; i < argc; i++) {
        console_putstr(cons, argv[i], COL8_LIGHT_GRAY);
        console_putchar(cons, (i == argc - 1 ? '\n' : ' '), COL8_LIGHT_GRAY);
    }
}

struct COMMAND {
    char *name;
    void (*func)(struct CONSOLE *cons, int argc, char **argv);
}; 

//文件系统相关 
void cmd_dir(struct CONSOLE *cons) {
    char s[32];
    int i;
    for (i = 0; i < FILE_MAX; i++) {
        if (fileinfo[i].name[0] == 0x00) continue;
        int j;
        for (j = 0; j < 8; j++) s[j] = fileinfo[i].name[j];
        s[8] = '.';
        for (j = 0; j < 3; j++) s[9 + j] = fileinfo[i].name[8 + j];
        s[12] = 0;

        console_putstr(cons, s, 7);
        console_putstr(cons, "  ", 7);

        s[0] = 0;
        int size = fileinfo[i].size;
        int idx = 0;
        do {
            s[idx++] = '0' + (size % 10);
            size /= 10;
        } while (size > 0);
        s[idx] = 0;

        // reverse string
        int l,r;
        for (l = 0, r = idx - 1; l < r; l++, r--) {
            char tmp = s[l]; s[l] = s[r]; s[r] = tmp;
        }

        console_putstr(cons, s, 7);
        console_putstr(cons, "\n", 7);
    }
}

// 文件系统相关命令部分，替换以下内容：

// type 命令
void cmd_type(struct CONSOLE *cons, int argc, char **argv) {
    if (argc < 2) {
        console_putstr(cons, "Usage: type <filename>\n", COL8_LIGHT_GRAY);
        return;
    }

    struct FILEINFO *finfo = fs_find(argv[1]);
    if (finfo == 0) {
        console_putstr(cons, "File not found.\n", COL8_LIGHT_GRAY);
        return;
    }

    char *p = (char *)&disk[finfo->clustno * SECTOR_SIZE];
    int i;
    for (i = 0; i < finfo->size; i++) {
       char ch = p[i];
		if (ch == '\n') {
		    console_putstr(cons, "\n", COL8_LIGHT_GRAY);
		} else if (ch == '\r') {
		    // 可选：忽略或处理回车符
		} else {
		    char s[2] = {ch, 0};
		    console_putstr(cons, s, COL8_LIGHT_GRAY);
		}
    }
    console_putstr(cons, "\n", COL8_LIGHT_GRAY);
}

// create 命令
void cmd_create(struct CONSOLE *cons, int argc, char **argv) {
    if (argc < 3) {
        console_putstr(cons, "Usage: create <filename> <text>\n", COL8_LIGHT_GRAY);
        return;
    }
    if (fs_create(argv[1], argv[2]) == 0) {
        console_putstr(cons, "File created.\n", COL8_LIGHT_GRAY);
        create_file_and_draw(sht_files, binfo->scrnx, argv[1]);
    } else {
        console_putstr(cons, "Create failed.\n", COL8_LIGHT_GRAY);
    }
}

// del 命令
void cmd_del(struct CONSOLE *cons, int argc, char **argv) {
    if (argc < 2) {
        console_putstr(cons, "Usage: del <filename>\n", COL8_LIGHT_GRAY);
        return;
    }
    if (fs_delete(argv[1]) == 0) {
        console_putstr(cons, "File deleted.\n", COL8_LIGHT_GRAY);
        delete_file_icon(sht_files, binfo->scrnx, argv[1]);
    } else {
        console_putstr(cons, "Delete failed.\n", COL8_LIGHT_GRAY);
    }
}

// 命令表，替换commands数组里对应部分：
struct COMMAND commands[] = {
    {"mem",  cmd_mem},
    {"cls",  cmd_cls},
    {"ver",  cmd_ver},
    {"help", cmd_help},
    {"echo", cmd_echo},
    {"dir",  (void *)cmd_dir},
    {"type", cmd_type},
    {"create", cmd_create},
    {"del",  cmd_del},
    {0, 0}
};

// command_exec函数，替换你的同名函数
void command_exec(struct CONSOLE *cons, char *line) {
    char *argv[5];
    int argc = 0;
    char *p = line;
    while (*p && argc < 5) {
        while (*p == ' ') p++;
        if (*p == 0) break;
        argv[argc++] = p;
        while (*p && *p != ' ') p++;
        if (*p) {
            *p = 0;
            p++;
        }
    }
    if (argc == 0) return;

    int i;
    for (i = 0; commands[i].name != 0; i++) {
        if (mstrcmp(argv[0], commands[i].name) == 0) {
            if (commands[i].func != 0) {
                commands[i].func(cons, argc, argv);
            } else {
                console_putstr(cons, "Command not implemented.\n", COL8_WHITE);
            }
            return;
        }
    }
    console_putstr(cons, "Unknown command\n", COL8_WHITE);
}


