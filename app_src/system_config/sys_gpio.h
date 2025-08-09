/*
 **************************************************************************************
 * @file    sys_gpio.h
 * @brief  
 * 
 * @author   
 * @version V1.0.0
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */ 


#ifndef __SYS_GPIO__
#define __SYS_GPIO__

#include "gpio.h"
#include "app_config.h"
#include "i2c_host.h"
//----- prohibit modify------------------------------------------------------//
#define  CONNECT(x, y, z)                                x ## y ## z
#define  CONVERT(x)                                      #x
#define  CONVERT_STRING(x)                               CONVERT(x)
#define  STRING_CONNECT(x, y , z)                        CONNECT(x, y, z)
#define  CONNECT2(x, y)                                  x ## y
#define  STRING_CONNECT2(x, y)                           CONNECT2(x, y)
//----- prohibit modify------------------------------------------------------//
//-----------register define-----------------------------------------------//
#define  GPIO                                           GPIO_
#define  GPIE                                           _IE
#define  GPOE                                           _OE
#define  GPPD                                           _PD
#define  GPPU                                           _PU
#define  GPOUT                                          _OUT
#define  GPIN                                           _IN
#define  GPPULLDOWN                                     _PULLDOWN
#define  PORT_INDEX                                     GPIO_INDEX
#define  GPINT                                           _INT
//-----------register define-----------------------------------------------//

#ifdef CFG_APP_LINEIN_MODE_EN
	 //#define CFG_LINEIN_DET_EN
    #ifdef	CFG_LINEIN_DET_EN
	#define LINEIN_DET_GPIO					GPIOA28
	#define LINEIN_DET_GPIO_IN 				GPIO_A_IN
	#define LINEIN_DET_BIT_MASK				GPIO_INDEX28
	#define LINEIN_DET_GPIO_IE 				GPIO_A_IE
	#define LINEIN_DET_GPIO_OE 				GPIO_A_OE
	#define LINEIN_DET_GPIO_PU 				GPIO_A_PU
	#define LINEIN_DET_GPIO_PD 				GPIO_A_PD
	#endif
#endif




#if (defined(CFG_APP_CARD_PLAY_MODE_EN) ) //|| defined(CFG_APP_USB_AUDIO_MODE_EN)
#define	CFG_RES_CARD_USE 
#ifdef CFG_RES_CARD_USE
#include "sd_card.h"
#ifdef CFG_CHIP_BP10128
#define	CFG_RES_CARD_GPIO				SDIO_A20_A21_A22 //ע�������ļ������޸ġ�
#else
#define	CFG_RES_CARD_GPIO				SDIO_A15_A16_A17 //ע�������ļ������޸ġ�
#endif
#define	CFG_FUNC_CARD_DETECT //���sd�����뵯������·���
#if CFG_RES_CARD_GPIO == SDIO_A15_A16_A17
	#define SDIO_Clk_Disable				SDIO_ClkDisable
	#define SDIO_Clk_Eable					SDIO_ClkEnable
	#define CARD_DETECT_GPIO				GPIOA16
	#define CARD_DETECT_GPIO_IN				GPIO_A_IN
	#define CARD_DETECT_BIT_MASK			GPIOA16
	#define CARD_DETECT_GPIO_IE				GPIO_A_IE
	#define CARD_DETECT_GPIO_OE				GPIO_A_OE
	#define CARD_DETECT_GPIO_PU				GPIO_A_PU
	#define CARD_DETECT_GPIO_PD				GPIO_A_PD
	#define CARD_DETECT_GPIO_OUT			GPIO_A_OUT
#elif CFG_RES_CARD_GPIO == SDIO_A20_A21_A22
	#define SDIO_Clk_Disable				SDIO_ClkDisable
	#define SDIO_Clk_Eable					SDIO_ClkEnable
	#define CARD_DETECT_GPIO				GPIOA21
	#define CARD_DETECT_GPIO_IN				GPIO_A_IN
	#define CARD_DETECT_BIT_MASK			GPIOA21
	#define CARD_DETECT_GPIO_IE				GPIO_A_IE
	#define CARD_DETECT_GPIO_OE				GPIO_A_OE
	#define CARD_DETECT_GPIO_PU				GPIO_A_PU
	#define CARD_DETECT_GPIO_PD				GPIO_A_PD
#endif
 
#endif //CFG_RES_CARD_USE
#endif

//#define CFG_FUNC_POWER_MONITOR_EN
#ifdef CFG_FUNC_POWER_MONITOR_EN
	//#define	 CFG_FUNC_OPTION_CHARGER_DETECT 	 //�򿪸ú궨�壬֧��GPIO������豸���빦��
	#ifdef CFG_FUNC_OPTION_CHARGER_DETECT
	//�����˿�����
	#define CHARGE_DETECT_PORT_PU			GPIO_A_PU
	#define CHARGE_DETECT_PORT_PD			GPIO_A_PD
	#define CHARGE_DETECT_PORT_IN			GPIO_A_IN
	#define CHARGE_DETECT_PORT_IE			GPIO_A_IE
	#define CHARGE_DETECT_PORT_OE			GPIO_A_OE
	#define CHARGE_DETECT_GPIO				GPIOA31
	#endif

	//#define BAT_VOL_DET_LRADC //�򿪸ú궨����ΪADC����ص���  �ر�ΪĬ�ϵ�LDOIN����ص���
	#ifdef BAT_VOL_DET_LRADC
    #define BAT_VOL_LRADC_CHANNEL_PORT		ADC_CHANNEL_GPIOA30
	#define BAT_VOL_LRADC_CHANNEL_ANA_EN	GPIO_A_ANA_EN
	#define BAT_VOL_LRADC_CHANNEL_ANA_MASK	GPIO_INDEX30
	#endif
