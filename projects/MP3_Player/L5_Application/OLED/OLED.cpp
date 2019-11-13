/*
 * OLED.cpp
 *
 *  Created on: Apr 21, 2019
 *      Author: Nick
 */

#include <OLED/OLED.h>

OLED* OLED::instance = nullptr;

static const uint8_t oled_font6x8[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // sp
        0x00, 0x00, 0x00, 0x2f, 0x00, 0x00, // !
        0x00, 0x00, 0x07, 0x00, 0x07, 0x00, // "
        0x00, 0x14, 0x7f, 0x14, 0x7f, 0x14, // #
        0x00, 0x24, 0x2a, 0x7f, 0x2a, 0x12, // $
        0x00, 0x62, 0x64, 0x08, 0x13, 0x23, // %
        0x00, 0x36, 0x49, 0x55, 0x22, 0x50, // &
        0x00, 0x00, 0x05, 0x03, 0x00, 0x00, // '
        0x00, 0x00, 0x1c, 0x22, 0x41, 0x00, // (
        0x00, 0x00, 0x41, 0x22, 0x1c, 0x00, // )
        0x00, 0x14, 0x08, 0x3E, 0x08, 0x14, // *
        0x00, 0x08, 0x08, 0x3E, 0x08, 0x08, // +
        0x00, 0x00, 0x00, 0xA0, 0x60, 0x00, // ,
        0x00, 0x08, 0x08, 0x08, 0x08, 0x08, // -
        0x00, 0x00, 0x60, 0x60, 0x00, 0x00, // .
        0x00, 0x20, 0x10, 0x08, 0x04, 0x02, // /
        0x00, 0x3E, 0x51, 0x49, 0x45, 0x3E, // 0
        0x00, 0x00, 0x42, 0x7F, 0x40, 0x00, // 1
        0x00, 0x42, 0x61, 0x51, 0x49, 0x46, // 2
        0x00, 0x21, 0x41, 0x45, 0x4B, 0x31, // 3
        0x00, 0x18, 0x14, 0x12, 0x7F, 0x10, // 4
        0x00, 0x27, 0x45, 0x45, 0x45, 0x39, // 5
        0x00, 0x3C, 0x4A, 0x49, 0x49, 0x30, // 6
        0x00, 0x01, 0x71, 0x09, 0x05, 0x03, // 7
        0x00, 0x36, 0x49, 0x49, 0x49, 0x36, // 8
        0x00, 0x06, 0x49, 0x49, 0x29, 0x1E, // 9
        0x00, 0x00, 0x36, 0x36, 0x00, 0x00, // :
        0x00, 0x00, 0x56, 0x36, 0x00, 0x00, // ;
        0x00, 0x08, 0x14, 0x22, 0x41, 0x00, // <
        0x00, 0x14, 0x14, 0x14, 0x14, 0x14, // =
        0x00, 0x00, 0x41, 0x22, 0x14, 0x08, // >
        0x00, 0x02, 0x01, 0x51, 0x09, 0x06, // ?
        0x00, 0x32, 0x49, 0x59, 0x51, 0x3E, // @
        0x00, 0x7C, 0x12, 0x11, 0x12, 0x7C, // A
        0x00, 0x7F, 0x49, 0x49, 0x49, 0x36, // B
        0x00, 0x3E, 0x41, 0x41, 0x41, 0x22, // C
        0x00, 0x7F, 0x41, 0x41, 0x22, 0x1C, // D
        0x00, 0x7F, 0x49, 0x49, 0x49, 0x41, // E
        0x00, 0x7F, 0x09, 0x09, 0x09, 0x01, // F
        0x00, 0x3E, 0x41, 0x49, 0x49, 0x7A, // G
        0x00, 0x7F, 0x08, 0x08, 0x08, 0x7F, // H
        0x00, 0x00, 0x41, 0x7F, 0x41, 0x00, // I
        0x00, 0x20, 0x40, 0x41, 0x3F, 0x01, // J
        0x00, 0x7F, 0x08, 0x14, 0x22, 0x41, // K
        0x00, 0x7F, 0x40, 0x40, 0x40, 0x40, // L
        0x00, 0x7F, 0x02, 0x0C, 0x02, 0x7F, // M
        0x00, 0x7F, 0x04, 0x08, 0x10, 0x7F, // N
        0x00, 0x3E, 0x41, 0x41, 0x41, 0x3E, // O
        0x00, 0x7F, 0x09, 0x09, 0x09, 0x06, // P
        0x00, 0x3E, 0x41, 0x51, 0x21, 0x5E, // Q
        0x00, 0x7F, 0x09, 0x19, 0x29, 0x46, // R
        0x00, 0x46, 0x49, 0x49, 0x49, 0x31, // S
        0x00, 0x01, 0x01, 0x7F, 0x01, 0x01, // T
        0x00, 0x3F, 0x40, 0x40, 0x40, 0x3F, // U
        0x00, 0x1F, 0x20, 0x40, 0x20, 0x1F, // V
        0x00, 0x3F, 0x40, 0x38, 0x40, 0x3F, // W
        0x00, 0x63, 0x14, 0x08, 0x14, 0x63, // X
        0x00, 0x07, 0x08, 0x70, 0x08, 0x07, // Y
        0x00, 0x61, 0x51, 0x49, 0x45, 0x43, // Z
        0x00, 0x00, 0x7F, 0x41, 0x41, 0x00, // [
        0x00, 0x55, 0x2A, 0x55, 0x2A, 0x55, // backslash
        0x00, 0x00, 0x41, 0x41, 0x7F, 0x00, // ]
        0x00, 0x04, 0x02, 0x01, 0x02, 0x04, // ^
        0x00, 0x40, 0x40, 0x40, 0x40, 0x40, // _
        0x00, 0x00, 0x01, 0x02, 0x04, 0x00, // '
        0x00, 0x20, 0x54, 0x54, 0x54, 0x78, // a
        0x00, 0x7F, 0x48, 0x44, 0x44, 0x38, // b
        0x00, 0x38, 0x44, 0x44, 0x44, 0x20, // c
        0x00, 0x38, 0x44, 0x44, 0x48, 0x7F, // d
        0x00, 0x38, 0x54, 0x54, 0x54, 0x18, // e
        0x00, 0x08, 0x7E, 0x09, 0x01, 0x02, // f
        0x00, 0x18, 0xA4, 0xA4, 0xA4, 0x7C, // g
        0x00, 0x7F, 0x08, 0x04, 0x04, 0x78, // h
        0x00, 0x00, 0x44, 0x7D, 0x40, 0x00, // i
        0x00, 0x40, 0x80, 0x84, 0x7D, 0x00, // j
        0x00, 0x7F, 0x10, 0x28, 0x44, 0x00, // k
        0x00, 0x00, 0x41, 0x7F, 0x40, 0x00, // l
        0x00, 0x7C, 0x04, 0x18, 0x04, 0x78, // m
        0x00, 0x7C, 0x08, 0x04, 0x04, 0x78, // n
        0x00, 0x38, 0x44, 0x44, 0x44, 0x38, // o
        0x00, 0xFC, 0x24, 0x24, 0x24, 0x18, // p
        0x00, 0x18, 0x24, 0x24, 0x18, 0xFC, // q
        0x00, 0x7C, 0x08, 0x04, 0x04, 0x08, // r
        0x00, 0x48, 0x54, 0x54, 0x54, 0x20, // s
        0x00, 0x04, 0x3F, 0x44, 0x40, 0x20, // t
        0x00, 0x3C, 0x40, 0x40, 0x20, 0x7C, // u
        0x00, 0x1C, 0x20, 0x40, 0x20, 0x1C, // v
        0x00, 0x3C, 0x40, 0x30, 0x40, 0x3C, // w
        0x00, 0x44, 0x28, 0x10, 0x28, 0x44, // x
        0x00, 0x1C, 0xA0, 0xA0, 0xA0, 0x7C, // y
        0x00, 0x44, 0x64, 0x54, 0x4C, 0x44, // z
        0x00, 0x00, 0x08, 0x77, 0x41, 0x00, // {
        0x00, 0x00, 0x00, 0x63, 0x00, 0x00, // ¦
        0x00, 0x00, 0x41, 0x77, 0x08, 0x00, // }
        0x00, 0x08, 0x04, 0x08, 0x08, 0x04, // ~
        0x00, 0x3D, 0x40, 0x40, 0x20, 0x7D, // ü
        0x00, 0x3D, 0x40, 0x40, 0x40, 0x3D, // Ü
        0x00, 0x21, 0x54, 0x54, 0x54, 0x79, // ä
        0x00, 0x7D, 0x12, 0x11, 0x12, 0x7D, // Ä
        0x00, 0x39, 0x44, 0x44, 0x44, 0x39, // ö
        0x00, 0x3D, 0x42, 0x42, 0x42, 0x3D, // Ö
        0x00, 0x02, 0x05, 0x02, 0x00, 0x00, // °
        0x00, 0x7E, 0x01, 0x49, 0x55, 0x73, // ß
        };

