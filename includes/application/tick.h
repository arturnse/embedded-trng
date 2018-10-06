
#ifndef APPLICATION_TICK_H_
#define APPLICATION_TICK_H_

#include "api_config.h"
#include "general_defs.h"

typedef uint32_t apptick_t;
typedef struct
{
    uint32_t ms;
    uint32_t tick;
}apptickcomposed_t;

//!
//!  Inicializa a aplicacao
//!
//!  \return None.
//!
void AppTickInit(void);

//!
//!  Informa o RTC da peca
//!
//!  \return Numero de segundos desde que a peca foi acionada
//!
uint32_t AppTick_TimeS(void);

//!
//!
//! Informa o periodo do clock principal, em Hz.
//!
//! Este clock e apenas informativo.
//!
//! \return O valor do Clock principal, em Hz.
//!
uint32_t AppTickClockPeriodHz(void);

//!
//! Captura o Tick atual.
//!
//! Este Tick e util para calcular via software
//! o tempo gasto em nas funcoes executadas.
//!
//! \return O valor do tick, armazenado em apptick_t
apptick_t AppTickGet(void);


//!
//! Informa o intervalo do tick, em us.
//!
//! \param tLastTick e o tick coletado utilizando a funcao AppTickGet
//!
//! \return O tempo gasto em microssegundos
//!
uint32_t AppTickGetIntervalUs(apptick_t tLastTick);

//!
//! Informa o intervalo do tick, em ns.
//!
//! \param tLastTick e o tick coletado utilizando a funcao AppTickGet
//!
//! \return O tempo gasto em nanossegundos
//!
uint32_t AppTickGetIntervalNs(apptick_t tLastTick);

//!
//! Verifica se houve timeout de tempo.
//!
//! \param ui32timeoutNs Timeout em nanossegundos (1s * 10^(-9))
//! \param tLastTick Tick utilizado no temporizador
//!
//! Caso o timeout estoure, seu valor sera preparado
//! automaticamente para o proximo disparo.
//!
//! \return \b true se o timeout tiver ocorrido.
//!
bool_t AppTickTimeoutNs(uint32_t ui32timeoutNs, apptick_t *tLastTick);

//!
//! Verifica se houve timeout de tempo.
//!
//! \param ui32timeoutUs Timeout em microssegundos (1s * 10^(-6))
//! \param tLastTick Tick utilizado no temporizador
//!
//! Caso o timeout estoure, seu valor sera preparado
//! automaticamente para o proximo disparo.
//!
//! \return \b true se o timeout tiver ocorrido.
//!
bool_t AppTickTimeoutUs(uint32_t ui32timeoutUs, apptick_t *tLastTick);
#define AppTickTimeoutMs(_ms,_tick) AppTickTimeoutUs((_ms*1000), (_tick))


#endif /* APPLICATION_TICK_H_ */
