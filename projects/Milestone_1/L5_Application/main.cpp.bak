/*
 *  Nickolas Schiffer
 *  CMPE 146 S19
 *  MP3 Project Milestone 1 (OLED & Filename enumeration)
 */

/**
 * @file
 * @brief This is the application entry point.
 */

#include <stdio.h>
#include "utilities.h"
#include "io.hpp"
#include <tasks.hpp>
#include <ff.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <storage.hpp>
#include <i2c2.hpp>
#include <OLED/OLED.h>

#define OLED_DEBUG

char buffer[BUFSIZ + 1];
std::vector<FILINFO> files;

//uint8_t addr = 0xF0;
uint8_t addr = 0x78;

static const uint8_t oled_font6x8[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // sp
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

FRESULT scan_files(char* path /* Start node to be scanned (***also used as work area***) */
)
{
    FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;

    res = f_opendir(&dir, path); /* Open the directory */
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno); /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break; /* Break on error or end of dir */
            if (fno.fattrib & AM_DIR) { /* It is a directory */
                i = strlen(path);
                sprintf(&path[i], "/%s", fno.fname);
                res = scan_files(path); /* Enter the directory */
                if (res != FR_OK) break;
                path[i] = 0;
            }
            else { /* It is a file. */
                printf("%s/%s\n", path, fno.fname);
            }
        }
        f_closedir(&dir);
    }

    return res;
}

FRESULT getFilesFromSD(std::vector<FILINFO> *filinfo_vec)
{
    DIR Dir;
    FILINFO Finfo;
    FATFS *fs;
    FRESULT returnCode = FR_OK;

    unsigned int fileBytesTotal = 0, numFiles = 0, numDirs = 0;
#if _USE_LFN
    char Lfname[_MAX_LFN];
#endif
    const char *dirPath = "1:";
    if (FR_OK != (returnCode = f_opendir(&Dir, dirPath))) {
        return FR_NO_PATH;
    }

#if 0
    // Offset the listing
    while(lsOffset-- > 0) {
#if _USE_LFN
        Finfo.lfname = Lfname;
        Finfo.lfsize = sizeof(Lfname);
#endif
        if (FR_OK != f_readdir(&Dir, &Finfo)) {
            break;
        }
    }
#endif

    for (;;) {
#if _USE_LFN
        Finfo.lfname = Lfname;
        Finfo.lfsize = sizeof(Lfname);
#endif

        returnCode = f_readdir(&Dir, &Finfo);
        if ((FR_OK != returnCode) || !Finfo.fname[0]) {
            break;
        }

        if (Finfo.fattrib & AM_DIR) {
            numDirs++;
        }
        else {
            numFiles++;
            fileBytesTotal += Finfo.fsize;
            filinfo_vec->push_back(Finfo);

        }
    }

    return FR_OK;
}

enum tColor { BLACK, WHITE };

uint8_t width = 128, height = 64;
uint16_t pages = (height + 7) / 8;
uint16_t bufsize = pages * width;
uint8_t *screen_buffer = (uint8_t *)malloc(bufsize);
uint8_t X = 0, Y = 0;
//uint16_t bufsize = size;
void draw_byte(uint_fast8_t x, uint_fast8_t y, uint8_t b, tColor color)
{
    // Invalid position
    if (x >= width || y >= height)
    {
#ifdef OLED_DEBUG
        puts("draw_byte invalid position");
#endif
        return;
    }

    uint_fast16_t buffer_index = y / 8 * width + x;

    if (color == WHITE)

    {
        // If the y position matches a page, then it goes quicker
        if (y % 8 == 0)
        {
            if (buffer_index < bufsize)
            {
                screen_buffer[buffer_index] |= b;
            }
        }
        else
        {
            uint16_t w = (uint16_t) b << (y % 8);
            if (buffer_index < bufsize)
            {
                screen_buffer[buffer_index] |= (w & 0xFF);
            }
            uint16_t buffer_index2 = buffer_index + width;
            if (buffer_index2 < bufsize)
            {
                screen_buffer[buffer_index2] |= (w >> 8);
            }
        }
    }
    else
    {
        // If the y position matches a page, then it goes quicker
        if (y % 8 == 0)
        {
            if (buffer_index < bufsize)
            {
                screen_buffer[buffer_index] &= ~b;
            }
        }
        else
        {
            uint16_t w = (uint16_t) b << (y % 8);
            if (buffer_index < bufsize)
            {
                screen_buffer[buffer_index] &= ~(w & 0xFF);
            }
            uint16_t buffer_index2 = buffer_index + width;
            if (buffer_index2 < bufsize)
            {
                screen_buffer[buffer_index2] &= ~(w >> 8);
            }
        }
    }
    return;
}


