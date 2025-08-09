/**
 *****************************************************************************
 * @file     otg_detect.h
 * @author   Owen
 * @version  V1.0.0
 * @date     27-03-2017
 * @brief    otg port detect module driver interface
 *****************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2015 MVSilicon </center></h2>
 */

/**
 * @addtogroup OTG
 * @{
 * @defgroup otg_detect otg_detect.h
 * @{
 */
#ifndef __OTG_DETECT_H__
#define	__OTG_DETECT_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus 


#include "type.h"

/**
 * @brief  Port���Ӽ�⺯��
 * @param  NONE
 * @return NONE
 */
void OTG_PortLinkCheck(void);

/**
 * @brief  ���Port�˿����Ƿ���һ��U���豸����
 * @param  NONE
 * @return 1-��U���豸���ӣ�0-��U���豸����
 */
bool OTG_PortHostIsLink(void);

/**
 * @brief  ���Port�˿����Ƿ���һ��PC����
 * @param  NONE
 * @return 1-��PC���ӣ�0-��PC����
 */
bool OTG_PortDeviceIsLink(void);


/**
 * @brief  ʹ��port�˿ڼ��U������
 * @param  HostEnable: �Ƿ���U������
 * @param  DeviceEnable: �Ƿ���PC����
 * @return NONE
 */
void OTG_PortSetDetectMode(bool HostEnable, bool DeviceEnable);

/**
 * @brief  ʹ��port�˿ڼ����ͣ
 * @param  PauseEnable: �Ƿ���ͣ���
 * @return NONE
 */
void OTG_PortSetDetectPause(bool PauseEnable);

/**
 * @brief  ��ȡPort��DP��ƽ
 * @param  NONE
 * @return 1-DP��ƽΪ�ߣ�0-DP��ƽΪ��
 */
bool OTG_PortGetDP(void);

/**
 * @brief  ��ȡPort��DM��ƽ
 * @param  NONE
 * @return 1-DM��ƽΪ�ߣ�0-DM��ƽΪ��
 */
bool OTG_PortGetDM(void);

/**
 * @brief  ʹ��Port DP��1.2K��������
 * @param  NONE
 * @return NONE
 */
void OTG_PortEnableDPPullUp(void);

/**
 * @brief  ��ֹPort DP��1.2K��������
 * @param  NONE
 * @return NONE
 */
void OTG_PortDisableDPPullUp(void);

/**
 * @brief  �ж�Port DP��1.2K���������Ƿ�ʹ��
 * @param  NONE
 * @return 1-ʹ�ܣ�0-δʹ��
 */
bool OTG_PortIsEnableDPPullUp(void);

/**
 * @brief  ʹ��Port DP��DM��15K��������
 * @param  NONE
 * @return NONE
 */
void OTG_PortEnablePullDown(void);

/**
 * @brief  ȡ��Port DP��DM��15K��������
 * @param  NONE
 * @return NONE
 */
void OTG_PortDisablePullDown(void);

/**
 * @brief  �ж�Port DP��DM��15K���������Ƿ�ʹ��
 * @param  NONE
 * @return 1-ʹ�ܣ�0-δʹ��
 */
bool OTG_PortIsEnablePullDown(void);

/**
 * @brief  ʹ��Port DP��DM��500K��������
 * @param  NONE
 * @return NONE
 */
void OTG_PortEnablePullUp500K(void);
/**
 * @brief  ȡ��Port DP��DM��500K��������
 * @param  NONE
 * @return NONE
 */
void OTG_PortDisablePullUp500K(void);

#ifdef __cplusplus
}
#endif // __cplusplus 

#endif //__OTG_DETECT_H__

/**
 * @}
 * @}
 */
