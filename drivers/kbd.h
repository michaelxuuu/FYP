#ifndef KBD_H
#define KBD_H

#include"../include/type.h"

#include"low_level.h"

#include"../cpu/comn_hdlr.h"     // Register irq1 handler

#include"../Lib/util.h"

#include"screen.h"

#include"../kernel/kprintf.h"


// ps2 controller ports
#define PS2_CONTROLLER_PORT_DATA   0x60 // also used to interact with PS/2 devices - keyboard (encoder)/ mouse
#define PS2_CONTROLLER_PORT_CMD    0x64
#define PS2_CONTROLLER_PORT_STATUS 0x64


/**
 * Status byte is obtained by reading 0x64, showing status of the PS/2 controller
 * Unused status omitted, see the complete version at https://wiki.osdev.org/%228042%22_PS/2_Controller
 */
// ps2 controller status
#define PS2_CONTROLLER_STATUS_OUTBUF_FULL 0x1 // must be set before reading from 0x60
#define PS2_CONTROLLER_STATUS_INBUF_FULL  0x2 // must be clear before writing to 0x60 and 0x64

uint8_t ps2_controller_get_status(); // Read the controller's status register

void ps2_controller_send_cmd(uint8_t cmd);  // Send command to 0x64

uint8_t ps2_controller_read_data(); // Read data byte from 0x60 (could be a response byte from  the controller or a return code from the encoder)

void ps2_controller_send_data(uint8_t data); // Send data byte to 0x60

/**
 * This sturct maintains the current state of the keyboard
 * like which key's being held down or sth
 */
typedef int bool;
typedef struct kbd_state 
{
    bool numlock;           // numlock enabled
    bool scrolllock;        // scrlllock enabled
    bool capslock;          // capslock enabled
    bool shift;             // shift held down
    bool alt;               // alt held down
    bool ctrl;              // ctrl held dow

    int key;                // Code of the current key being pressed or released (extended scan code header 0xE0 or oxE1 will be marked as a INVALID key, see line 229)
    bool down;              // Is a keydown?
    bool ext;               // An extended key?
} kbd_state;

extern kbd_state kbd;

void kbd_callback();

void keyboard_init();

/**
 * Encode each key on the keyboard
 * for characters, we use ascii
 * and for other non-character keys
 * we enumerate them starting from
 * 256 where the ascii ends
 * 
 * Note that return key is also an
 * ascii character (known as Carriage 
 * Return) and falls into the control 
 * (non-printable) character set 0-31 
 * where 32-127 is the standard
 * character set and from 127 onward 
 * is the enxtend character set.
 * 
 * https://www.ascii-codes.com/
 */

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

extern int scan_code_map[]; // Map a particular make code to a key

/**
 * If it is a printable character, we have to determine which one it is
 * by taking both the keyboard status (is shift held down, is capslock enabled?)
 * and the keyboard key itself into the consideration.
 */
uint8_t get_printable_char();

void key_stroke_action(); // Ttanslates a key stroke to certain actions shown on the screen, such as printing/ deleting a character

#endif