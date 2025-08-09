#include "app_config.h"
#include "powercontroller.h"
#include "deepsleep.h"
#include "gpio.h"
#include "timeout.h"
#include "audio_adc.h"
#include "dac.h"
#include "clk.h"
#include "chip_info.h"
#include "otg_device_hcd.h"
#include "rtc.h"
#include "irqn.h"
#include "debug.h"
#include "adc_key.h"
#include "delay.h"
#include "FreeRTOS.h"
#include "task.h"
#include "adc_key.h"
#include "uarts.h"
//#include "OrioleReg.h"//for test
#include "sys.h"
#include "sadc_interface.h"
#include "watchdog.h"
#include "backup.h"
#include "ir_key.h"
#include "app_message.h"
#include "reset.h"
#include "bt_stack_service.h"
#include "key.h"
#include "main_task.h"
#include "efuse.h"
#include "bt_common_api.h"
#include "bt_manager.h"
#include "bt_app_sniff.h"
#include "hdmi_in_api.h"
#include "adc.h"
#include "mode_task_api.h"
#include "i2s.h"

#ifdef CFG_IDLE_MODE_DEEP_SLEEP
HDMIInfo  			 *gHdmiCt;
void GIE_ENABLE(void);
void SystemOscConfig(void);
void SleepMainAppTask(void);
void SleepAgainConfig(void);
void WakeupMain(void);
void SleepMain(void);
void LogUartConfig(bool InitBandRate);

#define CHECK_SCAN_TIME				5000		//����ȷ����Ч����Դ ɨ����ʱms��
TIMER   waitCECTime;
uint8_t	waitCECTimeFlag = 0;

#define DPLL_QUICK_START_SLOPE	(*(volatile unsigned long *) 0x40026028)
//DPLL ��������������ȡ��
void Clock_GetDPll(uint32_t* NDAC, uint32_t* OS, uint32_t* K1, uint32_t* FC);

static uint32_t sources;
#ifdef CFG_PARA_WAKEUP_SOURCE_RTC
uint32_t alarm = 0;
#endif

extern uint8_t uart_switch;

__attribute__((section(".driver.isr")))void WakeupInterrupt(void)
{
	sources |= Power_WakeupSourceGet();

	Power_WakeupSourceClear();
}


void SystermGPIOWakeupConfig(PWR_SYSWAKEUP_SOURCE_SEL source,PWR_WAKEUP_GPIO_SEL gpio,PWR_SYSWAKEUP_SOURCE_EDGE_SEL edge)
{
	if(gpio < 32)
	{
		GPIO_RegOneBitSet(GPIO_A_IE,   (1 << gpio));
		GPIO_RegOneBitClear(GPIO_A_OE, (1 << gpio));
		if( edge == SYSWAKEUP_SOURCE_NEGE_TRIG )
		{
			GPIO_RegOneBitSet(GPIO_A_PU, (1 << gpio));//��ΪоƬ��GPIO���ڲ�����������,ѡ���½��ش���ʱҪ��ָ����GPIO���ѹܽ�����Ϊ����
			GPIO_RegOneBitClear(GPIO_A_PD, (1 << gpio));
		}
		else if(edge == SYSWAKEUP_SOURCE_POSE_TRIG )
		{
			GPIO_RegOneBitClear(GPIO_A_PU, (1 << gpio));//��ΪоƬ��GPIO���ڲ����������裬����ѡ�������ش���ʱҪ��ָ����GPIO���ѹܽ�����Ϊ����
			GPIO_RegOneBitSet(GPIO_A_PD, (1 << gpio));
		}
	}
	else if(gpio < 41)
	{
		GPIO_RegOneBitSet(GPIO_B_IE,   (1 << (gpio - 32)));
		GPIO_RegOneBitClear(GPIO_B_OE, (1 << (gpio - 32)));
		if( edge == SYSWAKEUP_SOURCE_NEGE_TRIG )
		{
			GPIO_RegOneBitSet(GPIO_B_PU, (1 << (gpio - 32)));//��ΪоƬ��GPIO���ڲ�����������,ѡ���½��ش���ʱҪ��ָ����GPIO���ѹܽ�����Ϊ����
			GPIO_RegOneBitClear(GPIO_B_PD, (1 << (gpio - 32)));
		}
		else if( edge == SYSWAKEUP_SOURCE_POSE_TRIG )
		{
			GPIO_RegOneBitClear(GPIO_B_PU, (1 << (gpio - 32)));//��ΪоƬ��GPIO���ڲ����������裬����ѡ�������ش���ʱҪ��ָ����GPIO���ѹܽ�����Ϊ����
			GPIO_RegOneBitSet(GPIO_B_PD, (1 << (gpio - 32)));
		}
	}
	else if(gpio == 41)
	{

		BACKUP_WriteEnable();

		BACKUP_C0RegSet(BKUP_GPIO_C0_REG_IE_OFF, TRUE);
		BACKUP_C0RegSet(BKUP_GPIO_C0_REG_OE_OFF, FALSE);
		if( edge == SYSWAKEUP_SOURCE_NEGE_TRIG )
		{
			BACKUP_C0RegSet(BKUP_GPIO_C0_REG_PU_OFF, TRUE);
			BACKUP_C0RegSet(BKUP_GPIO_C0_REG_PD_OFF, FALSE);
		}
		else if( edge == SYSWAKEUP_SOURCE_POSE_TRIG )
		{
			BACKUP_C0RegSet(BKUP_GPIO_C0_REG_PU_OFF, FALSE);//��ΪоƬ��GPIO���ڲ����������裬����ѡ�������ش���ʱҪ��ָ����GPIO���ѹܽ�����Ϊ����
			BACKUP_C0RegSet(BKUP_GPIO_C0_REG_PD_OFF, TRUE);
		}
		BACKUP_WriteDisable();
	}

	Power_WakeupSourceClear();
	Power_WakeupSourceSet(source, gpio, edge);
	Power_WakeupEnable(source);

	NVIC_EnableIRQ(Wakeup_IRQn);
	NVIC_SetPriority(Wakeup_IRQn, 0);
	GIE_ENABLE();
}

