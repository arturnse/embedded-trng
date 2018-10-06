//*****************************************************************************
//
// project0.c - Example to demonstrate minimal TivaWare setup
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

#include "trng.h"

#include <string.h>
#include "inc/hw_ints.h"
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"

#include "driverlib/timer.h"

#include "common/entropy_pool.h"

#include "led.h"
#include "tick.h"


typedef void (*trng_gen_f)(void);
static struct
{
    entropy_pool_t pp;
    uint32_t cpu_time_ns;
    struct
    {
        uint8_t bit_qtt;
        bool_t use_seed;
    }config;
    trng_gen_f gen_f;
}_trng = { 0 };

#pragma DATA_SECTION(_trng_initVector, ".trng_seed")
static const uint32_t _trng_initVector[] =
{
    0x0383e433, 0x8837c9d7, 0xfd3eb6be, 0x27ec7810, 0x924f8510, 0x1278fedd, 0x3fae1bce, 0x5d1899ac,
    0xa7b59624, 0x786ad263, 0x92fa146e, 0x80cd58c0, 0xbd44b392, 0xb698f66c, 0x32c114f4, 0xfe2987b7,
    0xb7c6269d, 0x28bdae7b, 0x8003cef8, 0x9ad09dbd, 0x54dbc6d9, 0xf875aae5, 0xb3462578, 0x2a60e6c6,
    0x4c660748, 0x8e609a24, 0xcc713569, 0x428d4fcf, 0x662875c1, 0x4f1d7b87, 0x574493a4, 0x039579b3,
    0x3abca23a, 0x39a8a4c7, 0x748baed6, 0x0e7a8751, 0x811ca26a, 0xd926eab3, 0x84f4e10c, 0xec9eb56c,
    0x546c7cb0, 0x6b8851d5, 0xb511f98e, 0xcb373aee, 0xd3a4b021, 0x4c76bf12, 0xa920355b, 0x4b8a4c55,
    0xa7bc30ec, 0xe5f29445, 0x7964ed3e, 0x40f99680, 0xa2d78391, 0x35c30316, 0x3ab0b978, 0x2cba5560,
    0x975e8e00, 0x6e494fbb, 0x4f241c8c, 0x08522b6d, 0x44e6c0d5, 0x6864c810, 0x636127f3, 0x11fdab61,
    0x7a95ff5c, 0x45cb1bb3, 0xdbef5f7d, 0x229337d6, 0x2252a82b, 0xe73eb406, 0x1c9c2693, 0x6d1317e8,
    0x241d0dce, 0xb1f57e3f, 0x0af525d1, 0xa8715533, 0x4bd21861, 0xa49e5011, 0xb22ce1bd, 0xd2b9b9ad,
    0x840beeda, 0xc4438bb5, 0xe8ced06c, 0xdb39d9e1, 0x47eb7409, 0x2f342e34, 0x3cec37f5, 0x073e2191,
    0xb91e19c4, 0x7ec8cbd1, 0x053c88cf, 0x577846b0, 0xdcad13a7, 0xb063540f, 0xfb69e86e, 0x634bd490,
    0x46f8c31a, 0xaba186e7, 0x58e0a6ce, 0xb6884314, 0x1860dd40, 0x7a6505c5, 0x68997889, 0xc7e8e188,
    0xab570187, 0x6b11df88, 0xfbb67075, 0x83fcf0dc, 0x8102476c, 0x5022b0be, 0xe2d58862, 0xcbd95292,
    0xf3db48e7, 0xad161e57, 0x775c86e2, 0xa698ad2e, 0x1b6037b3, 0x5d2e4fb5, 0x34c403f6, 0x1483c07e,
    0x6c6b105c, 0xb408c888, 0x1bb37fab, 0x7df9836a, 0x73b70937, 0x7bfe4960, 0x99bce635, 0x011883c5,
    0x29ab83b7, 0xe9ebcc2c, 0xeab8b3b1, 0x1a85b6b0, 0x93dddd5d, 0xa34b12c5, 0x486be2fa, 0x4922f815,
    0x642c0f59, 0x6ce87160, 0xf4b4b344, 0x4cb2f54a, 0x6ea3791a, 0xc32548ac, 0x645d41d1, 0x8f2acdc3,
    0xc1651dfc, 0xe0610631, 0x523f9441, 0x9359f022, 0x65fe8e18, 0x79eadaf2, 0x0187e3b6, 0xaa812825,
    0xe39a99ae, 0x90e2a487, 0xc1bbd336, 0xf600004d, 0x4edd91c8, 0x188ac074, 0xdf5d9c9d, 0x34b13bfd,
    0x61e31ef8, 0xe1612142, 0x24675cd2, 0x06211d8a, 0x7e392014, 0x8a0c32af, 0x8f2c7b1c, 0x41bbb4c0,
    0xe02d5b96, 0x83ac27b8, 0xc3c017a3, 0x3b3062e9, 0x69e7b925, 0x08f540c3, 0xe27b82db, 0x72518054,
    0xe10bbaee, 0x4fb4b837, 0xb239fc60, 0x44fb4b4f, 0x8a16b588, 0x5079af44, 0x81585d06, 0xda68bd57,
    0x3a34088c, 0x0bc651cf, 0xa3f4b907, 0xd69f2f9d, 0x0eb79cbc, 0xe3987884, 0xbbc28561, 0x5b5d1e84,
    0xb28542a8, 0xf274266e, 0x4e55249b, 0x80714df5, 0x2b04a391, 0xd20eb6e2, 0x32ff5362, 0xfb1d6c19,
    0x392d23b1, 0xff12884c, 0x94508f0a, 0x6f4c6e2a, 0x903a9457, 0xf11cdbce, 0x4cde1bde, 0x7e1cea69,
    0x2bd5970f, 0x5f648bbb, 0x92499b34, 0x7cb70576, 0x2415a6c4, 0xc29119f9, 0x6e1d5b8b, 0x1aff7a1c,
    0x46e2abc6, 0xeb79cc69, 0x7990a613, 0x371b4d61, 0xd05a2077, 0x29e081ab, 0x9380e364, 0x6ba52dc7,
    0xa1b0e81c, 0x86b0ceb4, 0xd34b271e, 0x859242b7, 0x06f58a2f, 0xb9ee74a4, 0xf80fddb8, 0x84c07415,
    0xf51ebdf8, 0xb7fb505f, 0x2942240d, 0x5c826b15, 0x5ad8fc15, 0x4a22c5a0, 0x3ba26dd9, 0x146640fa,
    0xd5556270, 0xa1c1cc3c, 0xbd4aaf59, 0x192fb4fd, 0xb096611c, 0xf3eecfad, 0x6fe39ce5, 0xe8f0927a,
    0x32fe9003, 0x262337f9, 0x2e2da7ca, 0xb4336a2c, 0x088e2ecd, 0xaf9134f7, 0xb8ca102c, 0x5789bccb,
    0x9181e558, 0x9890a3df, 0xdc8c3901, 0xe54b3a47, 0x8001c035, 0xfa29f2d9, 0x9e3c87a6, 0xdf838f5d,
    0x0d17e995, 0x6ffa3692, 0x83c24d7f, 0x412428f3, 0x6adbf708, 0xca4e62ea, 0xea182aca, 0xb314d606,
    0x1421878e, 0x6d090a5c, 0x1629aeef, 0x7fbdb4a0, 0x45911793, 0xdb86adcf, 0x1b0da0ab, 0x3d3d0174,
    0x4fc98c3a, 0xdb260dfd, 0xefcddf53, 0x17258095, 0x67889a65, 0x8df21026, 0xcd7d653d, 0x1c136fd6,
    0x05e964bf, 0xd9c9c615, 0x5636c14f, 0x5c35d48e, 0x9a2999ee, 0xfaf9e3bb, 0xeddc57b2, 0xb0be85a0,
    0x0e84a871, 0xe81d6b08, 0x221b0b77, 0x98eb14b4, 0xc17fdb03, 0x78b9587a, 0x18a4a695, 0x82c07bd0,
    0x7c56f98d, 0x09ccea64, 0xab81c7c2, 0xa54f6b11, 0x53f7563a, 0xe9b9ee13, 0x99748f91, 0x59603e0f,
    0x6acfe9f1, 0x4cab5a5d, 0x94e1532f, 0xbb5abbf9, 0xdf9cb3e2, 0xca52c11f, 0xd2771c14, 0x55dc6f9b,
    0xc46f6b3c, 0x326ef0a6, 0x20b13c84, 0x58428eae, 0x79a9fb94, 0x051ea381, 0xd656d100, 0x3e89e52d,
    0xb0262c2e, 0xc9441a03, 0x9fd57aa1, 0x3fa51163, 0x419c5bbb, 0x628d4b98, 0x4b324d6c, 0x84b4ebd2,
    0x9383ff3c, 0xdccf6073, 0x3cd8b6e4, 0x5864fb98, 0x2d3f1ef9, 0xed1261ae, 0x63bfadb9, 0xebbe7b80,
    0x73d808ae, 0x8b54302a, 0x18077071, 0x0be60816, 0x4972f822, 0xcf6c9542, 0x58d996c1, 0x5c8b006e,
    0xe5201163, 0x424c2155, 0x9b6c9796, 0x7c7df946, 0x4e385b7a, 0xef317069, 0x27066237, 0xeb66a245,
    0x1c8e3455, 0xcedb5dd7, 0x0f962d50, 0x86dbd724, 0xedb85e19, 0x59fee2f5, 0xf837aac0, 0xcf9edb13,
    0x015f3933, 0xd2a5ed81, 0xacc3e45c, 0x3260dc4d, 0x58ab9f96, 0x53d35f72, 0x95b5a490, 0xdec63b97,
    0x72e8f90d, 0x82432df9, 0x8bc7a100, 0x8e92774c, 0x05f6bc49, 0xf87a4482, 0x361bbcaf, 0xce9d6e45,
    0xa9ef085f, 0x1c517166, 0x0f9896b5, 0xbb7a1367, 0x4ffbe6af, 0x4a9ef65b, 0xc8786bbc, 0x7aba7995,
    0x075a2c31, 0xc7cd652c, 0x0c91c133, 0xbcd338a8, 0x1faef86c, 0x1a9b5d84, 0xeb2613f0, 0x8957955d,
    0x774b0de5, 0xa2ef9f1a, 0xe190189d, 0x295d47a7, 0x12608878, 0xa54f60d6, 0x557c9ca9, 0x038add9d,
    0xb7d916f0, 0x95de84d2, 0x07b80178, 0x94b7293f, 0x8e6752ae, 0x7efffe3f, 0x54010140, 0x73624078,
    0xed8397be, 0x1a0164ab, 0x32f65adb, 0x0611bb9c, 0x3e319b82, 0x986f9a8d, 0x31b992ee, 0xab15bdf7,
    0x38877e84, 0xcca7f93e, 0xbd66a594, 0x52e873ad, 0x759b79cf, 0x1c7a3f49, 0x49373625, 0x2f41d499,
    0x4eee1c75, 0xe2d980cb, 0x9cb973f0, 0x7220fd96, 0xacb17914, 0x374eb47c, 0x911a38f3, 0x834266e4,
    0x4bd00fdb, 0x07c8b11b, 0xbab189e0, 0x44929b9e, 0x91494b6c, 0x612b7f15, 0x605aa600, 0x2d3a24b3,
    0xb0964d98, 0x434b286b, 0xabb42be2, 0x4d28cbbf, 0xa1336323, 0x3fc690b5, 0x32fdb1a4, 0x39571129,
    0x00dd0e66, 0x7457b223, 0xfe7c087e, 0x83941052, 0x8f2a196b, 0x22aea63d, 0x2a2da4fb, 0x39243b20,
    0x434fa63d, 0xfb255816, 0x974f9482, 0x2209bb2e, 0x2f5d9d61, 0xd70e11d4, 0x95953789, 0x0d5f0288,
    0x9841849a, 0x8ddfeb7a, 0x6baf6804, 0xe3b6ef53, 0xaa0d3ed5, 0x63fe111c, 0x4e25ccc3, 0x01874f21,
    0x242461ad, 0x18744e61, 0xdd61270e, 0x6c5538f1, 0x1b5c0a65, 0x5b481daa, 0xa44eddc1, 0x28252773,
    0x3af4fb6f, 0x8e02f4ff, 0x557a93ec, 0xd8219107, 0xb45678df, 0xd82400e3, 0xcf9fd27e, 0x1c4c18fa,
    0x1c671c4f, 0xe5aa0d23, 0x86dde8ae, 0xf34bf5fc, 0x92f759bb, 0x1c7a9f7b, 0x2fd93128, 0xfeb6c287,
    0x0d275d1c, 0xa840d6a3, 0x07ef1c03, 0x2aefce4f, 0xeced39e9, 0x854283b3, 0x36436364, 0x9ffcfdf6
};

