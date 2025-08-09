#include <string.h>
#include <nds32_intrinsic.h>
#include "math.h"
#include "debug.h"
#include "app_config.h"
#include "rtos_api.h"
#include "audio_effect_api.h"
#include "audio_effect_library.h"
#include "audio_core_api.h"
#include "main_task.h"
#include "audio_effect.h"
#include "tws_mode.h"
#include "bt_tws_api.h"
#include "remind_sound.h"
#include "ctrlvars.h"
#include "comm_param.h"
#include "bt_manager.h"
#include "mode_task.h"
#include "bt_hf_api.h"
#include "audio_effect_user.h"

#ifdef CFG_FUNC_EQMODE_FADIN_FADOUT_EN
extern uint32_t music_eq_mode_unit;
#endif
extern PCM_DATA_TYPE * pcm_buf_1;
extern PCM_DATA_TYPE * pcm_buf_2;
extern PCM_DATA_TYPE * pcm_buf_3;
extern PCM_DATA_TYPE * pcm_buf_4;
extern PCM_DATA_TYPE * pcm_buf_5;
extern PCM_DATA_TYPE * pcm_buf_6;

#ifdef CFG_AUDIO_WIDTH_24BIT
#define AUDIO_WIDTH    		  24
#else
#define AUDIO_WIDTH    		  16
#endif

#ifdef CFG_FUNC_RECORDER_SILENCE_DECTOR
extern SilenceDetectorUnit UserSilenceDetector;
#endif

void AudioEffectMutex_Lock(void)
{
	#ifdef FUNC_OS_EN
	if(AudioEffectMutex != NULL)
	{
		osMutexLock(AudioEffectMutex);
	}
	#endif
}

void AudioEffectMutex_Unlock(void)
{
	#ifdef FUNC_OS_EN
	if(AudioEffectMutex != NULL)
	{
		osMutexUnlock(AudioEffectMutex);
	}
	#endif
}

void MV_memcpy_16bit(int16_t *pcm_out, int16_t *pcm_in, uint32_t n)
{
	int16_t  s;
	for(s = 0; s < n; s++)
	{
		pcm_out[2*s + 0] = pcm_in[2*s + 0];
		pcm_out[2*s + 1] = pcm_in[2*s + 1];
	}
}


void MV_memcpy_16bit2(int16_t *pcm_out, int16_t *pcm_in, uint32_t n)
{
	int16_t  s;
	for(s = 0; s < n; s++)
	{
		pcm_out[2*s + 0] = pcm_in[s];
		pcm_out[2*s + 1] = pcm_in[s];
	}
}



void MV_memcpy_24bit(int32_t *pcm_out, int32_t *pcm_in, uint32_t n)
{
	int16_t  s;
	for(s = 0; s < n; s++)
	{
		pcm_out[2*s + 0] = pcm_in[2*s + 0];
		pcm_out[2*s + 1] = pcm_in[2*s + 1];
	}
}



