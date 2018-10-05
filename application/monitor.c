//*****************************************************************************
//
// uart_echo.c - Example for reading data from and writing data to the UART in
//               an interrupt driven fashion.
//
// Copyright (c) 2012-2015 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 2.1.2.111 of the EK-TM4C123GXL Firmware Package.
//
//*****************************************************************************


#include "monitor.h"

#include <string.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"

#include "common/ring_buffer.h"
#include "common/conv.h"
#include "led.h"
#include "tick.h"
#include "trng.h"


#define APP_MON_NEWLINE "\r\n"

static struct
{
    enum
    {
        eAppMonStOffline = 0,
        eAppMonStWaitingForCommand,
        eAppMonStRandomGen,
        eAppMonStBenchmark,
    }state;

    struct
    {
        ringbuff_t rx;
        ringbuff_t tx;
    }rb;

    apptick_t timeout_cmd;

    struct
    {
        uint8_t pos;
        bool_t reset;
    }benchmark;

    struct
    {
        uint8_t buff[64];
        ringbuff_t rb;
        apptick_t timeout;
        uint32_t last;
        uint32_t sample;
    }trng_rate;
}_amon;

typedef void (*amon_cfg_trng_f)(void);

static void _appmon_cmd0TrngGen(void);
static void _appmon_cmd1TrngGen(void);
static void _appmon_cmd2TrngGen(void);
static void _appmon_cmd3TrngGen(void);
static void _appmon_cmd4TrngGen(void);

static const amon_cfg_trng_f _amon_cfg[] = {
    _appmon_cmd0TrngGen,
    _appmon_cmd1TrngGen,
    _appmon_cmd2TrngGen,
    _appmon_cmd3TrngGen,
    _appmon_cmd4TrngGen,
};

static void
_amon_intEnable(bool_t pEnable)
{
    if(pEnable)
    {
        return ROM_IntEnable(INT_UART0);
    }
    else
    {
        return ROM_IntDisable(INT_UART0);
    }

}

static void
_appmon_wr(const uint8_t *psData, uint32_t pu32sz)
{
    _amon_intEnable(false);
    if( RingBuff_WrMany(&_amon.rb.tx, psData, pu32sz, false) )
    {
        ROM_UARTIntEnable(UART0_BASE, UART_INT_TX);
    }
    _amon_intEnable(true);
}

static void
_amon_wrStr(const char_t *usCmd)
{
    return _appmon_wr((const uint8_t*)usCmd, strlen(usCmd));
}

static void
_amon_wrInt(int32_t pi32val)
{
    char_t number[16];
    (void)Conv_IntToStr(pi32val, number, STRLEN(number));
    return _appmon_wr((const uint8_t *)number, strlen(number));
}

uint32_t _amon_getBitCpuTime(void)
{
    uint32_t sample;
    uint32_t pp = 0;
    uint32_t result = 0;
    ringbuff_t rb = _amon.trng_rate.rb;

    while( RingBuff_IsEmpty(&rb) == false )
    {
        if( RingBuff_RdOne(&rb, &sample) )
        {
            pp ++;
            result += sample;
        }
    }
    //tira a media
    return result /= pp;
}

uint32_t _amon_getTrngGenerationRate(void)
{
    uint32_t hz;
    uint8_t bitsz;

    (void)TRNG_GetFreqGenerationHz(&hz, &bitsz);

    return hz;
}

uint32_t _amon_getTrngGenerationBitQtt(void)
{
    uint32_t hz;
    uint8_t bitsz;

    (void)TRNG_GetFreqGenerationHz(&hz, &bitsz);

    return bitsz;
}

static void
_appmon_printBenchmark(char_t * psDescription)
{
    _amon_wrStr(APP_MON_NEWLINE);
    _amon_wrStr(psDescription);
    _amon_wrStr(APP_MON_NEWLINE"  Bit CPU time: ");
    _amon_wrInt(_amon_getBitCpuTime());
    _amon_wrStr("ns/bit");
    _amon_wrStr(APP_MON_NEWLINE"  TRNG generation rate: ");
    _amon_wrInt(_amon_getTrngGenerationBitQtt());
    _amon_wrStr("bits @");
    _amon_wrInt(_amon_getTrngGenerationRate());
    _amon_wrStr("hz");
}

static void _appmon_cmd0TrngGen(void)
{
    (void)TRNG_Configure(trng_simplest, 1, false);
}

static void _appmon_cmd1TrngGen(void)
{
    (void)TRNG_Configure(trng_simple, 2, false);
}