void SystermIRWakeupConfig(IR_MODE_SEL ModeSel, IR_IO_SEL GpioSel, IR_CMD_LEN_SEL CMDLenSel)
{

	Clock_BTDMClkSelect(RC_CLK32_MODE);//sniff����ʱʹ��Ӱ������������
	Reset_FunctionReset(IR_FUNC_SEPA);
	IRKeyInit();
	IR_WakeupEnable();


	if(GpioSel == IR_GPIOB6)
	{
		GPIO_RegOneBitSet(GPIO_B_IE,   GPIO_INDEX6);
		GPIO_RegOneBitClear(GPIO_B_OE, GPIO_INDEX6);
		GPIO_RegOneBitSet(GPIO_B_IN,   GPIO_INDEX6);
		GPIO_RegOneBitClear(GPIO_B_OUT, GPIO_INDEX6);
		GPIO_RegOneBitClear(GPIO_B_PD, GPIO_INDEX6);
	}
	else if(GpioSel == IR_GPIOB7)
	{
		GPIO_RegOneBitSet(GPIO_B_IE,   GPIO_INDEX7);
		GPIO_RegOneBitClear(GPIO_B_OE, GPIO_INDEX7);
		GPIO_RegOneBitSet(GPIO_B_IN,   GPIO_INDEX7);
		GPIO_RegOneBitClear(GPIO_B_OUT, GPIO_INDEX7);
		GPIO_RegOneBitClear(GPIO_B_PD, GPIO_INDEX7);
	}
	else
	{
		GPIO_RegOneBitSet(GPIO_A_IE,   GPIO_INDEX29);
		GPIO_RegOneBitClear(GPIO_A_OE, GPIO_INDEX29);
		GPIO_RegOneBitSet(GPIO_A_IN,   GPIO_INDEX29);
		GPIO_RegOneBitClear(GPIO_A_OUT, GPIO_INDEX29);
		GPIO_RegOneBitClear(GPIO_A_PD, GPIO_INDEX29);
	}
	NVIC_EnableIRQ(Wakeup_IRQn);
	NVIC_SetPriority(Wakeup_IRQn, 0);
	GIE_ENABLE();

	Power_WakeupSourceClear();
	Power_WakeupSourceSet(SYSWAKEUP_SOURCE9_IR, 0, 0);
	Power_WakeupEnable(SYSWAKEUP_SOURCE9_IR);
}

void SystermIRWakeupConfig_sniff(IR_MODE_SEL ModeSel, IR_IO_SEL GpioSel, IR_CMD_LEN_SEL CMDLenSel)
{

	Reset_FunctionReset(IR_FUNC_SEPA);
	IRKeyInit();
	IR_WakeupEnable();


	if(GpioSel == IR_GPIOB6)
	{
		GPIO_RegOneBitSet(GPIO_B_IE,   GPIO_INDEX6);
		GPIO_RegOneBitClear(GPIO_B_OE, GPIO_INDEX6);
		GPIO_RegOneBitSet(GPIO_B_IN,   GPIO_INDEX6);
		GPIO_RegOneBitClear(GPIO_B_OUT, GPIO_INDEX6);
		GPIO_RegOneBitClear(GPIO_B_PD, GPIO_INDEX6);
	}
	else if(GpioSel == IR_GPIOB7)
	{
		GPIO_RegOneBitSet(GPIO_B_IE,   GPIO_INDEX7);
		GPIO_RegOneBitClear(GPIO_B_OE, GPIO_INDEX7);
		GPIO_RegOneBitSet(GPIO_B_IN,   GPIO_INDEX7);
		GPIO_RegOneBitClear(GPIO_B_OUT, GPIO_INDEX7);
		GPIO_RegOneBitClear(GPIO_B_PD, GPIO_INDEX7);
	}
	else
	{
		GPIO_RegOneBitSet(GPIO_A_IE,   GPIO_INDEX29);
		GPIO_RegOneBitClear(GPIO_A_OE, GPIO_INDEX29);
		GPIO_RegOneBitSet(GPIO_A_IN,   GPIO_INDEX29);
		GPIO_RegOneBitClear(GPIO_A_OUT, GPIO_INDEX29);
		GPIO_RegOneBitClear(GPIO_A_PD, GPIO_INDEX29);
	}
	NVIC_EnableIRQ(Wakeup_IRQn);
	NVIC_SetPriority(Wakeup_IRQn, 0);
	GIE_ENABLE();

	Power_WakeupSourceClear();
	Power_WakeupSourceSet(SYSWAKEUP_SOURCE9_IR, 0, 0);
	Power_WakeupEnable(SYSWAKEUP_SOURCE9_IR);
}



#ifdef CFG_PARA_WAKEUP_SOURCE_RTC
//RTC���� ���������RTC�ж�
void SystermRTCWakeupConfig(uint32_t SleepSecond)
{
	//RTC_REG_TIME_UNIT start;
	RTC_ClockSrcSel(OSC_24M);
	RTC_IntDisable();
	RTC_IntFlagClear();
	RTC_WakeupDisable();

	alarm = RTC_SecGet() + SleepSecond;
	RTC_SecAlarmSet(alarm);
	RTC_WakeupEnable();
	RTC_IntEnable();

	NVIC_EnableIRQ(Wakeup_IRQn);
	NVIC_SetPriority(Wakeup_IRQn, 0);
	NVIC_EnableIRQ(Rtc_IRQn);
	NVIC_SetPriority(Rtc_IRQn, 1);
	GIE_ENABLE();

	Power_WakeupSourceClear();
	Power_WakeupSourceSet(SYSWAKEUP_SOURCE7_RTC, 0, 0);
	Power_WakeupEnable(SYSWAKEUP_SOURCE7_RTC);
}
#endif //CFG_PARA_WAKEUP_RTC

void DeepSleepIOConfig()
{
	//cansle all IO AF
	{
		int IO_cnt = 0;
		for(IO_cnt = 0;IO_cnt < 32;IO_cnt++)
		{
			if(IO_cnt == 30)
				continue;//A30��sw��
			if(IO_cnt == 31)
				continue;//A31��sw��
			if(IO_cnt == 23)
				continue;//A23�ǻ���Դ

			GPIO_PortAModeSet(GPIOA0 << IO_cnt,0);
		}
		for(IO_cnt = 0;IO_cnt < 8;IO_cnt++)
		{
			if(IO_cnt == 0)
				continue;//B1��sw��
			if(IO_cnt == 1)
				continue;//B0��sw��

			GPIO_PortBModeSet(GPIOB0 << IO_cnt,0);
		}
	}

	SleepAgainConfig();//������ͬ
}

