#ifndef UTIL_H
#define UTIL_H

void mem_copy(char * src, char * dest, int n);
void mem_set(void * p, int val, int size);
void swap(char s[], int i, int j);
void reverse_str(char s[], int len);
int int_to_str(int a, char s[]);
void float_to_str(double a, char s[]);
void swap(char s[], int x, int y);
void reverse(char s[], int len);
int iToStr(long i, char * s, int num);
long pow(long base, int power);
void fToStr(double f, char * s, int num);
int strToInt(const char * s, int i, int j);
int isDigit(char c);
int isLetter(char c);

#define low_16(addr) (uint16_t)(addr & 0xFFFF)
#define high_16(addr) (uint16_t)((addr >> 16) & 0xFFFF)

#endif