#endif

/**ADC**/
#ifdef CFG_RES_ADC_KEY_USE
	#define CFG_RES_POWERKEY_ADC_EN		  
  	//#define CFG_RES_ADC_KEY_PORT_CH1		ADC_CHANNEL_GPIOA20_A23
	//#define CFG_RES_ADC_KEY_CH1_ANA_EN		GPIO_A_ANA_EN
	#define CFG_RES_ADC_KEY_CH1_ANA_MASK	GPIO_INDEX23
	#define CFG_PARA_WAKEUP_GPIO_ADCKEY		WAKEUP_GPIOA23 
	#ifdef CFG_CHIP_BP10128
	#define CFG_RES_ADC_KEY_PORT_CH2		ADC_CHANNEL_GPIOA26
	#endif
	#define CFG_RES_ADC_KEY_CH2_ANA_EN		GPIO_A_ANA_EN
	#define CFG_RES_ADC_KEY_CH2_ANA_MASK	GPIO_INDEX26
#endif //CFG_RES_ADC_KEY_USE

#if defined(CFG_RES_IR_KEY_SCAN) || defined(CFG_PARA_WAKEUP_SOURCE_IR)
	#define CFG_RES_IR_PIN					IR_GPIOB6	
#endif


#ifdef CFG_RES_CODE_KEY_USE
	#define CFG_CODE_KEY1P_BANK				'A'
	#define CFG_CODE_KEY1P_PIN				(8)
	#define CFG_CODE_KEY1N_BANK				'A'
	#define CFG_CODE_KEY1N_PIN				(9)
#endif


#ifdef CFG_RES_IO_KEY_SCAN
	#define  CFG_SOFT_POWER_KEY_EN                        
    #define  CFG_GPIO_KEY1_EN                              
    #define  CFG_GPIO_KEY2_EN                               
 
	#define CFG_PARA_WAKEUP_GPIO_IOKEY1		WAKEUP_GPIOA23 
	#define CFG_PARA_WAKEUP_GPIO_IOKEY2		WAKEUP_GPIOA26 
#endif


#ifdef CFG_ADC_LEVEL_KEY_EN
	#ifdef TEST
		#define  ADCLEVL_CHANNEL_MAP            (ADC_GPIOA24|ADC_GPIOA29|ADC_GPIOA28|ADC_GPIOA30)
		//#define  ADCLEVL_CHANNEL_MAP            (ADC_GPIOA20|ADC_GPIOA21|ADC_GPIOA22|ADC_GPIOA23)
	#else
		#define  ADCLEVL_CHANNEL_MAP            (ADC_GPIOA20|ADC_GPIOA21|ADC_GPIOA22|ADC_GPIOA23)

	#endif
#endif






#if (defined (CFG_APP_OPTICAL_MODE_EN)) || (defined (CFG_APP_COAXIAL_MODE_EN)) || (defined (CFG_FUNC_SPDIF_MIX_MODE))

#ifdef CFG_FUNC_SPDIF_MIX_MODE

	#define SPDIF_MIX_INDEX						GPIOA31
	#define SPDIF_MIX_PORT_MODE					8
	#define SPDIF_MIX_PORT_ANA_INPUT			SPDIF_ANA_INPUT_A31

#else
	#ifdef CFG_APP_OPTICAL_MODE_EN
		#define SPDIF_OPTICAL_PORT_ANA_INPUT	SPDIF_ANA_INPUT_A30
		//#define PORT_B_INPUT_DIGATAL
		#ifndef PORT_B_INPUT_DIGATAL
		#define SPDIF_OPTICAL_INDEX				GPIOA30
		#define SPDIF_OPTICAL_PORT_MODE			8
		#define SPDIF_OPTICAL_PORT_ANA_INPUT	SPDIF_ANA_INPUT_A30
		#else
		#define SPDIF_OPTICAL_INDEX				GPIOB1
		#define SPDIF_OPTICAL_PORT_MODE			1//(digital)
		//#define SPDIF_OPTICAL_PORT_ANA_INPUT	SPDIF_ANA_INPUT_A31
		#endif
	#endif
	
	#ifdef CFG_APP_COAXIAL_MODE_EN
		#define SPDIF_COAXIAL_INDEX				GPIOA31
		#define SPDIF_COAXIAL_PORT_MODE			8
		#define SPDIF_COAXIAL_PORT_ANA_INPUT	SPDIF_ANA_INPUT_A31
	#endif
#endif
#endif

