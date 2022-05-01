#include "kbd.h"

kbd_state kbd;

int scan_code_map[] =
{
    // kbd key			scancode
	KBD_KEY_INVALID,	//0
	KBD_KEY_ESC,        //1
	KBD_KEY_1,			//2
	KBD_KEY_2,			//3
	KBD_KEY_3,			//4
	KBD_KEY_4,			//5
	KBD_KEY_5,			//6
	KBD_KEY_6,			//7
	KBD_KEY_7,			//8
	KBD_KEY_8,			//9
	KBD_KEY_9,			//0xa
	KBD_KEY_0,			//0xb
	KBD_KEY_MIN,		//0xc
	KBD_KEY_EQU,		//0xd
	KBD_KEY_BACKSPACE,	//0xe
	KBD_KEY_TAB,		//0xf
	KBD_KEY_Q,			//0x10
	KBD_KEY_W,			//0x11
	KBD_KEY_E,			//0x12
	KBD_KEY_R,			//0x13
	KBD_KEY_T,			//0x14
	KBD_KEY_Y,			//0x15
	KBD_KEY_U,			//0x16
	KBD_KEY_I,			//0x17
	KBD_KEY_O,			//0x18
	KBD_KEY_P,			//0x19
	KBD_KEY_LBRA,       //0x1a
	KBD_KEY_RBRA,       //0x1b
	KBD_KEY_RET,		//0x1c
	KBD_KEY_LCTRL,		//0x1d      ext: RCTRL
	KBD_KEY_A,			//0x1e
	KBD_KEY_S,			//0x1f
	KBD_KEY_D,			//0x20
	KBD_KEY_F,			//0x21
	KBD_KEY_G,			//0x22
	KBD_KEY_H,			//0x23
	KBD_KEY_J,			//0x24
	KBD_KEY_K,			//0x25
	KBD_KEY_L,			//0x26
	KBD_KEY_SEMI,	    //0x27
	KBD_KEY_QUOTE,		//0x28
	KBD_KEY_BACKTICK,	//0x29
	KBD_KEY_LSHIFT,		//0x2a
	KBD_KEY_BACKSLASH,	//0x2b
	KBD_KEY_Z,			//0x2c
	KBD_KEY_X,			//0x2d
	KBD_KEY_C,			//0x2e
	KBD_KEY_V,			//0x2f
	KBD_KEY_B,			//0x30
	KBD_KEY_N,			//0x31
	KBD_KEY_M,			//0x32
	KBD_KEY_COMMA,		//0x33
	KBD_KEY_DOT,		//0x34
	KBD_KEY_SLASH,		//0x35
	KBD_KEY_RSHIFT,		//0x36
	KBD_KEY_INVALID,    //0x37      KBD_KEY_KP_ASTERISK                         (not implemented)
	KBD_KEY_LALT,		//0x38      ext: RALT
	KBD_KEY_SPACE,		//0x39
	KBD_KEY_CAPSLOCK,	//0x3a
	KBD_KEY_F1,			//0x3b      
	KBD_KEY_F2,			//0x3c      
	KBD_KEY_F3,			//0x3d      
	KBD_KEY_F4,			//0x3e      
	KBD_KEY_F5,			//0x3f      
	KBD_KEY_F6,			//0x40      
	KBD_KEY_F7,			//0x41      
	KBD_KEY_F8,			//0x42      
	KBD_KEY_F9,			//0x43      
	KBD_KEY_F10,		//0x44      
	KBD_KEY_INVALID,	//0x45      KBD_KEY_KP_NUMLOCK                          (not implemented)
	KBD_KEY_INVALID,	//0x46      KBD_KEY_SCROLLLOCK                          (not implemented)
	KBD_KEY_INVALID,	//0x47      KBD_KEY_KP_7/ KBD_KEY_HOME                  (not implemented)
	KBD_KEY_UP,		    //0x48      already ext!        non-ext: KBD_KEY_KP_8
	KBD_KEY_INVALID,	//0x49      KBD_KEY_KP_9/ KBD_KEY_PAGEUP                (not implemented)
	KBD_KEY_INVALID,	//0x4a      KBD_KEY_KP_PLUS                             (not implemented)
	KBD_KEY_LEFT,	    //0x4b      already ext!        non-ext: KBD_KEY_KP_4
	KBD_KEY_INVALID,	//0x4c      KBD_KEY_KP_5                                (not implemented)
	KBD_KEY_RIGHT,	    //0x4d      already ext!        non-ext: KBD_KEY_KP_6
	KBD_KEY_INVALID,	//0x4e      KBD_KEY_KP_PLUS                             (not implemented)
    KBD_KEY_INVALID,	//0x4f      KBD_KEY_KP_1/ END                           (not implemented)
	KBD_KEY_DOWN,	    //0x50      already ext!        non-ext: KBD_KEY_KP_2
	KBD_KEY_INVALID,	//0x51      KBD_KEY_KP_3/ PAGE_DOWN                     (not implemented)
	KBD_KEY_INVALID,	//0x52      KBD_KEY_KP_0/ INSERT                        (not implemented)
    KBD_KEY_DEL,    	//0x53      already ext!        non-ext: KBD_KEY_KP_DOT
	KBD_KEY_INVALID,    //0x54      No keys associated to!
    KBD_KEY_INVALID,    //0x55      No keys associated to!
    KBD_KEY_INVALID,    //0x56      No keys associated to!
    KBD_KEY_F11,		//0x57
    KBD_KEY_F12,		//0x58
};

