#include <string.h>
#include "type.h"
#include "irqn.h"
#include "gpio.h"
#include "dma.h"
#include "rtos_api.h"
#include "app_config.h"
#include "app_message.h"
#include "debug.h"
#include "delay.h"
#include "audio_adc.h"
#include "dac.h"
#include "adc_interface.h"
#include "dac_interface.h"
#include "audio_core_api.h"
#include "audio_core_service.h"
#include "decoder.h"
#include "remind_sound.h"
#include "main_task.h"
#include "audio_effect.h"
#include "powercontroller.h"
#include "deepsleep.h"
#include "backup_interface.h"
#include "breakpoint.h"
#include "otg_device_hcd.h"
#include "otg_device_standard_request.h"
#include "audio_vol.h"
#include "ctrlvars.h"
#include "main_task.h"
#include "reset.h"
#include "backup.h"
#include "mode_task_api.h"
#include "adc.h"
#include "adc_key.h"
#include "ir_key.h"
#include "otg_detect.h"
#include "sadc_interface.h"
#include "device_detect.h"
#include "watchdog.h"
#include "bb_api.h"
#include "bt_stack_service.h"
#include "bt_manager.h"
#include "rtc.h"


#define MAIN_APP_TASK_SLEEP_PRIO		6 //����deepsleep ��Ҫ�������task������ȼ���

#ifdef CFG_APP_IDLE_MODE_EN

#define IDLE_NOT_REQUIRED_MODE	(BIT(ModeIdle) | BIT(ModeTwsSlavePlay) | BIT(ModeBtHfPlay))	

#ifdef CFG_FUNC_REMIND_SOUND_EN
#define CFG_FUNC_REMIND_DEEPSLEEP		//Deepsleep ǰ����(sys)��ʾ��
#define CFG_FUNC_REMIND_WAKEUP			//Deepsleep �����Ժ󲥷���ʾ��
#endif

extern bool ModeCommonInit(void);
extern bool SystemPowerKeyIdleModeInit(uint32_t Mode, uint16_t CountTime);
extern bool SystemPowerKeyIdleModeWakeUpDetect(void);

#if defined(CFG_APP_IDLE_MODE_EN)&&defined(CFG_FUNC_REMIND_SOUND_EN)
extern volatile uint32_t gIdleRemindSoundTimeOutTimer;
#endif

#define POWER_ON_IDLE				0
#define NEED_POWER_ON				1
#define WAIT_POWER_ON_REMIND_SOUND	2
#define ENTER_POWER_ON				3

static struct
{
	uint16_t	AutoPowerOnState 	:	2;

#if	defined(CFG_IDLE_MODE_POWER_KEY)
	uint16_t	PowerKeyWakeUpCheckFlag 	: 	1; //�ϵ�/powerkey���Ѽ��
#endif
#ifdef  CFG_FUNC_REMIND_SOUND_EN
	uint16_t	RemindSoundFlag 	:	1;
	uint16_t	DeepSleepFlag 	:	1;
#endif		
	uint16_t	FristPowerOnFlag 	:	1;

	SysModeNumber	SavePrevMode;
}IdleMode;


#ifdef	CFG_IDLE_MODE_POWER_KEY

#define USE_POWERKEY_MSG_SP 	MSG_POWERDOWN

