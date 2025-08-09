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
#include "spdif.h"
#include "spdif_interface.h"
#include "clk.h"

#ifdef CFG_RES_AUDIO_SPDIFOUT_EN

#define SPDIF_FIFO_LEN					(10 * 1024)
uint32_t SpdifAddr[SPDIF_FIFO_LEN/4];

bool Clock_APllLock(uint32_t PllFreq);

#if CFG_RES_MIC_SELECT
void AudioMIC_SampleRateChange(uint32_t SampleRate)
{
	if((SampleRate == 11025) || (SampleRate == 22050) || (SampleRate == 44100))
	{
		Clock_AudioMclkSel(AUDIO_ADC1, PLL_CLOCK1);
	}
	else
	{
		Clock_AudioMclkSel(AUDIO_ADC1, PLL_CLOCK2);
	}
	AudioADC_SampleRateSet(ADC1_MODULE, SampleRate);
}
#endif

void AudioSpdifOut_SampleRateChange(uint32_t SampleRate)
{
	if((SampleRate == 11025) || (SampleRate == 22050) || (SampleRate == 44100) || (SampleRate == 88200))
	{
		Clock_APllLock(225792);
	}
	else
	{
		Clock_APllLock(245760);
	}
	SPDIF_SampleRateSet(SampleRate);
}

void AudioSpdifOutParamsSet(void)
{
	GPIO_PortAModeSet(GPIOA28, 7);
//	GPIO_PortAModeSet(GPIOA29, 7);
//	GPIO_PortAModeSet(GPIOA30, 7);
//	GPIO_PortAModeSet(GPIOA31, 6);
//	Clock_APllLock(225792);
    SPDIF_ClockSourceSelect(SPIDF_CLK_SOURCE_AUPLL);
	SPDIF_TXInit(1, 1, 0, 10);
	DMA_CircularConfig(PERIPHERAL_ID_SPDIF_TX, sizeof(SpdifAddr)/4, SpdifAddr, sizeof(SpdifAddr));
//	SPDIF_SampleRateSet(CFG_PARA_SAMPLE_RATE);
	AudioSpdifOut_SampleRateChange(CFG_PARA_SAMPLE_RATE);
	SPDIF_ModuleEnable();
	DMA_ChannelEnable(PERIPHERAL_ID_SPDIF_TX);
}


