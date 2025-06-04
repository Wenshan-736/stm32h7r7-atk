/**
 ****************************************************************************************************
 * @file        delay.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2024-05-21
 * @brief       ��ʱ���ܴ���
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

#include "./SYSTEM/delay/delay.h"

/* ΢�뼶��ʱ������ */
static uint32_t g_fac_us = 0;

/* ���SYS_SUPPORT_OS������,˵��Ҫ֧��OS��(������UCOS) */
#if SYS_SUPPORT_OS

/* ��ӹ���ͷ�ļ� ( ucos��Ҫ�õ�) */
#include "os.h"

/* ����g_fac_ms����, ��ʾms��ʱ�ı�����, ����ÿ�����ĵ�ms��, (����ʹ��os��ʱ��,��Ҫ�õ�) */
static uint16_t g_fac_ms = 0;

/*
 *  ��delay_us/delay_ms��Ҫ֧��OS��ʱ����Ҫ������OS��صĺ궨��ͺ�����֧��
 *  ������3���궨��:
 *      delay_osrunning    :���ڱ�ʾOS��ǰ�Ƿ���������,�Ծ����Ƿ����ʹ����غ���
 *      delay_ostickspersec:���ڱ�ʾOS�趨��ʱ�ӽ���,delay_init�����������������ʼ��systick
 *      delay_osintnesting :���ڱ�ʾOS�ж�Ƕ�׼���,��Ϊ�ж����治���Ե���,delay_msʹ�øò����������������
 *  Ȼ����3������:
 *      delay_osschedlock  :��������OS�������,��ֹ����
 *      delay_osschedunlock:���ڽ���OS�������,���¿�������
 *      delay_ostimedly    :����OS��ʱ,���������������.
 *
 *  �����̽���UCOSII��֧��,����OS,�����вο�����ֲ
 */

/* ֧��UCOSII */
#define delay_osrunning     OSRunning           /* OS�Ƿ����б��,0,������;1,������ */
#define delay_ostickspersec OS_TICKS_PER_SEC    /* OSʱ�ӽ���,��ÿ����ȴ��� */
#define delay_osintnesting  OSIntNesting        /* �ж�Ƕ�׼���,���ж�Ƕ�״��� */


/**
 * @brief     us����ʱʱ,�ر��������(��ֹ���us���ӳ�)
 * @param     ��
 * @retval    ��
 */
void delay_osschedlock(void)
{
    OSSchedLock();                      /* UCOSII�ķ�ʽ,��ֹ���ȣ���ֹ���us��ʱ */
}

/**
 * @brief     us����ʱʱ,�ָ��������
 * @param     ��
 * @retval    ��
 */
void delay_osschedunlock(void)
{
    OSSchedUnlock();                    /* UCOSII�ķ�ʽ,�ָ����� */
}

/**
 * @brief     us����ʱʱ,�ָ��������
 * @param     ticks: ��ʱ�Ľ�����
 * @retval    ��
 */
void delay_ostimedly(uint32_t ticks)
{
    OSTimeDly(ticks);                               /* UCOSII��ʱ */
}

/**
 * @brief     systick�жϷ�����,ʹ��OSʱ�õ�
 * @param     ticks : ��ʱ�Ľ�����  
 * @retval    ��
 */  
void SysTick_Handler(void)
{
    /* OS ��ʼ����,��ִ�������ĵ��ȴ��� */
    if (delay_osrunning == OS_TRUE)
    {
        /* ���� uC/OS-II �� SysTick �жϷ����� */
        OS_CPU_SysTickHandler();
    }
    HAL_IncTick();
}
#endif

/**
 * @brief   ��ʼ����ʱ
 * @param   sysclk: ϵͳʱ��Ƶ�ʣ���λ��MHz��
 * @retval  ��
 */
void delay_init(uint16_t sysclk)
{
#if SYS_SUPPORT_OS
    uint32_t reload;
#endif
    
    g_fac_us = sysclk;
#if SYS_SUPPORT_OS
    reload = sysclk;
    reload *= 1000000 / delay_ostickspersec;
    g_fac_ms = 1000 / delay_ostickspersec;
    SysTick->CTRL |= 1 << 1;
    SysTick->LOAD = reload;
    SysTick->CTRL |= 1 << 0;
#endif
}

/**
 * @brief   ΢�뼶��ʱ
 * @param   nus: ��ʱʱ�䣨��λ��us��
 * @retval  ��
 */
void delay_us(uint32_t nus)
{
    uint32_t reload;
    uint32_t ticks;
    uint32_t told;
    uint32_t tnow;
    uint32_t tcnt = 0;
    
    reload = SysTick->LOAD;
    ticks = nus * g_fac_us;
    
#if SYS_SUPPORT_OS
    if (delay_osrunning == OS_TRUE)
    {
        delay_osschedlock();
    }
#endif
    
    told = SysTick->VAL;
    while (1)
    {
        tnow = SysTick->VAL;
        if (tnow != told)
        {
            if (tnow < told)
            {
                tcnt += told - tnow;
            }
            else
            {
                tcnt += reload - tnow + told;
            }
            told = tnow;
            if (tcnt >= ticks)
            {
                break;
            }
        }
    }
    
#if SYS_SUPPORT_OS
    if (delay_osrunning == OS_TRUE)
    {
        delay_osschedunlock();
    }
#endif
}

/**
 * @brief   ���뼶��ʱ
 * @param   nms: ��ʱʱ�䣨��λ��ms��
 * @retval  ��
 */
void delay_ms(uint16_t nms)
{
#if SYS_SUPPORT_OS
    if (delay_osrunning && (delay_osintnesting == 0))
    {
        if (nms >= g_fac_ms)
        {
            delay_ostimedly(nms / g_fac_ms);
        }
        nms %= g_fac_ms;
    }
#endif
    
    delay_us((uint32_t)nms * 1000);
}

/**
 * @brief   HAL����뼶��ʱ����
 * @param   Delay: ��ʱʱ�䣨��λ��ms��
 * @retval  ��
 */
//void HAL_Delay(uint32_t Delay)
//{
//    delay_us(Delay * 1000);
//}
