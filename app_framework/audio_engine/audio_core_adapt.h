/*
 * audio_core_adapt.h
 *
 *  Created on: Mar 11, 2021
 *      Author: piwang
 */

#ifndef BT_AUDIO_APP_SRC_INC_AUDIO_CORE_ADAPT_H_
#define BT_AUDIO_APP_SRC_INC_AUDIO_CORE_ADAPT_H_
#include "type.h"
#include "resampler_polyphase.h"
#include "mcu_circular_buf.h"
//#include "sra.h"
#include "resampler_farrow.h"
#include "app_config.h"
#define SRA_MAX_CHG 		2
#define SRA_BLOCK 			128
#define POLYNOMIAL_ORDER	 4

typedef enum _MIX_NET_
{
	DefaultNet,
//	SeparateNet,
//......
	MaxNet,
} MIX_NET;

typedef enum _AUDIO_ADAPT_
{
	STD = 0,	//one frame /Direct
	SRC_ONLY,
	SRA_ONLY,
	CLK_ADJUST_ONLY,
	SRC_SRA,
	SRC_ADJUST,
} AUDIO_ADAPT;


typedef struct _SRC_ADAPTER_
{
	uint32_t					SampleRate;
	ResamplerPolyphaseContext	SrcCt;
	MCU_CIRCULAR_CONTEXT		SrcBufHandler;
} SRC_ADAPTER;

typedef struct _SRA_ADAPTER_
{
	bool						Enable;//ʹ�ܿ��أ�ͨ·����ʱ��Ч
	ResamplerFarrowContext		SraResFarCt;
	MCU_CIRCULAR_CONTEXT		SraBufHandler;
	uint32_t					TotalNum;//΢��ˮλͳ��--ּ�ڶ����ƽ��
	uint16_t					Count;//΢��ˮλͳ�Ƽ�����
	int8_t						AdjustVal;//���΢�� ����ɾ����ֵ��Դ��������ˮλ�ж�
	uint32_t					Depth;//LenGetFunc()�����������
	uint8_t						HighLevelCent;
	uint8_t						LowLevelCent;
} SRA_ADAPTER;

typedef struct _CLK_ADJUST_ADAPTER
{
	bool						Enable;//ʹ�ܿ��أ�ͨ·����ʱ��Ч
	uint32_t					TotalNum;//΢��ˮλͳ��--ּ�ڶ����ƽ��
	uint16_t					Count;//΢��ˮλͳ�Ƽ�����
	uint32_t					Depth;//LenGetFunc()�����������
	uint8_t						HighLevelCent;
	uint8_t						LowLevelCent;
	uint32_t					LastLevel;//ǰһ��ƽ��ˮλ
	int8_t						RiseTimes;//�������Ǵ������µ�����
	int8_t						AdjustVal;//��Ƶ�Ĵ���С������
} CLK_ADJUST_ADAPTER;

typedef struct _AudioCoreIO_
{
	uint8_t						Channels;//1/2 ע�⣺init�����������ֵ��ͨ�������ڼ��������Դ�ı䡣
	bool						Sync; //TRUE:ÿ֡ͬ��������ʱ����ͨ·������ FALSE:֡������Source���ݲ���ʱ���㣬Sink���ռ䲻��ʱ����
	MIX_NET						Net; //ȱʡ��DefaultNet������ͨ·����
	AUDIO_ADAPT					Adapt; //ȱʡ:STD,ͨ·������΢����������
#ifdef	CFG_AUDIO_WIDTH_24BIT
	uint8_t						IOBitWidth;//������Ƶλ��0,16bit��1,24bit
	uint8_t						IOBitWidthConvFlag;//������Ƶλ���Ƿ���Ҫ���䵽24bit
#endif
	void						*DataIOFunc;//��Ƶ������� ��������  @sample
	void						*LenGetFunc;//source���ݳ��� sink�ռ䳤�� �������� @sample
	//΢���������ã�Adapt��֧��΢��ʱȱʡ0
	uint8_t						HighLevelCent;//��ˮλ%������60
	uint8_t						LowLevelCent;//��ˮλ%������40
	uint16_t					Depth;//LenGetFunc()���ֵ @sample
	//ת����������Adapt��֧��ת����ʱȱʡ0
	uint32_t					SampleRate;//DataIOFunc()����fifo���ݵĲ�����
	bool						Resident;//sink��������Ӧ��Ƶ�������ͷ�֡bufʱ ��TRUE
} AudioCoreIO;//��Ƶͨ·�ӿڲ��� ���

