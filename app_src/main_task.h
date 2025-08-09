/**
 **************************************************************************************
 * @file    main_task.h
 * @brief   Program Entry 
 *
 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2016-6-29 13:06:47$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#ifndef __MAIN_TASK_H__
#define __MAIN_TASK_H__


#include "type.h"
#include "rtos_api.h"
#include "app_config.h"
#include "audio_core_api.h"
#include "app_message.h"
#include "timeout.h"
#include "mode_task.h"
#include "flash_boot.h"
#ifdef CFG_FUNC_DISPLAY_EN
#include "display.h"
#endif

#define SoftFlagMask			0xFFFFFFFF

#ifdef CFG_RES_IR_NUMBERKEY
extern bool Number_select_flag;
extern uint16_t Number_value;
extern TIMER Number_selectTimer;
#endif

typedef struct _SysVolContext
{
	bool		MuteFlag;	
	int8_t	 	AudioSourceVol[AUDIO_CORE_SOURCE_MAX_NUM];
	int8_t	 	AudioSourceVol32[AUDIO_CORE_SOURCE_MAX_NUM];
	int8_t	 	AudioSinkVol[AUDIO_CORE_SINK_MAX_NUM];		
	
}SysVolContext;




typedef struct _MainAppContext
{
	xTaskHandle			taskHandle;
	MessageHandle		msgHandle;
	TaskState			state;

	SysModeNumber		SysCurrentMode;
	SysModeNumber		SysPrevMode;

/*************************mode common*************************************/
#ifdef CFG_RES_AUDIO_DAC0_EN
	uint32_t			*DACFIFO;
	uint32_t			DACFIFO_LEN;
#endif

#ifdef CFG_RES_AUDIO_DACX_EN
	uint32_t			*DACXFIFO;
	uint32_t			DACXFIFO_LEN;
#endif

#ifdef CFG_RES_AUDIO_I2SOUT_EN
	uint32_t			*I2SFIFO;
	uint32_t			I2SFIFO_LEN;
#endif

#ifdef CFG_RES_AUDIO_I2S0OUT_EN
	uint32_t			*I2S0_TX_FIFO;
    uint32_t			I2S0_TX_FIFO_LEN;
#endif

#ifdef CFG_RES_AUDIO_I2S1OUT_EN
	uint32_t			*I2S1_TX_FIFO;
	uint32_t			I2S1_TX_FIFO_LEN;
#endif

#ifdef CFG_RES_AUDIO_I2S0IN_EN
	uint32_t			*I2S0_RX_FIFO;
	uint32_t			I2S0_RX_FIFO_LEN;
#endif

#ifdef CFG_RES_AUDIO_I2S1IN_EN
	uint32_t			*I2S1_RX_FIFO;
	uint32_t			I2S1_RX_FIFO_LEN;
#endif

	uint32_t			*ADCFIFO;

/******************************************************************/
	AudioCoreContext 	*AudioCore;
	uint16_t			SourcesMuteState;
	uint32_t 			SampleRate;
	bool				AudioCoreSync;
	SysVolContext		gSysVol;



    uint8_t     	MusicVolume;
	uint8_t     	MusicVolume_VR;

	#ifdef CFG_APP_BT_MODE_EN
		uint8_t     HfVolume;
	#endif

	uint8_t     	EffectMode;
    uint8_t     	MicVolume;
	uint8_t     	MicVolumeBak;

	#ifdef CFG_FUNC_MUSIC_EQ_MODE_EN
		uint8_t     EqMode;
		#ifdef CFG_FUNC_EQMODE_FADIN_FADOUT_EN   
			uint8_t     EqModeBak;
			uint8_t     EqModeFadeIn;
			uint8_t     eqSwitchFlag;
		#endif
	#endif



	uint8_t			MicEffectDelayStep;
	uint8_t			MicEffectDelayStepBak;

	#ifdef CFG_FUNC_MUSIC_TREB_BASS_EN	
		uint8_t 	MusicBassStep;
		uint8_t 	MusicMidStep;
		uint8_t 	MusicTrebStep;
	#endif

	#ifdef CFG_FUNC_SILENCE_AUTO_POWER_OFF_EN
		uint32_t    Silence_Power_Off_Time;
	#endif

	bool        	tws_device_init_flag;
}MainAppContext;







