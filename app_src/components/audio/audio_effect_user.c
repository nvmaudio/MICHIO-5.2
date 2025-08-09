#include <string.h>
#include <nds32_intrinsic.h>
#include <stdlib.h>
#include "math.h"
#include "debug.h"
#include "app_config.h"
#include "audio_effect_api.h"
#include "audio_effect_library.h"
#include "audio_effect.h"
#include "ctrlvars.h"
#include "comm_param.h"
#include "audio_effect_user.h"
#include "eq_params.h"
#include "main_task.h"


#include "sys_param.h"



ControlVariablesUserContext gCtrlUserVars;

#ifdef CFG_FUNC_MUSIC_TREB_BASS_EN
	EQUnit 			*FreEQ_DAC0 = NULL;
	EQUnit			*FreEQ_DACX	= NULL;
#endif



#ifdef CFG_FUNC_MUSIC_EQ_MODE_EN
EQUnit 				*music_mode_eq_unit = NULL;//eq mode
EQDRCUnit 			*music_mode_eq_drc_unit = NULL;//eq drc mode
#endif
#ifdef CFG_FUNC_MIC_AUTOTUNE_STEP_EN
AutoTuneUnit 		*mic_autotune_unit = NULL;//
#endif
#ifdef CFG_FUNC_MIC_CHORUS_STEP_EN
ChorusUnit	 		*mic_chorus_unit = NULL;//
#endif
#ifdef CFG_FUNC_MIC_ECHO_STEP_EN
EchoUnit 			*mic_echo_unit = NULL;
#endif
#ifdef CFG_FUNC_MIC_REVERB_STEP_EN
ReverbUnit			*mic_reverb_unit = NULL;
#endif
#ifdef CFG_FUNC_MIC_PLATE_REVERB_STEP_EN
PlateReverbUnit 	*mic_platereverb_unit = NULL;
#endif
#ifdef CFG_FUNC_MIC_REVERB_PRO_STEP_EN
ReverbProUnit 		*mic_reverbpro_unit = NULL;
#endif

#ifdef CFG_FUNC_MUSIC_EQ_MODE_EN

// EQ �������غ���
void LoadEqMode(const uint8_t *buff)
{
#ifndef MUSIC_EQ_DRC
	if(music_mode_eq_unit == NULL){
		return;
	}
	memcpy(&music_mode_eq_unit->param, buff, EQ_PARAM_LEN);
	AudioEffectEQFilterConfig(music_mode_eq_unit, gCtrlVars.sample_rate);
	gCtrlVars.AutoRefresh = 1;//////����ʱģʽ�����ı䣬��λ�����Զ���ȡ��Ч���ݣ�1=������λ����0=����Ҫ��λ����ȡ
#else
	if(music_mode_eq_drc_unit == NULL){
		return;
	}
	memcpy(&music_mode_eq_drc_unit->param, buff, EQDRC_PARAM_LEN);
	AudioEffectEQDRCInit(music_mode_eq_drc_unit, 2, gCtrlVars.sample_rate);
	gCtrlVars.AutoRefresh = 1;//////����ʱģʽ�����ı䣬��λ�����Զ���ȡ��Ч���ݣ�1=������λ����0=����Ҫ��λ����ȡ
#endif	
}

//EQ Mode���ں���
void EqModeSet(uint8_t EqMode)
{
    switch(EqMode)
	{
#ifndef MUSIC_EQ_DRC
		case EQ_MODE_FLAT:
			LoadEqMode(&Flat[0]);
			break;
		case EQ_MODE_CLASSIC:
			LoadEqMode(&Classical[0]);
			break;
		case EQ_MODE_POP:
			LoadEqMode(&Pop[0]);
			break;
		case EQ_MODE_ROCK:
			LoadEqMode(&Rock[0]);
			break;
		case EQ_MODE_JAZZ:
			LoadEqMode(&Jazz[0]);
			break;
		case EQ_MODE_VOCAL_BOOST:
			LoadEqMode(&Vocal_Booster[0]);
			break;
#else
		case EQ_MODE_FLAT:
			LoadEqMode(&Flat_EQDRC[0]);
			break;
		case EQ_MODE_CLASSIC:
			LoadEqMode(&Classical_EQDRC[0]);
			break;
		case EQ_MODE_POP:
			LoadEqMode(&Pop_EQDRC[0]);
			break;
		case EQ_MODE_ROCK:
			LoadEqMode(&Rock_EQDRC[0]);
			break;
		case EQ_MODE_JAZZ:
			LoadEqMode(&Jazz_EQDRC[0]);
			break;
		case EQ_MODE_VOCAL_BOOST:
			LoadEqMode(&Vocal_Booster_EQDRC[0]);
			break;
#endif
		default:
			break;
	}
}
#endif