#ifdef CFG_FUNC_AUDIO_EFFECT_EN
__attribute__((optimize("Og")))
void AudioMusicProcess(AudioCoreContext *pAudioCore)
{
	uint16_t i;
	int16_t  s;
	uint16_t n = AudioCoreFrameSizeGet(AudioCore.CurrentMix);
	EffectNode*  pNode 			= NULL;

	#ifdef BT_TWS_SUPPORT
		//int16_t *line_in        	= NULL;//pBuf->line_in;
		//int16_t *spdif_in       	= NULL;//pBuf->spdif_in;
		int16_t *music_in    		= NULL;//pBuf->music_in;///music input
		int16_t *tws_out    		= NULL;
		PCM_DATA_TYPE *tws_in		= NULL;
	#else
		PCM_DATA_TYPE *music_in    	= NULL;//pBuf->music_in;///music input
		PCM_DATA_TYPE *line_in      = NULL;//pBuf->line_in;
		PCM_DATA_TYPE *spdif_in     = NULL;//pBuf->spdif_in;
	#endif

	PCM_DATA_TYPE *monitor_out	= NULL;
	PCM_DATA_TYPE *record_out   = NULL;//pBuf->dacx_out;
	PCM_DATA_TYPE *remind_in    = NULL;//pBuf->remind_in;
	//PCM_DATA_TYPE *rec_pcm_buf	= NULL;
	
	#if defined (CFG_APP_USB_AUDIO_MODE_EN)

		int16_t *mic_pcm			= NULL;
		int16_t *usb_out			= NULL;

		if(pAudioCore->AudioSource[MIC_SOURCE_NUM].Active == TRUE)
		{
			mic_pcm = (int16_t *)pAudioCore->AudioSource[MIC_SOURCE_NUM].PcmInBuf;
		}
		if(pAudioCore->AudioSink[USB_AUDIO_SINK_NUM].Active == TRUE)  
		{
			usb_out = (int16_t *)pAudioCore->AudioSink[USB_AUDIO_SINK_NUM].PcmOutBuf;
			if(mic_pcm){
				MV_memcpy_16bit(usb_out, mic_pcm, n);
			}
		}
	#endif

	
	if(pAudioCore->AudioSource[APP_SOURCE_NUM].Active == TRUE)
	{
		#ifdef BT_TWS_SUPPORT
			music_in = (int16_t *)pAudioCore->AudioSource[APP_SOURCE_NUM].PcmInBuf;
		#else
			music_in = pAudioCore->AudioSource[APP_SOURCE_NUM].PcmInBuf;
		#endif
	}
	
	
	if(pAudioCore->AudioSink[AUDIO_DAC0_SINK_NUM].Active == TRUE)
	{
		monitor_out = pAudioCore->AudioSink[AUDIO_DAC0_SINK_NUM].PcmOutBuf;
	}
	
	#ifdef CFG_RES_AUDIO_DACX_EN
		if(pAudioCore->AudioSink[AUDIO_DACX_SINK_NUM].Active == TRUE)   			////dacx buff
		{
			record_out = pAudioCore->AudioSink[AUDIO_DACX_SINK_NUM].PcmOutBuf;
		}
	#endif

	#if defined(CFG_FUNC_REMIND_SOUND_EN)
		if(pAudioCore->AudioSource[REMIND_SOURCE_NUM].Active == TRUE)				////remind buff
		{
			remind_in = pAudioCore->AudioSource[REMIND_SOURCE_NUM].PcmInBuf;
		}
	#endif

	#ifdef CFG_RES_AUDIO_I2SOUT_EN
		PCM_DATA_TYPE *i2s_out      = NULL;//pBuf->i2s0_out;
		if(pAudioCore->AudioSink[AUDIO_I2SOUT_SINK_NUM].Active == TRUE) 			////i2s buff
		{
			i2s_out = pAudioCore->AudioSink[AUDIO_I2SOUT_SINK_NUM].PcmOutBuf;
		}
		if(i2s_out){
			memset(i2s_out, 0, n * sizeof(PCM_DATA_TYPE) * 2);						//mono*2 stereo*4
		}	
	#endif


	#ifdef CFG_RES_AUDIO_I2S0OUT_EN
		PCM_DATA_TYPE 	*i2s0_out  	= NULL;
		if(pAudioCore->AudioSink[AUDIO_I2S0_OUT_SINK_NUM].Active == TRUE)
		{
			i2s0_out = pAudioCore->AudioSink[AUDIO_I2S0_OUT_SINK_NUM].PcmOutBuf;
		}	
		if (i2s0_out){
			memset(i2s0_out, 0, n * sizeof(PCM_DATA_TYPE) * 2);
		}
	#endif

	#ifdef CFG_RES_AUDIO_I2S1OUT_EN
		PCM_DATA_TYPE 	*i2s1_out  	= NULL;
		if(pAudioCore->AudioSink[AUDIO_I2S1_OUT_SINK_NUM].Active == TRUE)
		{
			i2s1_out = pAudioCore->AudioSink[AUDIO_I2S1_OUT_SINK_NUM].PcmOutBuf;
		}	
		if (i2s1_out){
			memset(i2s1_out, 0, n * sizeof(PCM_DATA_TYPE) * 2);
		}
	#endif

	#ifdef BT_TWS_SUPPORT
		if(pAudioCore->AudioSource[TWS_SOURCE_NUM].Active == TRUE)
		{
			tws_in = pAudioCore->AudioSource[TWS_SOURCE_NUM].PcmInBuf;
		}
		if(pAudioCore->AudioSink[TWS_SINK_NUM].Active == TRUE)
		{
			tws_out = (int16_t*)pAudioCore->AudioSink[TWS_SINK_NUM].PcmOutBuf;
		}
		if(tws_out){
			memset(tws_out, 0, n * sizeof(PCM_DATA_TYPE) * 2);
		}
	#endif

    if(monitor_out){
    	memset(monitor_out, 0, n * sizeof(PCM_DATA_TYPE) * 2);
    }
    if(record_out){
    	memset(record_out, 0, n * sizeof(PCM_DATA_TYPE) * 2);
    }

	#ifdef CFG_FUNC_EQMODE_FADIN_FADOUT_EN
		if((music_in)&&(mainAppCt.EqModeBak != mainAppCt.EqMode))
		{		
			EqModeSet(mainAppCt.EqMode);
			mainAppCt.EqModeBak = mainAppCt.EqMode;
		}
	#endif

	#ifdef BT_TWS_SUPPORT
		if(music_in)
		{
			AudioCoreAppSourceVolApply(APP_SOURCE_NUM, music_in, n, 2);
		}
		
		if(tws_in)
		{
			AudioEffectMutex_Lock();
			for(i=0; i<AUDIO_EFFECT_NODE_NUM; i++)
			{
				pNode = &gEffectNodeList[0].EffectNode[i];
				if((pNode->Enable == FALSE) || (pNode->EffectUnit == NULL))
				{
					continue;
				}
				pNode->FuncAudioEffect(pNode->EffectUnit, (int16_t *)tws_in, (int16_t *)tws_in, n);
			}	

			for(i=0; i<AUDIO_EFFECT_NODE_NUM; i++)
			{
				pNode = &gEffectNodeList[1].EffectNode[i];
				if((pNode->Enable == FALSE) || (pNode->EffectUnit == NULL))
				{
					if(pNode->NodeType == NodeType_Bifurcate)
					{
						
						AudioCoreAppSourceVolApply(TWS_SOURCE_NUM, (int16_t *)tws_in, n, 2);
						if(record_out)
						{
							memcpy(record_out, tws_in, n * sizeof(PCM_DATA_TYPE) * 2);
						}
					}
					continue;
				}
				
				pNode->FuncAudioEffect(pNode->EffectUnit, (int16_t *)tws_in, (int16_t *)tws_in, n);
				if(pNode->NodeType == NodeType_Bifurcate)
				{
					AudioCoreAppSourceVolApply(TWS_SOURCE_NUM, (int16_t *)tws_in, n, 2);
					if(record_out)
					{
						memcpy(record_out, tws_in, n * sizeof(PCM_DATA_TYPE) * 2);
					}
				}
			}

			AudioEffectMutex_Unlock();
		}
	#else//BT_TWS_SUPPORT
		if(music_in)
		{
			#ifdef CFG_FUNC_RECORDER_EN
			if(GetSystemMode() == ModeCardPlayBack || GetSystemMode() == ModeUDiskPlayBack)// || GetSystemMode() == ModeFlashFsPlayBack)
			{
				AudioCoreAppSourceVolApply(PLAYBACK_SOURCE_NUM, music_in, n, 2);
			}
			#endif
			
			AudioEffectMutex_Lock();

			//appͨ·����Ԥ���洦��
			for(i=0; i<AUDIO_EFFECT_NODE_NUM; i++)
			{
				pNode = &gEffectNodeList[0].EffectNode[i];//����ʹ�õ�0����Ч�б�
				if((pNode->Enable == FALSE) || (pNode->EffectUnit == NULL))
				{
					continue;
				}
				pNode->FuncAudioEffect(pNode->EffectUnit, (int16_t *)music_in, (int16_t *)music_in, n);
			}

			for(i=0; i<AUDIO_EFFECT_NODE_NUM; i++)
			{
				pNode = &gEffectNodeList[1].EffectNode[i];//����ʹ�õ�1����Ч�б�
				if((pNode->Enable == FALSE) || (pNode->EffectUnit == NULL))
				{
					if(pNode->NodeType == NodeType_Bifurcate)
					{
						//ϵͳ������ƣ����ݴӹ����飬��ɢ�ŵ���Ч������������
						AudioCoreAppSourceVolApply(APP_SOURCE_NUM, (int16_t *)music_in, n, 2);//����������ź��������֮����
						//ע��������ƴ����ڴ����зŵ�λ��һ��Ҫע�⣬��Ҫ������Ч�б�����������
						if(record_out)
						{
							memcpy(record_out, music_in, n * sizeof(PCM_DATA_TYPE) * 2);//������Ч֮���һ���ֲ�ڵ�
						}
					}
					continue;
				}

				pNode->FuncAudioEffect(pNode->EffectUnit, (int16_t *)music_in, (int16_t *)music_in, n);
				//�ֲ�ڵ㴦��
				if(pNode->NodeType == NodeType_Bifurcate)
				{
					//ϵͳ������ƣ����ݴӹ����飬��ɢ�ŵ���Ч������������
					AudioCoreAppSourceVolApply(APP_SOURCE_NUM, (int16_t *)music_in, n, 2);//����������ź��������֮����
					//ע��������ƴ����ڴ����зŵ�λ��һ��Ҫע�⣬��Ҫ������Ч�б�����������
					if(record_out)
					{
						memcpy(record_out, music_in, n * sizeof(PCM_DATA_TYPE) * 2);//������Ч֮���һ���ֲ�ڵ�
					}
				}
			}
			AudioEffectMutex_Unlock();
		}
	#endif//BT_TWS_SUPPORT

	if(monitor_out == NULL)
	{
		#ifdef CFG_RES_AUDIO_I2SOUT_EN
			monitor_out = i2s_out;
		#endif
		#ifdef CFG_RES_AUDIO_I2S0OUT_EN
			//monitor_out = i2s0_out;
		#endif
		#ifdef CFG_RES_AUDIO_I2S1OUT_EN
			//monitor_out = i2s1_out;
		#endif
		#ifdef CFG_RES_AUDIO_SPDIFOUT_EN
			monitor_out = spdif_out;
		#endif
	}


	if(monitor_out)
	{
		#ifdef CFG_FUNC_REMIND_SOUND_EN
			if(remind_in)
			{
				AudioCoreAppSourceVolApply(REMIND_SOURCE_NUM, (int16_t *)remind_in, n, 2);

				#ifdef BT_TWS_SUPPORT
					if((tws_out) && (music_in)){
						MV_memcpy_16bit(tws_out, music_in, n);
					}
					
					if(tws_in)
					{
						for(s = 0; s < n; s++)
						{
							monitor_out[2*s + 0] = __nds32__clips((((int32_t)tws_in[2*s + 0] + (int32_t)remind_in[2*s + 0])), AUDIO_WIDTH - 1);
							monitor_out[2*s + 1] = __nds32__clips((((int32_t)tws_in[2*s + 1] + (int32_t)remind_in[2*s + 1])), AUDIO_WIDTH - 1);
						}
					}
				#else
					if(music_in)
					{
						for(s = 0; s < n; s++)
						{
							monitor_out[2*s + 0] = __nds32__clips((((int32_t)music_in[2*s + 0] + (int32_t)remind_in[2*s + 0])), AUDIO_WIDTH - 1);
							monitor_out[2*s + 1] = __nds32__clips((((int32_t)music_in[2*s + 1] + (int32_t)remind_in[2*s + 1])), AUDIO_WIDTH - 1);
						}
					}
					else
					{
						MV_memcpy_24bit(monitor_out, remind_in, n);
					}
				#endif
			}
			else
		#endif				//CFG_FUNC_REMIND_SOUND_EN
		{
			#ifdef BT_TWS_SUPPORT
				if(tws_out)
				{
					if(music_in)
					{
						MV_memcpy_16bit(tws_out, music_in, n);
					}
				}
				
				if(tws_in){
					MV_memcpy_16bit(monitor_out, tws_in, n);
				}
			#else
				if(music_in){
					MV_memcpy_16bit(monitor_out, music_in, n);
				}
			#endif
		}


		#ifdef CFG_RES_AUDIO_I2S0OUT_EN
			if (i2s0_out){
				MV_memcpy_16bit(i2s0_out, monitor_out, n);
			}
		#endif

		#ifdef CFG_RES_AUDIO_I2SOUT_EN
			if (i2s_out){
				MV_memcpy_16bit(i2s_out, monitor_out, n);
			}
		#endif
	}

	#ifdef CFG_RES_AUDIO_DACX_EN
		if(record_out)
		{
			for(s = 0; s < n; s++){
				record_out[s] = __nds32__clips(((int32_t)record_out[2*s] + (int32_t)record_out[2*s + 1] +1 ) >>1, AUDIO_WIDTH - 1);
			}

			AudioEffectMutex_Lock();
			
			for(i=0; i<AUDIO_EFFECT_NODE_NUM; i++)
			{
				pNode = &gEffectNodeList[2].EffectNode[i];
				if((pNode->Enable == FALSE) || (pNode->EffectUnit == NULL))
				{
					continue;
				}
				pNode->FuncAudioEffect(pNode->EffectUnit, (int16_t *)record_out, (int16_t *)record_out, n);
			}

			AudioEffectMutex_Unlock();

			#ifdef CFG_FUNC_REMIND_SOUND_EN
			if(remind_in)
			{
				if(RemindSoundIsMix())
				{
					PCM_DATA_TYPE temp;
					for(s = 0; s < n; s++){
						temp 			= __nds32__clips(((int32_t)remind_in[2*s] + (int32_t)remind_in[2*s + 1] + 1) >> 1, AUDIO_WIDTH - 1);
						record_out[s]	= __nds32__clips(((int32_t)temp + (int32_t)record_out[s]), AUDIO_WIDTH - 1);
					}
				}
				else
				{
					for(s = 0; s < n; s++){
						record_out[s] 	= __nds32__clips(((int32_t)remind_in[2*s] + (int32_t)remind_in[2*s + 1] + 1)>>1, AUDIO_WIDTH - 1);
					}
				}
			}
			#endif

			#ifdef CFG_RES_AUDIO_I2S1OUT_EN
				if (i2s1_out){
					for(s = 0; s < n; s++)
					{
						i2s1_out[2*s + 0] = record_out[s]; // Left
						i2s1_out[2*s + 1] = record_out[s]; // Right
					}
				}
			#endif
		}
	#endif
}