static void _appmon_cmd2TrngGen(void)
{
    (void)TRNG_Configure(trng_simple, 4, true);
}

static void _appmon_cmd3TrngGen(void)
{
    (void)TRNG_Configure(trng_allbytes, 5, true);
}

static void _appmon_cmd4TrngGen(void)
{
    (void)TRNG_Configure(trng_allbytes, 8, true);
}

static void
_appmon_ProcessCommand(uint8_t u8cmd)
{
    switch(u8cmd)
    {
        case 'b':
        case 'B':
            _amon.state = eAppMonStBenchmark;
            _amon.benchmark.reset = true;
            break;
        case '0':
            _appmon_cmd0TrngGen();
            _amon.state = eAppMonStRandomGen;
            break;
        case '1':
            _appmon_cmd1TrngGen();
            _amon.state = eAppMonStRandomGen;
            break;
        case '2':
            _appmon_cmd2TrngGen();
            _amon.state = eAppMonStRandomGen;
            break;
        case '3':
            _appmon_cmd3TrngGen();
            _amon.state = eAppMonStRandomGen;
            break;
        case '4':
            _appmon_cmd4TrngGen();
            _amon.state = eAppMonStRandomGen;
            break;

        case 'S':
        case 's':
            _amon_wrStr(APP_MON_NEWLINE"  Saving TRNG seed...");
            if( RES_SUCCESS == TRNG_SeedSave() )
            {
                _amon_wrStr("OK");
            }
            else
            {
                _amon_wrStr("ERROR.");
            }
            break;

        default:
            if( eAppMonStWaitingForCommand != _amon.state )
            {
                _amon.state = eAppMonStWaitingForCommand;
                _amon_wrStr(APP_MON_NEWLINE);
                _amon_wrStr(APP_MON_NEWLINE"True Random Number Generator monitor online.");
                _amon_wrStr(APP_MON_NEWLINE"  g: generate entropy");
                _amon_wrStr(APP_MON_NEWLINE"  1: configure TRNG as level 1");
                _amon_wrStr(APP_MON_NEWLINE"  p: print TRNG current status");
                _amon_wrStr(APP_MON_NEWLINE);
                //ignora o comando.
                (void)u8cmd;
                return;
            }
            break;
    }
}

void
AppMonitorInit(void * pui8buff, uint16_t pu16buffSz)
{
    struct
    {
        struct
        {
            void * tx;
            void * rx;
        }ptr;
        struct
        {
            uint32_t tx;
            uint32_t rx;
        }sz;
    }buff;

    //2% vai pra buffer de recebimento
    buff.sz.rx = (pu16buffSz * 2) / 100;
    buff.sz.tx = pu16buffSz - buff.sz.rx;

    buff.ptr.tx = (uint8_t*)pui8buff + 0;
    buff.ptr.rx = (uint8_t*)pui8buff + buff.sz.tx;

    if( buff.sz.rx + buff.sz.tx != pu16buffSz)
    {
        return;
    }

    RingBuff_Init(&_amon.rb.rx, buff.ptr.rx, buff.sz.rx, sizeof(char_t));
    RingBuff_Init(&_amon.rb.tx, buff.ptr.tx, buff.sz.tx, sizeof(char_t));

    //buffer util para calcular a media de processamento gasto na geracao dos bits
    RingBuff_Init(&_amon.trng_rate.rb, _amon.trng_rate.buff, sizeof(_amon.trng_rate.buff), sizeof(_amon.trng_rate.last));

    // Habilita os perifericos
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    //
    // Set GPIO A0 and At1 as UART pins.
    //
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Configure the UART for 115,200, 8-N-1 operation.
    //
    ROM_UARTConfigSetExpClk(UART0_BASE, ROM_SysCtlClockGet(), 230400*2,//230400,//115200,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));

    //ROM_UARTFIFOEnable()
    ROM_UARTFIFOEnable(UART0_BASE);
    ROM_UARTTxIntModeSet(UART0_BASE, UART_TXINT_MODE_EOT);
    ROM_UARTFIFOLevelSet(UART0_BASE, UART_FIFO_TX4_8, UART_FIFO_RX1_8);
    ROM_UARTCharPut(UART0_BASE, 0);

    //
    // Enable the UART interrupt.
    //
    ROM_UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT | UART_INT_TX);
    _amon_intEnable(true);


}

