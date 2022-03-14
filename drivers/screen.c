#include "screen.h"

// Cursor is handled by hardware : CRT microcontroller
// CRT data register is mapped to port address 0x3d4
// CRT index rehister is mapped to port address 0x3d5
// Two steps to get cursor info:
// 1. Specify it's cursor data that we want, 
//    we need to write to the index register through port 0x3d5,
//    ,index 14 for cursor location high and 15 for high
// 2. Read data register through port 0x3d4 to get cursor data
//    (step 1 has led CRT to write the data we instructed to data register!)
int get_cursor() {
    int pixel_offset;
    port_byte_out(CRT_INDEX_REG, 14);
    pixel_offset = port_byte_in(CRT_DATA_REG);
    pixel_offset = pixel_offset << 8;
    port_byte_out(CRT_INDEX_REG, 15);
    pixel_offset += port_byte_in(CRT_DATA_REG);
    return pixel_offset * 2;
}
void set_cursor(int byte_offset) {
    // It is required to use pixel offsets to set the cursor
    int pixel_offset = byte_offset / 2;
    port_byte_out(CRT_INDEX_REG, 14);
    port_byte_out(CRT_DATA_REG, (unsigned char)(pixel_offset >> 8));
    port_byte_out(CRT_INDEX_REG, 15);
    port_byte_out(CRT_DATA_REG, (unsigned char)pixel_offset);
}

int get_screen_offset(int col, int row) {
    // Return byte offset
    return (row * 80 + col) * 2;
}

void print_char (char character, int col, int row, char char_attrib) {
    // Video memory ptr
    unsigned char * video_mem = (unsigned char *) VIDEO_ADDRESS;
    // If color is not set, use the default one
    if (!char_attrib) {
        char_attrib = WHITE_ON_BLACK;
    }
    // Calculate screen offset
    unsigned short offset;
    if (col >= 0 && row >= 0) {
        offset = get_screen_offset(col, row);
    }
    // If offset are negati
    else {
        offset = get_cursor();
    }
    // Handle '\n' differently
    if (character == '\n') {
        // Set the offset to the end of the current row
        // so that it will be advanced to the next row when leter updating offset
        int current_row = offset / (MAX_COL * 2);
        offset = get_screen_offset(79, current_row);
    }
    else {
        // Write to video memory
        video_mem[offset] = character;
        video_mem[offset + 1] = char_attrib;
    }
    // Incermenting offset it by 2 (2 byte per pixel)
    offset += 2;
    // Scroll if reaching the end of the screen
    offset = handle_scrolling(offset);
    // Update cursor position
    set_cursor(offset);
}

// Fill screen with space characters
void clear_screen() {
    for (int i = 0; i < MAX_ROW; i++) {
        for (int j = 0; j < MAX_COL; j++) {
            print_char(' ', j, i, WHITE_ON_BLACK);
        }  
    }
    set_cursor(0);
}

void print_at(char * msg, int col, int row)
{
    if (col >= 0 && row >= 0) {
        int byte_offset = get_screen_offset(col, row);
        set_cursor(byte_offset);
    }
    // If col and row are not set, print at the current curosr position
    for (int i = 0; msg[i] != 0; i++) {
        print_char(msg[i], -1, -1, WHITE_ON_BLACK);
    }
}

void print(char * msg) {
    print_at(msg, -1, -1);
}

void putchar(char c) {
    print_char(c, -1, -1, WHITE_ON_BLACK);
}

// Decide if the offset exceeds the boundary and scroll the screen 1 row up if so
// and returns the new offset or the offset remains unchanged
int handle_scrolling(int offset) {
    if (offset < MAX_COL * MAX_ROW * 2) {
        return offset;
    }
    // Scroll
    // Copy each row up 1 row (starting from row 1 not row 0)
    // Clear the last row (mustn't fill up the memory with 0 or the cursor will disappear -> must keep WHITE_On_BLACK property)
    // Set the cursor to the begining last row
    mem_copy((char *) (get_screen_offset(0, 1) + VIDEO_ADDRESS),
            (char *) (get_screen_offset(0, 0) + VIDEO_ADDRESS),
            MAX_COL * (MAX_ROW - 1) * 2);
    
    for (int i = 0; i < MAX_COL - 1; i++) {
        print_char(' ', i, MAX_ROW - 1, WHITE_ON_BLACK);
    }

    return get_screen_offset(0, MAX_ROW - 1);
}

enum format_types {
    FMT_INT,
    FMT_FLOAT,
    FMT_STR
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
        PARSE_PLACEHOLDER
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
                    else
                        state = PRINT_STR;
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
        }
    }
    
}
