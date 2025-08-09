//  maintainer: 
#ifndef __POWER_MONITOR_H__
#define __POWER_MONITOR_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

// ��ص�ѹ�ȼ�����0 ~ 10��0��ʾ��ʾ�͵�ػ���1��ʾ�͵�����ʾ
typedef enum _PWR_LEVEL
{
	PWR_LEVEL_0 = 0,
	PWR_LEVEL_1,
	PWR_LEVEL_2,
	PWR_LEVEL_3,
	PWR_LEVEL_4,
	PWR_LEVEL_5,
	PWR_LEVEL_6,
	PWR_LEVEL_7,
	PWR_LEVEL_8,
	PWR_LEVEL_9,
	PWR_FULL,      //max level
} PWR_LEVEL;

/*
**********************************************************
*					��������
**********************************************************
*/
//���ܼ��ӳ�ʼ��
//ʵ��ϵͳ���������еĵ͵�ѹ��⴦���Լ����ó���豸������IO��
void PowerMonitorInit(void);

//��ȡ��ǰ��ص���,�������ϴ��������ֻ���
//return: level(0-9)
PWR_LEVEL PowerLevelGet(void);

//��ص�ѹ����
void PowerVoltageSampling(void);

void BatteryScan(void);

void PowerMonitorDisp(void);

extern void SetBtHfpBatteryLevel(PWR_LEVEL level, uint8_t flag);


#ifdef  __cplusplus
}
#endif//__cplusplus

#endif
