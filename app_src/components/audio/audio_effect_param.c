#include "type.h"
#include "comm_param.h"
#include "audio_effect_param.h"
#include "audio_effect_api.h"
#include "audio_effect.h"
#include "app_config.h"

#ifdef CFG_FUNC_AUDIO_EFFECT_ONLINE_TUNING_EN

AudioEffectCommParamBuffContext	gAudioEffectCommParamBuffVars;
AudioEffectAECParam_HFPBuffContext	gAudioEffectAECParam_HFPBuffVars;
extern void* effect_list_param[AUDIO_EFFECT_SUM];

void AudioEffectParaminit(uint8_t* CommParam)
{
	uint32_t i;
	for(i=0;i<AUDIO_EFFECT_SUM;i++)
	{
		effect_list_param[i] = NULL;
	}
	if((EffectComCt*)CommParam == AudioEffectCommParam)
	{
		effect_list_param[0] = &gAudioEffectCommParamBuffVars.paramGainControl_0;
		effect_list_param[1] = &gAudioEffectCommParamBuffVars.paramGainControl_1;
		effect_list_param[2] = &gAudioEffectCommParamBuffVars.paramGainControl_2;
		effect_list_param[3] = &gAudioEffectCommParamBuffVars.paramGainControl_3;
		effect_list_param[4] = &gAudioEffectCommParamBuffVars.paramGainControl_4;
		effect_list_param[5] = &gAudioEffectCommParamBuffVars.paramGainControl_5;
		effect_list_param[6] = &gAudioEffectCommParamBuffVars.paramGainControl_6;
		effect_list_param[7] = &gAudioEffectCommParamBuffVars.paramExpander_7;
		effect_list_param[8] = &gAudioEffectCommParamBuffVars.paramSilenceDetector_8;
		effect_list_param[9] = &gAudioEffectCommParamBuffVars.paramCompander_9;
		effect_list_param[10] = &gAudioEffectCommParamBuffVars.paramLLCompressor_10;
		effect_list_param[11] = &gAudioEffectCommParamBuffVars.paramVB_11;
		effect_list_param[12] = &gAudioEffectCommParamBuffVars.paramVBClassic_12;
		effect_list_param[13] = &gAudioEffectCommParamBuffVars.paramThreeD_13;
		effect_list_param[14] = &gAudioEffectCommParamBuffVars.paramExciter_14;
		effect_list_param[15] = &gAudioEffectCommParamBuffVars.paramStereoWidener_15;
		effect_list_param[16] = &gAudioEffectCommParamBuffVars.paramEQ_16;
		effect_list_param[17] = &gAudioEffectCommParamBuffVars.paramEQ_17;
		effect_list_param[18] = &gAudioEffectCommParamBuffVars.paramDynamicEq_18;
		effect_list_param[19] = &gAudioEffectCommParamBuffVars.paramEQ_19;
		effect_list_param[20] = &gAudioEffectCommParamBuffVars.paramEQDRC_20;
		effect_list_param[21] = &gAudioEffectCommParamBuffVars.paramEQ_21;
		effect_list_param[22] = &gAudioEffectCommParamBuffVars.paramGainControl_22;
		effect_list_param[23] = &gAudioEffectCommParamBuffVars.paramLLCompressor_23;
		effect_list_param[24] = &gAudioEffectCommParamBuffVars.paramPhaseControl_24;
		effect_list_param[25] = &gAudioEffectCommParamBuffVars.paramVB_25;
		effect_list_param[26] = &gAudioEffectCommParamBuffVars.paramVBClassic_26;
		effect_list_param[27] = &gAudioEffectCommParamBuffVars.paramEQ_27;
		effect_list_param[28] = &gAudioEffectCommParamBuffVars.paramEQ_28;
		effect_list_param[29] = &gAudioEffectCommParamBuffVars.paramDynamicEq_29;
		effect_list_param[30] = &gAudioEffectCommParamBuffVars.paramEQ_30;
		effect_list_param[31] = &gAudioEffectCommParamBuffVars.paramEQDRC_31;
		effect_list_param[32] = &gAudioEffectCommParamBuffVars.paramEQ_32;
		effect_list_param[33] = &gAudioEffectCommParamBuffVars.paramGainControl_33;

	}
	else if((EffectComCt*)CommParam == AudioEffectAECParam_HFP)
	{
		effect_list_param[0] = &gAudioEffectAECParam_HFPBuffVars.paramExpander_0;
		effect_list_param[1] = &gAudioEffectAECParam_HFPBuffVars.paramEQ_1;
		effect_list_param[2] = &gAudioEffectAECParam_HFPBuffVars.paramAEC_2;
		effect_list_param[3] = &gAudioEffectAECParam_HFPBuffVars.paramGainControl_3;
		effect_list_param[4] = &gAudioEffectAECParam_HFPBuffVars.paramDRC_4;

	}
}
#endif
