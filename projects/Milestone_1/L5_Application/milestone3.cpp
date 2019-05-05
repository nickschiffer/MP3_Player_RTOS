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
#include <str.hpp>
#include <stdlib.h>
#include <vector>
#include <storage.hpp>
#include <i2c2.hpp>
#include <OLED/OLED.h>
#include <tuple>
#include <MP3_DECODER/mp3decoder.h>
#include <eint.h>
#include <GPIO/GPIOInterrupt.hpp>
#include <GPIO/GPIO_0_1_2.hpp>

char term_songName[20];
mp3Decoder myPlayer;
const int BUFFERSIZE = 512;
volatile bool isPlaying;
volatile bool ff;
volatile bool stop;

TaskHandle_t xPause;
SemaphoreHandle_t xReadSemaphore;
SemaphoreHandle_t xPauseSemaphore;
QueueHandle_t xSongQueue;
QueueHandle_t xvolumeQueue;

typedef struct {
    FILINFO info;
    std::string artist;
    std::string track;
} song_t;

std::vector<song_t> songs;

void Eint3Handler(){
    GPIOInterrupt *interruptHandler = GPIOInterrupt::getInstance();
    interruptHandler->HandleInterrupt();
}

FRESULT getFilesFromSD(std::vector<song_t> *song_vec)
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
            song_t song;
            if (strcasecmp(Finfo.lfname, "") == 0){
                printf("short filename: %s\n", Finfo.fname);
                char *token = strtok(Finfo.fname, "-");
                std::string artist(token);
                song.artist = artist;
                token = strtok(NULL,".");
                std::string track(token);
                song.track = track;
                song.info = Finfo;
                song_vec->push_back(song);
            }
            else {
                printf("long filename: %s\n", Finfo.lfname);
                char *token = strtok(Finfo.lfname, "-");
                std::string artist(token);
                song.artist = artist;
                token = strtok(NULL,".");
                std::string track(token);
                song.track = track;
                song.info = Finfo;
                song_vec->push_back(song);

            }
        }
    }

    return FR_OK;
}

CMD_HANDLER_FUNC(playSong)
{

    if(cmdParams.getLen() > 12)
    {

        return false;
    }
    else
    {
        sprintf(term_songName, "1:%s", cmdParams.c_str());
        xSemaphoreGive(xReadSemaphore);
        return true;
    }
}

CMD_HANDLER_FUNC(volume)
{
    int vol = (int)cmdParams;
    xQueueSend(xvolumeQueue, &vol, portMAX_DELAY);

    return true;
}

CMD_HANDLER_FUNC(stopSong)
{
    stop = 1;
    return true;
}

CMD_HANDLER_FUNC(unpauseSong)
{
    vTaskResume(xPause);
    return true;
}

void xPauseSong( void ) {

    if(isPlaying) {
        isPlaying = 0;
    }
    else {
        xSemaphoreGive(xPauseSemaphore);
    }
}

void xFastForward( void ) {
    ff = 1;
}

void vReadSong (void *pvParameters) {

    uint8_t songBuff[BUFFERSIZE] = {0};
    FIL mySong;
    UINT readBytes;

    while(1) {
        if(xSemaphoreTake(xReadSemaphore, portMAX_DELAY)) {
           if(FR_NO_FILE == f_open(&mySong, term_songName, FA_READ)) {
               puts("File not found!");
           } else {
               readBytes = BUFFERSIZE;
               isPlaying = 1;
               ff = 0;
               stop = 0;
               while(! myPlayer.readyForData());
               while(! (readBytes < BUFFERSIZE)) {
                   if(!isPlaying) {
                       xSemaphoreTake(xPauseSemaphore, portMAX_DELAY);
                       isPlaying = 1;
                   }
                   if(ff) {
                       f_lseek(&mySong, f_tell(&mySong) + 51200);
                       ff = 0;
                   }
                   if(stop) {
                       break;
                   }
                   f_read(&mySong, songBuff, BUFFERSIZE, &readBytes);
                   xQueueSend(xSongQueue, songBuff, portMAX_DELAY);
               }
               f_close(&mySong);
           }

        }
    }
}

void vPlaySong (void *pvParameters) {

    int volume;
    uint8_t songBuff[BUFFERSIZE];
    while(1) {
        if(xQueueReceive(xvolumeQueue, &volume, 0)) {
            printf("%d\n", volume);
            int vol = 254 - (((100 * (100 - (int)volume)) + (254 * (int)volume)) / 100);
            myPlayer.setVolume(vol, vol);
        }

        xQueueReceive(xSongQueue, songBuff, portMAX_DELAY);

        myPlayer.sendData(songBuff, BUFFERSIZE);
        vTaskDelay(15);
    }
}

