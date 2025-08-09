/**
 **************************************************************************************
 * @file    audio_core_api.h
 * @brief   audio core 
 *
 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2016-6-29 13:06:47$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#ifndef __AUDIO_CORE_API_H__
#define __AUDIO_CORE_API_H__
#include "app_config.h"

#include "audio_core_adapt.h"
#include "resampler.h"




enum
{
	MIC_SOURCE_NUM,
	APP_SOURCE_NUM,	
#ifdef CFG_FUNC_REMIND_SOUND_EN
	REMIND_SOURCE_NUM,
#endif

#ifdef CFG_RES_AUDIO_I2S0IN_EN
	I2S0_SOURCE_NUM,          
#endif
#ifdef CFG_RES_AUDIO_I2S1IN_EN
	I2S1_SOURCE_NUM,           
#endif
#ifdef CFG_FUNC_LINE_MIX_MODE
	LINE_SOURCE_NUM,           
#endif

#ifdef CFG_FUNC_SPDIF_MIX_MODE
	SPDIF_MIX_SOURCE_NUM,
#endif

#ifdef BT_TWS_SUPPORT
	TWS_SOURCE_NUM,
#endif

#ifdef CFG_FUNC_RECORDER_EN
	PLAYBACK_SOURCE_NUM,	   
#endif
	AUDIO_CORE_SOURCE_MAX_NUM,
};

#define USB_AUDIO_SOURCE_NUM	APP_SOURCE_NUM




enum
{
	AUDIO_DAC0_SINK_NUM,		 
#ifdef CFG_FUNC_RECORDER_EN
	AUDIO_RECORDER_SINK_NUM,	 
#endif
#if	(defined(CFG_APP_BT_MODE_EN) && (BT_HFP_SUPPORT == ENABLE)) || defined(CFG_APP_USB_AUDIO_MODE_EN)
	AUDIO_APP_SINK_NUM,
#endif
#ifdef CFG_RES_AUDIO_DACX_EN
	AUDIO_DACX_SINK_NUM,		
#endif

#if defined(CFG_RES_AUDIO_I2SOUT_EN)
	AUDIO_STEREO_SINK_NUM,      
#endif



#ifdef CFG_RES_AUDIO_I2S0OUT_EN
	AUDIO_I2S0_OUT_SINK_NUM,      
#endif

#ifdef CFG_RES_AUDIO_I2S1OUT_EN
	AUDIO_I2S1_OUT_SINK_NUM,     
#endif




#ifdef CFG_RES_AUDIO_SPDIFOUT_EN
	AUDIO_SPDIF_SINK_NUM,      	 
#endif

#ifdef BT_TWS_SUPPORT
	TWS_SINK_NUM,				 
#endif
	AUDIO_CORE_SINK_MAX_NUM,
};

#define AUDIO_HF_SCO_SINK_NUM 	AUDIO_APP_SINK_NUM
#define USB_AUDIO_SINK_NUM		AUDIO_APP_SINK_NUM

#if defined(CFG_RES_AUDIO_I2SOUT_EN)
	#define AUDIO_I2SOUT_SINK_NUM	AUDIO_STEREO_SINK_NUM
#endif

typedef uint16_t (*AudioCoreDataGetFunc)(void* Buf, uint16_t Samples);
typedef uint16_t (*AudioCoreDataLenFunc)(void);
typedef uint16_t (*AudioCoreDataSetFunc)(void* Buf, uint16_t Samples);
typedef uint16_t (*AudioCoreSpaceLenFunc)(void);


typedef void (*AudioCoreProcessFunc)(void*);

#ifdef CFG_AUDIO_WIDTH_24BIT
	#define PCM_DATA_TYPE	int32_t
	typedef enum
	{
		PCM_DATA_16BIT_WIDTH = 0,
		PCM_DATA_24BIT_WIDTH,
	}PCM_DATA_WIDTH;

#else
	#define PCM_DATA_TYPE	int16_t
#endif

typedef struct _AudioCoreSource
{
	bool						Enable;              // Bật/tắt kênh âm thanh (kích hoạt đường tín hiệu)
	bool						Sync;                // TRUE: mỗi khung dữ liệu đều đồng bộ, đảm bảo nguồn và đích luôn khớp; 
	                                              	// FALSE: nếu nguồn và đích không đồng bộ theo từng khung thì dữ liệu có thể bị mất
	uint8_t						Channels;            // Số kênh âm thanh (mono = 1, stereo = 2...)
	bool						LeftMuteFlag;        // Cờ mute (tắt tiếng) kênh trái
	bool						RightMuteFlag;       // Cờ mute (tắt tiếng) kênh phải
	uint32_t					MutedCount;          // Bộ đếm số lần bị mute, để xác định khi nào cần unmute tự động
	bool						Active;              // Cờ trạng thái hoạt động của đường tín hiệu, dùng trong AudioProcessMain
	MIX_NET						Net;                 // Lưới trộn tín hiệu (mixer net), mặc định là DefaultNet/0, dùng để đồng bộ hóa theo từng khung dữ liệu
	AUDIO_ADAPT					Adapt;               // Kiểu điều chỉnh tín hiệu đầu vào, mặc định: STD (chuẩn), hỗ trợ chuẩn hóa tín hiệu hoặc SRC/SRA
	AudioCoreDataGetFunc		DataGetFunc;         // Con trỏ hàm lấy dữ liệu đầu vào cho đường tín hiệu
	AudioCoreDataLenFunc		DataLenFunc;         // Con trỏ hàm lấy độ dài mẫu dữ liệu (sample length)
	PCM_DATA_TYPE				*PcmInBuf;	          // Bộ đệm dữ liệu đầu vào của khung (frame buffer)
	PCM_DATA_TYPE				*AdaptBuf;	          // Bộ đệm dùng cho quá trình điều chỉnh SRC (chuyển đổi tần số lấy mẫu) & SRA (đồng bộ tốc độ)

#ifdef	CFG_AUDIO_WIDTH_24BIT
	PCM_DATA_WIDTH				BitWidth;            // Độ rộng bit của tín hiệu âm thanh: 0 = 16 bit, 1 = 24 bit
	bool						BitWidthConvFlag;    // Cờ cho biết có cần chuyển đổi độ rộng bit sang 24 bit hay không
#endif

	int16_t						PreGain;             // Mức khuếch đại trước (áp dụng trước các xử lý khác)

	int16_t						LeftVol;             // Âm lượng kênh trái (cấu hình)
	int16_t						RightVol;            // Âm lượng kênh phải (cấu hình)

	int16_t						LeftCurVol;          // Âm lượng hiện tại kênh trái (đang áp dụng thực tế)
	int16_t						RightCurVol;         // Âm lượng hiện tại kênh phải (đang áp dụng thực tế)
	
	SRC_ADAPTER					*SrcAdapter;         // Con trỏ đến đối tượng chuyển đổi tốc độ lấy mẫu (SRC Adapter)
	void						*AdjAdapter;         // Con trỏ đến bộ điều chỉnh vi mô tín hiệu (microscale adapter, ví dụ như SRA, AGC...)
} AudioCoreSource;



typedef struct _AudioCoreSink
{
	bool							Enable;//ͨ·��ͣ
	bool							Sync;//TRUE:ÿ֡ͬ��������ʱ����ͨ·������ FALSE:֡������Source���ݲ���ʱ���㣬Sink���ռ䲻��ʱ����
	uint8_t							Channels;
	bool							Active;//������ϵ�����/�ռ�֡�걸������AudioProcessMain����ͨ·���
	MIX_NET							Net; //ȱʡDefaultNet/0,  ʹ��������ͬ��֡��������
	AUDIO_ADAPT						Adapt;//ȱʡ:STD,ͨ·������΢����������
	bool							LeftMuteFlag;//������־
	bool							RightMuteFlag;//������־
	uint32_t						MutedCount;//mute����������������muted��������
	AudioCoreDataSetFunc			DataSetFunc;//������� ��buf->�⻺����ƺ���
	AudioCoreSpaceLenFunc			SpaceLenFunc;//�������ռ�sample�� ����
	uint32_t						Depth;//LenGetFunc()����������ȣ����ڼ������������
	PCM_DATA_TYPE					*PcmOutBuf;//ͨ·����֡����buf
	PCM_DATA_TYPE					*AdaptBuf;//SRC&SRA����buf
#ifdef	CFG_AUDIO_WIDTH_24BIT
	PCM_DATA_WIDTH					BitWidth;//������Ƶλ����0,16bit��1,24bit
	bool							BitWidthConvFlag;//������Ƶλ�����Ƿ���Ҫλ��ת�� 24bit <--> 16bit
#endif
	int16_t							LeftVol;	//����
	int16_t							RightVol;	//����
	int16_t							LeftCurVol;	//��ǰ����
	int16_t							RightCurVol;//��ǰ����
	void							*AdjAdapter; //΢��������
	SRC_ADAPTER						*SrcAdapter; //ת����������
}AudioCoreSink;


typedef struct _AudioCoreContext
{
	uint32_t AdaptIn[(MAX_FRAME_SAMPLES * sizeof(PCM_DATA_TYPE)) / 2]; 						// Bộ đệm đầu vào cho quá trình chuyển đổi và điều chỉnh tín hiệu (SRC/SRA), 
	uint32_t AdaptOut[(MAX_FRAME_SAMPLES * SRC_SCALE_MAX * sizeof(PCM_DATA_TYPE)) / 2]; 	// Bộ đệm đầu ra sau quá trình chuyển đổi/điều chỉnh tín hiệu, có thể lớn hơn do SRC phóng đại khung
	MIX_NET CurrentMix; 										// Lưới trộn hiện tại (Current Mix Net), dùng để xác định quá trình trộn âm từ nhiều đường tín hiệu
	uint16_t FrameReady; 										// Cờ trạng thái cho biết khung dữ liệu đã sẵn sàng (có thể được ghi hoặc đọc); dùng trong đồng bộ khung
	uint32_t SampleRate[MaxNet]; 								// Tần số lấy mẫu (sample rate) của từng lưới trộn (mixer net), thường [DefaultNet]/[0] là chính
	uint16_t FrameSize[MaxNet]; 								// Kích thước khung (frame size) của mỗi lưới trộn; hỗ trợ cả lưới chia riêng (SeparateNet) hoặc mặc định
	AudioCoreSource AudioSource[AUDIO_CORE_SOURCE_MAX_NUM]; 	// Danh sách các đường tín hiệu đầu vào (nguồn âm thanh), tối đa AUDIO_CORE_SOURCE_MAX_NUM
	AudioCoreProcessFunc AudioEffectProcess; 					// Hàm xử lý hiệu ứng âm thanh chính, được gọi để áp dụng các bộ lọc, hiệu ứng, xử lý âm thanh
	AudioCoreSink AudioSink[AUDIO_CORE_SINK_MAX_NUM]; 			// Danh sách các đường tín hiệu đầu ra (thiết bị phát), tối đa AUDIO_CORE_SINK_MAX_NUM
	
} AudioCoreContext;





extern AudioCoreContext		AudioCore;

//typedef void (*AudioCoreProcessFunc)(AudioCoreContext *AudioCore);
/**
 * @func        AudioCoreSourceFreqAdjustEnable
 * @brief       ʹ��ϵͳ��Ƶ��Ƶ΢����ʹ�ŵ�֮��ͬ��(���첽��Դ)
 * @param       uint8_t AsyncIndex   �첽��ƵԴ�����ŵ����
 * @param       uint16_t LevelLow   �첽��ƵԴ�����ŵ���ˮλ������ֵ
 * @param       uint16_t LevelHigh   �첽��ƵԴ�����ŵ���ˮλ������ֵ
 * @Output      None
 * @return      None
 * @Others
 * Record
 * 1.Date        : 20180518
 *   Author      : pi.wang
 *   Modification: Created function
 */