uint8_t ps2_controller_get_status()
{
    return port_byte_in(PS2_CONTROLLER_PORT_STATUS);
}

void ps2_controller_send_cmd(uint8_t cmd)
{
    // Wait if the input buffer is full
    while (ps2_controller_get_status() & PS2_CONTROLLER_STATUS_INBUF_FULL);
    port_byte_out(PS2_CONTROLLER_PORT_CMD, cmd);
}

uint8_t ps2_controller_read_data()
{
    // Wait if the output buffer is empty
    while (!(ps2_controller_get_status() & PS2_CONTROLLER_STATUS_OUTBUF_FULL));
    return port_byte_in(PS2_CONTROLLER_PORT_DATA);
}

void ps2_controller_send_data(uint8_t data)
{
    // Wait if the input buffer is full
    while (ps2_controller_get_status() & PS2_CONTROLLER_STATUS_INBUF_FULL);
    port_byte_out(PS2_CONTROLLER_PORT_DATA, data);
}

void kbd_callback()
{
    int make_code;

    // When irq1 is fired, check if controller's data register's been written with a SCAN CODE
    uint8_t scan_code = ps2_controller_read_data();

    // If it is the first byte of an extended scan code (a multi-byte long scan code such as PAGE UP and DELETE), they also start with 0xE0 or 0xE1
    if (scan_code == 0xE0 || scan_code == 0xE1)
    {
        kbd.key = KBD_KEY_INVALID; // 0xE0 and 0xE1 are just handers and associated with no phsycial keys
        kbd.ext = 1;
    }
    else // Second byte of an extened scan code or a single-byte scan code
    {
        // If the code is a break code (keyup), which is always the correspodning make code + 0x80 and whereby always having its 7th bit set
        if (scan_code & 0x80)
        {
            kbd.down = 0;

            // Get the key's make code (by subtracting 0x80 from its break code)
            make_code = scan_code - 0x80;
            kbd.key = scan_code_map[make_code];

            // Actions required only for releasing some special keys: alt, shift, and ctrl, either left or right ones
            switch (kbd.key)
            {
                case KBD_KEY_LCTRL:
                    kbd.ctrl = 0;
                    break;

                case KBD_KEY_LSHIFT:
                case KBD_KEY_RSHIFT:
                    kbd.shift = 0;
                    break;

                case KBD_KEY_LALT:
                    kbd.alt = 0;
                    break;
            }
        }
        else // Make code
        {
            kbd.down = 1;

            // Get the encoded key by its scan code
            kbd.key = scan_code_map[scan_code];

            // If some special key pressed, we change the keyboard state accordingly
            switch (kbd.key)
            {
                case KBD_KEY_LCTRL:
                    kbd.ctrl = 1;
                    break;

                case KBD_KEY_LSHIFT:
                case KBD_KEY_RSHIFT:
                    kbd.shift = 1;
                    break;

                case KBD_KEY_LALT:
                    kbd.alt = 1;
                    break;

                case KBD_KEY_CAPSLOCK:
                    kbd.capslock = kbd.capslock == 1 ? 0 : 1;
                    break;
            }
        }
    }
    key_stroke_action();
}