#if defined(CFG_APP_BT_MODE_EN) && (BT_HFP_SUPPORT == ENABLE)
__attribute__((optimize("Og")))
void AudioEffectProcessBTHF(AudioCoreContext *pAudioCore)
{
	int16_t  s;
	uint16_t i;
	uint16_t n = AudioCoreFrameSizeGet(AudioCore.CurrentMix);
	EffectNode*  pNode 		= NULL;
	int16_t *remind_in      = NULL;//pBuf->remind_in;
	int16_t *monitor_out    = NULL;//pBuf->dac0_out;
	int16_t *record_out     = NULL;//pBuf->dacx_out;
	int16_t *i2s_out        = NULL;//pBuf->i2s_out;
	int16_t *i2s0_out       = NULL;//pBuf->i2s0_out;
	int16_t *i2s1_out       = NULL;//pBuf->i2s1_out;
	int16_t *usb_out        = NULL;//pBuf->usb_out;
#ifdef CFG_RES_AUDIO_SPDIFOUT_EN
	int16_t *spdif_out       = NULL;//pBuf->spdif_out;
#endif

	int16_t  *hf_mic_in     = NULL;//pBuf->hf_mic_in;//����ͨ��mic����buffer
	int16_t  *hf_pcm_in     = NULL;//pBuf->hf_pcm_in;//����ͨ���´�buffer
	int16_t  *hf_aec_in		= NULL;//pBuf->hf_aec_in;;//����ͨ���´�delay buffer,ר��aec�㷨����
	int16_t  *hf_pcm_out    = NULL;//pBuf->hf_pcm_out;//����ͨ���ϴ�buffer
	int16_t  *hf_dac_out    = NULL;//pBuf->hf_dac_out;//����ͨ��DAC��buffer
	int16_t  *hf_rec_out    = NULL;//pBuf->hf_rec_out;//����ͨ����¼����buffer
	int16_t  *u_pcm_tmp     = (int16_t *)pcm_buf_4;
	int16_t  *d_pcm_tmp     = (int16_t *)pcm_buf_5;


	if(pAudioCore->AudioSource[MIC_SOURCE_NUM].Active == TRUE)////mic buff
	{
		hf_mic_in = (int16_t *)pAudioCore->AudioSource[MIC_SOURCE_NUM].PcmInBuf;
	}

	if(pAudioCore->AudioSource[APP_SOURCE_NUM].Active == TRUE)////music buff
	{
		//hf sco: nomo
		hf_pcm_in = (int16_t *)pAudioCore->AudioSource[APP_SOURCE_NUM].PcmInBuf;

		//aec process:push the new data, pop the old data
		if(BtHf_AECRingDataSpaceLenGet() > CFG_BTHF_PARA_SAMPLES_PER_FRAME)
			BtHf_AECRingDataSet(hf_pcm_in, CFG_BTHF_PARA_SAMPLES_PER_FRAME);
		hf_aec_in = BtHf_AecInBuf();
	}

#if defined(CFG_FUNC_REMIND_SOUND_EN)
	if(pAudioCore->AudioSource[REMIND_SOURCE_NUM].Active == TRUE)////remind buff
	{
		remind_in = (int16_t *)pAudioCore->AudioSource[REMIND_SOURCE_NUM].PcmInBuf;
	}
#endif

#ifdef CFG_RES_AUDIO_DAC0_EN
    if(pAudioCore->AudioSink[AUDIO_DAC0_SINK_NUM].Active == TRUE)   ////dac0 buff
	{
		//hf mode
		hf_dac_out = (int16_t *)pAudioCore->AudioSink[AUDIO_DAC0_SINK_NUM].PcmOutBuf;
		hf_pcm_out = (int16_t *)pAudioCore->AudioSink[AUDIO_HF_SCO_SINK_NUM].PcmOutBuf;
	}
#else
	hf_pcm_out = pAudioCore->AudioSink[AUDIO_HF_SCO_SINK_NUM].PcmOutBuf;
#endif

#ifdef CFG_RES_AUDIO_DACX_EN
	if(pAudioCore->AudioSink[AUDIO_DACX_SINK_NUM].Active == TRUE)   ////dacx buff
	{
		#if (BT_HFP_SUPPORT == ENABLE)
		if(GetSystemMode() == ModeBtHfPlay)
		{
			record_out = NULL;
		}
		else
		#endif
		{
			record_out = (int16_t *)pAudioCore->AudioSink[AUDIO_DACX_SINK_NUM].PcmOutBuf;
		}
	}
#endif

#ifdef CFG_RES_AUDIO_I2SOUT_EN
	if(pAudioCore->AudioSink[AUDIO_I2SOUT_SINK_NUM].Active == TRUE)	////i2s buff
	{
		i2s_out = (int16_t *)pAudioCore->AudioSink[AUDIO_I2SOUT_SINK_NUM].PcmOutBuf;
	}
#endif

#ifdef CFG_RES_AUDIO_I2S0OUT_EN
	if(pAudioCore->AudioSink[AUDIO_I2S0_OUT_SINK_NUM].Active == TRUE)	////i2s0 buff
	{
		i2s0_out = (int16_t *)pAudioCore->AudioSink[AUDIO_I2S0_OUT_SINK_NUM].PcmOutBuf;
	}
#endif

#ifdef CFG_RES_AUDIO_I2S1OUT_EN
	if(pAudioCore->AudioSink[AUDIO_I2S1_OUT_SINK_NUM].Active == TRUE)	////i2s0 buff
	{
		i2s1_out = (int16_t *)pAudioCore->AudioSink[AUDIO_I2S1_OUT_SINK_NUM].PcmOutBuf;
	}
#endif

#ifdef CFG_RES_AUDIO_SPDIFOUT_EN
	if(pAudioCore->AudioSink[AUDIO_SPDIF_SINK_NUM].Active == TRUE)	////i2s buff
	{
		spdif_out = (int16_t *)pAudioCore->AudioSink[AUDIO_SPDIF_SINK_NUM].PcmOutBuf;
	}
#endif

    if(monitor_out){
		memset(monitor_out, 0, n * 2 * 2);
    }

    if(record_out){
		memset(record_out, 0, n * 2);
    }

    if(usb_out){
		memset(usb_out, 0, n * 2 * 2);//mono*2 stereo*4
    }

    if(i2s_out){
		memset(i2s_out, 0, n * 2 * 2);//mono*2 stereo*4
	}
	
    if(i2s0_out){
		memset(i2s0_out, 0, n * 2 * 2);//mono*2 stereo*4
	}
	
    if(i2s1_out){
		memset(i2s1_out, 0, n * 2 * 2);//mono*2 stereo*4
	}	

	if(hf_pcm_out){
		memset(hf_pcm_out, 0, n * 2);
	}

	if(hf_dac_out){
		memset(hf_dac_out, 0, n * 2 * 2);
	}

	if(hf_rec_out){
		memset(hf_rec_out, 0, n * 2);
	}

	if(hf_dac_out == NULL)
	{
		#ifdef CFG_RES_AUDIO_I2SOUT_EN
		hf_dac_out = i2s_out;
		#endif
		#ifdef CFG_RES_AUDIO_I2S0OUT_EN
		hf_dac_out = i2s0_out;
		#endif
		#ifdef CFG_RES_AUDIO_I2S1OUT_EN
		hf_dac_out = i2s1_out;
		#endif
		#ifdef CFG_RES_AUDIO_SPDIFOUT_EN
		hf_dac_out = spdif_out;
		#endif
	}

	if(hf_mic_in && hf_pcm_in && hf_pcm_out && (hf_dac_out || i2s_out || i2s0_out || i2s1_out))
	{
		AudioCoreAppSourceVolApply(APP_SOURCE_NUM, hf_pcm_in, n, 1);

#if defined(CFG_FUNC_REMIND_SOUND_EN)
		if(remind_in)
		{
			AudioCoreAppSourceVolApply(REMIND_SOURCE_NUM, remind_in, n, 2);
			for(s = 0; s < n; s++)
			{
				hf_dac_out[2*s + 0] = __nds32__clips((((int32_t)hf_pcm_in[s] + (int32_t)remind_in[2*s + 0])), 16-1);
				hf_dac_out[2*s + 1] = __nds32__clips((((int32_t)hf_pcm_in[s] + (int32_t)remind_in[2*s + 1])), 16-1);
			}
		}
		else
#endif
		{
			for(s = 0; s < n; s++)
			{
				hf_dac_out[s*2 + 0] = hf_pcm_in[s];
				hf_dac_out[s*2 + 1] = hf_pcm_in[s];
			}
		}

		for(s = 0; s < n; s++)
		{
			hf_pcm_out[s] = __nds32__clips((((int32_t)hf_mic_in[2 * s + 0] + (int32_t)hf_mic_in[2 * s + 1])), 16-1);
		}

		for(i=0; i<AUDIO_EFFECT_NODE_NUM; i++)
		{
			pNode = &gEffectNodeList[0].EffectNode[i];//����ʹ�õ�0����Ч�б�
			if((pNode->Enable == FALSE) || (pNode->EffectUnit == NULL))
			{
				continue;
			}
#if CFG_AUDIO_EFFECT_AEC_EN
			if(pNode->NodeType == NodeType_AEC)
			{
				for(s = 0; s < n; s++)
				{
					d_pcm_tmp[s] = hf_pcm_out[s];
					u_pcm_tmp[s] = hf_aec_in[s];
				}
				AudioEffectAECApply(pNode->EffectUnit, u_pcm_tmp , d_pcm_tmp, hf_pcm_out, n);
			}
			else
#endif
			{
				pNode->FuncAudioEffect(pNode->EffectUnit, hf_pcm_out, hf_pcm_out, n);
				if(pNode->NodeType == NodeType_Vol)
				{
					AudioCoreAppSourceVolApply(MIC_SOURCE_NUM, hf_pcm_out, n, 1);
				}
			}
		}
	}
	else if(hf_dac_out || i2s_out)
	{
		#if defined(CFG_FUNC_REMIND_SOUND_EN)//��ʾ����Ч����
		if(remind_in)
		{
			AudioCoreAppSourceVolApply(REMIND_SOURCE_NUM, remind_in, n, 2);
			for(s = 0; s < n; s++)
			{
				hf_dac_out[2*s + 0] = remind_in[2*s + 0];
				hf_dac_out[2*s + 1] = remind_in[2*s + 1];
			}
		}
		#endif
	}

	#ifdef CFG_RES_AUDIO_I2SOUT_EN
	if(i2s_out && hf_dac_out != i2s_out)
	{
		for(s = 0; s < n; s++)
		{
			i2s_out[2*s + 0] = hf_dac_out[2*s + 0];
			i2s_out[2*s + 1] = hf_dac_out[2*s + 1];
		}
	}
	#endif

	#ifdef CFG_RES_AUDIO_I2S0OUT_EN
	if(i2s0_out && hf_dac_out != i2s0_out)
	{
		for(s = 0; s < n; s++)
		{
			i2s0_out[2*s + 0] = hf_dac_out[2*s + 0];
			i2s0_out[2*s + 1] = hf_dac_out[2*s + 1];
		}
	}
	#endif

	#ifdef CFG_RES_AUDIO_I2S1OUT_EN
	if(i2s1_out && hf_dac_out != i2s1_out)
	{
		for(s = 0; s < n; s++)
		{
			i2s1_out[2*s + 0] = hf_dac_out[2*s + 0];
			i2s1_out[2*s + 1] = hf_dac_out[2*s + 1];
		}
	}
	#endif	
#ifdef CFG_RES_AUDIO_SPDIFOUT_EN
	if(spdif_out && hf_dac_out != spdif_out)
	{
		for(s = 0; s < n; s++)
		{
			spdif_out[2*s + 0] = hf_dac_out[2*s + 0];
			spdif_out[2*s + 1] = hf_dac_out[2*s + 1];
		}
	}
#endif

	#ifdef CFG_RES_AUDIO_DACX_EN
	//DAC X������¼����Ч����
	if(record_out)
	{
		for(s = 0; s < n; s++)
		{
			record_out[s] 	= __nds32__clips(((int32_t)hf_dac_out[2*s] + (int32_t)hf_dac_out[2*s + 1] + 1)>>1, 16-1);
		}
	}
	#endif
}
#endif
#endif