#ifdef CFG_FUNC_MUSIC_TREB_BASS_EN



	int16_t tinh_gain(int16_t VR_Gain, int16_t buoc_eq)
	{
		float buoc_eq_float = (float)buoc_eq;						// Tổng dB mỗi bên, dùng kiểu float cho độ chính xác
		float gain_dB = ((VR_Gain - 16) / 16.0f) * buoc_eq_float;	// Tính gain dB theo độ lệch từ mức giữa (16), chia cho 16.0 để chuẩn hóa
		return (int16_t)(gain_dB * 256);							// Chuyển sang định dạng Q8.8
	}


	
	void MusicBassTrebAjust(int16_t BassGain, int16_t MidGain,	int16_t TrebGain)
	{

		DBG("[MusicBassTrebAjust]   BassGain: %d MidGain: %d TrebGain: %d \n",BassGain,MidGain,TrebGain);

		#include "bt_app_tws.h"
		tws_master_music_bass_treb_send( BassGain, MidGain , TrebGain );

		if(FreEQ_DAC0==NULL)
		{
			DBG("[MusicBassTrebAjust]   FreEQ_DAC0 is Null\n");
			return;
		}
	
		int16_t buoc_eq = sys_parameter.Buoc_eq;

		#include "michio.h"

		if(sys_parameter.Vr_dac_line == 0 )
		{
			FreEQ_DAC0->param.eq_params[1].gain =  tinh_gain(BassGain,buoc_eq);
			FreEQ_DAC0->param.eq_params[2].gain =  tinh_gain(MidGain,buoc_eq);
			FreEQ_DAC0->param.eq_params[3].gain =  tinh_gain(TrebGain,buoc_eq); 

			#if	CFG_AUDIO_EFFECT_EQ_EN
				AudioEffectEQFilterConfig(FreEQ_DAC0, gCtrlVars.sample_rate);
			#endif
		}
		
		if(sys_parameter.Vr_dac_line == 1 )
		{
			if(FreEQ_DACX==NULL)
			{
				DBG("[MusicBassTrebAjust]   FreEQ_DACX is Null\n");
				return;
			}

			FreEQ_DACX->param.eq_params[1].gain =  	tinh_gain(BassGain,buoc_eq);
			FreEQ_DAC0->param.eq_params[2].gain = 	tinh_gain(MidGain,buoc_eq);
			FreEQ_DAC0->param.eq_params[3].gain =   tinh_gain(TrebGain,buoc_eq);
			#if	CFG_AUDIO_EFFECT_EQ_EN
				AudioEffectEQFilterConfig(FreEQ_DACX, gCtrlVars.sample_rate);
				AudioEffectEQFilterConfig(FreEQ_DAC0, gCtrlVars.sample_rate);
			#endif

		}

		if(sys_parameter.Vr_dac_line == 2 )
		{
			if(FreEQ_DACX==NULL)
			{
				DBG("[MusicBassTrebAjust]   FreEQ_DACX is Null\n");
				return;
			}

			FreEQ_DACX->param.eq_params[1].gain =  	0;
			FreEQ_DACX->param.pregain			=  	tinh_gain(BassGain,buoc_eq);

			FreEQ_DAC0->param.eq_params[2].gain = 	tinh_gain(MidGain,buoc_eq);
			FreEQ_DAC0->param.eq_params[3].gain =   tinh_gain(TrebGain,buoc_eq);

			#if	CFG_AUDIO_EFFECT_EQ_EN
				AudioEffectEQPregainConfig(FreEQ_DACX);
				AudioEffectEQFilterConfig(FreEQ_DAC0, gCtrlVars.sample_rate);
			#endif

		}

		if(sys_parameter.Vr_dac_line == 3 )
		{
			if(FreEQ_DAC0!=NULL)
			{
				int16_t buoc_eq2 = (int16_t)(sys_parameter.Buoc_eq * ((float)sys_parameter.Heso / 10.0f));

				FreEQ_DAC0->param.eq_params[1].gain =  	tinh_gain(BassGain,buoc_eq2);
				FreEQ_DAC0->param.eq_params[2].gain = 	tinh_gain(MidGain,buoc_eq);
				FreEQ_DAC0->param.eq_params[3].gain =   tinh_gain(TrebGain,buoc_eq);

				#if	CFG_AUDIO_EFFECT_EQ_EN
					AudioEffectEQFilterConfig(FreEQ_DAC0, gCtrlVars.sample_rate);
				#endif
			}
			if(FreEQ_DACX!=NULL)
			{
				FreEQ_DACX->param.eq_params[1].gain =  	tinh_gain(BassGain,buoc_eq);
				#if	CFG_AUDIO_EFFECT_EQ_EN
					AudioEffectEQFilterConfig(FreEQ_DACX, gCtrlVars.sample_rate);
					//AudioEffectEQFilterConfig(FreEQ_DAC0, gCtrlVars.sample_rate);
				#endif
			}
		}
		if(sys_parameter.Vr_dac_line == 4 )
		{
			if(FreEQ_DAC0!=NULL)
			{
				int16_t buoc_eq2 = (int16_t)(sys_parameter.Buoc_eq * ((float)sys_parameter.Heso / 10.0f));

				FreEQ_DAC0->param.eq_params[1].gain =  	tinh_gain(BassGain,buoc_eq2);
				FreEQ_DAC0->param.eq_params[2].gain = 	tinh_gain(MidGain,buoc_eq);
				FreEQ_DAC0->param.eq_params[3].gain =   tinh_gain(TrebGain,buoc_eq);
				#if	CFG_AUDIO_EFFECT_EQ_EN
					AudioEffectEQFilterConfig(FreEQ_DAC0, gCtrlVars.sample_rate);
				#endif
			}
			if(FreEQ_DACX!=NULL)
			{
				FreEQ_DACX->param.eq_params[1].gain =  	0;
				FreEQ_DACX->param.pregain			=  	tinh_gain(BassGain,buoc_eq);

				#if	CFG_AUDIO_EFFECT_EQ_EN
					AudioEffectEQPregainConfig(FreEQ_DACX);
					//AudioEffectEQFilterConfig(FreEQ_DAC0, gCtrlVars.sample_rate);
				#endif
			}
		}
 



		gCtrlVars.AutoRefresh = 1;
	}
