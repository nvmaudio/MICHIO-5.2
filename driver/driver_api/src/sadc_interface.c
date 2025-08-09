#include <string.h>
#include "type.h"
#include "adc.h"
#include "clk.h"
#include "backup.h"
#include "powercontroller.h"
#ifdef CFG_APP_CONFIG
#include "app_config.h"
#else
#define CFG_RES_POWERKEY_ADC_EN
#endif
void SarADC_Init(void)
{
	Clock_Module1Enable(ADC_CLK_EN);
	
	#ifdef  CFG_RES_POWERKEY_ADC_EN
		ADC_PowerkeyChannelEnable();
		BACKUP_VBK22KMode();
	#endif

	ADC_Enable();
	ADC_ClockDivSet(CLK_DIV_128);
	ADC_VrefSet(ADC_VREF_VDDA);			//1:VDDA; 0:VDD
	ADC_Calibration();
}


int16_t SarADC_LDOINVolGet(void)
{
	uint32_t DC_Data1;
	uint32_t DC_Data2;

	DC_Data1 = ADC_SingleModeDataGet(ADC_CHANNEL_VIN);
	DC_Data2 = ADC_SingleModeDataGet(ADC_CHANNEL_VDD1V2);
	DC_Data1 = (DC_Data1 * 2 * Power_LDO12Get()) / DC_Data2;
	if(DC_Data1 < 3200)
	{
		DC_Data1 += 30;
	}
	//DBG("LDOIN �� %d\n", DC_Data1);
	return (int16_t)DC_Data1;
}

