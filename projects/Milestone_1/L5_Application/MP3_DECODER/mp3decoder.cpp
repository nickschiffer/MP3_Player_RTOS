/*
 * mp3decoder.cpp
 *
 *  Created on: Apr 24, 2019
 *      Author: chadw
 */
#include <MP3_DECODER/mp3decoder.h>

static mp3FilePlayer *myself;
volatile bool feedBufferLock = false;

static void feeder(void) {
  myself->feedBuffer();
}


mp3FilePlayer::mp3FilePlayer(int8_t rst, int8_t cs, int8_t dcs, int8_t dreq, int8_t cardCS)
{
    playingMusic = false;
    _cardCS = cardCS;
}

bool mp3FilePlayer::begin(void)
{
    pinMode(_cardCS, OUTPUT);
    digitalWrite(_cardCS, HIGH);

    uint8_t v  = Adafruit_VS1053::begin();

    //dumpRegs();
    //Serial.print("Version = "); Serial.println(v);
    return (v == 4);
}

bool mp3FilePlayer::useInterrupt(uint8_t type)
{
}

void mp3FilePlayer::feedBuffer(void)
{
}

bool mp3FilePlayer::isMP3File(const char* fileName)
{
}

unsigned long mp3FilePlayer::mp3_ID3Jumper(File mp3)
{
}

bool mp3FilePlayer::startPlayingFile(const char* trackname)
{
}

bool mp3FilePlayer::playFullFile(const char* trackname)
{
}

void mp3FilePlayer::stopPlaying(void)
{
}

bool mp3FilePlayer::paused(void)
{
}

bool mp3FilePlayer::stopped(void)
{
}

void mp3FilePlayer::pausePlaying(bool pause)
{
}

void mp3FilePlayer::feedBuffer_noLock(void)
{
}