void SystermDeepSleepConfig(void)
{
	uint32_t IO_cnt;

	for(IO_cnt = 0;IO_cnt < 32;IO_cnt++)
	{
#ifdef CFG_PARA_WAKEUP_GPIO_CEC
		if(IO_cnt == CFG_PARA_WAKEUP_GPIO_CEC)
			continue;
#endif
		GPIO_PortAModeSet(GPIOA0 << IO_cnt, 0);
	}

	for(IO_cnt = 0;IO_cnt < 8;IO_cnt++)
	{
		if ((IO_cnt != 0) && (IO_cnt != 1))
		{
			GPIO_PortBModeSet(GPIOB0 << IO_cnt, 0);
		}
	}

	SleepAgainConfig();//������ͬ

	SleepMain();

#if defined(CFG_PARA_WAKEUP_SOURCE_CEC) && defined(CFG_PARA_WAKEUP_GPIO_CEC)
	if(gHdmiCt == NULL)
	{
		HDMI_CEC_DDC_Init();
		gHdmiCt->hdmiReportStatus = 0;
	}
//	osTaskDelay(10);
	while(!HDMI_CEC_IsReadytoDeepSleep(10));//����10ms�����Ϻ���18msû��ͨ�ź��ж���Ӧ��
#endif
#ifdef CFG_PARA_WAKEUP_SOURCE_RTC
	alarm = 0;
#else

	//Clock_LOSCDisable(); //����RTCӦ���򲻹ر�32K���� BKD mark sleep
//	BACKUP_32KDisable(OSC32K_SOURCE);// bkd add

#endif
}

//���ö������Դʱ��sourceͨ���������������ظ���
void WakeupSourceSet(void)
{
#ifdef CFG_PARA_WAKEUP_SOURCE_RTC
	SystermRTCWakeupConfig(CFG_PARA_WAKEUP_TIME_RTC);
#endif	
#if defined(CFG_PARA_WAKEUP_SOURCE_ADCKEY)
	SystermGPIOWakeupConfig(CFG_PARA_WAKEUP_SOURCE_ADCKEY, CFG_PARA_WAKEUP_GPIO_ADCKEY, SYSWAKEUP_SOURCE_NEGE_TRIG);
#ifdef CFG_PARA_WAKEUP_SOURCE_POWERKEY
	SystermGPIOWakeupConfig(SYSWAKEUP_SOURCE6_POWERKEY, 42, SYSWAKEUP_SOURCE_NEGE_TRIG);
#endif
#endif
#if defined(CFG_PARA_WAKEUP_SOURCE_IOKEY1) && defined(CFG_PARA_WAKEUP_GPIO_IOKEY1)
	SystermGPIOWakeupConfig(CFG_PARA_WAKEUP_SOURCE_IOKEY1, CFG_PARA_WAKEUP_GPIO_IOKEY1, SYSWAKEUP_SOURCE_NEGE_TRIG);
#endif
#if defined(CFG_PARA_WAKEUP_SOURCE_IOKEY2) && defined(CFG_PARA_WAKEUP_GPIO_IOKEY2)
	SystermGPIOWakeupConfig(CFG_PARA_WAKEUP_SOURCE_IOKEY2, CFG_PARA_WAKEUP_GPIO_IOKEY2, SYSWAKEUP_SOURCE_NEGE_TRIG);
#endif
#if defined(CFG_PARA_WAKEUP_SOURCE_IR)
	SystermIRWakeupConfig(CFG_PARA_IR_SEL, CFG_RES_IR_PIN, CFG_PARA_IR_BIT);
#endif	
#if defined(CFG_PARA_WAKEUP_SOURCE_CEC) && defined(CFG_PARA_WAKEUP_GPIO_CEC)
	SystermGPIOWakeupConfig(CFG_PARA_WAKEUP_SOURCE_CEC, CFG_PARA_WAKEUP_GPIO_CEC, SYSWAKEUP_SOURCE_BOTH_EDGES_TRIG);
#endif	
}


void ModeCommonDeInit_deepsleep(void)
{
#if defined(CFG_RES_AUDIO_DAC0_EN)
	AudioDAC_Disable(DAC0);
	AudioDAC_FuncReset(DAC0);
	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_DAC0_TX, DMA_DONE_INT);
	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_DAC0_TX, DMA_THRESHOLD_INT);
	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_DAC0_TX, DMA_ERROR_INT);
	DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_DAC0_TX);

	if(mainAppCt.DACFIFO != NULL)
	{
		osPortFree(mainAppCt.DACFIFO);
		mainAppCt.DACFIFO = NULL;
	}
	AudioCoreSinkDeinit(AUDIO_DAC0_SINK_NUM);
	AudioDAC_PowerDown(DAC0);
#endif

#if defined(CFG_RES_AUDIO_DACX_EN)
	AudioCoreSinkDisable(AUDIO_DACX_SINK_NUM);
	AudioDAC_Disable(DAC1);
	AudioDAC_FuncReset(DAC1);
	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_DAC1_TX, DMA_DONE_INT);
	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_DAC1_TX, DMA_THRESHOLD_INT);
	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_DAC1_TX, DMA_ERROR_INT);
	DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_DAC1_TX);

	if(mainAppCt.DACXFIFO != NULL)
	{
		osPortFree(mainAppCt.DACXFIFO);
		mainAppCt.DACXFIFO = NULL;
	}
	AudioCoreSinkDeinit(AUDIO_DACX_SINK_NUM);
	AudioDAC_PowerDown(DAC1);
#endif

#if defined(CFG_RES_AUDIO_I2SOUT_EN)
	I2S_ModuleDisable(CFG_RES_I2S_MODULE);
	RST_I2SModule(CFG_RES_I2S_MODULE);

	DMA_InterruptFlagClear(PERIPHERAL_ID_I2S0_TX + CFG_RES_I2S * 2, DMA_DONE_INT);
	DMA_InterruptFlagClear(PERIPHERAL_ID_I2S0_TX + CFG_RES_I2S * 2, DMA_THRESHOLD_INT);
	DMA_InterruptFlagClear(PERIPHERAL_ID_I2S0_TX + CFG_RES_I2S * 2, DMA_ERROR_INT);
	DMA_ChannelDisable(PERIPHERAL_ID_I2S0_TX + CFG_RES_I2S * 2);

	if(mainAppCt.I2SFIFO != NULL)
	{
		APP_DBG("I2SFIFO\n");
		osPortFree(mainAppCt.I2SFIFO);
		mainAppCt.I2SFIFO = NULL;
	}
	AudioCoreSinkDeinit(AUDIO_I2SOUT_SINK_NUM);
#endif

#if defined(CFG_RES_AUDIO_I2S0OUT_EN)
	I2S_ModuleDisable(I2S0_MODULE);
	RST_I2SModule(I2S0_MODULE);

	DMA_InterruptFlagClear(PERIPHERAL_ID_I2S0_TX, DMA_DONE_INT);
	DMA_InterruptFlagClear(PERIPHERAL_ID_I2S0_TX, DMA_THRESHOLD_INT);
	DMA_InterruptFlagClear(PERIPHERAL_ID_I2S0_TX, DMA_ERROR_INT);
	DMA_ChannelDisable(PERIPHERAL_ID_I2S0_TX);

	if(mainAppCt.I2S0_TX_FIFO != NULL)
	{
		APP_DBG("I2S0_TX_FIFO\n");
		osPortFree(mainAppCt.I2S0_TX_FIFO);
		mainAppCt.I2S0_TX_FIFO = NULL;
	}
	AudioCoreSinkDeinit(AUDIO_I2S0_OUT_SINK_NUM);
