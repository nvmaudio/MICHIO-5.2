/**
 *******************************************************************************
 * @file    powercontroller.h
 * @brief	powercontroller module driver interface

 * @author  Sean
 * @version V1.0.0

 * $Created: 2017-11-13 16:51:05$
 * @Copyright (C) 2017, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 *******************************************************************************
 */

/**
 * @addtogroup POWERCONTROLLER
 * @{
 * @defgroup powercontroller powercontroller.h
 * @{
 */
 
#ifndef __POWERCONTROLLER_H__
#define __POWERCONTROLLER_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include "type.h"

typedef enum __PWR_SYSWAKEUP_SOURCE_SEL{
	SYSWAKEUP_SOURCE0_GPIO = (1 << 0),				/**<system wakeup source0 macro,can be waked up by both edge*/
	SYSWAKEUP_SOURCE1_GPIO = (1 << 1),				/**<system wakeup source1 macro,can be waked up by both edge*/
	SYSWAKEUP_SOURCE2_GPIO = (1 << 2),				/**<system wakeup source2 macro,can be waked up by both edge*/
	SYSWAKEUP_SOURCE3_GPIO = (1 << 3),				/**<system wakeup source3 macro,can be waked up by both edge*/
	SYSWAKEUP_SOURCE4_GPIO = (1 << 4),				/**<system wakeup source4 macro,can be waked up by both edge*/
	SYSWAKEUP_SOURCE5_GPIO = (1 << 5),				/**<system wakeup source5 macro,can be waked up by both edge*/
	SYSWAKEUP_SOURCE6_POWERKEY = (1 << 6),				/**<system wakeup source6 macro,can be waked up by both edge*/
	SYSWAKEUP_SOURCE7_RTC = (1 << 7),				/**<system wakeup source7 macro,should be waked up by rise edge*/
	SYSWAKEUP_SOURCE8_LVD = (1 << 8),				/**<system wakeup source8 macro,һ�㲻��,should be waked up by rise edge*/
	SYSWAKEUP_SOURCE9_IR = (1 << 9),				/**<system wakeup source9 macro,should be waked up by rise edge*/
	SYSWAKEUP_SOURCE10_TIMER3_INT = (1 << 10),        /**<system wakeup source10 macro,should be waked up by rise edge*/
	SYSWAKEUP_SOURCE11_UART0_RX = (1 << 11),          /**<system wakeup source11 macro,should be waked up by rise edge*/
	SYSWAKEUP_SOURCE12_UART1_RX = (1 << 12),          /**<system wakeup source12 macro,should be waked up by rise edge*/
	SYSWAKEUP_SOURCE13_BT = (1 << 13)				/**<system wakeup source13 macro,should be waked up by rise edge*/

}PWR_SYSWAKEUP_SOURCE_SEL;