OLED::OLED(uint8_t i2c_address, uint8_t width, uint8_t height)
{
    OLED::i2c_address = i2c_address;
    OLED::height = height;
    OLED::width = width;
    OLED::pages = (height + 7) / 8;
    OLED::bufsize = OLED::pages * width;
    OLED::screen_buffer = (uint8_t *) malloc(OLED::bufsize);
    OLED::X = OLED::Y = 0;
    OLED::ttyMode = false //OLED_DEFAULT_TTY_MODE
    ;
#ifdef DEBUG_OLED
    printf("address: %X, height: %u, width: %u, pages: %u, bufsize: %u, screenbuffer: %X\n", OLED::i2c_address, OLED::height, OLED::width, OLED::pages,
            OLED::bufsize, OLED::screen_buffer);
#endif
}

void OLED::init()
{
    delay_ms(100);
    sendCommand(0xAE);
    sendCommand(0x20);
    sendCommand(0x10);
    sendCommand(0xB0);
    sendCommand(0xC8);
    sendCommand(0x00);
    sendCommand(0x10);
    sendCommand(0x40);
    sendCommand(0x81);
    sendCommand(0x7F);
    sendCommand(0xA1);
    sendCommand(0xA6);
    sendCommand(0xA8);
    sendCommand(0x3F);
    sendCommand(0xA4);
    sendCommand(0xD3);
    sendCommand(0x00);
    sendCommand(0xD5);
    sendCommand(0xF0);
    sendCommand(0xD9);
    sendCommand(0x22);
    sendCommand(0xDA);
    sendCommand(0x12);
    sendCommand(0xDB);
    sendCommand(0x20);
    sendCommand(0x8D);
    sendCommand(0x14);
    sendCommand(0xAF);
    delay_ms(100);
}