#ifdef  CFG_APP_HDMIIN_MODE_EN
	/**ARC**/
	#define HDMI_ARC_RECV_IO_OE				GPIO_A_OE
	#define HDMI_ARC_RECV_IO_IE				GPIO_A_IE
	#define HDMI_ARC_RECV_IO_ANA			GPIO_A_ANA_EN
	#define HDMI_ARC_RECV_IO_PIN			GPIOA29
	#define HDMI_ARC_RECV_ANA_PIN           SPDIF_ANA_INPUT_A29

	/**CEC**/
	//timer3����ʹ��rcʱ�ӻ���ϵͳʱ�ӣ�
	//PWC���Ը�������GPIO�ڣ���PWMֻ��ָ��GPIO�ڿ����ã����Զ��߽ӿڲ����÷���һ����CEC����IO��������RECV_IO��RECV_IO_PIN
	#define HDMI_CEC_CLK_MODE				RC_CLK_MODE//SYSTEM_CLK_MODE//RC_CLK_MODE /*����cec���ѹ���ʱ��ֻ����rc*/
	#define HDMI_CEC_IO_TYPE				IO_TYPE_A
	#define HDMI_CEC_IO_INDEX				27
	//#define HDMI_CEC_RECV_IO	            28
	//#define HDMI_CEC_RECV_IO_PIN	    	28
	#define HDMI_CEC_SEND_IO	            TIMER3_PWM_A0_A8_A22_A27 /*dma table���*/
	#define HDMI_CEC_SEND_IO_PIN			(3)
    #define HDMI_CEC_RECV_DATA_ADDR	    	TIMER3_PWC_DATA_ADDR
    #define HDMI_CEC_SEND_DATA_ADDR         TIMER3_PWM_DUTY_ADDR

	#define CFG_PARA_WAKEUP_GPIO_CEC		WAKEUP_GPIOA27

	#define	TIMER3_PWC_DATA_ADDR			(0x4002C034)
	#define TIMER3_PWM_DUTY_ADDR			(0x4002C024)

	#define	TIMER4_PWC_DATA_ADDR			(0x4002C834)
	#define TIMER4_PWM_DUTY_ADDR			(0x4002C824)

	#define	TIMER5_PWC_DATA_ADDR			(0x4002F034)
	#define TIMER5_PWM_DUTY_ADDR			(0x4002F024)

	#define	TIMER6_PWC_DATA_ADDR			(0x4002F834)
	#define TIMER6_PWM_DUTY_ADDR			(0x4002F824)

	#define HDMI_CEC_RECV_TIMER_ID	        TIMER3
	#define HDMI_CEC_RECV_DMA_ID	        PERIPHERAL_ID_TIMER3
	#define HDMI_CEC_RECV_DMA_ADDR	        TIMER3_PWC_DATA_ADDR

	#define HDMI_CEC_SEND_TIMER_ID	        TIMER3
	#define HDMI_CEC_SEND_DMA_ID	        PERIPHERAL_ID_TIMER3
	#define HDMI_CEC_SEND_DMA_ADDR	        TIMER3_PWM_DUTY_ADDR

	#define HDMI_CEC_ARBITRATION_TIMER_ID   TIMER5
	#define HDMI_CEC_ARBITRATION_TIMER_IRQ  Timer5_IRQn
	#define HDMI_CEC_ARBITRATION_TIMER_FUNC Timer5Interrupt

	/**HPD**/
	#define HDMI_HPD_CHECK_DETECT_EN        //HDMI����Ȳ�����ѡ��

	#define HDMI_HPD_CHECK_STATUS_IO	   	GPIO_A_IN
	#define HDMI_HPD_CHECK_INI_IO		   	GPIO_A_INT
	#define HDMI_HPD_CHECK_STATUS_IO_PIN   	GPIOA24
	#define HDMI_HPD_ACTIVE_LEVEL			1  //�ߵ�ƽ��Ч

	/**DDC**/
	#define DDC_USE_SW_I2C
	#ifdef DDC_USE_SW_I2C
		#define DDCSclPortSel               PORT_B
		#define HdmiSclIndex                    5
		#define DDCSdaPortSel               PORT_B
		#define HdmiSdaIndex                    4
	#else
	   #define HDMI_DDC_DATA_IO_INIT()		GPIO_RegOneBitSet(GPIO_B_PU, GPIOB4),\
										    GPIO_RegOneBitClear(GPIO_B_PD, GPIOB4)
	   #define HDMI_DDC_CLK_IO_INIT()		GPIO_RegOneBitSet(GPIO_B_PU, GPIOB5),\
										    GPIO_RegOneBitClear(GPIO_B_PD, GPIOB5)
	   #define HDMI_DDC_IO_PIN				I2C_PORT_B4_B5
    #endif
#endif

#ifdef CFG_FUNC_DISPLAY_EN
//NOTE: seg_led_disp.h  include 7_LED PIN config
#ifdef DISP_DEV_SLED
	#define DISP_LED_INIT()  	    GPIO_PortAModeSet(GPIOA10, 0),\
									 GPIO_RegOneBitClear(GPIO_A_PU, GPIOA10),\
									 GPIO_RegOneBitClear(GPIO_A_PD, GPIOA10),\
									 GPIO_RegOneBitSet(GPIO_A_OE, GPIOA10),\
									 GPIO_RegOneBitClear(GPIO_A_IE, GPIOA10),\
									 GPIO_RegOneBitSet(GPIO_A_OUT, GPIOA10)
   #define DISP_LED_ON()            GPIO_RegOneBitSet(GPIO_A_OUT, GPIOA10)
   #define DISP_LED_OFF()           GPIO_RegOneBitClear(GPIO_A_OUT, GPIOA10)
#endif
#endif



#define POWER_CTL_PORT             B
#define POWER_CTL_PIN              0	//GPIO_INDEX31
	 
#ifdef	CFG_SOFT_POWER_KEY_EN
	//SOFT POWER KEY, GPIOA27��POWER KEY���
	#define POWER_KEY_PORT             A
	#define POWER_KEY_PIN              GPIO_INDEX27
#endif
	 //------------------------------------------------------------------------//
	 
	 //------GPIO KEY���IOѡ��------------------------------------------------//
#ifdef CFG_GPIO_KEY1_EN
#define GPIO_KEY1_PORT             A
#define GPIO_KEY1                  GPIO_INDEX23
#endif
	 