extern MainAppContext	mainAppCt;
#define SoftFlagNoRemind				BIT(0)	//��ʾ������
#define SoftFlagMediaDevicePlutOut		BIT(1)
#if FLASH_BOOT_EN 
#define SoftFlagMvaInCard				BIT(2)	//�ļ�Ԥ��������SD����MVA�� ���γ�ʱ����
#define SoftFlagMvaInUDisk				BIT(3)	//�ļ�Ԥ��������U����Mva�� U�̰γ�ʱ����
#endif
#define SoftFlagDiscDelayMask			BIT(4)//ͨ��ģʽ,�����Ͽ�����,��ʱ������ʾ��,���˻ص�ÿ��ģʽʱ����
#define SoftFlagWaitBtRemindEnd			BIT(5)//�������ʱ�ȴ���ʾ����������ٽ���ͨ��״̬
#define SoftFlagDelayEnterBtHf			BIT(6)//�����ʱ����ͨ��״̬
#define SoftFlagFrameSizeChange			BIT(7)//ּ�ڵǼ�ϵͳ֡�л�������һ״̬�������ϡ�
#define SoftFlagBtCurPlayStateMask		BIT(8)//�������ʱ��¼��ǰ�������ŵ�״̬
#ifdef BT_TWS_SUPPORT
#define SoftFlagTwsRemind				BIT(9)//���tws���ӳɹ��¼� �ȴ�unmute����ʾ������
#define SoftFlagTwsSlaveRemind			BIT(10)
#endif
#ifdef CFG_FUNC_BT_OTA_EN
#define SoftFlagBtOtaUpgradeOK			BIT(11)
#endif
#ifdef CFG_APP_IDLE_MODE_EN
#ifdef CFG_IDLE_MODE_DEEP_SLEEP
#define SoftFlagIdleModeEnterSleep		BIT(12)//��ǽ���˯��ģʽ
#endif
#ifdef	CFG_IDLE_MODE_POWER_KEY
#define SoftFlagIdleModeEnterPowerDown	BIT(13)
#endif
#endif
#define SoftFlagMediaModeRead			BIT(14)// ����media mode ��һ��U��SD
#define SoftFlagMediaNextOrPrev			BIT(15)// 0:next 1:prev
#define SoftFlagUpgradeOK				BIT(16)

#define SoftFlagAudioCoreSourceIsDeInit		BIT(18)	//AudioCoreSource��Դ�Ѿ���ɾ��

#define SoftFlagUDiskEnum				BIT(19)	//u��ö�ٱ�־
#define SoftFlagRecording				BIT(20)	//¼�����б�ǣ���ֹ����Ȳ���ģʽ�л�������

//��Ǳ���deepsleep��Ϣ�Ƿ�������TV
#define SoftFlagDeepSleepMsgIsFromTV 	BIT(21)

//��Ǳ��λ���Դ�Ƿ�ΪCEC����
#define SoftFlagWakeUpSouceIsCEC 		BIT(22)

//����AVRCP MEDIA INFO
#define SoftFlagBtMediInfo 				BIT(23)	//��ȡ��������ID3��Ϣ��,����ϵͳӦ�ý������ݵ����

#ifdef BT_SNIFF_ENABLE
#define SoftFlagIdleModeEnterSniff		BIT(24)//��ǽ���sniffģʽ
#endif

void SoftFlagRegister(uint32_t SoftEvent);
void SoftFlagDeregister(uint32_t SoftEvent);
bool SoftFlagGet(uint32_t SoftEvent);
int32_t MainAppTaskStart(void);
MessageHandle GetMainMessageHandle(void);
uint32_t GetSystemMode(void);

uint32_t IsBtHfMode(void);

uint32_t IsBtAudioMode(void);

uint32_t IsBtTwsSlaveMode(void);
uint32_t IsIdleModeReady(void);
void PowerOffMessage(void);
void BatteryLowMessage(void);

//uint8_t Get_Resampler_Polyphase(int32_t resampler);
#endif /*__MAIN_TASK_H__*/