#if	POWERKEY_MODE == POWERKEY_MODE_PUSH_BUTTON
	#define 	POWER_KEY_JITTER_TIME		100			//����ʱ�䣬��ʱ��������ؿ��ػ�Ӳ��ʱ���й�
	#define 	POWER_KEY_CP_TIME			1000

	typedef enum _POWER_KEY_STATE
	{
		POWER_KEY_STATE_IDLE,
		POWER_KEY_STATE_JITTER,
		POWER_KEY_STATE_PRESS_DOWN,
		POWER_KEY_STATE_CP,
	} POWER_KEY_STATE;


	TIMER			PowerKeyWaitTimer;
	POWER_KEY_STATE	PowerKeyState = POWER_KEY_STATE_IDLE;

	void PowerPushButtonKeyInit(void)
	{
		PowerKeyState = POWER_KEY_STATE_IDLE;
		TimeOutSet(&PowerKeyWaitTimer, 0);
	}
	
	uint16_t GetPowerPushButtonKey(void)
	{
		uint16_t Msg = MSG_NONE;
		
		switch(PowerKeyState)
		{
			case POWER_KEY_STATE_IDLE:
				//powerkey���������Ժ� ��Ҫ�ȴ�powerkey�����ͷ�
				if(IdleMode.PowerKeyWakeUpCheckFlag)
				{
					if(BACKUP_PowerKeyPinStateGet())
					{	
						IdleMode.PowerKeyWakeUpCheckFlag = FALSE;
					}				
					break;
				}
				if(!BACKUP_PowerKeyPinStateGet())
				{	
					TimeOutSet(&PowerKeyWaitTimer, POWER_KEY_JITTER_TIME);
					PowerKeyState = POWER_KEY_STATE_JITTER;
				}
				break;
			case POWER_KEY_STATE_JITTER:
				if(BACKUP_PowerKeyPinStateGet())
				{
					PowerKeyState = POWER_KEY_STATE_IDLE;
				}
				else if(IsTimeOut(&PowerKeyWaitTimer))
				{
					PowerKeyState = POWER_KEY_STATE_PRESS_DOWN;
					TimeOutSet(&PowerKeyWaitTimer, POWER_KEY_CP_TIME);
				}
				break;
				
			case POWER_KEY_STATE_PRESS_DOWN:
				if(BACKUP_PowerKeyPinStateGet())
				{
					PowerKeyState = POWER_KEY_STATE_IDLE;
					#ifdef USE_POWERKEY_PUSH_BUTTON_MSG_SP
						Msg = USE_POWERKEY_PUSH_BUTTON_MSG_SP;
					#else
						Msg = USE_POWERKEY_MSG_SP;
					#endif
				}
				else if(IsTimeOut(&PowerKeyWaitTimer))
				{
					PowerKeyState = POWER_KEY_STATE_CP;
					Msg = USE_POWERKEY_MSG_SP;
				}
				break;
				
			case POWER_KEY_STATE_CP:
				//�˴�����֤һ�ΰ���������Ӧ��ζ̰�
				if(BACKUP_PowerKeyPinStateGet())
				{
					PowerKeyState = POWER_KEY_STATE_IDLE;
				}
				else
				{
					//do no thing
				}
				break;
				
			default:
				PowerKeyState = POWER_KEY_STATE_IDLE;
				break;
		}
		return Msg;
	}
#else
	#define POWER_KEY_JITTER_CNT		50
	#define POWER_KEY_WAIT_RELEASE		0xffff
	static uint32_t CntTimer = POWER_KEY_JITTER_CNT;

	void PowerKeySlideSwitchInit(void)
	{
		CntTimer = POWER_KEY_JITTER_CNT;
	}
	
	uint16_t GetPowerKeySlideSwitch(void)
	{
		uint16_t Msg = MSG_NONE;
		
		if(SystemPowerKeyDetect())
		{
			if(CntTimer > 0 && CntTimer != POWER_KEY_WAIT_RELEASE)
				CntTimer--;
			if(CntTimer == 0)
			{
				CntTimer = POWER_KEY_WAIT_RELEASE;
				Msg = USE_POWERKEY_MSG_SP;
			}
		}
		else
		{
			CntTimer = POWER_KEY_JITTER_CNT;
		}
		return Msg;
	}
#endif
#endif

#ifdef CFG_IDLE_MODE_DEEP_SLEEP
	void DeepSleepKeyInit(void)
	{

	}

	uint16_t GetDeepSleepKey(void)
	{
		return MSG_NONE;
	}
#endif

void EnterIdleModeScanInit(void)
{
#ifdef	CFG_IDLE_MODE_POWER_KEY
#if	POWERKEY_MODE == POWERKEY_MODE_PUSH_BUTTON
		PowerPushButtonKeyInit();
#else
		PowerKeySlideSwitchInit();
#endif
#endif
	
#ifdef CFG_IDLE_MODE_DEEP_SLEEP
		DeepSleepKeyInit();
#endif
}

uint16_t GetEnterIdleModeScanKey(void)
{
	uint16_t Msg = MSG_NONE;
#ifdef	CFG_IDLE_MODE_POWER_KEY
#if	POWERKEY_MODE == POWERKEY_MODE_PUSH_BUTTON
	Msg = GetPowerPushButtonKey();
#else
	Msg = GetPowerKeySlideSwitch();	
#endif
#endif
	
#ifdef CFG_IDLE_MODE_DEEP_SLEEP
	if(Msg == MSG_NONE)
	{
		Msg = GetDeepSleepKey();
	}
#endif

	return Msg;
}