#ifdef CFG_GPIO_KEY2_EN
#define GPIO_KEY2_PORT             A
#define GPIO_KEY2                  GPIO_INDEX26
#endif
	 //------------------------------------------------------------------------//
	 
	 //------�������IOѡ��----------------------------------------------------//
#ifdef CFG_FUNC_DETECT_PHONE_EN
#define PHONE_DETECT_PORT          B
#define PHONE_DETECT_PIN           GPIO_INDEX27
#endif
	 //------------------------------------------------------------------------//
	 
	 //------MIC ���IOѡ��----------------------------------------------------//
#ifdef CFG_FUNC_DETECT_MIC_EN
#define MIC1_DETECT_PORT           B
#define MIC1_DETECT_PIN            GPIO_INDEX26
	 
#define MIC2_DETECT_PORT           B
#define MIC2_DETECT_PIN            GPIO_INDEX25
#endif
	 //------------------------------------------------------------------------//
	 
	 //------3��/4�߶������ͼ��IOѡ��----------------------------- -----------//
#ifdef CFG_FUNC_DETECT_MIC_SEG_EN
#define MIC_SEGMENT_PORT           B
#define MIC_SEGMENT_PIN            GPIO_INDEX24
#endif
	 //------------------------------------------------------------------------//
	 
	 //------mic mute ����IOѡ��-----------------------------------------------//
#define MIC_MUTE_CTL_PORT          B
#define MIC_MUTE_CTL_PIN           0//GPIO_INDEX23
	 //------------------------------------------------------------------------//
	 
	 //------�����IOѡ��----------------------------------------------------//
#if CFG_CHARGER_DETECT_EN
#define CHARGE_DETECT_PORT         B
#define CHARGE_DETECT_PIN          GPIO_INDEX22
#endif
	 //------------------------------------------------------------------------//
	 
	 //------MUTE��·�����MUTE����IOѡ��--------------------------------------//
#define MUTE_CTL_PORT              B
#define MUTE_CTL_PIN               0//GPIO_INDEX21
	 
	 //------------------------------------------------------------------------//
	 
	 //------LED����IOѡ��-----------------------------------------------------//
#if CFG_LED_EN
#define POWER_LED_PORT             B
#define POWER_LED_PIN              GPIO_INDEX20
	 
#define EAR_LED_PORT               B
#define EAR_LED_PIN                GPIO_INDEX19
	 
#define SONG_LED_PORT              B
#define SONG_LED_PIN               GPIO_INDEX18
	 
#define TALK_LED_PORT              B
#define TALK_LED_PIN               GPIO_INDEX17
	 
#define KTV_LED_PORT               B
#define KTV_LED_PIN                GPIO_INDEX16
	 
#define AUTO_TUNE_LED_PORT         B 
#define AUTO_TUNE_LED_PIN          GPIO_INDEX15
	 
#define SHUNNING_LED_PORT          B
#define SHUNNING_LED_PIN           GPIO_INDEX14
#endif
	 //------------------------------------------------------------------------//

 /*
 **************************************************************************************
 *	���²���Ҫ�޸ģ���
 *	
 **************************************************************************************
 */
 //------ϵͳ��Դ���ؿ���IO�Ĵ�������--------------------------------------------//
#define POWER_CTL_IE               STRING_CONNECT(GPIO,POWER_CTL_PORT,GPIE)         
#define POWER_CTL_OE               STRING_CONNECT(GPIO,POWER_CTL_PORT,GPOE) 
#define POWER_CTL_PU               STRING_CONNECT(GPIO,POWER_CTL_PORT,GPPU) 
#define POWER_CTL_PD               STRING_CONNECT(GPIO,POWER_CTL_PORT,GPPD)         
#define POWER_CTL_IN               STRING_CONNECT(GPIO,POWER_CTL_PORT,GPIN)         
#define POWER_CTL_OUT              STRING_CONNECT(GPIO,POWER_CTL_PORT,GPOUT) 
	 //------------------------------------------------------------------------------//
	 
	 //------�ⲿ�����ؼ��IO�Ĵ�������----------------------------------------------//
#ifdef	CFG_SOFT_POWER_KEY_EN
#define POWER_KEY_IE               STRING_CONNECT(GPIO,POWER_KEY_PORT,GPIE)         
#define POWER_KEY_OE               STRING_CONNECT(GPIO,POWER_KEY_PORT,GPOE) 
#define POWER_KEY_PU               STRING_CONNECT(GPIO,POWER_KEY_PORT,GPPU) 
#define POWER_KEY_PD               STRING_CONNECT(GPIO,POWER_KEY_PORT,GPPD)         
#define POWER_KEY_IN               STRING_CONNECT(GPIO,POWER_KEY_PORT,GPIN)         
#define POWER_KEY_OUT              STRING_CONNECT(GPIO,POWER_KEY_PORT,GPOUT) 
#endif
	 //------------------------------------------------------------------------------//
	 
	 //------GPIO KEY���IO�Ĵ�������------------------------------------------------// 
#ifdef CFG_GPIO_KEY1_EN
#define GPIO_KEY1_IE               STRING_CONNECT(GPIO,GPIO_KEY1_PORT,GPIE)         
#define GPIO_KEY1_OE               STRING_CONNECT(GPIO,GPIO_KEY1_PORT,GPOE) 
#define GPIO_KEY1_PU               STRING_CONNECT(GPIO,GPIO_KEY1_PORT,GPPU) 
#define GPIO_KEY1_PD               STRING_CONNECT(GPIO,GPIO_KEY1_PORT,GPPD)         
#define GPIO_KEY1_IN               STRING_CONNECT(GPIO,GPIO_KEY1_PORT,GPIN)         
#define GPIO_KEY1_OUT              STRING_CONNECT(GPIO,GPIO_KEY1_PORT,GPOUT) 
#endif
	 
