#include "screen.h"

uint8_t crt_read_data(uint8_t index) {
    port_byte_out(CRT_PORT_INDEX, index); // Select whihc data register to read
    return port_byte_in(CRT_PORT_DATA);
}

uint8_t crt_write_data(uint8_t index, uint8_t data) {
    port_byte_out(CRT_PORT_INDEX, index); // Select whihc data register to write
    port_byte_out(CRT_PORT_DATA, data);
}

// Cursor is handled by hardware : CRT microcontroller
// CRT data register is mapped to port address 0x3d4
// CRT index rehister is mapped to port address 0x3d5
// Two steps to get cursor info:
// 1. Specify it's cursor data that we want, 
//    we need to write to the index register through port 0x3d5,
//    ,index 14 for cursor location high and 15 for high
// 2. Read data register through port 0x3d4 to get cursor data
//    (step 1 has led CRT to write the data we instructed to data register!)
int get_cursor_offset() {
    return ((crt_read_data(CRT_REG_INDEX_CURSOR_HIGH) << 8) + crt_read_data(CRT_REG_INDEX_CURSOR_LOW)) * 2;
}

void set_cursor(int byte_offset) {
    int pixel_offset = byte_offset / 2; // It is required to use pixel offsets to set the cursor
    crt_write_data(CRT_REG_INDEX_CURSOR_HIGH, (uint8_t)(pixel_offset >> 8));
    crt_write_data(CRT_REG_INDEX_CURSOR_LOW, (uint8_t)pixel_offset);
}

int get_screen_offset(int col, int row) {
    // Return byte offset
    return (row * 80 + col) * 2;
}

void print_char (char character, int col, int row, char char_attrib) {
    // If color is not set, default to WHITE_ON_BLACK
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
        offset = get_cursor_offset();
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
        ((char*)VIDEO_MEM_TEXT)[offset] = character;
        ((char*)VIDEO_MEM_TEXT)[offset + 1] = char_attrib;
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

// Warpper for print_char
void putchar(char c) {
    print_char(c, -1, -1, WHITE_ON_BLACK);
}

void print_str(char * msg, int col, int row)
{
    if (col >= 0 && row >= 0) {
        int byte_offset = get_screen_offset(col, row);
        set_cursor(byte_offset);
    }
    // If col and row are not set, print at the current curosr position
    for (int i = 0; msg[i]; i++)
        putchar(msg[i]);
}

// Warpper for print_str
void print(char * msg) {
    print_str(msg, -1, -1);
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
    mem_copy((char *) (get_screen_offset(0, 1) + VIDEO_MEM_TEXT),
            (char *) (get_screen_offset(0, 0) + VIDEO_MEM_TEXT),
            MAX_COL * (MAX_ROW - 1) * 2);
    
    for (int i = 0; i < MAX_COL - 1; i++) {
        print_char(' ', i, MAX_ROW - 1, WHITE_ON_BLACK);
    }

    return get_screen_offset(0, MAX_ROW - 1);
}