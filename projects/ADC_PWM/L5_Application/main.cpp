#include <stdlib.h>
#include <LPC17xx.h>
#include <tasks.hpp>
#include <stdio.h>
#include "ADC/adcDriver.hpp"
#include "PWM/pwmDriver.hpp"
#include "GPIO/GPIOInterrupt.hpp"
#include <math.h>

typedef bool Mode;

#define VREF 3.3
#define MODE   bool
#define NORMAL false
#define EC     true

LabAdc::ADC_Channel pot_channel        = LabAdc::channel_3;
LabAdc::ADC_Channel light_sens_channel = LabAdc::channel_2;
LabAdc::Pin pot_pin        = LabAdc::k0_26;
LabAdc::Pin light_sens_pin = LabAdc::k0_25;

LabPwm::PWM_Pin red_pin   = LabPwm::k2_0;
LabPwm::PWM_Pin green_pin = LabPwm::k2_1;
LabPwm::PWM_Pin blue_pin  = LabPwm::k2_2;

struct sw{
    uint8_t port = 2;
    uint8_t pin  = 7;
}sw1;

typedef enum state{
    Normal,
    RGBPulse,
    LightSense,
    KnobRGB
};

state mode = Normal;





float duty_cycle_red, duty_cycle_green, duty_cycle_blue;


inline float map(float x, float in_min, float in_max, float out_min, float out_max){
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/*
 * Upon Interrupt, this method changes the operation mode
 */
void vSwitchMode(){
    switch (mode){
        case Normal:
            mode = RGBPulse;
            break;
        case RGBPulse:
            mode = LightSense;
            break;
        case LightSense:
            mode = KnobRGB;
            break;
        case KnobRGB:
            mode = Normal;
            break;
        default:
            break;
    }
    return;
}

/*
 *  **Extra Credit**
 * Maps Light Sensor or Potentiometer into point in rainbow.
 * Rainbow is calculated via 3 offset sine waves.
 * Voltage value is mapped from 0-3.3v to an angle 0-360 degrees to be used in sine function.
 */
void vLightRGB(void *pvParameters){
    auto pwm = LabPwm();
    auto adc = LabAdc();
    adc.AdcSelectPin(pot_pin);
    adc.AdcSelectPin(light_sens_pin);

    float voltage;
    double angle = 0;

    while(1){
        while((mode !=LightSense) && (mode != KnobRGB)){
            vTaskDelay(100);
        }
        if (mode == LightSense)
            voltage = adc.ReadAdcVoltageByChannel(light_sens_channel);
        else
            voltage = adc.ReadAdcVoltageByChannel(pot_channel);
        angle = (double)map(voltage, 0, 3.3, 0, 360);
        duty_cycle_red   = (float)((1*(sin((double)(angle/180*M_PI))+1))/2);
        duty_cycle_green = (float)((1*(sin((double)(angle/180*M_PI+((double)(2.f/3.f)*M_PI)))+1))/2);
        duty_cycle_blue  = (float)((1*(sin((double)(angle/180*M_PI+((double)(4.f/3.f)*M_PI)))+1))/2);
        pwm.SetDutyCycle(red_pin,duty_cycle_red);
        pwm.SetDutyCycle(green_pin,duty_cycle_green);
        pwm.SetDutyCycle(blue_pin,duty_cycle_blue);
        vTaskDelay(10);


        }

}

/*
 *  **Extra Credit**
 * Rainbow is calculated via 3 offset sine waves.
 * for-loop loops through angle 0-360 degrees which covers the rainbow.
 * Potentiometer adjusts the step size for iterating through the circle, therefore increasing the
 * rainbow speed.
 */
void vRGBTEST(void *pvParamters){
    auto pwm = LabPwm();
    auto adc = LabAdc();
    adc.AdcSelectPin(pot_pin);

    float voltage;
    uint16_t delay = 1;

    while(1){
        while(mode != RGBPulse){
            vTaskDelay(100);
        }
        for (double x = 0; x < 360; x+=delay){
            voltage = adc.ReadAdcVoltageByChannel(pot_channel);
            delay = (uint16_t)map(voltage, 0, 3.3, 1, 50);
            duty_cycle_red   = (float)((1*(sin((double)(x/180*M_PI))+1))/2);
            duty_cycle_green = (float)((1*(sin((double)(x/180*M_PI+((double)(1.5)*M_PI)))+1))/2);
            duty_cycle_blue  = (float)((1*(sin((double)(x/180*M_PI+((double)(0.5)*M_PI)))+1))/2);
            pwm.SetDutyCycle(red_pin,duty_cycle_red);
            pwm.SetDutyCycle(green_pin,duty_cycle_green);
            pwm.SetDutyCycle(blue_pin,duty_cycle_blue);
            vTaskDelay(10);
        }


        }


}

/*
 * Prints operation mode,
 * Voltage,
 * and duty cycles
 */
void vPrintTask(void *pvParamters){
    auto adc = LabAdc();
    adc.AdcSelectPin(pot_pin);
    float voltage;

    while (1){
        voltage = adc.ReadAdcVoltageByChannel(pot_channel);
        printf("Mode: %d\nvoltage: %f\nr_ds: %f\ng_ds: %f\nb_ds: %f\n\n", mode, voltage, duty_cycle_red, duty_cycle_green, duty_cycle_blue);
        vTaskDelay(1000);
    }
}

/*
 * Sets duty cycle of voltage to VREF.
 */
void vPWMADCTEST(void *pvParameters){
    auto pwm = LabPwm();
    auto adc = LabAdc();

    adc.AdcSelectPin(pot_pin);
    adc.AdcInitBurstMode();

    pwm.PwmSelectPin(red_pin);
    pwm.PwmSelectPin(green_pin);
    pwm.PwmSelectPin(blue_pin);
    pwm.PwmInitSingleEdgeMode(100);
    float voltage;

    while (1){
        while(mode != Normal){
            vTaskDelay(100);
        }

        voltage = adc.ReadAdcVoltageByChannel(pot_channel);
        duty_cycle_red = duty_cycle_green = duty_cycle_blue = (float)(voltage / (float)VREF);
        pwm.SetDutyCycle(red_pin, duty_cycle_red);
        pwm.SetDutyCycle(green_pin, duty_cycle_green);
        pwm.SetDutyCycle(blue_pin, duty_cycle_blue);
        vTaskDelay(10);
    }


}
/*
 * Detects switch button interrupt. Used to change operation mode.
 */
void Eint3Handler(){
    GPIOInterrupt *interruptHandler = GPIOInterrupt::getInstance();
    interruptHandler->HandleInterrupt();
}

int main(){
    scheduler_add_task(new terminalTask(PRIORITY_HIGH));

    printf("Clock: %u\n", sys_get_cpu_clock());

    GPIOInterrupt *gpio_interrupts = GPIOInterrupt::getInstance();
    gpio_interrupts->Initialize();
    gpio_interrupts->AttachInterruptHandler(sw1.port,sw1.pin, (IsrPointer)vSwitchMode, kRisingEdge);
    isr_register(EINT3_IRQn, Eint3Handler);


    xTaskCreate(vPWMADCTEST, "PWMADCTest", 1000, NULL, PRIORITY_LOW, NULL);
    xTaskCreate(vRGBTEST, "RGBTest", 1000, NULL, PRIORITY_LOW, NULL);
    xTaskCreate(vPrintTask, "Print", 1000, NULL, PRIORITY_LOW, NULL);
    xTaskCreate(vLightRGB, "LightSens", 1000, NULL, PRIORITY_LOW, NULL);

    scheduler_start();
    return EXIT_FAILURE;
}
