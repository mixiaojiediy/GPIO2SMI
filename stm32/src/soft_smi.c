#include <stdio.h>
#include "soft_smi.h"
#include "bsp_SysTick.h"
#include "shell.h"
#include "board.h"
#include "common.h"


#define SMI_DEBUG_PRINTF 1 // 调试信息打印

#define SMI_FRAME_PREAMBLE_VALUE 0xFFFFFFFF // 前导码，32bit
#define SMI_FRAME_PREAMBLE_BIT_LEN   32

#define SMI_FRAME_ST_VALUE 0x1 // 帧起始，2bit
#define SMI_FRAME_ST_BIT_LEN   2

#define SMI_FRAME_OP_READ_VALUE 0x2 // 读取操作码，2bit
#define SMI_FRAME_OP_WRITE_VALUE 0x1 // 写入操作码，2bit
#define SMI_FRAME_OP_BIT_LEN   2

#define SMI_FRAME_WRITE_TA 0x2 // 写入操作TA值：10b
#define SMI_FRAME_WRITE_TA_BIT_LEN 2 // SMI写周转位长度

#define SMI_FRAME_READ_TA_BIT_LEN 2 // SMI读周转位长度
#define SMI_FRAME_DATA_BIT_LEN 16 // SMI数据位长度

#define SMI_FRAME_PRADDR_BIT_LEN 10 // phy地址+reg地址位长度

#define smi_delay Delay_us(2) // SMI延时

/* smi句柄 */
smi_portcfg_t g_smi_portcfg = {0};

/**
 * @brief 设置MDC输出低电平
 * 
 */
void smi_set_mdc_low(void)
{
    HAL_GPIO_WritePin(g_smi_portcfg.MDC_PORT, g_smi_portcfg.MDC_PIN, GPIO_PIN_RESET);
}

/**
 * @brief 设置MDC输出高电平
 * 
 */
void smi_set_mdc_high(void)
{
    HAL_GPIO_WritePin(g_smi_portcfg.MDC_PORT, g_smi_portcfg.MDC_PIN, GPIO_PIN_SET);
}

/**
 * @brief 设置MDIO输出低电平
 * 
 */
void smi_set_mdio_low(void)
{
    HAL_GPIO_WritePin(g_smi_portcfg.MDIO_PORT, g_smi_portcfg.MDIO_PIN, GPIO_PIN_RESET);
}

/**
 * @brief 设置MDIO输出高电平
 * 
 */
void smi_set_mdio_high(void)
{
    HAL_GPIO_WritePin(g_smi_portcfg.MDIO_PORT, g_smi_portcfg.MDIO_PIN, GPIO_PIN_SET);
}

/**
 * @brief 获取MDIO输入电平
 * 
 * @return uint8_t 0-低电平 1-高电平
 */
uint8_t smi_read_mdio(void)
{
    return HAL_GPIO_ReadPin(g_smi_portcfg.MDIO_PORT, g_smi_portcfg.MDIO_PIN);
}

/**
 * @brief 使能SMI端口时钟
 * 
 * @param port 端口地址
 */
void smi_enable_clk(GPIO_TypeDef *port)
{
    switch ((uint32_t )port)
    {
        case GPIOA_BASE:
            __HAL_RCC_GPIOA_CLK_ENABLE();
            break;
        case GPIOB_BASE:
            __HAL_RCC_GPIOB_CLK_ENABLE();
            break;
        case GPIOC_BASE:
            __HAL_RCC_GPIOC_CLK_ENABLE();
            break;
        case GPIOD_BASE:
            __HAL_RCC_GPIOD_CLK_ENABLE();
            break;
        case GPIOE_BASE:
            __HAL_RCC_GPIOE_CLK_ENABLE();
            break;
        case GPIOF_BASE:
            __HAL_RCC_GPIOF_CLK_ENABLE();
            break;
        case GPIOG_BASE:
            __HAL_RCC_GPIOG_CLK_ENABLE();
            break;
        case GPIOH_BASE:
            __HAL_RCC_GPIOH_CLK_ENABLE();
            break;
        default:
            break;
    }
}

/**
 * @brief 设置SMI的MDIO为推挽输出模式
 * 
 */
void smi_set_mdio_ouput(void)
{
    GPIO_InitTypeDef GPIO_Initure;
    /* 配置MDIO为推挽输出 */
    GPIO_Initure.Pin = g_smi_portcfg.MDIO_PIN;
    GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_Initure.Pull = GPIO_PULLUP;
    GPIO_Initure.Speed = GPIO_SPEED_HIGH;
    smi_set_mdio_high();
    HAL_GPIO_Init(g_smi_portcfg.MDIO_PORT, &GPIO_Initure);
}