#endif

#if defined(CFG_RES_AUDIO_I2S1OUT_EN)
	I2S_ModuleDisable(I2S1_MODULE);
	RST_I2SModule(I2S1_MODULE);

	DMA_InterruptFlagClear(PERIPHERAL_ID_I2S1_TX, DMA_DONE_INT);
	DMA_InterruptFlagClear(PERIPHERAL_ID_I2S1_TX, DMA_THRESHOLD_INT);
	DMA_InterruptFlagClear(PERIPHERAL_ID_I2S1_TX, DMA_ERROR_INT);
	DMA_ChannelDisable(PERIPHERAL_ID_I2S1_TX);

	if(mainAppCt.I2S1_TX_FIFO != NULL)
	{
		APP_DBG("I2S1_TX_FIFO\n");
		osPortFree(mainAppCt.I2S1_TX_FIFO);
		mainAppCt.I2S1_TX_FIFO = NULL;
	}
	AudioCoreSinkDeinit(AUDIO_I2S1_OUT_SINK_NUM);
#endif
}


void DeepSleeping(void)
{
	uint32_t GpioAPU_Back,GpioAPD_Back,GpioBPU_Back,GpioBPD_Back;

//	WDG_Disable();
	WDG_Feed();
	
	OTG_DeepSleepBackup();
	ADC_Disable();//����ɨ��
	AudioADC_PowerDown();
	//AudioADC_VcomConfig(2);//ע�⣬VCOM���DAC�����ص�������2��PowerDown VCOM
	SPDIF_AnalogModuleDisable();//spdif,HDMI

	ModeCommonDeInit_deepsleep();

	GpioAPU_Back = GPIO_RegGet(GPIO_A_PU);
	GpioAPD_Back = GPIO_RegGet(GPIO_A_PD);
	GpioBPU_Back = GPIO_RegGet(GPIO_B_PU);
	GpioBPD_Back = GPIO_RegGet(GPIO_B_PD);
	SystermDeepSleepConfig();
	WakeupSourceSet();
	waitCECTimeFlag = 0;
	WDG_Disable();
	Power_GotoDeepSleep();
	WDG_Enable(WDG_STEP_4S);
	while(!SystermWackupSourceCheck())
	{
		SleepAgainConfig();
		Power_WakeupDisable(0xff);
		WakeupSourceSet();
		waitCECTimeFlag = 0;
		WDG_Disable();
		Power_GotoDeepSleep();
		WDG_Enable(WDG_STEP_4S);
	}
	Power_WakeupDisable(0xff);

#if defined(CFG_PARA_WAKEUP_SOURCE_CEC) && defined(CFG_PARA_WAKEUP_GPIO_CEC)
	HDMI_CEC_DDC_DeInit();
#endif
	//GPIO�ָ�������
	GPIO_RegSet(GPIO_A_PU, GpioAPU_Back);
	GPIO_RegSet(GPIO_A_PD, GpioAPD_Back);
	GPIO_RegSet(GPIO_B_PU, GpioBPU_Back);
	GPIO_RegSet(GPIO_B_PD, GpioBPD_Back);
#if 1
    GPIO_PortBModeSet(GPIOB0, 0x0001); //���Կ� SW�ָ� ��������
    GPIO_PortBModeSet(GPIOB1, 0x0001);
#else
	GPIO_PortAModeSet(GPIOA30, 0x0005);//���Կ� SW�ָ� ��������
	GPIO_PortAModeSet(GPIOA31, 0x0004);
#endif
	WDG_Feed();
	WakeupMain();
	SysTickInit();
	WDG_Feed();
	uart_switch = 1;

#if defined(CFG_RES_ADC_KEY_SCAN) || defined(CFG_RES_IR_KEY_SCAN) || defined(CFG_RES_CODE_KEY_USE)|| defined(CFG_ADC_LEVEL_KEY_EN) || defined(CFG_RES_IO_KEY_SCAN)
	KeyInit();//Init keys
#endif	
#ifdef BT_TWS_SUPPORT
	tws_time_init();
#endif

	OTG_WakeupResume();

	ModeCommonInit();

#ifdef CFG_FUNC_LED_REFRESH
	//Ĭ�����ȼ�Ϊ0��ּ�����ˢ�����ʣ��ر��Ƕϵ�����дflash������Ӱ��ˢ���������ϸ���������timer6�жϵ��ö���TCM���룬�����õ�driver�����
	//��ȷ��GPIO_RegOneBitSet��GPIO_RegOneBitClear��TCM��������api����ȷ�ϡ�
	NVIC_SetPriority(Timer6_IRQn, 0);
 	Timer_Config(TIMER6,1000,0);
 	Timer_Start(TIMER6);
 	NVIC_EnableIRQ(Timer6_IRQn);
#endif

#ifdef CFG_FUNC_POWER_MONITOR_EN
	extern void PowerMonitorInit(void);
	PowerMonitorInit();
#endif

#if defined(CFG_FUNC_DISPLAY_EN)
    DispInit(0);
#endif
}


