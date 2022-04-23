#include"lib/stdio.h"
#include"lib/stdlib.h"
#include"lib/string.h"
#include"lib/ctype.h"

#include"shell.h"

struct
{
    char s[128];
    int i;
}cmd;

char wdir[23];

int main()
{

    get_cur_dir(wdir);
    printf("%s$", wdir);

    while (1)
    {
        int key = read_kbd_buf();
        if (key != -1)
            key_stroke_action(key);
    }
}

void traverse_dir()
{
    dirent curdir;
    fopen(wdir, DIRENT_ATTRIB_DIR | DIRENT_ATTRIB_USED, &curdir);
    dirent ent;
    int i = 0;
    readdir(&curdir, &ent, i);
    while (ent.attrib & DIRENT_ATTRIB_USED)
    {
        i++;
        printf("%s\n", ent.name);
        readdir(&curdir, &ent, i);
    }
}

void shell_execute()
{
    int args[10];
    int argct = 0;

    for (int i = 0; cmd.s[i]; i++)
    {
        if (cmd.s[i] == ' ')
            cmd.s[i] = 0;
        else if ((i == 0) | cmd.s[i-1] == 0)
        {
            args[argct] = i;
            argct++;
        }
    }
    argct--;

    if(str_cmp("ls", cmd.s + args[0]) == 0)
    {
        traverse_dir();
    }
    else if (str_cmp("cd", cmd.s + args[0]) == 0)
    {
        dirent dir;
        dir.blockno = 0;
        fopen(cmd.s + args[1], DIRENT_ATTRIB_USED | DIRENT_ATTRIB_DIR, &dir);
        if (!dir.blockno)
            printf("Dir not found!\n");
        else
            mem_copy(cmd.s + args[1], wdir, str_len(cmd.s + args[1]) + 1);
    }
    else if (str_cmp("mkdir", cmd.s + args[0]) == 0)
    {
        
    }
    

    printf("\n%s$", wdir);
        
    cmd.i = 0;
}

// Actiuon required only for keydown
void key_stroke_action(int key)
{

    switch (key)
    {
        case KBD_KEY_TAB:
            printf("    ");
            cmd.s[cmd.i++] = ' '; // tab becomes a single space char
            break;
        case KBD_KEY_BACKSPACE:
            cmd.i--;
            move_cursor(0);
            break;
        case KBD_KEY_RET:
            putchar('\n');
            cmd.s[cmd.i] = 0;
            shell_execute();
            break;
        case KBD_KEY_UP:
            move_cursor(1);
            break;
        case KBD_KEY_DOWN:
            move_cursor(2);
            break;
        case KBD_KEY_LEFT:
            move_cursor(3);
            break;
        case KBD_KEY_RIGHT:
            move_cursor(4);
            break;
        case KBD_KEY_F1:
            printf("F1\n");
            break;
        case KBD_KEY_F2:
            printf("F2\n");
            break;
        case KBD_KEY_F3:
            printf("F3\n");
            break;
        case KBD_KEY_F4:
            printf("F4\n");
            break;
        case KBD_KEY_F5:
            printf("F5\n");
            break;
        case KBD_KEY_F6:
            printf("F6\n");
            break;
        case KBD_KEY_F7:
            printf("F7\n");
            break;
        
        case KBD_KEY_F8:
            printf("F8\n");
            break;
        case KBD_KEY_F9:
            printf("F9\n");
            break;
        case KBD_KEY_F10:
            printf("F10\n");
            break;
        case KBD_KEY_F11:
            printf("F11\n");
            break;
        case KBD_KEY_F12:
            printf("F12\n");
            break;
        default:
            putchar(key);
            cmd.s[cmd.i++] = key;
            break;
        }
}