#include "bootpack.h"
#include "font.h"

unsigned char *disk;         // 指向模拟磁盘内存
struct FILEINFO fileinfo[FILE_MAX];

// 转大写，格式化成8+3标准格式，输出11字节字符串，末尾不加0
void fs_normalize_filename(const char *src, char *dest) {
    int i = 0, j = 0;
    // 处理主文件名部分，最多8字符，遇到'.'停止
    while (src[i] && src[i] != '.' && j < 8) {
        char c = src[i++];
        if ('a' <= c && c <= 'z') c -= 0x20;  // 转大写
        dest[j++] = c;
    }
    // 不足8字符补空格
    while (j < 8) dest[j++] = ' ';

    // 如果遇到'.'，跳过
    if (src[i] == '.') i++;

    // 处理扩展名，最多3字符
    int ext = 0;
    while (src[i] && ext < 3) {
        char c = src[i++];
        if ('a' <= c && c <= 'z') c -= 0x20;
        dest[j++] = c;
        ext++;
    }
    // 不足3字符补空格
    while (ext < 3) {
        dest[j++] = ' ';
        ext++;
    }
    // 不需要字符串结束符，文件名是11字节定长
}

// 用于比较两个11字节文件名，类似strncmp但长度固定11
int mstrncmp(const char *a, const char *b, int len) {
    int i;
    for (i = 0; i < len; i++) {
        if (a[i] != b[i]) return (unsigned char)a[i] - (unsigned char)b[i];
    }
    return 0;
}

void fs_init(struct MEMMAN *memman) {
    disk = (unsigned char *) memman_alloc(memman, DISK_SIZE);
    if (disk == NULL) {
        // 内存申请失败，处理错误
        return;
    }
    fs_format();
    fs_mock_write("README.TXT", "This is MyOS.\nBuilt from scratch.\n");
    fs_mock_write("HELLO.TXT", "Hello World!\nWelcome to your OS.\n");
}

void fs_format(void) {
    int i;
    for (i = 0; i < DISK_SIZE; i++) disk[i] = 0;
    for (i = 0; i < FILE_MAX; i++) {
        fileinfo[i].name[0] = 0x00;
        fileinfo[i].type = 0;
        fileinfo[i].size = 0;
        fileinfo[i].clustno = 0;
    }
}

void fs_mock_write(const char *name, const char *content) {
    int i;
    for (i = 0; i < FILE_MAX; i++) {
        if (fileinfo[i].name[0] == 0x00) break;
    }
    if (i == FILE_MAX) return;

    int len = mstrlen(content);
    if (len > SECTOR_SIZE) len = SECTOR_SIZE;

    fs_normalize_filename(name, fileinfo[i].name);  // 使用格式化函数

    fileinfo[i].type = 1;
    fileinfo[i].size = len;
    fileinfo[i].clustno = i;

    int j;
    for (j = 0; j < len; j++) {
        disk[SECTOR_SIZE * i + j] = content[j];
    }
}

struct FILEINFO *fs_find(const char *name) {
    char formatted[12];
    fs_normalize_filename(name, formatted);

    int i;
    for (i = 0; i < FILE_MAX; i++) {
        if (fileinfo[i].name[0] == 0x00) continue;
        if (mstrncmp(fileinfo[i].name, formatted, 11) == 0) {
            return &fileinfo[i];
        }
    }
    return 0;
}

int fs_read(const struct FILEINFO *finfo, char *buf, int maxlen) {
    if (!finfo) return -1;
    int len = finfo->size;
    if (len > maxlen) len = maxlen;

    int base = finfo->clustno * SECTOR_SIZE;
    int i;
    for (i = 0; i < len; i++) {
        buf[i] = disk[base + i];
    }
    return len;
}

int fs_create(const char *name, const char *text) {
    int i;
    for (i = 0; i < FILE_MAX; i++) {
        if (fileinfo[i].name[0] == 0x00) break;
    }
    if (i == FILE_MAX) return -1;

    int len = mstrlen(text);
    if (len > SECTOR_SIZE) len = SECTOR_SIZE;

    fs_normalize_filename(name, fileinfo[i].name);  // 格式化文件名

    fileinfo[i].type = 1;
    fileinfo[i].size = len;
    fileinfo[i].clustno = i;

    int j;
    for (j = 0; j < len; j++) {
        disk[SECTOR_SIZE * i + j] = text[j];
    }

    return 0;
}

