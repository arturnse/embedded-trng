
#ifndef APPLICATION_TRNG_H_
#define APPLICATION_TRNG_H_

#include "api_config.h"

typedef enum
{
    trng_simplest,      ///< TRNG mais simples
    trng_simple,        ///< TRNG simples.
    trng_allbytes,      ///< TRNG processa todos os bytes dos clocks
}trng_t;

//!
//! Configura o TRNG escolhido
//!
//! \param eType            Tipo do gerador que sera configurado.
//! \param ui8CfgBitQtt     Configuracao de quantos bits serao gerados
//! por ciclo. Para usar o valor minimo basta informar \b 0 (zero)
//! \param ubUseSeed        Antes de gerar os bytes, carrega uma seed
//! no pool.
//!
//! \return \b RES_SUCCESS caso a configuracao tenha sucesso.
//!
res_t
TRNG_Configure(trng_t eType, uint8_t ui8CfgBitQtt, bool_t ubUseSeed);

//!
//! Inicializa o modulo
//!
//! Eh necessario informar um buffer de dados.
//!
//! \param sPoolBuff        e o ponteiro pro buffer de dados
//! \param ui32PoolBuffSz   e o tamanho do buffer, em bytes.
//!
//! \return \b RES_SUCCESS caso a configuracao tenha sucesso
//!
res_t
TRNG_Init(void * pvPoolBuff, uint32_t ui32PoolBuffSz);

//!
//! Informa quantos bits aleatorios estao disponiveis
//!
//! \return Bits disponiveis para leitura
//!
uint32_t
TRNG_GetCountBits(void);

//!
//! Informa quanto tempo de processamento foi necessario
//! para ler os ultimos bit da entropia, em
//! microssegundos ( 1s * 10^(-6) )
//!
//! \return tempo em microssegundos necessario para capturar
//! um unico bit
//!
uint32_t
TRNG_GetBitCpuTimeUs(void);

//!
//! Informa quanto tempo de processamento foi necessario
//! para ler os ultimos bits da entropia, em
//! nanossegundos ( 1s * 10^(-9) )
//!
//! \return tempo em nanossegundos necessario para capturar
//! um unico bit
//!
uint32_t
TRNG_GetBitCpuTimeNs(void);

//!
//! Salva uma nova seed do TRNG.
//!
//! \return \b RES_SUCCESS caso a nova seed tenha sido salva
//!
res_t
TRNG_SeedSave(void);

//!
//! Informa a frequencia de chamada da funcao de captura
//! de entropia em Hz
//!
//! \param pui32freq Ponteiro para a frequencia
//! \param pui8bitQtt numero de bits capturados em cada
//! processamento
//!
//! \return \b RES_SUCCESS caso os valores retornados
//! sejam validos
//!
res_t
TRNG_GetFreqGenerationHz(uint32_t *pui32freq, uint8_t *pui8bitQtt);

//!
//! Le bytes gerados
//!
//! \param svBuff e o ponteiro que contera os dados
//! \param ui32BuffSz e o tamanho maximo do ponteiro, em bytes
//!
//! \return o numero de bytes copiados para o buffer.
//!
uint32_t
TRNG_Read(void * svBuff, uint32_t ui32BuffSz);


//!
//! Tarefa da aplicacao
//!
//! \return None.
//!
void TRNG_Task(void);

#endif /* APPLICATION_TRNG_H_ */
