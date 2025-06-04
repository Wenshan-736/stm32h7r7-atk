/**
 ****************************************************************************************************
 * @file        hyperram.h
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

#ifndef __HYPERRAM_H
#define __HYPERRAM_H

#include "./SYSTEM/sys/sys.h"

/* HyperRAM��ض��� */
#define HYPERRAM_BASE_ADDR XSPI2_BASE

/* �������� */
uint8_t hyperram_init(void);        /* ��ʼ��HyperRAM */
uint32_t hyperram_get_size(void);   /* ��ȡHyperRAM���� */

#endif
