///////////////////////////////////////////////////////////////////////////////
//               Mountain View Silicon Tech. Inc.
//                       All rights reserved.
//               Filename: power_management.c
///////////////////////////////////////////////////////////////////////////////
#include "type.h"
#include "app_config.h"
#include "adc.h"
#include "clk.h"
#include "gpio.h"
#include "timeout.h"
#include "adc_key.h"
#include "debug.h"
#include "sadc_interface.h"
#include "delay.h"
#include "bt_config.h"
#include <string.h>
#include "main_task.h"

#ifdef CFG_FUNC_POWER_MONITOR_EN
#include "power_monitor.h"

#define LDOIN_POWER_OFF_CYCLE       30      //�͵�ػ����ʱ�� 15s
#define LDOIN_LOW_WARNING_CYCLE     120     //�͵�1���Ӳ���һ��
#define BAT_DET_VALUES_MAX          20      //10����ȥ������
#define BAT_DET_WIN_TIME            30      //��ص�����ⴰ�� 15S
#define BAT_DET_WIN_COUNT           10      //10�����ڹ۲죬150s�ȶ������ߵ���


#define LDOIN_SAMPLE_COUNT			30      //��ȡLDOIN����ʱ����ƽ���Ĳ�������
#define LDOIN_SAMPLE_PERIOD			50	    //��ȡLDOIN����ʱ��ȡ����ֵ�ļ��(ms)
#define LDOIN_DETECT_PERIOD			500	    //�ϲ��ȡ�����ȼ����(ms)


//���¶��岻ͬ�ĵ�ѹ����¼��Ĵ�����ѹ(��λmV)���û���������ϵͳ��ص��ص�������
#define LDOIN_VOLTAGE_FULL			4150    //�����ѹ
#define LDOIN_VOLTAGE_9			    4100
#define LDOIN_VOLTAGE_8			    4000
#define LDOIN_VOLTAGE_7			    3900
#define LDOIN_VOLTAGE_6			    3800
#define LDOIN_VOLTAGE_5			    3740
#define LDOIN_VOLTAGE_4			    3680
#define LDOIN_VOLTAGE_3			    3600
#define LDOIN_VOLTAGE_LOW			3500    //���ڴ˵�ѹֵ��ʾ�͵��ѹ
#define LDOIN_VOLTAGE_OFF			3350	//���ڴ˵�ѹֵ�ػ�

#ifdef BAT_VOL_DET_LRADC
const uint16_t LRADC_GradeVol[PWR_FULL + 1] =
{
    0,                 //���� 0 -- �͵�ػ�
    LDOIN_VOLTAGE_OFF, //���� 1 -- �͵籨��
    LDOIN_VOLTAGE_LOW, //���� 2
    LDOIN_VOLTAGE_3,   //���� 3
    LDOIN_VOLTAGE_4,   //���� 4
    LDOIN_VOLTAGE_5,   //���� 5
    LDOIN_VOLTAGE_6,   //���� 6
    LDOIN_VOLTAGE_7,   //���� 7
    LDOIN_VOLTAGE_8,   //���� 8
    LDOIN_VOLTAGE_9,   //���� 9
    LDOIN_VOLTAGE_FULL,//���� 10 -- �����ѹ
};
#else
const uint16_t LdoinGradeVol[PWR_FULL + 1] =
{
	0,                 //���� 0 -- �͵�ػ�
	LDOIN_VOLTAGE_OFF, //���� 1 -- �͵籨��
	LDOIN_VOLTAGE_LOW, //���� 2
	LDOIN_VOLTAGE_3,   //���� 3
	LDOIN_VOLTAGE_4,   //���� 4
	LDOIN_VOLTAGE_5,   //���� 5
	LDOIN_VOLTAGE_6,   //���� 6
	LDOIN_VOLTAGE_7,   //���� 7
	LDOIN_VOLTAGE_8,   //���� 8
	LDOIN_VOLTAGE_9,   //���� 9
	LDOIN_VOLTAGE_FULL,//���� 10 -- �����ѹ
};
#endif

//���ڵ�ѹ���ı���
TIMER PowerMonitorTimer;
TIMER PowerMonitorDetectTimer;

