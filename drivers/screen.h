#ifndef SCREEN_H
#define SCREEN_H

#include <stdarg.h>
#include "low_level.h"
#include "./../kernel/util.h"
#define VIDEO_ADDRESS 0xC00b8000
#define MAX_ROW 25
#define MAX_COL 80
#define WHITE_ON_BLACK 0x07
#define CRT_INDEX_REG 0x3D4
#define CRT_DATA_REG 0x3D5
#define PIXEL_SIZE 2

void print_char (char character, int col, int row, char char_attrib);
int get_screen_offset(int col, int row);
int get_cursor(void);
void set_cursor(int offset);
void clear_screen(void);
void print_at(char * msg, int col, int row);
void print(char * msg);
int handle_scrolling(int offset);
void printf(const char * fromat, ...);
void putchar(char c);

#endif