__attribute__((optimize("Og")))
void AudioBypassProcess(AudioCoreContext *pAudioCore)
{
	int16_t  s;
	uint16_t n = AudioCoreFrameSizeGet(AudioCore.CurrentMix);
	PCM_DATA_TYPE *mic_pcm    		= NULL;//pBuf->mic_in;///mic input
	PCM_DATA_TYPE *monitor_out    	= NULL;//pBuf->dac0_out;
	PCM_DATA_TYPE *remind_in 		= NULL;
	PCM_DATA_TYPE *music_pcm    	= NULL;
#ifdef CFG_RES_AUDIO_DACX_EN
	PCM_DATA_TYPE *record_out     	= NULL;//pBuf->dacx_out;
#endif
	PCM_DATA_TYPE *i2s_out       	= NULL;//pBuf->i2s0_out;
#ifdef CFG_RES_AUDIO_SPDIFOUT_EN
	int16_t *spdif_out       		= NULL;//pBuf->i2s0_out;
#endif
	if(pAudioCore->AudioSource[APP_SOURCE_NUM].Active == TRUE)////music buff
	{
		music_pcm = pAudioCore->AudioSource[APP_SOURCE_NUM].PcmInBuf;// include line/bt/usb/sd/spdif/hdmi/i2s/radio source
	}
	if(pAudioCore->AudioSource[MIC_SOURCE_NUM].Active == TRUE)////mic buff
	{
		mic_pcm =  pAudioCore->AudioSource[MIC_SOURCE_NUM].PcmInBuf;//˫mic����
	}

    if(pAudioCore->AudioSink[AUDIO_DAC0_SINK_NUM].Active == TRUE)   ////dac0 buff
	{
    	monitor_out = pAudioCore->AudioSink[AUDIO_DAC0_SINK_NUM].PcmOutBuf;
	}
#ifdef CFG_RES_AUDIO_DACX_EN
	if(pAudioCore->AudioSink[AUDIO_DACX_SINK_NUM].Active == TRUE)   ////dacx buff
	{
		record_out = pAudioCore->AudioSink[AUDIO_DACX_SINK_NUM].PcmOutBuf;
	}
#endif	
#if defined(CFG_FUNC_REMIND_SOUND_EN)
	if(pAudioCore->AudioSource[REMIND_SOURCE_NUM].Active == TRUE)////remind buff
	{
		remind_in = pAudioCore->AudioSource[REMIND_SOURCE_NUM].PcmInBuf;
	}
#endif
#ifdef CFG_RES_AUDIO_I2SOUT_EN
	if(pAudioCore->AudioSink[AUDIO_I2SOUT_SINK_NUM].Active == TRUE) ////i2s buff
	{
		i2s_out = (int16_t *)pAudioCore->AudioSink[AUDIO_I2SOUT_SINK_NUM].PcmOutBuf;
	}
#endif

#ifdef CFG_RES_AUDIO_SPDIFOUT_EN
	if(pAudioCore->AudioSink[AUDIO_SPDIF_SINK_NUM].Active == TRUE) ////i2s buff
	{
		spdif_out = (int16_t *)pAudioCore->AudioSink[AUDIO_SPDIF_SINK_NUM].PcmOutBuf;
	}
#endif

    if(monitor_out)
	{
		memset(monitor_out, 0, n * sizeof(PCM_DATA_TYPE) * 2);
    }

	//DAC0������������Ч����
	if(monitor_out || i2s_out)
	{
		if(monitor_out == NULL)
			monitor_out = i2s_out;

		//ϵͳ������ƣ����ݴӹ����飬��ɢ�ŵ���Ч������������
		AudioCoreAppSourceVolApply(APP_SOURCE_NUM, (int16_t *)music_pcm, n, 2);
		#ifdef CFG_FUNC_REMIND_SOUND_EN
		AudioCoreAppSourceVolApply(REMIND_SOURCE_NUM, (int16_t *)remind_in, n, 2);
		#endif

		for(s = 0; s < n; s++)
		{
			#if defined(CFG_FUNC_REMIND_SOUND_EN)
			if(remind_in)
			{
				monitor_out[2*s + 0] = remind_in[2*s + 0];
				monitor_out[2*s + 1] = remind_in[2*s + 1];
			}
			else
			#endif			
			if(mic_pcm)
			{
				monitor_out[2*s + 0] = mic_pcm[2*s + 0];
				monitor_out[2*s + 1] = mic_pcm[2*s + 1];
			}

			if(music_pcm)
			{
				monitor_out[2*s + 0] = __nds32__clips((((int32_t)monitor_out[2 * s + 0] + (int32_t)music_pcm[2 * s + 0])), AUDIO_WIDTH-1);
				monitor_out[2*s + 1] = __nds32__clips((((int32_t)monitor_out[2 * s + 1] + (int32_t)music_pcm[2 * s + 1])), AUDIO_WIDTH-1);
			}
		}

		if(i2s_out && i2s_out != monitor_out)
		{
			for(s = 0; s < n; s++)
			{
				i2s_out[2*s + 0] = monitor_out[2*s + 0];
				i2s_out[2*s + 1] = monitor_out[2*s + 1];
			}
		}		
#ifdef CFG_RES_AUDIO_SPDIFOUT_EN
		if(spdif_out && spdif_out != monitor_out)
		{
			for(s = 0; s < n; s++)
			{
				spdif_out[2*s + 0] = monitor_out[2*s + 0];
				spdif_out[2*s + 1] = monitor_out[2*s + 1];
			}
		}
#endif		
	}

#ifdef CFG_RES_AUDIO_DACX_EN
	//DAC X
	if(record_out && monitor_out)
	{
		for(s = 0; s < n; s++)
		{
			record_out[s] = __nds32__clips((( (int32_t)monitor_out[2*s+0] + (int32_t)monitor_out[2*s+1])), AUDIO_WIDTH-1);
		}		
	}
#endif	
}


