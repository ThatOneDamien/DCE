#ifndef _DCE_APP_H
#define _DCE_APP_H

typedef struct
{
    size_t Capacity;
    size_t Size;
    char* Symbols;
} EditorLine;

typedef struct
{
    size_t Line_Capacity;    // Cap for number of lines
    size_t Line_Count;       // Number of lines
    EditorLine* Lines;       // Array of lines
    size_t Cursor_Line;      // Line number of cursor
    size_t Cursor_XOff;      // Character offset of cursor
    size_t Cursor_XPixelPos; // Cursor x position in pixels from left
} EditorStorage;



void dce_close_app();


#endif // _DCE_APP_H
