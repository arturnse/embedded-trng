
#include "pool.h"

static INLINE uint32_t
_pool_updateIndex( uint32_t ui32id, uint32_t ui32szUpdate, uint32_t ui32szMax )
{
    ui32id += ui32szUpdate;

    if( ui32id >= ui32szMax )
    {
        ui32id -= ui32szMax;
    }

    return ui32id;
}

uint32_t
Pool_Init(pool_t *psPool, void * const psBuff, uint32_t ui32sz)
{
    if( psPool == NULL )
    {
        return 0;
    }
    if( psBuff == NULL )
    {
        return 0;
    }
    if( ui32sz == 0 )
    {
        return 0;
    }

    psPool->buff.d = psBuff;
    psPool->buff.sz = ui32sz;
    psPool->buff.head = psPool->buff.tail = psPool->full = psPool->entropy_cur = 0;
    psPool->f = ePoolInitialized;
    return psPool->buff.sz * 8;
}

uint32_t
Pool_Store(pool_t *psPool, uint32_t ui32entropy, uint32_t ui32eBits)
{
    uint8_t * pool;
    uint8_t entropy;
    uint32_t posBit;

    if( psPool == NULL )
    {
        return 0;
    }
    if( (psPool->f & ePoolInitialized) == 0 )
    {
        return 0;
    }

    ui32eBits %= (sizeof(ui32entropy)*8);   //maximo e 32 bits
    entropy = (uint8_t)ui32eBits;

    while( ui32eBits-- )
    {
        pool = psPool->buff.d;
        posBit = psPool->entropy_cur % 8;

        pool[psPool->buff.tail] ^= (ui32entropy & 1) << posBit;

        ui32entropy >>= 1;
        psPool->entropy_cur += 1;

        if( (psPool->entropy_cur % 8) == 0 )
        {
            if( psPool->full )
            {
                //sobrescreve o inicio do buffer
                psPool->buff.head = _pool_updateIndex(psPool->buff.head, 1, psPool->buff.sz);
            }
            //incrementa o ponteiro
            psPool->buff.tail = _pool_updateIndex(psPool->buff.tail, 1, psPool->buff.sz);
            //verifica se encheu
            psPool->full = psPool->buff.tail == psPool->buff.head;
        }
    }

    return entropy;
}

uint32_t
Pool_Bits(pool_t *psPool)
{
    return Pool_Used(psPool) * 8;
}

uint32_t
Pool_BitsMax(pool_t *psPool)
{
    return Pool_Sz(psPool) * 8;
}

uint32_t
Pool_RdOnePtr(pool_t *psPool, void **pEl)
{
    uint8_t *ptr;

    if( psPool == NULL )
    {
        return 0;
    }
    if( Pool_IsEmpty(psPool) )
    {
        return 0;
    }

    ptr = &((uint8_t*)psPool->buff.d)[psPool->buff.head];

    psPool->buff.head = _pool_updateIndex(psPool->buff.head, 1, psPool->buff.sz);
    psPool->full = 0;

    if( pEl == NULL )
    {
    }
    else
    {
        *pEl = (void *)ptr;
    }

    return 1;
}

uint32_t Pool_RdOne(pool_t *psPool, void *pEl)
{
    uint8_t *ptr;

    if( psPool == NULL )
    {
        return 0;
    }
    if( Pool_IsEmpty(psPool) )
    {
        return 0;
    }

    ptr = &((uint8_t*)psPool->buff.d)[psPool->buff.head];

    psPool->buff.head = _pool_updateIndex(psPool->buff.head, 1, psPool->buff.sz);
    psPool->full = 0;

    if( pEl == NULL )
    {
    }
    else
    {
        memcpy(pEl, ptr, 1);
    }

    return 1;
}

uint32_t Pool_RdMany(pool_t *psPool, void *pvRdData, uint32_t ui32sz)
{
    uint32_t result;
    uint8_t *ptr = pvRdData;
    uint32_t cpSz;

    if( pvRdData == NULL )
    {
        return Pool_Discard( psPool, ui32sz );
    }

    if( psPool == NULL )
    {
        return 0;
    }

    if( ! (psPool->f & ePoolInitialized) )
    {
        return 0;
    }

    result = ui32sz = MIN( ui32sz, Pool_Used(psPool) );

    //segmenta em duas partes: ate o final do buffer e entao do inicio ate o meio.
    // assim: h = head t = tail
    //
    //  |=====t---------------------h=======| pega o tail ate o final
    //  |h====t-----------------------------| pega o inicio ate o head
    //  |----ht-----------------------------|

    if( psPool->buff.head + (1 * ui32sz) > psPool->buff.sz )
    {
        ptr = &((uint8_t*)psPool->buff.d)[psPool->buff.head];

        cpSz = psPool->buff.sz - psPool->buff.head;

        memcpy(pvRdData, ptr, cpSz);

        //por isso o buffer precisa estar alinhado!
        psPool->buff.head = _pool_updateIndex(psPool->buff.head, cpSz, psPool->buff.sz );
        psPool->full = 0;

        pvRdData = &(((uint8_t*)pvRdData)[cpSz]);

        //numero de elementos retirados:
        ui32sz -= (cpSz/1);
    }

    ptr = &((uint8_t*)psPool->buff.d)[psPool->buff.head];

    cpSz = (ui32sz*1);

    memcpy(pvRdData, ptr, cpSz);

    psPool->buff.head = _pool_updateIndex(psPool->buff.head, cpSz, psPool->buff.sz );
    if( ui32sz > 0 )
    {
        psPool->full = 0;
    }

    return result;
}

uint32_t Pool_Discard(pool_t *psPool, uint32_t ui32sz)
{

    if( psPool == NULL )
    {
        return 0;
    }

    ui32sz = MIN( ui32sz, Pool_Used(psPool) );

    psPool->buff.head = _pool_updateIndex(psPool->buff.head, ui32sz, psPool->buff.sz );
    if( ui32sz > 0 )
    {
        psPool->full = 0;
    }

    return ui32sz;

}

void Pool_Flush(pool_t *psPool)
{
    if( psPool == NULL )
    {
        return;
    }

    psPool->buff.head = psPool->buff.tail = psPool->full = 0;

    return;
}

uint32_t Pool_Used(pool_t *psPool)
{
    if( psPool == NULL )
    {
        return 0;
    }
    if( ! (psPool->f & ePoolInitialized) )
    {
        return 0;
    }

    if( (psPool->buff.head == psPool->buff.tail) && !(psPool->full) )
    {
        return 0;
    }
    else if(psPool->buff.tail > psPool->buff.head)
    {
        return (psPool->buff.tail - psPool->buff.head);
    }
    else
    {
        return (psPool->buff.sz - (psPool->buff.head - psPool->buff.tail));
    }
}

bool_t Pool_IsEmpty(pool_t *psPool)
{
    return (bool_t)(Pool_Used(psPool) == 0);
}

bool_t Pool_IsFull(pool_t *psPool)
{
    return (bool_t)(Pool_Sz(psPool) == Pool_Used(psPool));
}

uint32_t Pool_Sz(pool_t *psPool)
{
    if( psPool == NULL )
    {
        return 0;
    }
    if( ! (psPool->f & ePoolInitialized) )
    {
        return 0;
    }
    else
    {
        return psPool->buff.sz;
    }

}
