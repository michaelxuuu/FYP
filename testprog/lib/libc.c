#include"stdio.h"
#include"ctype.h"
#include"string.h"

// =============================================================== STDIO.H ===============================================================

#define PRINTS(s) \
    __asm__ volatile \
    ( \
        "mov $0x0, %%eax;" \
        "mov %0, %%edi;" \
        "int $0x80;"\
        :: "b"((int)s)\
    );

// c warpper
void puts(const char *s)
{
    PRINTS(s);
}

#define PRINTC(c) \
    __asm__ volatile \
    ( \
        "mov $0x1, %%eax;" \
        "mov %0, %%edi;" \
        "int $0x80;"\
        :: "b"((int)c)\
    );

// c warpper
void putchar(char c)
{
    PRINTC(c);
}

#define FOPEN(path, attrib, dirent_ptr) \
    __asm__ volatile \
    ( \
        "mov $0x1, %%eax;" \
        "mov %0, %%edi;" \
        "mov %1, %%esi;" \
        "mov %2, %%edx;" \
        "int $0x80;" \
        :: "r"((int)path), "r"((int)attrib), "r"((int)dirent_ptr) \
    );

// C wrapper
void fopen(char *path, uint32_t attrib, dirent* p)
{
    FOPEN(path, attrib, p);
}

// -----------------------------------  PRINTF  ----------------------------------------------
enum format_types {
    FMT_INT,
    FMT_FLOAT,
    FMT_STR,
    FMT_CHAR
};

typedef struct placeHolderStruct {
    int type;
    int padFromBack;
    int numOfZeros;
    int numOfDecimalPts;
} placeHolderStruct;

int placeholder_parser(const char* *s, placeHolderStruct * ph_struct)
{
    const char * iter = *s;
    typedef enum STATES{
        INIT,
        PARSE_MINUS,
        PARSE_DOT,
        PARSE_NUM,
        PARSE_FORMAT
    } STATE;
    int dotFlag = 0;
    STATE state = INIT;
    char numOfZerosToPad[2] = {-1};
    char numOfDigitsReserved[2] = {-1};
    // DFA
    while (1)
    {
        switch (state)
        {
            case INIT:
            {
                if (*iter == '-')
                    state = PARSE_MINUS;
                else if (isDigit(*iter))
                    state = PARSE_NUM;
                else if (isLetter(*iter))
                    state = PARSE_FORMAT;
                else if (*iter == '.')
                    state = PARSE_DOT;
                else
                    // Error incomplete format
                    return 1;
            } break;

            case PARSE_MINUS:
            {
                ph_struct->padFromBack = 1;
                ++iter;
                if (isDigit(*iter))
                    state = PARSE_NUM;
                else if (isLetter(*iter))
                    state = PARSE_FORMAT;
                else if (*iter == '.')
                    state = PARSE_DOT;
                else
                    // Error incomplete format
                    // or other unexpcted character
                    return 1;
            } break;

            case PARSE_NUM:
            {
                if (dotFlag == 0)
                {
                    numOfZerosToPad[0] = iter-*s;
                    for (; isDigit(*iter); iter++);
                    numOfZerosToPad[1] = iter-1-*s;
                }
                else
                {
                    numOfDigitsReserved[0] = iter-*s;
                    for (; isDigit(*iter); iter++);
                    numOfDigitsReserved[1] = iter-1-*s;
                }
                if (*iter == '.')
                    state = PARSE_DOT;
                else if (isLetter(*iter))
                    state = PARSE_FORMAT;
                else
                    // Error : no type specifier
                    return 1;
            } break;

            case PARSE_DOT:
            {
                if (dotFlag)
                    // Error multiple dots : invalid syntax
                    return 1;
                dotFlag = 1;
                iter++;
                if (isLetter(*iter))
                    state = PARSE_FORMAT;
                else if (isDigit(*iter))
                    state = PARSE_NUM;
                else
                    // Error
                    return 1;
            } break;

            case PARSE_FORMAT:
            {
                if (*iter == 'd')
                {
                    ph_struct->type = FMT_INT;
                    ph_struct->numOfDecimalPts = 0;
                    // If numOfZerosToPad[0] is -1 there is no padding info in the format string
                    if (numOfZerosToPad[0] >= 0)
                        ph_struct->numOfZeros = strToInt(*s, numOfZerosToPad[0], numOfZerosToPad[1]);
                }
                else if (*iter == 'f')
                {
                    ph_struct->type = FMT_FLOAT;
                    // Number of digits to reserve after the decimal point
                    if (numOfDigitsReserved[0] >= 0)
                        ph_struct->numOfDecimalPts = strToInt(*s, numOfDigitsReserved[0], numOfDigitsReserved[1]);
                    else // Reserve 4 digits after the decimal point by default if not specified in the format string
                        ph_struct->numOfDecimalPts = 4;
                    // Number of zeros to be padded
                    if (numOfZerosToPad[0] >= 0)
                        ph_struct->numOfZeros = strToInt(*s, numOfZerosToPad[0], numOfZerosToPad[1]);
                }
                else if (*iter == 's')
                {
                    ph_struct->type = FMT_STR;
                    ph_struct->numOfDecimalPts = 0;
                    if (numOfZerosToPad[0] >= 0)
                        ph_struct->numOfZeros = strToInt(*s, numOfZerosToPad[0], numOfZerosToPad[1]);
                }
                else if (*iter == 'c')
                {
                    ph_struct->type = FMT_CHAR;
                    ph_struct->numOfDecimalPts = 0;
                    if (numOfZerosToPad[0] >= 0)
                        ph_struct->numOfZeros = strToInt(*s, numOfZerosToPad[0], numOfZerosToPad[1]);
                }
                else
                    // Error: unknown format
                    return 1;
                // Return successfully
                // Update iterator in the format string
                *s = iter;
                return 0;
            } break;

            // Not necessary
            default:
                // Error
                return 1;
                break;
        }
    }
}