#endif









#ifdef CFG_FUNC_MIC_AUTOTUNE_STEP_EN
#if CFG_AUDIO_EFFECT_AUTO_TUNE_EN
void AutoTuneStepSet(uint8_t AutoTuneStep)
{
	if(mic_autotune_unit == NULL)
	{
		return;
	}
	mic_autotune_unit->param.key = AutoTuneKeyTbl[AutoTuneStep];
	AudioEffectAutoTuneInit(mic_autotune_unit, mic_autotune_unit->channel, gCtrlVars.sample_rate);

	gCtrlVars.AutoRefresh = 1;
}
#endif
#endif



#ifdef CFG_FUNC_MIC_CHORUS_STEP_EN
#if CFG_AUDIO_EFFECT_CHORUS_EN
void ChorusStepSet(uint8_t ChorusStep)
{
	uint16_t step;
	if(mic_chorus_unit == NULL)
	{
		return;
	}
    step = gCtrlUserVars.max_chorus_wet / MAX_MIC_EFFECT_DELAY_STEP;
	if(ChorusStep >= (MAX_MIC_EFFECT_DELAY_STEP-1))
	{
		mic_chorus_unit->param.wet = gCtrlUserVars.max_chorus_wet;
	}
	else
	{
		mic_chorus_unit->param.wet = ChorusStep * step;
	}
}
#endif
#endif

#ifdef CFG_FUNC_MIC_ECHO_STEP_EN
#if CFG_AUDIO_EFFECT_ECHO_EN
void EchoStepSet(uint8_t EchoStep)
{
	uint16_t step;

	if(mic_echo_unit == NULL)
	{
		return;
	}
	step = gCtrlUserVars.max_echo_delay / MAX_MIC_EFFECT_DELAY_STEP;
	if(EchoStep >= (MAX_MIC_EFFECT_DELAY_STEP-1))
	{
		//gCtrlVars.echo_unit.delay_samples = gCtrlUserVars.max_echo_delay;
		mic_echo_unit->param.delay = gCtrlUserVars.max_echo_delay;
	}
	else
	{
		//gCtrlVars.echo_unit.delay_samples = EchoStep * step;
		mic_echo_unit->param.delay = EchoStep * step;
	}
	step = gCtrlUserVars.max_echo_depth/ MAX_MIC_EFFECT_DELAY_STEP;
	if(EchoStep >= (MAX_MIC_EFFECT_DELAY_STEP-1))
	{
		mic_echo_unit->param.attenuation = gCtrlUserVars.max_echo_depth;
	}
	else
	{
		mic_echo_unit->param.attenuation = EchoStep * step;
	}

	gCtrlVars.AutoRefresh = 1;
}
#endif
#endif

