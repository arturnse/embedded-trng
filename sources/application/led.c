
#include "application/led.h"

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/rom.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"


#define LED_RED   GPIO_PIN_1
#define LED_BLUE  GPIO_PIN_2
#define LED_GREEN GPIO_PIN_3

const static struct
{
    eAppLedColor color;
    uint32_t io;
}_appled_codes[] = {
        { eAppLedColorRed, LED_RED },
        { eAppLedColorGreen, LED_GREEN },
        { eAppLedColorBlue, LED_BLUE },
        { eAppLedOff, 0 },
};

void AppLedInit(void)
{
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while( !ROM_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF) );
    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, LED_RED|LED_BLUE|LED_GREEN);
}

eAppLedColor AppLedGetColor(void)
{
    uint8_t pp;
    uint32_t rd = ROM_GPIOPinRead(GPIO_PORTF_BASE, LED_RED|LED_BLUE|LED_GREEN);

    for( pp = 0; pp < NELEMENTS(_appled_codes); pp++ )
    {
        if(_appled_codes[pp].io == rd)
        {
            return _appled_codes[pp].color;
        }
    }

    return eAppLedOff;

}


void AppLedSetColor(eAppLedColor eColor)
{

    uint8_t pp;

    for( pp = 0; pp < NELEMENTS(_appled_codes); pp++ )
    {
        if(_appled_codes[pp].color == eColor)
        {
            ROM_GPIOPinWrite(GPIO_PORTF_BASE, LED_RED|LED_BLUE|LED_GREEN, _appled_codes[pp].io);
        }
    }
}