uint16_t LdoinSampleCnt;            //LDOIN_SAMPLE_COUNT;
uint16_t LdoinLevelAverage;		    //��ǰLDOIN��ѹƽ��ֵ
uint16_t bat_low_conuter;           //�͵籨����ʱ��
uint16_t bat_low_power_off_conuter; //�͵�ػ���ʱ��

uint16_t bat_value_win_min[BAT_DET_WIN_COUNT]; //ÿ15��һ������ͳ����Сֵ
uint16_t bat_values[BAT_DET_VALUES_MAX];
uint16_t LdoinSampleVal[LDOIN_SAMPLE_COUNT];

uint16_t bat_value_index;
uint16_t bat_value_cur_win_min;  //��ǰ������С����ֵ
uint16_t bat_value_win_tick;     //��ⴰ�ڼ�ʱ
uint16_t bat_value_win_count;    //�Ѿ�ͳ�ƴ�����
uint16_t bat_value_counter;      //��ص�ѹ���ͼ�ʱ��

static PWR_LEVEL PowerLevel = PWR_FULL;  //��ǰϵͳ����ֵ

#ifdef	CFG_FUNC_OPTION_CHARGER_DETECT
//Ӳ�����PC ����������״̬
//ʹ���ڲ���������PC����������ʱ������Ϊ�͵�ƽ����ʱ����Ϊ�ߵ�ƽ
bool IsInCharge(void)
{
	//��Ϊ���룬��������
	GPIO_PortAModeSet(CHARGE_DETECT_GPIO, 0x0);
	GPIO_RegOneBitSet(CHARGE_DETECT_PORT_PU, CHARGE_DETECT_GPIO);
	GPIO_RegOneBitClear(CHARGE_DETECT_PORT_PD, CHARGE_DETECT_GPIO);
	GPIO_RegOneBitClear(CHARGE_DETECT_PORT_OE, CHARGE_DETECT_GPIO);
	GPIO_RegOneBitSet(CHARGE_DETECT_PORT_IE, CHARGE_DETECT_GPIO);
	WaitUs(2);
	if(GPIO_RegOneBitGet(CHARGE_DETECT_PORT_IN,CHARGE_DETECT_GPIO))
	{
		return TRUE;
	}   	

	return FALSE;
}
#endif

//������ʼ��
void battery_detect_var_init(void)
{
	uint8_t i;
	
	bat_value_index = 0;
	bat_value_cur_win_min = PWR_FULL;
	bat_value_win_tick = 0;
	bat_value_win_count = 0;
	bat_value_counter = 0;

	LdoinSampleCnt = 0;
	LdoinLevelAverage = 0;
	bat_low_conuter = 0;
	bat_low_power_off_conuter = 0;

	TimeOutSet(&PowerMonitorTimer, 0);	
	TimeOutSet(&PowerMonitorDetectTimer, 0);	

	for(i=0; i<BAT_DET_VALUES_MAX; i++)
	{
		bat_values[i] = PWR_FULL;
	}
}


//��ص�ѹ����
void PowerVoltageSampling(void)
{
	uint16_t bat_vol;
	
	if(IsTimeOut(&PowerMonitorTimer))
	{
		TimeOutSet(&PowerMonitorTimer, LDOIN_SAMPLE_PERIOD);

		#ifdef BAT_VOL_DET_LRADC
		bat_vol = ADC_SingleModeDataGet(BAT_VOL_LRADC_CHANNEL_PORT);
		#else
		bat_vol = SarADC_LDOINVolGet();
		#endif
		
		//����ֵС��1V����Ϊ��ǰ������󣬷�������
		if(bat_vol < 1000)
		{
			bat_vol = LDOIN_VOLTAGE_FULL;
		}
		
		if(LdoinSampleCnt >= LDOIN_SAMPLE_COUNT)
		{
			LdoinSampleCnt = 0;
		}
		
		//��ص�ѹ����
		LdoinSampleVal[LdoinSampleCnt] = bat_vol;
		LdoinSampleCnt++;
	}
}