void PowerOnRemindSound(void)
{
#ifdef  CFG_FUNC_REMIND_SOUND_EN
	RemindSoundClearPlay();
#if !defined(CFG_FUNC_REMIND_WAKEUP) && defined(CFG_IDLE_MODE_DEEP_SLEEP)
	if(IdleMode.DeepSleepFlag)
	{
		IdleMode.DeepSleepFlag = FALSE;
		return; 		
	}
#endif

#if (TWS_PAIRING_MODE == CFG_TWS_ROLE_MASTER) && defined(CFG_TWS_SOUNDBAR_APP)
	if(btManager.twsState == BT_TWS_STATE_CONNECTED)
	{
		vTaskDelay(1000); //��ʱ�ȴӻ�Ҳ��������һ����ʾ��
	}
#endif

	{
		IdleMode.RemindSoundFlag = RemindSoundServiceItemRequest(SOUND_REMIND_KAIJI, REMIND_ATTR_NEED_CLEAR_INTTERRUPT_PLAY|REMIND_PRIO_SYS);
		IdleMode.DeepSleepFlag = FALSE;
		gIdleRemindSoundTimeOutTimer = 0;
	}


#endif//CFG_FUNC_REMIND_SOUND_EN
}

void PowerDownRemindSound(void)
{
#ifdef  CFG_FUNC_REMIND_SOUND_EN

#if !defined(CFG_FUNC_REMIND_DEEPSLEEP) && defined(CFG_IDLE_MODE_DEEP_SLEEP)
	if(SoftFlagGet(SoftFlagIdleModeEnterSleep))
	{
		return;			
	}
#endif	
	IdleMode.RemindSoundFlag = RemindSoundServiceItemRequest(SOUND_REMIND_GUANJI, REMIND_ATTR_NEED_CLEAR_INTTERRUPT_PLAY|REMIND_PRIO_SYS);
	gIdleRemindSoundTimeOutTimer = 0;
	RemindSoundItemRequestDisable();
#endif
}

bool GetPowerRemindSoundPlayEnd(void)
{
#ifdef  CFG_FUNC_REMIND_SOUND_EN
	if(IdleMode.RemindSoundFlag)
	{
		if(!RemindSoundIsPlay() || gIdleRemindSoundTimeOutTimer > 2000)
			IdleMode.RemindSoundFlag = FALSE;
		return TRUE;
	}
#endif
	return FALSE;
}
#ifdef	CFG_IDLE_MODE_POWER_KEY
void PowerKeyModeInit(void)
{
	RTC_IntDisable();//Ĭ�Ϲر�RTC�ж�
	RTC_IntFlagClear();
#ifdef CFG_CHIP_BP1064L2
	Backup_Clock(BACKUP_CLK_32K_OSC);
#else
	Backup_Clock(BACKUP_CLK_32K_RC32);//BACKUPĬ��ʹ��Ƭ��RC32K;RTC������Ҫ�߾���RTC(�ⲿ24M����),���õ�power key���ܣ���rtc���ܲ�����ʹ��!!!!
#endif

	while(!BACKUP_IsOscClkToggle());	//wait backup clk ready.

#if	POWERKEY_MODE != POWERKEY_MODE_PUSH_BUTTON
	IdleMode.PowerKeyWakeUpCheckFlag = SystemPowerKeyIdleModeInit(POWERKEY_MODE, POWERKEY_CNT);
#else
	IdleMode.PowerKeyWakeUpCheckFlag = SystemPowerKeyIdleModeInit(POWERKEY_MODE, 200);
#endif
	PowerKeyModeGet();
}
#endif

void IdleModeConfig(void)
{
#ifdef	CFG_IDLE_MODE_POWER_KEY
	PowerKeyModeInit();
#endif

	EnterIdleModeScanInit();

	IdleMode.AutoPowerOnState	= NEED_POWER_ON;
	
#ifdef  CFG_FUNC_REMIND_SOUND_EN
	IdleMode.RemindSoundFlag 	= FALSE;
	IdleMode.DeepSleepFlag = FALSE;
#endif
	IdleMode.FristPowerOnFlag = TRUE;
}

