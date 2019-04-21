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

char buffer[BUFSIZ + 1];
std::vector<FILINFO> files;

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


void OLEDInit(){
    auto oled = I2C2::getInstance();
    bool ok = oled.writeReg(0x78, 0x00, 0xAE);
    //oled.writeReg();
    //printf("ok? %d\n", ok);

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

int main(void)
{
    scheduler_add_task(new terminalTask(PRIORITY_HIGH));
    //OLEDInit();

    //xTaskCreate(vDirRead, "DirRead", 1024, NULL, PRIORITY_LOW, NULL);

    scheduler_start();
    return -1;
}
