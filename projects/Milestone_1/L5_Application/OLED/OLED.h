/*
 * OLED.h
 *
 *  Created on: Apr 21, 2019
 *      Author: Nick
 */

#ifndef OLED_H_
#define OLED_H_

#define DEBUG_OLED

#include <stddef.h>
//#include <stdint.h>
//#include <sys/_stdint.h>
#include <i2c2.hpp>
#include <stdlib.h>
#include <utilities.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/**
 * Font pixel size
 */
#define OLED_FONT_HEIGHT 8
#define OLED_FONT_WIDTH 6

/**
 * tty mode default
 */
#define OLED_DEFAULT_TTY_MODE true;

class OLED {
public:
    static OLED *getInstance();
    /** Possible colors for drawing */
       enum tColor { BLACK, WHITE };

       /** Filling mode */
       enum tFillmode { HOLLOW, SOLID };

       /** Supported text sizes. Normal=6x8 pixels, Double=12x16 pixels */
       enum tSize { NORMAL_SIZE, DOUBLE_SIZE };

       /** Scroll effects supported by the display controller, note that there is no plain vertical scrolling */
       enum tScrollEffect { NO_SCROLLING=0, HORIZONTAL_RIGHT=0x26, HORIZONTAL_LEFT=0x27, DIAGONAL_RIGHT=0x29, DIAGONAL_LEFT=0x2A };


    void init();
    void display();
    void clear(tColor color=BLACK);
    //void draw_bytes(uint_fast8_t x, uint_fast8_t y, const uint8_t* data, uint_fast8_t size, tSize scaling, tColor color);
    void draw_bitmap(uint_fast8_t x, uint_fast8_t y, uint_fast8_t width, uint_fast8_t height, const uint8_t* data, tColor color=WHITE);
    void draw_bitmap_P(uint_fast8_t x, uint_fast8_t y, uint_fast8_t width, uint_fast8_t height, const uint8_t* data, tColor color=WHITE);
    size_t draw_character(uint_fast8_t x, uint_fast8_t y, char c, tSize scaling = tSize::NORMAL_SIZE, tColor = tColor::WHITE);
    void draw_string(uint_fast8_t x, uint_fast8_t y, const char* s, tSize scaling, tColor color);
    void draw_string_P(uint_fast8_t x, uint_fast8_t y, const char* s, tSize scaling=NORMAL_SIZE, tColor color=WHITE);
    void draw_rectangle(uint_fast8_t x0, uint_fast8_t y0, uint_fast8_t x1, uint_fast8_t y1, tFillmode fillMode=HOLLOW, tColor color=WHITE);
    void draw_circle(uint_fast8_t x, uint_fast8_t y, uint_fast8_t radius, tFillmode fillMode=HOLLOW, tColor color=WHITE);
    void draw_line(uint_fast8_t x0, uint_fast8_t y0, uint_fast8_t x1, uint_fast8_t y1, tColor color=WHITE);
    void draw_pixel(uint_fast8_t x, uint_fast8_t y, tColor color=WHITE);
    void set_invert(bool enable);
    void set_contrast(uint8_t contrast);
    void set_scrolling(tScrollEffect scroll_type, uint_fast8_t first_page=0, uint_fast8_t last_page=7);
    void scroll_up(uint_fast8_t num_lines=OLED_FONT_HEIGHT, uint_fast8_t delay_ms=0);
    size_t oprintf(uint_fast8_t x, uint_fast8_t y, const char *format = NULL, ...);
    size_t write(const uint8_t *buffer, size_t len);
    void setCursor(uint_fast8_t x, uint_fast8_t y);
    bool sendCommand(uint8_t command);
    bool power_on();
    bool power_off();
    void setTTYMode(bool Enabled);
    inline size_t write(unsigned long n)
        {
            return write((uint8_t) n);
        }
    inline size_t write(long n)
        {
            return write((uint8_t) n);
        }
    inline size_t write(unsigned int n)
        {
            return write((uint8_t) n);
        }
    inline size_t write(int n)
        {
            return write((uint8_t) n);
        }
    size_t write(uint8_t c);
    virtual ~OLED();
private:
    OLED(uint8_t i2c_address = 0x78, uint8_t width = 134, uint8_t height = 64);
    uint_fast8_t width;
    uint_fast8_t height;
    uint_fast8_t i2c_address;
    uint_fast8_t pages;
    uint_fast8_t bufsize;
    uint8_t *screen_buffer;
    uint_fast8_t X;
    uint_fast8_t Y;
    bool ttyMode;
    void draw_byte(uint_fast8_t x, uint_fast8_t y, uint8_t b, tColor color);
    void draw_bytes(uint_fast8_t x, uint_fast8_t y, const uint8_t* data, uint_fast8_t size, tSize scaling, tColor color);
    static OLED *instance;
};

#endif /* OLED_H_ */
