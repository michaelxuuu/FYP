#ifndef SHELL_H
#define SHELL_H

struct
{
    char s[128];
    int tail;
    int cursor;
}cmdbuf;

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