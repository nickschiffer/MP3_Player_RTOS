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

int main(void){

    xReadSemaphore = xSemaphoreCreateBinary();
    xPauseSemaphore = xSemaphoreCreateBinary();
    xvolumeQueue = xQueueCreate(1, sizeof(int));
    xSongQueue = xQueueCreate(1, BUFFERSIZE);

    eint3_enable_port2(0, eint_rising_edge, xFastForward);

    if(! myPlayer.begin()) {
        printf("Could not initialize decoder!\n");

    } else {
        printf("Initialization successful!\n");
    }

    myPlayer.sineTest(0x01, 1000);

    scheduler_add_task(new terminalTask(PRIORITY_HIGH));
    xTaskCreate(vReadSong, "SongRead", 2048, NULL, PRIORITY_LOW, &xPause);
    xTaskCreate(vPlaySong, "SongPlay", 2048, NULL, PRIORITY_MEDIUM, NULL);
    scheduler_start();
    return -1;
}
