/**
 **************************************************************************************
 * @file    bluetooth_tws_connect.h
 * @brief	tws��ع��ܺ����ӿ�
 *
 * @author  kk
 * @version V1.0.0
 *
 * $Created: 2019-10-28 18:00:00$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#ifndef __BT_TWS_APP_FUNC_H__
#define __BT_TWS_APP_FUNC_H__

#include "type.h"
#include "bt_manager.h"

#define BT_TWS_RECONNECT_TIMEOUT		50*30 //(����20ms����һ��,���30s��ʱ)


typedef uint32_t BT_TWS_EVENT_BIT;

#define BT_TWS_EVENT_NONE				0x00
#define BT_TWS_EVENT_PAIRING			0x01	//�������
#define BT_TWS_EVENT_SLAVE_TIMEOUT		0x02	//����slave��ʱ,�������ɱ������ɱ�����״̬
#define BT_TWS_EVENT_LINKLOSS			0x04	//���Ӷ�ʧ,slave����
#define BT_TWS_EVENT_SIMPLE_PAIRING		0x08	//����TWS�������
#define BT_TWS_EVENT_PAIRING_READY		0x10	//����TWS���ǰ׼������(�ȴ�TWS�Ͽ�)
#define BT_TWS_EVENT_REPAIR_CLEAR		0x20	//�������,�ȴ�˫��������

typedef struct _BtTwsAppCt
{
	BT_TWS_EVENT_BIT	btTwsEvent;

	//��Գ�ʱ�Ĵ���
	uint32_t			btTwsPairingStart;
	uint32_t			btTwsPairingTimeroutCnt;
	uint32_t			btTwsRepairTimerout;

	//link loss, slave���������ȴ�count
	uint32_t			btTwsLinkLossTimeoutCnt;

	uint32_t			btTwsSlaveTimeoutCount;	//��ʱ����ɱ������ɱ�����״̬
}BtTwsAppCt;

extern BtTwsAppCt *gBtTwsAppCt;

void BtTwsAppInit(void);
void BtTwsAppDeinit(void);

void BtTwsDeviceReconnect(void);

void BtTwsDeviceDisconnect(void);

void BtTwsDeviceDisconnectExt(void);

void BtTwsLinkLoss(void);

void BtTwsConnectApi(void);
void BtTwsDisconnectApi(void);

void BtTwsEnterPeerPairingMode(void);

void BtTwsExitPeerPairingMode(void);

void CheckBtTwsPairing(void);
void BtTwsEnterPairingMode(void);
void BtTwsExitPairingMode(void);
void BtTwsPeerEnterPairingMode(void);
void BtTwsEnterSimplePairingMode(void);//soundbar
void BtTwsExitSimplePairingMode(void);
void BtTwsRepairDiscon(void);
void BtTwsRunLoop(void);
//void sniff_wakeup_set(uint8_t set);
//uint8_t sniff_wakeup_get();
void sniff_lmpsend_set(uint8_t set);
uint8_t sniff_lmpsend_get();
bool BtTwsPairingState(void);
void tws_msg_process(uint16_t msg);
void BtTwsPairingStart(void);

#endif /*__BT_TWS_APP_FUNC_H__*/



