/**
 **************************************************************************************
 * @file    decoder_service.h
 * @brief   
 *
 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2016-6-29 13:06:47$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#ifndef __DECODER_SERVICE_H__
#define __DECODER_SERVICE_H__

#include "type.h"
#include "rtos_api.h"
#include "audio_decoder_api.h"
#include "mvstdio.h"
#include "app_message.h"

typedef enum
{
	DecoderStateNone = 0,
	DecoderStateInitialized,
	DecoderStateDeinitializing,
	DecoderStateStop,
	DecoderStatePlay,
	DecoderStatePause,
	DecoderStateDecoding,
	DecoderStateToSavePcmData,
	DecoderStateSavePcmData,

}DecoderState;


typedef enum
{
	DS_EVENT_SERVICE_INITED,
	DS_EVENT_SERVICE_STARTED,
	DS_EVENT_DECODE_INITED,
	DS_EVENT_DECODE_DECODED,
} DecoderServiceEvent;

typedef enum
{
	DECODER_MODE_CHANNEL,
#ifdef CFG_REMIND_SOUND_DECODING_USE_LIBRARY	
	DECODER_REMIND_CHANNEL,
#endif	
	DECODER_MAX_CHANNELS,
}DecoderChannels;
#ifdef LOSSLESS_DECODER_HIGH_RESOLUTION
	#define DECODER_BUF_SIZE   					(1024 * 42)	//������������Unit:  BYTE�����ֲ��ԡ�
#else
	#define DECODER_BUF_SIZE   					(1024 * 28)	//������������Unit:  BYTE�����ֲ��ԡ�
#endif
#define DECODER_BUF_SIZE_MP3				(1024 * 19)
#define DECODER_BUF_SIZE_SBC				(1024 * 7)

//WMA:28536,AAC:28444,APE:25340,FLA:22580,MP3:19136,AIF:11236,WAV:11228,SBC:5624,ʵ�ʿ��С��䣬����adpcm��wav��

#define DECODER_FIFO_SIZE_MIN				(CFG_PARA_MAX_SAMPLES_PER_FRAME * 4)//��С fifo ����
#ifdef LOSSLESS_DECODER_HIGH_RESOLUTION
#define DECODER_FIFO_SIZE_FOR_PLAYER 		(CFG_PARA_MAX_SAMPLES_PER_FRAME * 24 * 4)	//�����ȡ����ѹ���������������Ҫ��󻺳塣�磺Flac24bit 1.5MbpsҪ��Ϊ*20;1.7MbpsҪ��Ϊ*24
#else
#define DECODER_FIFO_SIZE_FOR_PLAYER 		(CFG_PARA_MAX_SAMPLES_PER_FRAME * 24)	//�����ȡ����ѹ���������������Ҫ��󻺳塣�磺Flac24bit 1.5MbpsҪ��Ϊ*20;1.7MbpsҪ��Ϊ*24
#endif
#define	DECODER_FIFO_SIZE_FOR_SBC			(CFG_PARA_MAX_SAMPLES_PER_FRAME * 6)
#define DECODER_FIFO_SIZE_FOR_MP3			(CFG_PARA_MAX_SAMPLES_PER_FRAME * 8)

//memhandle ������ ����ˮλ����
#define SBC_DECODER_FIFO_MIN				(119*2)//(119*17)
#define AAC_DECODER_FIFO_MIN				(600)
#define MSBC_DECODER_FIFO_MIN				57*2//238
//������ ���� ����EMPTY��������趨	 ����μ� audio_decoder_error_code_summary.txt
#define MPX_ERROR_STREAM_EMPTY				-127
#define SBC_ERROR_STREAM_EMPTY				-123
#define AAC_ERROR_STREAM_EMPTY				-127
#define AMR_ERROR_STREAM_EMPTY				-127
#define WAV_ERROR_STREAM_EMPTY				-127
//�������������λ�����룬��Ӧ�ó���������EMPTY�����ۻ�����ǰ���������Ϣ���ơ�
#define MUTLI_EMPTY_FOR_MSG					300//ͳһ�趨���������б�Ҫ��initʱ���������ض�ֵ��

//void DecoderServiceStart(DecoderChannels DecoderChannel);
//void DecoderServicePause(DecoderChannels DecoderChannel);
//void DecoderServiceResume(DecoderChannels DecoderChannel);
void DecoderServiceStop(DecoderChannels DecoderChannel);
void DecoderServiceDeinit(DecoderChannels DecoderChannel);
MessageHandle GetDecoderServiceMsgHandle(void);
//TaskState GetDecoderServiceState(DecoderChannels DecoderChannel);


int32_t DecoderInit(void *io_handle, DecoderChannels DecoderChannel, int32_t ioType, int32_t decoderType);
void DecoderPlay(DecoderChannels DecoderChannel);
//void DecoderReset(DecoderChannels DecoderChannel);
void DecoderStop(DecoderChannels DecoderChannel);
void DecoderMuteAndStop(DecoderChannels DecoderChannel);
void DecoderPause(DecoderChannels DecoderChannel);

void DecoderResume(DecoderChannels DecoderChannel);

//ע�⣬StepTime ��λΪ���룬ʵ�ʴ���ֵΪ������
void DecoderFF(uint32_t StepTime,DecoderChannels DecoderChannel);

//ע�⣬StepTime ��λΪ���룬ʵ�ʴ���ֵΪ������
void DecoderFB(uint32_t StepTime,DecoderChannels DecoderChannel);

bool DecoderSeek(uint32_t Time,DecoderChannels DecoderChannel);

uint32_t DecoderServicePlayTimeGet(DecoderChannels DecoderChannel);

uint32_t GetDecoderState(DecoderChannels DecoderChannel);

uint8_t DecoderSourceNumGet(DecoderChannels DecoderChannel);

//�����������audiocore sourceͨ·�����õ�Num��
void DecoderSourceNumSet(uint8_t Num,DecoderChannels DecoderChannel);
bool DecoderServiceInit(MessageHandle ParentMsgHandle, DecoderChannels DecoderChannel,uint32_t BufSize, uint16_t FifoSize);
void RemindDecodeProcess(void);
void ModeDecodeProcess(void);
uint16_t ModeDecoderPcmDataGet(void * pcmData,uint16_t sampleLen);// call by audio core one by one
//uint16_t RemindDecoderPcmDataGet(void * pcmData,uint16_t sampleLen);// call by audio core one by one
uint16_t ModeDecoderPcmDataLenGet(void);
//uint16_t RemindDecoderPcmDataLenGet(void);


#endif /*__DECODER_SERVICE_H__*/


