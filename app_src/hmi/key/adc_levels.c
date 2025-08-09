/**
 *************************************************************************************
 * @file	adc_levels.c
 * @author	ken bu/bkd
 * @version	v0.0.1
 * @date    2019/04/24
 * @brief	 for  Sliding rheostat
 * @ maintainer:
 * Copyright (C) Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 *************************************************************************************
 */

#include "app_config.h"
#include "app_message.h"

#ifdef CFG_ADC_LEVEL_KEY_EN
#include "adc_levels.h"
#include "adc.h"
// #include "config.h"
#include "timeout.h"
#include "gpio.h"
#include "debug.h"
#include "key.h"

const uint32_t ADC_CHANNEL_Init_Tab[14 * 2] =
	{

		GPIO_A_ANA_EN, GPIO_INDEX20, /**channel 0*/

		GPIO_A_ANA_EN, GPIO_INDEX21, /**channel 1*/

		GPIO_A_ANA_EN, GPIO_INDEX22, /**channel 2*/

		GPIO_A_ANA_EN, GPIO_INDEX23, /**channel 3*/

		GPIO_A_ANA_EN, GPIO_INDEX24, /**channel 4*/

		GPIO_A_ANA_EN, GPIO_INDEX25, /**channel 5*/

		GPIO_A_ANA_EN, GPIO_INDEX26, /**channel 6*/

		GPIO_A_ANA_EN, GPIO_INDEX27, /**channel 7*/

		GPIO_A_ANA_EN, GPIO_INDEX28, /**channel 8*/

		GPIO_A_ANA_EN, GPIO_INDEX29, /**channel 9*/

		GPIO_A_ANA_EN, GPIO_INDEX30, /**channel 10*/

		GPIO_A_ANA_EN, GPIO_INDEX31, /**channel 11*/

		GPIO_B_ANA_EN, GPIO_INDEX0, /**channel 12*/

		GPIO_B_ANA_EN, GPIO_INDEX1, /**channel 13*/

};

const uint32_t ADC_CHANNEL_Select_Tab[14] =
	{
		ADC_GPIOA20,
		ADC_GPIOA21,
		ADC_GPIOA22,
		ADC_GPIOA23,
		ADC_GPIOA24,
		ADC_GPIOA25,
		ADC_GPIOA26,
		ADC_GPIOA27,
		ADC_GPIOA28,
		ADC_GPIOA29,
		ADC_GPIOA30,
		ADC_GPIOA31,
		ADC_GPIOB0,
		ADC_GPIOB1,
};

static uint8_t ADCLevelChannelTotal = 0;

static uint8_t ADCLevelsScanCount = 0;
static uint32_t ADCLevelsChannel[14] = {0};
static uint8_t repeat_count[5];
static uint8_t ADCLevels_STEP_Store[14] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

static uint16_t ADCLevels_Msg_Ch[14] = {MSG_ADC_LEVEL_CH1, MSG_ADC_LEVEL_CH2, MSG_ADC_LEVEL_CH3, MSG_ADC_LEVEL_CH4, MSG_ADC_LEVEL_CH5, MSG_ADC_LEVEL_CH6, MSG_ADC_LEVEL_CH7, MSG_ADC_LEVEL_CH8, MSG_ADC_LEVEL_CH9, MSG_ADC_LEVEL_CH10, MSG_ADC_LEVEL_CH11};

