#ifndef SHELL_H
#define SHELL_H

struct
{
    char s[128];
    int tail;
    int cursor;
}cmdbuf;

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

// Actiuon required only for keydown
void key_stroke_action(int key);

void cmd_init();

void key_char(char c);

void key_left();

void key_right();

void key_delete();

void shebang();

void ls(char *p);

void shell_execute();

void clear();

void cd(char *p);

// Actiuon required only for keydown
void key_stroke_action(int key);

#endif