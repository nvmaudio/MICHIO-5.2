#ifndef __EQ_PARAMS_H__
#define __EQ_PARAMS_H__

#ifdef  __cplusplus
extern "C" {
#endif//__cplusplus


#define EQ_PARAM_LEN   104//112//ֱ������Ϊ�̶�ֵ��EQ��Ч����Ϊ�̶�ֵ
#define EQDRC_PARAM_LEN   156//112//ֱ������Ϊ�̶�ֵ��EQ_DRC��Ч����Ϊ�̶�ֵ


//EQ style: Classical
extern const unsigned char Classical[104];
extern const unsigned char Classical_EQDRC[156];

//EQ style: Vocal Booster
extern const unsigned char Vocal_Booster[104];
extern const unsigned char Vocal_Booster_EQDRC[156];

//EQ style: Flat
extern const unsigned char Flat[104];
extern const unsigned char Flat_EQDRC[156];

//EQ style: Pop
extern const unsigned char Pop[104];
extern const unsigned char Pop_EQDRC[156];

//EQ style: Rock
extern const unsigned char Rock[104];
extern const unsigned char Rock_EQDRC[156];

//EQ style: Jazz
extern const unsigned char Jazz[104];
extern const unsigned char Jazz_EQDRC[156];


#ifdef  __cplusplus
}
#endif//__cplusplus

#endif//__EQ_PARAMS_H__