void display()
{
    uint16_t index = 0;
    I2C2& oled = I2C2::getInstance();
    for (uint_fast8_t page = 0; page < pages; page++)
    {
        // Set memory address to fill

        uint8_t trans[] = {
                (uint8_t)(0xB0 + page), 0x00, 0x10
                //(uint8_t)(0xB0 + page), 0x21, 0x00, (uint8_t)(width - 1)
        };

        oled.writeRegisters(addr, 0x00, trans, sizeof(trans));
//        i2c_start();
//        i2c_send(i2c_address << 1); // address + write
//        i2c_send(0x00); // command
//
//            i2c_send(0xB0 + page); // set page
//            i2c_send(0x00); // lower columns address =0
//            i2c_send(0x10); // upper columns address =0
//
//        i2c_stop();

        // send one page of buffer to the display
//        i2c_start();
//        i2c_send(i2c_address << 1); // address + write
//        i2c_send(0x40); // data
//        for (uint_fast8_t column = 0; column < width; column++)
//        {
//            i2c_send(buffer[index++]);
//        }
//        i2c_stop();
//        yield(); // to avoid that the watchdog triggers
        //printf("page %d: %X size: %d\n", page, *trans, sizeof(trans));
        oled.writeRegisters(addr, 0x40, (screen_buffer + index), width * sizeof(uint8_t));
        index += width;
    }
}

bool sendCommand(uint8_t command){
    I2C2& oled = I2C2::getInstance();

    return oled.writeReg(addr, 0x80, command);
}