bool IdleModeInit(void)
{
	if(!ModeCommonInit())
	{
		return FALSE;
	}

	//Core Process
	//AudioCoreProcessConfig((void*)AudioNoAppProcess);
#ifdef CFG_FUNC_AUDIO_EFFECT_EN
	AudioCoreProcessConfig((void*)AudioMusicProcess);
#else
	AudioCoreProcessConfig((void*)AudioBypassProcess);
#endif

	DBG("Idle Mode Init\n");
#if (defined(CFG_APP_BT_MODE_EN) && (BT_HFP_SUPPORT == ENABLE))	
	if((GetA2dpState(BtCurIndex_Get()) >= BT_A2DP_STATE_CONNECTED)
		|| (GetHfpState(BtCurIndex_Get()) >= BT_HFP_STATE_CONNECTED)
		|| (GetAvrcpState(BtCurIndex_Get()) >= BT_AVRCP_STATE_CONNECTED))
	{
		//�ֶ��Ͽ�
		BtStackServiceMsgSend(MSG_BTSTACK_MSG_BT_DISCONNECT_DEV_CTRL);
	}
#endif	

#ifdef  CFG_FUNC_REMIND_SOUND_EN	
	if(!IdleMode.FristPowerOnFlag)
	{
		PowerDownRemindSound();
	}
#endif	
	DBG("Idle Mode run\n");
#ifdef CFG_APP_BT_MODE_EN
	if((!IdleMode.FristPowerOnFlag) && sys_parameter.Bt_BackgroundType == BT_BACKGROUND_POWER_ON)
		BtFastPowerOff();
#endif
	IdleMode.FristPowerOnFlag = FALSE;
	return TRUE;
}


bool IdleModeDeinit(void)
{
	DBG("Idle Mode Deinit\n");
	if(IsAudioPlayerMute() == FALSE)
	{
		HardWareMuteOrUnMute();
	}	
	
	PauseAuidoCore();
	//ע�⣺AudioCore�����������mainApp�£��˴�ֻ�ر�AudioCoreͨ�������ر�����
	AudioCoreProcessConfig((void*)AudioNoAppProcess);
	ModeCommonDeinit();//ͨ·ȫ���ͷ�
#ifdef	CFG_IDLE_MODE_POWER_KEY
	SoftFlagDeregister(SoftFlagIdleModeEnterPowerDown);
#endif				
#ifdef CFG_IDLE_MODE_DEEP_SLEEP
	SoftFlagDeregister(SoftFlagIdleModeEnterSleep);
#endif	

#ifdef CFG_APP_BT_MODE_EN
	if(sys_parameter.Bt_BackgroundType != BT_BACKGROUND_DISABLE)
	{
		BtStackServiceMsgSend(MSG_BTSTACK_RUN_START);
		if(sys_parameter.Bt_BackgroundType == BT_BACKGROUND_POWER_ON)
			BtFastPowerOn();
	}
#endif
#ifdef BT_TWS_SUPPORT
	AudioCoreSourceUnmute(TWS_SOURCE_NUM, TRUE, TRUE);
#endif
	return TRUE;
}

void SendEnterIdleModeMsg(void)
{
	MessageContext		msgSend;
	
	msgSend.msgId = MSG_ENTER_IDLE_MODE;
	MessageSend(GetMainMessageHandle(), &msgSend);
}

void SendQuitIdleModeMsg(void)
{
	MessageContext		msgSend;

	msgSend.msgId = MSG_QUIT_IDLE_MODE;
	MessageSend(GetMainMessageHandle(), &msgSend);	
}


