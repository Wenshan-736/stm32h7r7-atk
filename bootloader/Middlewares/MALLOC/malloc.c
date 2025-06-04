/**
 ****************************************************************************************************
 * @file        malloc.c
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

#include "./MALLOC/malloc.h"

/* �ڴ�ض��� */
static uint8_t mem1base[MEM1_MAX_SIZE] __attribute__((aligned(64)));                                                        /* AXI-SRAM1~2�ڴ�� */
static uint8_t mem2base[MEM2_MAX_SIZE] __attribute__((aligned(64))) __attribute__((section(".bss.ARM.__at_0x701F4000")));   /* XSPI2 HyperRAM�ڴ��,LTDCʹ��ǰ1280 * 800 * 2=0x1F4000�ֽڿռ� */
static uint8_t mem3base[MEM3_MAX_SIZE] __attribute__((aligned(64))) __attribute__((section(".bss.ARM.__at_0x30000000")));   /* AHB-SRAM1~2�ڴ�� */
static uint8_t mem4base[MEM4_MAX_SIZE] __attribute__((aligned(64))) __attribute__((section(".bss.ARM.__at_0x20000000")));   /* DTCM�ڴ�� */
static uint8_t mem5base[MEM5_MAX_SIZE] __attribute__((aligned(64))) __attribute__((section(".bss.ARM.__at_0x00000000")));   /* ITCM�ڴ�� */

/* �ڴ����� */
static MT_TYPE mem1mapbase[MEM1_ALLOC_TABLE_SIZE];                                                                          /* AXI-SRAM1~2�ڴ��MAP */
#if (CONFIG_HYPERRAM_HIGH_SPEC == 0)
static MT_TYPE mem2mapbase[MEM2_ALLOC_TABLE_SIZE] __attribute__((section(".bss.ARM.__at_0x707A4F0C")));                     /* XSPI2 HyperRAM�ڴ��MAP */
#else
static MT_TYPE mem2mapbase[MEM2_ALLOC_TABLE_SIZE] __attribute__((section(".bss.ARM.__at_0x71E3B878")));                     /* XSPI2 HyperRAM�ڴ��MAP */
#endif
static MT_TYPE mem3mapbase[MEM3_ALLOC_TABLE_SIZE] __attribute__((section(".bss.ARM.__at_0x30007840")));                     /* AHB-SRAM1~2�ڴ��MAP */
static MT_TYPE mem4mapbase[MEM4_ALLOC_TABLE_SIZE] __attribute__((section(".bss.ARM.__at_0x2000F0C0")));                     /* DTCM�ڴ��MAP */
static MT_TYPE mem5mapbase[MEM5_ALLOC_TABLE_SIZE] __attribute__((section(".bss.ARM.__at_0x0000F0C0")));                     /* ITCM�ڴ��MAP */

/* �ڴ������� */
const uint32_t memtblsize[SRAMBANK] = { MEM1_ALLOC_TABLE_SIZE, MEM2_ALLOC_TABLE_SIZE, MEM3_ALLOC_TABLE_SIZE,
                                        MEM4_ALLOC_TABLE_SIZE, MEM5_ALLOC_TABLE_SIZE};                                      /* �ڴ���С */

const uint32_t memblksize[SRAMBANK] = { MEM1_BLOCK_SIZE, MEM2_BLOCK_SIZE, MEM3_BLOCK_SIZE,
                                        MEM4_BLOCK_SIZE, MEM5_BLOCK_SIZE};                                                  /* �ڴ�ֿ��С */

const uint32_t memsize[SRAMBANK] = { MEM1_MAX_SIZE, MEM2_MAX_SIZE, MEM3_MAX_SIZE,
                                     MEM4_MAX_SIZE, MEM5_MAX_SIZE};                                                         /* �ڴ��ܴ�С */

