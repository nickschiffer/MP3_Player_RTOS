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
#include <ADC/adcDriver.hpp>

//char term_songName[20];
mp3Decoder myPlayer;
const int BUFFERSIZE = 512;
volatile bool isPlaying = false;
volatile bool ff;
volatile bool stop;
volatile uint8_t vol = 40;

//auto adc = LabAdc();
//LabAdc::ADC_Channel pot_channel = LabAdc::channel_3;
//LabAdc::Pin pot_pin             = LabAdc::k0_26;

TaskHandle_t      xPause;
TaskHandle_t      xNowPlayingHandle;
TaskHandle_t      xMenuHandle;
SemaphoreHandle_t xReadSemaphore;
SemaphoreHandle_t xPauseSemaphore;
QueueHandle_t     xSongQueue;
QueueHandle_t     xVolumeQueue;
QueueHandle_t     xREQueue;
SemaphoreHandle_t xPlayScreen;

auto play_pause_button    = GPIO_0_1_2(2, 0);
auto prev_track_button    = GPIO_0_1_2(2, 1);
auto next_track_button    = GPIO_0_1_2(2, 2);
auto screen_toggle_button = GPIO_0_1_2(2, 3);
auto re_clk               = GPIO_0_1_2(2, 4);
auto re_data              = GPIO_0_1_2(2, 5);
auto select_button        = GPIO_0_1_2(2, 6);

unsigned int selected_song = 0;
unsigned int song_cursor   = 0;



typedef struct {
    FILINFO info;
    std::string artist;
    std::string track;
    char filename[15];
} song_t;

typedef enum {
    kLEFT_TURN,
    kRIGHT_TURN,
    kBUTTON_PRESS,
    kLEFT_TURN_BUTTON_PRESSED,
    kRIGHT_TURN_BUTTON_PRESSED,
    kNOTHING
} re_state_t ;

typedef enum {
    kMENU,
    kNOW_PLAYING
} menu_state_t;

menu_state_t menu_state = kMENU;

//re_state_t state = 0;

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
                snprintf(song.filename,15, "1:%s", Finfo.fname);
                char *token = strtok(Finfo.fname, "-");
                std::string artist(token);
                song.artist = artist;
                token = strtok(NULL,".");
                std::string track(token);
                song.track = track;
                song.info = Finfo;
//                snprintf(song.filename,15, "1:%s", Finfo.fname);
                song_vec->push_back(song);
            }
            else {
                printf("long filename: %s\n", Finfo.lfname);
                snprintf(song.filename,15, "1:%s", Finfo.fname);
                char *token = strtok(Finfo.lfname, "-");
                std::string artist(token);
                song.artist = artist;
                token = strtok(NULL,".");
                std::string track(token);
                song.track = track;
                song.info = Finfo;
//                snprintf(song.filename,15, "1:%s", Finfo.fname);
                song_vec->push_back(song);

            }
        }
    }

    return FR_OK;
}

CMD_HANDLER_FUNC(playSong)
{

//    if(cmdParams.getLen() > 12)
//    {
//
//        return false;
//    }
//    else
//    {
//        sprintf(term_songName, "1:%s", cmdParams.c_str());
//        xSemaphoreGive(xReadSemaphore);
//        return true;
//    }
    return true;
    //TODO remove this
}

