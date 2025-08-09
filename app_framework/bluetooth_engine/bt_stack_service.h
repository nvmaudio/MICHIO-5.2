/**
 **************************************************************************************
 * @file    bt_stack_service.h
 * @brief   
 *
 * @author  kk
 * @version V1.0.0
 *
 * $Created: 2016-6-29 13:06:47$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#ifndef __BT_STACK_SERVICE_H__
#define __BT_STACK_SERVICE_H__

#include "type.h"
#include "rtos_api.h"

uint8_t GetBtStackCt(void);

xTaskHandle GetBtStackServiceTaskHandle(void);
uint8_t GetBtStackServiceTaskPrio(void);

MessageHandle GetBtStackServiceMsgHandle(void);
void BtStackServiceMsgSend(uint16_t msgId);
/**
 * @brief	Start bluetooth stack service.
 * @param	NONE
 * @return  
 */
bool BtStackServiceStart(void);
/**
 * @brief	Kill bluetooth stack service.
 * @param	NONE
 * @return  
 */
bool BtStackServiceKill(void);
void BtBbStart(void);
void BT_IntDisable(void);
void BT_ModuleClose(void);
void BtStackServiceWaitResume(void);
void BtStackServiceWaitClear(void);
#ifdef	BT_SNIFF_ENABLE
void BtStartEnterSniffMode(void);
void BtStartEnterSniffStep(void);
void BtExitSniffReconnectPhone(void);
void BtExitSniffReconnectFlagSet(void);
#endif//BT_SNIFF_ENABLE
/***********************************************************************************
 * ��������DUTģʽ
 * �˳�DUT��,���ϵͳ����
 **********************************************************************************/
void BtEnterDutModeFunc(void);
/***********************************************************************************
 * ���ٿ�������
 * ����֮ǰ���ӹ��������豸
 * ����Э��ջ�ں�̨��������,δ�ر�
 **********************************************************************************/
void BtFastPowerOn(void);
/***********************************************************************************
 * ���ٹر�����
 * �Ͽ��������ӣ��������벻�ɱ����������ɱ�����״̬
 * δ�ر�����Э��ջ
 **********************************************************************************/
void BtFastPowerOff(void);

void BtLocalDeviceNameUpdate(char *deviceName);


void BtPowerOff(void);
void BtPowerOn(void);



#endif //__BT_STACK_SERVICE_H__

