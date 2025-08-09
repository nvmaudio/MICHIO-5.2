/*******************************************************
 *         MVSilicon Audio Effects Process
 *
 *                All Right Reserved
 *******************************************************/

#ifndef __AUDIO_EFFECT_H__
#define __AUDIO_EFFECT_H__

#ifdef  __cplusplus
extern "C" {
#endif//__cplusplus

#include "audio_core_api.h"

//��ЧApply������ָ��
typedef void (*AudioEffectApplyFunc)(void *effectUint, int16_t *pcm_in, int16_t *pcm_out, uint32_t n);


#define AUDIO_EFFECT_GROUP_NUM			3//��Ч�б�����
#define AUDIO_EFFECT_NODE_NUM			16//һ����Ч������

#define AUDIO_EFFECT_SUM				(AUDIO_EFFECT_GROUP_NUM * AUDIO_EFFECT_NODE_NUM)

typedef enum __NodeType
{
	NodeType_Normal = 0,     // Bình thường
	NodeType_Bifurcate,	    // Một nhánh chia làm hai
	NodeType_Group1,	    // Nút bắt đầu của nhóm phân chia thứ nhất
	NodeType_Group2,	    // Nút bắt đầu của nhóm phân chia thứ hai
	NodeType_Group3,	    // Nút bắt đầu của nhóm phân chia thứ ba
	NodeType_Mix,		    // Nút trộn của phân chia
	NodeType_Vol,		    // 5 - Âm lượng
	NodeType_AEC,		    // 6 - AEC (Thuật toán khử tiếng vọng, cần xử lý riêng)

	// NodeType_TONE,		// 7 - TREB BASS (âm cao, âm trầm)
	// NodeType_EqMode,	    // 8 - EQ chế độ chuyển đổi (tối ưu hóa chuyển đổi chế độ EQ)

	NodeType_VS,		    // Hiệu ứng giả lập âm thanh vòm 7.1

	// ----- Các định nghĩa EQ động ----- //
	NodeType_DynamicEqMin,              // Nhỏ nhất trong dải EQ động
	//----------------------------------//
	NodeType_DynamicEqGroup0,
	NodeType_DynamicEqGroup0_LP,        // Lọc thấp nhóm 0
	NodeType_DynamicEqGroup0_HP,        // Lọc cao nhóm 0
	//----------------------------------//
	NodeType_DynamicEqGroup1,
	NodeType_DynamicEqGroup1_LP,        // Lọc thấp nhóm 1
	NodeType_DynamicEqGroup1_HP,        // Lọc cao nhóm 1
	//----------------------------------//
	NodeType_DynamicEqMax,              // Lớn nhất trong dải EQ động
	//----------------------------------//
} NodeType;


/* Hiệu quả nút */
typedef struct __EffectNode
{
	uint8_t					Enable;         		// Kích hoạt hiệu ứng
	uint8_t					EffectType;     		// Loại hiệu ứng
	uint8_t					NodeType;       		// Loại nút, ví dụ như có phải là nút con hay không, có phải là nút đầu vào hoặc đầu ra hay không
	uint8_t					Width;          		// Độ rộngong chuỗi hiệu ứng
	void*		 			EffectUnit;     		// Đơn vị hiệu ứng hiệu ứng
	uint8_t					Index;          		// Chỉ số của nút tr
	AudioEffectApplyFunc	FuncAudioEffect;		// Hàm áp dụng hiệu ứng âm thanh
	uint8_t  				Effect_hex;          	// Chỉ mục thực tế của hiệu ứng nếu cần xử lý hiệu ứng cụ thể, nếu không thì đặt là 0xFF
} EffectNode;



typedef struct __EffectNodeList
{
	uint8_t		Channel;							// Kênh: chỉ định kênh hiệu ứng, ví dụ như kênh âm thanh cần áp dụng hiệu ứng
	EffectNode	EffectNode[AUDIO_EFFECT_NODE_NUM];	// Mảng các nút hiệu ứng, số lượng được xác định bởi AUDIO_EFFECT_NODE_NUM
} EffectNodeList;



extern EffectNodeList  gEffectNodeList[AUDIO_EFFECT_GROUP_NUM];
//��ǰģʽ��ʹ�õ���Чλ����Ч����б��еı��
extern uint8_t AudioModeIndex ;

//flash �洢��Ч����, ������Ч�ڵ�
bool AudioEffectParsePackage(uint8_t* add, uint16_t PackageLen, uint8_t* CommParam, bool IsReload);

void EffectPcmBufClear(uint32_t SampleLen);
void EffectPcmBufMalloc(uint32_t SampleLen);
void AudioEffectsInit(void);
void AudioEffectsDeInit(void);
void AudioEffectsLoadInit(bool IsReload, uint8_t mode);
void AudioEffectProcess(AudioCoreContext *pAudioCore);
void AudioMusicProcess(AudioCoreContext *pAudioCore);
void AudioBypassProcess(AudioCoreContext *pAudioCore);
void AudioEffectProcessBTHF(AudioCoreContext *pAudioCore);
void AudioNoAppProcess(AudioCoreContext *pAudioCore);
void EffectPcmBufRelease(void);

void du_efft_fadein_sw(int16_t* pcm_in, uint16_t pcm_length, uint16_t ch);		
void du_efft_fadeout_sw(int16_t* pcm_in, uint16_t pcm_length, uint16_t ch);		

uint8_t FineAudioEffectParamasIndex(uint8_t mode);
uint8_t GetAudioEffectParamasIndex(void);
extern bool IsEffectChange;
extern bool IsEffectChangeReload;

void AudioAPPDigitalGianProcess(uint32_t AppMode);


void AudioEffectON_OFF(uint8_t effect);



#ifdef  __cplusplus
}
#endif//__cplusplus

#endif//__AUDIO_EFFECT_H__
