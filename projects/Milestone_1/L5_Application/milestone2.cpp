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

char term_songName[20];
mp3Decoder myPlayer;
const int BUFFERSIZE = 512;
volatile bool isPlaying;

TaskHandle_t xPause;
SemaphoreHandle_t xReadSemaphore;
QueueHandle_t xSongQueue;

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
    int vol = 254 - (((100 * (100 - (int)cmdParams)) + (254 * (int)cmdParams)) / 100);
    myPlayer.setVolume(vol, vol);

    return true;
}

CMD_HANDLER_FUNC(stopSong)
{
    isPlaying = 0;
    return true;
}

CMD_HANDLER_FUNC(pauseSong)
{
    vTaskSuspend(xPause);
    return true;
}

CMD_HANDLER_FUNC(unpauseSong)
{
    vTaskResume(xPause);
    return true;
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
               myPlayer.initSong();
               while(! myPlayer.readyForData());
               while(! (readBytes < BUFFERSIZE)) {
                   if(!isPlaying) {
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

    uint8_t songBuff[BUFFERSIZE];
    while(1) {
        xQueueReceive(xSongQueue, songBuff, portMAX_DELAY);

        myPlayer.sendData(songBuff, BUFFERSIZE);
        vTaskDelay(15);
    }
}

int main(void){

    xReadSemaphore = xSemaphoreCreateBinary();
    xSongQueue = xQueueCreate(1, BUFFERSIZE);


    if(! myPlayer.begin()) {
        printf("Could not initialize decoder!\n");

    } else {
        printf("Initialization successful!\n");
    }

    //myPlayer.sineTest(0x01, 1000);

    scheduler_add_task(new terminalTask(PRIORITY_HIGH));
    xTaskCreate(vReadSong, "SongRead", 2048, NULL, PRIORITY_LOW, &xPause);
    xTaskCreate(vPlaySong, "SongPlay", 2048, NULL, PRIORITY_MEDIUM, NULL);
    scheduler_start();
    return -1;
}
