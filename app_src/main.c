#include <stdlib.h>
#include <nds32_intrinsic.h>
#include "uarts.h"
#include "uarts_interface.h"
#include "backup.h"
#include "backup_interface.h"
#include "type.h"
#include "debug.h"
#include "timeout.h"
#include "clk.h"
#include "dma.h"
#include "timer.h"
#include "adc.h"
#include "i2s.h"
#include "watchdog.h"
#include "reset.h"
#include "rtc.h"
#include "spi_flash.h"
#include "gpio.h"
#include "chip_info.h"
#include "irqn.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "remap.h"
#include "rtos_api.h"
#include "main_task.h"
#include "dac.h"
#include "otg_detect.h"
#include "sw_uart.h"
#include "remind_sound.h"
#ifdef CFG_APP_BT_MODE_EN
#include "bt_common_api.h"
#endif
#include "file.h"
#include "flash_boot.h"
#include "sadc_interface.h"
#include "app_config.h"
#include "powercontroller.h"
#include "audio_decoder_api.h"
#include "sys.h"
#ifdef CFG_FUNC_DISPLAY_EN
#include "display.h"
#endif

#ifdef CFG_APP_BT_MODE_EN
#if (BT_HFP_SUPPORT == ENABLE)
#include "bt_hf_mode.h"
#endif
#endif
#include "tws_mode.h"
#include "rtc_timer.h"
#include "rtc_ctrl.h"
#include "efuse.h"
#include "mode_task.h"
#include "device_detect.h"
#include "idle_mode.h"
#include "flash_table.h"
#include "sys_param.h"

#include "bt_em_config.h"

//-----------------globle timer----------------------//
volatile uint32_t gInsertEventDelayActTimer = 2500; // ms
volatile uint32_t gChangeModeTimeOutTimer = CHANGE_MODE_TIMEOUT_COUNT;
volatile uint32_t gDeviceCheckTimer = DEVICE_DETECT_TIMER;				  // ms
volatile uint32_t gDeviceUSBDeviceTimer = DEVICE_USB_DEVICE_DETECT_TIMER; // ms
#ifdef CFG_FUNC_CARD_DETECT
volatile uint32_t gDeviceCardTimer = DEVICE_CARD_DETECT_TIMER; // ms
#endif
#ifdef CFG_LINEIN_DET_EN
volatile uint32_t gDeviceLineTimer = DEVICE_LINEIN_DETECT_TIMER; // ms
#endif

#ifdef HDMI_HPD_CHECK_DETECT_EN
volatile uint32_t gDevicehdmiTimer = DEVICE_HDMI_DETECT_TIMER; // ms
#endif

#ifdef CFG_FUNC_BREAKPOINT_EN
volatile uint32_t gBreakPointTimer = 0; // ms
#endif
#if defined(CFG_APP_IDLE_MODE_EN) && defined(CFG_FUNC_REMIND_SOUND_EN)
volatile uint32_t gIdleRemindSoundTimeOutTimer = 0; // ms
#endif
//-----------------globle timer----------------------//
extern void DBUS_Access_Area_Init(uint32_t start_addr);
extern const unsigned char *GetLibVersionFatfsACC(void);
extern void UsbAudioTimer1msProcess(void);
extern void user_fs_api_init(void);
extern void EnableSwUartAsUART(uint8_t EnableFlag); // retarget.c
extern void report_up_grate(void);
extern void vApplicationIdleHook(void);
extern void trace_TASK_SWITCHED_IN(void);
extern void trace_TASK_SWITCHED_OUT(void);
extern void bt_api_init(void);
extern void uart_log_out(void);
typedef void (*rtosfun)(void);

#ifndef CFG_FUNC_STRING_CONVERT_EN
extern rtosfun pvApplicationIdleHook;
extern rtosfun ptrace_TASK_SWITCHED_IN;
extern rtosfun ptrace_TASK_SWITCHED_OUT;
#endif

extern volatile uint8_t uart_switch;

void _printf_float()
{
}

