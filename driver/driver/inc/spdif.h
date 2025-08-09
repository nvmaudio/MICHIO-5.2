/**
  *****************************************************************************
  * @file:			spdif.h
  * @author			Cecilia
  * @version		V2.0.0
  * @data			2015-7-20
  * @Brief			SPDIF driver interface
  ******************************************************************************
 */
/**
 * @addtogroup SPDIF
 * @{
 * @defgroup spdif spdif.h
 * @{
 */
 
#ifndef __SPDIF_H__
#define __SPDIF_H__
 
#ifdef  __cplusplus
extern "C" {
#endif//__cplusplus

#include "type.h"

typedef enum __SPDIF_AUDIO_DATA_TYPE
{
	SPDIF_AUDIO_NULL_DATA_TYPE 				= 0,
	SPDIF_AUDIO_AC_3_DATA_TYPE 				= 1,
	SPDIF_AUDIO_TIME_STAMP_DATA_TYPE 		= 2,
	SPDIF_AUDIO_PAUSE_DATA_TYPE				= 3,
	SPDIF_AUDIO_MPEG_1_LAYER_1_DATA_TYPE 	= 4,
	SPDIF_AUDIO_MPEG_1_LAYER_2_3_DATA_TYPE 	= 5,
	SPDIF_AUDIO_MPEG_2_DATA_TYPE			= 6,
	SPDIF_AUDIO_MPEG_2_AAC_DATA_TYPE 		= 7,
	SPDIF_AUDIO_MPEG_2_LAYER_1_DATA_TYPE 	= 8,
	SPDIF_AUDIO_MPEG_2_LAYER_2_DATA_TYPE 	= 9,
	SPDIF_AUDIO_MPEG_2_LAYER_3_DATA_TYPE 	= 10,
	SPDIF_AUDIO_DTS_TYPE_1_DATA_TYPE 		= 11,
	SPDIF_AUDIO_DTS_TYPE_2_DATA_TYPE 		= 12,
	SPDIF_AUDIO_DTS_TYPE_3_DATA_TYPE 		= 13,
	SPDIF_AUDIO_ATRAC_DATA_TYPE 			= 14,
	SPDIF_AUDIO_ATRAC_2_3_DATA_TYPE 		= 15,
	SPDIF_AUDIO_ATRAC_X_DATA_TYPE 			= 16,
	SPDIF_AUDIO_DTS_TYPE_4_DATA_TYPE 		= 17,
	SPDIF_AUDIO_WMA_DATA_TYPE 				= 18,
	SPDIF_AUDIO_MPEG_2_AAC_LOW_DATA_TYPE	= 19,
	SPDIF_AUDIO_MPEG_4_AAC_LOW_DATA_TYPE    = 20,
	SPDIF_AUDIO_EN_AC_3_DATA_TYPE 			= 21,
	SPDIF_AUDIO_MAT_DATA_TYPE 				= 22,
	SPDIF_AUDIO_SMPTE_KLV_DATA_TYPE 		= 27,
	SPDIF_AUDIO_DOLBY_E_DATA_TYPE 			= 28,
	SPDIF_AUDIO_CAPTIONING_DATA_TYPE 		= 29,
	SPDIF_AUDIO_USER_DEFINED_DATA_TYPE 		= 30,

	SPDIF_AUDIO_PCM_DATA_TYPE 				= 100,

} SPDIF_AUDIO_DATA_TYPE;

typedef enum __SPDIF_ANA_CH_SEL
{
	SPDIF_ANA_INPUT_A28 = 0,

	SPDIF_ANA_INPUT_A29,

	SPDIF_ANA_INPUT_A30,

	SPDIF_ANA_INPUT_A31

} SPDIF_ANA_CH_SEL;

typedef enum __SPDIF_ANA_INPUT_LEVEL
{
	SPDIF_ANA_LEVEL_300mVpp = 0,

	SPDIF_ANA_LEVEL_200mVpp

} SPDIF_ANA_LEVEL;
    
typedef enum __SPDIF_DATA_WORDLTH
{
    SPDIF_WORDLTH_16BIT = 16,
    SPDIF_WORDLTH_20BIT = 20,
    SPDIF_WORDLTH_24BIT = 24
} SPDIF_DATA_WORDLTH;    

/**
 * @brief 20bit��24bitʱ�����ݴ�Ÿ�ʽ
 */
typedef enum _SPDIF_20BIT_24BIT_ALIGN_MODE
{
	SPDIF_HIGH_BITS_ACTIVE = 0,
	SPDIF_LOW_BITS_ACTIVE,
} SPDIF_20BIT_24BIT_ALIGN_MODE;

typedef enum __SPDIF_FLAG_STATUS
{
    PARITY_FLAG_STATUS = 21,
    UNDER_FLAG_STATUS  = 22,
    OVER_FLAG_STATUS   = 23,
    EMPTY_FLAG_STATUS  = 24,
    AEMPTY_FLAG_STATUS = 25,
    FULL_FLAG_STATUS   = 26,
    AFULL_FLAG_STATUS  = 27,
    SYNC_FLAG_STATUS   = 28,
    LOCK_FLAG_STATUS   = 29,
    BEGIN_FLAG_STATUS  = 30
    
} SPDIF_STATUS_TYPE;    

typedef enum __SPDIF_INT_TYPE
{
    PARITY_ERROR_INT   = 21,    //У������ж�
    UNDER_ERROR_INT    = 22,    //
    OVER_ERROR_INT     = 23,    //
    EMPTY_ERROR_INT    = 24,    //FIFO���ж�
    AEMPTY_ERROR_INT   = 25,    //FIFO�ӽ��յ���Ϊ���ж�
    FULL_ERROR_INT     = 26,
    AFULL_ERROR_INT    = 27,
    SYNC_ERROR_INT     = 28,
    LOCK_INT           = 29,
    BEGIN_FRAME_INT    = 30,
    ALL_SPDIF_INT_DISABLE = 31     //�����ж�ȫ���ر�

} SPDIF_INT_TYPE;    

typedef enum __SPIDF_CLOCK_SOURCE_INDEX{
	SPIDF_CLK_SOURCE_PLL = 0,	/*����SPDIF��ʱ��ԴΪPLL*/
	SPIDF_CLK_SOURCE_AUPLL		/*����SPDIF��ʱ��ԴΪAUPLL*/
}SPIDF_CLOCK_SOURCE_INDEX;

typedef struct __SPDIF_TYPE_STR
{
    uint8_t              audio_type;
    uint8_t				 bitstream_number;
    uint8_t              IsNewFrame;//0-��֡û��֡ͷ��1-��֡��֡ͷ�� 2-��֡�еڶ�֡��֡ͷ
    int32_t             total_length;
    int32_t             output_length;

} SPDIF_TYPE_STR;


/**
 * @brief  ʹ��spdif����ǰ�˵�ƽת����·
 * @param  ChannelIO�� ͨ���� �����#SPDIF_ANA_CH_SEL
 * @param  Level: SPDIF_ANA_LEVEL_300mVpp: ������С�ź�ѹ��֧��300mV
 * 				  SPDIF_ANA_LEVEL_200mVpp: ������С�ź�ѹ��֧��200mV
 * @return ��
 */
void SPDIF_AnalogModuleEnable(SPDIF_ANA_CH_SEL ChannelIO, SPDIF_ANA_LEVEL Level);


/**
 * @brief  ����spdif����ǰ�˵�ƽת����·
 * @param  ��
 * @return ��
 */
void SPDIF_AnalogModuleDisable(void);

/**
 * @brief  ��ȡ��ǰSPDIF�Ĳ�����
 * @param  ��
 * @return ��ǰ������
 */
uint32_t SPDIF_SampleRateGet(void);

/**
 * @brief  ���÷���ʱ�Ĳ�����
 * @param  SpdifSampleRate: ���Ͳ�����
 * @return ��
 */
void SPDIF_SampleRateSet(uint32_t SpdifSampleRate);

/**
 * @brief  ��SPDIF����Ϊ����ģʽ�����г�ʼ��
 * @param  ParitySrc�����ɼ���Ԫ��Դѡ��0 - ������� 1 - Ӳ������
 * @param  IsValidityEn: ��Чλ�Ƿ���м�⣬0 - ��Чλ����⣬ȫ��д��FIFO�У� 
                                           1 - ֻ����Чλ��ȷ��д��FIFO��
 * @param  ChannelMoe: 0: ģʽ0 - SPDIF��Ϊ����ģ�����ģ��ʱ����Ϊ˫��������
                       1: ģʽ1 - SPDIF��Ϊ����ģ��ʱ��ֻ��������������д��FIFO�У���Ϊ����ģ��ʱ����������������������������Ϊ��������������
                       2: ģʽ2 - SPDIF��Ϊ����ģ��ʱ��ͬģʽ1��ͬ����Ϊ����ģ��ʱ������������������������������0
 * @param  SyncFrameDelay: ͬ��֡��reset֮��ĵ�SyncFrameDelay֮֡���ͣ����Ϊ0����Reset֮����������ͬ��֡
 * @return ��
 */
void SPDIF_TXInit(uint8_t ParitySrc, uint8_t IsValidityEn, uint8_t ChannelMode, uint8_t SyncFrameDelay);

/**
 * @brief  ��SPDIF����Ϊ����ģʽ�����г�ʼ��
 * @param  ParitySrc�����ɼ���Ԫ��Դѡ��0 - ������� 1 - Ӳ������
 * @param  IsValidityEn: ��Чλ�Ƿ���м�⣬0 - ��Чλ����⣬ȫ��д��FIFO�У� 
                                            1 - ֻ����Чλ��ȷ��д��FIFO��
 * @param  ChannelMoe: 0: ģʽ0 - SPDIF��Ϊ����ģ�����ģ��ʱ����Ϊ˫��������
                       1: ģʽ1 - SPDIF��Ϊ����ģ��ʱ��ֻ��������������д��FIFO�У���Ϊ����ģ��ʱ������������������������������0
                       2: ģʽ2 - SPDIF��Ϊ����ģ��ʱ��ͬģʽ1��ͬ����Ϊ����ģ��ʱ����������������������������Ϊ��������������
 * @return ��
 */
void SPDIF_RXInit(uint8_t ParitySrc, uint8_t IsValidityEn, uint8_t ChannelMode);

/**
 * @brief  ʹ��SPDIFģ��
 * @param  ��
 * @return ��
 */
void SPDIF_ModuleEnable(void);

/**
 * @brief  ����SPDIFģ��
 * @param  ��
 * @return ��
 */
void SPDIF_ModuleDisable(void);

/**
 * @brief  ��ȡSPDIF״̬��־λ
 * @param  FlagType�� SPDIF��־���ͣ���� #SPDIF_STATUS_TYPE
 * @return ��־״̬
 */
bool SPDIF_FlagStatusGet(SPDIF_STATUS_TYPE FlagType);

// *******************************************************************************************************************
//                                                  �������жϲ���
// *******************************************************************************************************************
/**
 * @brief  ������ر�SPDIF�ж�
 * @param  IntType: SPDIF�ж����ͣ���� #SPDIF_INT_TYPE
 * @param  IsEnable: 0 - ���ܸ������жϣ�1 - �����������ж�
 * @return ��
 */
void SPDIF_SetInt(SPDIF_INT_TYPE IntType, bool IsEnable);

/**
 * @brief  ��ȡ�жϱ�־
 * @param  IntType: SPDIF�ж����ͣ���� #SPDIF_INT_TYPE
 * @return �жϱ�־
 */
bool SPDIF_GetIntFlg(SPDIF_INT_TYPE IntType);

/**
 * @brief  ����жϱ�־
 * @param  IntType: SPDIF�ж����ͣ���� #SPDIF_INT_TYPE
 * @return ��
 */
void SPDIF_ClearIntFlg(SPDIF_INT_TYPE IntType);

// *******************************************************************************************************************
//                                        ������PCM��ʽ��SPDIF��ʽ֮���ת��
// *******************************************************************************************************************

/**
 * @brief  PCM��ʽ����תSPDIF��ʽ����
 * @param  PcmBuf��PCM˫�������ݸ�ʽ��LRLR...
 * @param  Length: PCM���ݵĳ��ȣ���λΪ��Byte
 * @param  SpdifBuf: SPDIF��ʽ����
 * @param  Wordlth: ���ݿ�ȣ����#SPDIF_DATA_WORDLTH
 * @return SPDIF���ݳ��ȣ���λ��Byte
 */
int32_t SPDIF_PCMDataToSPDIFData(int32_t *PcmBuf, int32_t Length, int32_t *SpdifBuf, SPDIF_DATA_WORDLTH Wordlth);

/**
 * @brief  SPDIF��ʽ����תPCM��ʽ����
 * @param  SpdifBuf�����SPDIF���ݵ��׵�ַ
 * @param  Length: SPDIF���ݵĳ��ȣ���λΪ��Byte
 * @param  PcmBuf: ���PCM���ݵ��׵�ַ
 * @param  Wordlth: ���ݿ�ȣ����#SPDIF_DATA_WORDLTH
 * @return >0: PCM���ݳ��ȣ���λ��Byte;
 */
int32_t SPDIF_SPDIFDataToPCMData(int32_t *SpdifBuf, int32_t Length, int32_t *PcmBuf, SPDIF_DATA_WORDLTH Wordlth);


void SPDIF_SPDIFToAudioStatusClear(void);

/**
 * @brief  SPDIF��ʽ����תAudio��ʽ����,�������Ժͷ����ԡ�
 * @param  SpdifBuf�����SPDIF���ݵ��׵�ַ
 * @param  Length: SPDIF���ݵĳ��ȣ���λΪ��Byte
 * @param  AudioBuf: ���AudioBuf���ݵ��׵�ַ
 * @param  Wordlth: ���ݿ�ȣ����#SPDIF_DATA_WORDLTH
 * @param  p: ��ŵ�ǰ���֡�Ĵ�С��Byte����ͨ���ţ����͵������Ϣ
 * @return ��;
 */
void SPDIF_SPDIFDatatoAudioData(int32_t *SpdifBuf, int32_t InLength, int32_t *AudioBuf, SPDIF_DATA_WORDLTH Wordlth, SPDIF_TYPE_STR *p);

/**
 * @brief  ��20bit��24bitʱ�����ݸ�ʽѡ��
 * @param  AlignMode I2S_20BIT_24BIT_ALIGN_MODE
 *
 * @return NONE
 */
void SPDIF_AlignModeSet(SPDIF_20BIT_24BIT_ALIGN_MODE AlignMode);

/**
 * @brief	����spdifʱ��Դ
 * @param	SourceType 0: PLL, 1: AUPLL
 */
void SPDIF_ClockSourceSelect(SPIDF_CLOCK_SOURCE_INDEX SourceType);

//clockValue spdif���Ե�ʱ��ֵ��Hz
//sampleRate ������ Hz
//�����Ƽ���spdif��Ƶֵ
uint32_t SpdifGetSpdifBetterDivValue(uint32_t clockValue, uint32_t sampleRate);

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif//__SPDIF_H__

/**
 * @}
 * @}
 */
