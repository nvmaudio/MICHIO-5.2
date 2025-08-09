/**
 *******************************************************************************
 * @file    clk.h
 * @brief	Clock driver interface
 *
 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2015-11-05 10:46:11$
 *
 * @Copyright (C) 2015, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 ******************************************************************************* 
 */
 
 
/**
 * @addtogroup CLOCK
 * @{
 * @defgroup clk clk.h
 * @{
 */
 
#ifndef __CLK_H__
#define __CLK_H__
 
#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include "spi_flash.h"
/**
 * CLK module switch macro define
 */
typedef enum __CLOCK_MODULE1_SWITCH
{
	MCLK_PLL_CLK0_EN = (1 << 0),		/**<MCLK PLL0 module clk switch */
	MCLK_PLL_CLK1_EN = (1 << 1),		/**<MCLK PLL1 module clk switch */
	FLASH_CONTROL_PLL_CLK_EN = (1 << 2),/**<Flash Control PLL module clk switch */
	SPDIF_PLL_CLK_EN = (1 << 3),  		/**<SPDIF PLL module clk switch */
	USB_UART_PLL_CLK_EN = (1 << 4),		/**<USB/UART PLL module clk switch */
	AUDIO_DAC0_CLK_EN = (1 << 5),		/**<Audio DAC port0 module clk switch */
	AUDIO_DAC1_CLK_EN = (1 << 6),       /**<Audio DAC port1 module clk switch */
	AUDIO_ADC0_CLK_EN = (1 << 7),		/**<Audio ADC port0 module clk switch */
	AUDIO_ADC1_CLK_EN = (1 << 8),       /**<Audio ADC port1 module clk switch */
	I2S0_CLK_EN = (1 << 9),				/**<I2S port0 module clk switch */
	I2S1_CLK_EN = (1 << 10),			/**<I2S port1 module clk switch */
	PPWM_CLK_EN = (1 << 11),			/**<PPWM module clk switch */
	FLASH_CONTROL_CLK_EN = (1 << 12),	/**<Flash control module clk switch */
	
	USB_CLK_EN = (1 << 14),           	/**<USB module clk switch */
	UART0_CLK_EN = (1 << 15),           /**<UART0 module clk switch */
	UART1_CLK_EN = (1 << 16),           /**<UART1 module clk switch */
	DMA_CLK_EN = (1 << 17),   			/**<DMA control module clk switch */
	SPDIF_CLK_EN = (1 << 18),           /**<SPDIF module clk switch */
	FFT_CLK_EN = (1 << 19),            	/**<FFT module clk switch */
	ADC_CLK_EN = (1 << 20),				/**<sarADC module clk switch */
	EFUSE_CLK_EN = (1 << 21),          	/**<EFUSE module clk switch */
	I2C_CLK_EN = (1 << 22),          	/**<I2C module clk switch */
	SPIM_CLK_EN = (1 << 23),          	/**<SPI master module clk switch */
	SDIO_CLK_EN = (1 << 24),          	/**<SDIO module clk switch */
	ROM_CLK_EN = (1 << 25),          	/**<ROM module clk switch */
	TIMER1_CLK_EN = (1 << 26),          /**<TIMER1 module clk switch */
	TIMER2_CLK_EN = (1 << 27),          /**<TIMER2 module clk switch */
	TIMER3_CLK_EN = (1 << 28),          /**<TIMER3 module clk switch */
	TIMER4_CLK_EN = (1 << 29),          /**<TIMER4 module clk switch */
	TIMER5_CLK_EN = (1 << 30),          /**<TIMER5 module clk switch */
	TIMER6_CLK_EN = (1 << 31),          /**<TIMER6 module clk switch */

	ALL_MODULE1_CLK_SWITCH = (0xFFFFFFFF),/**<all module clk SWITCH*/
} CLOCK_MODULE1_SWITCH;