void ResetFlagGet(uint8_t Flag)
{
	APP_DBG("RstFlag = %x\n", Flag);

	if (Flag & 0x01)
	{
		APP_DBG("power on reset\n");
	}
	if (Flag & 0x02)
	{
		APP_DBG("pin reset\n");
	}
	if (Flag & 0x04)
	{
		APP_DBG("watchdog reset\n");
	}
	if (Flag & 0x08)
	{
		APP_DBG("LVD reset\n");
	}
	if (Flag & 0x10)
	{
		APP_DBG("cpu debug reset\n");
	}
	if (Flag & 0x20)
	{
		APP_DBG("system reset\n");
	}
	if (Flag & 0x40)
	{
		APP_DBG("cpu core reset\n");
	}
	APP_DBG("\n");
}

//__attribute__((section(".driver.isr")))
void OneMSTimer(void)
{
	if (gInsertEventDelayActTimer)
		gInsertEventDelayActTimer--;
	if (gChangeModeTimeOutTimer)
		gChangeModeTimeOutTimer--;
	if (gDeviceCheckTimer)
		gDeviceCheckTimer--;
	if (gDeviceUSBDeviceTimer > 1)
		gDeviceUSBDeviceTimer--;

#ifdef CFG_FUNC_BREAKPOINT_EN
	if (gBreakPointTimer > 1)
		gBreakPointTimer--;
#endif

#if defined(CFG_APP_IDLE_MODE_EN) && defined(CFG_FUNC_REMIND_SOUND_EN)
	gIdleRemindSoundTimeOutTimer++;
#endif
}

void Timer2Interrupt(void)
{
	Timer_InterruptFlagClear(TIMER2, UPDATE_INTERRUPT_SRC);
#if defined(CFG_FUNC_USB_DEVICE_EN) || defined(CFG_FUNC_UDISK_DETECT)
	OTG_PortLinkCheck();
#endif

#ifdef CFG_APP_USB_AUDIO_MODE_EN
	UsbAudioTimer1msProcess();
#endif

	uart_log_out();
	OneMSTimer();
}

void SystemClockInit(void)
{
	//	Clock_SysClkSelect(RC_CLK_MODE);

	Clock_Config(1, 24000000);
	Clock_PllLock(316108);

	*(uint32_t *)0x40026008 = 0x27837B;

	Clock_APllLock(240000);
	Clock_USBClkDivSet(4);
	Clock_SysClkSelect(PLL_CLK_MODE);
	Clock_USBClkSelect(APLL_CLK_MODE);
	Clock_UARTClkSelect(APLL_CLK_MODE);
	Clock_Timer3ClkSelect(RC_CLK_MODE);
	Clock_Module1Enable(ALL_MODULE1_CLK_SWITCH);
	Clock_Module2Enable(ALL_MODULE2_CLK_SWITCH);
	Clock_Module3Enable(ALL_MODULE3_CLK_SWITCH);
	Clock_ApbClkDivSet(5);
}

void LogUartConfig(bool InitBandRate)
{
	GPIO_PortAModeSet(GPIOA6, 5); // Tx, A6:uart0_rxd_1
	if (InitBandRate)
	{
		DbgUartInit(0, CFG_UART_BANDRATE, 8, 0, 1);
	}
}

#ifdef CFG_IDLE_MODE_DEEP_SLEEP
extern void FshcClkSwitch(FSHC_CLK_MODE ClkSrc, uint32_t flash_clk);
void SleepMain(void)
{
#ifdef BT_TWS_SUPPORT
	RTC_IntDisable();
	NVIC_DisableIRQ(Rtc_IRQn);
	RTC_ClockSrcSel(RC_32K);
#endif
	Power_DeepSleepLDO12ConfigTest(0, 5, 0); // ����deepsleep��1V2 ����Ϊ1.0V���Ը���ʡ�硣
	Clock_UARTClkSelect(RC_CLK_MODE);		 // ���л�log clk������������ٴ���
	LogUartConfig(TRUE);					 // scan����ӡʱ ������
	SysTickDeInit();
	Efuse_ReadDataDisable();								 //////////�ر�EFUSE////////
	SpiFlashInit(80000000, MODE_1BIT, 0, FSHC_PLL_CLK_MODE); // rcʱ�� ��֧��flash 4bit��ϵͳ�ָ�ʱ���䡣
	FshcClkSwitch(FSHC_RC_CLK_MODE, 80000000);				 // ����RC
	Clock_DeepSleepSysClkSelect(RC_CLK_MODE, FSHC_RC_CLK_MODE, 1);
	Clock_PllClose();
	Clock_AUPllClose(); // AUPLL������Լ980uA�Ĺ���
#if !defined(CFG_RES_RTC_EN)
	Clock_HOSCDisable(); // ����RTCӦ�ò���RTC����ʱ����HOSC���򲻹ر�HOSC����24M����
#endif
	//	Clock_LOSCDisable(); //����RTCӦ�ò���RTC����ʱ����LOSC���򲻹ر�LOSC����32K����

	SysTickInit();
}