bool SystermWackupSourceCheck(void)
{
#ifdef CFG_RES_ADC_KEY_SCAN
	AdcKeyMsg AdcKeyVal;
#endif

#ifdef CFG_PARA_WAKEUP_SOURCE_IR
	IRKeyMsg IRKeyMsg;
#endif
	TIMER WaitScan;

#if defined(CFG_PARA_WAKEUP_SOURCE_ADCKEY)
	SarADC_Init();
	AdcKeyInit();
#endif
	uart_switch = 0;
//********************
	//����IO����
	LogUartConfig(FALSE);//�˴��������clk�����ʣ���Ϊ��ʱ�������䡣
	SysTickInit();
	//APP_DBG("Scan:%x\n", (int)sources);
#ifdef CFG_PARA_WAKEUP_SOURCE_RTC
	if(sources & CFG_PARA_WAKEUP_SOURCE_RTC)
	{
		sources = 0;//����Դ����
		//APP_DBG("Alarm!%d", RTC_SecGet());
		return TRUE;
	}
	else if(alarm)//����RTC�����¼���ʧ
	{
		uint32_t NowTime;
		NowTime = RTC_SecGet();
		if(NowTime + 2 + CHECK_SCAN_TIME / 1000 > alarm)//������ڶ������Դ��rtc������ǰ����()���Ա��ⶪʧ
		{
			//APP_DBG("Timer");
			sources = 0;//����Դ����
			alarm = 0;
			return TRUE;
		}
	}
#endif

#if defined(CFG_PARA_WAKEUP_SOURCE_CEC) && defined(CFG_PARA_WAKEUP_GPIO_CEC)
	bool CecRest = FALSE;//cec����״̬(18ms����)������Ӱ����������ͻ��ѡ�
	HDMI_HPD_CHECK_IO_INIT();
#endif

	TimeOutSet(&WaitScan, CHECK_SCAN_TIME);
	while(!IsTimeOut(&WaitScan)
#if defined(CFG_PARA_WAKEUP_SOURCE_CEC) && defined(CFG_PARA_WAKEUP_GPIO_CEC)
			|| !CecRest
#endif
		)
	{
		WDG_Feed();
#ifdef CFG_PARA_WAKEUP_SOURCE_IR
		if(sources & CFG_PARA_WAKEUP_SOURCE_IR)
		{
			IRKeyMsg = IRKeyScan();			
			if(IRKeyMsg.index != IR_KEY_NONE && IRKeyMsg.type != IR_KEY_UNKOWN_TYPE)
			{
				//APP_DBG("IRID:%d,type:%d\n", IRKeyMsg.index, IRKeyMsg.type);
				SetIrKeyValue(IRKeyMsg.type,IRKeyMsg.index);
				if((GetGlobalKeyValue() == MSG_DEEPSLEEP)||(GetGlobalKeyValue() == MSG_BT_SNIFF))
				{
					sources = 0;
					ClrGlobalKeyValue();
					return TRUE;
				}
				ClrGlobalKeyValue();
			}
		}
#endif
#ifdef CFG_PARA_WAKEUP_SOURCE_POWERKEY
		if(sources & SYSWAKEUP_SOURCE6_POWERKEY)
		{
			sources = 0;
			return TRUE;
		}
#endif
#if defined(CFG_PARA_WAKEUP_SOURCE_ADCKEY) && defined(CFG_RES_ADC_KEY_SCAN)
		if(sources & (CFG_PARA_WAKEUP_SOURCE_ADCKEY))
		{
			AdcKeyVal = AdcKeyScan();
			if(AdcKeyVal.index != ADC_CHANNEL_EMPTY && AdcKeyVal.type != ADC_KEY_UNKOWN_TYPE)
			{
				//APP_DBG("KeyID:%d,type:%d\n", AdcKeyVal.index, AdcKeyVal.type);
				SetAdcKeyValue(AdcKeyVal.type,AdcKeyVal.index);
				if((GetGlobalKeyValue() == MSG_DEEPSLEEP)||(GetGlobalKeyValue() == MSG_BT_SNIFF))
				{
					sources = 0;
					ClrGlobalKeyValue();
					return TRUE;
				}
				ClrGlobalKeyValue();
			}
		}
#endif
#ifdef CFG_RES_IO_KEY_SCAN
#ifdef CFG_PARA_WAKEUP_SOURCE_IOKEY1
		if(sources & CFG_PARA_WAKEUP_SOURCE_IOKEY1)
		{
			sources = 0;
			return TRUE;
		}
#endif
#ifdef CFG_PARA_WAKEUP_SOURCE_IOKEY2
		if(sources & CFG_PARA_WAKEUP_SOURCE_IOKEY2)
		{
			sources = 0;
			return TRUE;
		}
#endif
#endif

#if defined(CFG_PARA_WAKEUP_SOURCE_CEC) && defined(CFG_PARA_WAKEUP_GPIO_CEC)
		HDMI_CEC_Scan(0);
		if(gHdmiCt->hdmi_poweron_flag == 1)
		{
			SoftFlagRegister(SoftFlagWakeUpSouceIsCEC);
			APP_DBG("CEC PowerOn\n");
			return TRUE;
		}
		if(IsTimeOut(&WaitScan))//��ʱ֮��ȴ�������ƽ��ͬʱ����scan��
		{
			CecRest = HDMI_CEC_IsReadytoDeepSleep(6);
		}
#endif

	}
	sources = 0;
//	SysTickDeInit();
	return FALSE;
}


void SleepAgainConfig(void)
{
#ifndef	BT_SNIFF_ENABLE
	GPIO_RegSet(GPIO_A_IE,0x00000000);
	GPIO_RegSet(GPIO_A_OE,0x00000000);
	GPIO_RegSet(GPIO_A_OUTDS,0x00000000);//bkd GPIO_A_REG_OUTDS
	GPIO_RegSet(GPIO_A_PD,0xffffffff
#if defined(CFG_PARA_WAKEUP_GPIO_CEC) && defined(CFG_PARA_WAKEUP_SOURCE_CEC)//cec�˿ڲ������������ã���Ҫcec״̬���ϡ�
			& ~ BIT(CFG_PARA_WAKEUP_GPIO_CEC)
#endif
			);
	GPIO_RegSet(GPIO_A_PU,0x00000000);//��ʱ��flash��CS��������0x00400000
	GPIO_RegSet(GPIO_A_ANA_EN,0x00000000);
	GPIO_RegSet(GPIO_A_PULLDOWN0,0x00000000);//bkd
	GPIO_RegSet(GPIO_A_PULLDOWN1,0x00000000);//bkd

	GPIO_RegSet(GPIO_B_IE,0x00);
	GPIO_RegSet(GPIO_B_OE,0x00); 
	GPIO_RegSet(GPIO_B_OUTDS,0x00); // bkd mark GPIO_B_REG_OUTDS
	GPIO_RegSet(GPIO_B_PD,0xff);//B2��B3������B4,B5���� 0x1cc
	GPIO_RegSet(GPIO_B_PU,0x00);//B0��B1���� 0x03
	GPIO_RegSet(GPIO_B_ANA_EN,0x00);
	GPIO_RegSet(GPIO_B_PULLDOWN,0x00);//bkd mark GPIO_B_REG_PULLDOWN

#else

	GPIO_RegSet(GPIO_A_IE,0x00000000 | (BIT(23)));
	GPIO_RegSet(GPIO_A_OE,0x00000000);
	GPIO_RegSet(GPIO_A_OUTDS,0x00000000);//bkd GPIO_A_REG_OUTDS
	GPIO_RegSet(GPIO_A_PD,0xffffffff & (~ BIT(23)));
	GPIO_RegSet(GPIO_A_PU,0x00000000 | (BIT(23)));//��ʱ��flash��CS��������0x00400000
	GPIO_RegSet(GPIO_A_ANA_EN,0x00000000);
	GPIO_RegSet(GPIO_A_PULLDOWN0,0x00000000);//bkd
	GPIO_RegSet(GPIO_A_PULLDOWN1,0x00000000);//bkd

	GPIO_RegSet(GPIO_B_IE,0x00);
	GPIO_RegSet(GPIO_B_OE,0x00);
	GPIO_RegSet(GPIO_B_OUTDS,0x00); // bkd mark GPIO_B_REG_OUTDS
	GPIO_RegSet(GPIO_B_PD,0xff);//B2��B3������B4,B5���� 0x1cc
	GPIO_RegSet(GPIO_B_PU,0x00);//B0��B1���� 0x03
	GPIO_RegSet(GPIO_B_ANA_EN,0x00);
	GPIO_RegSet(GPIO_B_PULLDOWN,0x00);//bkd mark GPIO_B_REG_PULLDOWN
#endif
}

