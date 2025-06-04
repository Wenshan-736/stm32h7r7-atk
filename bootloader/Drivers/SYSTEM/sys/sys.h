/**
 ****************************************************************************************************
 * @file        sys.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2024-05-21
 * @brief       ϵͳ��ʼ������
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

#ifndef __SYS_H
#define __SYS_H

#include "stm32h7rsxx_hal.h"

/* ���ܶ��� */
#define SYS_SUPPORT_OS 0    /* SYSTEM��OS֧�� */

/* ��������PLL */
extern RCC_OscInitTypeDef rcc_osc_init_struct;

/* �ڴ�ӳ�俪����־λ */
extern uint8_t sys_memory_mapped_flag;

/* �������� */
void sys_mpu_config(void);                                                  /* ����MPU */
void sys_cache_enable(void);                                                /* ʹ��Cache */
uint8_t sys_stm32_clock_init(uint32_t plln, uint32_t pllm, uint32_t pllp);  /* ����ʱ�� */
void sys_xspi1_enable_memmapmode(void);
#endif