void OLED::display()
{
    uint16_t index = 0;
    I2C2& oled = I2C2::getInstance();
    for (uint_fast8_t page = 0; page < pages; page++) {
        // Set memory address to fill

        uint8_t trans[] = { (uint8_t) (0xB0 + page), 0x00, 0x10
        //(uint8_t)(0xB0 + page), 0x21, 0x00, (uint8_t)(width - 1)
                };

        oled.writeRegisters(i2c_address, 0x00, trans, sizeof(trans));
        oled.writeRegisters(i2c_address, 0x40, (screen_buffer + index), width * sizeof(uint8_t));
        index += width;
    }
}

void OLED::clear(tColor color)
{
    if (color == WHITE) {
        memset(screen_buffer, 0xFF, bufsize);
    }
    else {
        memset(screen_buffer, 0x00, bufsize);
    }
    X = 0;
    Y = 0;
}

void OLED::draw_bytes(uint_fast8_t x, uint_fast8_t y, const uint8_t* data, uint_fast8_t size, tSize scaling, tColor color)
{
    for (uint_fast8_t column = 0; column < size; column++) {
        uint8_t b;
        b = *data;

        data++;
        if (scaling == DOUBLE_SIZE) {
            // Stretch vertically
            uint16_t w = 0;
            for (uint_fast8_t bit = 0; bit < 7; bit++) {
                if (b & (1 << bit)) {
                    uint_fast8_t pos = bit << 1;
                    w |= ((1 << pos) | (1 << (pos + 1)));
                }
            }

            // Output 2 times to stretch horizontally
            draw_byte(x, y, w & 0xFF, color);
            draw_byte(x, y + 8, (w >> 8), color);
            x++;
            draw_byte(x, y, w & 0xFF, color);
            draw_byte(x, y + 8, (w >> 8), color);
            x++;
        }
        else // NORMAL_SIZE
        {
            draw_byte(x++, y, b, color);
        }
    }
}