#endif//CFG_FUNC_DEEPSLEEP_EN



#ifdef BT_SNIFF_ENABLE

//IR�˳�sniff��ѯ
void IrWakeupProcess(void)
{
	uint32_t Cmd = 0;
	uint8_t val = 0;

	//printf("IrWakeupProcess\n");
	if(IR_CommandFlagGet())
	{
		Cmd = IR_CommandDataGet();
		val = IRKeyIndexGet_BT(Cmd);

		SetIrKeyValue((uint8_t)2,(uint16_t)val);
		//APP_DBG("cmd:0x%lx,0x%d,%x\n",Cmd,GetIrKeyValue(),val);
		if(GetIrKeyValue() == MSG_BT_SNIFF)
		{
			extern void BtSniffExit_process(void);
#ifdef CFG_IDLE_MODE_DEEP_SLEEP
			sources = 0;
#endif
			ClrGlobalKeyValue();
			IR_Disable();
			BtSniffExit_process();
		}
		IR_IntFlagClear();
		IR_CommandFlagClear();
	}
}

uint8_t GetDebugPrintPort(void);
void UartClkChange(CLK_MODE clk_change)//���������������
{
	//����ʱ���л�
#ifdef FUNC_OS_EN
	if(GetDebugPrintPort())
	{
		osMutexLock(UART1Mutex);
	}
	else
	{
		osMutexLock(UART0Mutex);
	}
#endif
	Clock_UARTClkSelect(clk_change);//���л�log clk������������ٴ���
	LogUartConfig(TRUE);
#ifdef FUNC_OS_EN
	if(GetDebugPrintPort())
	{
		osMutexUnlock(UART1Mutex);;
	}
	else
	{
		osMutexUnlock(UART0Mutex);;
	}

#endif
}

#ifdef CFG_APP_HDMIIN_MODE_EN
//cec�˳�sniff
void CecWakeupProcess(void)
{
	HDMI_CEC_Scan(0);
	if((gHdmiCt->hdmi_poweron_flag == 1) && (GetSystemMode() == ModeHdmiAudioPlay))
	{
		//APP_DBG("CEC waked!!!!\n");
		SoftFlagRegister(SoftFlagWakeUpSouceIsCEC);
		extern void BtSniffExit_process(void);
		sources = 0;
		BtSniffExit_process();
	}
}
#endif

uint8_t sniffiocnt = 0;
uint8_t sniff_wakeup_check()
{
#if defined(CFG_PARA_WAKEUP_SOURCE_CEC) && defined(CFG_APP_HDMIIN_MODE_EN)
	bool CecRest = FALSE;
#endif

//	if(!GPIO_RegOneBitGet(GPIO_A_IN,GPIOA23))
//	{//�������̣�������������sniff���ھͻ��ѡ������ʱ���жϿ��ܵ���sniff���ܲ�����
//
//		if(sniffiocnt > 2)//���������������ھ��˳���
//		{//�˳�sniff��
//			sniffiocnt = 0;
//			extern void BtSniffExit_process(void);
//			BtSniffExit_process();
//
//			return 1;
//		}
//		else
//		{
//			sniffiocnt++;
//
//			return 1;
//		}
//
//	}

#if defined(CFG_PARA_WAKEUP_SOURCE_ADCKEY) && defined(CFG_RES_ADC_KEY_SCAN)
		if(sources & (CFG_PARA_WAKEUP_SOURCE_ADCKEY))
		{
			AdcKeyMsg AdcKeyVal;

			SarADC_Init();
			AdcKeyInit();

			sources = 0;
			//���û����RELEASED����while����
			do
			{
				AdcKeyVal = AdcKeyScan();

				if(AdcKeyVal.index != ADC_CHANNEL_EMPTY && AdcKeyVal.type != ADC_KEY_UNKOWN_TYPE)
				{
//					APP_DBG("KeyID:%d,type:%d\n", AdcKeyVal.index, AdcKeyVal.type);
					SetAdcKeyValue(AdcKeyVal.type,AdcKeyVal.index);
					if((GetGlobalKeyValue() == MSG_DEEPSLEEP)||(GetGlobalKeyValue() == MSG_BT_SNIFF))
					{
						ClrGlobalKeyValue();
						extern void BtSniffExit_process(void);
						BtSniffExit_process();
						return 1;
					}
					ClrGlobalKeyValue();
				}
			}while((AdcKeyVal.type != ADC_KEY_RELEASED) && (AdcKeyVal.type != ADC_KEY_LONG_RELEASED));
			return 1;
		}
#endif

#ifdef CFG_PARA_WAKEUP_SOURCE_POWERKEY
		if(sources & SYSWAKEUP_SOURCE6_POWERKEY)
		{
			APP_DBG("POWER_Key!\n");
			sources = 0;
			extern void BtSniffExit_process(void);
			BtSniffExit_process();
			return 1;
		}
#endif

#if defined(CFG_PARA_WAKEUP_SOURCE_IR)
	if(sources & CFG_PARA_WAKEUP_SOURCE_IR)
	{
		sources = 0;
		IrWakeupProcess();
		return 1;
	}
#endif

#if defined(CFG_PARA_WAKEUP_SOURCE_CEC) && defined(CFG_APP_HDMIIN_MODE_EN)
	if(sources & (CFG_PARA_WAKEUP_SOURCE_CEC))
	{
		//while(!IsTimeOut(&waitCECTime))
		{
#if defined(CFG_PARA_WAKEUP_SOURCE_IR)
			IrWakeupProcess();
#endif
			CecWakeupProcess();
		}
		if(IsTimeOut(&waitCECTime))//��ʱ֮��ȴ�������ƽ��ͬʱ����scan��
			CecRest = HDMI_CEC_IsReadytoDeepSleep(6);
		else
			return 1;

		if(!CecRest)
		{
			return 1;
		}
		sources = 0;
		return 1;
	}
#endif
	sources = 0;
	return 0;

}