#ifdef CFG_GPIO_KEY2_EN
#define GPIO_KEY2_IE               STRING_CONNECT(GPIO,GPIO_KEY2_PORT,GPIE)         
#define GPIO_KEY2_OE               STRING_CONNECT(GPIO,GPIO_KEY2_PORT,GPOE) 
#define GPIO_KEY2_PU               STRING_CONNECT(GPIO,GPIO_KEY2_PORT,GPPU) 
#define GPIO_KEY2_PD               STRING_CONNECT(GPIO,GPIO_KEY2_PORT,GPPD)         
#define GPIO_KEY2_IN               STRING_CONNECT(GPIO,GPIO_KEY2_PORT,GPIN)         
#define GPIO_KEY2_OUT              STRING_CONNECT(GPIO,GPIO_KEY2_PORT,GPOUT) 
#endif
	 //-------------------------------------------------------------------------------//
	 
	 //------�������IO�Ĵ�������-----------------------------------------------------//
#ifdef CFG_FUNC_DETECT_PHONE_EN
#define PHONE_DETECT_IE            STRING_CONNECT(GPIO,PHONE_DETECT_PORT,GPIE)         
#define PHONE_DETECT_OE            STRING_CONNECT(GPIO,PHONE_DETECT_PORT,GPOE) 
#define PHONE_DETECT_PU            STRING_CONNECT(GPIO,PHONE_DETECT_PORT,GPPU) 
#define PHONE_DETECT_PD            STRING_CONNECT(GPIO,PHONE_DETECT_PORT,GPPD)         
#define PHONE_DETECT_IN            STRING_CONNECT(GPIO,PHONE_DETECT_PORT,GPIN)         
#define PHONE_DETECT_OUT           STRING_CONNECT(GPIO,PHONE_DETECT_PORT,GPOUT) 
#endif
	 //-------------------------------------------------------------------------------//
	 
	 //------MIC���IO�Ĵ�������------------------------------------------------------//
#ifdef CFG_FUNC_DETECT_MIC_EN
#define MIC1_DETECT_IE              STRING_CONNECT(GPIO,MIC1_DETECT_PORT,GPIE)         
#define MIC1_DETECT_OE              STRING_CONNECT(GPIO,MIC1_DETECT_PORT,GPOE) 
#define MIC1_DETECT_PU              STRING_CONNECT(GPIO,MIC1_DETECT_PORT,GPPU) 
#define MIC1_DETECT_PD              STRING_CONNECT(GPIO,MIC1_DETECT_PORT,GPPD)         
#define MIC1_DETECT_IN              STRING_CONNECT(GPIO,MIC1_DETECT_PORT,GPIN)         
#define MIC1_DETECT_OUT             STRING_CONNECT(GPIO,MIC1_DETECT_PORT,GPOUT) 
	 
#define MIC2_DETECT_IE              STRING_CONNECT(GPIO,MIC2_DETECT_PORT,GPIE)         
#define MIC2_DETECT_OE              STRING_CONNECT(GPIO,MIC2_DETECT_PORT,GPOE) 
#define MIC2_DETECT_PU              STRING_CONNECT(GPIO,MIC2_DETECT_PORT,GPPU) 
#define MIC2_DETECT_PD              STRING_CONNECT(GPIO,MIC2_DETECT_PORT,GPPD)         
#define MIC2_DETECT_IN              STRING_CONNECT(GPIO,MIC2_DETECT_PORT,GPIN)         
#define MIC2_DETECT_OUT             STRING_CONNECT(GPIO,MIC2_DETECT_PORT,GPOUT) 
#endif
	 //-------------------------------------------------------------------------------//
	 
	 //------3��/4�߶������ͼ��IO�Ĵ�������------------------------------ -----------//
#ifdef CFG_FUNC_DETECT_MIC_SEG_EN
#define MIC_SEGMENT_IE             STRING_CONNECT(GPIO,MIC_SEGMENT_PORT,GPIE)         
#define MIC_SEGMENT_OE             STRING_CONNECT(GPIO,MIC_SEGMENT_PORT,GPOE) 
#define MIC_SEGMENT_PU             STRING_CONNECT(GPIO,MIC_SEGMENT_PORT,GPPU) 
#define MIC_SEGMENT_PD             STRING_CONNECT(GPIO,MIC_SEGMENT_PORT,GPPD)         
#define MIC_SEGMENT_IN             STRING_CONNECT(GPIO,MIC_SEGMENT_PORT,GPIN)         
#define MIC_SEGMENT_OUT            STRING_CONNECT(GPIO,MIC_SEGMENT_PORT,GPOUT) 
#endif
	 //-------------------------------------------------------------------------------//
	 
	 //------mic mute ����IO�Ĵ�������------------------------------------------------//
#define MIC_MUTE_CTL_IE            STRING_CONNECT(GPIO,MIC_MUTE_CTL_PORT,GPIE)         
#define MIC_MUTE_CTL_OE            STRING_CONNECT(GPIO,MIC_MUTE_CTL_PORT,GPOE) 
#define MIC_MUTE_CTL_PU            STRING_CONNECT(GPIO,MIC_MUTE_CTL_PORT,GPPU) 
#define MIC_MUTE_CTL_PD            STRING_CONNECT(GPIO,MIC_MUTE_CTL_PORT,GPPD)         
#define MIC_MUTE_CTL_IN            STRING_CONNECT(GPIO,MIC_MUTE_CTL_PORT,GPIN)         
#define MIC_MUTE_CTL_OUT           STRING_CONNECT(GPIO,MIC_MUTE_CTL_PORT,GPOUT) 
	 //-------------------------------------------------------------------------------//
	 
	 //------�������IO�Ĵ�������-----------------------------------------------------//
