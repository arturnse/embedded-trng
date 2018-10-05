

#include "tick.h"

#include "inc/hw_ints.h"
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/rom.h"

static struct
{
    uint32_t frequency_hz;
    uint32_t tick_duration_ns;
    uint32_t rtc;
}_apptick = { 0 };

void _interrupt_systick(void)
{
    _apptick.rtc += 4;
}

static
apptick_t _apptick_getCur(void)
{
    return (apptick_t)ROM_SysTickValueGet();
}

void
AppTickInit(void)
{
    const struct
    {
        uint32_t clk_sys;   //clock do sistema
        uint32_t clk_iosc;  //clock interno dividido por 4
    }config = {
            (NVIC_ST_CTRL_CLK_SRC | NVIC_ST_CTRL_ENABLE),
            (NVIC_ST_CTRL_ENABLE)
    };

    const struct
    {
        uint32_t iosc_1s;
        uint32_t iosc_4s;
        uint32_t period_max;
    }period = {
        ((((16*1000*1000) / 4)*1) - 1),
        ((((16*1000*1000) / 4)*4) - 1),
        0xffffff
    };

    //ROM_SysTickEnable();
    HWREG(NVIC_ST_CTRL) = config.clk_iosc;

    ROM_SysTickPeriodSet(period.iosc_4s);

    //frequencia do tick:
    _apptick.frequency_hz = (period.iosc_1s + 1);
    //numero de ticks por segundo:
    _apptick.tick_duration_ns = (1*1000*1000*1000) / _apptick.frequency_hz;

    ROM_SysTickIntEnable();
}

uint32_t AppTick_TimeS(void)
{
    return _apptick.rtc + (AppTickGetIntervalUs(0) / (1000*1000));
}

apptick_t AppTickGet(void)
{
    return _apptick_getCur();
}

uint32_t AppTickGetIntervalUs(apptick_t tLastTick)
{
    apptick_t cur = _apptick_getCur();
    apptick_t ticks;

    if( cur < tLastTick )
    {
        ticks = tLastTick - cur;
    }
    else
    {
        ticks = tLastTick + (ROM_SysTickPeriodGet() - cur);
    }

    return ((ticks * _apptick.tick_duration_ns)/1000);
}

uint32_t AppTickGetIntervalNs(apptick_t tLastTick)
{
    apptick_t cur = _apptick_getCur();
    apptick_t ticks;

    if( cur < tLastTick )
    {
        ticks = tLastTick - cur;
    }
    else
    {
        ticks = tLastTick + (ROM_SysTickPeriodGet() - cur);
    }

    return ((ticks * _apptick.tick_duration_ns));
}

bool_t AppTickTimeoutNs(uint32_t ui32timeoutNs, apptick_t *tTick)
{
    if( tTick == NULL )
    {
        return false;
    }

    if( AppTickGetIntervalNs(*tTick) < ui32timeoutNs )
    {
        return false;
    }

    *tTick = _apptick_getCur();
    return true;
}

bool_t AppTickTimeoutUs(uint32_t ui32timeoutUs, apptick_t *tTick)
{
    if( tTick == NULL )
    {
        return false;
    }

    if( AppTickGetIntervalUs(*tTick) < ui32timeoutUs )
    {
        return false;
    }

    *tTick = _apptick_getCur();
    return true;
}
