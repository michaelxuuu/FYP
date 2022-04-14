#ifndef SCREEN_H
#define SCREEN_H

#include "low_level.h"
#include "../include/type.h"

#define VIDEO_MEM_TEXT 0xC00b8000

#define CRT_PORT_INDEX 0x3D4
#define CRT_PORT_DATA 0x3D5

#define CRT_REG_INDEX_CURSOR_HIGH 0xE
#define CRT_REG_INDEX_CURSOR_LOW 0xF

#define MAX_ROW 25
#define MAX_COL 80

#define WHITE_ON_BLACK 0x07

void print_char (char character, int col, int row, char char_attrib);
int get_screen_offset(int col, int row);
int get_cursor_offset(void);
void set_cursor(int offset);
void clear_screen(void);
void print_str(char * msg, int col, int row);
void print(char * msg);
int handle_scrolling(int offset);
void putchar(char c);

#endif