void OLED::draw_bitmap(uint_fast8_t x, uint_fast8_t y, uint_fast8_t width, uint_fast8_t height, const uint8_t* data, tColor color)
{
}

void OLED::draw_bitmap_P(uint_fast8_t x, uint_fast8_t y, uint_fast8_t width, uint_fast8_t height, const uint8_t* data, tColor color)
{
}

size_t OLED::draw_character(uint_fast8_t x, uint_fast8_t y, char c, tSize scaling, tColor color)
{
    // Invalid position
    if (x >= width || y >= height || c < 32) {
#ifdef OLED_DEBUG
        puts("draw_character invalid position");
#endif
        return 0;
    }

    // Remap extended Latin1 character codes
    switch ((unsigned char) c) {
        case 252 /* u umlaut */:
            c = 127;
            break;
        case 220 /* U umlaut */:
            c = 128;
            break;
        case 228 /* a umlaut */:
            c = 129;
            break;
        case 196 /* A umlaut */:
            c = 130;
            break;
        case 246 /* o umlaut */:
            c = 131;
            break;
        case 214 /* O umlaut */:
            c = 132;
            break;
        case 176 /* degree   */:
            c = 133;
            break;
        case 223 /* szlig    */:
            c = 134;
            break;
    }

    uint16_t font_index = (c - 32) * 6;

    // Invalid character code/font index
    if (font_index >= sizeof(oled_font6x8)) {
#ifdef OLED_DEBUG
        puts("draw_byte character code/font index");
#endif
        return 0;
    }

    draw_bytes(x, y, &oled_font6x8[font_index], 6, scaling, color);
    return 1;
}

void OLED::draw_string(uint_fast8_t x, uint_fast8_t y, const char* s, tSize scaling, tColor color)
{
    while (*s) {
        draw_character(x, y, *s, scaling, color);
        if (scaling == DOUBLE_SIZE) {
            x += 12;
        }
        else // NORMAL_SIZE
        {
            x += 6;
        }
        s++;
    }
}

void OLED::draw_string_P(uint_fast8_t x, uint_fast8_t y, const char* s, tSize scaling, tColor color)
{
}

void OLED::draw_rectangle(uint_fast8_t x0, uint_fast8_t y0, uint_fast8_t x1, uint_fast8_t y1, tFillmode fillMode, tColor color)
{
    // Swap x0 and x1 if in wrong order
        if (x0 > x1)
        {
            uint_fast8_t tmp = x0;
            x0 = x1;
            x1 = tmp;
        }
        // Swap y0 and y1 if in wrong order
        if (y0 > y1)
        {
            uint_fast8_t tmp = y0;
            y0 = y1;
            y1 = tmp;
        }
        if (fillMode == SOLID)
        {
            for (uint_fast8_t y = y0; y <= y1; y++)
            {
                draw_line(x0, y, x1, y, color);
            }
        }
        else
        {
            draw_line(x0, y0, x1, y0, color);
            draw_line(x0, y1, x1, y1, color);
            draw_line(x0, y0, x0, y1, color);
            draw_line(x1, y0, x1, y1, color);
        }
}

void OLED::draw_circle(uint_fast8_t x, uint_fast8_t y, uint_fast8_t radius, tFillmode fillMode, tColor color)
{
}