typedef enum __CLOCK_MODULE2_SWITCH
{
	BTDM_HCLK_EN = (1 << 0),			/**< module clk switch */
	MDM_CLK_EN = (1 << 1),				/**< module clk switch */
	BTDM_MST_CLK_EN = (1 << 2),			/**< module clk switch */
	MDM_PCLK_EN = (1 << 3),				/**< module clk switch */
	PPWM_DBL_CLK_EN = (1 << 4),			/**< module clk switch */
	ALL_MODULE2_CLK_SWITCH = (0x1F),	/**<all module clk SWITCH*/
} CLOCK_MODULE2_SWITCH;

typedef enum __CLOCK_MODULE3_SWITCH
{
	USB_REG_CLK_EN = (1 << 0),			/**< module clk switch */
	SDIO_REG_CLK_EN = (1 << 1),			/**< module clk switch */
	SPIM_REG_CLK_EN = (1 << 2),			/**< module clk switch */
	SPIS_REG_CLK_EN = (1 << 3),			/**< module clk switch */
	UART0_REG_CLK_EN = (1 << 4),		/**< module clk switch */
	UART1_REG_CLK_EN = (1 << 5),		/**< module clk switch */
	SPDIF_REG_CLK_EN = (1 << 6),		/**< module clk switch */
	ALL_MODULE3_CLK_SWITCH = (0x7F),	/**<all module clk SWITCH*/
} CLOCK_MODULE3_SWITCH;


typedef enum __CLK_MODE{
	RC_CLK_MODE,	/*RC 12M source*/
	PLL_CLK_MODE,	/*DPLL source*/
	APLL_CLK_MODE,	/*APLL source*/
	SYSTEM_CLK_MODE /*System Clock*/
}CLK_MODE;

typedef enum _CLK_32K_MODE{
	RC_CLK32_MODE,			/*RC32K(RC 12M div) source*/
	OSC_32K_MODE,			/*OSC32K source(HOSC_DIV_32K || LOSC_32K)*/
	HOSC_DIV_32K_CLK_MODE,	/*HOSC DIV source*/
	LOSC_32K_MODE,			/*LOSC 32K source*/
}CLK_32K_MODE;


typedef enum __PLL_CLK_INDEX{
	PLL_CLK_1 = 0,	/**PLL��Ƶʱ����1,������������Ϊ11.2896M�����������޸�*/
	PLL_CLK_2		/**PLL��Ƶʱ����2,������������Ϊ12.288M�����������޸�*/
}PLL_CLK_INDEX;

typedef enum __AUDIO_MODULE{
	AUDIO_DAC0,
	AUDIO_DAC1,
	AUDIO_ADC0,
	AUDIO_ADC1,
	AUDIO_I2S0,
	AUDIO_I2S1,
	AUDIO_PPWM
}AUDIO_MODULE;

typedef enum __MCLK_CLK_SEL{
	PLL_CLOCK1 = 0,	/**PLL��Ƶ1ʱ�ӣ���������PLL��Ƶʱ��������Ƶ��Ϊ11.2896M����������Ƶ��*/
	PLL_CLOCK2,		/**PLL��Ƶ2ʱ�ӣ���������PLL��Ƶʱ��������Ƶ��Ϊ12.288M����������Ƶ��*/
	GPIO_IN0,		/**GPIO MCLK in0����*/
	GPIO_IN1		/**GPIO MCLK in1����*/
}MCLK_CLK_SEL;


typedef enum __CLOCK_GPIO_Port
{
	GPIO_CLK_OUT_A29,		/**GPIO A29���ʱ�ӹ۲�˿�*/
	GPIO_CLK_OUT_B6,		/**GPIO B6���ʱ�ӹ۲�˿�*/
	GPIO_CLK_OUT_B7,		/**GPIO B7���ʱ�ӹ۲�˿�*/
}CLK_GPIO_Port;

