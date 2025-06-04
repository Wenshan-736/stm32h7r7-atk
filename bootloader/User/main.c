/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2024-05-21
 * @brief       �ڴ����ʵ��
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
  * @brief ��ת����cpoy��Art-Pi2�Ͱ�������bootloader
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
	
	// appû����cubemx��������ʱ��
//	HAL_RCC_DeInit();
	
    /* �رյδ�ʱ������λ��Ĭ��ֵ */
	SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;
	
	/* �ر������жϣ���������жϹ����־ */
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
	/* ����MPU��ʹ��Cache */
	sys_mpu_config();
    sys_cache_enable();                 
	
	/* ��ʼ��HAL�� */
    HAL_Init();                         
	
	/* ����ʱ�ӣ�600MHz�����õ�Դ��flash��ʼ���������ó����� */
    sys_stm32_clock_init(300, 6, 2);    
	
	/* ����ѡ���֣��ҿ�Art-Pi2��bootloader���������� */
	MX_FLASH_Init();
	
	/* ʹ��XSPI1 NOR Flash�ڴ�ӳ�� */
    sys_xspi1_enable_memmapmode();
    sys_memory_mapped_flag = 1;
	
	/* ��ʼ��HyperRAM */
	hyperram_init();                    /* ��ʼ��HyperRAM */
	
	/* �����ʼ�� */
    delay_init(600);                    /* ��ʼ����ʱ��HAL_DELAY��дע���� */
    usart_init(115200);                 /* ��ʼ������ */
    led_init();                         /* ��ʼ��LED */
    
	/* ��ת���� */
	JumpToApplication();
	
	/* ��������������� */
    while (1)
    {
        
    }
}