void OLED::draw_line(uint_fast8_t x0, uint_fast8_t y0, uint_fast8_t x1, uint_fast8_t y1, tColor color)
{
    // Algorithm copied from Wikipedia
    int_fast16_t dx = abs(static_cast<int_fast16_t>(x1) - static_cast<int_fast16_t>(x0));
    int_fast16_t sx = x0 < x1 ? 1 : -1;
    int_fast16_t dy = -abs(static_cast<int_fast16_t>(y1) - static_cast<int_fast16_t>(y0));
    int_fast16_t sy = y0 < y1 ? 1 : -1;
    int_fast16_t err = dx + dy;
    int_fast16_t e2;

    while (1) {
        draw_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) {
            break;
        }
        e2 = 2 * err;
        if (e2 > dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void OLED::draw_pixel(uint_fast8_t x, uint_fast8_t y, tColor color)
{
    if (x >= width || y >= height) {
        return;
    }
    if (color == WHITE) {
        screen_buffer[x + (y / 8) * width] |= (1 << (y & 7)); // set bit
    }
    else {
        screen_buffer[x + (y / 8) * width] &= ~(1 << (y & 7)); // clear bit
    }
}

void OLED::set_invert(bool enable)
{
        sendCommand(0x00); // command
        if (enable)
        {
            sendCommand(0xA7); // invert display
        }
        else
        {
            sendCommand(0xA6); // normal display
        }
}

void OLED::set_contrast(uint8_t contrast)
{
}

void OLED::set_scrolling(tScrollEffect scroll_type, uint_fast8_t first_page, uint_fast8_t last_page)
{
}

void OLED::scroll_up(uint_fast8_t num_lines, uint_fast8_t delay_ms)
{
    if (delay_ms == 0) {
        // Scroll full pages, fast algorithm
        uint_fast8_t scroll_pages = num_lines / 8;
        for (uint_fast8_t i = 0; i < pages; i++) {
            for (uint_fast8_t x = 0; x < width; x++) {
                uint16_t index = i * width + x;
                uint16_t index2 = (i + scroll_pages) * width + x;
                if (index2 < bufsize) {
                    screen_buffer[index] = screen_buffer[index2];
                }
                else {
                    screen_buffer[index] = 0;
                }
            }
        }
        num_lines -= scroll_pages * 8;
    }

    // Scroll the remainder line by line
    bool need_refresh = true;
    if (num_lines > 0) {
        uint16_t start = xTaskGetMsCount() & 0xFFFF;
        uint16_t target_time = 0;

        for (uint_fast8_t i = 0; i < num_lines; i++) {

            // Scroll everything 1 line up
            for (uint_fast8_t j = 0; j < pages; j++) {
                uint16_t index = j * width;
                uint16_t index2 = index + width;
                for (uint_fast8_t x = 0; x < width; x++) {
                    uint_fast8_t carry = 0;
                    if (index2 < bufsize) {
                        if (screen_buffer[index2] & 1) {
                            carry = 128;
                        }
                    }
                    screen_buffer[index] = (screen_buffer[index] >> 1) | carry;
                    index++;
                    index2++;
                }
            }
            need_refresh = true;
            target_time += delay_ms;

            // Refresh the display only if we have some time
            uint16_t now = xTaskGetMsCount() & 0xFFFF;
            if (now - start < target_time) {
                display();
                need_refresh = false;
            }

            // If we have still more time, then yield a while
            while ((xTaskGetMsCount() & 0xFFFF) - start < target_time) {
                //yield();
            }
        }
    }

    if (need_refresh) {
        display();
    }
}

size_t OLED::oprintf(uint_fast8_t x, uint_fast8_t y, const char *format, ...)
{
    //TODO printf
    va_list arg;
    va_start(arg, format);
    char temp[64];
    char* buffer = temp;
    size_t len = vsnprintf(temp, sizeof(temp), format, arg);
    va_end(arg);
    if (len > sizeof(temp) - 1) {
        buffer = new char[len + 1];
        if (!buffer) {
            return 0;
        }
        va_start(arg, format);
        vsnprintf(buffer, len + 1, format, arg);
        va_end(arg);
    }
    X = x;
    Y = y;
    len = write((const uint8_t*) buffer, len);

    if (buffer != temp) {
        delete[] buffer;
    }

    return len;
    //return 0;
}

size_t OLED::write(const uint8_t* buffer, size_t len)
{
    //TODO write
    for (size_t ix = 0; ix < len; ix++) {
        // If CR and LF are found consecutive, only one is processed
        // If two or more CR are consecutive, both are processed
        // If two or more LF are consecutive, both are processed
        if (buffer[ix] == '\r') {
            //Serial.printf("n=%d r",ix);
            X = 3;
            Y += (OLED_FONT_HEIGHT);
            if (buffer[ix + 1] == '\n') {
                //Serial.print(" + n");Serial.println();
                ix++; // skip char
            }
            //Serial.printf("n=%d",ix);
            //Serial.println();
        }
        else if (buffer[ix] == '\n') {
            //Serial.printf("n=%d n",ix);
            X = 3;
            Y += (OLED_FONT_HEIGHT);
            if (buffer[ix + 1] == '\r') {
                //Serial.print(" + r");Serial.println();
                ix++; // skip char
            }
            //Serial.printf("n=%d",ix);
            //Serial.println();
        }
        else if (*(buffer + ix) == '\f') {
            // FORM FEED
            scroll_up(height);
            X = 3;
            Y = 0;
        }
        else {
            write(buffer[ix]);
        }

        if (ttyMode) {
            // Scroll up if cursor position is out of screen
            if (Y >= height) {
                scroll_up(OLED_FONT_HEIGHT);
                Y = height - OLED_FONT_HEIGHT;
            }
        }
    }
    if (ttyMode) display();
    return len;
//    return 0;
}

void OLED::setCursor(uint_fast8_t x, uint_fast8_t y)
{
    //TODO setCursor
    X = x;
    Y = x;
}

bool OLED::sendCommand(uint8_t command)
{
    I2C2& oled = I2C2::getInstance();
    return oled.writeReg(i2c_address, 0x80, command);
}

bool OLED::power_on()
{
    return sendCommand(0xA5);
}

bool OLED::power_off()
{
    return sendCommand(0xA4);
}

size_t OLED::write(uint8_t c)
{
    //TODO write
    int n = 1;
    n = draw_character(X, Y, c);
    X += OLED_FONT_WIDTH;
    /*
     }
     */
    return n;
    //return 0;
}

OLED::~OLED()
{
    free(screen_buffer);
}

void OLED::draw_byte(uint_fast8_t x, uint_fast8_t y, uint8_t b, tColor color)
{
// Invalid position
    if (x >= width || y >= height) {
#ifdef OLED_DEBUG
        puts("draw_byte invalid position");
#endif
        return;
    }

    uint_fast16_t buffer_index = y / 8 * width + x;

    if (color == WHITE) {
        // If the y position matches a page, then it goes quicker
        if (y % 8 == 0) {
            if (buffer_index < bufsize) {
                screen_buffer[buffer_index] |= b;
            }
        }
        else {
            uint16_t w = (uint16_t) b << (y % 8);
            if (buffer_index < bufsize) {
                screen_buffer[buffer_index] |= (w & 0xFF);
            }
            uint16_t buffer_index2 = buffer_index + width;
            if (buffer_index2 < bufsize) {
                screen_buffer[buffer_index2] |= (w >> 8);
            }
        }
    }
    else {
        // If the y position matches a page, then it goes quicker
        if (y % 8 == 0) {
            if (buffer_index < bufsize) {
                screen_buffer[buffer_index] &= ~b;
            }
        }
        else {
            uint16_t w = (uint16_t) b << (y % 8);
            if (buffer_index < bufsize) {
                screen_buffer[buffer_index] &= ~(w & 0xFF);
            }
            uint16_t buffer_index2 = buffer_index + width;
            if (buffer_index2 < bufsize) {
                screen_buffer[buffer_index2] &= ~(w >> 8);
            }
        }
    }
    return;
}

OLED* OLED::getInstance()
{
    if (instance == nullptr) instance = new OLED();
    return instance;

}