/**
 * @brief 设置SMI的MDIO为上拉输入模式
 * 
 */
void smi_set_mdio_input(void)
{
    GPIO_InitTypeDef GPIO_Initure;
    /* 配置MDIO为上拉输入 */
    GPIO_Initure.Pin = g_smi_portcfg.MDIO_PIN;
    GPIO_Initure.Mode = GPIO_MODE_INPUT;
    GPIO_Initure.Pull = GPIO_PULLUP;
    GPIO_Initure.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(g_smi_portcfg.MDIO_PORT, &GPIO_Initure);
}

/**
 * @brief 初始化SMI的端口时钟、模式
 * 
 * @param smi_portcfg SMI配置句柄地址
 */
void smi_init_port(smi_portcfg_t *smi_portcfg)
{
    GPIO_InitTypeDef GPIO_Initure;
    g_smi_portcfg = *smi_portcfg;

    smi_enable_clk(g_smi_portcfg.MDC_PORT);
    smi_enable_clk(g_smi_portcfg.MDIO_PORT);

    /* 配置MDC为推挽输出 */
    GPIO_Initure.Pin = g_smi_portcfg.MDC_PIN;
    GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_Initure.Pull = GPIO_PULLUP;
    GPIO_Initure.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(g_smi_portcfg.MDC_PORT, &GPIO_Initure);


    /* 配置MDIO为上拉输入 */
    GPIO_Initure.Pin = g_smi_portcfg.MDIO_PIN;
    GPIO_Initure.Mode = GPIO_MODE_INPUT;
    GPIO_Initure.Pull = GPIO_PULLUP;
    GPIO_Initure.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(g_smi_portcfg.MDIO_PORT, &GPIO_Initure);

    smi_set_mdc_low();
}

/**
 * @brief 通过SMI写入n位数据
 * 
 * @param data 数据地址
 * @param count 数据长度（bit）
 */