static uint32_t
_trng_getValueTimer0()
{
    return ROM_TimerValueGet(TIMER0_BASE, TIMER_A);
}

static uint32_t
_trng_getValueWatchdog1()
{
    return ROM_WatchdogValueGet(WATCHDOG1_BASE);
}

static void
_trng_wdog1Init()
{
    //     - Period (16M ciclos sao 1 segundo)
    const uint32_t period = (16*1000*1000) * 5;

    //     - aciona o periferico
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_WDOG1);

    //     - aguarda o periferico ser inicializado
    while( !ROM_SysCtlPeripheralReady(SYSCTL_PERIPH_WDOG1) );

    //     - valor a carregar quando o watchdog resetar
    ROM_WatchdogReloadSet(WATCHDOG1_BASE, period);
    //     - Watchdog vai ser pausado durante uma sessao de debug
    ROM_WatchdogStallEnable(WATCHDOG1_BASE);
    //     - Aciona a interrupcao do watchdog
    ROM_WatchdogIntEnable(WATCHDOG1_BASE);
    ROM_IntEnable(INT_WATCHDOG);
}

static void
_trng_storeBits(uint32_t ui32val, uint8_t ui8bits)
{
#if defined(DEBUG)
    volatile uint32_t result;
    result = EntropyPool_Store32(&_trng.pp, ui32val, ui8bits);
#else
    (void)EntropyPool_Store32(&_trng.pp, ui32val, ui8bits);
#endif
}