void IdleModeRun(uint16_t msgId)
{
#if	defined(CFG_IDLE_MODE_POWER_KEY) && (POWERKEY_MODE != POWERKEY_MODE_PUSH_BUTTON)
	if(IdleMode.PowerKeyWakeUpCheckFlag)
	{	
		PowerKeyModeInit();
		SystemPowerKeyIdleModeWakeUpDetect();
		IdleMode.PowerKeyWakeUpCheckFlag = FALSE;
	}
#endif	
	
#ifdef CFG_IDLE_MODE_POWER_KEY
	if(SoftFlagGet(SoftFlagIdleModeEnterPowerDown)
#ifdef	CFG_FUNC_REMIND_SOUND_EN
	 && (!GetPowerRemindSoundPlayEnd())
#endif
	 )
	{
	#if (POWERKEY_MODE == POWERKEY_MODE_PUSH_BUTTON)
		if(TRUE == BACKUP_PowerKeyPinStateGet()) 
		{
			SoftFlagDeregister(SoftFlagIdleModeEnterPowerDown);
			PowerKeyModeInit();
			ADC_PowerkeyChannelDisable();
			BACKUP_SystemPowerDown();
			while(1);
		}			
	#else
		SoftFlagDeregister(SoftFlagIdleModeEnterPowerDown);
		if(TRUE == SystemPowerKeyDetect())
		{
			PowerKeyModeInit();
			BACKUP_SystemPowerDown();
			while(1);
		}
		else
		{
			//������powerkey���� ֱ�ӿ���
			IdleMode.AutoPowerOnState = NEED_POWER_ON;
		}
	#endif				
	}
#endif	

#ifdef CFG_IDLE_MODE_DEEP_SLEEP
	if(SoftFlagGet(SoftFlagIdleModeEnterSleep)
#ifdef	CFG_FUNC_REMIND_SOUND_EN
		&& (!GetPowerRemindSoundPlayEnd())
#endif
	)
	{

		SoftFlagDeregister(SoftFlagIdleModeEnterSleep);
	#ifdef  CFG_FUNC_REMIND_SOUND_EN
		IdleMode.DeepSleepFlag = TRUE;
	#endif	
		PauseAuidoCore();
		UBaseType_t pri = uxTaskPriorityGet(NULL);
#ifdef CFG_APP_BT_MODE_EN
		if(sys_parameter.Bt_BackgroundType != BT_BACKGROUND_DISABLE
#ifdef BT_SNIFF_ENABLE
				&& !SoftFlagGet(SoftFlagIdleModeEnterSniff)
#endif
			)
		{
			vTaskDelay(50);
			//bb reset
			RF_PowerDownBySw();
			WDG_Feed();
			rwip_reset();
			BT_IntDisable();
			WDG_Feed();
			//Kill bt stack service
			BtStackServiceKill();
			WDG_Feed();
			vTaskDelay(10);
			RF_PowerDownByHw();
			BT_ModuleClose();
		}
#endif		

		vTaskPrioritySet(NULL, MAIN_APP_TASK_SLEEP_PRIO);//�趨������ȼ�
		
 		NVIC_DisableIRQ(Timer2_IRQn);
	
#ifdef BT_SNIFF_ENABLE
 		if(SoftFlagGet(SoftFlagIdleModeEnterSniff))
 		{
 			DeepSleeping_BT();
 		}
 		else
#endif
 		{
 			DeepSleeping();
 		}

	 	Timer_Config(TIMER2,1000,0);
	 	Timer_Start(TIMER2);
	 	Timer_InterruptFlagClear(TIMER2, UPDATE_INTERRUPT_SRC);
		NVIC_EnableIRQ(Timer2_IRQn);		

#ifdef CFG_APP_BT_MODE_EN
		if(sys_parameter.Bt_BackgroundType != BT_BACKGROUND_DISABLE
#ifdef BT_SNIFF_ENABLE
				&& !SoftFlagGet(SoftFlagIdleModeEnterSniff)
#endif
			)
		{
			#ifdef BT_TWS_SUPPORT
			if (btManager.twsState == BT_TWS_STATE_CONNECTED ){
				tws_link_state_set(BT_TWS_STATE_DISCONNECT);
			}
			#endif
			WDG_Feed();
			BtStackServiceStart();
			WDG_Feed();
		}
#endif
#ifdef CFG_RES_IR_KEY_SCAN
		IRKeyInit();//�������İ���
#endif

//#ifdef CFG_RES_AUDIO_I2SOUT_EN
//		AudioI2sOutParamsSet();
//#endif

//#ifdef CFG_FUNC_I2S_MIX_MODE
//		AudioI2s0ParamsSet();
//		AudioI2s1ParamsSet();
//#endif

#ifdef CFG_RES_AUDIO_SPDIFOUT_EN		 
		AudioSpdifOutParamsSet();
#endif

		vTaskPrioritySet(NULL, pri);
		AudioCoreServiceResume();
		osTaskDelay(10);// for printf 
	
#ifdef BT_SNIFF_ENABLE
		SoftFlagDeregister(SoftFlagIdleModeEnterSniff);
#endif

		IdleMode.AutoPowerOnState = NEED_POWER_ON;
	}
#endif	

	switch(IdleMode.AutoPowerOnState)
	{
		case NEED_POWER_ON:
			WDG_Feed();
			PowerOnRemindSound();
			WDG_Feed();

			IdleMode.AutoPowerOnState = WAIT_POWER_ON_REMIND_SOUND;
			break;
		case WAIT_POWER_ON_REMIND_SOUND:
			if(!GetPowerRemindSoundPlayEnd())
				IdleMode.AutoPowerOnState = ENTER_POWER_ON;	
			break;
		case ENTER_POWER_ON:
			IdleMode.AutoPowerOnState = POWER_ON_IDLE;
#if (TWS_PAIRING_MODE == CFG_TWS_ROLE_MASTER) && defined(CFG_TWS_SOUNDBAR_APP)
			if(btManager.twsState == BT_TWS_STATE_CONNECTED)
			{
				WDG_Feed();
				vTaskDelay(1000); //��ʱ�ȴӻ�Ҳ��������һ����ʾ��
				tws_sync_reinit();
				WDG_Feed();
			}
#endif
			SendQuitIdleModeMsg();
			break;
		case POWER_ON_IDLE:
			break;
		default:
			IdleMode.AutoPowerOnState = POWER_ON_IDLE;
			break;
	}

	switch(msgId)
	{
		case MSG_POWER:
		case MSG_POWERDOWN:
		case MSG_DEEPSLEEP:
		case MSG_BT_SNIFF:
#ifdef CFG_IDLE_MODE_POWER_KEY
			if(SoftFlagGet(SoftFlagIdleModeEnterPowerDown)
#ifdef	CFG_FUNC_REMIND_SOUND_EN
			 && (!GetPowerRemindSoundPlayEnd())
#endif
			 )
				break;
#endif
			if(IdleMode.AutoPowerOnState == POWER_ON_IDLE && (!GetPowerRemindSoundPlayEnd()))
				IdleMode.AutoPowerOnState = NEED_POWER_ON;
			break;
		default:
			CommonMsgProccess(msgId);
			break;
	}
}


