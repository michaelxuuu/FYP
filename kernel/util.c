#include "util.h"

void mem_copy(char * src, char * dest, int n) {
    for (int i = 0; i < n; i++) {
        *(dest + i) = *(src + i);
    }
}

void mem_set(void * str, int val, int size) {
    unsigned char * p = str;
    while (size--)
        *p++ = val;
}

void swap(char s[], int x, int y) {
    char temp = s[x];
    s[x] = s[y];
    s[y] = temp;
}

// len is even: len/2 is 1 right to the middle
// len is odd: len/2 is the middle index
// => swapping always ends at and excludes len/2
void reverse(char s[], int len) {
    for (int i = 0; i < len/2; i++)
        swap(s, i, len-i-1);
}

// num is how many digits to reserve (output string length)
int iToStr(long i, char * s, int num) {
    int ct = 0;
    long temp_i = i;
    // If it's a negative integer, convert it to a positive integer
    // Set the first byte of the output stirng to a negative sign
    if (i < 0) {
        temp_i = -i;
        s[0] = '-';
        ++s;
    }
    if (temp_i == 0)
    {
        s[0] = '0';
        ++ct;
    }
    else 
    {
        for (long base = 1; temp_i >= base; base *= 10, ct++)
            // Deviding a decimal number by base 10 can be seen as shifting it left by 1 decimal digit
            // then mod 10 will always yield the last digit
            s[ct] = '0' + (temp_i / base) % 10;
            // If the number of digits doesn't match the required output string length
            // pad 0's until it hits the required length
    }
    for (; ct < num; ct++)
        s[ct] = '0';
    // Reverse the output string
    reverse(s, ct);
    // Add null terminator
    s[ct] = 0;
    return ct;
}

// Return the base to the power of power
long pow(long base, int power) {
    long result = base;
    for (int i = 1; i < power; i++)
        result *= base;
    return result;
}

// num is how many digits to reserve after the decimal point
void fToStr(double f, char * s, int num) {
    double temp_f = f;
    // If the floating number is negative we convert it to positive
    // Set the first byte of the output string to a negative sign
    if (f < 0) {
        s[0] = '-';
        temp_f = -f;
        ++s;
    }
    // Split the floating point number into 2 parts
    long ipart = (long) temp_f;
    double fpart = temp_f - (double) ipart;
    // Convert the integer part to a string
    int ipart_end = iToStr(ipart, s, -1);
    if (!num)
        return;
    // Add a decimal point
    *(s + ipart_end) = '.';
    // Convert the floating part to an integer accroding to how many digits to reserve
    fpart *= pow(10, num);
    iToStr((long) fpart, s + ipart_end + 1, num);
}

int strToInt(const char * s, int i, int j) {
    int result = 0;
    if (i != -1)
        for (;i <= j; i++)
            result = result * 10 + s[i] - '0';
    else
        for (int i = 0; s[i] != 0; i++)
            result = result * 10 + s[i] - '0';
    return result;
}

int isDigit(char c) {
    return c >= '0' && c <= '9';
}

int isLetter(char c) {
    return c >= 'a' && c <= 'z';
}