//��ȡ�����ȼ�
uint16_t GetPowerVoltage(void)
{
    uint32_t bat_val;
    int16_t i;

    bat_val = 0;
    for (i = 0; i < LDOIN_SAMPLE_COUNT; i++)
    {
        bat_val += LdoinSampleVal[i];
    }	

	LdoinLevelAverage = (uint16_t)(bat_val / LDOIN_SAMPLE_COUNT);
	APP_DBG("LDOin 5V Volt: %lu\n", (uint32_t)LdoinLevelAverage);

	if (LdoinLevelAverage > LDOIN_VOLTAGE_FULL)
	{
		return PWR_FULL;
	}
	else
	{
	    for (i = PWR_FULL; i >= 0; i--)
	    {
	    	#ifdef BAT_VOL_DET_LRADC
	        if (LdoinLevelAverage >= LRADC_GradeVol[i])
			#else
			if (LdoinLevelAverage >= LdoinGradeVol[i])
			#endif
	        {
	            return i;
	        }
	    }
	}

	return 0;
}


//��������
PWR_LEVEL ShakeEliminationProcessing(void)
{
	uint16_t i;
	uint16_t bat_value;
	uint16_t last_bat_value;
	uint16_t tmp_bat_value_min;
	uint16_t tmp_bat_value_min_wins;
	uint16_t bat_value_min_cn;
	bool bat_det_stable_flag;
	
	last_bat_value = PowerLevel;
	
	bat_value = GetPowerVoltage();
	if (bat_value > PWR_FULL)
	{
		bat_value = PWR_FULL;
	}

	//����Ϊ0����ٽ���
	if(bat_value == 0)
	{
		if (PowerLevel > 0)
		{
			PowerLevel--;
		}
		DBG("bat_value down: %d -> %d\n", last_bat_value, PowerLevel);
		bat_value_counter = 0;
	}
	else
	{
        bat_values[bat_value_index] = bat_value;
        bat_value_index++;
        if (bat_value_index >= BAT_DET_VALUES_MAX)	
        {
			bat_value_index = 0;
		}
		
        tmp_bat_value_min = PWR_FULL;
		//���������С����ֵ
        for (i = 0; i < BAT_DET_VALUES_MAX; i++)
        {
            if (bat_values[i] < tmp_bat_value_min)
            {
                tmp_bat_value_min = bat_values[i];
            }
        }		

        //ͳ�ƴ���
        if (tmp_bat_value_min < bat_value_cur_win_min)
        {
            bat_value_cur_win_min = tmp_bat_value_min;
        }		

        bat_value_win_tick++;
        if (bat_value_win_tick >= BAT_DET_WIN_TIME)
        {
        	//ͳ��10�����ڹ۲�
    		if (bat_value_win_count >= BAT_DET_WIN_COUNT)
			{
				bat_value_win_count = 0;
			}
            if (bat_value_win_count < BAT_DET_WIN_COUNT)
            {
                bat_value_win_min[bat_value_win_count] = bat_value_cur_win_min;
                bat_value_win_count++;
            }
			
            bat_value_win_tick = 0;
			bat_value_cur_win_min = PWR_FULL;
        }

		//��ѹ�½�
		if (tmp_bat_value_min < PowerLevel)
		{
            bat_det_stable_flag = TRUE;
			bat_value_min_cn = 0;
            for (i = 0; i < BAT_DET_VALUES_MAX; i++)
            {
            	//���20�ε���С��ϵͳ��������
            	if (PowerLevel > bat_values[i])
            	{
					bat_value_min_cn++;
				}
				//ƫ���1���������ȶ�
                if ((bat_values[i] - tmp_bat_value_min) > 1)
                {
                    bat_det_stable_flag = FALSE;
                }
            }		

			bat_value_counter++;
			//�����ѹ���ȶ��������20����10�ε�ѹС��ϵͳ�����������Ͻ��͵�ѹ
			if ((bat_det_stable_flag == TRUE)&&(bat_value_min_cn >= 10))
			{
				bat_value_counter = 0;
				if (PowerLevel > tmp_bat_value_min)
				{
					PowerLevel--;
				}
				DBG("bat_value down1: %d -> %d\n", last_bat_value, PowerLevel);
			}
			else if (bat_value_counter >= BAT_DET_WIN_TIME) //�����ѹ���ȶ�������Ҫ����15���Ӳ�������͵�������ֹ����
			{
				if (PowerLevel > tmp_bat_value_min)
				{
					PowerLevel--;
					DBG("bat_value down2: %d -> %d\n", last_bat_value, PowerLevel);
				}

				bat_value_counter = 0;
			}
		}
		else if (tmp_bat_value_min > PowerLevel)
		{
			//��Ҫ����120s���������ߵ�������ֹ����
			if ((bat_value_win_tick == 0)&&(bat_value_win_count == BAT_DET_WIN_COUNT))
			{
                tmp_bat_value_min_wins = PWR_FULL;
                for (i = 0; i < BAT_DET_WIN_COUNT; i++)
                {
                    if (bat_value_win_min[i] < tmp_bat_value_min_wins)
                    {
                        tmp_bat_value_min_wins = bat_value_win_min[i];
                    }
                }	
				if (tmp_bat_value_min_wins > PowerLevel)
				{
					PowerLevel++;
                    DBG("bat_value up: %d -> %d\n", last_bat_value, PowerLevel);
				}
			}
		}
	}

	return PowerLevel;
}


