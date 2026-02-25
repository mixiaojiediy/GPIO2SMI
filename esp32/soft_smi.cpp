#include <Arduino.h>
#include "soft_smi.h"

#define SMI_DEBUG_PRINTF 1 // Debug information printing

#define SMI_FRAME_PREAMBLE_VALUE 0xFFFFFFFF // Preamble, 32 bits
#define SMI_FRAME_PREAMBLE_BIT_LEN 32

#define SMI_FRAME_ST_VALUE 0x1 // Frame start, 2 bits
#define SMI_FRAME_ST_BIT_LEN 2

#define SMI_FRAME_OP_READ_VALUE 0x2  // Read operation code, 2 bits
#define SMI_FRAME_OP_WRITE_VALUE 0x1 // Write operation code, 2 bits
#define SMI_FRAME_OP_BIT_LEN 2

#define SMI_FRAME_WRITE_TA 0x2       // Write operation TA value: 10b
#define SMI_FRAME_WRITE_TA_BIT_LEN 2 // SMI write turnaround bit length

#define SMI_FRAME_READ_TA_BIT_LEN 2 // SMI read turnaround bit length
#define SMI_FRAME_DATA_BIT_LEN 16   // SMI data bit length

#define SMI_FRAME_PRADDR_BIT_LEN 10 // PHY address + reg address bit length

#define smi_delay delayMicroseconds(10) // SMI delay (2 microseconds)

/* SMI handle */
smi_portcfg_t g_smi_portcfg = {0};

void smi_set_mdc_low(void)
{
    digitalWrite(g_smi_portcfg.MDC_PIN, LOW);
}

void smi_set_mdc_high(void)
{
    digitalWrite(g_smi_portcfg.MDC_PIN, HIGH);
}

void smi_set_mdio_low(void)
{
    digitalWrite(g_smi_portcfg.MDIO_PIN, LOW);
}

void smi_set_mdio_high(void)
{
    digitalWrite(g_smi_portcfg.MDIO_PIN, HIGH);
}

uint8_t smi_read_mdio(void)
{
    return digitalRead(g_smi_portcfg.MDIO_PIN);
}

void smi_set_mdio_output(void)
{
    pinMode(g_smi_portcfg.MDIO_PIN, OUTPUT);
    smi_set_mdio_high();
}

void smi_set_mdio_input(void)
{
    pinMode(g_smi_portcfg.MDIO_PIN, INPUT_PULLUP);
}

void smi_init_port(smi_portcfg_t *smi_portcfg)
{
    g_smi_portcfg = *smi_portcfg;

    pinMode(g_smi_portcfg.MDC_PIN, OUTPUT);
    pinMode(g_smi_portcfg.MDIO_PIN, INPUT_PULLUP); // Start with MDIO in input mode

    smi_set_mdc_low();
}

void smi_write_n_bit(uint8_t *data, uint8_t count)
{
    for (int i = 0; i < count; i++)
    {
        uint8_t writeData = data[i / 8];
        smi_set_mdc_low();
        if ((writeData >> (7 - i % 8)) & 0x01)
        {
            smi_set_mdio_high();
        }
        else
        {
            smi_set_mdio_low();
        }
        smi_delay;
        smi_set_mdc_high();
        smi_delay;
    }
    smi_set_mdc_low();
}

void smi_read_n_bit(uint8_t *data, uint8_t count)
{
    for (int i = 0; i < count; i++)
    {
        smi_set_mdc_low();
        smi_delay;
        if (smi_read_mdio() == 1)
        {
            data[i / 8] |= (1 << (7 - i % 8));
        }
        smi_set_mdc_high();
        smi_delay;
    }
    smi_set_mdc_low();
}

uint16_t smi_read_reg(uint8_t phyAddr, uint8_t regAddr)
{
    smi_set_mdio_output();

    uint32_t preambleData = SMI_FRAME_PREAMBLE_VALUE;
    smi_write_n_bit((uint8_t *)&preambleData, SMI_FRAME_PREAMBLE_BIT_LEN);

    uint8_t stData = SMI_FRAME_ST_VALUE << 6;
    smi_write_n_bit(&stData, SMI_FRAME_ST_BIT_LEN);

    uint8_t opData = SMI_FRAME_OP_READ_VALUE << 6;
    smi_write_n_bit(&opData, SMI_FRAME_OP_BIT_LEN);

    uint8_t praddr[2] = {0};
    praddr[0] = (phyAddr << 3) | ((regAddr & 0x1C) >> 2);
    praddr[1] = (regAddr & 0x3) << 6;
    smi_write_n_bit(praddr, SMI_FRAME_PRADDR_BIT_LEN);

    smi_set_mdio_input();

    uint8_t taData = 0;
    smi_read_n_bit(&taData, SMI_FRAME_READ_TA_BIT_LEN);

    if ((taData & 0x40) != 0x00)
    {
        Serial.println("Error: TA bit2 value is 1");
    }

    uint8_t regValue[2] = {0};
    smi_read_n_bit(&regValue[0], 16);

    smi_set_mdio_input();

    return (regValue[0] << 8) | regValue[1];
}

void smi_set_reg(uint8_t phyAddr, uint8_t regAddr, uint16_t regVal)
{
    smi_set_mdio_output();

    uint32_t preambleData = SMI_FRAME_PREAMBLE_VALUE;
    smi_write_n_bit((uint8_t *)&preambleData, SMI_FRAME_PREAMBLE_BIT_LEN);

    uint8_t stData = SMI_FRAME_ST_VALUE << 6;
    smi_write_n_bit(&stData, SMI_FRAME_ST_BIT_LEN);

    uint8_t opData = SMI_FRAME_OP_WRITE_VALUE << 6;
    smi_write_n_bit(&opData, SMI_FRAME_OP_BIT_LEN);

    uint8_t praddr[2] = {0};
    praddr[0] = (phyAddr << 3) | ((regAddr & 0x1C) >> 2);
    praddr[1] = (regAddr & 0x3) << 6;
    smi_write_n_bit(praddr, SMI_FRAME_PRADDR_BIT_LEN);

    uint8_t taData = SMI_FRAME_WRITE_TA << 6;
    smi_write_n_bit(&taData, SMI_FRAME_WRITE_TA_BIT_LEN);

    uint8_t regValue[2] = {regVal >> 8, regVal & 0xFF};
    smi_write_n_bit(&regValue[0], 16);

    smi_set_mdio_input();
}

void soft_smi_init(void)
{
    smi_portcfg_t smi_portcfg_setting;
    smi_portcfg_setting.MDC_PIN = 21;  // D21
    smi_portcfg_setting.MDIO_PIN = 22; // D22

    smi_init_port(&smi_portcfg_setting);
}
