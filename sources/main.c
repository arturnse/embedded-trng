#include <stdint.h>
#include <stdbool.h>

#include "application/led.h"
#include "application/monitor.h"
#include "application/tick.h"
#include "application/trng.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"


//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

//   buffers auxiliares:
//       - Auxiliar para escrever na UART
static uint8_t _buff_AppMonitor[2*1024];
//       - Auxiliar para armanenar entropia
static uint8_t _buff_AppTrng[1*1024];


static void
_initMainClock(void)
{
    uint32_t ui32Config;

    //          - Config original (16Mhz)
    //ui32Config = (SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    //
    //          - Config PLL/2.5 com cristal externo de 16Mhz
    ui32Config = (SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    ROM_SysCtlClockSet(ui32Config);

    //
    // Valor esperado: 80 Mhz
    //
    if( ROM_SysCtlClockGet() != 80*1000*1000 )
    {
        for(;;);
    }
}

uint32_t _stacktest(uint32_t value, uint16_t test)
{
    value -= test;

    if( value )
    {
        return _stacktest(value, test);
    }
    else
    {
        return value;
    }
}

int
main(void)
{
    //
    // Enable lazy stacking for interrupt handlers.  This allows floating-point
    // instructions to be used within interrupt handlers, but at the expense of
    // extra stack usage.
    //
    ROM_FPUEnable();
    ROM_FPULazyStackingEnable();

    _initMainClock();

    AppMonitorInit(_buff_AppMonitor, sizeof(_buff_AppMonitor));
    AppLedInit();
    AppTickInit();
    TRNG_Init(_buff_AppTrng, sizeof(_buff_AppTrng));

    for(;;)
    {
        AppMonitorTask();
        TRNG_Task();
    }
}