//��ص���
void BatteryScan(void)
{
	if(IsTimeOut(&PowerMonitorDetectTimer))
	{
		TimeOutSet(&PowerMonitorDetectTimer, LDOIN_DETECT_PERIOD);	
		
		//��ȡ����������ĵ����ȼ�
		PowerLevel = ShakeEliminationProcessing();
		//DBG("PowerLevel = %d\n", PowerLevel);
		
#ifdef CFG_FUNC_OPTION_CHARGER_DETECT
		//����豸�Ѿ����룬�Ͳ�ִ��������ε͵�ѹ���ʹ������
		if(IsInCharge()) 
		{
			bat_low_conuter = 0;
			bat_low_power_off_conuter = 0;
		}
		else
#endif
		{
			if (PowerLevel == PWR_LEVEL_0)
			{
				bat_low_power_off_conuter++;
				//�ػ����15s
				if (bat_low_power_off_conuter == LDOIN_POWER_OFF_CYCLE)
				{
					//�ȱ��͵���ʾ��
					BatteryLowMessage();
				}
				else if (bat_low_power_off_conuter >= (LDOIN_POWER_OFF_CYCLE+6))
				{
					bat_low_power_off_conuter = 0;
					//��ʱ3s�ٹػ�
					PowerOffMessage();
				}
			}
			else if (PowerLevel == PWR_LEVEL_1)
			{
				bat_low_conuter++;
				//�͵籨��
				if (bat_low_conuter > LDOIN_LOW_WARNING_CYCLE)
				{
					bat_low_conuter = 0;
					BatteryLowMessage();
				}
			}
			else
			{
				bat_low_conuter = 0;
				bat_low_power_off_conuter = 0;
			}
		}

		//�ϴ���ص������ֻ���
#ifdef CFG_APP_BT_MODE_EN
#if (BT_HFP_SUPPORT == ENABLE)
		SetBtHfpBatteryLevel(PowerLevelGet(), 0);
#endif
#endif		
		//������ʾ
		PowerMonitorDisp();
	}
}

//���ܼ��ӳ�ʼ��
//ʵ��ϵͳ���������еĵ͵�ѹ��⴦���Լ���ʼ������豸������IO��
void PowerMonitorInit(void)
{
	uint32_t bat_vol;
	uint8_t i;

	i = 0;
	bat_vol = 0;
	battery_detect_var_init();

	//init
	#ifdef BAT_VOL_DET_LRADC
	GPIO_RegOneBitSet(BAT_VOL_LRADC_CHANNEL_ANA_EN, BAT_VOL_LRADC_CHANNEL_ANA_MASK);
	#endif

	//ϵͳ���������еĵ͵�ѹ���
	//����ʱ��ѹ��⣬���С�ڿ�����ѹ���������豸���Ͳ������̣�ֱ�ӹػ�
	//������Ϊʱ50ms������ϵͳ�������г�ʼ��
	while(i < 10)
	{
		#ifdef BAT_VOL_DET_LRADC
		bat_vol += ADC_SingleModeDataGet(BAT_VOL_LRADC_CHANNEL_PORT);
		#else
		bat_vol += SarADC_LDOINVolGet();
		#endif
		i++;
		vTaskDelay(5);
	}

	LdoinLevelAverage = (uint16_t)(bat_vol/10);
	for(i=0; i<LDOIN_SAMPLE_COUNT; i++)
	{
		LdoinSampleVal[i] = LdoinLevelAverage;
	}

	PowerLevel = GetPowerVoltage();
#ifdef CFG_FUNC_OPTION_CHARGER_DETECT
	if(!IsInCharge()) //���ϵͳ����ʱ������豸�Ѿ����룬����ʾ�͵���ػ�
#endif
	{
		if (PowerLevel == PWR_LEVEL_1)
		{
			//�����͵磬5s�󱨾�
			bat_low_conuter = LDOIN_LOW_WARNING_CYCLE - 10;
		}
		else if (PowerLevel == PWR_LEVEL_0)
		{
			//�����͵磬���Ϲػ�
			bat_low_power_off_conuter = LDOIN_POWER_OFF_CYCLE - 1;
		}
	}

	//������ʾ
	PowerMonitorDisp();
}