void WakeupMain(void)
{
	Chip_Init(1);
	SysTickDeInit();
	WDG_Enable(WDG_STEP_4S);
	Efuse_ReadDataEnable();
	SystemClockInit();
	SysTickInit();
	//	Efuse_ReadDataEnable();
	Clock_DeepSleepSysClkSelect(PLL_CLK_MODE, FSHC_PLL_CLK_MODE, 0);

	if (Clock_CoreClockFreqGet() == 360 * 1000000)
	{
		SpiFlashInit(90000000, MODE_4BIT, 0, FSHC_PLL_CLK_MODE);
	}
	else if (Clock_CoreClockFreqGet() > 320 * 1000000)
	{
		SpiFlashInit(100000000, MODE_4BIT, 0, FSHC_APLL_CLK_MODE);
	}
	else
	{
		SpiFlashInit(96000000, MODE_4BIT, 0, FSHC_PLL_CLK_MODE);
	}

	LogUartConfig(TRUE); // ����ʱ�Ӻ����䴮��ǰ��Ҫ��ӡ��
						 // Clock_Pll5ClkDivSet(8);//
						 // B0B1ΪSW��ʽ�˿ڣ��ڵ��Խ׶���ϵͳ�����˵͹���ģʽʱ�ر���GPIO����ģʽ�����ڴ˴�����
						 // GPIO_PortBModeSet(GPIOB0, 0x3); //B0 sw_clk(i)
						 // GPIO_PortBModeSet(GPIOB1, 0x4); //B1 sw_d(io)
						 // APP_DBG("Main:wakeup\n");
}
#endif

extern const RTC_DATE_TIME ResetRtcVal;
#ifdef CFG_APP_BT_MODE_EN
extern uint32_t bt_em_size(void);
void bt_em_size_init(void)
{
	uint32_t bt_em_mem;
	app_bt_em_params_config();
	bt_em_mem = bt_em_size();
	if (bt_em_mem % 4096)
	{
		bt_em_mem = ((bt_em_mem / 4096) + 1) * 4096;
	}
	if (bt_em_mem > BB_EM_SIZE)
	{
		APP_DBG("bt em config error!\nyou must check BB_EM_SIZE\n%s%u \n", __FILE__, __LINE__);
		while (1)
			;
	}
	else
	{
		APP_DBG("bt em size:%uKB\n", (unsigned int)bt_em_mem / 1024);
	}
}
#endif


void start_up_grate(uint32_t UpdateResource);
void Ckeck_Flash_Boot()
{
	#ifdef GPIO_FLASH_BOOT
		GPIO_PortAModeSet(GPIO_FLASH_BOOT, 0x00);	
		GPIO_RegOneBitSet(GPIO_A_IE, GPIO_FLASH_BOOT);
		GPIO_RegOneBitSet(GPIO_A_PU, GPIO_FLASH_BOOT);
		GPIO_RegOneBitClear(GPIO_A_OE, GPIO_FLASH_BOOT);


		DelayMs(10);
		if(!GPIO_RegOneBitGet(GPIO_A_IN,GPIO_FLASH_BOOT))
		{
			APP_DBG("[Ckeck_Flash_Boot]   start_up_grate usb\n");
			GPIO_RegOneBitClear(GPIO_A_IE, LED_CPU);
			GPIO_RegOneBitSet(GPIO_A_OE, LED_CPU);
			GPIO_RegOneBitSet(GPIO_A_OUT, LED_CPU);
			start_up_grate(3);
		}else{
			APP_DBG("[Ckeck_Flash_Boot]   No update\n");
			GPIO_RegOneBitClear(GPIO_A_PU, GPIO_FLASH_BOOT);
			GPIO_RegOneBitClear(GPIO_A_IN, GPIO_FLASH_BOOT);
		}
	#endif
}