#if CFG_CHARGER_DETECT_EN
#define CHARGE_DETECT_IE           STRING_CONNECT(GPIO,CHARGE_DETECT_PORT,GPIE)         
#define CHARGE_DETECT_OE           STRING_CONNECT(GPIO,CHARGE_DETECT_PORT,GPOE) 
#define CHARGE_DETECT_PU           STRING_CONNECT(GPIO,CHARGE_DETECT_PORT,GPPU) 
#define CHARGE_DETECT_PD           STRING_CONNECT(GPIO,CHARGE_DETECT_PORT,GPPD)         
#define CHARGE_DETECT_IN           STRING_CONNECT(GPIO,CHARGE_DETECT_PORT,GPIN)         
#define CHARGE_DETECT_OUT          STRING_CONNECT(GPIO,CHARGE_DETECT_PORT,GPOUT) 
#endif
	 //-------------------------------------------------------------------------------//
	 
	 //------MUTE��·�����MUTE����IO�Ĵ�������---------------------------------------//
#define MUTE_CTL_IE               STRING_CONNECT(GPIO,MUTE_CTL_PORT,GPIE)         
#define MUTE_CTL_OE               STRING_CONNECT(GPIO,MUTE_CTL_PORT,GPOE) 
#define MUTE_CTL_PU               STRING_CONNECT(GPIO,MUTE_CTL_PORT,GPPU) 
#define MUTE_CTL_PD               STRING_CONNECT(GPIO,MUTE_CTL_PORT,GPPD)         
#define MUTE_CTL_IN               STRING_CONNECT(GPIO,MUTE_CTL_PORT,GPIN)         
#define MUTE_CTL_OUT              STRING_CONNECT(GPIO,MUTE_CTL_PORT,GPOUT) 
	 //-------------------------------------------------------------------------------//
	 
	 //------LED����IO�Ĵ�������------------------------------------------------------//
#if CFG_LED_EN
#define POWER_LED_IE              STRING_CONNECT(GPIO,POWER_LED_PORT,GPIE)         
#define POWER_LED_OE              STRING_CONNECT(GPIO,POWER_LED_PORT,GPOE) 
#define POWER_LED_PU              STRING_CONNECT(GPIO,POWER_LED_PORT,GPPU) 
#define POWER_LED_PD              STRING_CONNECT(GPIO,POWER_LED_PORT,GPPD)         
#define POWER_LED_IN              STRING_CONNECT(GPIO,POWER_LED_PORT,GPIN)         
#define POWER_LED_OUT             STRING_CONNECT(GPIO,POWER_LED_PORT,GPOUT) 
	 
#define EAR_LED_IE                STRING_CONNECT(GPIO,EAR_LED_PORT,GPIE)         
#define EAR_LED_OE                STRING_CONNECT(GPIO,EAR_LED_PORT,GPOE) 
#define EAR_LED_PU                STRING_CONNECT(GPIO,EAR_LED_PORT,GPPU) 
#define EAR_LED_PD                STRING_CONNECT(GPIO,EAR_LED_PORT,GPPD)         
#define EAR_LED_IN                STRING_CONNECT(GPIO,EAR_LED_PORT,GPIN)         
#define EAR_LED_OUT               STRING_CONNECT(GPIO,EAR_LED_PORT,GPOUT) 
	 
#define SONG_LED_IE               STRING_CONNECT(GPIO,SONG_LED_PORT,GPIE)         
#define SONG_LED_OE               STRING_CONNECT(GPIO,SONG_LED_PORT,GPOE) 
#define SONG_LED_PU               STRING_CONNECT(GPIO,SONG_LED_PORT,GPPU) 
#define SONG_LED_PD               STRING_CONNECT(GPIO,SONG_LED_PORT,GPPD)         
#define SONG_LED_IN               STRING_CONNECT(GPIO,SONG_LED_PORT,GPIN)         
#define SONG_LED_OUT              STRING_CONNECT(GPIO,SONG_LED_PORT,GPOUT) 
	 
#define TALK_LED_IE               STRING_CONNECT(GPIO,TALK_LED_PORT,GPIE)         
#define TALK_LED_OE               STRING_CONNECT(GPIO,TALK_LED_PORT,GPOE) 
#define TALK_LED_PU               STRING_CONNECT(GPIO,TALK_LED_PORT,GPPU) 
#define TALK_LED_PD               STRING_CONNECT(GPIO,TALK_LED_PORT,GPPD)         
#define TALK_LED_IN               STRING_CONNECT(GPIO,TALK_LED_PORT,GPIN)         
#define TALK_LED_OUT              STRING_CONNECT(GPIO,TALK_LED_PORT,GPOUT) 
	 
#define KTV_LED_IE               STRING_CONNECT(GPIO,KTV_LED_PORT,GPIE)         
#define KTV_LED_OE               STRING_CONNECT(GPIO,KTV_LED_PORT,GPOE) 
#define KTV_LED_PU               STRING_CONNECT(GPIO,KTV_LED_PORT,GPPU) 
#define KTV_LED_PD               STRING_CONNECT(GPIO,KTV_LED_PORT,GPPD)         
#define KTV_LED_IN               STRING_CONNECT(GPIO,KTV_LED_PORT,GPIN)         
#define KTV_LED_OUT              STRING_CONNECT(GPIO,KTV_LED_PORT,GPOUT) 
	 
