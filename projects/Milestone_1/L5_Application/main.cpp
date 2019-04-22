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

char buffer[BUFSIZ + 1];
std::vector<FILINFO> files;

//uint8_t addr = 0xF0;
uint8_t addr = 0x78;

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

uint8_t width = 134, height = 64;
uint16_t pages = (height + 7) / 8;
uint16_t size = pages * width;
uint8_t *screen_buffer = (uint8_t *)malloc(size);
uint16_t bufsize = size;
void draw_byte(uint_fast8_t x, uint_fast8_t y, uint8_t b, tColor color)
{
    // Invalid position
    if (x >= width || y >= height)
    {
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
    for (uint_fast8_t page = 0; page < pages; page++)
    {
        // Set memory address to fill

        uint8_t trans[] = {
                (uint8_t)(0xB0 + page), 0x00, 0x10
        };
        auto oled = I2C2::getInstance();
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
        oled.writeRegisters(addr, 0x40, screen_buffer + index, width * sizeof(uint8_t));
        index += width;
    }
}

bool sendCommand(uint8_t command){
    auto oled = I2C2::getInstance();

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

void vToggle(void * pvParameters){
    uint8_t val = 0x00;
    auto oled = I2C2::getInstance();
    while(1){
        val = ~val;
        memset(screen_buffer, val, bufsize);
        display();
        //printf("toggled, val: %X\n", val);
        if (oled.checkDeviceResponse(0x78)){
            //puts("device responding");
        }
        else {
            //puts("device not responding");
        }
        vTaskDelay(1000);
    }
}

int main(void)
{
    scheduler_add_task(new terminalTask(PRIORITY_HIGH));
    OLEDInit2();
    power_on();
    draw_byte(10, 10, 0xFF, tColor::WHITE);
    draw_byte(20, 20, 0xFF, tColor::BLACK);
    draw_byte(30, 30, 0x00, tColor::WHITE);
    draw_byte(40, 40, 0x00, tColor::BLACK);



    memset(screen_buffer, 0xFF, bufsize);
    display();

    Storage::write("1:screen_buffer.txt", screen_buffer, bufsize, 0);

    //xTaskCreate(vDirRead, "DirRead", 1024, NULL, PRIORITY_LOW, NULL);
    //xTaskCreate(vToggle, "Toggle", 1024, NULL, PRIORITY_LOW, NULL);

    scheduler_start();
    return -1;
}
