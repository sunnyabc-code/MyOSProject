#include "bootpack.h"
#include "font.h"

unsigned char *disk;         // ָ��ģ������ڴ�
struct FILEINFO fileinfo[FILE_MAX];

// ת��д����ʽ����8+3��׼��ʽ�����11�ֽ��ַ�����ĩβ����0
void fs_normalize_filename(const char *src, char *dest) {
    int i = 0, j = 0;
    // �������ļ������֣����8�ַ�������'.'ֹͣ
    while (src[i] && src[i] != '.' && j < 8) {
        char c = src[i++];
        if ('a' <= c && c <= 'z') c -= 0x20;  // ת��д
        dest[j++] = c;
    }
    // ����8�ַ����ո�
    while (j < 8) dest[j++] = ' ';

    // �������'.'������
    if (src[i] == '.') i++;

    // ������չ�������3�ַ�
    int ext = 0;
    while (src[i] && ext < 3) {
        char c = src[i++];
        if ('a' <= c && c <= 'z') c -= 0x20;
        dest[j++] = c;
        ext++;
    }
    // ����3�ַ����ո�
    while (ext < 3) {
        dest[j++] = ' ';
        ext++;
    }
    // ����Ҫ�ַ������������ļ�����11�ֽڶ���
}

// ���ڱȽ�����11�ֽ��ļ���������strncmp�����ȹ̶�11
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
        // �ڴ�����ʧ�ܣ��������
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

    fs_normalize_filename(name, fileinfo[i].name);  // ʹ�ø�ʽ������

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

    fs_normalize_filename(name, fileinfo[i].name);  // ��ʽ���ļ���

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
    
    // ����ͼ��ͻ�����
    struct SHEET *sht;
    sht= sheet_alloc(shtctl);
    buf = (unsigned char *)memman_alloc(memman, win_w * win_h);
    sheet_setbuf(sht, buf, win_w, win_h, -1);

    // �������������Ĵ��ڣ�1 ��ʾ�йرհ�ť��
    make_window8(buf, win_w, win_h, filename, 1);

    
    // ��ȡ�ļ�����
    struct FILEINFO *finfo = fs_find(filename);
    if (finfo) {
        char file_buf[512];  // ��ʱ����ļ�����
        int len = fs_read(finfo, file_buf, sizeof(file_buf) - 1);
        file_buf[len] = '\0'; // ȷ�����ַ�������

        // ���л��ƣ���������߶� 16 ���أ�
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
            *line_end = '\0';  // ��ʱ�ضϳ�һ��

            putfonts8_asc_sht(sht, 8, y, COL8_BLACK, COL8_LIGHT_GRAY, p, line_len);

            *line_end = saved_char; // �ָ�
            if (*line_end == '\n') {
                p = line_end + 1;
            } else {
                p = line_end;
            }
            y += 16; // ��һ��
        }
    } else {
        putfonts8_asc_sht(sht, 8, 28, COL8_BLACK, COL8_LIGHT_GRAY, "File not found!", 15);
    }

    // ����λ�ò���ʾ
    sheet_slide(sht, 50, 120);
    sheet_updown(sht, shtctl->top - 1);
}

//�ļ�ϵͳ���ӻ�
void refresh_file_manager(struct SHEET *sht) {
    int i, y = 24;  // ���������24px ��ʼ
    char s[64], num[16];

    // ������䣨��վ����ݣ�
    boxfill8(sht->buf, sht->bxsize, COL8_WHITE, 
             0, 24, sht->bxsize, sht->bysize);
    
    for (i = 0; i < FILE_MAX; i++) {
        if (fileinfo[i].name[0] != 0x00) {  // ��Ч�ļ�
            // �ļ���
            mstrcpy(s, fileinfo[i].name);

            // ����
            if (fileinfo[i].type == 1) {
                mstrcpy(s + mstrlen(s), "  [FILE]");
            } else {
                mstrcpy(s + mstrlen(s), "  [?]");
            }

            // ��С
            mstrcpy(s + mstrlen(s), "   ");
            int_to_str(fileinfo[i].size, num);
            mstrcpy(s + mstrlen(s), num);
            mstrcpy(s + mstrlen(s), " bytes");

            // ����һ��
            putfonts8_asc_sht(sht, 8, y, COL8_BLACK, COL8_WHITE, s, mstrlen(s));
            y += 16;
        }
    }
    
    sheet_refresh(sht, 0, 24, sht->bxsize, sht->bysize);
}

struct SHEET *open_file_manager() {
    unsigned char *buf_win;

    // ���䴰��ͼ��
    sht_file_manager = sheet_alloc(shtctl);
    buf_win = (unsigned char *) memman_alloc(memman, 300 * 200);
    sheet_setbuf(sht_file_manager, buf_win, 300, 200, -1);

    // ��������
    make_window8(buf_win, 300, 200, "File Manager", 1);

    // ˢ���ļ��б�
    refresh_file_manager(sht_file_manager);

    // ��ʾ����
    sheet_slide(sht_file_manager, 100, 100);   // ��ʼλ��
    sheet_updown(sht_file_manager, shtctl->top - 1);

    return sht_file_manager;
}
 