typedef enum __PWR_WAKEUP_GPIO_SEL{
	WAKEUP_GPIOA0 = 0,          /**<wakeup by GPIOA0 macro*/
	WAKEUP_GPIOA1,              /**<wakeup by GPIOA1 macro*/
	WAKEUP_GPIOA2,              /**<wakeup by GPIOA2 macro*/
	WAKEUP_GPIOA3,              /**<wakeup by GPIOA3 macro*/
	WAKEUP_GPIOA4,              /**<wakeup by GPIOA4 macro*/
	WAKEUP_GPIOA5, 				/**<wakeup by GPIOA5 macro*/
	WAKEUP_GPIOA6, 				/**<wakeup by GPIOA6 macro*/
	WAKEUP_GPIOA7, 				/**<wakeup by GPIOA7 macro*/
	WAKEUP_GPIOA8,			    /**<wakeup by GPIOA8 macro*/
	WAKEUP_GPIOA9, 				/**<wakeup by GPIOA9 macro*/
	WAKEUP_GPIOA10, 			/**<wakeup by GPIOA10 macro*/
	WAKEUP_GPIOA11, 			/**<wakeup by GPIOA11 macro*/
	WAKEUP_GPIOA12, 			/**<wakeup by GPIOA12 macro*/
	WAKEUP_GPIOA13, 			/**<wakeup by GPIOA13 macro*/
	WAKEUP_GPIOA14, 			/**<wakeup by GPIOA14 macro*/
	WAKEUP_GPIOA15, 			/**<wakeup by GPIOA15 macro*/
	WAKEUP_GPIOA16,				/**<wakeup by GPIOA16 macro*/
	WAKEUP_GPIOA17,			    /**<wakeup by GPIOA17 macro*/
	WAKEUP_GPIOA18,				/**<wakeup by GPIOA18 macro*/
	WAKEUP_GPIOA19,             /**<wakeup by GPIOA19 macro*/
	WAKEUP_GPIOA20,			    /**<wakeup by GPIOA20 macro*/
	WAKEUP_GPIOA21, 			/**<wakeup by GPIOA21 macro*/
	WAKEUP_GPIOA22,				/**<wakeup by GPIOA22 macro*/
	WAKEUP_GPIOA23,				/**<wakeup by GPIOA23 macro*/
	WAKEUP_GPIOA24, 			/**<wakeup by GPIOA24 macro*/
	WAKEUP_GPIOA25,		    	/**<wakeup by GPIOA25 macro*/
	WAKEUP_GPIOA26,			    /**<wakeup by GPIOA26 macro*/
	WAKEUP_GPIOA27,             /**<wakeup by GPIOA27 macro*/
	WAKEUP_GPIOA28,             /**<wakeup by GPIOA28 macro*/
	WAKEUP_GPIOA29,             /**<wakeup by GPIOA29 macro*/
	WAKEUP_GPIOA30,             /**<wakeup by GPIOA30 macro*/
	WAKEUP_GPIOA31,             /**<wakeup by GPIOA31 macro*/
	WAKEUP_GPIOB0,              /**<wakeup by GPIOB0 macro*/
	WAKEUP_GPIOB1,              /**<wakeup by GPIOB1 macro*/
	WAKEUP_GPIOB2,              /**<wakeup by GPIOB2 macro*/
	WAKEUP_GPIOB3,              /**<wakeup by GPIOB3 macro*/
	WAKEUP_GPIOB4,              /**<wakeup by GPIOB4 macro*/
	WAKEUP_GPIOB5,              /**<wakeup by GPIOB5 macro*/
	WAKEUP_GPIOB6,              /**<wakeup by GPIOB6 macro*/
	WAKEUP_GPIOB7,              /**<wakeup by GPIOB7 macro*/
	WAKEUP_GPIOB8,              /**<wakeup by GPIOB8 macro*/
	WAKEUP_GPIOC0               /**<wakeup by GPIOC0 macro*/

}PWR_WAKEUP_GPIO_SEL;

typedef enum __PWR_SYSWAKEUP_SOURCE_EDGE_SEL{
	SYSWAKEUP_SOURCE_NEGE_TRIG = 0,  /**negedge trigger*/
	SYSWAKEUP_SOURCE_POSE_TRIG,       /**posedge trigger*/
	SYSWAKEUP_SOURCE_BOTH_EDGES_TRIG       /**both edges trigger*/
	                                 /**ֻ��SourSelΪ[0~6]���������ػ����½��ش���֮�֣�����SourSel����Ĭ�������ش���*/

}PWR_SYSWAKEUP_SOURCE_EDGE_SEL;

typedef enum __PWR_LVD_Threshold_SEL{
	PWR_LVD_Threshold_2V3 = 0,  /**LVD threshold select:2.3V */
	PWR_LVD_Threshold_2V4,       /**LVD threshold select:2.4V*/
	PWR_LVD_Threshold_2V5,       /**LVD threshold select:2.5V*/
	PWR_LVD_Threshold_2V6,       /**LVD threshold select:2.6V*/
	PWR_LVD_Threshold_2V7,       /**LVD threshold select:2.7V*/
	PWR_LVD_Threshold_2V8,       /**LVD threshold select:2.8V*/
	PWR_LVD_Threshold_2V9,       /**LVD threshold select:2.9V*/
	PWR_LVD_Threshold_3V0,       /**LVD threshold select:3.0V*/

}PWR_LVD_Threshold_SEL;

/**
 * @brief  ϵͳ����sleepģʽ
 * @param  ��
 * @return ��
 */
void Power_GotoSleep(void);

/**
 * @brief  ϵͳ����deepsleepģʽ
 * @param  ��
 * @return ��
 */
void Power_GotoDeepSleep(void);

/**
 * @brief  ����DeepSleep����Դ
 * @param  SourSel ���û���Դ,�ֱ�Ϊ��GPIO��PowerKey,RTC,LVD(һ�㲻��),IR,BTΪ�����¼�����
 * @param  Gpio   ����gpio�������ţ�[0~41],�ֱ��ӦGPIOA[31:0]��GPIOB[8:0]��GPIOC[0]
 * @param  Edge   gpio�ı��ش�����ʽѡ��1������0�½���
 * @note   ����gpio��    ֻ��SourSelΪGPIO����Ч
 *         ����edge��    ֻ��SourSelΪGPIO��PowerKey���������ػ����½��ش���֮�֣�����SourSel����Ĭ�������ش���
 * @return ��
 */
