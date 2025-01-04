#ifndef _SOFT_SMI
#define _SOFT_SMI

#include "stm32f4xx.h"

typedef struct
{
    uint32_t MDC_PIN;
    uint32_t MDIO_PIN;
    GPIO_TypeDef  *MDC_PORT;
    GPIO_TypeDef  *MDIO_PORT;
} smi_portcfg_t;

extern void smi_init_port(smi_portcfg_t *smi_portcfg);
extern void soft_smi_init(void);

#endif