uint16_t GetAdcVal(uint32_t adc_ch)
{
	uint16_t Val = 0x00;

	if (adc_ch == ADC_GPIOA20)
	{
		GPIO_RegOneBitSet(GPIO_A_ANA_EN, GPIO_INDEX20);
		Val = ADC_SingleModeDataGet(ADC_CHANNEL_GPIOA20_A23);
		GPIO_RegOneBitClear(GPIO_A_ANA_EN, GPIO_INDEX20);
		GPIO_RegOneBitClear(GPIO_A_ANA_EN, GPIO_INDEX23);
	}
	else if (adc_ch == ADC_GPIOA21)
	{
		GPIO_RegOneBitSet(GPIO_A_ANA_EN, GPIO_INDEX21);
		Val = ADC_SingleModeDataGet(ADC_CHANNEL_GPIOA21_A24);
		GPIO_RegOneBitClear(GPIO_A_ANA_EN, GPIO_INDEX21);
		GPIO_RegOneBitClear(GPIO_A_ANA_EN, GPIO_INDEX24);
	}
	else if (adc_ch == ADC_GPIOA22)
	{
		GPIO_RegOneBitSet(GPIO_A_ANA_EN, GPIO_INDEX22);
		Val = ADC_SingleModeDataGet(ADC_CHANNEL_GPIOA22_A25);
		GPIO_RegOneBitClear(GPIO_A_ANA_EN, GPIO_INDEX22);
		GPIO_RegOneBitClear(GPIO_A_ANA_EN, GPIO_INDEX25);
	}
	else if (adc_ch == ADC_GPIOA23)
	{
		GPIO_RegOneBitSet(GPIO_A_ANA_EN, GPIO_INDEX23);
		Val = ADC_SingleModeDataGet(ADC_CHANNEL_GPIOA20_A23);
		GPIO_RegOneBitClear(GPIO_A_ANA_EN, GPIO_INDEX20);
		GPIO_RegOneBitClear(GPIO_A_ANA_EN, GPIO_INDEX23);
	}
	else if (adc_ch == ADC_GPIOA24)
	{
		GPIO_RegOneBitSet(GPIO_A_ANA_EN, GPIO_INDEX24);
		Val = ADC_SingleModeDataGet(ADC_CHANNEL_GPIOA21_A24);
		GPIO_RegOneBitClear(GPIO_A_ANA_EN, GPIO_INDEX21);
		GPIO_RegOneBitClear(GPIO_A_ANA_EN, GPIO_INDEX24);
	}
	else if (adc_ch == ADC_GPIOA25)
	{
		Val = ADC_SingleModeDataGet(ADC_CHANNEL_GPIOA22_A25);
	}
	else if (adc_ch == ADC_GPIOA26)
	{
		Val = ADC_SingleModeDataGet(ADC_CHANNEL_GPIOA26);
	}
	else if (adc_ch == ADC_GPIOA27)
	{
		Val = ADC_SingleModeDataGet(ADC_CHANNEL_GPIOA27);
	}
	else if (adc_ch == ADC_GPIOA28)
	{
		Val = ADC_SingleModeDataGet(ADC_CHANNEL_GPIOA28);
	}
	else if (adc_ch == ADC_GPIOA29)
	{
		Val = ADC_SingleModeDataGet(ADC_CHANNEL_GPIOA29);
	}
	else if (adc_ch == ADC_GPIOA30)
	{
		Val = ADC_SingleModeDataGet(ADC_CHANNEL_GPIOA30);
	}
	else if (adc_ch == ADC_GPIOA31)
	{
		Val = ADC_SingleModeDataGet(ADC_CHANNEL_GPIOA31);
	}
	else if (adc_ch == ADC_GPIOB0)
	{
		Val = ADC_SingleModeDataGet(ADC_CHANNEL_GPIOB0);
	}
	else if (adc_ch == ADC_GPIOB1)
	{
		Val = ADC_SingleModeDataGet(ADC_CHANNEL_GPIOB1);
	}
	else
	{
		Val = 0x00;
	}
	return Val;
}

void ADCLevelsKeyInit(void)
{
	uint8_t k;
	uint32_t adc_ch;

	ADCLevelChannelTotal = 0;
	adc_ch = ADCLEVL_CHANNEL_MAP;

	for (k = 0; k < (sizeof(ADC_CHANNEL_Select_Tab) / sizeof(ADC_CHANNEL_Select_Tab[0])); k++)
	{
		if (adc_ch & ADC_CHANNEL_Select_Tab[k])
		{
			GPIO_RegOneBitSet(ADC_CHANNEL_Init_Tab[k * 2], ADC_CHANNEL_Init_Tab[k * 2 + 1]);
			ADCLevelsChannel[ADCLevelChannelTotal] = ADC_CHANNEL_Select_Tab[k];
			ADCLevelChannelTotal++;
		}
	}
 	APP_DBG("ADCLevelsKeyInit  oke\n");
}

