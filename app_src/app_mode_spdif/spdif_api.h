#ifndef __SPDIF_API_H__
#define __SPDIF_API_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus
bool MixSpdifPlayInit(void);
bool MixSpdifPlayDeinit(void);
void AudioSpdif_DataInProcess();
#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __SPDIF_MODE_H__

