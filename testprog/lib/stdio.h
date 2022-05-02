#ifndef STDIO_H
#define STDIO_H

#include"stdarg.h"

#include"stdint.h"

#define DIRENT_ATTRIB_USED 0x1
#define DIRENT_ATTRIB_USER 0x2
#define DIRENT_ATTRIB_DIR 0x4
#define DIRENT_ATTRIB_DEVICE 0x8

typedef struct dirent
{

    uint8_t name[19];
    uint8_t attrib;
    uint32_t blockno;
    uint32_t size;
    uint32_t EOF;
} __attribute__((packed)) dirent;

enum kbd_key_codes
{
    /* Printable keys */

    // Lower symbols
    KBD_KEY_BACKTICK        = '`',
    KBD_KEY_1               = '1',
    KBD_KEY_2               = '2',
    KBD_KEY_3               = '3',
    KBD_KEY_4               = '4',
    KBD_KEY_5               = '5',
    KBD_KEY_6               = '6',
    KBD_KEY_7               = '7',
    KBD_KEY_8               = '8',
    KBD_KEY_9               = '9',
    KBD_KEY_0               = '0',
    KBD_KEY_MIN             = '-',
    KBD_KEY_EQU             = '=',
    KBD_KEY_Q               = 'q',
    KBD_KEY_W               = 'w',
    KBD_KEY_E               = 'e',
    KBD_KEY_R               = 'r',
    KBD_KEY_T               = 't',
    KBD_KEY_Y               = 'y',
    KBD_KEY_U               = 'u',
    KBD_KEY_I               = 'i',
    KBD_KEY_O               = 'o',
    KBD_KEY_P               = 'p',
    KBD_KEY_LBRA            = '[',
    KBD_KEY_RBRA            = ']',
    KBD_KEY_SLASH           = '\\',
    KBD_KEY_A               = 'a',
    KBD_KEY_S               = 's',
    KBD_KEY_D               = 'd',
    KBD_KEY_F               = 'f',
    KBD_KEY_G               = 'g',
    KBD_KEY_H               = 'h',
    KBD_KEY_J               = 'j',
    KBD_KEY_K               = 'k',
    KBD_KEY_L               = 'l',
    KBD_KEY_SEMI            = ';',
    KBD_KEY_QUOTE           = '\'',
    KBD_KEY_Z               = 'z',
    KBD_KEY_X               = 'x',
    KBD_KEY_C               = 'c',
    KBD_KEY_V               = 'v',
    KBD_KEY_B               = 'b',
    KBD_KEY_N               = 'n',
    KBD_KEY_M               = 'm',
    KBD_KEY_COMMA           = ',',
    KBD_KEY_DOT             = '.',
    KBD_KEY_BACKSLASH       = '/',
    KBD_KEY_SPACE           = ' ',

    /* Non-printable keys */
    KBD_KEY_ESC             = 256,
    KBD_KEY_BACKSPACE,
    KBD_KEY_DEL,
    KBD_KEY_TAB,
    KBD_KEY_CAPSLOCK,
    KBD_KEY_RET,
    KBD_KEY_LSHIFT,
    KBD_KEY_RSHIFT,
    KBD_KEY_LCTRL,
    KBD_KEY_RCTRL,
    KBD_KEY_LALT,
    KBD_KEY_RALT,
    KBD_KEY_UP,
    KBD_KEY_DOWN,
    KBD_KEY_LEFT,
    KBD_KEY_RIGHT,
    KBD_KEY_F1,
    KBD_KEY_F2,
    KBD_KEY_F3,
    KBD_KEY_F4,
    KBD_KEY_F5,
    KBD_KEY_F6,
    KBD_KEY_F7,
    KBD_KEY_F8,
    KBD_KEY_F9,
    KBD_KEY_F10,
    KBD_KEY_F11,
    KBD_KEY_F12,

    /* Invalid */
    KBD_KEY_INVALID
};

void putchar(char c);

void puts(const char *s);

void printf(const char *fmt, ...);

void fopen(char *path, uint32_t attrib, dirent* p);

int read_kbd_buf();

void get_cur_dir(dirent *d);

void readdir(dirent *dir_to_read, dirent *dir_to_write, int index);

void cursor_left(int i);

void cursor_right(int i);

void cursor_up(int i);

void cursor_down(int i);

void cursor_backspace(int i);

extern void clear_screen();

extern int make_dir(char *name);

extern int change_dir(char *name);

#endif