typedef enum __CLOCK_OUT_MODE
{
	MCLK_PLL_CLK = 0,		//0:   mclk_pll_clk0
	MCLK_PLL_CLK_DIV,	//1:   mclk_pll_clk0_div
	SPDIF_CLK,			//2:   spdif_clk
	Flash_Control_CLK,	//3:   fshc_clk
	USB_CLK,			//4:   usb_clk
	APB_CLK,			//5:   apb_clk
	SPIM_CLK,			//6:   spim_clk
	SDIO_CLK,			//7:   sdio_clk
	RC_CLK,				//8:   rc_clk
	BTDM_CLK,			//9:   btdm_hclk
	HOSC_CLK_DIV,		//10:  hosc_clk_div, ����ʱ�ӹ̶�2��Ƶ֮�����12M��������FMʱ��Դ
	BTDM_LP_CLK,		//11:  btdm_lp_clk
	OSC_32K_CLK,		//12:  osc_32k_clk������оƬ�ڲ�RTCʹ�ã����ڹ۲⣬��������FMʱ��Դ
	DPLL_CLK_DIV,		//13:  pll_clk_div
	APLL_CLK_DIV		//14:  apll_clk_div
}CLOCK_OUT_MODE;

//��������sniffʱstack��¼��ǰʱ��״̬
typedef struct __BTSNIFF_GET_DEFAULT_CLKCONFIG
{
	unsigned char sys_div;
	unsigned char core_div;
	unsigned int sys_clk;
	int get1V2;
}BTSNIFF_GET_DEFAULT_CLKCONFIG_t;

//����MCLK0 ����Ϊ11.2896M��MCLK1����Ϊ12.288M

#define		AUDIO_PLL_CLK1_FREQ		11289600//PLL1,11.2896MHz
#define		AUDIO_PLL_CLK2_FREQ		12288000//PLL2,12.288MHz


/**
 * @brief	ϵͳ�ο�ʱ��Դ����ѡ��
 * @param	IsOsc TURE������ʱ�ӣ�FALSE��XIN�˿���෽��ʱ��
 * @param   Freq  ϵͳ�ο�ʱ�ӵĹ���Ƶ�ʣ�32.768K,1M,2M,...40M,��λHZ
 * @return	��
 * @note	���ʹ��PLLʱ�������Ҫ�ȵ��øú���
 */
void Clock_Config(bool IsOsc, uint32_t Freq);

/**
 * @brief	��ȡCore������ʱ��Ƶ��
 * @param	��
 * @return	Core����Ƶ��
 * @note    Coreʱ�Ӻ�ϵͳʱ�ӿ���ͬƵ����ʱ�������144MHz
 */
uint32_t Clock_CoreClockFreqGet(void);

/**
 * @brief  ����Coreʱ�ӷ�Ƶϵ��
 * @param  DivVal [1-256]��Freq = Fpll/Div
 * @return ��
 * @note    Coreʱ�Ӻ�ϵͳʱ�ӿ���ͬƵ����ʱ�������144MHz
 */
void Clock_CoreClkDivSet(uint32_t DivVal);

/**
 * @brief   ��ȡCore��Ƶϵ��
 * @param   ��
 * @return  ϵͳ��Ƶϵ��[0-255]
 * @note    Coreʱ�Ӻ�ϵͳʱ�ӿ���ͬƵ����ʱ�������144MHz
 */
uint32_t Clock_CoreClkDivGet(void);

/**
 * @brief	��ȡϵͳ������ʱ��Ƶ��
 * @param	��
 * @return	ϵͳ����Ƶ��
 * @note    Coreʱ�Ӻ�ϵͳʱ�ӿ���ͬƵ����ʱ�������144MHz
 */
uint32_t Clock_SysClockFreqGet(void);

/**
 * @brief   ����ϵͳʱ�ӷ�Ƶϵ��
 * @param   DivVal [0-64]��0�رշ�Ƶ����, 1����Ƶ
 * @return  ��
 * @note    Coreʱ�Ӻ�ϵͳʱ�ӿ���ͬƵ����ʱ�������144MHz
 */
void Clock_SysClkDivSet(uint32_t DivVal);
    
