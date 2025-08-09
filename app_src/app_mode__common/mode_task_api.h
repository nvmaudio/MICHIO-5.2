/*
 * mode_task_api.h
 *
 *  Created on: Mar 30, 2021
 *      Author: piwang
 */

#ifndef _MODE_TASK_API_H_
#define _MODE_TASK_API_H_
#include "type.h"
#include "i2s.h"

//����ϵͳ��׼ͨ·
bool ModeCommonInit(void);

//������׼ͨ·���ã�ģʽ�˳���δ����ʱ ��׼ͨ·��̨������ģʽ�ڼ�����Ч��
void ModeCommonDeinit(void);

//��ģʽ�µ�ͨ����Ϣ����
void CommonMsgProccess(uint16_t Msg);

bool AudioIoCommonForHfp(uint32_t sampleRate, uint16_t gain, uint8_t gainBoostSel);

void tws_device_close(void);

void tws_device_open_isr(void);

#ifdef CFG_AUDIO_OUT_AUTO_SAMPLE_RATE_44100_48000
void AudioOutSampleRateSet(uint32_t SampleRate);
#endif

void PauseAuidoCore(void);

void AudioI2sOutParamsSet(void);
void I2S_GPIO_Port_ModeSet(I2S_MODULE I2SModuleIndex, uint8_t i2s_mode);



void AudioI2s0ParamsSet(void);
void AudioI2s1ParamsSet(void);


#ifdef CFG_FUNC_I2S_MIX_MODE

void I2sMixModeInit(void);
void I2sMixModeDeInit(void);
void I2sMixModeInit_HFP(void);
#endif

#ifdef CFG_RES_AUDIO_SPDIFOUT_EN
void AudioSpdifOutParamsSet(void);
bool spdif_out_init(void);
bool spdif_out_hfp_init(void);
bool spdif_out_deinit(void);
#endif

#endif /* _MODE_TASK_API_H_ */