void vPopulateSongs(void *pvParameters){
    getFilesFromSD(&songs);

    while(1){
        puts("Populated Songs");
        int i = 0;
        for (song_t &song : songs){
            printf("Track #%d\n", i++);
            printf("Artist: %s\n", (song.artist).c_str());
            printf("Track: %s\n", (song.track).c_str());
            vTaskDelay(500);

        }
        vTaskDelay(1000);
    }
}

void vMenuScreen(void *pvParameters){
    auto oled = OLED::getInstance();
    oled->init();
    oled->clear();
    oled->display();
    unsigned int selected_song = 0;
    unsigned int i = 0;

    while(1){
        i = 0;
        oled->draw_line(0, i, 134, i, OLED::tColor::WHITE);
        oled->display();
        i++;
        oled->oprintf(0,i++ * 8, " Track #:%d", selected_song);
        oled->oprintf(0,i++ * 8, " Artist:");
        oled->oprintf(0,i++ * 8, " %s ", songs[selected_song].artist.c_str());
//        token = strtok(NULL,"-");
        oled->oprintf(0,i++ * 8, " Track:");
        oled->oprintf(0,i++ * 8, " %s ", songs[selected_song].track.c_str());
        oled->oprintf(0,i++ * 8,"\n");
        selected_song = (selected_song + 1) % songs.size();
        oled->clear();
        vTaskDelay(1000);
    }

}

void vNowPlayingScreen(void *pvParameters){
    auto oled = OLED::getInstance();
    oled->init();
    oled->clear();
    oled->display();
    unsigned int selected_song = 0;
    unsigned int i = 0;

    while(1){
        i = 0;
        oled->draw_line(0, i, 134, i, OLED::tColor::WHITE);
        oled->display();
        i++;
        oled->oprintf(0,i++ * 8, " Now Playing:");
        oled->oprintf(0,i++ * 8, " Artist:");
        oled->oprintf(0,i++ * 8, " %s ", songs[selected_song].artist.c_str());
//        token = strtok(NULL,"-");
        oled->oprintf(0,i++ * 8, " Track:");
        oled->oprintf(0,i++ * 8, " %s ", songs[selected_song].track.c_str());
        oled->oprintf(0,i++ * 8,"\n");
        selected_song = (selected_song + 1) % songs.size();
        oled->clear();
        vTaskDelay(1000);
    }

}



int main(void){

    xReadSemaphore = xSemaphoreCreateBinary();
    xPauseSemaphore = xSemaphoreCreateBinary();
    xvolumeQueue = xQueueCreate(1, sizeof(int));
    xSongQueue = xQueueCreate(1, BUFFERSIZE);

    auto pp_button = GPIO_0_1_2(2, 0);
    pp_button.setAsInput();
    pp_button.setPulldown();
    GPIOInterrupt *gpio_interrupts = GPIOInterrupt::getInstance();
    gpio_interrupts->Initialize();
    gpio_interrupts->AttachInterruptHandler(2, 0, (IsrPointer)xPauseSong, kRisingEdge);
    //gpio_interrupts->AttachInterruptHandler(0, 0, (IsrPointer)vSemaphore2Supplier, kRisingEdge);
    isr_register(EINT3_IRQn, Eint3Handler);

    getFilesFromSD(&songs);

    //eint3_enable_port2(0, eint_rising_edge, xFastForward);

    if(! myPlayer.begin()) {
        printf("Could not initialize decoder!\n");

    } else {
        printf("Initialization successful!\n");
    }

    myPlayer.sineTest(0x01, 1000);

    scheduler_add_task(new terminalTask(PRIORITY_HIGH));
    //xTaskCreate(vReadSong, "SongRead", 2048, NULL, PRIORITY_LOW, &xPause);
    //xTaskCreate(vPlaySong, "SongPlay", 2048, NULL, PRIORITY_MEDIUM, NULL);
    //xTaskCreate(vPopulateSongs, "Get Songs", 4000, NULL, PRIORITY_LOW, NULL);
    xTaskCreate(vMenuScreen, "Menu Screen", 4000, NULL, PRIORITY_LOW, NULL);
    scheduler_start();
    return -1;
}