/**
 * @brief   ��ȡϵͳʱ�ӷ�Ƶϵ��
 * @param   NONE
 * @return  ϵͳ��Ƶϵ��[1-64]
 */
uint32_t Clock_SysClkDivGet(void);

/**
 * @brief   ����APB���߷�Ƶϵ��������ϵͳ���߷�Ƶ
 * @param   DivVal [2-14]
 * @return  ��
 */
void Clock_ApbClkDivSet(uint32_t DivVal);

/**
 * @brief   ��ȡAPB���߷�Ƶϵ��������ϵͳ���߷�Ƶ
 * @param   ��
 * @return  APB���߷�Ƶϵ��[2-14]
 * @note	APBʱ�Ӳ��ܴ���37.5MHz
 */
uint32_t Clock_ApbClkDivGet(void);

/**
 * @brief   ����Spdifʱ�ӷ�Ƶϵ��������PLLʱ�ӷ�Ƶ
 * @param   DivVal [2-32]
 * @return  ��
 */
void Clock_SpdifClkDivSet(uint32_t DivVal);

/**
 * @brief   ��ȡSpdifʱ�ӷ�Ƶϵ��������PLLʱ�ӷ�Ƶ
 * @param   ��
 * @return  Spdifʱ�ӷ�Ƶϵ��[2-32]
 */
uint32_t Clock_SpdifClkDivGet(void);

/**
 * @brief   Cấu hình hệ số chia tần số cho xung nhịp USB dựa trên xung PLL
 * @param   DivVal [2 - 16]  Giá trị chia tần số
 * @return  Không có giá trị trả về
 * @note    Hàm này dùng để thiết lập tần số xung nhịp USB bằng cách chia xung PLL theo DivVal
 */
void Clock_USBClkDivSet(uint32_t DivVal);

/**
 * @brief   ��ȡUSBʱ�ӷ�Ƶϵ��������PLLʱ�ӷ�Ƶ
 * @param   ��
 * @return  USBʱ�ӷ�Ƶϵ��[2-16]
 */
uint32_t Clock_USBClkDivGet(void);

/**
 * @brief  	����SPI master����ʱ�ӷ�Ƶϵ������ϵͳʱ�ӷ�Ƶ
 * @param  	DivVal ��Ƶϵ��[1-15]
 * @return  ��
 * @note	SPIMʱ�Ӳ�����120M,����Ϊ1ʱ��SPIMʱ��Ϊϵͳʱ�ӣ���Ҫ��SPIM�ӿ���ȥ����Ƶ��
 */
void Clock_SPIMClkDivSet(uint32_t DivVal);

/**
 * @brief  ��ȡSPI master����ʱ�ӷ�Ƶϵ������ϵͳʱ�ӷ�Ƶ
 * @param  ��
 * @return  ��Ƶϵ��[2-15]
 */
uint32_t Clock_SPIMClkDivGet(void);

/**
 * @brief   ����SDIO0����ʱ�ӷ�Ƶϵ������ϵͳʱ�ӷ�Ƶ
 * @param   DivVal ��Ƶϵ��[2-15]
 * @return  ��
 * @note	SDIOʱ�Ӳ��ܴ���60MHz
 */
void Clock_SDIOClkDivSet(uint32_t DivVal);

/**
 * @brief  ��ȡSDIO0����ʱ�ӷ�Ƶϵ������ϵͳʱ�ӷ�Ƶ
 * @param  ��
 * @return  ��Ƶϵ��[2-15]
 */
uint32_t Clock_SDIOClkDivGet(void);

/**
 * @brief  ����RC32K��Ƶϵ��������RC12M��Ƶ����ʱ����Ҫ��WDGʹ��
 * @param  DivVal ��Ƶϵ��[2-2048]
 * @return  ��
 */
void Clock_RC32KClkDivSet(uint32_t DivVal);

/**
 * @brief  ��ȡRC32K��Ƶϵ��������RC12M��Ƶ����ʱ����Ҫ��WDGʹ��
 * @param  ��
 * @return  ��Ƶϵ��[2-512]
 */