void Power_WakeupSourceSet(PWR_SYSWAKEUP_SOURCE_SEL SourSel, PWR_WAKEUP_GPIO_SEL Gpio, PWR_SYSWAKEUP_SOURCE_EDGE_SEL Edge);

/**
 * @brief  ����ĳ��ͨ������ʹ��
 * @param  SourSel ����Դ:GPIO��PowerKey,RTC,LVD(һ�㲻��),IR,BTΪ�����¼�����
 * @return ��
 */
void Power_WakeupEnable(PWR_SYSWAKEUP_SOURCE_SEL SourSel);

/**
 * @brief  ����ĳ��ͨ�����ѽ�ֹ
 * @param  SourSel ����Դ:GPIO��PowerKey,RTC,LVD(һ�㲻��),IR,BTΪ�����¼�����
 * @return ��
 */
void Power_WakeupDisable(PWR_SYSWAKEUP_SOURCE_SEL SourSel);

/**
 * @brief  ��ȡ����ͨ����־
 * @param  ��
 * @return ��ȡ����ͨ����־
 */
uint32_t Power_WakeupSourceGet(void);

/**
 * @brief  �������ͨ����־
 * @param  ��
 * @return ��
 */
void Power_WakeupSourceClear(void);

/**
 * @brief  ͨ������Դ��־��ѯgpio�������ţ�[0~41],�ֱ��ӦGPIOA[31:0]��GPIOB[8:0]��GPIOC[0]
 * @param  SourSelΪ����ȡ�Ļ���Դ���ֱ�Ϊ��GPIO��PowerKey,RTC,LVD(һ�㲻��),IR,Timer3�жϣ���ʱ����PWC����UART0����UART1
 * @return gpio��������
 * @note   �����Ȼ�ȡ����Դ
 *         ֻ��SourSelΪGPIO����Ч
 */
uint32_t Power_WakeupGpioGet(PWR_SYSWAKEUP_SOURCE_SEL SourSel);

/**
 * @brief  ͨ������Դ��־��ѯgpio���ѵĴ�������
 * @param  SourSel:
 * @return ����Դ�Ĵ�������      0���½��أ� 1��������
 * @note   �����Ȼ�ȡ����Դ
 *         ֻ��SourSelΪGPIO��PowerKey���������ػ����½��ش���֮�֣�����SourSel����Ĭ�������ش���
 */
uint8_t Power_WakeupEdgeGet(PWR_SYSWAKEUP_SOURCE_SEL SourSel);

/**
 * @brief  ��������ֵ
 * @param  Mode = 1������ֵ280mA�� Mode = 0������ֵ60mA
 * @return ��
 */
void Power_CurrentLimitcConfig(uint32_t Mode);

/**
 * @brief  ��������ֵ
 * @param  PowerMode: 1�ߵ�ѹ���룬����3.6V; 0:����3.6V
 * @param  IsChipWork:
 * @return ��
 */
void Power_PowerModeConfig(uint32_t PowerMode, bool IsChipWork);

/**
 * @brief  ����Rtc32K����
 * @param  alarm:����ʱ�䣨��λ�룩
 * @param  start:ʱ�Ӽ�����ֵ����λ�룬һ��Ϊ0
 * @param  stopʱ�Ӽ�����ֵ����λ�룬һ��Ϊ0
 * @return ��
 */
void Power_AlarmSet(uint32_t alarm,uint32_t start,uint32_t stop);

/**
 * @brief  ����underpower��LDO��ѹ��Ҫ���ô˺���
 * @param  src:systerm core src base address0
 * @param  dest:systerm core dest base address0
 * @param  size:systerm core base address0 map size
 * @return ��
 */
void Power_FlashRemap2Sram(uint32_t src, uint32_t dest, uint32_t size);

/**
 * @brief  ����underpower��LDO��ѹ��Ҫ���ô˺���
 * @param  TRUE:����DeepSleep�ر�RCʱ��; FALSE:����DeepSleep����ر�RCʱ��
 * @return ��
 */
void Power_DeepSleepLDO12Config(bool IsAudoCutRcClk);

/**
 * @brief  ����LDO12��ѹ��Ҫ���ô˺���
 * @param  value:��λ��mV��
 *               ��Χ��1000mV~1300mV
 * @return ��
 * @note  оƬ����֮��������������ƫ�ƫ�Χ��0mV~9mV֮�䣻
 */
void Power_LDO12Config(uint32_t value);

/**
 * @brief  ��ȡLDO12��ѹʵ��ֵ
 * @param  ��
 * @return  * @param  value:��λ��mV��
 *               ��Χ��1000mV~1300mV
 */
int32_t Power_LDO12Get(void);

