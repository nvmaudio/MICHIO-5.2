/**
 **************************************************************************************
 * @file    audio_effect_flash_param.h
 * @brief
 *
 * @author  Sam
 * @version V1.0.0
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
 
#ifndef __AUDIO_EFFECT_FLASH_PARAM_H__
#define __AUDIO_EFFECT_FLASH_PARAM_H__
 
#ifdef  __cplusplus
extern "C" {
#endif//__cplusplus

#include "type.h"
#include "audio_effect_api.h"

#pragma pack (1)
typedef struct _EffectPaCt{
	int8_t 		Flag;//��Ч�Ƿ�ʹ��
	int16_t 	Offset;//��ַƫ����
	int16_t		Len;//������Ч����
}EffectPaCt;
#pragma pack ()

typedef struct _EffectHeadPaCt{
	int32_t 	Head;//ͷ����Ϣ
	int32_t 	Num;//��Ч����Ч����
	int32_t		MaxLen;//������󳤶�
}EffectHeadPaCt;

//extern uint8_t EffectParamFlashIndex;
extern uint8_t EffectParamFlashUpdataFlag;

void EffectParamFlashUpdata(void);

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif//__AUDIO_EFFECT_FLASH_PARAM_H__