CMD_HANDLER_FUNC(volume)
{
//    int vol = (int)cmdParams;
//    xQueueSend(xVolumeQueue, &vol, portMAX_DELAY);
//
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

bool lastclock = 0;

void xRead_RE(void){
    //re_clk
    //printf("interrupt at: %lu\n", xTaskGetMsCount());
    bool clock_value = re_clk.getLevel();
    bool data_value = re_data.getLevel();
    if (lastclock != clock_value){
        lastclock = clock_value;
        if(menu_state == kMENU) {
            if (clock_value != data_value){
                re_state_t state = kLEFT_TURN;
                xQueueSend(xREQueue, &state, 0);
            }
            else {
                re_state_t state = kRIGHT_TURN;
                xQueueSend(xREQueue, &state, 0);
            }
        } else if(menu_state == kNOW_PLAYING) {
            if (clock_value != data_value){
                re_state_t state = kLEFT_TURN;
                xQueueSend(xVolumeQueue, &state, 0);
            }
            else {
                re_state_t state = kRIGHT_TURN;
                xQueueSend(xVolumeQueue, &state, 0);
            }
        }
        return;
    }
    return;

//    bool clock_value = re_clk.getLevel();
//    bool data_value = re_data.getLevel();
//    if (data_value != clock_value) {
//        re_state_t state = kLEFT_TURN;
//        xQueueSend(xREQueue, &state, 0);
//    }
//    else {
//        re_state_t state = kRIGHT_TURN;
//        xQueueSend(xREQueue, &state, 0);
//    }
    //re_button
}

void xRead_Select_button(void){
    if (menu_state == kMENU){
        //song selected
        selected_song = song_cursor;
        xSemaphoreGive(xReadSemaphore);
//        if (!isPlaying)
//            isPlaying = true;
        printf("song selected: %d\n", selected_song);
    }
    return;
}


void xNextTrack(void){
    selected_song = (selected_song + 1) % songs.size();
    xSemaphoreGive(xReadSemaphore);
    if(menu_state == kNOW_PLAYING)
        xSemaphoreGive(xPlayScreen);
}

void xPrevTrack(void){
    selected_song = (selected_song > 0) ? selected_song - 1 : songs.size() - 1;
    xSemaphoreGive(xReadSemaphore);
    if(menu_state == kNOW_PLAYING)
        xSemaphoreGive(xPlayScreen);
}

void xScreenToggle(void){
    if (menu_state == kMENU){
        //vTaskSuspend(xMenuHandle);
        //vTaskResume(xNowPlayingHandle);
        xSemaphoreGive(xPlayScreen);
        menu_state = kNOW_PLAYING;
    }
    else if (menu_state == kNOW_PLAYING){
        menu_state = kMENU;
        //vTaskResume(xMenuHandle);
        //vTaskSuspend(xNowPlayingHandle);
        re_state_t state = kNOTHING;
        xQueueSendFromISR(xREQueue, &state, 0);
        //xQueueSendFromISR(xREQueue, &state, 0);
    }

    return;
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

void vReadSongNew(void *pvParameters)
{
    uint8_t songBuff[BUFFERSIZE] = { 0 };
    FIL mySong;
    UINT readBytes;

    unsigned int current_track = 0;

    while (1) {
        if (xSemaphoreTake(xReadSemaphore, portMAX_DELAY)) {
            if (FR_NO_FILE == f_open(&mySong, songs[selected_song].filename, FA_READ)) {
                printf("file \"%s\" not found!\n", songs[selected_song].filename);

            }
            else {
                current_track = selected_song;
                readBytes = BUFFERSIZE;
                isPlaying = 1;
                ff = 0;
                stop = 0;
                while (!myPlayer.readyForData());
                while (!(readBytes < BUFFERSIZE)) {
                    if (!isPlaying) {
                        xSemaphoreTake(xPauseSemaphore, portMAX_DELAY);
                        isPlaying = 1;
                    }
                    if (ff) {
                        f_lseek(&mySong, f_tell(&mySong) + 51200);
                        ff = 0;
                    }
//                    if (stop) {
//                        break;
//                    }
                    if (current_track != selected_song){
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

void vReadSong (void *pvParameters) {

    uint8_t songBuff[BUFFERSIZE] = {0};
    FIL mySong;
    UINT readBytes;

    while(1) {
        if(xSemaphoreTake(xReadSemaphore, portMAX_DELAY)) {
           if(FR_NO_FILE == f_open(&mySong, songs[selected_song].filename, FA_READ)) {
               puts("File not found!");
           } else {
               puts("playing file");
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

inline float map(float x, float in_min, float in_max, float out_min, float out_max){
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void vPlaySong (void *pvParameters) {

//    auto adc = LabAdc();
//    adc.AdcInitBurstMode();
//    adc.AdcSelectPin(pot_pin);


    //= 254, volume_prev = 254;
    //int counter = 0;
    re_state_t re_state = kLEFT_TURN;
    uint8_t songBuff[BUFFERSIZE];

    while(1) {
        if(xQueueReceive(xVolumeQueue, &re_state, 0)) {
            printf("%d\n", vol);
            //volume = (int)map(volume, 0, 100, 80, 0);
            if(re_state == kLEFT_TURN) {
                //printf("Turned down\n");
                if(vol < 100)
                    vol += 5;
                myPlayer.setVolume(vol, vol);
            }
            else if(re_state == kRIGHT_TURN) {
                //printf("Turned up\n");
                if (vol > 0)
                    vol -= 5;
                myPlayer.setVolume(vol, vol);
            }
            //int vol = 254 - (((100 * (100 - (int)volume)) + (254 * (int)volume)) / 100);

        }
//        if (counter++ == 20){
//            counter = 0;
//            volume = adc.ReadAdcVoltageByChannel(pot_channel);
//            volume = (int)map(adc.ReadAdcVoltageByChannel(pot_channel), 0, 3.3, 80, 0);
//            //printf("setting volume to %d\n", volume);
//            if (volume != volume_prev){
//                myPlayer.setVolume(volume, volume);
//                volume_prev = volume;
//            }
//        }

       // if (xQueueReceive())

        xQueueReceive(xSongQueue, songBuff, portMAX_DELAY);

        myPlayer.sendData(songBuff, BUFFERSIZE);
        vTaskDelay(10);
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
            printf("Filename: %s\n", song.filename);
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
    unsigned int i = 0;

    re_state_t re_state = kLEFT_TURN;

    oled->clear();
    oled->display();
    i = 0;
    oled->draw_line(0, i, 134, i, OLED::tColor::WHITE);
    oled->display();
    i++;
    oled->oprintf(0,i++ * 8, " Track #:%d", song_cursor);
    oled->oprintf(0,i++ * 8, " Artist:");
    oled->oprintf(0,i++ * 8, " %s ", songs[song_cursor].artist.c_str());
//        token = strtok(NULL,"-");
    oled->oprintf(0,i++ * 8, " Track:");
    oled->oprintf(0,i++ * 8, " %s ", songs[song_cursor].track.c_str());
    oled->oprintf(0,i++ * 8,"\n");

    while(1){

        if (xQueueReceive(xREQueue, &re_state, portMAX_DELAY)){
            switch(re_state) {
                case kLEFT_TURN:
                    //puts("left turn received");
                    song_cursor = (song_cursor > 0) ? song_cursor - 1 : songs.size() - 1;
                    //printf("left selected song: %d\n",song_cursor);
                    break;
                case kRIGHT_TURN:
                    //puts("right turn received");
                    song_cursor = (song_cursor + 1) % songs.size();
                    //printf("left selected song: %d\n",song_cursor);
                    break;
                case kBUTTON_PRESS:
                    //play selected song
                    break;
                case kNOTHING:
                    printf("returned from Now Playing\nSong Cursor: %d\n", song_cursor);
                    break;
                default:
                    break;
            }
            //puts("hit test case\n");
            oled = OLED::getInstance();
            oled->clear();
            //puts("hit test case2\n");
            oled->display();
            i = 0;
            oled->draw_line(0, i, 134, i, OLED::tColor::WHITE);
            //puts("got to 473\n");
            oled->display();
            //puts("got to 475\n");
            i++;
            oled->oprintf(0,i++ * 8, " Track #:%d", song_cursor);
            oled->oprintf(0,i++ * 8, " Artist:");
            oled->oprintf(0,i++ * 8, " %s ", songs[song_cursor].artist.c_str());
    //        token = strtok(NULL,"-");
            oled->oprintf(0,i++ * 8, " Track:");
            oled->oprintf(0,i++ * 8, " %s ", songs[song_cursor].track.c_str());
            oled->oprintf(0,i++ * 8,"\n");
            oled->display();
            vTaskDelay(50);

            xQueueReset(xREQueue);

            //selected_song = (selected_song + 1) % songs.size();

            //vTaskDelay(1000);

        }

    }

}

void vNowPlayingScreen(void *pvParameters){
    //vTaskSuspend(NULL);
    auto oled = OLED::getInstance();
    //oled->init();
    oled->clear();
    oled->display();
    unsigned int i = 0;

    int previous_track = -1;

    while(1){
        //if ((int)selected_song != previous_track){
            xSemaphoreTake(xPlayScreen, portMAX_DELAY);
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
            oled->clear();
            previous_track = (int)selected_song;
       // }
        //else {
            vTaskDelay(1);
        //}

    }

}



int main(void){

    printf("sys clock: %u\n", sys_get_cpu_clock());

    xReadSemaphore  = xSemaphoreCreateBinary();
    xPauseSemaphore = xSemaphoreCreateBinary();
    xPlayScreen     = xSemaphoreCreateBinary();
    xVolumeQueue    = xQueueCreate(1, sizeof(int));
    xSongQueue      = xQueueCreate(1, BUFFERSIZE);
    xREQueue        = xQueueCreate(2, sizeof(re_state_t));



    play_pause_button.setAsInput();
    play_pause_button.setPulldown();

    prev_track_button.setAsInput();
    prev_track_button.setPulldown();

    next_track_button.setAsInput();
    next_track_button.setPulldown();

    screen_toggle_button.setAsInput();
    screen_toggle_button.setPulldown();

    re_clk.setAsInput();
    //re_clk.setPulldown();
    re_clk.setPullup();

    re_data.setAsInput();
    //re_data.setPulldown();
    re_data.setPullup();

    select_button.setAsInput();
    //re_button.setPulldown();
    select_button.setPulldown();

    GPIOInterrupt *gpio_interrupts = GPIOInterrupt::getInstance();
    gpio_interrupts->Initialize();
    gpio_interrupts->AttachInterruptHandler(2, 0, (IsrPointer)xPauseSong, kRisingEdge);
    gpio_interrupts->AttachInterruptHandler(2, 4, (IsrPointer)xRead_RE, kBothEdges);
    gpio_interrupts->AttachInterruptHandler(2, 6, (IsrPointer)xRead_Select_button, kRisingEdge);
    gpio_interrupts->AttachInterruptHandler(2, 1, (IsrPointer)xPrevTrack, kRisingEdge);
    gpio_interrupts->AttachInterruptHandler(2, 2, (IsrPointer)xNextTrack, kRisingEdge);
    gpio_interrupts->AttachInterruptHandler(2, 3, (IsrPointer)xScreenToggle, kRisingEdge);
    //gpio_interrupts->AttachInterruptHandler(0, 0, (IsrPointer)vSemaphore2Supplier, kRisingEdge);
    isr_register(EINT3_IRQn, Eint3Handler);

    getFilesFromSD(&songs);

    //eint3_enable_port2(0, eint_rising_edge, xFastForward);

    if(! myPlayer.begin()) {
        printf("Could not initialize decoder!\n");

    } else {
        printf("Initialization successful!\n");
    }

    myPlayer.setVolume(vol, vol);
    //myPlayer.sineTest(0x01, 1000);

    scheduler_add_task(new terminalTask(PRIORITY_HIGH));
    xTaskCreate(vReadSongNew, "SongRead", 2000, NULL, PRIORITY_LOW, NULL);
    xTaskCreate(vPlaySong, "SongPlay", 2000, NULL, PRIORITY_MEDIUM, NULL);
    //xTaskCreate(vPopulateSongs, "Get Songs", 4000, NULL, PRIORITY_LOW, NULL);
    xTaskCreate(vMenuScreen, "Menu Screen", 512, NULL, PRIORITY_LOW, &xMenuHandle);
    xTaskCreate(vNowPlayingScreen, "Now Playing Screen", 512, NULL, PRIORITY_LOW, &xNowPlayingHandle);
    scheduler_start();
    return -1;
}
