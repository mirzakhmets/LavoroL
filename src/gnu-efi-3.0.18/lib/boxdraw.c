/*++

Copyright (c) 1998  Intel Corporation

Module Name:

    BoxDraw.c

Abstract:
    Lib functions to support Box Draw Unicode code pages.



Revision History

--*/

#include "lib.h"

typedef struct {
    CHAR16  Unicode;
    CHAR8   PcAnsi;
    CHAR8   Ascii;
} UNICODE_TO_CHAR;


//
// This list is used to define the valid extend chars.
// It also provides a mapping from Unicode to PCANSI or
// ASCII. The ASCII mapping we just made up.
//
//

STATIC UNICODE_TO_CHAR UnicodeToPcAnsiOrAscii[] = {
    { BOXDRAW_HORIZONTAL,                 (CHAR8) 0xc4, L'-'}, 
    { BOXDRAW_VERTICAL,                   (CHAR8) 0xb3, L'|'},
    { BOXDRAW_DOWN_RIGHT,                 (CHAR8) 0xda, L'/'},
    { BOXDRAW_DOWN_LEFT,                  (CHAR8) 0xbf, L'\\'},
    { BOXDRAW_UP_RIGHT,                   (CHAR8) 0xc0, L'\\'},
    { BOXDRAW_UP_LEFT,                    (CHAR8) 0xd9, L'/'},
    { BOXDRAW_VERTICAL_RIGHT,             (CHAR8) 0xc3, L'|'},
    { BOXDRAW_VERTICAL_LEFT,              (CHAR8) 0xb4, L'|'},
    { BOXDRAW_DOWN_HORIZONTAL,            (CHAR8) 0xc2, L'+'},
    { BOXDRAW_UP_HORIZONTAL,              (CHAR8) 0xc1, L'+'},
    { BOXDRAW_VERTICAL_HORIZONTAL,        (CHAR8) 0xc5, L'+'},
    { BOXDRAW_DOUBLE_HORIZONTAL,          (CHAR8) 0xcd, L'-'},
    { BOXDRAW_DOUBLE_VERTICAL,            (CHAR8) 0xba, L'|'},
    { BOXDRAW_DOWN_RIGHT_DOUBLE,          (CHAR8) 0xd5, L'/'},
    { BOXDRAW_DOWN_DOUBLE_RIGHT,          (CHAR8) 0xd6, L'/'},
    { BOXDRAW_DOUBLE_DOWN_RIGHT,          (CHAR8) 0xc9, L'/'},
    { BOXDRAW_DOWN_LEFT_DOUBLE,           (CHAR8) 0xb8, L'\\'},
    { BOXDRAW_DOWN_DOUBLE_LEFT,           (CHAR8) 0xb7, L'\\'},
    { BOXDRAW_DOUBLE_DOWN_LEFT,           (CHAR8) 0xbb, L'\\'},
    { BOXDRAW_UP_RIGHT_DOUBLE,            (CHAR8) 0xd4, L'\\'},
    { BOXDRAW_UP_DOUBLE_RIGHT,            (CHAR8) 0xd3, L'\\'},
    { BOXDRAW_DOUBLE_UP_RIGHT,            (CHAR8) 0xc8, L'\\'},
    { BOXDRAW_UP_LEFT_DOUBLE,             (CHAR8) 0xbe, L'/'},
    { BOXDRAW_UP_DOUBLE_LEFT,             (CHAR8) 0xbd, L'/'},
    { BOXDRAW_DOUBLE_UP_LEFT,             (CHAR8) 0xbc, L'/'},
    { BOXDRAW_VERTICAL_RIGHT_DOUBLE,      (CHAR8) 0xc6, L'|'},
    { BOXDRAW_VERTICAL_DOUBLE_RIGHT,      (CHAR8) 0xc7, L'|'},
    { BOXDRAW_DOUBLE_VERTICAL_RIGHT,      (CHAR8) 0xcc, L'|'},
    { BOXDRAW_VERTICAL_LEFT_DOUBLE,       (CHAR8) 0xb5, L'|'},
    { BOXDRAW_VERTICAL_DOUBLE_LEFT,       (CHAR8) 0xb6, L'|'},
    { BOXDRAW_DOUBLE_VERTICAL_LEFT,       (CHAR8) 0xb9, L'|'},
    { BOXDRAW_DOWN_HORIZONTAL_DOUBLE,     (CHAR8) 0xd1, L'+'},
    { BOXDRAW_DOWN_DOUBLE_HORIZONTAL,     (CHAR8) 0xd2, L'+'},
    { BOXDRAW_DOUBLE_DOWN_HORIZONTAL,     (CHAR8) 0xcb, L'+'},
    { BOXDRAW_UP_HORIZONTAL_DOUBLE,       (CHAR8) 0xcf, L'+'},
    { BOXDRAW_UP_DOUBLE_HORIZONTAL,       (CHAR8) 0xd0, L'+'},
    { BOXDRAW_DOUBLE_UP_HORIZONTAL,       (CHAR8) 0xca, L'+'},
    { BOXDRAW_VERTICAL_HORIZONTAL_DOUBLE, (CHAR8) 0xd8, L'+'},
    { BOXDRAW_VERTICAL_DOUBLE_HORIZONTAL, (CHAR8) 0xd7, L'+'},
    { BOXDRAW_DOUBLE_VERTICAL_HORIZONTAL, (CHAR8) 0xce, L'+'},

    { BLOCKELEMENT_FULL_BLOCK,            (CHAR8) 0xdb, L'*'},
    { BLOCKELEMENT_LIGHT_SHADE,           (CHAR8) 0xb0, L'+'},

    { GEOMETRICSHAPE_UP_TRIANGLE,         (CHAR8) 0x1e, L'^'},
    { GEOMETRICSHAPE_RIGHT_TRIANGLE,      (CHAR8) 0x10, L'>'},
    { GEOMETRICSHAPE_DOWN_TRIANGLE,       (CHAR8) 0x1f, L'v'},
    { GEOMETRICSHAPE_LEFT_TRIANGLE,       (CHAR8) 0x11, L'<'},

    /* BugBug: Left Arrow is an ESC. We can not make it print
                on a PCANSI terminal. If we can make left arrow 
                come out on PC ANSI we can add it back.

    { ARROW_LEFT,                         0x1b, L'<'},
    */

    { ARROW_UP,                           (CHAR8) 0x18, L'^'},
    
    /* BugBut: Took out left arrow so right has to go too.
       { ARROW_RIGHT,                        0x1a, L'>'},
    */      
    { ARROW_DOWN,                         (CHAR8) 0x19, L'v'},
    
    { 0x0000, (CHAR8) 0x00, L'\0' }
};