uint32_t Clock_RC32KClkDivGet(void);

/**
 * @brief	����OSC��Ƶϵ������Ƶ֮���ʱ���ṩ��RTCʹ��
 * @param	DivVal ��Ƶϵ��[1-2048]��
 * @return  ��
 */
void Clock_OSCClkDivSet(uint32_t DivVal);

/**
 * @brief	��ȡOSC��Ƶϵ������Ƶ֮���ʱ���ṩ��RTCʹ��
 * @param	��
 * @return  OSC��Ƶϵ��[1-2048]
 */
uint32_t Clock_OSCClkDivGet(void);

/**
 * @brief	Cấu hình tần số PLL hệ thống, chờ PLL khóa tần số
 * @param	PllFreq Tần số PLL, đơn vị KHz [120000K - 288000K]
 * @return  Trạng thái PLL đã khóa thành công hay chưa
 *          TRUE: Tần số mục tiêu đã được thiết lập thành công
 */
bool Clock_PllLock(uint32_t PllFreq);

/**
 * @brief	Cấu hình tần số APLL (Audio PLL), chờ PLL khóa tần số
 * @param	APllFreq Tần số APLL, đơn vị KHz [120000K - 240000K]
 * @return  Trạng thái PLL đã khóa thành công hay chưa
 *          TRUE: Tần số mục tiêu đã được thiết lập thành công
 */
bool Clock_APllLock(uint32_t PllFreq);


/**
 * @brief	��ȡrcƵ��
 * @param	IsReCount �Ƿ��ٴλ�ȡӲ��������ֵ��TRUE���ٴ�����Ӳ����������FALSE����ȡ�ϴμ�¼ֵ��
 * @return  rcƵ�ʣ���λHz
 */	
uint32_t Clock_RcFreqGet(bool IsReCount);

/**
 * @brief	��ȡpllƵ��
 * @param	��
 * @return  pllƵ��
 * @note    �ú���������ҪӦ��ȷ��pll�Ѿ�lock�������������
 */
uint32_t Clock_PllFreqGet(void);

/**
 * @brief	����apll����Ƶ��,�ȴ�apll lock
 * @param	PllFreq pllƵ��,��λKHz[120000K-360000K]
 * @return  PLL�������  TRUE:���趨Ŀ��Ƶ������
 */
uint32_t Clock_APllFreqGet(void);

/**
 * @brief	��ȡpllƵ��
 * @param	Win ���������ڡ�3:2048��
 * @return  ��
 */
void Clock_RcCntWindowSet(uint32_t Win);

/**
 * @brief	�ر�rcƵ��Ӳ���Զ����¹���
 * @param	��
 * @return  ��
 */
void Clock_RcFreqAutoCntDisable(void);

/**
 * @brief	����rcƵ��Ӳ���Զ����¹���
 * @param	��
 * @return  ��
 */
void Clock_RcFreqAutoCntStart(void);

/**
 * @brief	��ȡrcƵ�ʣ�Ӳ���Զ����£�
 * @param	��
 * @return  ��
 */
uint32_t Clock_RcFreqAutoCntGet(void);

/*
 * ���´��뵥�λ�ȡRCƵ��
 * ex:
 *
 * Clock_RcFreqCntOneTimeStart();
 * while(!Clock_RcFreqCntOneTimeReady)
 * {
 * 		rc12 = Clock_RcFreqCntOneTimeGet(1);
 * }
*/

/**
 * @brief	���λ�ȡRCʱ�ӿ�ʼ
 * @param	��
 * @return  ��
 */
void Clock_RcFreqCntOneTimeStart(void);

/**
 * @brief	���λ�ȡRCʱ�ӿɶ�ȡ״̬
 * @param	��
 * @return  1 �ɶ�
 * 			0 ���ɶ�
 */
bool Clock_RcFreqCntOneTimeReady(void);