/* �ڴ��������� */
struct _m_mallco_dev mallco_dev =
{
    my_mem_init,                                                        /* �ڴ��ʼ�� */
    my_mem_perused,                                                     /* �ڴ�ʹ���� */
    mem1base, mem2base, mem3base, mem4base, mem5base,                   /* �ڴ�� */
    mem1mapbase, mem2mapbase, mem3mapbase, mem4mapbase, mem5mapbase,    /* �ڴ����״̬�� */
    0, 0, 0, 0, 0,                                                      /* �ڴ����δ���� */
};

/**
 * @brief       �����ڴ�
 * @param       *des : Ŀ�ĵ�ַ
 * @param       *src : Դ��ַ
 * @param       n    : ��Ҫ���Ƶ��ڴ泤��(�ֽ�Ϊ��λ)
 * @retval      ��
 */
void my_mem_copy(void *des, void *src, uint32_t n)
{
    uint8_t *xdes = des;
    uint8_t *xsrc = src;

    while (n--) *xdes++ = *xsrc++;
}

/**
 * @brief       �����ڴ�ֵ
 * @param       *s    : �ڴ��׵�ַ
 * @param       c     : Ҫ���õ�ֵ
 * @param       count : ��Ҫ���õ��ڴ��С(�ֽ�Ϊ��λ)
 * @retval      ��
 */
void my_mem_set(void *s, uint8_t c, uint32_t count)
{
    uint8_t *xs = s;

    while (count--) *xs++ = c;
}

/**
 * @brief       �ڴ�����ʼ��
 * @param       memx : �����ڴ��
 * @retval      ��
 */
void my_mem_init(uint8_t memx)
{
    uint8_t mttsize = sizeof(MT_TYPE);  /* ��ȡmemmap��������ͳ���(uint16_t /uint32_t)*/
    my_mem_set(mallco_dev.memmap[memx], 0, memtblsize[memx] * mttsize); /* �ڴ�״̬���������� */
    mallco_dev.memrdy[memx] = 1;        /* �ڴ�����ʼ��OK */
}

/**
 * @brief       ��ȡ�ڴ�ʹ����
 * @param       memx : �����ڴ��
 * @retval      ʹ����(������10��,0~1000,����0.0%~100.0%)
 */
uint16_t my_mem_perused(uint8_t memx)
{
    uint32_t used = 0;
    uint32_t i;

    for (i = 0; i < memtblsize[memx]; i++)
    {
        if (mallco_dev.memmap[memx][i]) used++;
    }

    return (used * 1000) / (memtblsize[memx]);
}

/**
 * @brief       �ڴ����(�ڲ�����)
 * @param       memx : �����ڴ��
 * @param       size : Ҫ������ڴ��С(�ֽ�)
 * @retval      �ڴ�ƫ�Ƶ�ַ
 *   @arg       0 ~ 0XFFFFFFFE : ��Ч���ڴ�ƫ�Ƶ�ַ
 *   @arg       0XFFFFFFFF     : ��Ч���ڴ�ƫ�Ƶ�ַ
 */
static uint32_t my_mem_malloc(uint8_t memx, uint32_t size)
{
    signed long offset = 0;
    uint32_t nmemb;         /* ��Ҫ���ڴ���� */
    uint32_t cmemb = 0;     /* �������ڴ���� */
    uint32_t i;

    if (!mallco_dev.memrdy[memx])
    {
        mallco_dev.init(memx);          /* δ��ʼ��,��ִ�г�ʼ�� */
    }
    
    if (size == 0) return 0XFFFFFFFF;   /* ����Ҫ���� */

    nmemb = size / memblksize[memx];    /* ��ȡ��Ҫ����������ڴ���� */

    if (size % memblksize[memx]) nmemb++;

    for (offset = memtblsize[memx] - 1; offset >= 0; offset--)  /* ���������ڴ������ */
    {
        if (!mallco_dev.memmap[memx][offset])
        {
            cmemb++;            /* �������ڴ�������� */
        }
        else 
        {
            cmemb = 0;          /* �����ڴ������ */
        }
        
        if (cmemb == nmemb)     /* �ҵ�������nmemb�����ڴ�� */
        {
            for (i = 0; i < nmemb; i++)         /* ��ע�ڴ��ǿ� */
            {
                mallco_dev.memmap[memx][offset + i] = nmemb;
            }

            return (offset * memblksize[memx]); /* ����ƫ�Ƶ�ַ */
        }
    }

    return 0XFFFFFFFF;  /* δ�ҵ����Ϸ����������ڴ�� */
}