// Print specified number of spaces
void print_spaces(int num){
    for (int i = 0; i < num; i++)
        putchar(' ');
}

void printf(const char * fromat, ...) {
    const char * iter = fromat;
    va_list args;
    va_start(args, fromat);
    typedef enum STATES {
        INIT,
        PRINT_CHAR,
        PRINT_INT,
        PRINT_FLOAT,
        PRINT_STR,
        PARSE_PLACEHOLDER,
        PRINT_CHAR_FORMAT
    } STATE;
    STATE state = INIT;
    placeHolderStruct ph_struct = {-1, 0, 0, 0};
    placeHolderStruct * ph_strcut_ptr = &ph_struct;
    while (1)
    {
        switch (state)
        {
            case INIT:
            {
                switch (*iter)
                {
                    case 0: return;
                    case '%': state = PARSE_PLACEHOLDER; break;
                    default: state = PRINT_CHAR; break;
                }
            } break;

            case PRINT_CHAR:
            {
                putchar(*(iter++));
                state = INIT;
            } break;

            case PARSE_PLACEHOLDER:
            {
                ph_struct.type = -1;
                ph_struct.numOfDecimalPts = 0;
                ph_struct.numOfZeros = 0;
                iter++;
                int invalid = placeholder_parser(&iter, ph_strcut_ptr);
                if (invalid)
                    // Error invalid syntax in placeholder
                    return;
                else {
                    if (ph_struct.type == FMT_INT)
                        state = PRINT_INT;
                    else if (ph_struct.type == FMT_FLOAT)
                        state = PRINT_FLOAT;
                    else if (ph_struct.type == FMT_STR)
                        state = PRINT_STR;
                    else 
                        state = PRINT_CHAR_FORMAT;
                }
                ++iter;
            } break;

            case PRINT_INT:
            {
                // Extract the integer from arg list
                int arg_int = va_arg(args, int);
                char s[100];
                iToStr(arg_int, s, -1);
                if (!ph_struct.padFromBack) {
                    print_spaces(ph_struct.numOfZeros);
                    puts(s);
                }
                else {
                    puts(s);
                    print_spaces(ph_struct.numOfZeros);
                }
                state = INIT;
            } break;

            case PRINT_FLOAT:
            {
                double arg_float = va_arg(args, double);
                char s[100];
                fToStr(arg_float, s, ph_struct.numOfDecimalPts);
                if (!ph_struct.padFromBack) {
                    print_spaces(ph_struct.numOfZeros);
                    puts(s);
                }
                else {
                    puts(s);
                    print_spaces(ph_struct.numOfZeros);
                }
                state = INIT;
            } break;

            case PRINT_STR:
            {
                char * arg_str = va_arg(args, char*);
                if (!ph_struct.padFromBack) {
                    print_spaces(ph_struct.numOfZeros);
                    puts(arg_str);
                }
                else {
                    puts(arg_str);
                    print_spaces(ph_struct.numOfZeros);
                }
                state = INIT;
            } break;

            case PRINT_CHAR_FORMAT:
            {
                char arg_char = va_arg(args, int);
                if (arg_char != 0) {
                    if (!ph_struct.padFromBack) {
                        print_spaces(ph_struct.numOfZeros);
                        putchar(arg_char);
                    }
                    else {
                        putchar(arg_char);
                        print_spaces(ph_struct.numOfZeros);
                    }
                }
                state = INIT;
            } break;
        }
    }
    
}

// =============================================================== CTYPE.H ===============================================================
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

int isascii(int c)
{
  return((c <= 127) && (c >= 0));
}


// =============================================================== STRING.H ===============================================================
int str_cmp(const char * s1, const char * s2, int size) {
    for (int i = 0; i < size; i++)
    {
        if (s1[i] > s2[i])
            return 1;
        else if (s1[i] < s2[i])
            return -1;
    }
    return 0;
}

int str_len(const char * s1) {
    int i;
    for (i = 0; s1[i]; i++);
    return i;
}