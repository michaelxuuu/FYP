#include"lib/stdio.h"
#include"lib/unistd.h"

void key_stroke_action(int key);

int main() {
    for(;;)
        key_stroke_action(read_kbd_buf());
}

// Actiuon required only for keydown
void key_stroke_action(int key)
{

    switch (key)
    {
        case KBD_KEY_TAB:
            printf("    ");
            break;
        case KBD_KEY_BACKSPACE:
            cursor_backspace(1);
            break;
        case KBD_KEY_RET:
            putchar('\n');
            break;
        case KBD_KEY_UP:
            printf("^[[A");
            break;
        case KBD_KEY_DOWN:
            printf("^[[B");
            break;
        case KBD_KEY_LEFT:
            printf("^[[D");
            break;
        case KBD_KEY_RIGHT:
            printf("^[[C");
            break;
        case KBD_KEY_F1:
            //printf("F1\n");
            break;
        case KBD_KEY_F2:
            //printf("F2\n");
            break;
        case KBD_KEY_F3:
            //printf("F3\n");
            break;
        case KBD_KEY_F4:
            //printf("F4\n");
            break;
        case KBD_KEY_F5:
            //printf("F5\n");
            break;
        case KBD_KEY_F6:
            //printf("F6\n");
            break;
        case KBD_KEY_F7:
            //printf("F7\n");
            break;
        case KBD_KEY_F8:
            //printf("F8\n");
            break;
        case KBD_KEY_F9:
            //printf("F9\n");
            break;
        case KBD_KEY_F10:
            //printf("F10\n");
            break;
        case KBD_KEY_F11:
            //printf("F11\n");
            break;
        case KBD_KEY_F12:
            //printf("F12\n");
            break;
        case -1: // no input key detected!
            break;
        default:
            putchar(key); // tab becomes a single space char
            break;
        }
}