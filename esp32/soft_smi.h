#ifndef _SOFT_SMI_H
#define _SOFT_SMI_H

typedef struct
{
    uint8_t MDC_PIN;
    uint8_t MDIO_PIN;
} smi_portcfg_t;

void smi_set_mdc_low(void);
void smi_set_mdc_high(void);
void smi_set_mdio_low(void);
void smi_set_mdio_high(void);
uint8_t smi_read_mdio(void);
void smi_set_mdio_output(void);
void smi_set_mdio_input(void);
void smi_init_port(smi_portcfg_t *smi_portcfg);
void smi_write_n_bit(uint8_t *data, uint8_t count);
void smi_read_n_bit(uint8_t *data, uint8_t count);
uint16_t smi_read_reg(uint8_t phyAddr, uint8_t regAddr);
void smi_set_reg(uint8_t phyAddr, uint8_t regAddr, uint16_t regVal);
void soft_smi_init(void);

#endif
