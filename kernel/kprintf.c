#include"kprintf.h"

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

void kprintf(const char * fromat, ...) {
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
                    print(s);
                }
                else {
                    print(s);
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
                    print(s);
                }
                else {
                    print(s);
                    print_spaces(ph_struct.numOfZeros);
                }
                state = INIT;
            } break;

            case PRINT_STR:
            {
                char * arg_str = va_arg(args, char*);
                if (!ph_struct.padFromBack) {
                    print_spaces(ph_struct.numOfZeros);
                    print(arg_str);
                }
                else {
                    print(arg_str);
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