void set_GPIO()
{
	//========================== GPIO_OUT ====================================
	APP_DBG("[SystemInit]   SET GPIO_OUT\n");

	#ifdef TEST

		GPIO_RegOneBitClear(GPIO_A_IE, PCM_EN);
		GPIO_RegOneBitSet(GPIO_A_OE, PCM_EN);
		GPIO_RegOneBitSet(GPIO_A_OUT, PCM_EN);

		//GPIO_RegOneBitClear(GPIO_A_IE, GPIO_PCM_SCK);
		//GPIO_RegOneBitSet(GPIO_A_OE, GPIO_PCM_SCK);
		//GPIO_RegOneBitClear(GPIO_A_OUT, GPIO_PCM_SCK);

	#else

		#ifdef GPIO_VR_3V3

			#ifndef CFG_RES_AUDIO_I2SOUT_EN

				GPIO_PortBModeSet(GPIO_VR, 0x00);

				GPIO_RegOneBitClear(GPIO_B_IE, GPIO_VR);
				GPIO_RegOneBitSet(GPIO_B_OE, GPIO_VR);
				GPIO_RegOneBitSet(GPIO_B_OUT, GPIO_VR);
				GPIO_PortBOutDsSet(GPIO_VR, GPIO_PortB_OUTDS_34MA);

			#endif

		#endif

		#ifdef LED_OUT

			GPIO_PortAModeSet(LED_AUX_OUT, 0x00);
			GPIO_RegOneBitClear(GPIO_A_IE, LED_AUX_OUT);
			GPIO_RegOneBitSet(GPIO_A_OE, LED_AUX_OUT);
			GPIO_RegOneBitClear(GPIO_A_OUT, LED_AUX_OUT);

			GPIO_PortBModeSet(LED_BLE_OUT, 0x00);
			GPIO_RegOneBitClear(GPIO_B_IE, LED_BLE_OUT);
			GPIO_RegOneBitSet(GPIO_B_OE, LED_BLE_OUT);
			GPIO_RegOneBitClear(GPIO_B_OUT, LED_BLE_OUT);

			GPIO_PortBModeSet(LED_PC_OUT, 0x00);
			GPIO_RegOneBitClear(GPIO_B_IE, LED_PC_OUT);
			GPIO_RegOneBitSet(GPIO_B_OE, LED_PC_OUT);
			GPIO_RegOneBitClear(GPIO_B_OUT, LED_PC_OUT);
		#endif
	#endif

	GPIO_RegOneBitClear(GPIO_A_IE, LED_CPU);
	GPIO_RegOneBitSet(GPIO_A_OE, LED_CPU);

	GPIO_RegOneBitClear(GPIO_A_IE, LED_MODE);
	GPIO_RegOneBitSet(GPIO_A_OE, LED_MODE);

	GPIO_PortBModeSet(GPIO_MUTE, 0x00);
	GPIO_RegOneBitClear(STRING_CONNECT(GPIO, GPIO_MUTE_PORT, GPIE), GPIO_MUTE);
	GPIO_RegOneBitSet(STRING_CONNECT(GPIO, GPIO_MUTE_PORT, GPOE), GPIO_MUTE);
	GPIO_RegOneBitClear(STRING_CONNECT(GPIO, GPIO_MUTE_PORT, GPOUT), GPIO_MUTE);

	//========================== GPIO_IN ====================================
	APP_DBG("[SystemInit]   SET GPIO_IN\n");
}