//ʹ��ͨ· λ����Ϻ�MASK
#define SOURCE_BIT_EN(Value, Index) 		(Value) |= (1<<(Index))
#define SOURCE_BIT_DIS(Value, Index)		(Value) &= ~(1<<(Index))
#define	SOURCE_BIT_GET(Value, Index)		(((Value) >> (Index)) & 0x01)
#define	SOURCE_BIT_MASK(Value)				((Value) & ((1<<AUDIO_CORE_SOURCE_MAX_NUM) - 1))
#define SINK_BIT_EN(Value, Index) 			(Value) |= (1<<(AUDIO_CORE_SOURCE_MAX_NUM + Index))
#define SINK_BIT_DIS(Value, Index)			(Value) &= ~(1<<(AUDIO_CORE_SOURCE_MAX_NUM + Index))
#define	SINK_BIT_GET(Value, Index)			(((Value) >> (AUDIO_CORE_SOURCE_MAX_NUM + Index)) & 0x01)
#define SINK_BIT_MASK(Value)			((Value) & (((1<<(AUDIO_CORE_SOURCE_MAX_NUM + AUDIO_CORE_SINK_MAX_NUM)) - 1) - ((1<<AUDIO_CORE_SOURCE_MAX_NUM) - 1)))

//#define SOURCES(Index)	(AudioCore.AudioSource[Index])
//#define SINKS(Index)	(AudioCore.AudioSink[Index])
#define SOURCEFRAME(Index)				(AudioCore.FrameSize[AudioCore.AudioSource[Index].Net])
#define SINKFRAME(Index)				(AudioCore.FrameSize[AudioCore.AudioSink[Index].Net])

#define	ADJUST_PERIOD				(512*3)		//�Բ�������� ����������ڣ�ƽ�Ⲩ�����ٶȣ�SRA:����һ�������,��ӦƵ������
#define ADJUST_APLL_PERIOD			(512*10)	//�Բ��������������ڣ�����Ӳ��΢����
#define ADJUST_DIV_MAX				6			//Ӳ��΢���޷��ȶ�Ӧ���֮���Ƶƫ��Դ��div��С����Ƶ������
#define	ADJUST_SHRESHOLD			2			//2~4 Low��high֮�� ����Ӳ��΢�����𵴣���������ˮλ�仯���������ơ�

#define SRC_INPUT_MAX		128 //�� MAX_FRAME_SAMPLES @resampler_polyphase.h
#define SRC_INPUT_MIN		8//������������ֵ������ת������ �����ۻ����
#define SRC_OUPUT_MIN		4
#define SRC_OUPUT_JITTER	3//���������ƫ�� ����������ƫ��1�㣬 ��һ֡����delay����2������


#define SRC_SCALE_MAX		(6)			//48000	<--	8000 RESAMPLER_POLYPHASE_SRC_RATIO_6_1  @file	resampler_polyphase.h
#define	SRC_SCALE_MIN		(147/640)	 //44100 <-- 192000	RESAMPLER_POLYPHASE_SRC_RATIO_147_640

#define SRC_FIFO_SIZE(FRAME)			(FRAME + SRC_INPUT_MIN * SRC_SCALE_MAX + SRC_OUPUT_JITTER)

//Ԥ��֡Сʱ�ӿ�fifo��2֡ ΢��Low 40%���ϣ�Ҫ���fifoҪ�Ŵ�
#define SRA_FIFO_SIZE(FRAME)			((FRAME > 128) ? (FRAME + SRA_BLOCK + SRA_MAX_CHG + 1) : (FRAME + SRA_BLOCK * 2))

//����ͨ·��������������buf
bool AudioCoreSourceInit(AudioCoreIO * AudioIO, uint8_t Index);