void OLEDInit2(){
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

bool power_on(){
    return sendCommand(0xA5);
}

bool power_off(){
    return sendCommand(0xA4);
}

void clear(tColor color)
{
    if (color == WHITE)
    {
        memset(screen_buffer, 0xFF, bufsize);
    }
    else
    {
        memset(screen_buffer, 0x00, bufsize);
    }
    X=0;
    Y=0;
}

void OLEDInit(){

    uint8_t init_buf[] = {
            //0x00,
            0xAE, 0xD5, 0x80, 0xA8,
            (uint8_t)(height - 1), 0xD3, 0x00, 0x40,
            0x8D, 0x14, 0x20, 0x00,
            0xA1, 0x38, 0xDA, 0x12,
            0x81, 0x80, 0xD9, 0x22,
            0xDB, 0x20, 0xA4, 0xA7,
            0x2E

    };

    uint8_t init_buf2[] = {
            0xAE, 0x02, 0x10, 0x40,
            0xB0, 0x81, 0x80, 0xA1,
            0xA6, 0xA8, 0x3F, 0xAD,
            0x8B, 0x33, 0xC8, 0xD3,
            0x00, 0xD5, 0x80, 0xD9,
            0x1F, 0xDA, 0x12, 0xDB,
            0x40, 0xAF
    };

    uint8_t power_on[] = {
        0x8D, 0x14, 0xAF
    };
    auto oled = I2C2::getInstance();
    oled.init(400);

    //bool ok  = oled.writeRegisters(addr, 0x00, init_buf2, sizeof(init_buf2));
    bool ok  = oled.writeRegisters(addr, 0x00, init_buf, sizeof(init_buf));
    delay_ms(100);
    memset(screen_buffer, 0x00, bufsize);
    oled.writeRegisters(addr, 0x40, screen_buffer, bufsize);
    bool ok1 = oled.writeRegisters(addr, 0x00, power_on, sizeof(power_on));
    delay_ms(100);
    //oled.writeReg();
//    printf("ok? %d\n", ok);
//    printf("ok1? %d\n", ok1);

}

void vDirRead(void *pvParameters)
{
    while (1) {
        FRESULT res = getFilesFromSD(&files);
        if (res == FR_OK) {
            printf("successfully read %d files\n", files.size());
            for (FILINFO file : files) {
                printf("file name: %s\n", file.fname);
                printf("file size: %lu\n", file.fsize);
                printf("file name long: %s\n\n\n", file.lfname);
            }

        }
        else {
            puts("file read error");
        }
        files.clear();
        vTaskDelay(2000);
    }

}

enum tSize { NORMAL_SIZE, DOUBLE_SIZE };
void draw_bytes(uint_fast8_t x, uint_fast8_t y, const uint8_t* data, uint_fast8_t size, tSize scaling, tColor color, bool useProgmem)
{
    for (uint_fast8_t column = 0; column < size; column++)
    {
        uint8_t b;
//        if (useProgmem)
//        {
//            b = pgm_read_byte(data);
//        }
//        else
        //{
            b = *data;
        //}
        data++;
        if (scaling == DOUBLE_SIZE)
        {
            // Stretch vertically
            uint16_t w = 0;
            for (uint_fast8_t bit = 0; bit < 7; bit++)
            {
                if (b & (1 << bit))
                {
                    uint_fast8_t pos = bit << 1;
                    w |= ((1 << pos) | (1 << (pos + 1)));
                }
            }

            // Output 2 times to strech hozizontally
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

size_t draw_character(uint_fast8_t x, uint_fast8_t y, char c, tSize scaling, tColor color)
{
    // Invalid position
    if (x >= width || y >= height || c < 32)
    {
#ifdef OLED_DEBUG
        puts("draw_character invalid position");
#endif
        return 0;
    }

    // Remap extended Latin1 character codes
    switch ((unsigned char) c)
    {
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

    uint16_t font_index = (c - 32)*6;

    // Invalid character code/font index
    if (font_index >= sizeof (oled_font6x8))
    {
#ifdef OLED_DEBUG
        puts("draw_byte character code/font index");
#endif
        return 0;
    }

    draw_bytes(x, y, &oled_font6x8[font_index], 6, scaling, color, true);
    return 1;
}

void draw_string(uint_fast8_t x, uint_fast8_t y, const char* s, tSize scaling, tColor color)
{
    while (*s)
    {
        draw_character(x, y, *s, scaling, color);
        if (scaling == DOUBLE_SIZE)
        {
            x += 12;
        }
        else // NORMAL_SIZE
        {
            x += 6;
        }
        s++;
    }
}

void vToggle(void * pvParameters){
    uint8_t val = 0x00;
    //I2C2& oled = I2C2::getInstance();
    //oled.init(100);
    //OLEDInit2();
    while(1){

        val = ~val;
        memset(screen_buffer, val, bufsize);
        display();
        //printf("toggled, val: %X\n", val);
//        if (oled.checkDeviceResponse(0x78)){
//            //puts("device responding");
//        }
//        else {
//            //puts("device not responding");
//        }
        vTaskDelay(10);
    }
}

void vPrintChars(void *pvParameters){
    uint16_t xPos, yPos;
    char sample_string[] = "Hello There";
    uint8_t string_iter = 0;
    xPos = yPos = 0;
    while (1){
        draw_character(xPos++ % width, yPos++ % height, sample_string[string_iter++ % sizeof(sample_string)], tSize::NORMAL_SIZE, tColor::WHITE);
        printf("xPos: %d, yPos: %d, char %c\n", (xPos % width), (yPos % height), sample_string[string_iter % sizeof(sample_string)]);
        vTaskDelay(1000);
    }
}

void vDrawString(void *pvParameters){
    const char test_string[] = "Hello There.\0";
//    clear(tColor::BLACK);
//    display();
//    vTaskDelay(500);
//    draw_string(0,0, test_string, tSize::NORMAL_SIZE, tColor::WHITE);
    display();
    while (1){
        clear(tColor::BLACK);
        display();
        vTaskDelay(500);
        draw_string(10,0, test_string, tSize::NORMAL_SIZE, tColor::WHITE);
        display();

        vTaskDelay(1000);
    }
}

void vClassTest(void *pvParamters){
    const char test_string[] = "Hello There.\0";
    //auto *oled = new OLED();
    auto oled = OLED::getInstance();
    oled->init();
    oled->clear();
    oled->display();
    puts("initialized");
    printf("size: %d\n", sizeof(oled));
    while (1){
        oled->clear();
        oled->display();
        vTaskDelay(500);
        oled->draw_string(10,0, test_string, OLED::tSize::NORMAL_SIZE, OLED::tColor::WHITE);
       oled->display();
//
        vTaskDelay(1000);
    }

}

void vWriteTest(void *pvParameters){
    auto oled = OLED::getInstance();
    oled->init();
    oled->clear();
    oled->display();
    const char string1[] = "Hello 1";
    const char string2[] = "Hello 2";
    const char string3[] = "Hello 3";
    const char string4[] = "Hello 4";
    while(1){
        oled->write((const unsigned char *)string1, strlen(string1));
        oled->display();
        vTaskDelay(100);
        oled->write((const unsigned char *)string2, strlen(string2));
        oled->display();
        vTaskDelay(100);
        oled->write((const unsigned char *)string3, strlen(string3));
        oled->display();
        vTaskDelay(100);
        oled->write((const unsigned char *)string4, strlen(string4));
        oled->display();
        oled->clear();
        oled->display();
    }
}

void vPrintFTest(void * pvParameters){
    auto oled = OLED::getInstance();
    oled->init();
    oled->clear();
    oled->display();



    while (1){
        for (uint8_t i = 0; i < 25; i++){
            oled->oprintf(10, i * 8, " Hello %d", i);
            oled->display();
            vTaskDelay(500);
        }
        //oled->oprintf(10, 0, "");
        //oled->display();
        //vTaskDelay(20000);

        oled->clear();
        oled->display();
    }
}


int main(void)
{
    scheduler_add_task(new terminalTask(PRIORITY_HIGH));
    //OLEDInit2();
    //power_on();
//    auto oled = I2C2::getInstance();
//    oled.init(400);
//    draw_byte(10, 10, 0xFF, tColor::WHITE);
//    draw_byte(20, 20, 0xFF, tColor::BLACK);
//    draw_byte(30, 30, 0x00, tColor::WHITE);
//    draw_byte(40, 40, 0x00, tColor::BLACK);



//    memset(screen_buffer, 0xFF, bufsize);
//    display();

//    Storage::write("1:screen_buffer.txt", screen_buffer, bufsize, 0);

    //xTaskCreate(vDirRead, "DirRead", 1024, NULL, PRIORITY_LOW, NULL);
    //xTaskCreate(vToggle, "Toggle", 1024, NULL, PRIORITY_LOW, NULL);
    //xTaskCreate(vPrintChars, "PrintChars", 1024, NULL, PRIORITY_LOW, NULL);
    //xTaskCreate(vDrawString, "DrawString", 1024, NULL, PRIORITY_LOW, NULL);
    //xTaskCreate(vClassTest, "ClassTest", 3000, NULL, PRIORITY_LOW, NULL);
    //xTaskCreate(vWriteTest, "WriteTest", 3000, NULL, PRIORITY_LOW, NULL);
    xTaskCreate(vPrintFTest, "PrintfTest", 3000, NULL, PRIORITY_LOW, NULL);


    scheduler_start();
    return -1;
}