BOOLEAN
LibIsValidTextGraphics (
    IN  CHAR16  Graphic,
    OUT CHAR8   *PcAnsi,    OPTIONAL
    OUT CHAR8   *Ascii      OPTIONAL
    )
/*++

Routine Description:

    Detects if a Unicode char is for Box Drawing text graphics.

Arguments:

    Grphic  - Unicode char to test.

    PcAnsi  - Optional pointer to return PCANSI equivalent of Graphic.

    Asci    - Optional pointer to return Ascii equivalent of Graphic.

Returns:

    TRUE if Gpaphic is a supported Unicode Box Drawing character.

--*/{
    UNICODE_TO_CHAR     *Table;

    if ((((Graphic & 0xff00) != 0x2500) && ((Graphic & 0xff00) != 0x2100))) {
     
        //
        // Unicode drawing code charts are all in the 0x25xx range, 
        //  arrows are 0x21xx
        //
        return FALSE;
    }

    for (Table = UnicodeToPcAnsiOrAscii; Table->Unicode != 0x0000; Table++) {
        if (Graphic == Table->Unicode) {
            if (PcAnsi) {
                *PcAnsi = Table->PcAnsi; 
            }
            if (Ascii) {
                *Ascii = Table->Ascii;
            }
            return TRUE;
        }
    }
    return FALSE;
}

BOOLEAN
IsValidAscii (
    IN  CHAR16  Ascii
    )
{
    if ((Ascii >= 0x20) && (Ascii <= 0x7f)) {
        return TRUE;
    }              
    return FALSE;
}

BOOLEAN
IsValidEfiCntlChar (
    IN  CHAR16  c
    )
{
    if (c == CHAR_NULL || c == CHAR_BACKSPACE || c == CHAR_LINEFEED || c == CHAR_CARRIAGE_RETURN) {
        return TRUE;
    }              
    return FALSE;
}