void IdlePrevModeSet(SysModeNumber mode)
{
	IdleMode.SavePrevMode = mode;
}

extern osMutexId SysModeMutex;
void IdleModeEnter(void)
{
	if(GetSysModeState(ModeIdle) == ModeStateInit || GetSysModeState(ModeIdle) == ModeStateRunning )
		return;
	osMutexLock(SysModeMutex);
	if(IDLE_NOT_REQUIRED_MODE & BIT(mainAppCt.SysCurrentMode))
		IdleMode.SavePrevMode = mainAppCt.SysPrevMode;
	else
		IdleMode.SavePrevMode = mainAppCt.SysCurrentMode;
	if(GetSysModeState(ModeIdle) == ModeStateSusend)
	{
		SetSysModeState(ModeIdle,ModeStateReady);
	}
	SysModeEnter(ModeIdle);
	osMutexUnlock(SysModeMutex);
	APP_DBG("enter idle mode\n");
}

void IdleModeExit(void)
{
#if (defined(BT_TWS_SUPPORT) && (TWS_PAIRING_MODE == CFG_TWS_ROLE_SLAVE))
		if(mainAppCt.SysPrevMode == ModeTwsSlavePlay)
		{
			if(btManager.twsState == BT_TWS_STATE_CONNECTED)
			{
				IdleMode.SavePrevMode = mainAppCt.SysPrevMode;
			}
			else
			{
				IdleMode.SavePrevMode = ModeBtAudioPlay;
			}
		}
		printf("CurrMode %d,PrevMode %d,  dd %d\n",mainAppCt.SysCurrentMode,mainAppCt.SysPrevMode,IdleMode.SavePrevMode);
#else
		if(IDLE_NOT_REQUIRED_MODE & BIT(IdleMode.SavePrevMode))
				IdleMode.SavePrevMode = ModeBtAudioPlay;
#endif

	osMutexLock(SysModeMutex);
	SysModeEnter(IdleMode.SavePrevMode);
	osMutexUnlock(SysModeMutex);
	APP_DBG("exit idle mode\n");

#ifdef BT_SNIFF_ENABLE
	if(sniff_wakeup_get())
	{
		sniff_wakeup_set(0);
	}
#endif

}

#else

void IdlePrevModeSet(SysModeNumber mode)
{
	mode = mode;
}

void IdleModeEnter(void)
{

}

void IdleModeExit(void)
{

}

#endif