int fs_delete(const char *name) {
    char formatted[12];
    fs_normalize_filename(name, formatted);

    int i;
    for (i = 0; i < FILE_MAX; i++) {
        if (fileinfo[i].name[0] == 0x00) continue;
        if (mstrncmp(fileinfo[i].name, formatted, 11) == 0) {
            fileinfo[i].name[0] = 0x00;
            fileinfo[i].type = 0;
            fileinfo[i].size = 0;
            fileinfo[i].clustno = 0;

            int j;
            for (j = 0; j < SECTOR_SIZE; j++) {
                disk[SECTOR_SIZE * i + j] = 0;
            }
            return 0;
        }
    }
    return -1;
}

extern struct SHTCTL *shtctl;
extern struct MEMMAN *memman;
struct SHEET *sht_file_manager;

void open_file_window(char *filename) {
    unsigned char *buf;
    int win_w = 160, win_h = 80;
    
    // 分配图层和缓冲区
    struct SHEET *sht;
    sht= sheet_alloc(shtctl);
    buf = (unsigned char *)memman_alloc(memman, win_w * win_h);
    sheet_setbuf(sht, buf, win_w, win_h, -1);

    // 创建带标题栏的窗口（1 表示有关闭按钮）
    make_window8(buf, win_w, win_h, filename, 1);

    
    // 读取文件内容
    struct FILEINFO *finfo = fs_find(filename);
    if (finfo) {
        char file_buf[512];  // 临时存放文件内容
        int len = fs_read(finfo, file_buf, sizeof(file_buf) - 1);
        file_buf[len] = '\0'; // 确保是字符串结束

        // 按行绘制（假设字体高度 16 像素）
        int y = 28;
        char *p = file_buf;
        while (*p && y < win_h - 16) {
            char *line_end = p;
            int line_len = 0;
            while (*line_end && *line_end != '\n' && line_len < 30) {
                line_end++;
                line_len++;
            }
            char saved_char = *line_end;
            *line_end = '\0';  // 临时截断成一行

            putfonts8_asc_sht(sht, 8, y, COL8_BLACK, COL8_LIGHT_GRAY, p, line_len);

            *line_end = saved_char; // 恢复
            if (*line_end == '\n') {
                p = line_end + 1;
            } else {
                p = line_end;
            }
            y += 16; // 下一行
        }
    } else {
        putfonts8_asc_sht(sht, 8, 28, COL8_BLACK, COL8_LIGHT_GRAY, "File not found!", 15);
    }

    // 设置位置并显示
    sheet_slide(sht, 50, 120);
    sheet_updown(sht, shtctl->top - 1);
}

//文件系统可视化
void refresh_file_manager(struct SHEET *sht) {
    int i, y = 24;  // 距离标题栏24px 开始
    char s[64], num[16];

    // 背景填充（清空旧内容）
    boxfill8(sht->buf, sht->bxsize, COL8_WHITE, 
             0, 24, sht->bxsize, sht->bysize);
    
    for (i = 0; i < FILE_MAX; i++) {
        if (fileinfo[i].name[0] != 0x00) {  // 有效文件
            // 文件名
            mstrcpy(s, fileinfo[i].name);

            // 类型
            if (fileinfo[i].type == 1) {
                mstrcpy(s + mstrlen(s), "  [FILE]");
            } else {
                mstrcpy(s + mstrlen(s), "  [?]");
            }

            // 大小
            mstrcpy(s + mstrlen(s), "   ");
            int_to_str(fileinfo[i].size, num);
            mstrcpy(s + mstrlen(s), num);
            mstrcpy(s + mstrlen(s), " bytes");

            // 绘制一行
            putfonts8_asc_sht(sht, 8, y, COL8_BLACK, COL8_WHITE, s, mstrlen(s));
            y += 16;
        }
    }
    
    sheet_refresh(sht, 0, 24, sht->bxsize, sht->bysize);
}

struct SHEET *open_file_manager() {
    unsigned char *buf_win;

    // 分配窗口图层
    sht_file_manager = sheet_alloc(shtctl);
    buf_win = (unsigned char *) memman_alloc(memman, 300 * 200);
    sheet_setbuf(sht_file_manager, buf_win, 300, 200, -1);

    // 创建窗口
    make_window8(buf_win, 300, 200, "File Manager", 1);

    // 刷新文件列表
    refresh_file_manager(sht_file_manager);

    // 显示窗口
    sheet_slide(sht_file_manager, 100, 100);   // 初始位置
    sheet_updown(sht_file_manager, shtctl->top - 1);

    return sht_file_manager;
}
 