//��ͨ·���ݵ����������ã�ͨ·initʱ��Ҫ�������������
void AudioCoreSourcePcmFormatConfig(uint8_t Index, uint16_t Format);

void AudioCoreSourceEnable(uint8_t Index);

void AudioCoreSourceDisable(uint8_t Index);

bool AudioCoreSourceIsEnable(uint8_t Index);

void AudioCoreSourceMute(uint8_t Index, bool IsLeftMute, bool IsRightMute);

void AudioCoreSourceUnmute(uint8_t Index, bool IsLeftUnmute, bool IsRightUnmute);

void AudioCoreSourceVolSet(uint8_t Index, uint16_t LeftVol, uint16_t RightVol);

void AudioCoreSourceVolGet(uint8_t Index, uint16_t* LeftVol, uint16_t* RightVol);

void AudioCoreSourceConfig(uint8_t Index, AudioCoreSource* Source);

void AudioCoreSinkEnable(uint8_t Index);

void AudioCoreSinkDisable(uint8_t Index);

bool AudioCoreSinkIsEnable(uint8_t Index);

void AudioCoreSinkMute(uint8_t Index, bool IsLeftMute, bool IsRightMute);

void AudioCoreSinkUnmute(uint8_t Index, bool IsLeftUnmute, bool IsRightUnmute);

void AudioCoreSinkVolSet(uint8_t Index, uint16_t LeftVol, uint16_t RightVol);

void AudioCoreSinkVolGet(uint8_t Index, uint16_t* LeftVol, uint16_t* RightVol);

void AudioCoreSinkConfig(uint8_t Index, AudioCoreSink* Sink);

void AudioCoreProcessConfig(AudioCoreProcessFunc AudioEffectProcess);


bool AudioCoreInit(void);

void AudioCoreDeinit(void);

void AudioCoreRun(void);

//��������
void AudioCoreSourceVolApply(void);
void AudioCoreSinkVolApply(void);
void AudioCoreAppSourceVolApply(uint16_t Source,int16_t *pcm_in,uint16_t n,uint16_t Channel);

#ifdef	CFG_AUDIO_WIDTH_24BIT
PCM_DATA_WIDTH AudioCoreSourceBitWidthGet(uint8_t Index);
#endif

#endif //__AUDIO_CORE_API_H__
