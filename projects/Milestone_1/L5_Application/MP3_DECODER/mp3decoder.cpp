/*
 * mp3decoder.cpp
 *
 *  Created on: Apr 24, 2019
 *      Author: chadw
 */
#include <MP3_DECODER/mp3decoder.h>
#include <stdio.h>

//GPIO cs_(P0_0);
//GPIO dreq_(P0_1);
//GPIO xdcs_(P0_29);
//GPIO reset_(P0_30);

auto cs_ = GPIO_0_1_2(0, 0);
auto dreq_ = GPIO_0_1_2(0, 1);
auto xdcs_ = GPIO_0_1_2(0, 29);
auto reset_ = GPIO_0_1_2(0, 30);

mp3Decoder::mp3Decoder()
{
}

bool mp3Decoder::begin(void)
{
    SPI.initialize(8, LabSpi::tSPI, 32);
    cs_.setAsOutput();
    cs_.setHigh();
    xdcs_.setAsOutput();
    xdcs_.setHigh();
    dreq_.setAsInput();
    reset_.setAsOutput();
    reset_.setLow();

    reset();

    uint8_t v  = (sciRead(VS1053_REG_STATUS) >> 4) & 0x0F;
//    printf("Version: %d\n", v);
//    printf("Mode = 0x%x\n", sciRead(VS1053_REG_MODE));
//    printf("Stat = 0x%x\n", sciRead(VS1053_REG_STATUS));
//    printf("ClkF = 0x%x\n", sciRead(VS1053_REG_CLOCKF));
//    printf("Vol. = 0x%x\n", sciRead(VS1053_REG_VOLUME));
    return (v == 4);
}

void mp3Decoder::reset(void)
{
    reset_.setLow();
    delay_ms(100);
    reset_.setHigh();
    cs_.setHigh();
    xdcs_.setHigh();
    delay_ms(100);
    sciWrite(VS1053_REG_MODE, VS1053_MODE_SM_SDINEW | VS1053_MODE_SM_RESET);
    delay_ms(200);

    sciWrite(VS1053_REG_CLOCKF, 0x6000);
    //  sciWrite(VS1053_REG_BASS, sciRead(VS1053_REG_BASS) | (15 << 4));
    //setVolume(0x20, 0x20);
}

uint16_t mp3Decoder::sciRead(uint8_t addr)
{
    uint16_t data;

    cs_.setLow();
    SPI.transfer(VS1053_SCI_READ);
    SPI.transfer(addr);

    //delay_us(10);
    data = SPI.transfer(0x00);
    data <<= 8;
    data |= SPI.transfer(0x00);
    cs_.setHigh();
    return data;
}

void mp3Decoder::sciWrite(uint8_t addr, uint16_t data)
{
    cs_.setLow();
    SPI.transfer(VS1053_SCI_WRITE);
    SPI.transfer(addr);
    SPI.transfer(data >> 8);
    SPI.transfer(data & 0xFF);
    cs_.setHigh();
}

void mp3Decoder::initSong()
{
    sciWrite(VS1053_REG_MODE, VS1053_MODE_SM_LINE1 | VS1053_MODE_SM_SDINEW);
    sciWrite(VS1053_REG_WRAMADDR, 0xC017);
    sciWrite(VS1053_REG_WRAM, 3);
    sciWrite(VS1053_REG_WRAMADDR, 0xC019);
    sciWrite(VS1053_REG_WRAM, 0);
    sciWrite(VS1053_REG_DECODETIME, 0x00);
    sciWrite(VS1053_REG_DECODETIME, 0x00);
}

void mp3Decoder::setDCS(SetBit s)
{
    if (!s) {
        xdcs_.setLow();
    } else {
        xdcs_.setHigh();
    }
}

void mp3Decoder::setVolume(uint8_t left, uint8_t right)
{
    uint16_t vol;
    vol = left;
    vol <<= 8;
    vol |= right;

    sciWrite(VS1053_REG_VOLUME, vol);
}

void mp3Decoder::setBassTreble(uint8_t b, uint8_t t) {
    uint16_t setting;
    setting = (15 | (b << 4) | (1 << 8) | (t << 12));
    sciWrite(VS1053_REG_BASS, setting);
}

void mp3Decoder::sendData(uint8_t *buffer, uint16_t buffsize)
{
    while(! readyForData());
    xdcs_.setLow();

//    for(uint8_t i = 0; i < buffsize; i++) {
//        //printf("%x\n", buffer[i]);
//        SPI.transfer(buffer[i]);
//    }
    //printf("End of chunk\n");

    while (buffsize--) {
        //printf("%d\n", buffsize);
        //printf("%x\n", buffer[0]);
        if((buffsize % 32) == 0)
        {
            while(! readyForData());
        }
        SPI.transfer(buffer[0]);
        buffer++;
    }

    xdcs_.setHigh();
}

void mp3Decoder::sineTest(uint8_t n, uint16_t ms)
{
    uint16_t mode = sciRead(VS1053_REG_MODE);
    mode |= 0x0020;
    sciWrite(VS1053_REG_MODE, mode);

    while (!readyForData());

    xdcs_.setLow();
    SPI.transfer(0x53);
    SPI.transfer(0xEF);
    SPI.transfer(0x6E);
    SPI.transfer(n);
    SPI.transfer(0x00);
    SPI.transfer(0x00);
    SPI.transfer(0x00);
    SPI.transfer(0x00);
    xdcs_.setHigh();

    delay_ms(ms);

    xdcs_.setLow();
    SPI.transfer(0x45);
    SPI.transfer(0x78);
    SPI.transfer(0x69);
    SPI.transfer(0x74);
    SPI.transfer(0x00);
    SPI.transfer(0x00);
    SPI.transfer(0x00);
    SPI.transfer(0x00);
    xdcs_.setHigh();
}

bool mp3Decoder::readyForData()
{
    return dreq_.getLevel();
}
