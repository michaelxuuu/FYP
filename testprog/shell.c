#include"lib/stdio.h"
#include"lib/stdlib.h"
#include"lib/string.h"
#include"lib/ctype.h"
#include"lib/unistd.h"
#include"shell.h"

dirent wdir;

int main() {
    cmd_init();
    shebang();
    for(;;)
        key_stroke_action(read_kbd_buf());
}

void cmd_init()
{
    cmdbuf.tail = 0;
    cmdbuf.cursor = 0;
}

void key_char(char c)
{
    if (cmdbuf.cursor == 126 | cmdbuf.tail == 126) // buffer full (1 char reserved for NULL terminator)
    {
        putchar(c);
        return;
    }

    if (cmdbuf.cursor == cmdbuf.tail) // append
    {
        putchar(c);
        cmdbuf.s[cmdbuf.cursor] = c;
    }
    else {  // insert with shifting

        // right shift the part of the cmdbuf starting from the cursor position by 1
        int d = cmdbuf.tail - cmdbuf.cursor;
        for (int i = cmdbuf.tail; i > cmdbuf.cursor; i--)
            cmdbuf.s[i] = cmdbuf.s[i-1];
        // insert the character at the vacancy made by the shift
        cmdbuf.s[cmdbuf.cursor] = c;

/* display on the screen */
        // erase the part of the cmdbuf on the screen starting from the cursor position
        cursor_right(d);
        cursor_backspace(d);
        // print the part of the cmdbuf deleted using the updated cmdbuf buffer
        for (int i = cmdbuf.cursor; i < cmdbuf.tail + 1; i++)
            putchar(cmdbuf.s[i]);
        // 'putchar' has made cursor be at the end, so we reset it so it can be consistent with cmdbuf.position
        cursor_left(d);
    }
    cmdbuf.cursor++;
    cmdbuf.tail++;
}

void key_left()
{
    if (cmdbuf.cursor <= 0)
        return;
    cursor_left(1);
    cmdbuf.cursor--;
}

void key_right()
{
    if (cmdbuf.cursor >= cmdbuf.tail)
        return;
    cursor_right(1);
    cmdbuf.cursor++;
}

void key_delete()
{
    if (!cmdbuf.cursor)
        return;
    
    if(cmdbuf.cursor == cmdbuf.tail)
        cursor_backspace(1);
    else{
        // left shift the part of the cmdbuf starting from the cursor position - 1 by 1
        int d = cmdbuf.tail - cmdbuf.cursor;
        for (int i = cmdbuf.cursor - 1; i < cmdbuf.tail; i++)
            cmdbuf.s[i] = cmdbuf.s[i+1];

/* display on the screen */
        // erase the part of the cmdbuf starting from the cursor position - 1
        cursor_right(d);
        cursor_backspace(d+1);
        // print the part of the cmdbuf deleted using the updated cmdbuf buffer
        for (int i = cmdbuf.cursor - 1; i < cmdbuf.tail - 1; i++)
            putchar(cmdbuf.s[i]);
        // set the screen cursor to the position that matches cmdbuf.curosr
        cursor_left(d);
    }
    cmdbuf.cursor--;
    cmdbuf.tail--;
}

void key_ret()
{
    putchar('\n'); // newline
    cmdbuf.s[cmdbuf.tail] = 0; // append terminating NULL to the tail
    shell_execute(); // execute the cmdbuf in the cmdbuf buffer
    cmd_init();
    shebang();
}

void shebang() {
    get_cur_dir(&wdir); // update widr
    printf("%s$", wdir.name);
}

void shell_execute()
{
    char* args[10];
    mem_set(args, 0, sizeof(args));

    int argct = 0;

    for (int i = 0; cmdbuf.s[i]; i++)
    {
        if (cmdbuf.s[i] == ' ')
            cmdbuf.s[i] = 0;
        else if (i == 0 || cmdbuf.s[i-1] == 0)
        {
            args[argct] = cmdbuf.s + i;
            argct++;
        }
    }
    argct--;

    if(str_cmp("ls", args[0]) == 0) 
    {   
        if (!argct)
            ls(".");
        else 
            ls(args[1]);
    }
    else if (str_cmp("cd", args[0]) == 0)
    {
        if (!argct)
            cd(".");
        else 
            cd(args[1]);
    }
    else if (str_cmp("mkdir", args[0]) == 0)
    {
        if (!argct)
            printf("Dir name missing!\n");
        else 
            make_dir(args[1]);
    }
    else if (str_cmp("clear", args[0]) == 0) 
        clear_screen();
    else if (args[0][0] == '.' && args[0][1] == '/')
    {
        if (!fork()) // child
        {
            if (!argct) // no args
                exec(args[0] + 2);
            else
                execv(args[0] + 2, args + 1);
        }
        wait();
        // parent
    }
}

// Actiuon required only for keydown
void key_stroke_action(int key)
{

    switch (key)
    {
        case KBD_KEY_TAB:
            printf("    ");
            key_char(' '); // tab becomes a single space char
            break;
        case KBD_KEY_BACKSPACE:
            key_delete();
            break;
        case KBD_KEY_RET:
            key_ret();
            break;
        case KBD_KEY_UP:
            //cursor_action(1);
            break;
        case KBD_KEY_DOWN:
            //cursor_action(2);
            break;
        case KBD_KEY_LEFT:
            key_left();
            break;
        case KBD_KEY_RIGHT:
            key_right();
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
            key_char(key); // tab becomes a single space char
            break;
        }
}

void ls(char *p)
{
    dirent d;
    if (str_cmp(p, ".") == 0)
        d = wdir;
    else if (str_cmp(p, "..") == 0)
        readdir(&wdir, &d, 1); // read the second entry of the current dir
    else 
        fopen(p, DIRENT_ATTRIB_DIR, &d);

    if (!d.blockno) {
        printf("Dir not found!\n");
        return;
    }

    for (int i = 0; i < 128; i++)
    {
        dirent e;
        readdir(&d, &e, i);
        if (e.attrib & DIRENT_ATTRIB_USED)
        {
            if (i == 0)
                printf(".\n");
            else if (i == 1)
                printf("..\n");
            else 
                printf("%s\n", e.name);
        }
    }
}

void cd(char *p)
{
    dirent d;
    int ret;
    if (str_cmp(p, ".") == 0)
        return;
    else if (str_cmp(p, "..") == 0) {
        readdir(&wdir, &d, 1);
        change_dir(d.name);
        get_cur_dir(&wdir);
    }
    else
    {
        if (change_dir(p))
            get_cur_dir(&wdir);
        else
            printf("Dir not found!\n");
    }
}