/**
 * @brief	���λ�ȡrcƵ��
 * @param	IsReCount �Ƿ��ٴλ�ȡӲ��������ֵ��TRUE���ٴ�����Ӳ����������FALSE����ȡ�ϴμ�¼ֵ��
 * @return  rcƵ�ʣ���λHz
 */
uint32_t Clock_RcFreqCntOneTimeGet(bool IsReCount);

/**
 * @brief	����pll����Ƶ��,��������ģʽ���ȴ�pll lock
 * @param	PllFreq pllƵ��,��λKHz[240000K-480000K]
 * @param	K1 [0-15]
 * @param	OS [0-31]
 * @param	NDAC [0-4095]
 * @param	FC [0-2]
 * @param	Slope [0-16777216]
 * @return  PLL�������  TRUE:���趨Ŀ��Ƶ������
 */
bool Clock_PllQuicklock(uint32_t PllFreq, uint8_t K1, uint8_t OS, uint32_t NDAC, uint32_t FC, uint32_t Slope);

/**
 * @brief	����pll����Ƶ��,��������ģʽ(����Ҫ���壬����У׼)
 * @param	PllFreq pllƵ��,��λKHz[240000K-480000K]
 * @param	K1 [0-15]
 * @param	OS [0-31]
 * @param	NDAC [0-4095]
 * @param	FC [0-2]
 * @return  ��
 */
void Clock_PllFreeRun(uint32_t PllFreq, uint32_t K1, uint32_t OS, uint32_t NDAC, uint32_t FC);

/**
 * @brief	pll free run һ��ֻ��Ҫ���˺�������
 * @param	��
 * @return  TRUE, ���óɹ�
 */
bool Clock_PllFreeRunEfuse(void);
/**
 * @brief	pllģ��ر�
 * @param	��
 * @return  ��
 */
void Clock_PllClose(void);

/**
 * @brief	AudioPllģ��ر�
 * @param	��
 * @return  ��
 */
void Clock_AUPllClose(void);

/**
 * @brief	ѡ��ϵͳ����ʱ��
 * @param	ClkMode, RC_CLK_MODE: RC12Mʱ��; PLL_CLK_MODE:pllʱ��;
 * @return  �Ƿ�ɹ��л�ϵͳʱ�ӣ�TRUE�������л���FALSE���л�ʱ��ʧ�ܡ�
 * @note    ϵͳʱ�ӹ�����pllʱ��ʱ��Ϊpllʱ�ӵ�4��Ƶ
 */
bool Clock_SysClkSelect(CLK_MODE ClkMode);

/**
 * @brief	Spdifģ��ʱ��ѡ��,DPLLʱ�ӻ���APLLʱ��
 * @param	ClkMode	ʱ��Դѡ��
 *   @arg	PLL_CLK_MODE:DPLLʱ�ӣ�PLL��Ƶ֮��
 *   @arg	APLL_CLK_MODE:APLLʱ�ӣ�APLL��Ƶ֮��
 * @return  ��
 */
void Clock_SpdifClkSelect(CLK_MODE ClkMode);

/**
 * @brief	USBģ��ʱ��ѡ��,DPLLʱ�ӻ���APLLʱ��
 * @param	ClkMode	ʱ��Դѡ��
 *   @arg	PLL_CLK_MODE:DPLLʱ�ӣ�PLL��Ƶ֮��
 *   @arg	APLL_CLK_MODE:APLLʱ�ӣ�APLL��Ƶ֮��
 * @return  ��
 * @note	ʱ��Ƶ�ʲ�Ҫ����60M��һ������Ϊ60M
 */
void Clock_USBClkSelect(CLK_MODE ClkMode);