//��ȡ��ǰ��ص���,�������ϴ��������ֻ���
//return: level(0-9)
PWR_LEVEL PowerLevelGet(void)
{
	PWR_LEVEL res = 0;

	//��Ӧ�ֻ���0% ~ 100%�������ϴ��ֻ�ʱ�Լ�һ������
	if (PowerLevel > 0)
	{
		res = PowerLevel - 1;
	}
	return res;
}

//������ʾ����
void PowerMonitorDisp(void)
{
#if (defined(FUNC_SEG_LED_EN) || defined(FUNC_SEG_LCD_EN) ||defined(FUNC_TM1628_LED_EN))
	static bool IsToShow = FALSE;

#ifdef CFG_FUNC_OPTION_CHARGER_DETECT
	static uint8_t  ShowStep = 0;
	if(IsInCharge())      //������Ѿ�����Ĵ���
	{
		switch(ShowStep)
		{
			case 0:
			DispIcon(ICON_BAT1, FALSE);
			DispIcon(ICON_BAT2, FALSE);
			DispIcon(ICON_BAT3, FALSE);
			break;
			case 1:
			DispIcon(ICON_BAT1, TRUE);
			DispIcon(ICON_BAT2, FALSE);
			DispIcon(ICON_BAT3, FALSE);
			break;
			case 2:
			DispIcon(ICON_BAT1, TRUE);
			DispIcon(ICON_BAT2, TRUE);
			DispIcon(ICON_BAT3, FALSE);
			break;
			case 3:
			DispIcon(ICON_BAT1, TRUE);
			DispIcon(ICON_BAT2, TRUE);
			DispIcon(ICON_BAT3, TRUE);
			break;
		}		
		
		ShowStep++;
		if(ShowStep > 3)
		{
			ShowStep = 0;
		}
	}
	else
#endif
	{
		switch(PowerLevel)
		{
			case PWR_FULL
			case PWR_LEVEL_9:
			DispIcon(ICON_BATFUL, TRUE);
			DispIcon(ICON_BATHAF, FALSE);
			//������ʾ����������������ʾ����			
			break;

			case PWR_LEVEL_8:
			case PWR_LEVEL_7:
			case PWR_LEVEL_6:
			DispIcon(ICON_BATFUL, FALSE);
			DispIcon(ICON_BATHAF, TRUE);
			//������ʾ3��������������ʾ����			
			break;

			case PWR_LEVEL_5:
			case PWR_LEVEL_4:
			DispIcon(ICON_BATFUL, FALSE);
			DispIcon(ICON_BATHAF, TRUE);
			//������ʾ2��������������ʾ����	
			break;

			case PWR_LEVEL_3:
			case PWR_LEVEL_2:
			DispIcon(ICON_BATFUL, FALSE);
			DispIcon(ICON_BATHAF, TRUE);
			//������ʾ1��������������ʾ����			
			break;

			case PWR_LEVEL_1:
			case PWR_LEVEL_0:
			DispIcon(ICON_BATFUL, FALSE);
			if(IsToShow)
			{
				DispIcon(ICON_BATHAF, TRUE);
			}
			else
			{
				DispIcon(ICON_BATHAF, FALSE);
			}
			IsToShow = !IsToShow;
			//������ʾ0��������������ʾ����	
			break;
			
			default:
			break;
		}
	}
#endif
}

#endif