#ifdef CFG_FUNC_MIC_REVERB_STEP_EN
#if CFG_AUDIO_EFFECT_REVERB_EN
void ReverbStepSet(uint8_t ReverbStep)
{
	uint16_t step;

	if(mic_reverb_unit == NULL)
	{
		return;
	}
    step = gCtrlUserVars.max_reverb_wet_scale/ MAX_MIC_EFFECT_DELAY_STEP;
	if(ReverbStep >= (MAX_MIC_EFFECT_DELAY_STEP-1))
	{
		mic_reverb_unit->param.wet_scale = gCtrlUserVars.max_reverb_wet_scale;
	}
	else
	{
		mic_reverb_unit->param.wet_scale = ReverbStep * step;
	}
    step = gCtrlUserVars.max_reverb_roomsize/ MAX_MIC_EFFECT_DELAY_STEP;
	if(ReverbStep >= (MAX_MIC_EFFECT_DELAY_STEP-1))
	{
		mic_reverb_unit->param.roomsize_scale = gCtrlUserVars.max_reverb_roomsize;
	}
	else
	{
		mic_reverb_unit->param.roomsize_scale = ReverbStep * step;
	}

	AudioEffectReverbConfig(mic_reverb_unit);

	gCtrlVars.AutoRefresh = 1;
}
#endif
#endif

#ifdef CFG_FUNC_MIC_PLATE_REVERB_STEP_EN
#if CFG_AUDIO_EFFECT_PLATE_REVERB_EN
void PlateReverbStepSet(uint8_t ReverbStep)
{
	uint16_t step;

	if(mic_platereverb_unit == NULL)
	{
		return;
	}
	step = gCtrlUserVars.max_plate_reverb_roomsize / MAX_MIC_EFFECT_DELAY_STEP;
	if(ReverbStep >= (MAX_MIC_EFFECT_DELAY_STEP-1))
	{
		mic_platereverb_unit->param.decay = gCtrlUserVars.max_plate_reverb_roomsize;
	}
	else
	{
		mic_platereverb_unit->param.decay = ReverbStep * step;
	}
	//APP_DBG("mic_wetdrymix   = %d\n",gCtrlVars.max_plate_reverb_wetdrymix);
	step = gCtrlUserVars.max_plate_reverb_wetdrymix / MAX_MIC_EFFECT_DELAY_STEP;
	if(ReverbStep >= (MAX_MIC_EFFECT_DELAY_STEP-1))
	{
		mic_platereverb_unit->param.wetdrymix = gCtrlUserVars.max_plate_reverb_wetdrymix;
	}
	else
	{
		mic_platereverb_unit->param.wetdrymix = ReverbStep * step;
	}

	AudioEffectPlateReverbConfig(mic_platereverb_unit);

	gCtrlVars.AutoRefresh = 1;
}
#endif
#endif

#ifdef CFG_FUNC_MIC_REVERB_PRO_STEP_EN
#if CFG_AUDIO_EFFECT_REVERB_PRO_EN
void ReverbProStepSet(uint8_t ReverbStep)
{
	uint16_t step,r;

	if(mic_reverbpro_unit == NULL)
	{
		return;
	}

	//+0  ~~~ -70
    r = abs(gCtrlUserVars.max_reverb_pro_wet);
    r = 70-r;
    step = r / MAX_MIC_EFFECT_DELAY_STEP;

	if(ReverbStep >= (MAX_MIC_EFFECT_DELAY_STEP-1))
	{
		mic_reverbpro_unit->param.wet = gCtrlUserVars.max_reverb_pro_wet;
	}
	else
	{
		r = MAX_MIC_EFFECT_DELAY_STEP - 1 - ReverbStep;
		r*= step;
		mic_reverbpro_unit->param.wet = gCtrlUserVars.max_reverb_pro_wet - r;

		if(ReverbStep == 0) mic_reverbpro_unit->param.wet = -70;
	}

    r = abs(gCtrlUserVars.max_reverb_pro_erwet);
    r = 70-r;
    step = r / MAX_MIC_EFFECT_DELAY_STEP;

	if(ReverbStep >= (MAX_MIC_EFFECT_DELAY_STEP-1))
	{
		mic_reverbpro_unit->param.erwet = gCtrlUserVars.max_reverb_pro_erwet;
	}
	else
	{
		r = MAX_MIC_EFFECT_DELAY_STEP - 1 - ReverbStep;
		r*= step;
		mic_reverbpro_unit->param.erwet = gCtrlUserVars.max_reverb_pro_erwet - r;

		if(ReverbStep == 0) mic_reverbpro_unit->param.erwet = -70;
	}

	AudioEffectReverbProInit(mic_reverbpro_unit, mic_reverbpro_unit->channel, gCtrlVars.sample_rate);

	gCtrlVars.AutoRefresh = 1;
}
#endif
#endif