/**
 * @brief	Uartģ��ʱ��ѡ��,pllʱ�ӻ���RCʱ��
 * @param	ClkMode ʱ��Դѡ�� 
 *   @arg	RC_CLK_MODE: RCʱ��;
 *   @arg	PLL_CLK_MODE:DPLLʱ�ӣ�PLL��Ƶ֮��
 *   @arg	APLL_CLK_MODE:APLLʱ�ӣ�APLL��Ƶ֮��
 * @return  ��
 * @note	ע���ʱ��Դ���ѡ��PLL����APLLʱ�ӣ��Ǻ�USBʱ��ԴͬԴ
 */
void Clock_UARTClkSelect(CLK_MODE ClkMode);

/**
 * @brief	Timer3ģ��ʱ��ѡ��,ϵͳʱ��ʱ�ӻ���RCʱ��
 * @param	ClkMode SYSTEM_CLK_MODE: ϵͳʱ��; RC_CLK_MODE:RC 12Mʱ��
 * @return  ��
 */
void Clock_Timer3ClkSelect(CLK_MODE ClkMode);

/**
 * @brief	����32Kʱ��ѡ��,ϵͳʱ��ʱ�ӻ���RCʱ��
 * @param	ClkMode 32K����ʱ��Դѡ��
 *   @arg	HOSC_DIV_32K_CLK_MODE: ��Ƶ����ʱ�ӷ�Ƶ��32;
 *   @arg	LOSC_32K_MODE: 32K����ʱ��;
 * @return  ��
 */
void Clock_OSC32KClkSelect(CLK_32K_MODE ClkMode);

/**
 * @brief	BTDMģ��ʱ��ѡ��,ϵͳʱ��ʱ�ӻ���RCʱ��
 * @param	ClkMode BTDMģ��
 *   @arg	RC_CLK32_MODE: RC12Mʱ�ӷ�Ƶ��32K;
 *   @arg	OSC_32K_MODE: ����32Kʱ��(ע����Դ);
 * @return  ��
 */
void Clock_BTDMClkSelect(CLK_32K_MODE ClkMode);
CLK_32K_MODE Clock_BTDMClkSelectGet( void);

/**
 * @brief	ģ��ʱ��ʹ��
 * @param	ClkSel ģ��Դ������CLOCK_MODULE1_SWITCHѡ��
 * @return  ��
 */
void Clock_Module1Enable(CLOCK_MODULE1_SWITCH ClkSel);

/**
 * @brief	ģ��ʱ�ӽ���
 * @param	ClkSel ģ��Դ������CLOCK_MODULE1_SWITCHѡ��
 * @return  ��
 */
void Clock_Module1Disable(CLOCK_MODULE1_SWITCH ClkSel);

/**
 * @brief	ģ��ʱ��ʹ��
 * @param	ClkSel ģ��Դ������CLOCK_MODULE2_SWITCHѡ��
 * @return  ��
 */
void Clock_Module2Enable(CLOCK_MODULE2_SWITCH ClkSel);

/**
 * @brief	ģ��ʱ�ӽ���
 * @param	ClkSel ģ��Դ������CLOCK_MODULE2_SWITCHѡ��
 * @return  ��
 */
void Clock_Module2Disable(CLOCK_MODULE2_SWITCH ClkSel);

/**
 * @brief	ģ��ʱ��ʹ��
 * @param	ClkSel ģ��Դ������CLOCK_MODULE3_SWITCHѡ��
 * @return  ��
 */
void Clock_Module3Enable(CLOCK_MODULE3_SWITCH ClkSel);

/**
 * @brief	ģ��ʱ�ӽ���
 * @param	ClkSel ģ��Դ������CLOCK_MODULE3_SWITCHѡ��
 * @return  ��
 */
void Clock_Module3Disable(CLOCK_MODULE3_SWITCH ClkSel);

/**
 * @brief	���ø�Ƶ����������������
 * @param	XICap ����������������
 * @param	XOCap ����������������
 * @return  ��
 */
void Clock_HOSCCapSet(uint32_t XICap, uint32_t XOCap);

/**
 * @brief	���ø�Ƶ����������������ƫ�õ���
 * @param	Current ����ֵ����0x0-0xF����0xF���
 * @return  ��
 * @note	ע�⣬����ֵ�ĵ�����Ӱ��24M�������Ƶ��
 */