#define AUTO_TUNE_LED_IE         STRING_CONNECT(GPIO,AUTO_TUNE_LED_PORT,GPIE)         
#define AUTO_TUNE_LED_OE         STRING_CONNECT(GPIO,AUTO_TUNE_LED_PORT,GPOE) 
#define AUTO_TUNE_LED_PU         STRING_CONNECT(GPIO,AUTO_TUNE_LED_PORT,GPPU) 
#define AUTO_TUNE_LED_PD         STRING_CONNECT(GPIO,AUTO_TUNE_LED_PORT,GPPD)         
#define AUTO_TUNE_LED_IN         STRING_CONNECT(GPIO,AUTO_TUNE_LED_PORT,GPIN)         
#define AUTO_TUNE_LED_OUT        STRING_CONNECT(GPIO,AUTO_TUNE_LED_PORT,GPOUT) 
	 
#define SHUNNING_LED_IE         STRING_CONNECT(GPIO,SHUNNING_LED_PORT,GPIE)         
#define SHUNNING_LED_OE         STRING_CONNECT(GPIO,SHUNNING_LED_PORT,GPOE) 
#define SHUNNING_LED_PU         STRING_CONNECT(GPIO,SHUNNING_LED_PORT,GPPU) 
#define SHUNNING_LED_PD         STRING_CONNECT(GPIO,SHUNNING_LED_PORT,GPPD)         
#define SHUNNING_LED_IN         STRING_CONNECT(GPIO,SHUNNING_LED_PORT,GPIN)         
#define SHUNNING_LED_OUT        STRING_CONNECT(GPIO,SHUNNING_LED_PORT,GPOUT) 
#endif
	 //-------------------------------------------------------------------------------//
	 
	 
	 /*
	 **************************************************************************************
	 *	GPIOӦ�õ��õĺ궨��
	 *	
		ע��1.����������Ҫʱ�ɶ���յĺ�;
			2.������Ҫ����ʵ�ʵİ��ӣ�������ȷ�Ŀ��Ƹߵ��߼���ƽ
	 **************************************************************************************
	 */
	 
	 //------LED������غ궨��--------------------------------------------------------//
	 
#if CFG_LED_EN
#define EAR_LED_OFF()	  do{\
								  GPIO_RegOneBitClear(EAR_LED_IE, EAR_LED_PIN);\
								  GPIO_RegOneBitSet(EAR_LED_OE, EAR_LED_PIN);\
								  GPIO_RegOneBitClear(EAR_LED_OUT, EAR_LED_PIN);\
								  }while(0)
								  
#define EAR_LED_ON()	   do{\
								  GPIO_RegOneBitClear(EAR_LED_IE, EAR_LED_PIN);\
								  GPIO_RegOneBitSet(EAR_LED_OE, EAR_LED_PIN);\
								  GPIO_RegOneBitSet(EAR_LED_OUT, EAR_LED_PIN);\
								  }while(0)
								  
#define SONG_LED_OFF()	  do{\
								 GPIO_RegOneBitClear(SONG_LED_IE, SONG_LED_PIN);\
								 GPIO_RegOneBitSet(SONG_LED_OE, SONG_LED_PIN);\
								 GPIO_RegOneBitSet(SONG_LED_OUT, SONG_LED_PIN);\
								 }while(0)
									  
#define SONG_LED_ON()	   do{\
								  GPIO_RegOneBitClear(SONG_LED_IE, SONG_LED_PIN);\
								  GPIO_RegOneBitSet(SONG_LED_OE, SONG_LED_PIN);\
								  GPIO_RegOneBitClear(SONG_LED_OUT, SONG_LED_PIN);\
								  }while(0)
								  
#define TALK_LED_OFF()	  do{\
								  GPIO_RegOneBitClear(TALK_LED_IE, TALK_LED_PIN);\
								  GPIO_RegOneBitSet(TALK_LED_OE, TALK_LED_PIN);\
								  GPIO_RegOneBitSet(TALK_LED_OUT, TALK_LED_PIN);\
								  }while(0)
								  
#define TALK_LED_ON()	   do{\
								  GPIO_RegOneBitClear(TALK_LED_IE, TALK_LED_PIN);\
								  GPIO_RegOneBitSet(TALK_LED_OE, TALK_LED_PIN);\
								  GPIO_RegOneBitClear(TALK_LED_OUT, TALK_LED_PIN);\
								  }while(0) 
								  
#define KTV_LED_OFF()	  do{\
								 GPIO_RegOneBitClear(KTV_LED_IE, KTV_LED_PIN);\
								 GPIO_RegOneBitSet(KTV_LED_OE, KTV_LED_PIN);\
								 GPIO_RegOneBitSet(KTV_LED_OUT, KTV_LED_PIN);\
								 }while(0)
									  
#define KTV_LED_ON()	   do{\
								 GPIO_RegOneBitClear(KTV_LED_IE, KTV_LED_PIN);\
								 GPIO_RegOneBitSet(KTV_LED_OE, KTV_LED_PIN);\
								 GPIO_RegOneBitClear(KTV_LED_OUT, KTV_LED_PIN);\
								 }while(0)
							   
#define AUTO_LED_OFF()	  do{\
								  GPIO_RegOneBitClear(AUTO_TUNE_LED_IE, AUTO_TUNE_LED_PIN);\
								  GPIO_RegOneBitSet(AUTO_TUNE_LED_OE, AUTO_TUNE_LED_PIN);\
								  GPIO_RegOneBitSet(AUTO_TUNE_LED_OUT, AUTO_TUNE_LED_PIN);\
								  }while(0)
								  