void AppMonitorTask(void)
{
    struct
    {
        char_t *cmd;
        uint8_t data[8];
        uint32_t sz;
    }rd;

    uint32_t rate;

    do
    {
        _amon_intEnable(false);
        rd.sz = RingBuff_RdOnePtr(&_amon.rb.rx, (void**)&rd.cmd);
        _amon_intEnable(true);
        if( rd.sz )
        {
            _appmon_ProcessCommand(*rd.cmd);
        }
    }while(rd.sz);

    switch(_amon.state)
    {
        case eAppMonStRandomGen:
            if( (TRNG_GetCountBits() / 8) > 16 )
            {
                rd.sz = TRNG_Read(rd.data, sizeof(rd.data));
#if !defined(DEBUG)
                _appmon_wr(rd.data, rd.sz);
#endif
            }

            break;
        case eAppMonStBenchmark:
            if(_amon.benchmark.reset)
            {
                _amon.benchmark.pos = 0;
                (void)RingBuff_Flush(&_amon.trng_rate.rb);
                _amon_cfg[_amon.benchmark.pos]();
                _amon.benchmark.reset = false;
                _amon_wrStr(APP_MON_NEWLINE"Data Benchmark ");
#if defined(DEBUG)
                _amon_wrStr("DEBUG");
#else
                _amon_wrStr("RELEASE");
#endif
            }
            else if( RingBuff_IsFull(&_amon.trng_rate.rb) )
            {
                if(_amon.benchmark.pos == 0)
                {
                    _appmon_printBenchmark("  TRNG Config: simplest, no seed ('0' to generate)");
                }
                if(_amon.benchmark.pos == 1)
                {
                    _appmon_printBenchmark("  TRNG Config: simple, no seed ('1' to generate)");
                }
                if(_amon.benchmark.pos == 2)
                {
                    _appmon_printBenchmark("  TRNG Config: simple, use seed ('2' to generate)");
                }
                if(_amon.benchmark.pos == 3)
                {
                    _appmon_printBenchmark("  TRNG Config: allclockbytes, use seed ('3' to generate)");
                }
                if(_amon.benchmark.pos == 4)
                {
                    _appmon_printBenchmark("  TRNG Config: allclockbytes, use seed ('4' to generate)");
                }
                _amon.benchmark.pos ++;

                if( _amon.benchmark.pos == NELEMENTS(_amon_cfg) )
                {
                    _amon.state = eAppMonStWaitingForCommand;
                }
                else
                {
                    (void)RingBuff_Flush(&_amon.trng_rate.rb);
                    _amon_cfg[_amon.benchmark.pos]();
                }
            }
            break;

        default:
            break;
    }

    rate = TRNG_GetBitCpuTimeNs();
    if( _amon.trng_rate.last != rate && ((++_amon.trng_rate.sample % 16) == 0) )
    {
        (void)RingBuff_WrOne(&_amon.trng_rate.rb, &rate, true);
    }
}


//*****************************************************************************
//
// The UART interrupt handler.
//
//*****************************************************************************
void
UARTIntHandler(void)
{
    uint32_t ui32ints;

    union
    {
        int32_t res;
        uint8_t byte;
        void *byteptr;
    }aux;

    // captura as interrupcoes acionadas
    ui32ints = ROM_UARTIntStatus(UART0_BASE, true);

    // le todos os bytes da FIFO
    if(ui32ints & (UART_INT_RX|UART_INT_RT))
    {
        ROM_UARTIntClear(UART0_BASE, (UART_INT_RX|UART_INT_RT));
        while(ROM_UARTCharsAvail(UART0_BASE))
        {
            aux.res = ROM_UARTCharGetNonBlocking(UART0_BASE);
            if( aux.res != -1 )
            {
                aux.byte = (uint8_t)aux.res;
                RingBuff_WrOne(&_amon.rb.rx, &aux.byte, true);
            }
        }
    }

    if(ui32ints & UART_INT_TX)
    {
        //
        // nao limpa a flag da interrupcao porque pode ser util chama-la
        // novamente quando houverem mais bytes para enviar
        //
        //ROM_UARTIntClear(UART0_BASE, UART_INT_TX);
        //
        while(ROM_UARTSpaceAvail(UART0_BASE) && RingBuff_Used(&_amon.rb.tx))
        {
            RingBuff_RdOnePtr(&_amon.rb.tx, &aux.byteptr);
            ROM_UARTCharPut(UART0_BASE, *(uint8_t*)aux.byteptr);
        }

        if( RingBuff_IsEmpty(&_amon.rb.tx) )
        {
            //
            // desabilita a interrupcao, mas NAO limpa a flag interrupcao
            //
            ROM_UARTIntDisable(UART0_BASE, UART_INT_TX);
        }
    }
}