static void
_trng_simple(void)
{
    //este TRNG aproveita apenas os bits menos significativos
    //dos contadores.
    //Eh um bom TRNG pois sao os bits mais imprevisiveis.
    uint32_t rand = 0;
    uint32_t valTimer = _trng_getValueTimer0();
    uint32_t valWtdog = _trng_getValueWatchdog1();

    valTimer <<= 1;
    valWtdog <<= 0;

    rand ^= valWtdog;
    rand ^= valTimer;

    return _trng_storeBits(rand, _trng.config.bit_qtt);
}

static void
_trng_simpleSingle(void)
{
    //Este TRNG captura o valor dos contadores
    //E os combina (XOR) numa unica variavel.
    uint32_t rand = 0;

    rand ^= _trng_getValueTimer0();
    rand ^= _trng_getValueWatchdog1();
    
    ///Armazena os bits menos significativos.
    ///A quantidade de bits eh informada na configuracao
    ///Os bits menos significativos tem maior qualidade...
    /// ...pois sao mais imprevisiveis.
    return _trng_storeBits(rand, _trng.config.bit_qtt);
}

static void
_trng_allBytes(void)
{
    //este TRNG aproveita todos os bytes dos contadores,
    //misturando-os numa unica palavra de 8 bits.
    static uint32_t rand = 0;
    uint32_t valTimer = _trng_getValueTimer0();
    uint32_t valWtdog = _trng_getValueWatchdog1();

    //Mistura todos os bytes da palavra em 8 bits...
    //os bytes mais significativos tem pior qualidade pois sao previsiveis
    rand ^= (valTimer >> (8*0));
    rand ^= (valTimer >> (8*1));
    rand ^= (valTimer >> (8*2));

    rand ^= (valWtdog >> (8*0));
    rand ^= (valWtdog >> (8*1));
    rand ^= (valWtdog >> (8*2));

    //Armazena os bits menos significativos
    //A quantidade de bits eh informada na configuracao
    //8 bits eh o maximo
    return _trng_storeBits(rand, _trng.config.bit_qtt);
}

