/*
 * audio_effect_class.h
 *
 *  Created on: Jul 2, 2024
 *      Author: szsj-1
 */

#ifndef APP_SRC_COMPONENTS_AUDIO_AUDIO_EFFECT_CLASS_H_
#define APP_SRC_COMPONENTS_AUDIO_AUDIO_EFFECT_CLASS_H_
//--------------------------------------//
#include "audio_effect_user_config.h"
#include "audio_effect_library.h"
#include "audio_effect.h"

//--------------------------------------//
enum
{
    boolType = 0,
	enumType,
	continueType,
	dispType,

};

typedef  void (*EffectFunc)(uint8_t , uint32_t , uint8_t *, uint32_t);

typedef struct _ParaNumbers_
{
	int32_t     paramete_totals;
}ParaNumbers;

typedef struct _TypeBool_
{
	//int32_t     index;
	int32_t     data_type;
	int32_t     defualt;
	char        *name;
}TypeBool;

typedef struct _TypeEnum_
{
	//int32_t     index;
	int32_t     data_type;
	char        *enum_name;
	int32_t     defualt;
	char        *name;
}TypeEnum;

typedef struct _TypeContinue_
{
	//int32_t     index;
	int32_t     data_type;
	int32_t     min;
	int32_t     max;
	int32_t     step;
	int32_t     ratio;
	int32_t     fraction;
	int32_t     defualt;
	char        *name;
	char        *unit;
}TypeContinue;

typedef struct _TypeDisp_
{
	//int32_t     index;
	int32_t     data_type;
	int32_t     min;
	int32_t     max;
	int32_t     ratio;
	int32_t     fraction;
	int32_t     defualt;
	char        *name;
	char        *unit;
}TypeDisp;

#define ParameterTotasSize   sizeof(ParaNumbers)/sizeof(int32_t)
#define LogicSize            sizeof(TypeBool)/sizeof(int32_t)
#define ContinueSize         sizeof(TypeContinue)/sizeof(int32_t)
#define EnumSize             sizeof(TypeEnum)/sizeof(int32_t)
#define DispSize             sizeof(TypeEnum)/sizeof(int32_t)
//---------------------------------------------------------//
//--------------------------------------//
typedef struct __DynamicEqDescribe
{
	ParaNumbers          numbers;//totals
	TypeContinue         P1;//low_energy_threshold
	TypeContinue         P2;//normal_energy_threshold
	TypeContinue         P3;//high_energy_threshold
	TypeContinue         P4;//attack_time
	TypeContinue         P5;//release_time
}DynamicEqDescribe;

typedef struct __DynamicEqParam
{
	int16_t  	 		 low_energy_threshold; //-9000 ~ 0 to cover -90.00dB ~ 0.00dB
	int16_t              normal_energy_threshold;//-9000 ~ 0 to cover -90.00dB ~ 0.00dB
	int16_t              high_energy_threshold;//-9000 ~ 0 to cover -90.00dB ~ 0.00dB
	int16_t              attack_time;          //0~1000ms
	int16_t              release_time;         //0~1000ms
}DynamicEqParam;

typedef struct __DynamicEqUnit
{
	DynamicEQContext     ct;
	DynamicEqParam       param;
	EQContext            *eq_low;
	EQContext            *eq_high;
	int16_t 			 enable;
	int16_t 		     channel;
}DynamicEqUnit;
//----------------------------//
typedef struct __ButterWorthDescribe
{
	ParaNumbers          numbers;//totals
	TypeEnum             P1;//filter_type
	TypeContinue         P2;//filter_order
	TypeContinue         P3;//<sample rate/2
}ButterWorthDescribe;

typedef struct __ButterWorthParam
{
	int16_t  	 		 filter_type; //0,1
	int16_t              filter_order;//1~10
	int16_t              fc;          // <sample rate/2
}ButterWorthParam;

typedef struct __ButterWorthUnit
{
	uint8_t              ct[240];//persistent_size = 236;//use easy
	ButterWorthParam     param;
	int16_t 			 enable;
	int16_t 		     channel;
}ButterWorthUnit;
//-------------------------------------------------------------------------------------------------------//
void AudioEffectDynamicEqInit(DynamicEqUnit *unit, uint8_t channel, uint32_t sample_rate);
void AudioEffectDynamicEqApply24(DynamicEqUnit *unit, int32_t *pcm_in, int32_t *pcm_out, uint32_t n);
void AudioEffectDynamicEqApply(DynamicEqUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n);

void AudioEffectButterWorthInit(ButterWorthUnit *unit, uint8_t channel, uint32_t sample_rate);
void AudioEffectButterWorthApply(ButterWorthUnit *unit, int16_t *pcm_in, int16_t *pcm_out, uint32_t n);
void AudioEffectButterWorthApply24(ButterWorthUnit *unit, int32_t *pcm_in, int32_t *pcm_out, int32_t n);

void Comm_Effect_ButterWorth(EffectNode *pNode, uint8_t * buf, uint8_t index);
void Communication_Effect_ButterWorth(uint8_t Control, EffectNode* addr, uint8_t *buf, uint32_t len);
void Comm_Effect_HarmonicDynamicEQ(EffectNode *pNode, uint8_t * buf, uint8_t index);
void Communication_Effect_DynamicEQ(uint8_t Control, EffectNode* addr, uint8_t *buf, uint32_t len);

//------------------------------------------------------------------------------------------------------//
#endif /* APP_SRC_COMPONENTS_AUDIO_AUDIO_EFFECT_CLASS_H_ */