#ifdef CFG_FUNC_MIC_EFFECT_DELAY_EN
void AudioEffectDelaySet(int16_t step)
{
	#ifdef CFG_FUNC_MIC_CHORUS_STEP_EN
	ChorusStepSet(step);
	#endif
	#ifdef CFG_FUNC_MIC_ECHO_STEP_EN
	EchoStepSet(step);
	#endif
	#ifdef CFG_FUNC_MIC_REVERB_STEP_EN
	ReverbStepSet(step);
	#endif
	#ifdef CFG_FUNC_MIC_PLATE_REVERB_STEP_EN
	PlateReverbStepSet(step);
	#endif
	#ifdef CFG_FUNC_MIC_REVERB_PRO_STEP_EN
	ReverbProStepSet(step);
	#endif
}
#endif


//�û��ֶ�������Чָ�븳��ֵΪNULL
//��Ч�л�����Ҫ��������һ��
void AudioEffectUserNodeInit(void)
{
	#ifdef CFG_FUNC_MUSIC_TREB_BASS_EN
		FreEQ_DAC0 = NULL;
		FreEQ_DACX = NULL;
	#endif

#ifdef CFG_FUNC_MUSIC_EQ_MODE_EN
	music_mode_eq_unit = NULL;//eq mode
	music_mode_eq_drc_unit = NULL;//eq mode
#endif
#ifdef CFG_FUNC_MIC_AUTOTUNE_STEP_EN
	mic_autotune_unit = NULL;//
#endif
#ifdef CFG_FUNC_MIC_CHORUS_STEP_EN
	mic_chorus_unit = NULL;//
#endif
#ifdef CFG_FUNC_MIC_ECHO_STEP_EN
 	mic_echo_unit = NULL;
#endif
#ifdef CFG_FUNC_MIC_REVERB_STEP_EN
	mic_reverb_unit = NULL;
#endif
#ifdef CFG_FUNC_MIC_PLATE_REVERB_STEP_EN
	mic_platereverb_unit = NULL;
#endif
#ifdef CFG_FUNC_MIC_REVERB_PRO_STEP_EN
	mic_reverbpro_unit = NULL;
#endif
}

void DefaultParamgsUserInit(void)
{
//#ifdef CFG_FUNC_MUSIC_EQ_MODE_EN
//	mainAppCt.EqMode = EQ_MODE_FLAT;
//#endif
//#ifdef CFG_FUNC_MUSIC_TREB_BASS_EN
//	mainAppCt.MicBassStep = 7;
//	mainAppCt.MicTrebStep = 7;
//#endif
//#ifdef CFG_FUNC_MIC_AUTOTUNE_STEP_EN
//	mainAppCt.MicAutoTuneStep = 2;
//#endif
//
//#ifdef CFG_FUNC_MIC_EFFECT_DELAY_EN
//	mainAppCt.MicEffectDelayStep = MAX_MIC_EFFECT_DELAY_STEP;
//#endif
}

//�û������Ч���ڲ���ͬ������
//����AudioEffectModeSel()������Чģʽ֮����Ҫ�ٵ����´˺�������֤�û�������Ч����ͬ��

void AudioEffectParamSync(void)
{
#ifdef CFG_FUNC_MUSIC_EQ_MODE_EN
	EqModeSet(mainAppCt.EqMode);
#endif
#ifdef CFG_FUNC_MUSIC_TREB_BASS_EN
MusicBassTrebAjust(mainAppCt.MusicBassStep, mainAppCt.MusicMidStep,mainAppCt.MusicTrebStep);
#endif
#ifdef CFG_FUNC_MIC_AUTOTUNE_STEP_EN
	AutoTuneStepSet(mainAppCt.MicAutoTuneStep);
#endif
#ifdef CFG_FUNC_MIC_EFFECT_DELAY_EN
	AudioEffectDelaySet(mainAppCt.MicEffectDelayStep);
#endif
#ifdef CFG_FUNC_AUDIO_EFFECT_EN
	{
		extern bool IsEffectChange;
		IsEffectChange = 0;//EQģʽ�л��л�EQģʽ�������£�����Ҫ������Ч���ڴ��ʼ������
	}
#endif
}
