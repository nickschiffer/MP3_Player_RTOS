/*
 * mp3decoder.h
 *
 *  Created on: Apr 24, 2019
 *      Author: chadw
 */

#ifndef MP3DECODER_H_
#define MP3DECODER_H_

#include <SPI/LabSpi.hpp>
#include <ff.h>
#include <io.hpp>
#include <GPIO/GPIO_0_1_2.hpp>
#include <utilities.h>

#define VS1053_FILEPLAYER_PIN_INT 5

#define VS1053_SCI_READ 0x03
#define VS1053_SCI_WRITE 0x02

#define VS1053_REG_MODE  0x00
#define VS1053_REG_STATUS 0x01
#define VS1053_REG_BASS 0x02
#define VS1053_REG_CLOCKF 0x03
#define VS1053_REG_DECODETIME 0x04
#define VS1053_REG_AUDATA 0x05
#define VS1053_REG_WRAM 0x06
#define VS1053_REG_WRAMADDR 0x07
#define VS1053_REG_HDAT0 0x08
#define VS1053_REG_HDAT1 0x09
#define VS1053_REG_VOLUME 0x0B

#define VS1053_MODE_SM_DIFF 0x0001
#define VS1053_MODE_SM_LAYER12 0x0002
#define VS1053_MODE_SM_RESET 0x0004
#define VS1053_MODE_SM_CANCEL 0x0008
#define VS1053_MODE_SM_EARSPKLO 0x0010
#define VS1053_MODE_SM_TESTS 0x0020
#define VS1053_MODE_SM_STREAM 0x0040
#define VS1053_MODE_SM_SDINEW 0x0800
#define VS1053_MODE_SM_ADPCM 0x1000
#define VS1053_MODE_SM_LINE1 0x4000
#define VS1053_MODE_SM_CLKRANGE 0x8000

#define VS1053_DATABUFFERLEN 512

enum SetBit {
    LOW,
    HIGH
};

class mp3Decoder {
public:


    //Constructor
    mp3Decoder();

    bool begin(void);
    void reset(void);
    void setVolume(uint8_t left, uint8_t right);
    void setBassTreble(uint8_t b, uint8_t t);
    void setDCS(SetBit s);
    void sendData(uint8_t *buffer, uint16_t buffersize);
    bool readyForData();
    void sineTest(uint8_t n, uint16_t ms);
    void initSong();


private:
    uint16_t sciRead(uint8_t addr);
    void sciWrite(uint8_t addr, uint16_t data);


    LabSpi SPI;

    uint8_t mp3buffer[VS1053_DATABUFFERLEN];
};

//class mp3FilePlayer : public mp3Decoder {
// public:
//
//  mp3FilePlayer (int8_t rst, int8_t cs, int8_t dcs, int8_t dreq);
//
//  bool begin(void);
//  bool useInterrupt(uint8_t type);
//  //File currentTrack;
//  volatile bool playingMusic;
//  void feedBuffer(void);
//  static bool isMP3File(const char* fileName);
//  bool startPlayingFile(const char *trackname);
//  bool playFullFile(const char *trackname);
//  void stopPlaying(void);
//  bool paused(void);
//  bool stopped(void);
//  void pausePlaying(bool pause);
//
// private:
//  void feedBuffer_noLock(void);
//
//  //uint8_t _cardCS;
//};

#endif /* MP3DECODER_H_ */
