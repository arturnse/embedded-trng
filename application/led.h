
#ifndef APPLICATION_LED_H_
#define APPLICATION_LED_H_

#include "api_config.h"

typedef enum
{
    eAppLedOff = 0,
    eAppLedColorBlue,
    eAppLedColorRed,
    eAppLedColorGreen,
}eAppLedColor;


void AppLedInit(void);



eAppLedColor AppLedGetColor(void);

void AppLedSetColor(eAppLedColor eColor);

#endif /* APPLICATION_LED_H_ */
