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
#include <tuple>

typedef struct {
    FILINFO info;
    std::string lname;
} song_info_t;

std::vector<song_info_t> files;

FRESULT getFilesFromSD(std::vector<song_info_t> *filinfo_vec)
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
            std::string lname;
            lname += Finfo.lfname;
            song_info_t song = {Finfo, lname};
            filinfo_vec->push_back(song);

        }
    }

    return FR_OK;
}

void vDirRead(void *pvParameters)
{
    auto oled = OLED::getInstance();
    oled->init();
    oled->clear();
    oled->display();
    while (1) {
        FRESULT res = getFilesFromSD(&files);
        if (res == FR_OK) {
            //printf("successfully read %d files\n", files.size());
            uint8_t i = 1, j = 1;
            oled->oprintf(5, 0, "Track Listing:\n");
            oled->draw_line(0, i * 8, 134, i * 8, OLED::tColor::WHITE);
            oled->display();
            i++;
            vTaskDelay(2000);
            for (song_info_t file : files) {
                if (strcasecmp(file.lname.c_str(), "") == 0){
                    //puts("no long name");
                    char *token = strtok(file.info.fname, "-");
                    oled->draw_line(0, i * 8, 134, i * 8, OLED::tColor::WHITE);
                    oled->display();
                    i++;
                    oled->oprintf(0,i++ * 8, " #:%d", j++);
                    oled->oprintf(0,i++ * 8, " Artist:");
                    oled->oprintf(0,i++ * 8, " %s ",token);
                    token = strtok(NULL,"-");
                    oled->oprintf(0,i++ * 8, " Track:");
                    oled->oprintf(0,i++ * 8, " %s ",token);
                    oled->oprintf(0,i++ * 8,"\n");

                    //oled->oprintf(0, i * 8," #%d: %s", i, file.info.fname);
                }
                else {
                    char *token = strtok((char *)file.lname.c_str(), "-");
                    oled->draw_line(3, i * 8, 134, i * 8, OLED::tColor::WHITE);
                    oled->display();
                    i++;
                    oled->oprintf(0,i++ * 8, " #:%d", j++);
                    oled->oprintf(0,i++ * 8, " Artist:");
                    oled->oprintf(0,i++ * 8, " %s ",token);
                    token = strtok(NULL,"-");
                    oled->oprintf(0,i++ * 8, " Track:");
                    oled->oprintf(0,i++ * 8, " %s ",token);
                    oled->oprintf(0,i++ * 8,"\n");
                    //oled->oprintf(0, i * 8," #%d: %s", i, file.lname.c_str());
                }
                //i++;
                vTaskDelay(2000);
            }

        }
        else {
            puts("file read error");
            oled->oprintf(0,0, "File Read Error");
        }
        oled->clear();
        oled->display();
        files.clear();
        vTaskDelay(2000);
    }

}


int main(void){
    scheduler_add_task(new terminalTask(PRIORITY_HIGH));
    xTaskCreate(vDirRead, "DirRead", 2000,NULL,PRIORITY_LOW,NULL);
    scheduler_start();
    return -1;
}