void smi_write_n_bit(uint8_t *data, uint8_t count)
{
    int i;
    uint8_t writeData;
    for (i = 0; i < count; i++)
    {
        writeData = data[i / 8];
        smi_set_mdc_low();
        if ((writeData >> (7 - i % 8) & 0x01) == 0x01)
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

/**
 * @brief 通过SMI读取n位数据
 * 
 * @param data 数据地址
 * @param count 数据长度（bit）
 */
void smi_read_n_bit(uint8_t *data, uint8_t count)
{
    int i;
    for (i = 0; i < count; i++)
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

/**
 * @brief 读取PHY设备的寄存器值
 * 
 * @param phyAddr PHY设备地址
 * @param regAddr 寄存器地址
 * @return uint16_t 寄存器值
 */
uint16_t smi_read_reg(uint8_t phyAddr, uint8_t regAddr)
{
    // 切换MDIO为推挽输出模式
    smi_set_mdio_ouput();

    // 前导码：32bit 1b
    uint32_t preambleData = SMI_FRAME_PREAMBLE_VALUE;
    smi_write_n_bit((uint8_t *)&preambleData, SMI_FRAME_PREAMBLE_BIT_LEN);

    // 帧起始：2bit 01b
    uint8_t stData = SMI_FRAME_ST_VALUE << 6;
    smi_write_n_bit(&stData, SMI_FRAME_ST_BIT_LEN);

    // 操作：2bit 10b（读取）
    uint8_t opData = SMI_FRAME_OP_READ_VALUE << 6;
    smi_write_n_bit(&opData, SMI_FRAME_OP_BIT_LEN);

    // PHY地址（5bit）+ 寄存器地址（5bit）
    uint8_t praddr[2] = {0};
    praddr[0] = (phyAddr << 5);
    praddr[0] |= ((regAddr & 0x1C) >> 2);
    praddr[1] = ((regAddr & 0x3) << 6);
    
    smi_write_n_bit(praddr, SMI_FRAME_PRADDR_BIT_LEN);

    // 切换MDIO为上拉输入模式
    smi_set_mdio_input();

    // 获取MDIO的TA状态，为0直接读取16bit数据
    uint8_t taData = 0;
    smi_read_n_bit(&taData, SMI_FRAME_READ_TA_BIT_LEN);

    if ((taData & 0x40) != 0x00)
    {
        printf("%s", SMI_DEBUG_PRINTF == 1 ? "Error : TA bit2 value is 1\r\n" : "");
    }
    // 读取数据，16bit
    uint8_t regValue[2] = {0};
    smi_read_n_bit(&regValue[0], 16);

    // 切换MDIO为上拉输入模式，驱动MDIO为高阻态
    smi_set_mdio_input();

    return (regValue[0] << 8) | (regValue[1]);
}


/**
 * @brief 设置PHY设备的寄存器值
 * 
 * @param phyAddr PHY设备地址
 * @param regAddr 寄存器地址
 * @param regVal 寄存器值
 */
void smi_set_reg(uint8_t phyAddr, uint8_t regAddr, uint16_t regVal)
{
    // 切换MDIO为推挽输出模式
    smi_set_mdio_ouput();

    // 前导码：32bit 1b
    uint32_t preambleData = SMI_FRAME_PREAMBLE_VALUE;
    smi_write_n_bit((uint8_t *)&preambleData, SMI_FRAME_PREAMBLE_BIT_LEN);

    // 帧起始：2bit 01b
    uint8_t stData = SMI_FRAME_ST_VALUE << 6;
    smi_write_n_bit(&stData, SMI_FRAME_ST_BIT_LEN);

    // 操作：2bit 10b（读取）
    uint8_t opData = SMI_FRAME_OP_WRITE_VALUE << 6;
    smi_write_n_bit(&opData, SMI_FRAME_OP_BIT_LEN);

    // PHY地址（5bit）+ 寄存器地址（5bit）
    uint8_t praddr[2] = {0};
    praddr[0] = (phyAddr << 5);
    praddr[0] |= ((regAddr & 0x1C) >> 2);
    praddr[1] = ((regAddr & 0x3) << 6);
    smi_write_n_bit(praddr, SMI_FRAME_PRADDR_BIT_LEN);

    // 设置MDIO的TA状态，2bit，10b
    uint8_t taData = SMI_FRAME_WRITE_TA;
    smi_write_n_bit(&taData, SMI_FRAME_WRITE_TA_BIT_LEN);

    // 设置，16bit
    uint8_t regValue[2] = {0};
    regValue[0] = regVal >> 8;
    regValue[1] = regVal & 0xFF;
    smi_write_n_bit(&regValue[0], 16);

    // 切换MDIO为上拉输入模式，驱动MDIO为高阻态
    smi_set_mdio_input();
}

int rdphy(int argc, char *argv[])
{
    u16 phyAddr, regAddr;
    u16 regVal;
    if (argc != 3)
    {
        printf("Param1:phy addr\r\n");
        printf("Param2:reg addr\r\n");
        return -1;
    }
    phyAddr = StrToInt(argv[1]);
    regAddr = StrToInt(argv[2]);
    regVal = smi_read_reg(phyAddr, regAddr);
    printf("Phy addr : 0x%04X\r\n", phyAddr);
    printf("Reg addr : 0x%04X\r\n", regAddr);
    printf("Reg val  : 0x%04X\r\n", regVal);
    return 0;
}

int setphy(int argc, char *argv[])
{
    u16 phyAddr, regAddr;
    u16 regVal;
    if (argc != 4)
    {
        printf("Param1:phy addr\r\n");
        printf("Param2:reg addr\r\n");
        printf("Param3:reg val\r\n");
        return -1;
    }
    phyAddr = StrToInt(argv[1]);
    regAddr = StrToInt(argv[2]);
    regVal = StrToInt(argv[3]);
    smi_set_reg(phyAddr, regAddr, regVal);
    printf("Phy addr : 0x%04X\r\n", phyAddr);
    printf("Reg addr : 0x%04X\r\n", regAddr);
    printf("Reg val  : 0x%04X\r\n", regVal);
    return 0;
}

int testphy(int argc, char *argv[])
{
    u64 cnt = 0;
    u16 val = 0;
    for (;;)
    {
        val++;
        smi_set_reg(0, 2, val);
        if (smi_read_reg(0, 2) != val)
        {
            printf("Test phy fail, cnt : %lld\r\n",  cnt);
            break;
        }
        cnt++;
        if ((cnt % 10000) == 0)
        {
            printf("Test phy cnt : %lld\r\n", cnt);
        }
        Raise_Dog();
    }
    return 0;
}


void soft_smi_init(void)
{
    smi_portcfg_t smi_portcfg_setting;
    smi_portcfg_setting.MDC_PIN = GPIO_PIN_1;
    smi_portcfg_setting.MDC_PORT = GPIOC;
    smi_portcfg_setting.MDIO_PIN = GPIO_PIN_2;
    smi_portcfg_setting.MDIO_PORT = GPIOA;

    smi_init_port(&smi_portcfg_setting);
    
    AddCmd("testphy", "testphy", cmd, sys, testphy);
    AddCmd("rdphy", "rdphy", cmd, sys, rdphy);
    AddCmd("setphy", "setphy", cmd, sys, setphy);
}
