/*
 * Font4x5
 *
 * The font data are defined as
 *
 * struct _FONT_ {
 *     uint16_t   font_Size_in_Bytes_over_all_included_Size_it_self;
 *     uint8_t    font_Width_in_Pixel_for_fixed_drawing;
 *     uint8_t    font_Height_in_Pixel_for_all_characters;
 *     unit8_t    font_First_Char;
 *     uint8_t    font_Char_Count;
 *
 *     uint8_t    font_Char_Widths[font_Last_Char - font_First_Char +1];
 *                  // for each character the separate width in pixels,
 *                  // characters < 128 have an implicit virtual right empty row
 *
 *     uint8_t    font_data[];
 *                  // bit field of all characters
 */

#include <inttypes.h>
#include <avr/pgmspace.h>

#ifndef FONT4X5_H
#define FONT4X5_H

#define FONT4X5_WIDTH 4
#define FONT4X5_HEIGHT 5

static uint8_t Font4x5[] PROGMEM = {
    0x00, 0x00, // size
    0x04, // width
    0x05, // height
    0x2D, // first char
    0x0D, // char count
    
    // char widths
    // null
    
    // font data
    0x00, 0x04, 0x04, 0x04,  // 45 -
    0x00, 0x00, 0x10, 0x00,  // 46 .
    0x10, 0x08, 0x04, 0x02,  // 47 /
    0x1f, 0x11, 0x11, 0x1f,  // 48 0
    0x00, 0x02, 0x1f, 0x00,  // 49 1
    0x1d, 0x15, 0x15, 0x17,  // 50 2
    0x15, 0x15, 0x15, 0x1f,  // 51 3
    0x07, 0x04, 0x1f, 0x04,  // 52 4
    0x17, 0x15, 0x15, 0x1d,  // 53 5
    0x1f, 0x15, 0x15, 0x1d,  // 54 6
    0x01, 0x01, 0x1d, 0x03,  // 55 7 
    0x1f, 0x15, 0x15, 0x1f,  // 56 8
    0x07, 0x05, 0x05, 0x1f,  // 57 9
    0x00, 0x00, 0x0a, 0x00   // 58 :
};

#endif
