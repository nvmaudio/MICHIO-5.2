#include <string.h>
#include "type.h"
#include "dma.h"
#include "adc_interface.h"
#include "dac_interface.h"
#include "app_config.h"
#include "mode_task_api.h"
#include "main_task.h"
#include "debug.h"
#include "i2s_interface.h"
//service
#include "audio_core_api.h"
#include "audio_core_service.h"
#include "mode_task_api.h"

/*
	//I2S0_MODULE Port0
	#if CFG_RES_I2S_MODE == 0
	GPIO_PortAModeSet(GPIOA0, 9);// mclk 9:out  ��master
	#else
	GPIO_PortAModeSet(GPIOA0, 3);// mclk 3:in   ��slave
	#endif
	GPIO_PortAModeSet(GPIOA1, 6);// lrclk
	GPIO_PortAModeSet(GPIOA2, 5);// bclk
	GPIO_PortAModeSet(GPIOA3, 7);// out
	GPIO_PortAModeSet(GPIOA4, 1);// din

	//I2S0_MODULE Port1
	#if CFG_RES_I2S_MODE == 0
	GPIO_PortAModeSet(GPIOA24, 9);// mclk 9:out  ��master
	#else
	GPIO_PortAModeSet(GPIOA24, 3);// mclk 3:in   ��slave
	#endif
	GPIO_PortAModeSet(GPIOA20, 6);//lrclk
	GPIO_PortAModeSet(GPIOA21, 5);//bclk
	GPIO_PortAModeSet(GPIOA22, 10);//do
	GPIO_PortAModeSet(GPIOA23, 3);//di


	//I2S1_MODULE Port0
	#if CFG_RES_I2S_MODE == 0
	GPIO_PortAModeSet(GPIOA27, 6);//mclk 6:out  ��master
	#else
	GPIO_PortAModeSet(GPIOA27, 1);//mclk 1:in   ��slave
	#endif
	GPIO_PortAModeSet(GPIOA28, 1);//lrclk
	GPIO_PortAModeSet(GPIOA29, 1);//bclk
	GPIO_PortAModeSet(GPIOA30, 6);//do
	GPIO_PortAModeSet(GPIOA31, 2);//di

	//I2S1_MODULE Port1
	#if CFG_RES_I2S_MODE == 0
	GPIO_PortAModeSet(GPIOA7, 5);//mclk 5:out  ��master
	#else
	GPIO_PortAModeSet(GPIOA7, 3);//mclk 3:in   ��slave
	#endif
	GPIO_PortAModeSet(GPIOA8, 1);//lrclk
	GPIO_PortAModeSet(GPIOA9, 2);//bclk
	GPIO_PortAModeSet(GPIOA10, 4);//do
	GPIO_PortAModeSet(GPIOA11, 2);//di

	//I2S1_MODULE Port2
	GPIO_PortAModeSet(GPIOA1, 7);//lrclk
	GPIO_PortAModeSet(GPIOA2, 6);//bclk
	GPIO_PortAModeSet(GPIOA31, 5);//do
	GPIO_PortAModeSet(GPIOA30, 2);//di

	//I2S1_MODULE Port3
	GPIO_PortAModeSet(GPIOA20, 7);//lrclk
	GPIO_PortAModeSet(GPIOA21, 6);//bclk
	GPIO_PortAModeSet(GPIOA11, 4);//do
	GPIO_PortAModeSet(GPIOA10, 1);//di
*/




void I2S_GPIO_Port_ModeSet(I2S_MODULE I2SModuleIndex, uint8_t i2s_mode)
{
	if (I2SModuleIndex == I2S0_MODULE)
	{
		GPIO_PortAModeSet(GPIOA20, 6);
		GPIO_PortAModeSet(GPIOA21, 5);
		GPIO_PortAModeSet(GPIOA22, 10);
	}

	if (I2SModuleIndex == I2S1_MODULE)
	{
		GPIO_PortAModeSet(GPIOA8, 1);	  
		GPIO_PortAModeSet(GPIOA9, 2);	 
		GPIO_PortAModeSet(GPIOA10, 4);

	}	
}







