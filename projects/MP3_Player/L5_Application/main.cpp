/*
 *  Nickolas Schiffer & Chad Palmer
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

mp3Decoder myPlayer;
const int BUFFERSIZE = 512;
volatile bool isPlaying = false;
volatile bool ff;
volatile bool rw;
volatile bool stop = 1;
volatile bool isPN = 0;
volatile uint8_t vol = 40;
volatile uint8_t bass = 7;
volatile uint8_t treble = 5;
volatile bool bt;
volatile bool isBT = 1;
uint32_t timestamp = 0;
uint16_t taskDelay = 300;
uint16_t maxVol = 10; //Volume is backward on decoder (0 being loudest, 255 being quietest)
uint16_t minVol = 100;

TaskHandle_t      xPause;
TaskHandle_t      xNowPlayingHandle;
TaskHandle_t      xMenuHandle;
SemaphoreHandle_t xReadSemaphore;
SemaphoreHandle_t xPauseSemaphore;
QueueHandle_t     xSongQueue;
QueueHandle_t     xVolumeQueue;
QueueHandle_t     xREQueue;
SemaphoreHandle_t xPlayScreen;
SemaphoreHandle_t xBTScreen;
SemaphoreHandle_t xUpdateBT;

auto play_pause_button    = GPIO_0_1_2(2, 0);
auto prev_track_button    = GPIO_0_1_2(2, 1);
auto next_track_button    = GPIO_0_1_2(2, 2);
auto screen_toggle_button = GPIO_0_1_2(2, 3);
auto re_clk               = GPIO_0_1_2(2, 4);
auto re_data              = GPIO_0_1_2(2, 5);
auto select_button        = GPIO_0_1_2(2, 6);

unsigned int selected_song = 0;
unsigned int song_cursor   = 0;
volatile unsigned int menu_counter = 0;

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
    kNOW_PLAYING,
    kTREB_BASS
} menu_state_t;

menu_state_t menu_state = kMENU;

std::vector<song_t> songs;

void Eint3Handler(){
    GPIOInterrupt *interruptHandler = GPIOInterrupt::getInstance();
    interruptHandler->HandleInterrupt();
}

FRESULT getFilesFromSD(std::vector<song_t> *song_vec)
{
    DIR Dir;
    FILINFO Finfo;
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
                song_vec->push_back(song);

            }
        }
    }

    return FR_OK;
}

bool lastclock = 0;

void xRead_RE(void){
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
            if(!select_button.getLevel()) {
                if (clock_value != data_value){
                    rw = 1;
                }
                else {
                    ff = 1;
                }
            } else {
                if (clock_value != data_value){
                    re_state_t state = kLEFT_TURN;
                    xQueueSend(xVolumeQueue, &state, 0);
                }
                else {
                    re_state_t state = kRIGHT_TURN;
                    xQueueSend(xVolumeQueue, &state, 0);
                }
            }
        } else if(menu_state == kTREB_BASS) {
            if (clock_value != data_value){
                if(bt == 0) {
                    if(bass > 0)
                        bass -= 1;
                } else {
                    if(treble == 0)
                        treble = 15;
                    else if(treble > 8 || (treble > 0 && treble < 8))
                        treble -= 1;
                }
            }
            else {
                if(bt == 0) {
                    if(bass < 15)
                        bass += 1;
                } else {
                    if(treble == 15)
                        treble = 0;
                    else if(treble < 7 || (treble < 15 && treble > 7))
                        treble += 1;
                }
            }
            isBT = 1;
            xSemaphoreGive(xUpdateBT);
        }
        return;
    }
    return;
}

void xRead_Select_button(void){
    if (menu_state == kMENU){
        selected_song = song_cursor;
        xSemaphoreGive(xReadSemaphore);
        menu_counter = 0;
    }
    if (menu_state == kTREB_BASS){
        bt = !bt;
        xSemaphoreGive(xUpdateBT);
        menu_counter = 0;
    }
    return;
}


void xNextTrack(void){
    uint32_t currentTime = xTaskGetMsCount();

    if((currentTime >= timestamp + taskDelay) || timestamp == 0) {
        timestamp = currentTime;
        selected_song = (selected_song + 1) % songs.size();
        xSemaphoreGive(xReadSemaphore);
        isPN = 1;
    }
}

void xPrevTrack(void){
    uint32_t currentTime = xTaskGetMsCount();

    if((currentTime >= timestamp + taskDelay) || timestamp == 0) {
        timestamp = currentTime;
        selected_song = (selected_song > 0) ? selected_song - 1 : songs.size() - 1;
        xSemaphoreGive(xReadSemaphore);
        isPN = 1;
    }
}

void xScreenToggle(void){
    uint32_t currentTime = xTaskGetMsCount();

    if((currentTime >= timestamp + taskDelay) || timestamp == 0) {
        timestamp = currentTime;
        if (menu_state == kMENU){
            menu_state = kTREB_BASS;
            xSemaphoreGive(xBTScreen);
        }
        else if (menu_state == kNOW_PLAYING) {
            menu_state = kMENU;
            re_state_t state = kNOTHING;
            xQueueSendFromISR(xREQueue, &state, 0);
        }
        else if (menu_state == kTREB_BASS) {
            menu_state = kNOW_PLAYING;
            xSemaphoreGive(xPlayScreen);
        }
    }
    return;
}

void xPauseSong( void ) {

    uint32_t currentTime = xTaskGetMsCount();

    if((currentTime >= timestamp + taskDelay) || timestamp == 0) {
        timestamp = currentTime;
        if(isPlaying) {
            isPlaying = 0;
        }
        else {
            xSemaphoreGive(xPauseSemaphore);
        }
    }
}

void vReadSong(void *pvParameters)
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
                rw = 0;
                stop = 0;
                isPN = 0;
                while (!myPlayer.readyForData());
                while (!(readBytes < BUFFERSIZE)) {
                    if (!isPlaying) {
                        xSemaphoreTake(xPauseSemaphore, portMAX_DELAY);
                        isPlaying = 1;
                    }
                    if (ff && f_tell(&mySong) < f_size(&mySong) - 51200) {
                        f_lseek(&mySong, f_tell(&mySong) + 51200);
                        ff = 0;
                    }
                    if (rw && f_tell(&mySong) > 51200) {
                        f_lseek(&mySong, f_tell(&mySong) - 51200);
                        rw = 0;
                    }
                    if (current_track != selected_song){
                        break;
                    }
                    f_read(&mySong, songBuff, BUFFERSIZE, &readBytes);
                    xQueueSend(xSongQueue, songBuff, portMAX_DELAY);
                }
                f_close(&mySong);
                if(!isPN)
                    stop = 1;
                if(menu_state == kNOW_PLAYING)
                    xSemaphoreGive(xPlayScreen);
            }
        }
    }
}

inline float map(float x, float in_min, float in_max, float out_min, float out_max){
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void vPlaySong (void *pvParameters) {
    re_state_t re_state = kLEFT_TURN;
    uint8_t songBuff[BUFFERSIZE];

    while(1) {
        if(xQueueReceive(xVolumeQueue, &re_state, 0)) {
            if(re_state == kLEFT_TURN) {
                if(vol < minVol)
                    vol += 2;
                myPlayer.setVolume(vol, vol);
            }
            else if(re_state == kRIGHT_TURN) {
                if (vol > maxVol)
                    vol -= 2;
                myPlayer.setVolume(vol, vol);
            }
        }
        if (isBT) {
            myPlayer.setBassTreble(bass, treble);
            isBT = 0;
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
    oled->oprintf(0,i++ * 8, " Track #:%d", song_cursor + 1);
    oled->oprintf(0,i++ * 8, " Artist:");
    oled->oprintf(0,i++ * 8, " %s ", songs[song_cursor].artist.c_str());
    oled->oprintf(0,i++ * 8, " Track:");
    oled->oprintf(0,i++ * 8, " %s ", songs[song_cursor].track.c_str());
    oled->oprintf(0,i++ * 8,"\n");
    oled->draw_line(0, 63, 134, 63, OLED::tColor::WHITE);
    oled->display();

    while(1){

        if (xQueueReceive(xREQueue, &re_state, portMAX_DELAY)){
            menu_counter = 0;
            switch(re_state) {
                case kLEFT_TURN:
                    song_cursor = (song_cursor > 0) ? song_cursor - 1 : songs.size() - 1;
                    break;
                case kRIGHT_TURN:
                    song_cursor = (song_cursor + 1) % songs.size();
                    break;
                case kBUTTON_PRESS:
                    //play selected song
                    break;
                case kNOTHING:
                    break;
                default:
                    break;
            }
            oled->clear();
            oled->display();
            i = 0;
            oled->draw_line(0, i, 134, i, OLED::tColor::WHITE);
            i++;
            oled->oprintf(0,i++ * 8, " Track #:%d", song_cursor + 1);
            oled->oprintf(0,i++ * 8, " Artist:");
            oled->oprintf(0,i++ * 8, " %s ", songs[song_cursor].artist.c_str());
            oled->oprintf(0,i++ * 8, " Track:");
            oled->oprintf(0,i++ * 8, " %s ", songs[song_cursor].track.c_str());
            oled->oprintf(0,i++ * 8,"\n");
            oled->draw_line(0, 63, 134, 63, OLED::tColor::WHITE);
            oled->display();

            xQueueReset(xREQueue);
        }
    }
}

void vNowPlayingScreen(void *pvParameters){
    auto oled = OLED::getInstance();
    oled->clear();
    oled->display();
    unsigned int i = 0;

    int previous_track = -1;

    while(1){
            xSemaphoreTake(xPlayScreen, portMAX_DELAY);
            if(stop) {
                oled->clear();
                oled->display();
                oled->draw_line(0, 0, 134, 0, OLED::tColor::WHITE);
                oled->oprintf(0, 8, " No Song Playing");
                oled->draw_line(0, 63, 134, 63, OLED::tColor::WHITE);
                oled->display();
            } else {
                i = 0;
                oled->clear();
                oled->display();
                oled->draw_line(0, i, 134, i, OLED::tColor::WHITE);
                oled->display();
                i++;
                oled->oprintf(0,i++ * 8, " Now Playing:");
                oled->oprintf(0,i++ * 8, " Artist:");
                oled->oprintf(0,i++ * 8, " %s ", songs[selected_song].artist.c_str());
                oled->oprintf(0,i++ * 8, " Track:");
                oled->oprintf(0,i++ * 8, " %s ", songs[selected_song].track.c_str());
                oled->oprintf(0,i++ * 8,"\n");
                oled->draw_line(0, 63, 134, 63, OLED::tColor::WHITE);
                oled->display();
                previous_track = (int)selected_song;
                vTaskDelay(1);
            }
    }

}

void vBassTreble (void *pvParameters) {
    auto oled = OLED::getInstance();
    uint8_t newTreble;

    while (1) {
        xSemaphoreTake(xBTScreen, portMAX_DELAY);
        if(treble > 7)
            newTreble = treble - 8;
        else
            newTreble = treble + 8;
        oled->clear();
        oled->display();
        //Top Line
        oled->draw_line(0, 0, 134, 0, OLED::tColor::WHITE);

        //Initial Bass/Treble Display
        if(bt == 0) {
            oled->draw_rectangle(5, 15, 32, 24, OLED::tFillmode::SOLID, OLED::tColor::WHITE);
            oled->draw_string(0, 2 * 8, " Bass\n", OLED::tSize::NORMAL_SIZE, OLED::tColor::BLACK);

            oled->draw_rectangle(5, 39, 44, 47, OLED::tFillmode::SOLID, OLED::tColor::BLACK);
            oled->draw_string(0, 5 * 8, " Treble\n", OLED::tSize::NORMAL_SIZE, OLED::tColor::WHITE);

        } else {
            oled->draw_rectangle(5, 39, 44, 47, OLED::tFillmode::SOLID, OLED::tColor::WHITE);
            oled->draw_string(0, 5 * 8, " Treble\n", OLED::tSize::NORMAL_SIZE, OLED::tColor::BLACK);

            oled->draw_rectangle(5, 15, 32, 24, OLED::tFillmode::SOLID, OLED::tColor::BLACK);
            oled->draw_string(0, 2 * 8, " Bass\n", OLED::tSize::NORMAL_SIZE, OLED::tColor::WHITE);
        }


        //Endpoint Ticks
        oled->draw_line(55, 15, 55, 23, OLED::tColor::WHITE);
        oled->draw_line(128, 15, 128, 23, OLED::tColor::WHITE);
        oled->draw_line(55, 39, 55, 47, OLED::tColor::WHITE);
        oled->draw_line(128, 39, 128, 47, OLED::tColor::WHITE);
        //Reset Treble and Bass Volume Lines
        oled->draw_line(56, 19, 127, 19, OLED::tColor::BLACK);
        oled->draw_line(56, 43, 127, 43, OLED::tColor::BLACK);
        oled->draw_line(55, 19, map(bass, 0, 15, 55, 128), 19, OLED::tColor::WHITE);
        oled->draw_line(55, 43, map(newTreble, 0, 15, 55, 128), 43, OLED::tColor::WHITE);
        oled->display();
        //Botton Line
        oled->draw_line(0, 63, 134, 63, OLED::tColor::WHITE);
        oled->display();

        while(menu_state == kTREB_BASS) {
            if(xSemaphoreTake(xUpdateBT, 0)) {
                menu_counter = 0;
                if(treble > 7)
                    newTreble = treble - 8;
                else
                    newTreble = treble + 8;
                if(bt == 0) {
                    oled->draw_rectangle(5, 15, 32, 24, OLED::tFillmode::SOLID, OLED::tColor::WHITE);
                    oled->draw_string(0, 2 * 8, " Bass\n", OLED::tSize::NORMAL_SIZE, OLED::tColor::BLACK);

                    oled->draw_rectangle(5, 39, 44, 47, OLED::tFillmode::SOLID, OLED::tColor::BLACK);
                    oled->draw_string(0, 5 * 8, " Treble\n", OLED::tSize::NORMAL_SIZE, OLED::tColor::WHITE);

                    oled->draw_line(56, 19, 127, 19, OLED::tColor::BLACK);
                    oled->draw_line(55, 19, map(bass, 0, 15, 55, 128), 19, OLED::tColor::WHITE);
                } else {
                    oled->draw_rectangle(5, 39, 44, 47, OLED::tFillmode::SOLID, OLED::tColor::WHITE);
                    oled->draw_string(0, 5 * 8, " Treble\n", OLED::tSize::NORMAL_SIZE, OLED::tColor::BLACK);

                    oled->draw_rectangle(5, 15, 32, 24, OLED::tFillmode::SOLID, OLED::tColor::BLACK);
                    oled->draw_string(0, 2 * 8, " Bass\n", OLED::tSize::NORMAL_SIZE, OLED::tColor::WHITE);

                    oled->draw_line(56, 43, 127, 43, OLED::tColor::BLACK);
                    oled->draw_line(55, 43, map(newTreble, 0, 15, 55, 128), 43, OLED::tColor::WHITE);
                }
                oled->display();
            }
            vTaskDelay(50);
        }


    }
}

void vMenuTimeOut (void *pvParameters) {

    while (1) {
        menu_counter++;
        if((menu_state == kMENU || menu_state == kTREB_BASS) && menu_counter == 80) {
            xSemaphoreGive(xPlayScreen);
            menu_state = kNOW_PLAYING;
            menu_counter = 0;
        }
        vTaskDelay(100);
    }
}



int main(void){

    xReadSemaphore  = xSemaphoreCreateBinary();
    xPauseSemaphore = xSemaphoreCreateBinary();
    xBTScreen       = xSemaphoreCreateBinary();
    xPlayScreen     = xSemaphoreCreateBinary();
    xUpdateBT       = xSemaphoreCreateBinary();
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
    re_clk.setPullup();

    re_data.setAsInput();
    re_data.setPullup();

    select_button.setAsInput();
    select_button.setPullup();

    GPIOInterrupt *gpio_interrupts = GPIOInterrupt::getInstance();
    gpio_interrupts->Initialize();
    gpio_interrupts->AttachInterruptHandler(2, 0, (IsrPointer)xPauseSong, kRisingEdge);
    gpio_interrupts->AttachInterruptHandler(2, 4, (IsrPointer)xRead_RE, kBothEdges);
    gpio_interrupts->AttachInterruptHandler(2, 6, (IsrPointer)xRead_Select_button, kFallingEdge);
    gpio_interrupts->AttachInterruptHandler(2, 1, (IsrPointer)xPrevTrack, kRisingEdge);
    gpio_interrupts->AttachInterruptHandler(2, 2, (IsrPointer)xNextTrack, kRisingEdge);
    gpio_interrupts->AttachInterruptHandler(2, 3, (IsrPointer)xScreenToggle, kRisingEdge);
    isr_register(EINT3_IRQn, Eint3Handler);

    getFilesFromSD(&songs);

    if(! myPlayer.begin()) {
        printf("Could not initialize decoder!\n");

    } else {
        printf("Initialization successful!\n");
    }

    myPlayer.setVolume(vol, vol);
    //myPlayer.sineTest(0x01, 1000);

    scheduler_add_task(new terminalTask(PRIORITY_HIGH));
    xTaskCreate(vReadSong, "SongRead", 2000, NULL, PRIORITY_LOW, NULL);
    xTaskCreate(vPlaySong, "SongPlay", 2000, NULL, PRIORITY_MEDIUM, NULL);
    xTaskCreate(vMenuScreen, "Menu Screen", 512, NULL, PRIORITY_LOW, &xMenuHandle);
    xTaskCreate(vNowPlayingScreen, "Now Playing Screen", 512, NULL, PRIORITY_LOW, &xNowPlayingHandle);
    xTaskCreate(vMenuTimeOut, "MenuWatch", 512, NULL, PRIORITY_LOW, NULL);
    xTaskCreate(vBassTreble, "BT Screen", 512, NULL, PRIORITY_LOW, NULL);
    scheduler_start();
    return -1;
}
