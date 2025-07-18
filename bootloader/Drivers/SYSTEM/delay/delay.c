/**
 ****************************************************************************************************
 * @file        delay.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2024-05-21
 * @brief       延时功能代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 * 
 * 实验平台:正点原子 H7R7开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 * 
 ****************************************************************************************************
 */

#include "./SYSTEM/delay/delay.h"

/* 微秒级延时倍乘数 */
static uint32_t g_fac_us = 0;

/* 如果SYS_SUPPORT_OS定义了,说明要支持OS了(不限于UCOS) */
#if SYS_SUPPORT_OS

/* 添加公共头文件 ( ucos需要用到) */
#include "os.h"

/* 定义g_fac_ms变量, 表示ms延时的倍乘数, 代表每个节拍的ms数, (仅在使能os的时候,需要用到) */
static uint16_t g_fac_ms = 0;

/*
 *  当delay_us/delay_ms需要支持OS的时候需要三个与OS相关的宏定义和函数来支持
 *  首先是3个宏定义:
 *      delay_osrunning    :用于表示OS当前是否正在运行,以决定是否可以使用相关函数
 *      delay_ostickspersec:用于表示OS设定的时钟节拍,delay_init将根据这个参数来初始化systick
 *      delay_osintnesting :用于表示OS中断嵌套级别,因为中断里面不可以调度,delay_ms使用该参数来决定如何运行
 *  然后是3个函数:
 *      delay_osschedlock  :用于锁定OS任务调度,禁止调度
 *      delay_osschedunlock:用于解锁OS任务调度,重新开启调度
 *      delay_ostimedly    :用于OS延时,可以引起任务调度.
 *
 *  本例程仅作UCOSII的支持,其他OS,请自行参考着移植
 */

/* 支持UCOSII */
#define delay_osrunning     OSRunning           /* OS是否运行标记,0,不运行;1,在运行 */
#define delay_ostickspersec OS_TICKS_PER_SEC    /* OS时钟节拍,即每秒调度次数 */
#define delay_osintnesting  OSIntNesting        /* 中断嵌套级别,即中断嵌套次数 */


/**
 * @brief     us级延时时,关闭任务调度(防止打断us级延迟)
 * @param     无
 * @retval    无
 */
void delay_osschedlock(void)
{
    OSSchedLock();                      /* UCOSII的方式,禁止调度，防止打断us延时 */
}

/**
 * @brief     us级延时时,恢复任务调度
 * @param     无
 * @retval    无
 */
void delay_osschedunlock(void)
{
    OSSchedUnlock();                    /* UCOSII的方式,恢复调度 */
}

/**
 * @brief     us级延时时,恢复任务调度
 * @param     ticks: 延时的节拍数
 * @retval    无
 */
void delay_ostimedly(uint32_t ticks)
{
    OSTimeDly(ticks);                               /* UCOSII延时 */
}

/**
 * @brief     systick中断服务函数,使用OS时用到
 * @param     ticks : 延时的节拍数  
 * @retval    无
 */  
void SysTick_Handler(void)
{
    /* OS 开始跑了,才执行正常的调度处理 */
    if (delay_osrunning == OS_TRUE)
    {
        /* 调用 uC/OS-II 的 SysTick 中断服务函数 */
        OS_CPU_SysTickHandler();
    }
    HAL_IncTick();
}
#endif

/**
 * @brief   初始化延时
 * @param   sysclk: 系统时钟频率（单位：MHz）
 * @retval  无
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
 * @brief   微秒级延时
 * @param   nus: 延时时间（单位：us）
 * @retval  无
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
 * @brief   毫秒级延时
 * @param   nms: 延时时间（单位：ms）
 * @retval  无
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
 * @brief   HAL库毫秒级延时函数
 * @param   Delay: 延时时间（单位：ms）
 * @retval  无
 */
//void HAL_Delay(uint32_t Delay)
//{
//    delay_us(Delay * 1000);
//}