#ifdef CFG_RES_AUDIO_I2SOUT_EN
	void AudioI2sOutParamsSet(void)
	{
		I2SParamCt i2s_set;
		i2s_set.IsMasterMode = CFG_RES_I2S_MODE;
		i2s_set.SampleRate = CFG_PARA_I2S_SAMPLERATE; 
		i2s_set.I2sFormat = I2S_FORMAT_I2S;

		#ifdef	CFG_AUDIO_WIDTH_24BIT
			i2s_set.I2sBits = I2S_LENGTH_24BITS;
		#else
			i2s_set.I2sBits = I2S_LENGTH_16BITS;
		#endif

		i2s_set.I2sTxRxEnable = 1;

		#if (CFG_RES_I2S == 0)
			i2s_set.TxPeripheralID = PERIPHERAL_ID_I2S0_TX;
		#else
			i2s_set.TxPeripheralID = PERIPHERAL_ID_I2S1_TX;
		#endif

		i2s_set.TxBuf = (void*)mainAppCt.I2SFIFO;

		i2s_set.TxLen = mainAppCt.I2SFIFO_LEN;

		I2S_GPIO_Port_ModeSet(CFG_RES_I2S_MODULE, CFG_RES_I2S_MODE);

		I2S_AlignModeSet(CFG_RES_I2S_MODULE, I2S_LOW_BITS_ACTIVE);
		AudioI2S_Init(CFG_RES_I2S_MODULE, &i2s_set);
	}
#endif







void AudioI2s0ParamsSet(void)
{
#ifdef CFG_RES_AUDIO_I2S0OUT_EN
	I2SParamCt i2s0_set;
	i2s0_set.IsMasterMode = CFG_RES_I2S_MODE;
	i2s0_set.SampleRate = CFG_PARA_I2S_SAMPLERATE;
	i2s0_set.I2sFormat = I2S_FORMAT_I2S;
	
	#ifdef	CFG_AUDIO_WIDTH_24BIT
		i2s0_set.I2sBits = I2S_LENGTH_24BITS;
	#else
		i2s0_set.I2sBits = I2S_LENGTH_16BITS;
	#endif

	i2s0_set.I2sTxRxEnable = 1;
	i2s0_set.TxPeripheralID = PERIPHERAL_ID_I2S0_TX;
	i2s0_set.RxPeripheralID = PERIPHERAL_ID_I2S0_RX;

	
	if (mainAppCt.I2S0_TX_FIFO == NULL){
		mainAppCt.I2S0_TX_FIFO = (uint32_t*)osPortMalloc(mainAppCt.I2S0_TX_FIFO_LEN);
	}
	if(mainAppCt.I2S0_TX_FIFO != NULL){
		memset(mainAppCt.I2S0_TX_FIFO, 0, mainAppCt.I2S0_TX_FIFO_LEN);
	}else{
		APP_DBG("malloc I2S0_TX_FIFO error\n");
		return;
	}

	i2s0_set.TxBuf = (void*)mainAppCt.I2S0_TX_FIFO;
	i2s0_set.TxLen = mainAppCt.I2S0_TX_FIFO_LEN;
	
	I2S_GPIO_Port_ModeSet(I2S0_MODULE, CFG_RES_I2S_MODE);
	I2S_ModuleDisable(I2S0_MODULE);
	I2S_AlignModeSet(I2S0_MODULE, I2S_LOW_BITS_ACTIVE);
	AudioI2S_Init(I2S0_MODULE, &i2s0_set);
#endif
}



void AudioI2s1ParamsSet(void)
{
#ifdef CFG_RES_AUDIO_I2S1OUT_EN
	I2SParamCt i2s1_set;
	i2s1_set.IsMasterMode = CFG_RES_I2S_MODE;
	i2s1_set.SampleRate = CFG_PARA_I2S_SAMPLERATE; 
	i2s1_set.I2sFormat = I2S_FORMAT_I2S;
	
	#ifdef	CFG_AUDIO_WIDTH_24BIT
		i2s1_set.I2sBits = I2S_LENGTH_24BITS;
	#else
		i2s1_set.I2sBits = I2S_LENGTH_16BITS;
	#endif


	i2s1_set.I2sTxRxEnable = 1;
	i2s1_set.TxPeripheralID = PERIPHERAL_ID_I2S1_TX;
	i2s1_set.RxPeripheralID = PERIPHERAL_ID_I2S1_RX;


	if (mainAppCt.I2S1_TX_FIFO == NULL){
		mainAppCt.I2S1_TX_FIFO = (uint32_t*)osPortMalloc(mainAppCt.I2S1_TX_FIFO_LEN);
	}
	if(mainAppCt.I2S1_TX_FIFO != NULL){
		memset(mainAppCt.I2S1_TX_FIFO, 0, mainAppCt.I2S1_TX_FIFO_LEN);
	}else{
		APP_DBG("malloc I2S1_TX_FIFO error\n");
		return;
	}

	i2s1_set.TxBuf = (void*)mainAppCt.I2S1_TX_FIFO;
	i2s1_set.TxLen = mainAppCt.I2S1_TX_FIFO_LEN;

	I2S_GPIO_Port_ModeSet(I2S1_MODULE, CFG_RES_I2S_MODE);
	I2S_ModuleDisable(I2S1_MODULE);
	I2S_AlignModeSet(I2S1_MODULE, I2S_LOW_BITS_ACTIVE);
	AudioI2S_Init(I2S1_MODULE, &i2s1_set);
#endif
}

