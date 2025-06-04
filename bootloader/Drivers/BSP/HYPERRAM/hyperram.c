/**
 ****************************************************************************************************
 * @file        hyperram.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2024-05-21
 * @brief       HyperRAM��������
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 * 
 * ʵ��ƽ̨:����ԭ�� H7R7������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 * 
 ****************************************************************************************************
 */

#include "./BSP/HYPERRAM/hyperram.h"

#define HYPERRAM_IDENTIFICATION_REGISTER_0  ((0x00000000UL) << 1)
#define HYPERRAM_IDENTIFICATION_REGISTER_1  ((0x00000001UL) << 1)
#define HYPERRAM_CONFIGURATION_REGISTER_0   ((0x00000800UL) << 1)
#define HYPERRAM_CONFIGURATION_REGISTER_1   ((0x00000801UL) << 1)

#define HYPERRAM_IDENTIFICATION_0_POS_MANUFACTURER  (0)
#define HYPERRAM_IDENTIFICATION_0_POS_COL_COUNT     (8)
#define HYPERRAM_IDENTIFICATION_0_POS_ROW_COUNT     (4)
#define HYPERRAM_IDENTIFICATION_1_POS_DEVICE_TYPE   (0)

#define HYPERRAM_IDENTIFICATION_0_MASK_MANUFACTURER ((uint16_t)0x000F << HYPERRAM_IDENTIFICATION_0_POS_MANUFACTURER)
#define HYPERRAM_IDENTIFICATION_0_MASK_COL_COUNT    ((uint16_t)0x000F << HYPERRAM_IDENTIFICATION_0_POS_COL_COUNT)
#define HYPERRAM_IDENTIFICATION_0_MASK_ROW_COUNT    ((uint16_t)0x001F << HYPERRAM_IDENTIFICATION_0_POS_ROW_COUNT)
#define HYPERRAM_IDENTIFICATION_1_MASK_DEVICE_TYPE  ((uint16_t)0x000F << HYPERRAM_IDENTIFICATION_1_POS_DEVICE_TYPE)

#define HYPERRAM_IDENTIFICATION_0_MANUFACTURER      ((uint16_t)0x0006)
#define HYPERRAM_IDENTIFICATION_1_DEVICE_TYPE       ((uint16_t)0x0001)

static XSPI_HandleTypeDef g_xspi_handle = {0};
static uint32_t hyperram_size = 0;

/**
 * @brief   Ӳ����λHyperRAM
 * @param   ��
 * @retval  ��
 */