void BtDeepSleepForUsr(void)//��������������ڣ�Ŀǰû������
{
	//��������pll�е�����
//	uint32_t SLOPE, NDAC, OS, K1, FC;
//	uint32_t GpioAPU_Back,GpioAPD_Back,GpioBPU_Back,GpioBPD_Back;
//
//	Clock_GetDPll(&NDAC, &OS, &K1, &FC);
//	SLOPE = DPLL_QUICK_START_SLOPE;//��ȡ����������������

//	GPIO_PortAModeSet(GPIOA30,0);		  //ȥ��Sw���á����Կ�
//	GPIO_PortAModeSet(GPIOA31,0);

	UartClkChange(RC_CLK_MODE);
	GIE_DISABLE();
	DeepSleepIOConfig();
	Power_DeepSleepLDO12ConfigTest(0,5,0);//����deepsleepʱ��ѹ��Ϊ1V0
	SysTickDeInit();

	NVIC_EnableIRQ(Wakeup_IRQn);
	NVIC_SetPriority(Wakeup_IRQn, 0);
	Power_WakeupSourceClear();
	Power_WakeupSourceSet(SYSWAKEUP_SOURCE13_BT, 0, 0);//��������Ϊ����Դ������IO�����������0
	Power_WakeupEnable(SYSWAKEUP_SOURCE13_BT);

#if defined(CFG_PARA_WAKEUP_SOURCE_IR)
	SystermIRWakeupConfig_sniff(CFG_PARA_IR_SEL, CFG_RES_IR_PIN, CFG_PARA_IR_BIT);
#endif

	NVIC_DisableForDeepsleep();//�ر������жϣ�ֻ�������ж�

	Clock_DeepSleepSysClkSelect(RC_CLK_MODE, FSHC_RC_CLK_MODE, TRUE);

	Clock_PllClose();
	Clock_LOSCDisable();
//	Clock_HOSCDisable();
#ifdef CFG_IDLE_MODE_DEEP_SLEEP
	sources = 0;
	waitCECTimeFlag = 0;
//	GIE_ENABLE();//Power_GotoDeepSleep�����п�ʱ�Ӷ���
#endif
	Power_GotoDeepSleep();

	GIE_DISABLE();
//	Clock_PllQuicklock(288000, K1, OS, NDAC, FC, SLOPE);
#ifdef BT_TWS_SUPPORT
	Clock_PllLock(SYS_CORE_DPLL_FREQ / 10);
	*(uint32_t*)0x40026008 = ((uint64_t)8192 * SYS_CORE_DPLL_FREQ / 10000);  // 0x2BBF48--349M    0x27837B--316M
#else
    Clock_PllLock(288000);
#endif
	Clock_DeepSleepSysClkSelect(PLL_CLK_MODE,FSHC_PLL_CLK_MODE,FALSE);

	NVIC_EnableForDeepsleep();//�жϻָ�

//	GIE_ENABLE();

	SysTickInit();//��ʼOS ��ȫ��ʱ��

	//UartClkChange(PLL_CLK_MODE);

//	GPIO_PortAModeSet(GPIOA30, 0x0005);//���Կ� SW�ָ� ��������
//	GPIO_PortAModeSet(GPIOA31, 0x0004);

//	extern uint8_t OtgPortLinkState;
//	Timer_Config(TIMER2,1000,0);
//	Timer_Start(TIMER2);
//	NVIC_EnableIRQ(Timer2_IRQn);
//	OtgPortLinkState = 0;


}

uint8_t sniff_wakeup_flag = 0;//�������߻����Ƿ񴥷�sniff��־��Ϊ��ϵͳ�ȶ����˱�־��ȷ��ϵͳ����stackӰ��
							  //0sniff����û����   1sniff�Ѿ�����
void sniff_wakeup_set(uint8_t set)
{
	sniff_wakeup_flag = set;
}
uint8_t sniff_wakeup_get(void)
{
	return sniff_wakeup_flag;
}

uint8_t sniff_lmp_sync_flag = 0;//sniff��lmpͬ�������Ѿ����͹���־����ֹ����UI��η���lmp�����쳣,�˱�־��ȷ��Э��ջ����ϵͳӰ��
								//0��ʾδ���Ϳɽ��ܣ�1��ʾ�Ѿ���������ȴ���
void sniff_lmpsend_set(uint8_t set)
{
	sniff_lmp_sync_flag = set;
}
uint8_t sniff_lmpsend_get()
{
	return sniff_lmp_sync_flag;
}

void tws_stop_callback()//����sniffʱ��������
{
	SysDeepsleepStandbyStatus();
	printf("SysDeepsleepStandbyStatus\n");
}

void tws_sniff_check_adda_process()
{
//	if(GetBtManager()->twsRole == BT_TWS_SLAVE)
	{

//		if(BtSniffADDAReadyGet() == 3)
//		{
//			printf("wakeup all ready\n");
//			if(sniff_lmpsend_get() == 1)
//			{
//				//##__�˳�sniff����־λ�ָ�__##
//				sniff_lmpsend_set(0);
//				printf("sniff_lmpsend_set(0)\n");
//				//##____________________##
//			}
//			BtSniffADDAReadySet(0);//���ADDA׼����־
//			tws_link_status_set(1);
//			printf("tws_link_status_set(1)\n");
//		}

		//��·��Ͽ��󣬷���sniff�б�־û�塣
		if((sniff_wakeup_get() /*|| sniff_lmpsend_get()*/)
#ifdef BT_TWS_SUPPORT
				&& (GetBtManager()->twsState != BT_TWS_STATE_CONNECTED)
#endif
				)
		{
			//�Ͽ����sniff״̬�ָ�
			sniff_wakeup_set(0);
			//sniff_lmpsend_set(0);

		}
	}
}