int main(void)
{
	uint16_t RstFlag = 0;
	extern char __sdk_code_start;

	#ifdef LDOIN_3V3
		Chip_Init(0);
	#else
		Chip_Init(1);
	#endif

	WDG_Enable(WDG_STEP_4S);
	
	// WDG_Disable();
	// Clock_HOSCCurrentSet(0xF);

	// #if FLASH_BOOT_EN
	//	RstFlag = Reset_FlagGet_Flash_Boot();
	// #else
	RstFlag = Reset_FlagGet();
	Reset_FlagClear();
	// #endif

	BACKUP_NVMInit();
	Power_LDO12Config(1250);

	SystemClockInit();
	LogUartConfig(TRUE);

	DBUS_Access_Area_Init(0);
	Remap_DisableTcm();
	Remap_InitTcm(FLASH_ADDR, 8);

#ifdef FUNC_OS_DEBUG
	memcpy((void *)0x20000000, (void *)(0x40000), 20 * 1024);
	Remap_AddrRemapSet(ADDR_REMAP1, 0x40000, 0x20000000, 20);
#else
	memcpy((void *)0x20006000, (void *)(&__sdk_code_start), TCM_SIZE * 1024);
	Remap_AddrRemapSet(ADDR_REMAP1, (uint32_t)(&__sdk_code_start), 0x20006000, TCM_SIZE);
#endif

	if (Clock_CoreClockFreqGet() == 360 * 1000000)
	{
		SpiFlashInit(90000000, MODE_4BIT, 0, FSHC_PLL_CLK_MODE);
	}
	else if (Clock_CoreClockFreqGet() > 320 * 1000000)
	{
		SpiFlashInit(100000000, MODE_4BIT, 0, FSHC_APLL_CLK_MODE);
	}
	else
	{
		SpiFlashInit(96000000, MODE_4BIT, 0, FSHC_PLL_CLK_MODE);
	}
	Clock_RC32KClkDivSet(Clock_RcFreqGet(TRUE) / 32000);

	// SpiFlashIOCtrl(IOCTL_FLASH_PROTECT, FLASH_LOCK_RANGE_HALF);

	prvInitialiseHeap();
	osSemaphoreMutexCreate();

#ifdef CFG_SOFT_POWER_KEY_EN
	SoftPowerInit();
	WaitSoftKey();
#endif

	NVIC_EnableIRQ(SWI_IRQn);
	GIE_ENABLE();

	APP_DBG("\n");
	APP_DBG("****************************************************************\n");
	APP_DBG("|                    MVsilicon B1 SDK                          |\n");
	APP_DBG("|            Mountain View Silicon Technology Co.,Ltd.         |\n");
	APP_DBG("|            SDK Version: %d.%d.%d                                |\n", CFG_SDK_MAJOR_VERSION, CFG_SDK_MINOR_VERSION, CFG_SDK_PATCH_VERSION);
	APP_DBG("****************************************************************\n");
	APP_DBG("sys clk =%ld\n", Clock_SysClockFreqGet());

#ifdef CFG_APP_IDLE_MODE_EN
	IdleModeConfig();
#endif

	Ckeck_Flash_Boot();
	set_GPIO();

	flash_table_init();
	sys_parameter_init();

#ifdef CFG_APP_BT_MODE_EN
	bt_em_size_init();
#endif

#if FLASH_BOOT_EN
	report_up_grate();
#endif
	ResetFlagGet(RstFlag);

	APP_DBG("Audio Decoder Version: %s\n", (unsigned char *)audio_decoder_get_lib_version());
	APP_DBG("Driver Version: %s %x\n", GetLibVersionDriver(), Chip_Version());
	APP_DBG("BtLib Version: %s\n", (unsigned char *)GetLibVersionBt());
	APP_DBG("Fatfs presearch acc Lib Version: %s\n", (unsigned char *)GetLibVersionFatfsACC());
	APP_DBG("\n");

	RTC_SecGet();

#if !defined(CFG_FUNC_STRING_CONVERT_EN) && !defined(USE_DBG_CODE)
	pvApplicationIdleHook = vApplicationIdleHook;
	ptrace_TASK_SWITCHED_IN = trace_TASK_SWITCHED_IN;
	ptrace_TASK_SWITCHED_OUT = trace_TASK_SWITCHED_OUT;
#endif

#ifdef CFG_APP_BT_MODE_EN
	bt_api_init();
#ifdef BT_TWS_SUPPORT
	tws_time_init();
#else
	__nds32__mtsr(0, NDS32_SR_PFMC0);
	__nds32__mtsr(1, NDS32_SR_PFM_CTL);
#endif
#else
	__nds32__mtsr(0, NDS32_SR_PFMC0);
	__nds32__mtsr(1, NDS32_SR_PFM_CTL);
#endif

	uart_switch = 1;

	MainAppTaskStart();
	vTaskStartScheduler();

	while (1)
		;
}
