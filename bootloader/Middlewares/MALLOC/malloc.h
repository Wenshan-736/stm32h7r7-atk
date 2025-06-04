/**
 ****************************************************************************************************
 * @file        malloc.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2024-05-21
 * @brief       �ڴ������������
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

#ifndef __MALLOC_H
#define __MALLOC_H

#include "./SYSTEM/sys/sys.h"

/**
 * ������ʹ��HyperRAM�Ĺ��
 * 
 * 0: HyperRAM����Ϊ8MB��Ĭ�ϣ�
 * 1: HyperRAM����Ϊ32MB
 */
#define CONFIG_HYPERRAM_HIGH_SPEC 0

/* �ڴ�ر�Ŷ��� */
#define SRAMIN      0       /* AXI-SRAM1~4�ڴ��,AXI-SRAM1~4��456KB */
#define SRAMEX      1       /* XSPI2 HyperRAM�ڴ��,XSPI2 HyperRAM��32MB */
#define SRAM12      2       /* AHB-SRAM1/2�ڴ��,AHB-SRAM1~2,��32KB */
#define SRAMDTCM    3       /* DTCM�ڴ��,DTCM��64KB,�˲����ڴ��CPU��HPDMA(ͨ��AHB)���Է���!!!! */
#define SRAMITCM    4       /* ITCM�ڴ��,DTCM��64KB,�˲����ڴ��CPU��HPDMA(ͨ��AHB)���Է���!!!! */

#define SRAMBANK    5       /* ����֧�ֵ�SRAM����. */


/* �ڴ��������Ͷ���,�豣֤(((1 << (sizeof(MT_TYPE) * 8)) - 1) * MEMx_BLOCK_SIZE) >= MEMx_MAX_SIZE */
#define MT_TYPE     uint32_t

/* mem1�ڴ�����趨.mem1��H7R�ڲ���AXI-SRAM1~4 */
#define MEM1_BLOCK_SIZE         (64)                                                                                                /* �ڴ���СΪ64�ֽ� */
#define MEM1_MAX_SIZE           ((0x00062000 / (MEM1_BLOCK_SIZE + sizeof(MT_TYPE))) * MEM1_BLOCK_SIZE)                              /* AXI-SRAM1~4������0x00072000�ֽ� */
#define MEM1_ALLOC_TABLE_SIZE   (MEM1_MAX_SIZE / MEM1_BLOCK_SIZE)                                                                   /* �ڴ���С */

/* mem2�ڴ�����趨.mem2��H7R�ⲿ��XSPI2 HyperRAM */
#define MEM2_BLOCK_SIZE         (64)                                                                                                /* �ڴ���СΪ64�ֽ� */
#if (CONFIG_HYPERRAM_HIGH_SPEC == 0)
#define MEM2_MAX_SIZE           (((0x00800000 - (2UL * 1280 * 800)) / (MEM2_BLOCK_SIZE + sizeof(MT_TYPE))) * MEM2_BLOCK_SIZE)       /* XSPI2 HyperRAM���пռ� */
#else
#define MEM2_MAX_SIZE           (((0x02000000 - (2UL * 1280 * 800)) / (MEM2_BLOCK_SIZE + sizeof(MT_TYPE))) * MEM2_BLOCK_SIZE)       /* XSPI2 HyperRAM���пռ� */
#endif
#define MEM2_ALLOC_TABLE_SIZE   (MEM2_MAX_SIZE / MEM2_BLOCK_SIZE)                                                                   /* �ڴ���С */

/* mem3�ڴ�����趨.mem3��H7R�ڲ���AHB-SRAM1~2 */
#define MEM3_BLOCK_SIZE         (64)                                                                                                /* �ڴ���СΪ64�ֽ� */
#define MEM3_MAX_SIZE           ((0x00008000 / (MEM3_BLOCK_SIZE + sizeof(MT_TYPE))) * MEM3_BLOCK_SIZE)                              /* AHB-SRAM1~2����0x00008000�ֽ� */
#define MEM3_ALLOC_TABLE_SIZE   (MEM3_MAX_SIZE / MEM3_BLOCK_SIZE)                                                                   /* �ڴ���С */

/* mem4�ڴ�����趨.mem4��H7R�ڲ���DTCM */
#define MEM4_BLOCK_SIZE         (64)                                                                                                /* �ڴ���СΪ64�ֽ� */
#define MEM4_MAX_SIZE           ((0x00010000 / (MEM4_BLOCK_SIZE + sizeof(MT_TYPE))) * MEM4_BLOCK_SIZE)                              /* DTCM����0x00010000�ֽ� */
#define MEM4_ALLOC_TABLE_SIZE   (MEM4_MAX_SIZE / MEM4_BLOCK_SIZE)                                                                   /* �ڴ���С */

/* mem5�ڴ�����趨.mem5��H7R�ڲ���ITCM */
#define MEM5_BLOCK_SIZE         (64)                                                                                                /* �ڴ���СΪ64�ֽ� */
#define MEM5_MAX_SIZE           ((0x00010000 / (MEM5_BLOCK_SIZE + sizeof(MT_TYPE))) * MEM5_BLOCK_SIZE)                              /* ITCM����0x00010000�ֽ� */
#define MEM5_ALLOC_TABLE_SIZE   (MEM5_MAX_SIZE / MEM5_BLOCK_SIZE)                                                                   /* �ڴ���С */

/* �ڴ��������� */
struct _m_mallco_dev
{
    void (*init)(uint8_t);          /* ��ʼ�� */
    uint16_t (*perused)(uint8_t);   /* �ڴ�ʹ���� */
    uint8_t *membase[SRAMBANK];     /* �ڴ�� ����SRAMBANK��������ڴ� */
    MT_TYPE *memmap[SRAMBANK];      /* �ڴ����״̬�� */
    uint8_t  memrdy[SRAMBANK];      /* �ڴ�����Ƿ���� */
};

extern struct _m_mallco_dev mallco_dev; /* ��mallco.c���涨�� */


/* �û����ú��� */
void my_mem_init(uint8_t memx);                          /* �ڴ�����ʼ������(��/�ڲ�����) */
uint16_t my_mem_perused(uint8_t memx) ;                  /* ����ڴ�ʹ����(��/�ڲ�����) */
void my_mem_set(void *s, uint8_t c, uint32_t count);     /* �ڴ����ú��� */
void my_mem_copy(void *des, void *src, uint32_t n);      /* �ڴ濽������ */

void myfree(uint8_t memx, void *ptr);                    /* �ڴ��ͷ�(�ⲿ����) */
void *mymalloc(uint8_t memx, uint32_t size);             /* �ڴ����(�ⲿ����) */
void *myrealloc(uint8_t memx, void *ptr, uint32_t size); /* ���·����ڴ�(�ⲿ����) */

#endif













