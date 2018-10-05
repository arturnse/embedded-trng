

#ifndef APPLICATION_POOL_H_
#define APPLICATION_POOL_H_

#include "api_config.h"

typedef struct
{
    struct
    {
        void * d;
        uint32_t head;
        uint32_t tail;
        uint32_t sz;
    }buff;
    uint32_t entropy_cur;

    uint32_t full;
    enum
    {
        ePoolInitialized = BF_BIT(5),
    }f;
}pool_t;

//!
//! Inicializa e configura o modulo
//!
//! \param psPool e o ponteiro pro pool
//! \param psBuff buffer que armazenara o pool
//! \param ui32sz tamanho em bytes do ponteiro que armazenara o pool
//!
//! \return ui32sz Quantidade de bits que o pool suporta.
//!
uint32_t
Pool_Init(pool_t *psPool, void * const psBuff, uint32_t ui32sz);

//!
//! Armazena entropia no pool.
//!
//! \param psPool e o ponteiro pro pool
//! \param ui32entropy e a palavra little-endian contendo os bits lidos da entropia.
//! \param ui32sz quantidade de entropia. Maximo sao \b 32 bits.
//!
//! \return \b quantidade de bits adicionados.
//!
uint32_t
Pool_Store(pool_t *psPool, uint32_t ui32entropy, uint32_t ui32eBits);

//!
//! Informa quantos bits estao disponiveis no pool.
//!
//! \param psPool e o ponteiro pro pool
//!
//! \return \b numero de bits de entropia.
//!
uint32_t
Pool_Bits(pool_t *psPool);

//!
//! Informa o maximo de bits que o pool suporta.
//!
//! \param psPool e o ponteiro pro pool
//!
//! \return \b numero de bits
//!
uint32_t
Pool_BitsMax(pool_t *psPool);

uint32_t
Pool_RdOnePtr(pool_t *psPool, void **pEl);

uint32_t
Pool_RdOne(pool_t *psPool, void *pEl);

uint32_t
Pool_RdMany(pool_t *psPool, void *pRdBuff, uint32_t pN);

uint32_t
Pool_Discard(pool_t *psPool, uint32_t ui32sz);

void
Pool_Flush(pool_t *psPool);

uint32_t
Pool_Used(pool_t *psPool);

bool_t
Pool_IsEmpty(pool_t *psPool);

bool_t
Pool_IsFull(pool_t *psPool);

uint32_t
Pool_Sz(pool_t *psPool);

#endif /* APPLICATION_POOL_H_ */
