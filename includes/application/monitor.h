
#ifndef APPLICATION_MONITOR_H_
#define APPLICATION_MONITOR_H_

#include "api_config.h"

//!
//! Inicializa o monitor
//!
//! Inicializa o hardware e as funcoes do monitor
//!
//! \return None.
//!
void AppMonitorInit(void * pui8buff, uint16_t pu16buffSz);

//!
//! Tarefa 
//!
//! \return None.
//!
void AppMonitorTask(void);


#endif /* APPLICATION_MONITOR_H_ */
