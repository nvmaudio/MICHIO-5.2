/**
 **************************************************************************************
 * @file    comm_param.h
 * @brief
 *
 * @author  Sam
 * @version V1.0.0
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#ifndef __COMM_PARAM_H__
#define __COMM_PARAM_H__

#ifdef  __cplusplus
extern "C" {
#endif//__cplusplus

#include "type.h"
#include "audio_effect_api.h"
#include "audio_effect.h"

typedef struct _EffectComCt
{
	uint8_t  	channel;
	NodeType 	AudioNodeType;
	EffectType 	AudioEffectType;
	char* 		EffectName;
	uint8_t		EffectWidth;
	uint8_t  	Index;//�����Ҫ��ĳ����Ч�����⴦��д��ʵ�ʵ�index���, ������0xFF
}EffectComCt;

//��Ч���Ƶ��ַ�����󳤶�Ϊ32���ַ�
#define EFFECT_LIST_NAME_MAX	32


extern const EffectComCt AudioEffectCommParam[];
extern const EffectComCt AudioEffectAECParam_HFP[];
extern const unsigned char CommParam[1146];
extern const unsigned char AECParam_HFP[328];

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif//__COMM_PARAM_H__