void
_interrupt_timer0(void)
{
    static uint32_t status = 0;
    static volatile apptick_t tick;
    volatile uint32_t total;

    ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    // TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    if( status++ & 1 )
    {
        AppLedSetColor(eAppLedColorGreen);
    }
    else
    {
        AppLedSetColor(eAppLedOff);
    }

    total = AppTickGetIntervalUs(tick);
    tick = AppTickGet();
    (void)total;
    (void)tick;
}

void
_interrupt_watchdog1(void)
{
    // destroi o watchdog, mas ocorre reset mesmo com o
    // bit de output de reset desligado.
    ROM_WatchdogIntClear(WATCHDOG1_BASE);
}

static void
_trng_tmr0Init()
{
    //     - Periodo de 1 segundo (80M ciclos)
    uint32_t period = ROM_SysCtlClockGet();

    //     - aciona o periferico
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

    //     - aguarda o periferico ser inicializado
    while( !ROM_SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0) );

    //     - Configurado como contador de 32 bits
    //     - Timer periodico. nao eh one-time, ao zerar sera reiniciado
    ROM_TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);

    //     - Timer vai congelar durante sessao de debug
    ROM_TimerControlStall(TIMER0_BASE, TIMER_A, true);

    //     - Valor que sera carregado apos o reset
    ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, period );

    //      - prescaler = 0 (timer incrementado a cada periodo de clock)
    ROM_TimerPrescaleSet(TIMER0_BASE, TIMER_BOTH, 0);

    ROM_TimerEnable(TIMER0_BASE, TIMER_A);
}