//����������ͷŶ�̬buf
void AudioCoreSourceDeinit(uint8_t Index);

//����ͨ·��������������buf
bool AudioCoreSinkInit(AudioCoreIO * AudioIO, uint8_t Index);

//����������ͷŶ�̬buf
void AudioCoreSinkDeinit(uint8_t Index);

//ʵ��audiocore�����Adapter����Ǩ�ƣ�����֡���
void AudioCoreIOLenProcess(void);

//����ͨ· ͬ����⣬������ͬ����ͨ·��� ����AudioProcessMain
bool AudioCoreSourceSync(void);

//����sinkͨ·��ϴ���
bool AudioCoreSinkSync(void);

//������Ƶ���� ���÷�0ֵ������0ʱ����ԭֵ
void AudioCoreSourceChange(uint8_t Index, uint8_t Channels, uint32_t SampleRate);

//������Ƶ���� ���÷�0ֵ������0ʱ����ԭֵ
void AudioCoreSinkChange(uint8_t Index, uint8_t Channels, uint32_t SampleRate);

//����LenGetFunc()��Ӧ�����ֵ  @sample
void AudioCoreSourceDepthChange(uint8_t Index, uint32_t NewDepth);

//����LenGetFunc()��Ӧ�����ֵ  @sample
void AudioCoreSinkDepthChange(uint8_t Index, uint32_t NewDepth);

//ͨ·�����ã�ʵ����֡buf�������Ϊ��׼
bool AudioCoreSourceIsInit(uint8_t Index);

bool AudioCoreSinkIsInit(uint8_t Index);

//ͨ·΢�����أ���initʱAdapt���ͼ��������ϣ�ͨ·�ر�ʱ����Ч��
void AudioCoreSourceAdjust(uint8_t Index, bool OnOff);

//ͨ·΢�����أ���initʱAdapt���ͼ��������ϣ�ͨ·�ر�ʱ����Ч��
void AudioCoreSinkAdjust(uint8_t Index, bool OnOff);

//AudioCore.FrameSize[DefaultNet]���������ã��ض���ͨ·���ֵ��ԭ������Чbuf���� @Samples
//��APIֻ�ı�ͨ·֡����mix���ͨ·֡��buf��ҪӦ�ò����ͷţ����ô�api ���������롣
bool AudioCoreFrameSizeSet(MIX_NET Nets, uint16_t Size);

//ȱʡ���� DefaultNet
uint16_t AudioCoreFrameSizeGet(MIX_NET MixNet);

//audiocore mix�Ĳ���������, ���ͨ·��ת����ʱ����api����initת����������
void AudioCoreMixSampleRateSet(MIX_NET MixNet, uint32_t SampleRate);

uint32_t AudioCoreMixSampleRateGet(MIX_NET MixNet);

MIX_NET AudioCoreSourceMixNetGet(uint8_t Index);

MIX_NET AudioCoreSinkMixNetGet(uint8_t Index);

//Source��ͨ����������������Ϊmute�� muted�����ѽ���Sink�����Թر�
bool AudioCoreSourceReadyForClose(uint8_t Index);

//Sink��ͨ������������ͬΪmute��muted�ѽ���fifo(δmuteʱ��Ϊ��)�����Թر�ͨ·��
bool AudioCoreSinkReadyForClose(uint8_t Index);

//Sink��ͨ������������ͬΪmute��muted��������(δmuteʱ��Ϊ��)����������fifo��
bool AudioCoreSinkReadyFifoClear(uint8_t Index);

//audiocore������������Ϊmute��ͨ·�� muted�����ѽ���FIFO�����Թر�
bool AudioCoreMutedForClose(void);

//audiocore������������Ϊmute��ͨ·��ȫ���Ѳ�����muted���ݣ���������fifo
bool AudioCoreMutedForClear(void);

void AudioFadein(int16_t* pcm_in, uint16_t pcm_length, uint16_t ch);

void AudioFadeout(int16_t* pcm_in, uint16_t pcm_length, uint16_t ch);
#endif /* BT_AUDIO_APP_SRC_INC_AUDIO_CORE_ADAPT_H_ */