uint8_t get_printable_char()
{
    uint8_t c = 0;
    // Is the key printable? Test if this key is associated with an ascii code.
    // Pitfall: ascii control characters aren't printable. Need to exclude Carriage Return
    if (isascii(kbd.key) && kbd.key != KBD_KEY_RET && kbd.key != KBD_KEY_INVALID)
    {
        if (kbd.ctrl)
            return c;
        c = kbd.key;
        // If the shift key is being held down
        if (kbd.shift) {
            // Letter should be all capitalized
            if (isLetter(kbd.key)) {
                c = kbd.key - 32;
            }
            else {
                // For keyboard keys with two symbols, the upper symbols is chosen
                switch (kbd.key) {

                    case KBD_KEY_BACKTICK: c = '~'; break;

                    case KBD_KEY_1: c = '!'; break;

                    case KBD_KEY_2: c = '@'; break;

                    case KBD_KEY_3: c = '#'; break;

                    case KBD_KEY_4: c = '$'; break;

                    case KBD_KEY_5: c = '%'; break;

                    case KBD_KEY_6: c = '^'; break;

                    case KBD_KEY_7: c = '&'; break;

                    case KBD_KEY_8: c = '*'; break;

                    case KBD_KEY_9: c = '('; break;

                    case KBD_KEY_0: c = ')'; break;

                    case KBD_KEY_LBRA: c = '{'; break;

                    case KBD_KEY_RBRA: c = '}'; break;

                    case KBD_KEY_BACKSLASH: c = '|'; break;

                    case KBD_KEY_MIN: c = '_'; break;

                    case KBD_KEY_EQU: c = '+'; break;

                    case KBD_KEY_SEMI: c = ':'; break;

                    case KBD_KEY_QUOTE: c = '\"'; break;

                    case KBD_KEY_COMMA: c = '<'; break;

                    case KBD_KEY_DOT: c = '>'; break;

                    case KBD_KEY_SLASH: c = '?'; break;
                }
            }
        }
        // If shift is not being held down but the capslock is active
        else if (kbd.capslock) {
            if (isLetter(kbd.key)) {
                c = kbd.key - 32;
            }
        }
    }
    return c;
}

// Actiuon required only for keydown
void key_stroke_action()
{
    if (!kbd.down && kbd.key != KBD_KEY_INVALID)
        return;

    // Print a character onto the screen
    uint8_t c = get_printable_char();

    if (c) {
        proc_buf_write(cur_proc, c);
    }
    else // Not a printable key, is a control key
    {
        switch (kbd.key)
        {
            case KBD_KEY_TAB:
            case KBD_KEY_BACKSPACE:
            case KBD_KEY_RET:
            case KBD_KEY_CAPSLOCK:
            case KBD_KEY_UP:
            case KBD_KEY_DOWN:
            case KBD_KEY_LEFT:
            case KBD_KEY_RIGHT:
            case KBD_KEY_F1:
            case KBD_KEY_F2:
            case KBD_KEY_F3:
            case KBD_KEY_F4:
            case KBD_KEY_F5:
            case KBD_KEY_F6:
            case KBD_KEY_F7:
            case KBD_KEY_F8:
            case KBD_KEY_F9:
            case KBD_KEY_F10:
            case KBD_KEY_F11:
            case KBD_KEY_F12:
                proc_buf_write(cur_proc, kbd.key);
                break;
            case KBD_KEY_C:
                if (kbd.ctrl)
                    if (cur_proc->id != 0)
                        syscall_exit(0,0,0,0,0,0);
                    else
                        print("^C");
            default:
                break;
        }
    }
}

void keyboard_init()
{

    /* Register irq1 handler */
    register_handler(33, kbd_callback); // Valid hander must be registered before initializing the encoder or irq1 caused by the response byte will lead to page fault (handler address being 0x0)

    kbd.alt = kbd.ctrl = kbd.shift = 0;
    kbd.capslock = 0;
}