/**
 * @brief  ����LDO_33D��ѹ��Ҫ���ô˺���
 * @param  value:��λ��mV��
 *               ��Χ��2930mV~3350mV
 * @return ��
 * @note  оƬ����֮��������������ƫ�ƫ�Χ��0mV~40mV֮�䣻
 */
void Power_LDO33DConfig(uint32_t value);

/**
 * @brief  ����LDO_33A��ѹ��Ҫ���ô˺���
 * @param  value:��λ��mV��
 *               ��Χ��2950mV~3600mV
 * @return ��
 * @note  оƬ����֮��������������ƫ�ƫ�Χ��0mV~20mV֮�䣻
 * @note  33A��ѹ����֮���33D��ѹ��Ӱ��
 */
void Power_LDO33AConfig(uint32_t value);

/**
 * @brief  ʹ��LVD�͵�ѹ��⣨VIN��ѹ��
 * @param  ��
 * @return ��
 * @note   ��
 */
void Power_LVDEnable(void);

/**
 * @brief  ����LVD�͵�ѹ��⣨VIN��ѹ��
 * @param  ��
 * @return ��
 * @note   ��
 */
void Power_LVDDisable(void);

/**
 * @brief  Chọn ngưỡng phát hiện điện áp thấp LVD cho điện áp VIN
 *         (Select LVD low voltage threshold level for VIN)
 * @param  Lvd_Threshold_Sel 
 *         Giá trị lựa chọn mức ngưỡng [2.3~3.0], mặc định là 2.3V
 * @return NONE
 *         Không trả về giá trị
 * @note   NONE
 */
void Power_LVDThresholdSel(PWR_LVD_Threshold_SEL Lvd_Threshold_Sel);

/**
 * @brief  ʹ��DeepSleep����ʱ����LVD�͵�ѹ����ˮλ��VIN��ѹ��
 * @param  Lvd_Threshold_Sel  LVD�͵�ѹ����ˮλѡ��Ĭ��ˮλΪ2.3V��
 * @return ��
 * @note   ��
 */
void Power_LVDWakeupConfig(PWR_LVD_Threshold_SEL Lvd_Threshold_Sel);

/**
 * @brief  ʹ��DeepSleep����ʱ�����Ƿ���LPMģʽ
 * @param  IsLDO33A_LPM  TRUE:����LDO33A��LPMģʽ��FALSE:Ĭ��ֵ������LDO33A��HPMģʽ��
 * @param  IsLDO33D_LPM  TRUE:����LDO33D��LPMģʽ��FALSE:Ĭ��ֵ������LDO33D��HPMģʽ��
 * @param  IsLDO12_LPM   TRUE:����LDO12��LPMģʽ��FALSE:Ĭ��ֵ������LDO12��HPMģʽ��
 * @return ��
 * @note   ��
 */
void Power_PowerModeConfigTest(bool IsLDO33A_LPM,bool IsLDO33D_LPM, bool IsLDO12_LPM);

/**
 * @brief  ʹ��DeepSleep����ʱ����LDO12�ĵ�ѹ
 * @param  LDO1V2_HV_Val  LDO12���ߵ�ѹ������ڵĲ���(ֻ��1����λ����1����1V)
 * @param  LDO1V2_LV_Val  LDO12���͵�ѹ������ڵĲ���(0~15��16����λ����ѹ�������Խ��ͣ�ÿ��λ����40mV)
 * @param  LDO1V2_TRIM_Val  LDO12��ѹ������������仯��0~31�������Խ��ͣ�ÿ����λ3mV��32��ѹ���32~63�������Խ��ͣ�ÿ����λ3mV��
 * @return ��
 * @note   ��
 */
void Power_DeepSleepLDO12ConfigTest(uint8_t LDO1V2_LV_Val, uint8_t LDO1V2_HV_Val, uint8_t LDO1V2_TRIM_Val);

/**
 * @brief   Is HRC run during deepsleep?
 * @param   IsOpen  TRUE: Run HRC during deepsleep
 *                  FALSE: Not run HRC during deepsleep
 * @return TRUE:���óɹ�
 *         FALSE:����ʧ��
 * @note   ������ֵ��FALSE��˵��ǰ���Ѿ����õĻ���Դ������Ҫ�õ�HRC��Source,���Խ�ֹ�ر�
 */
bool Power_HRCCtrlByHwDuringDeepSleep(bool IsOpen);

/**
 * @brief  Close RF3V3D for DeepSleep
 * @param  ��
 * @return ��
 * @note   ��
 */
void RF_PowerDown(void);


#ifdef  __cplusplus
}
#endif//__cplusplus

#endif //__POWERCONTROLLER_H__

/**
 * @}
 * @}
 */