void Clock_HOSCCurrentSet(uint32_t Current);

/**
 * @brief	��ƵPLLʱ������
 * @param	CLK_MODE PLLԴ: PLL_CLK ���� APLL_CLK
 * @param	Index PLLԴ��PLL_CLK_1:11.2896M;PLL_CLK_2:12.288M;
 * @param	TargetFreq ��������ƵMCLKƵ�ʣ��Ƽ�����PLL1�� 11 2896M��PLL2: 12.288M��Unit:Hz
 * @return  ��
 */
void Clock_AudioPllClockSet(CLK_MODE ClkMode, PLL_CLK_INDEX Index, uint32_t TargetFreq);

/**
 * @brief	��Ƶģ����ʱ��Դѡ��
 * @param	Module ��Ƶģ��
 * @param	ClkSel ʱ����Դ����ѡ��
 * @return  ��
 */
void Clock_AudioMclkSel(AUDIO_MODULE Module, MCLK_CLK_SEL ClkSel);

/**
 * @brief	��Ƶʱ��Դ������΢����PLL1��PLL2
 * @param	Index PLLԴ��PLL_CLK_1:11.2896M;PLL_CLK_2:12.288M;
 * @param	Sign  0��������1������
 * @param	Ppm ʱ��΢��������Ϊ0ʱӲ��ʱ����΢�����ܡ�
 * @return  ��
 */
void Clock_AudioPllClockAdjust(PLL_CLK_INDEX Index,uint8_t Sign, uint8_t Ppm);

/**
 * @brief	ʹ��DeepSleep����ʱ��ϵͳʱ��ѡ���flashʱ��ѡ��
 * @param	ClockSelect
 * @param	FlashClockSel
 * @param	IsEnterDeepSeep  TRUE:ϵͳ����DeepSleep  FALSE:ϵͳ����DeepSleep
 * @return  bool  TRUE:ʱ���л��ɹ�     FALSE:ʱ���л�ʧ��
 */
bool Clock_DeepSleepSysClkSelect(CLK_MODE SysClockSelect, FSHC_CLK_MODE FlashClockSel, bool IsEnterDeepSeep);

/**
 * @brief	HOSC����ر�
 * @param	��
 * @return  ��
 */
void Clock_HOSCDisable(void);

/**
 * @brief	LOSC����ر�
 * @param	��
 * @return  ��
 */
void Clock_LOSCDisable(void);

/**
 * @brief	ʱ�������GPIO�˿���
 * @param	CLK_GPIO_Port GPIO�������ʱ�Ӷ˿ڣ�GPIOA29��GPIOB6��GPIOB7
 * @param	CLOCK_OUT_MODE ʱ�������Դ
 * @return  ��
 */
void Clock_GPIOOutSel(CLK_GPIO_Port Port, CLOCK_OUT_MODE mode);

/**
 * @brief	�ر�GPIO�˿��ϵ�ʱ���������
 * @param	CLK_GPIO_Port GPIO�������ʱ�Ӷ˿ڣ�GPIOA29��GPIOB6��GPIOB7
 * @return  ��
 */
void Clock_GPIOOutDisable(CLK_GPIO_Port Port);

/**
 * @brief	����BB����HOSC�Ĺ��ܣ�Ĭ��Ϊ1
 * @param	0 : �ر�BB����HOSC
 * @param	1 : ����BB����HOSC
 * @return  ��
 */
void Clock_BBCtrlHOSCInDeepsleep(uint8_t set);
uint8_t Clock_BBCtrlHOSCInDeepsleepGet(void);

/**
 * @brief	HOSC��32K�ķ�Ƶϵ��
 * @param	clk_div : ��Ƶϵ��ֵ(����)
 * @return  ��
 */
void Clock_32KClkDivSet(uint32_t clk_div);
uint32_t Clock_32KClkDivGet(void);

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif

/**
 * @}
 * @}
 */
 
