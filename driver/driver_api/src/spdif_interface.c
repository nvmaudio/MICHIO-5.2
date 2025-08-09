#include <string.h>
#include "type.h"
#include "spdif.h"
#include "dma.h"
#include "clk.h"
#include "debug.h"
#include "app_config.h"
#include "mode_task.h"
//#include "rtos_api.h"

uint16_t AudioSpdifRx_GetDataLen(void)
 {
   uint16_t NumSamples = 0;
   NumSamples = DMA_CircularDataLenGet(PERIPHERAL_ID_SPDIF_RX);
   return NumSamples / 8;
 }
void *osPortMalloc(uint16_t osWantedSize);
void osPortFree( void *ospv );
uint16_t AudioSpdifRx_GetData(void* Buf, uint16_t Len)
{
	uint16_t Length = 0;//Samples
	uint8_t *SpdifBuf;

   	Length = DMA_CircularDataLenGet(PERIPHERAL_ID_SPDIF_RX) / 8;

	if(Length > Len)
	{
		Length = Len;
	}

	SpdifBuf = (uint8_t *)osPortMalloc(Length * 8);
	if(SpdifBuf == NULL)
	{
		DBG("Malloc failure!\n");
		return 0;
	}
	Length = DMA_CircularDataGet(PERIPHERAL_ID_SPDIF_RX, SpdifBuf, Length * 8);
	Length = SPDIF_SPDIFDataToPCMData((int32_t *)SpdifBuf, Length, (int32_t *)Buf, SPDIF_WORDLTH_16BIT);
	osPortFree(SpdifBuf);
    return Length / 4;
}

void AudioSpdif_DeInit(void)
{
	SPDIF_ModuleDisable();
    DMA_ChannelDisable(PERIPHERAL_ID_SPDIF_RX);
    //DMA_ChannelClose(PERIPHERAL_ID_SPDIF_RX);
}


#ifdef CFG_RES_AUDIO_SPDIFOUT_EN
int32_t SpdifBuf[1024];

uint16_t AudioSpdifTXDataSet(void* Buf, uint16_t Len)
{
	uint16_t Length;

	if(Buf == NULL) return 0;
#ifdef CFG_AUDIO_SPDIFOUT_MEDIA_NO_SRC
	if(GetSystemMode() == ModeUDiskAudioPlay || GetSystemMode() == ModeCardAudioPlay)
	{
		extern uint16_t GetSongInfoPcmBitWidth(void);
		if(GetSongInfoPcmBitWidth() == 16)
		{
			Length = Len * 4;

			uint32_t n;
			int32_t	*PcmBuf32 = Buf;
			int16_t	*PcmBuf16 = Buf;
			//24bit转成16bit数据
			for(n=0; n < Len*2; n++)
			{
				PcmBuf16[n] = PcmBuf32[n] >> 8;
			}

			//if(((DMA_CircularSpaceLenGet(PERIPHERAL_ID_SPDIF_TX) / 8) * 8) >=  Length * 2)
			{
				int m;

				m = SPDIF_PCMDataToSPDIFData((int32_t *)Buf, Length, (int32_t *)SpdifBuf, 16);
				//DBG("SPDIF_PCMDataToSPDIFData = %d Length = %d  \n",m,Length);
				DMA_CircularDataPut(PERIPHERAL_ID_SPDIF_TX, (void *)SpdifBuf, m & 0xFFFFFFFC);
			}
		}
		else
		{
			Length = Len * 8;
			DMA_CircularDataPut(PERIPHERAL_ID_SPDIF_TX, (void *)Buf, Length & 0xFFFFFFFC);
		}
		return 0;
	}
#endif

#ifndef CFG_AUDIO_WIDTH_24BIT
	Length = Len * 4;

	//if(((DMA_CircularSpaceLenGet(PERIPHERAL_ID_SPDIF_TX) / 8) * 8) >=  Length * 2)
	{
		int m;

		m = SPDIF_PCMDataToSPDIFData((int32_t *)Buf, Length, (int32_t *)SpdifBuf, 16);
		//DBG("SPDIF_PCMDataToSPDIFData = %d Length = %d  \n",m,Length);
		DMA_CircularDataPut(PERIPHERAL_ID_SPDIF_TX, (void *)SpdifBuf, m & 0xFFFFFFFC);
	}
#else
	#ifndef CFG_AUDIO_SPDIFOUT_24BIT	// SPDIF_TX 16bit输出
		Length = Len * 4;

		uint32_t n;
		int32_t	*PcmBuf32 = Buf;
		int16_t	*PcmBuf16 = Buf;
		//24bit转成16bit数据
		for(n=0; n < Len*2; n++)
		{
			PcmBuf16[n] = PcmBuf32[n] >> 8;
		}

		//if(((DMA_CircularSpaceLenGet(PERIPHERAL_ID_SPDIF_TX) / 8) * 8) >=  Length * 2)
		{
			int m;

			m = SPDIF_PCMDataToSPDIFData((int32_t *)Buf, Length, (int32_t *)SpdifBuf, 16);
			//DBG("SPDIF_PCMDataToSPDIFData = %d Length = %d  \n",m,Length);
			DMA_CircularDataPut(PERIPHERAL_ID_SPDIF_TX, (void *)SpdifBuf, m & 0xFFFFFFFC);
		}
	#else	// SPDIF_TX 24bit输出
		Length = Len * 8;
		DMA_CircularDataPut(PERIPHERAL_ID_SPDIF_TX, (void *)Buf, Length & 0xFFFFFFFC);
	#endif
#endif
	return 0;
}

uint16_t AudioSpdifTXDataSpaceLenGet(void)
{
	return DMA_CircularSpaceLenGet(PERIPHERAL_ID_SPDIF_TX) / 8;
}
#endif