#define AUTO_LED_ON()	   do{\
								  GPIO_RegOneBitClear(AUTO_TUNE_LED_IE, AUTO_TUNE_LED_PIN);\
								  GPIO_RegOneBitSet(AUTO_TUNE_LED_OE, AUTO_TUNE_LED_PIN);\
								  GPIO_RegOneBitClear(AUTO_TUNE_LED_OUT, AUTO_TUNE_LED_PIN);\
								  }while(0)    
								  
#define SHUNNING_LED_OFF()	do{\
								 GPIO_RegOneBitClear(SHUNNING_LED_IE, SHUNNING_LED_PIN);\
								 GPIO_RegOneBitSet(SHUNNING_LED_OE, SHUNNING_LED_PIN);\
								 GPIO_RegOneBitSet(SHUNNING_LED_OUT, SHUNNING_LED_PIN);\
								 }while(0)
									  
#define SHUNNING_LED_ON()  do{\
								 GPIO_RegOneBitClear(SHUNNING_LED_IE, SHUNNING_LED_PIN);\
								 GPIO_RegOneBitSet(SHUNNING_LED_OE, SHUNNING_LED_PIN);\
								 GPIO_RegOneBitClear(SHUNNING_LED_OUT, SHUNNING_LED_PIN);\
								 }while(0)
	 
#define POWER_LED_ON()     do{\
							  GPIO_RegOneBitClear(POWER_LED_IE, POWER_LED_PIN);\
							  GPIO_RegOneBitSet(POWER_LED_OE, POWER_LED_PIN);\
							  GPIO_RegOneBitClear(POWER_LED_OUT, POWER_LED_PIN);\
							  }while(0)
#define POWER_LED_OFF()     do{\
							  GPIO_RegOneBitClear(POWER_LED_IE, POWER_LED_PIN);\
							  GPIO_RegOneBitSet(POWER_LED_OE, POWER_LED_PIN);\
							  GPIO_RegOneBitSet(POWER_LED_OUT, POWER_LED_PIN);\
							  }while(0)
	 
#endif
	 //-------------------------------------------------------------------------------//
	 
	 //------�ⲿMUTE��·����ſ�����غ궨��-----------------------------------------//
#define MUTE_OFF()	  do{\
							  GPIO_RegOneBitClear(MUTE_CTL_IE, MUTE_CTL_PIN);\
							  GPIO_RegOneBitSet(MUTE_CTL_OE, MUTE_CTL_PIN);\
							  GPIO_RegOneBitSet(MUTE_CTL_OUT, MUTE_CTL_PIN);\
							  }while(0)
							  
#define MUTE_ON()	   do{\
							  GPIO_RegOneBitClear(MUTE_CTL_IE, MUTE_CTL_PIN);\
							  GPIO_RegOneBitSet(MUTE_CTL_OE, MUTE_CTL_PIN);\
							  GPIO_RegOneBitClear(MUTE_CTL_OUT, MUTE_CTL_PIN);\
							  }while(0)
	 //-------------------------------------------------------------------------------//
	 
	 
	 //------MIC MUTE������غ궨��---------------------------------------------------//
#define MIC_MUTE_ON()	  do{\
							  GPIO_RegOneBitClear(MIC_MUTE_CTL_IE, MIC_MUTE_CTL_PIN);\
							  GPIO_RegOneBitSet(MIC_MUTE_CTL_OE, MIC_MUTE_CTL_PIN);\
							  GPIO_RegOneBitSet(MIC_MUTE_CTL_OUT, MIC_MUTE_CTL_PIN);\
							  }while(0)
							  
#define MIC_MUTE_OFF()	   do{\
							  GPIO_RegOneBitClear(MIC_MUTE_CTL_IE, MIC_MUTE_CTL_PIN);\
							  GPIO_RegOneBitSet(MIC_MUTE_CTL_OE, MIC_MUTE_CTL_PIN);\
							  GPIO_RegOneBitClear(MIC_MUTE_CTL_OUT, MIC_MUTE_CTL_PIN);\
							  }while(0) 				  
	 //-------------------------------------------------------------------------------//
	 
	 
	 //------�ⲿ�ܵ�Դ������غ궨��-------------------------------------------------//
#define POWER_ON()     do{\
							  GPIO_RegOneBitClear(POWER_CTL_IE, POWER_CTL_PIN);\
							  GPIO_RegOneBitSet(POWER_CTL_OE, POWER_CTL_PIN);\
							  GPIO_RegOneBitSet(POWER_CTL_OUT, POWER_CTL_PIN);\
							  }while(0);
#define POWER_OFF()     do{\
							  GPIO_RegOneBitClear(POWER_CTL_IE, POWER_CTL_PIN);\
							  GPIO_RegOneBitSet(POWER_CTL_OE, POWER_CTL_PIN);\
							  GPIO_RegOneBitClear(POWER_CTL_OUT, POWER_CTL_PIN);\
							  }while(0);
	 //-------------------------------------------------------------------------------//
	 
#endif //end of __USER_HW_INTERFACE__

#if defined(CFG_FUNC_SHELL_EN) && CFG_UART_TX_PORT == 2 && defined(CFG_APP_HDMIIN_MODE_EN) && HDMI_HPD_CHECK_STATUS_IO_PIN == GPIOA24
#error Conflict: Uart Rx Pin  X  Hdmi HPD Pin!
#endif