bool spdif_out_init(void)
{
	AudioCoreIO AudioIOSet;
	memset(&AudioIOSet, 0, sizeof(AudioCoreIO));

	AudioIOSet.Depth = AudioCore.FrameSize[0] * 2 ;

	if(!AudioCoreSinkIsInit(AUDIO_SPDIF_SINK_NUM))
	{
		AudioIOSet.Adapt = STD;//SRC_ONLY

		AudioIOSet.Sync = TRUE;//I2S slave 时候如果master没有接，有可能会导致DAC也不出声音。
		AudioIOSet.Channels = 2;
		AudioIOSet.Net = DefaultNet;
		AudioIOSet.HighLevelCent = 60;
		AudioIOSet.LowLevelCent = 40;
		AudioIOSet.SampleRate = CFG_PARA_SAMPLE_RATE;//根据实际外设选择

		AudioIOSet.DataIOFunc = AudioSpdifTXDataSet;
		AudioIOSet.LenGetFunc = AudioSpdifTXDataSpaceLenGet;

#ifdef	CFG_AUDIO_WIDTH_24BIT
		AudioIOSet.IOBitWidth = PCM_DATA_24BIT_WIDTH;//0,16bit,1:24bit
		AudioIOSet.IOBitWidthConvFlag = 0;//不需要做位宽转换处理
#endif
		if(!AudioCoreSinkInit(&AudioIOSet, AUDIO_SPDIF_SINK_NUM))
		{
			DBG("spdif out init error");
			return FALSE;
		}

		AudioSpdifOutParamsSet();
		AudioCoreSinkEnable(AUDIO_SPDIF_SINK_NUM);
		AudioCoreSinkAdjust(AUDIO_SPDIF_SINK_NUM, TRUE);
	}
	else//sam add,20230221
	{
		SPDIF_SampleRateSet(CFG_PARA_SAMPLE_RATE);
		#ifdef	CFG_AUDIO_WIDTH_24BIT
		AudioIOSet.IOBitWidth = PCM_DATA_24BIT_WIDTH;//0,16bit,1:24bit
		AudioIOSet.IOBitWidthConvFlag = 0;//DAC 24bit ,sink最后一级输出时需要转变为24bi
		AudioCore.AudioSink[AUDIO_SPDIF_SINK_NUM].BitWidth = AudioIOSet.IOBitWidth;
		AudioCore.AudioSink[AUDIO_SPDIF_SINK_NUM].BitWidthConvFlag = AudioIOSet.IOBitWidthConvFlag;
		#endif
	}

}
bool spdif_out_hfp_init(void)
{
	AudioCoreIO AudioIOSet;
	memset(&AudioIOSet, 0, sizeof(AudioCoreIO));
	AudioIOSet.Depth = AudioCore.FrameSize[DefaultNet] * 2 ;

	if(!AudioCoreSinkIsInit(AUDIO_SPDIF_SINK_NUM))
	{
		AudioIOSet.Adapt = SRC_ONLY;
		AudioIOSet.Sync = TRUE;//I2S slave 时候如果master没有接，有可能会导致DAC也不出声音。
		AudioIOSet.Channels = 2;
		AudioIOSet.Net = DefaultNet;
		AudioIOSet.HighLevelCent = 60;
		AudioIOSet.LowLevelCent = 40;
		AudioIOSet.SampleRate = CFG_PARA_SAMPLE_RATE;//根据实际外设选择

		AudioIOSet.DataIOFunc = AudioSpdifTXDataSet;
		AudioIOSet.LenGetFunc = AudioSpdifTXDataSpaceLenGet;

#ifdef	CFG_AUDIO_WIDTH_24BIT
		AudioIOSet.IOBitWidth = PCM_DATA_24BIT_WIDTH;//0,16bit,1:24bit
		AudioIOSet.IOBitWidthConvFlag = 0;//不需要做位宽转换处理
#endif
		if(!AudioCoreSinkInit(&AudioIOSet, AUDIO_SPDIF_SINK_NUM))
		{
			DBG("SPIDF out init error");
			return FALSE;
		}

		AudioSpdifOutParamsSet();
		AudioCoreSinkEnable(AUDIO_SPDIF_SINK_NUM);
		AudioCoreSinkAdjust(AUDIO_SPDIF_SINK_NUM, TRUE);
	}
	else
	{
		SPDIF_SampleRateSet(CFG_BTHF_PARA_SAMPLE_RATE);
		#ifdef	CFG_AUDIO_WIDTH_24BIT
		AudioIOSet.IOBitWidth = PCM_DATA_16BIT_WIDTH;//0,16bit,1:24bit
		AudioIOSet.IOBitWidthConvFlag = 1;//DAC 24bit ,sink最后一级输出时需要转变为24bi
		AudioCore.AudioSink[AUDIO_SPDIF_SINK_NUM].BitWidth = AudioIOSet.IOBitWidth;
		AudioCore.AudioSink[AUDIO_SPDIF_SINK_NUM].BitWidthConvFlag = AudioIOSet.IOBitWidthConvFlag;
		AudioCore.AudioSink[AUDIO_SPDIF_SINK_NUM].Sync = FALSE;
		#endif
	}
}

bool spdif_out_deinit(void)
{
	SPDIF_ModuleDisable();
	DMA_InterruptFlagClear(PERIPHERAL_ID_SPDIF_TX, DMA_DONE_INT);
	DMA_InterruptFlagClear(PERIPHERAL_ID_SPDIF_TX, DMA_THRESHOLD_INT);
	DMA_InterruptFlagClear(PERIPHERAL_ID_SPDIF_TX, DMA_ERROR_INT);
	DMA_ChannelDisable(PERIPHERAL_ID_SPDIF_TX);
	AudioCoreSinkDeinit(AUDIO_SPDIF_SINK_NUM);
}

#endif