static res_t
_trng_setupPool( void * sPoolBuff, uint32_t ui32PoolBuffSz )
{
    if( sPoolBuff == (void *)0 )
    {
        return RES_ERROR;
    }

    if( ui32PoolBuffSz < sizeof(uint32_t) )
    {
        return RES_ERR_SZ;
    }

    (void)EntropyPool_Init(&_trng.pp, sPoolBuff, ui32PoolBuffSz);

    return RES_SUCCESS;
}

static void _trng_initializeVector(void *sVector, uint32_t ui32sz)
{
    uint32_t cpSz;

    while(ui32sz)
    {
        cpSz = MIN(ui32sz, sizeof(_trng_initVector));
        memcpy(sVector, _trng_initVector, cpSz);
        ui32sz -= cpSz;
        sVector = (uint8_t *)sVector + cpSz;
    }
}

res_t
TRNG_Configure(trng_t eType, uint8_t ui8CfgBitQtt, bool_t ubUseSeed)
{
    struct
    {
        void *d;
        uint32_t sz;
    }pool = { _trng.pp.buff.d, _trng.pp.buff.sz };

    struct
    {
        const void *d;
        uint32_t sz;
    }seed = { (const void *)_trng_initVector, sizeof(_trng_initVector) };

    //valor default e 2
    if( ui8CfgBitQtt == 0 )
    {
        ui8CfgBitQtt = 2;
    }

    switch(eType)
    {
        case trng_simple:
            _trng.config.bit_qtt = MIN(8, ui8CfgBitQtt);
            _trng.config.use_seed = ubUseSeed;
            _trng.gen_f = _trng_simple;
            break;

        case trng_simplest:
            _trng.config.bit_qtt = MIN(8, ui8CfgBitQtt);
            _trng.config.use_seed = ubUseSeed;
            _trng.gen_f = _trng_simpleSingle;
            break;

        case trng_allbytes:
            _trng.config.bit_qtt = MIN(8, ui8CfgBitQtt);
            _trng.config.use_seed = ubUseSeed;
            _trng.gen_f = _trng_allBytes;
            break;

        default:
            return RES_ERROR;
    }

    if(ubUseSeed)
    {
        while(pool.sz)
        {
            memcpy(pool.d, seed.d, MIN(pool.sz, seed.sz));
            pool.sz -= MIN(pool.sz, seed.sz);
        }
    }
    else
    {
        (void)memset(pool.d, 0x55, pool.sz);
    }
    EntropyPool_Flush(&_trng.pp);

    //gera 2 bits para limpar os registradores de benchmark
    TRNG_Task();
    TRNG_Task();

    return RES_SUCCESS;
}

