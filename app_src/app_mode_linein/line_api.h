#include "rtos_api.h"

#ifndef __LINE_API_H__
#define __LINE_API_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus 
void MixLineInPlayResInit(void);
void MixLineInPlayDeInit(void);
void AudioLine_ConfigInit(uint16_t SampleLen);
#ifdef __cplusplus
}
#endif // __cplusplus 

#endif // __LINE_MODE_H__