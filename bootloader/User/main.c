/**
 ****************************************************************************************************
 * @file        main.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2024-05-21
 * @brief       内存管理实验
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

#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/delay/delay.h"
#include "./SYSTEM/usart/usart.h"
#include "./BSP/LED/led.h"
#include "./BSP/KEY/key.h"
#include "./BSP/HYPERRAM/hyperram.h"

#define APPLICATION_ADDRESS XSPI1_BASE  //(uint32_t)0x90000000

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}


/**
  * @brief FLASH Initialization Function
  * @param None
  * @retval None
  */
static void MX_FLASH_Init(void)
{

    FLASH_OBProgramInitTypeDef pOBInit = {0};

    if (HAL_FLASH_Unlock() != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_FLASH_OB_Unlock() != HAL_OK)
    {
        Error_Handler();
    }
    pOBInit.OptionType = OPTIONBYTE_USER;
    pOBInit.USERType = OB_USER_IWDG_SW | OB_USER_NRST_STOP
                       | OB_USER_NRST_STDBY | OB_USER_VDDIO_HSLV
                       | OB_USER_IWDG_STOP | OB_USER_IWDG_STDBY
                       | OB_USER_XSPI1_HSLV | OB_USER_XSPI2_HSLV
                       | OB_USER_I2C_NI3C;
    pOBInit.USERConfig1 = OB_IWDG_SW | OB_STOP_NORST
                          | OB_STANDBY_NORST | OB_VDDIO_HSLV_DISABLE
                          | OB_IWDG_STOP_RUN | OB_IWDG_STDBY_RUN
                          | OB_XSPI1_HSLV_ENABLE | OB_XSPI2_HSLV_ENABLE;
    pOBInit.USERConfig2 = OB_I2C_NI3C_I2C;
    if (HAL_FLASHEx_OBProgram(&pOBInit) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_FLASH_OB_Lock() != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_FLASH_Lock() != HAL_OK)
    {
        Error_Handler();
    }

}


/**
  * @brief 跳转程序，cpoy自Art-Pi2和安富莱的bootloader
  * @param None
  * @retval None
  */
int JumpToApplication(void)
{
    typedef void (*pFunction)(void);
    pFunction JumpToApp;
	uint32_t Application_vector;
	uint32_t i = 0;
	
	__set_PRIMASK(1);
	
	// app没法用cubemx生成配置时钟
//	HAL_RCC_DeInit();
	
    /* 关闭滴答定时器，复位到默认值 */
	SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;
	
	/* 关闭所有中断，清除所有中断挂起标志 */
	for (i = 0; i < 8; i++)
	{
		NVIC->ICER[i]=0xFFFFFFFF;
		NVIC->ICPR[i]=0xFFFFFFFF;
	}
	
    /* Disable I-Cache---------------------------------------------------------*/
    SCB_DisableICache();  //TODO SCB_Disables Cache and jump success

    /* Disable D-Cache---------------------------------------------------------*/
    SCB_DisableDCache();

    /* Apply offsets for image location and vector table offset */
    Application_vector = APPLICATION_ADDRESS;
    SCB->VTOR = (uint32_t)Application_vector;
    JumpToApp = (pFunction)(*(__IO uint32_t *)(Application_vector + 4u));

	__set_PRIMASK(0);
	
    __set_MSP(*(__IO uint32_t *)Application_vector);
    __set_CONTROL(0);

    JumpToApp();
    return 0;
}


int main(void)
{
	/* 配置MPU并使能Cache */
	sys_mpu_config();
    sys_cache_enable();                 
	
	/* 初始化HAL库 */
    HAL_Init();                         
	
	/* 配置时钟，600MHz，设置电源，flash初始化被单独拿出来了 */
    sys_stm32_clock_init(300, 6, 2);    
	
	/* 配置选项字，我看Art-Pi2的bootloader里面有配置 */
	MX_FLASH_Init();
	
	/* 使能XSPI1 NOR Flash内存映射 */
    sys_xspi1_enable_memmapmode();
    sys_memory_mapped_flag = 1;
	
	/* 初始化HyperRAM */
	hyperram_init();                    /* 初始化HyperRAM */
	
	/* 常规初始化 */
    delay_init(600);                    /* 初始化延时，HAL_DELAY覆写注释了 */
    usart_init(115200);                 /* 初始化串口 */
    led_init();                         /* 初始化LED */
    
	/* 跳转程序 */
	JumpToApplication();
	
	/* 正常不会进入这里 */
    while (1)
    {
        
    }
}