TIMER   sniffrerequsettimer;
#define RESEND_SCAN_TIME				2000		//����ȷ����Ч����Դ ɨ����ʱms��
uint32_t deepsleep_count = 0;
void DeepSleeping_BT(void)
{
	uint32_t GpioAPU_Back,GpioAPD_Back,GpioBPU_Back,GpioBPD_Back;

	//Efuse_ReadDataDisable();
//	SysDeepsleepStart();


	OTG_DeepSleepBackup();
	ADC_Disable();//����ɨ��
	AudioADC_PowerDown();
	//AudioADC_VcomConfig(2);//ע�⣬VCOM���DAC�����ص�������2��PowerDown VCOM
	SPDIF_AnalogModuleDisable();//spdif,HDMI

	#ifdef CFG_RES_AUDIO_DAC0_EN
		AudioCoreSinkDeinit(AUDIO_DAC0_SINK_NUM);
		AudioDAC_PowerDown(DAC0);
	#endif

	#ifdef CFG_RES_AUDIO_DACX_EN
		AudioCoreSinkDeinit(AUDIO_DACX_SINK_NUM);
		AudioDAC_PowerDown(DAC1);
	#endif


	deepsleep_count = 1;

	BtStartEnterSniffMode();
	TimeOutSet(&sniffrerequsettimer, RESEND_SCAN_TIME);
	while((Bt_sniff_sniff_start_state_get() == 0) ||
			(Bt_sniff_sleep_state_get() == 0))
	{
		WDG_Feed();
		if(IsTimeOut(&sniffrerequsettimer))
		{
			APP_DBG("LMP sniff state ERR!!!\r\n");
			BtStartEnterSniffMode();
#ifdef BT_TWS_SUPPORT
			//�Ͽ����Ӻ������ȴ�siff req��Ȼ�����͹���ɨ��
			if(GetBtManager()->twsState != BT_TWS_STATE_CONNECTED)
			{
				Bt_sniff_sniff_start();
				break;
			}

			deepsleep_count++;
			if(deepsleep_count > 3)
			{
				Bt_sniff_sniff_start();
				break;
			}
#endif
			TimeOutSet(&sniffrerequsettimer, RESEND_SCAN_TIME);
		}

		vTaskDelay(2);
	}


	GpioAPU_Back = GPIO_RegGet(GPIO_A_PU);//����������
	GpioAPD_Back = GPIO_RegGet(GPIO_A_PD);
	GpioBPU_Back = GPIO_RegGet(GPIO_B_PU);
	GpioBPD_Back = GPIO_RegGet(GPIO_B_PD);

#ifdef BT_TWS_SUPPORT
	if(tws_get_role() == BT_TWS_MASTER)
		BTSetAccessMode(BtAccessModeNotAccessible);
	
#if (TWS_PAIRING_MODE == CFG_TWS_ROLE_MASTER)
		DisableAdvertising();
#endif
#endif

#ifdef CFG_PARA_WAKEUP_SOURCE_POWERKEY
	SystermGPIOWakeupConfig(SYSWAKEUP_SOURCE6_POWERKEY, 42, SYSWAKEUP_SOURCE_NEGE_TRIG);
#endif

#if defined(CFG_PARA_WAKEUP_SOURCE_ADCKEY)
	SystermGPIOWakeupConfig(CFG_PARA_WAKEUP_SOURCE_ADCKEY, CFG_PARA_WAKEUP_GPIO_ADCKEY, SYSWAKEUP_SOURCE_NEGE_TRIG);
#endif

#ifdef CFG_IDLE_MODE_DEEP_SLEEP
	sources = 0;//����ǰ������л����жϡ�
	waitCECTimeFlag = 0;
#endif
	BTSniffSet();//׼������sniff

	WDG_Feed();
	while(Bt_sniff_sniff_start_state_get())//û�˳�sniff��Ϣ������sniff������ѯ��
	{
		vTaskDelay(1);
		if(Bt_sniff_sleep_state_get())
		{
			Bt_sniff_sleep_exit();
#ifdef BT_TWS_SUPPORT
			if(GetBtManager()->twsRole == BT_TWS_MASTER)
			{
				//����Ŀ�ӻ���UI������ע�͵��˴ӻ����ѵ��߼�
				if(sniff_wakeup_check())// ������ֻ��ѱ�־�����ڲ�˯�����Һ����ڲ���������sniff
				{
					Bt_sniff_sleep_exit();
					continue;
				}
			}
#endif
			BtDeepSleepForUsr();

			if(sources & (0x1fff)) //except bt source
			{
				if(sniff_wakeup_check()) //���������Ѻ�,���ȴ�����������Դ
				{
					Bt_sniff_sleep_exit();
					continue;
				}
			}
		}
	}
	WDG_Feed();

#ifdef BT_TWS_SUPPORT
	if(tws_get_role() == BT_TWS_MASTER)
	{
		if(sys_parameter.Bt_BackgroundType == BT_BACKGROUND_FAST_POWER_ON_OFF)
		{
			if(IsBtAudioMode())
			{
				BtExitSniffReconnectFlagSet();
			}
		}
		else
			BtExitSniffReconnectFlagSet();
		BTSetAccessMode(BtAccessModeGeneralAccessible);
	}
#endif

	GPIO_RegSet(GPIO_A_PU, GpioAPU_Back);
	GPIO_RegSet(GPIO_A_PD, GpioAPD_Back);
	GPIO_RegSet(GPIO_B_PU, GpioBPU_Back);
	GPIO_RegSet(GPIO_B_PD, GpioBPD_Back);

	Efuse_ReadDataEnable();

#ifdef BT_TWS_SUPPORT
#if (TWS_PAIRING_MODE == CFG_TWS_ROLE_MASTER)
	ble_advertisement_data_update();
#elif (TWS_PAIRING_MODE == CFG_TWS_ROLE_SLAVE)
	BleScanParamConfig_Default();
#endif
#endif
	UartClkChange(APLL_CLK_MODE);

//	WDG_Feed();
//	WakeupMain();
//	WDG_Feed();

#ifdef BT_TWS_SUPPORT
	tws_time_init();//TWS RTCУ׼
#endif

#if (TWS_PAIRING_MODE == CFG_TWS_ROLE_SLAVE || TWS_PAIRING_MODE == CFG_TWS_ROLE_MASTER)

	WDG_Feed();
#if defined(CFG_RES_ADC_KEY_SCAN) || defined(CFG_RES_IR_KEY_SCAN) || defined(CFG_RES_CODE_KEY_USE)|| defined(CFG_ADC_LEVEL_KEY_EN) || defined(CFG_RES_IO_KEY_SCAN)
//vTaskDelay(1000);
	SarADC_Init();
	KeyInit();//Init keys
#endif

#endif//(TWS_PAIRING_MODE == CFG_TWS_ROLE_SLAVE)

	WDG_Feed();
	ModeCommonInit();
	WDG_Feed();

#ifdef CFG_FUNC_LED_REFRESH
	//Ĭ�����ȼ�Ϊ0��ּ�����ˢ�����ʣ��ر��Ƕϵ�����дflash������Ӱ��ˢ���������ϸ���������timer6�жϵ��ö���TCM���룬�����õ�driver�����
	//��ȷ��GPIO_RegOneBitSet��GPIO_RegOneBitClear��TCM��������api����ȷ�ϡ�
	NVIC_SetPriority(Timer6_IRQn, 0);
 	Timer_Config(TIMER6,1000,0);
 	Timer_Start(TIMER6);
 	NVIC_EnableIRQ(Timer6_IRQn);

 	//���д������������ʱ�����Timer�жϴ����������ͻ�һ��Ҫ���޸ĵ���
 	//GPIO_RegOneBitSet(GPIO_A_OE, GPIO_INDEX2);//only test��user must modify
#endif

#if defined(CFG_FUNC_DISPLAY_EN)
    DispInit(0);
#endif

#ifdef BT_SNIFF_ENABLE
	sniff_lmpsend_set(0);
#endif

}

#else
void tws_stop_callback()//����sniffʱ��������
{

}

#endif //BT_SNIFF_ENABLE

