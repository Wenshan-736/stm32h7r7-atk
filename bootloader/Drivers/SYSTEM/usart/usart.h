/**
 ****************************************************************************************************
 * @file        usart.h
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

#ifndef __USART_H
#define __USART_H

#include "./SYSTEM/sys/sys.h"
#include <stdio.h>

/* Ӳ������ */
#define USART_TX_GPIO_PORT          GPIOB
#define USART_TX_GPIO_PIN           GPIO_PIN_14
#define USART_TX_GPIO_AF            GPIO_AF4_USART1
#define USART_TX_GPIO_CLK_ENABLE()  do { __HAL_RCC_GPIOB_CLK_ENABLE(); } while(0)
#define USART_RX_GPIO_PORT          GPIOB
#define USART_RX_GPIO_PIN           GPIO_PIN_15
#define USART_RX_GPIO_AF            GPIO_AF4_USART1
#define USART_RX_GPIO_CLK_ENABLE()  do { __HAL_RCC_GPIOB_CLK_ENABLE(); } while(0)
#define USART_UX                    USART1
#define USART_UX_IRQn               USART1_IRQn
#define USART_UX_IRQHandler         USART1_IRQHandler
#define USART_UX_CLK_ENABLE()       do { __HAL_RCC_USART1_CLK_ENABLE(); } while(0)

/* ���ܶ��� */
#define USART_EN_RX                 1           /* ʹ�ܴ��ڽ��չ��� */
#define RXBUFFERSIZE                1           /* �����жϽ��ջ�������С */
#define USART_REC_LEN               200         /* ���ڽ��ջ�������С */

/* �������� */
extern UART_HandleTypeDef g_uart1_handle;       /* UART��� */
#if USART_EN_RX
extern uint8_t g_rx_buffer[RXBUFFERSIZE];       /* �����жϽ��ջ����� */
extern uint8_t g_usart_rx_buf[USART_REC_LEN];   /* ���ڽ��ջ����� */
extern uint16_t g_usart_rx_sta;                 /* ���ڽ���״̬��� */
#endif

/* �������� */
void usart_init(uint32_t baudrate);             /* ��ʼ������ */

#endif