static void hyperram_hardware_reset(void)
{
    HAL_GPIO_WritePin(GPION, GPIO_PIN_12, GPIO_PIN_RESET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(GPION, GPIO_PIN_12, GPIO_PIN_SET);
    HAL_Delay(1);
}

/**
 * @brief   ��HyperRAM�Ĵ���
 * @param   address: �Ĵ�����ַ
 * @param   data: ���ݻ�����ָ��
 * @retval  ��ȡ���
 * @arg     0: ��ȡ�ɹ�
 * @arg     1: ��ȡʧ��
 */
static uint8_t hyperram_read_register(uint32_t address, uint16_t *data)
{
    XSPI_HyperbusCmdTypeDef xspi_hyperbus_cmd_struct = {0};
    
    xspi_hyperbus_cmd_struct.AddressSpace = HAL_XSPI_REGISTER_ADDRESS_SPACE;
    xspi_hyperbus_cmd_struct.Address = address;
    xspi_hyperbus_cmd_struct.AddressWidth = HAL_XSPI_ADDRESS_32_BITS;
    xspi_hyperbus_cmd_struct.DataLength = 2;
    xspi_hyperbus_cmd_struct.DQSMode = HAL_XSPI_DQS_ENABLE;
    xspi_hyperbus_cmd_struct.DataMode = HAL_XSPI_DATA_8_LINES;
    if (HAL_XSPI_HyperbusCmd(&g_xspi_handle, &xspi_hyperbus_cmd_struct, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return 1;
    }
    
    if (HAL_XSPI_Receive(&g_xspi_handle, (uint8_t *)data, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return 1;
    }
    
    return 0;
}

/**
 * @brief   дHyperRAM�Ĵ���
 * @param   address: �Ĵ�����ַ
 * @param   data: ����
 * @retval  д����
 * @arg     0: д��ɹ�
 * @arg     1: д��ʧ��
 */
static uint8_t hyperram_write_register(uint32_t address, uint16_t data)
{
    XSPI_HyperbusCmdTypeDef xspi_hyperbus_cmd_struct = {0};
    
    xspi_hyperbus_cmd_struct.AddressSpace = HAL_XSPI_REGISTER_ADDRESS_SPACE;
    xspi_hyperbus_cmd_struct.Address = address;
    xspi_hyperbus_cmd_struct.AddressWidth = HAL_XSPI_ADDRESS_32_BITS;
    xspi_hyperbus_cmd_struct.DataLength = 2;
    xspi_hyperbus_cmd_struct.DQSMode = HAL_XSPI_DQS_ENABLE;
    xspi_hyperbus_cmd_struct.DataMode = HAL_XSPI_DATA_8_LINES;
    if (HAL_XSPI_HyperbusCmd(&g_xspi_handle, &xspi_hyperbus_cmd_struct, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return 1;
    }
    
    if (HAL_XSPI_Transmit(&g_xspi_handle, (uint8_t *)&data, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return 1;
    }
    
    return 0;
}

/**
 * @brief   ���HyperRAM��ID
 * @param   ��
 * @retval  �����
 * @arg     0: ���ɹ�
 * @arg     1: ���ʧ��
 */
static uint8_t hyperram_check_identification(void)
{
    uint16_t data;
    
    if (hyperram_read_register(HYPERRAM_IDENTIFICATION_REGISTER_1, &data) != 0)
    {
        return 1;
    }
    
    if (((data & HYPERRAM_IDENTIFICATION_1_MASK_DEVICE_TYPE) >> HYPERRAM_IDENTIFICATION_1_POS_DEVICE_TYPE) != HYPERRAM_IDENTIFICATION_1_DEVICE_TYPE)
    {
        return 1;
    }
    
    if (hyperram_read_register(HYPERRAM_IDENTIFICATION_REGISTER_0, &data) != 0)
    {
        return 1;
    }
    
    if (((data & HYPERRAM_IDENTIFICATION_0_MASK_MANUFACTURER) >> HYPERRAM_IDENTIFICATION_0_POS_MANUFACTURER) != HYPERRAM_IDENTIFICATION_0_MANUFACTURER)
    {
        return 1;
    }
    
    hyperram_size = 1UL << (((((data & HYPERRAM_IDENTIFICATION_0_MASK_COL_COUNT) >> HYPERRAM_IDENTIFICATION_0_POS_COL_COUNT) + 1) + (((data & HYPERRAM_IDENTIFICATION_0_MASK_ROW_COUNT) >> HYPERRAM_IDENTIFICATION_0_POS_ROW_COUNT) + 1)) + 1);
    
    return 0;
}

/**
 * @brief   ����HyperRAM���ʱ��ģʽ
 * @param   ��
 * @retval  ���ý��
 * @arg     0: ���óɹ�
 * @arg     1: ����ʧ��
 */
static uint8_t hyperram_config_differential_clock(void)
{
    uint16_t data;
    
    if (hyperram_read_register(HYPERRAM_CONFIGURATION_REGISTER_1, &data) != 0)
    {
        return 1;
    }
    
    data &= ~(1UL << 6);
    if (hyperram_write_register(HYPERRAM_CONFIGURATION_REGISTER_1, data) != 0)
    {
        return 1;
    }
    
    return 0;
}

/**
 * @brief   ����HyperRAM�ڴ�ӳ��
 * @param   ��
 * @retval  ���ý��
 * @arg     0: ���óɹ�
 * @arg     1: ����ʧ��
 */
static uint8_t hyperram_memory_mapped(void)
{
    XSPI_HyperbusCmdTypeDef xspi_hyperbus_cmd_struct = {0};
    XSPI_MemoryMappedTypeDef xspi_memory_mapped_struct = {0};
    
    xspi_hyperbus_cmd_struct.AddressSpace = HAL_XSPI_MEMORY_ADDRESS_SPACE;
    xspi_hyperbus_cmd_struct.AddressWidth = HAL_XSPI_ADDRESS_32_BITS;
    xspi_hyperbus_cmd_struct.DQSMode = HAL_XSPI_DQS_ENABLE;
    xspi_hyperbus_cmd_struct.DataMode = HAL_XSPI_DATA_8_LINES;
    if (HAL_XSPI_HyperbusCmd(&g_xspi_handle, &xspi_hyperbus_cmd_struct, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return 1;
    }
    
    xspi_memory_mapped_struct.TimeOutActivation = HAL_XSPI_TIMEOUT_COUNTER_ENABLE;
    xspi_memory_mapped_struct.TimeoutPeriodClock = 0;
    if (HAL_XSPI_MemoryMapped(&g_xspi_handle, &xspi_memory_mapped_struct) != HAL_OK)
    {
        return 1;
    }
    
    return 0;
}

/**
 * @brief   ��ʼ��HyperRAM
 * @param   ��
 * @retval  ��ʼ�����
 * @arg     0: ��ʼ���ɹ�
 * @arg     1: ��ʼ��ʧ��
 */
uint8_t hyperram_init(void)
{
    RCC_PeriphCLKInitTypeDef rcc_periph_clk_init_struct = {0};
    GPIO_InitTypeDef gpio_init_struct = {0};
    XSPIM_CfgTypeDef xspim_cfg_struct = {0};
    XSPI_HyperbusCfgTypeDef xspi_hyperbus_cfg_struct = {0};
    
    /* ����XSPIʱ��Դ */
    rcc_periph_clk_init_struct.PeriphClockSelection = RCC_PERIPHCLK_XSPI2;
    rcc_periph_clk_init_struct.Xspi2ClockSelection = RCC_XSPI2CLKSOURCE_PLL2S;
    if (HAL_RCCEx_PeriphCLKConfig(&rcc_periph_clk_init_struct) != HAL_OK)
    {
        return 1;
    }
    
    /* ʹ��ʱ�� */
      __HAL_RCC_XSPIM_CLK_ENABLE();
    __HAL_RCC_XSPI2_CLK_ENABLE();
    __HAL_RCC_GPION_CLK_ENABLE();
    
    /* ��ʼ��XSPIM���� */
    gpio_init_struct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11;
    gpio_init_struct.Mode = GPIO_MODE_AF_PP;
    gpio_init_struct.Pull = GPIO_NOPULL;
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio_init_struct.Alternate = GPIO_AF9_XSPIM_P2;
    HAL_GPIO_Init(GPION, &gpio_init_struct);
    
    /* ��ʼ����λ���� */
    gpio_init_struct.Pin = GPIO_PIN_12;
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init_struct.Pull = GPIO_PULLUP;
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPION, &gpio_init_struct);
    
    /* ��ʼ��XSPI */
    g_xspi_handle.Instance = XSPI2;
    g_xspi_handle.Init.FifoThresholdByte = 4;
    g_xspi_handle.Init.MemoryMode = HAL_XSPI_SINGLE_MEM;
    g_xspi_handle.Init.MemoryType = HAL_XSPI_MEMTYPE_HYPERBUS;
    g_xspi_handle.Init.MemorySize = HAL_XSPI_SIZE_256MB;
    g_xspi_handle.Init.ChipSelectHighTimeCycle = 2;
    g_xspi_handle.Init.FreeRunningClock = HAL_XSPI_FREERUNCLK_DISABLE;
    g_xspi_handle.Init.ClockMode = HAL_XSPI_CLOCK_MODE_0;
    g_xspi_handle.Init.WrapSize = HAL_XSPI_WRAP_32_BYTES;
    g_xspi_handle.Init.ClockPrescaler = 2 - 1;
    g_xspi_handle.Init.SampleShifting = HAL_XSPI_SAMPLE_SHIFT_NONE;
    g_xspi_handle.Init.DelayHoldQuarterCycle = HAL_XSPI_DHQC_DISABLE;
    g_xspi_handle.Init.ChipSelectBoundary = HAL_XSPI_BONDARYOF_NONE;
    g_xspi_handle.Init.MaxTran = 0;
    g_xspi_handle.Init.Refresh = 0;
    g_xspi_handle.Init.MemorySelect = HAL_XSPI_CSSEL_NCS1;
    if (HAL_XSPI_Init(&g_xspi_handle) != HAL_OK)
    {
        return 1;
    }
    
    /* ����XSPIM */
    xspim_cfg_struct.nCSOverride = HAL_XSPI_CSSEL_OVR_NCS1;
    xspim_cfg_struct.IOPort = HAL_XSPIM_IOPORT_2;
    if (HAL_XSPIM_Config(&g_xspi_handle, &xspim_cfg_struct, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return 1;
    }
    
    /* ����HyperBus���� */
    xspi_hyperbus_cfg_struct.RWRecoveryTimeCycle = 7;
    xspi_hyperbus_cfg_struct.AccessTimeCycle = 7;
    xspi_hyperbus_cfg_struct.WriteZeroLatency = HAL_XSPI_LATENCY_ON_WRITE;
    xspi_hyperbus_cfg_struct.LatencyMode = HAL_XSPI_FIXED_LATENCY;
    if (HAL_XSPI_HyperbusCfg(&g_xspi_handle, &xspi_hyperbus_cfg_struct, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return 1;
    }
    
    /* Ӳ����λHyperRAM */
    hyperram_hardware_reset();
    
    /* ���HyperRAM��ID */
    if (hyperram_check_identification() != 0)
    {
        return 1;
    }
    
    /* ����HyperRAM���ʱ��ģʽ */
    if (hyperram_config_differential_clock() != 0)
    {
        return 1;
    }
    
    /* ����HyperRAM�ڴ�ӳ�� */
    if (hyperram_memory_mapped() != 0)
    {
        return 1;
    }
    
    return 0;
}

/**
 * @brief   ��ȡHyperRAM����
 * @param   ��
 * @retval  HyperRAM����
 */
uint32_t hyperram_get_size(void)
{
    return hyperram_size;
}
