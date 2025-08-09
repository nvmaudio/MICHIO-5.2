
#include "app_message.h"
#include "main_task.h"
#include "debug.h"
#include "bt_manager.h"
#include "bt_app_tws.h"

#include "audio_vol.h"

//#include "app_config.h"


bool MUTE_CS = TRUE;

bool APP_MUTE = TRUE;
bool REMIND_MUTE = TRUE;


bool get_APP_MUTE(void) {
    return APP_MUTE;
}

void MUTE()
{
	if (!MUTE_CS)
	{
		MUTE_CS = TRUE;
		GPIO_RegOneBitClear(STRING_CONNECT(GPIO, GPIO_MUTE_PORT, GPOUT), GPIO_MUTE); // low
	}
}
void UN_MUTE()
{
	if (MUTE_CS)
	{
		MUTE_CS = FALSE;
		GPIO_RegOneBitSet(STRING_CONNECT(GPIO, GPIO_MUTE_PORT, GPOUT), GPIO_MUTE); // high
	}

	if (IsAudioPlayerMute() == TRUE)
	{
		HardWareMuteOrUnMute();
	}
}

void MUTE_ON_OFF()
{
	if (MUTE_CS)
	{
		UN_MUTE();
	}
	else
	{
		MUTE();
	}
}

void REMIND_CMD(bool Mute)
{
	if (REMIND_MUTE != Mute)
	{
		REMIND_MUTE = Mute;
		if (!Mute)
		{
			UN_MUTE();
		}
		if (Mute && APP_MUTE)
		{
			MUTE();
		}
	}
}

void APP_MUTE_CMD(bool Mute)
{
	if (APP_MUTE != Mute)
	{
		APP_MUTE = Mute;
		if (!Mute)
		{
			UN_MUTE();
			tws_master_mute_send(FALSE);
		}
		else
		{
			tws_master_mute_send(TRUE);
		}
		if (Mute && REMIND_MUTE)
		{
			MUTE();
		}
		APP_DBG("[APP_MUTE_CMD] APP_MUTE %d \n", APP_MUTE);
	}
}

static void SetLed(GPIO_INDEX LED_GPIO, bool ON_OFF)
{
	if (ON_OFF)
	{
		GPIO_RegOneBitSet(GPIO_A_OUT, LED_GPIO);
	}
	else
	{
		GPIO_RegOneBitClear(GPIO_A_OUT, LED_GPIO);
	}
}

static void ToggleLED(GPIO_INDEX LED_GPIO)
{
	if (GPIO_RegOneBitGet(GPIO_A_OUT, LED_GPIO))
	{
		SetLed(LED_GPIO, FALSE);
	}
	else
	{
		SetLed(LED_GPIO, TRUE);
	}
}

#ifdef LED_OUT
	static void SetLed_B(GPIO_INDEX LED_GPIO, bool ON_OFF)
	{
		if (ON_OFF)
		{
			GPIO_RegOneBitSet(GPIO_B_OUT, LED_GPIO);
		}
		else
		{
			GPIO_RegOneBitClear(GPIO_B_OUT, LED_GPIO);
		}
	}

	static void ToggleLED_B(GPIO_INDEX LED_GPIO)
	{
		if (GPIO_RegOneBitGet(GPIO_B_OUT, LED_GPIO))
		{
			SetLed_B(LED_GPIO, FALSE);
		}
		else
		{
			SetLed_B(LED_GPIO, TRUE);
		}
	}
#endif

extern BT_MANAGER_ST btManager;

TIMER led;

void Michio(void)
{

	if (IsTimeOut(&led))
	{

		if (MUTE_CS)
		{
			SetLed(LED_CPU, TRUE);
		}
		else
		{
			ToggleLED(LED_CPU);
		}

		#ifdef LED_OUT
				if (mainAppCt.SysCurrentMode == ModeLineAudioPlay)
				{
					GPIO_RegOneBitSet(GPIO_A_OUT, LED_AUX_OUT);
				}

				if (mainAppCt.SysCurrentMode == ModeUsbDevicePlay)
				{
					GPIO_RegOneBitSet(GPIO_B_OUT, LED_PC_OUT);
				}
		#endif

		if (mainAppCt.SysCurrentMode == ModeBtAudioPlay || mainAppCt.SysCurrentMode == ModeTwsSlavePlay)
		{
			#ifdef LED_OUT
				GPIO_RegOneBitClear(GPIO_B_OUT, LED_PC_OUT);
				GPIO_RegOneBitClear(GPIO_A_OUT, LED_AUX_OUT);
			#endif
#ifdef BT_TWS_SUPPORT
			if (btManager.twsRole == BT_TWS_SLAVE && btManager.twsState == BT_TWS_STATE_CONNECTED)
			{
				SetLed(LED_MODE, TRUE);
				#ifdef LED_OUT
					SetLed_B(LED_BLE_OUT, TRUE);
				#endif
			}
			else
#endif
			{
				if (!btManager.btLinkState)
				{
					if (MUTE_CS)
					{
						ToggleLED(LED_MODE);
						#ifdef LED_OUT
							ToggleLED_B(LED_BLE_OUT);
						#endif
					}
					else
					{
						if (GPIO_RegOneBitGet(GPIO_A_OUT, LED_CPU))
						{
							SetLed(LED_MODE, TRUE);
							#ifdef LED_OUT
								SetLed_B(LED_BLE_OUT, TRUE);
							#endif	
						}
						else
						{
							SetLed(LED_MODE, FALSE);
							#ifdef LED_OUT
								SetLed_B(LED_BLE_OUT, FALSE);
							#endif
						}
					}
				}
				else
				{
					SetLed(LED_MODE, TRUE);
					#ifdef LED_OUT
						SetLed_B(LED_BLE_OUT, TRUE);
					#endif
				}
			}

#ifdef BT_TWS_SUPPORT
			if (btManager.twsState == BT_TWS_STATE_CONNECTED)
			{
				TimeOutSet(&led, 250);
			}
			else
#endif
			{
				TimeOutSet(&led, 500);
			}
		}
		else
		{
			SetLed(LED_MODE, FALSE);
			#ifdef LED_OUT
				SetLed_B(LED_BLE_OUT, FALSE);
			#endif
			TimeOutSet(&led, 1000);
		}
	}
}
