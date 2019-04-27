/*
 * mp3decoder.cpp
 *
 *  Created on: Apr 24, 2019
 *      Author: chadw
 */
#include <MP3_DECODER/mp3decoder.h>
#include <stdio.h>

GPIO cs_(P0_0);
GPIO dreq_(P0_1);
GPIO xdcs_(P0_29);
GPIO reset_(P0_30);

mp3Decoder::mp3Decoder()
{
}

bool mp3Decoder::begin(void)
{
    SPI.initialize(8, LabSpi::tSPI, 16);
    cs_.setAsOutput();
    cs_.setHigh();
    xdcs_.setAsOutput();
    xdcs_.setHigh();
    dreq_.setAsInput();
    reset_.setAsOutput();
    reset_.setLow();

    reset();

    uint8_t v  = (sciRead(VS1053_REG_STATUS) >> 4) & 0x0F;
    printf("Version: %d\n", v);
    printf("Mode = 0x%x\n", sciRead(VS1053_REG_MODE));
    printf("Stat = 0x%x\n", sciRead(VS1053_REG_STATUS));
    printf("ClkF = 0x%x\n", sciRead(VS1053_REG_CLOCKF));
    printf("Vol. = 0x%x\n", sciRead(VS1053_REG_VOLUME));
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
    setVolume(40, 40);
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

void mp3Decoder::setVolume(uint8_t left, uint8_t right)
{
    uint16_t vol;
    vol = left;
    vol <<= 8;
    vol |= right;

    sciWrite(VS1053_REG_VOLUME, vol);
}

void mp3Decoder::sineTest(uint8_t n, uint16_t ms)
{
}