void AudioNoAppProcess(AudioCoreContext *pAudioCore)
{
	int16_t  s;
	uint16_t n = AudioCoreFrameSizeGet(AudioCore.CurrentMix);
	int16_t *mic_pcm    	= NULL;//pBuf->mic_in;///mic input
	int16_t *monitor_out    = NULL;//pBuf->dac0_out;
	int16_t *remind_in 		= NULL;
#ifdef CFG_RES_AUDIO_DACX_EN
	int16_t *record_out     = NULL;//pBuf->dacx_out;
#endif
	int16_t *i2s_out       	= NULL;//pBuf->i2s0_out;
#ifdef CFG_RES_AUDIO_SPDIFOUT_EN
	int16_t *spdif_out       = NULL;//pBuf->spdif_out;
#endif
	if(pAudioCore->AudioSource[MIC_SOURCE_NUM].Active == TRUE)////mic buff
	{
		mic_pcm = (int16_t *)pAudioCore->AudioSource[MIC_SOURCE_NUM].PcmInBuf;//˫mic����
	}

    if(pAudioCore->AudioSink[AUDIO_DAC0_SINK_NUM].Active == TRUE)   ////dac0 buff
	{
    	monitor_out = (int16_t *)pAudioCore->AudioSink[AUDIO_DAC0_SINK_NUM].PcmOutBuf;
	}
#ifdef CFG_RES_AUDIO_DACX_EN
	if(pAudioCore->AudioSink[AUDIO_DACX_SINK_NUM].Active == TRUE)   ////dacx buff
	{
		record_out = (int16_t *)pAudioCore->AudioSink[AUDIO_DACX_SINK_NUM].PcmOutBuf;
	}
#endif	

#if defined(CFG_FUNC_REMIND_SOUND_EN)
	if(pAudioCore->AudioSource[REMIND_SOURCE_NUM].Active == TRUE)////remind buff
	{
		remind_in = (int16_t *)pAudioCore->AudioSource[REMIND_SOURCE_NUM].PcmInBuf;
	}
#endif
#ifdef CFG_RES_AUDIO_I2SOUT_EN
	if(pAudioCore->AudioSink[AUDIO_I2SOUT_SINK_NUM].Active == TRUE)	////i2s buff
	{
		i2s_out = (int16_t *)pAudioCore->AudioSink[AUDIO_I2SOUT_SINK_NUM].PcmOutBuf;
	}
#endif

#ifdef CFG_RES_AUDIO_SPDIFOUT_EN
	if(pAudioCore->AudioSink[AUDIO_SPDIF_SINK_NUM].Active == TRUE)	////i2s buff
	{
		spdif_out = (int16_t *)pAudioCore->AudioSink[AUDIO_SPDIF_SINK_NUM].PcmOutBuf;
	}
#endif

    if(monitor_out)
	{
		memset(monitor_out, 0, n * 2 * 2);
    }


	if(monitor_out || i2s_out)
	{
		if(monitor_out == NULL)
			monitor_out = i2s_out;
		for(s = 0; s < n; s++)
		{
			#if defined(CFG_FUNC_REMIND_SOUND_EN)
			if(remind_in)
			{
				monitor_out[2*s + 0] = remind_in[2*s + 0];
				monitor_out[2*s + 1] = remind_in[2*s + 1];
			}
			else
			#endif			
			if(mic_pcm)
			{
				monitor_out[2*s + 0] = mic_pcm[2*s + 0];
				monitor_out[2*s + 1] = mic_pcm[2*s + 1];
			}
			else
			{
				monitor_out[2*s + 0] = 0;
				monitor_out[2*s + 1] = 0;
			}
		}

		if(i2s_out && i2s_out != monitor_out)
		{
			for(s = 0; s < n; s++)
			{
				i2s_out[2*s + 0] = monitor_out[2*s + 0];
				i2s_out[2*s + 1] = monitor_out[2*s + 1];
			}
		}
#ifdef CFG_RES_AUDIO_SPDIFOUT_EN
		if(spdif_out && spdif_out != monitor_out)
		{
			for(s = 0; s < n; s++)
			{
				spdif_out[2*s + 0] = monitor_out[2*s + 0];
				spdif_out[2*s + 1] = monitor_out[2*s + 1];
			}
		}
#endif		
	}

#ifdef CFG_RES_AUDIO_DACX_EN
	//DAC X
	if(record_out && monitor_out)
	{
		for(s = 0; s < n; s++)
		{
			record_out[s] = __nds32__clips((( (int32_t)monitor_out[2*s+0] + (int32_t)monitor_out[2*s+1])), 16-1);					    
		}	
	}
#endif	//CFG_RES_AUDIO_DACX_EN
}

