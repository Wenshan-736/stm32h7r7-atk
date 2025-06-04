/**
 ****************************************************************************************************
 * @file        usart.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2024-05-21
 * @brief       ���ڳ�ʼ������
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

#include "./SYSTEM/usart/usart.h"
#if SYS_SUPPORT_OS
#include "os.h"
#endif

int fputc(int c, FILE *stream)
{
    while (!(USART_UX->ISR & USART_ISR_TC));
    USART_UX->TDR = (uint8_t)c;
    
    return c;
}

/* UART��� */
UART_HandleTypeDef g_uart1_handle = {0};

#if USART_EN_RX
/* �����жϽ��ջ����� */
uint8_t g_rx_buffer[RXBUFFERSIZE];
/* ���ڽ��ջ����� */
uint8_t g_usart_rx_buf[USART_REC_LEN];
/* ���ڽ���״̬��� */
uint16_t g_usart_rx_sta = 0;
#endif

/**
 * @brief   ��ʼ������
 * @param   baudrate: ͨ�Ų����ʣ���λ��bps��
 * @retval  ��
 */
void usart_init(uint32_t baudrate)
{
    /* ��ʼ��UART */
    g_uart1_handle.Instance = USART_UX;
    g_uart1_handle.Init.BaudRate = baudrate;
    g_uart1_handle.Init.WordLength = UART_WORDLENGTH_8B;
    g_uart1_handle.Init.StopBits = UART_STOPBITS_1;
    g_uart1_handle.Init.Parity = UART_PARITY_NONE;
#if USART_EN_RX
    g_uart1_handle.Init.Mode = UART_MODE_TX_RX;
#else
    g_uart1_handle.Init.Mode = UART_MODE_TX;
#endif
    g_uart1_handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    g_uart1_handle.Init.OverSampling = UART_OVERSAMPLING_16;
    g_uart1_handle.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    g_uart1_handle.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    g_uart1_handle.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    HAL_UART_Init(&g_uart1_handle);
    
#if USART_EN_RX
    /* UART�жϽ������� */
    HAL_UART_Receive_IT(&g_uart1_handle, g_rx_buffer, sizeof(g_rx_buffer));
#endif
}

/**
 * @brief   HAL��UART��ʼ��MSP����
 * @param   huart: UART���ָ��
 * @retval  ��
 */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    RCC_PeriphCLKInitTypeDef rcc_periph_clk_init_struct = {0};
    GPIO_InitTypeDef gpio_init_struct = {0};
    
    if (huart->Instance == USART_UX)
    {
        /* ����ʱ��Դ */
        rcc_periph_clk_init_struct.PeriphClockSelection = RCC_PERIPHCLK_USART1;
        rcc_periph_clk_init_struct.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
        HAL_RCCEx_PeriphCLKConfig(&rcc_periph_clk_init_struct);
        
        /* ʹ��ʱ�� */
        USART_UX_CLK_ENABLE();
        USART_TX_GPIO_CLK_ENABLE();
#if USART_EN_RX
        USART_RX_GPIO_CLK_ENABLE();
#endif
        
        /* ��ʼ��TX���� */
        gpio_init_struct.Pin = USART_TX_GPIO_PIN;
        gpio_init_struct.Mode = GPIO_MODE_AF_PP;
        gpio_init_struct.Pull = GPIO_PULLUP;
        gpio_init_struct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        gpio_init_struct.Alternate = USART_TX_GPIO_AF;
        HAL_GPIO_Init(USART_TX_GPIO_PORT, &gpio_init_struct);
        
#if USART_EN_RX
        /* ��ʼ��RX���� */
        gpio_init_struct.Pin = USART_RX_GPIO_PIN;
        gpio_init_struct.Mode = GPIO_MODE_AF_PP;
        gpio_init_struct.Pull = GPIO_PULLUP;
        gpio_init_struct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        gpio_init_struct.Alternate = USART_RX_GPIO_AF;
        HAL_GPIO_Init(USART_RX_GPIO_PORT, &gpio_init_struct);
#endif
        
#if USART_EN_RX
        /* �����ж� */
        HAL_NVIC_SetPriority(USART_UX_IRQn, 3, 0);
        HAL_NVIC_EnableIRQ(USART_UX_IRQn);
#endif
    }
}

/**
 * @brief   HAL��UART������ɻص�����
 * @param   huart: UART���ָ��
 * @retval  ��
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART_UX)
    {
#if USART_EN_RX
        if((g_usart_rx_sta & 0x8000) == 0)
        {
            if(g_usart_rx_sta & 0x4000)
            {
                if(g_rx_buffer[0] != 0x0A)
                {
                    g_usart_rx_sta = 0;
                }
                else
                {
                    g_usart_rx_sta |= 0x8000;
                }
            }
            else
            {
                if(g_rx_buffer[0] == 0x0D)
                {
                    g_usart_rx_sta |= 0x4000;
                }
                else
                {
                    g_usart_rx_buf[g_usart_rx_sta & 0x3FFF] = g_rx_buffer[0];
                    g_usart_rx_sta++;
                    if(g_usart_rx_sta > (USART_REC_LEN - 1))
                    {
                        g_usart_rx_sta = 0;
                    }
                }
            }
        }
        
        HAL_UART_Receive_IT(&g_uart1_handle, g_rx_buffer, sizeof(g_rx_buffer));
#endif
    }
}

/**
 * @brief   UART�жϻص�����
 * @param   ��
 * @retval  ��
 */
void USART_UX_IRQHandler(void)
{
#if SYS_SUPPORT_OS
    OSIntEnter();
#endif
    
    HAL_UART_IRQHandler(&g_uart1_handle);
    
#if SYS_SUPPORT_OS
    OSIntExit();
#endif
}