#define MAX_ADC_VAL 4095
#define MAX_STEP_NUMBER 33

#define STEP_WIDTH ((MAX_ADC_VAL + 1) / MAX_STEP_NUMBER)

#define HYSTERESIS_VAL 30

#define ABS(x) ((x) < 0 ? (-(x)) : (x))

#define SAMPLE_INTERVAL_MS 10
#define SAMPLE_COUNT (1000 / SAMPLE_INTERVAL_MS)

static uint16_t adc_buffer[SAMPLE_COUNT];
static uint8_t buffer_index = 0;

void DBG_Noise(uint16_t Val)
{
	adc_buffer[buffer_index++] = Val;

	if (buffer_index >= SAMPLE_COUNT)
	{
		buffer_index = 0;
		uint16_t min = adc_buffer[0];
		uint16_t max = adc_buffer[0];

		for (int i = 1; i < SAMPLE_COUNT; i++)
		{
			if (adc_buffer[i] < min)
			{
				min = adc_buffer[i];
			}

			if (adc_buffer[i] > max)
			{
				max = adc_buffer[i];
			}
		}

		uint16_t noise = max - min;
		APP_DBG("[AdcLevelKeyProcess] Noise = %d (min = %d, max = %d)\n", noise, min, max);
	}
}

uint16_t AdcLevelKeyProcess(void)
{
	uint16_t Val;
	uint8_t i_count, ch;

	if (ADCLevelChannelTotal == 0)
	{
		return MSG_NONE;
	}

	if (ADCLevelsScanCount >= ADCLevelChannelTotal)
	{
		ADCLevelsScanCount = 0;
	}

	ch = ADCLevelsScanCount;

	Val = GetAdcVal(ADCLevelsChannel[ch]);

	if (ch == 0)
	{
		//DBG_Noise(Val);
	} 

	i_count = Val / STEP_WIDTH;

	if(i_count > 32)
	{
		i_count = 32;
	}

	uint8_t prev_level = ADCLevels_STEP_Store[ch];
	uint16_t level_center = prev_level * STEP_WIDTH + STEP_WIDTH / 2;

	ADCLevelsScanCount++; // Tăng sau khi xử lý xong

	// Bộ lọc chống nhiễu bằng hysteresis
	if (ABS((int)Val - (int)level_center) < HYSTERESIS_VAL)
	{
		repeat_count[ch] = 0;
		return MSG_NONE;
	}

	// Nếu có thay đổi mức thì đợi ổn định vài lần đọc trước khi chấp nhận
	if (i_count != prev_level)
	{
		repeat_count[ch]++;

		if (repeat_count[ch] > 3)
		{
			ADCLevels_STEP_Store[ch] = i_count;
			repeat_count[ch] = 0;
			return (ADCLevels_Msg_Ch[ch] + i_count);
		}
	}
	else
	{
		repeat_count[ch] = 0;
	}

	return MSG_NONE;
}


#include "main_task.h"

void load_vr(void)
{
	uint16_t Val;
	uint8_t i_count;

	#ifdef TEST
		//GPIO_RegOneBitSet(GPIO_A_ANA_EN,GPIO_INDEX28);
		//Val = GetAdcVal(ADC_GPIOA28);
		GPIO_RegOneBitSet(GPIO_A_ANA_EN,GPIO_INDEX20);
		Val = GetAdcVal(ADC_GPIOA20);

	#else
		GPIO_RegOneBitSet(GPIO_A_ANA_EN,GPIO_INDEX20);
		Val = GetAdcVal(ADC_GPIOA20);
	#endif

	i_count = Val / STEP_WIDTH;
	mainAppCt.MusicVolume_VR 	= i_count; 

	APP_DBG("ADCLevelsKeyInit  load_vr  Vol %d\n",i_count);
}
#endif