/**
 * @brief       �ͷ��ڴ�(�ڲ�����)
 * @param       memx   : �����ڴ��
 * @param       offset : �ڴ��ַƫ��
 * @retval      �ͷŽ��
 *   @arg       0, �ͷųɹ�;
 *   @arg       1, �ͷ�ʧ��;
 *   @arg       2, ��������(ʧ��);
 */
static uint8_t my_mem_free(uint8_t memx, uint32_t offset)
{
    int i;

    if (!mallco_dev.memrdy[memx])   /* δ��ʼ��,��ִ�г�ʼ�� */
    {
        mallco_dev.init(memx);
        return 1;                   /* δ��ʼ�� */
    }

    if (offset < memsize[memx])     /* ƫ�����ڴ����. */
    {
        int index = offset / memblksize[memx];      /* ƫ�������ڴ����� */
        int nmemb = mallco_dev.memmap[memx][index]; /* �ڴ������ */

        for (i = 0; i < nmemb; i++)                 /* �ڴ������ */
        {
            mallco_dev.memmap[memx][index + i] = 0;
        }

        return 0;
    }
    else
    {
        return 2;   /* ƫ�Ƴ�����. */
    }
}

/**
 * @brief       �ͷ��ڴ�(�ⲿ����)
 * @param       memx : �����ڴ��
 * @param       ptr  : �ڴ��׵�ַ
 * @retval      ��
 */
void myfree(uint8_t memx, void *ptr)
{
    uint32_t offset;

    if (ptr == NULL) return;    /* ��ַΪ0. */

    offset = (uint32_t)ptr - (uint32_t)mallco_dev.membase[memx];
    my_mem_free(memx, offset);  /* �ͷ��ڴ� */
}

/**
 * @brief       �����ڴ�(�ⲿ����)
 * @param       memx : �����ڴ��
 * @param       size : Ҫ������ڴ��С(�ֽ�)
 * @retval      ���䵽���ڴ��׵�ַ.
 */
void *mymalloc(uint8_t memx, uint32_t size)
{
    uint32_t offset;
    offset = my_mem_malloc(memx, size);

    if (offset == 0XFFFFFFFF)   /* ������� */
    {
        return NULL;            /* ���ؿ�(0) */
    }
    else                        /* ����û����, �����׵�ַ */
    {
        return (void *)((uint32_t)mallco_dev.membase[memx] + offset);
    }
}

/**
 * @brief       ���·����ڴ�(�ⲿ����)
 * @param       memx : �����ڴ��
 * @param       *ptr : ���ڴ��׵�ַ
 * @param       size : Ҫ������ڴ��С(�ֽ�)
 * @retval      �·��䵽���ڴ��׵�ַ.
 */
void *myrealloc(uint8_t memx, void *ptr, uint32_t size)
{
    uint32_t offset;
    offset = my_mem_malloc(memx, size);

    if (offset == 0XFFFFFFFF)   /* ������� */
    {
        return NULL;            /* ���ؿ�(0) */
    }
    else                        /* ����û����, �����׵�ַ */
    {
        my_mem_copy((void *)((uint32_t)mallco_dev.membase[memx] + offset), ptr, size);  /* �������ڴ����ݵ����ڴ� */
        myfree(memx, ptr);      /* �ͷž��ڴ� */
        return (void *)((uint32_t)mallco_dev.membase[memx] + offset);                   /* �������ڴ��׵�ַ */
    }
}












