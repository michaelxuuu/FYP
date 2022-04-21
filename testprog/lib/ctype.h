#ifndef CTYPE_H
#define CTYPE_H

void mem_copy(char * src, char * dest, int n);

void mem_set(void * str, int val, int size);

void swap(char s[], int x, int y);

// len is even: len/2 is 1 right to the middle
// len is odd: len/2 is the middle index
// => swapping always ends at and excludes len/2
void reverse(char s[], int len);

// num is how many digits to reserve (output string length)
int iToStr(long i, char * s, int num);

// Return the base to the power of power
long pow(long base, int power);

// num is how many digits to reserve after the decimal point
void fToStr(double f, char * s, int num);

int strToInt(const char * s, int i, int j);

int isDigit(char c);

int isLetter(char c);

int isascii(int c);

#endif