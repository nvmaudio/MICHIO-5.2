#include "comm_param.h"
#include "audio_effect_api.h"
#include "audio_effect.h"


const EffectComCt AudioEffectCommParam[]=
{
	{2, NodeType_Normal, 	GAIN_CONTROL,			"1:BT Play Gain", 					24,  0xFF},//0x81
	{2, NodeType_Normal, 	GAIN_CONTROL,			"1:USB Play Gain", 					24,  0xFF},//0x82
	{2, NodeType_Normal, 	GAIN_CONTROL,			"1:USB Device Gain", 				24,  0xFF},//0x83
	{2, NodeType_Normal, 	GAIN_CONTROL,			"1:SD Play Gain",					24,  0xFF},//0x84
	{2, NodeType_Normal, 	GAIN_CONTROL,			"1:I2S Gain",						24,  0xFF},//0x85
	{2, NodeType_Normal, 	GAIN_CONTROL,			"1:OPTICAL Gain ",					24,  0xFF},//0x86
	{2, NodeType_Normal, 	GAIN_CONTROL,			"1:COAXIAL Gain ",					24,  0xFF},//0x87
	
	{2, NodeType_Normal, 	EXPANDER, 				"2:Music Noise Suppressor",			24,  0xFF},//0x88
	{2, NodeType_Bifurcate, SILENCE_DETECTOR,		"2:Music Silence Detector", 		24,  0xFF},//0x89
	{2, NodeType_Normal, 	COMPANDER,				"2:Music Compander",				24,  0xFF},//0x8A
	{2, NodeType_Normal, 	LOW_LEVEL_COMPRESSOR,	"2:Music Low Level Compressor",		24,  0xFF},//0x8B
	{2, NodeType_Normal, 	VIRTUAL_BASS, 			"2:Music VB",						24,  0x8C},//0x8C
	{2, NodeType_Normal, 	VIRTUAL_BASS_CLASSIC,	"2:Music VB Classic",				24,  0xFF},//0x8D
	{2, NodeType_Normal, 	THREE_D, 				"2:Music 3D",						24,  0x8E},//0x8E
	{2, NodeType_Normal, 	EXCITER, 				"2:Music Exciter",					24,  0xFF},//0x8F
	{2, NodeType_Normal, 	STEREO_WIDENER,			"2:Music Stereo Widener",			24,  0xFF},//0x90
	{2, NodeType_DynamicEqGroup0_LP,	EQ,			"2:Music DynamicEQLP",				24,  0x91},//0x91
	{2, NodeType_DynamicEqGroup0_HP,	EQ,			"2:Music DynamicEQHP",				24,  0x92},//0x92
	{2, NodeType_DynamicEqGroup0,		DynamicEQ,	"2:Music Dynamic EQ",				24,  0xFF},//0x93
	{2, NodeType_Normal, 	EQ,						"2:Music Pre EQ",					24,  0x94},//0x94
	{2, NodeType_Normal, 	EQ_DRC,					"2:Music EQ_DRC",					24,  0xFF},//0x95
	{2, NodeType_Normal, 	EQ,						"2:Music EQ",						24,  0xFF},//0x96
	{2, NodeType_Normal, 	GAIN_CONTROL,			"2:Music GAIN_OUT",					24,  0xFF},//0x97

	
	{1, NodeType_Normal, 	LOW_LEVEL_COMPRESSOR,	"3:Music DACX Low Level Compressor",24,  0xFF},//0x98
	{1, NodeType_Normal, 	PHASE_CONTROL,			"3:Music DACX Phase Control",		24,  0xFF},//0x99
	{1, NodeType_Normal, 	VIRTUAL_BASS,			"3:Music DACX VB",					24,  0x9a},//0x9a
	{1, NodeType_Normal, 	VIRTUAL_BASS_CLASSIC,	"3:Music DACX VB Classic",			24,  0xFF},//0x9b
	{1, NodeType_DynamicEqGroup1_LP,	EQ,			"3:Music DACX DynamicEQLP",			24,  0x9c},//0x9c
	{1, NodeType_DynamicEqGroup1_HP,	EQ,			"3:Music DACX DynamicEQHP",			24,  0x9d},//0x9d
	{1, NodeType_DynamicEqGroup1,		DynamicEQ,	"3:Music DACX Dynamic EQ",			24,  0x9e},//0x9e
	{1, NodeType_Normal, 	EQ,						"3:Music DACX Pre EQ",				24,  0x9f},//0x9f
	{1, NodeType_Normal, 	EQ_DRC,					"3:Music DACX EQ_DRC",				24,  0xFF},//0xa0
	{1, NodeType_Normal, 	EQ,						"3:Music DACX EQ",					24,  0xFF},//0xa1
	{1, NodeType_Normal, 	GAIN_CONTROL,			"3:Music GAIN_OUT",					24,  0xFF},//0x97
};


const EffectComCt AudioEffectAECParam_HFP[]=
{
    {1, NodeType_Vol, 		EXPANDER, 				"1:Noise Suppressor	",	16, 0xFF},//0x81
    {1, NodeType_Normal,	EQ,						"1:EQ        		",	16, 0xFF},//0x82
	{1, NodeType_AEC, 		AEC, 					"1:AEC       		",	16, 0xFF},//0x83
	{1, NodeType_Normal, 	GAIN_CONTROL,			"1:Gain      		",	16, 0xFF},//0x84
	{1, NodeType_Normal, 	DRC,					"1:DRC				",	16, 0xFF},//0x85
};