res_t
TRNG_Init(void * pvPoolBuff, uint32_t ui32PoolBuffSz)
{
    res_t rr;

    if( RES_SUCCESS != (rr = _trng_setupPool(pvPoolBuff, ui32PoolBuffSz)) )
    {
        return rr;
    }

    if( 1 == 0 )
    {
        return RES_ERROR;
    }

    _trng.config.bit_qtt = 1;
    _trng.config.use_seed = false;

    if( _trng.config.use_seed )
    {
        _trng_initializeVector(pvPoolBuff, ui32PoolBuffSz);
    }
    else
    {
        (void)memset(pvPoolBuff, 0x55, ui32PoolBuffSz);
    }

    //
    // inicializa o hardware que utiliza o clock interno (IOSC)
    //
    _trng_wdog1Init();

    //
    // inicializa o hardware que utiliza o clock principal (XTAL)
    //
    _trng_tmr0Init();

    return RES_SUCCESS;
}

uint32_t
TRNG_GetCountBits(void)
{
    return EntropyPool_Bits(&_trng.pp);
}

uint32_t
TRNG_GetBitCpuTimeUs(void)
{
    return _trng.cpu_time_ns / 1000 / _trng.config.bit_qtt;
}

uint32_t
TRNG_GetBitCpuTimeNs(void)
{
    return _trng.cpu_time_ns / _trng.config.bit_qtt;
}

res_t
TRNG_SeedSave(void)
{
    uint32_t addrSeed = (uint32_t)_trng_initVector;
    uint32_t szMin = 1*1024;
    int32_t szTotal = sizeof(_trng_initVector);
    //tamanho menor de copia de flash eh 1K
    if( EntropyPool_BitsMax(&_trng.pp) < szMin )
    {
        return RES_ERROR;
    }

    while(szTotal > szMin)
    {
        //espera a entropia encher
        while(TRNG_GetCountBits() < szMin)
        {
            TRNG_Task();
        }

        if( 0 != ROM_FlashErase(addrSeed) )
        {
            return RES_ERROR;
        }
        if( 0 != ROM_FlashProgram(_trng.pp.buff.d, addrSeed, szMin) )
        {
            return RES_ERROR;
        }
        addrSeed += szMin;
        szTotal -= szMin;
    }

    return RES_SUCCESS;
}

res_t
TRNG_GetFreqGenerationHz(uint32_t *pui32freq, uint8_t *pui8bitQtt)
{
    if( pui32freq == NULL )
    {
        return RES_ERROR;
    }
    if( pui8bitQtt == NULL )
    {
        return RES_ERROR;
    }

    *pui32freq = (1*1000*1000) / TRNG_GetBitCpuTimeUs();
    *pui8bitQtt = _trng.config.bit_qtt;

    return RES_SUCCESS;
}

uint32_t
TRNG_Read(void * svBuff, uint32_t ui32BuffSz)
{
    return EntropyPool_RdMany(&_trng.pp, svBuff, ui32BuffSz);
}

void TRNG_Task(void)
{
    static apptick_t timerCpu = 0;

    if( _trng.gen_f )
    {
        _trng.gen_f();
        _trng.cpu_time_ns = AppTickGetIntervalNs(timerCpu);
        timerCpu = AppTickGet();
    }
}
