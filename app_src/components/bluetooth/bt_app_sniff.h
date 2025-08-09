/**
 **************************************************************************************
 * @file    bluetooth_sniff.h
 * @brief	bluetooth sniff��ع��ܺ����ӿ�
 *
 * @author  kk
 * @version V1.0.0
 *
 * $Created: 2019-10-28 18:00:00$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#ifndef __BLUETOOTH_SNIFF_H__
#define __BLUETOOTH_SNIFF_H__

#include "bt_config.h"

#ifdef BT_SNIFF_ENABLE

typedef struct _BT_SNIFF_CTRL
{
	uint8_t bt_sniff_start       : 1;
	uint8_t bt_sleep_enter       : 1;
#ifndef BT_TWS_SUPPORT
	uint8_t bt_sleep_fastpower_f : 1;
#endif
}_BT_SNIFF_CTRL;

//����sniff�Ĵ�������
void Bt_sniff_state_init(void);
//����deepsleep flag
void Bt_sniff_sleep_enter(void);
void Bt_sniff_sleep_exit(void);
uint8_t Bt_sniff_sleep_state_get(void);

//����sniff flag
void Bt_sniff_sniff_start(void);
void Bt_sniff_sniff_stop(void);
uint8_t Bt_sniff_sniff_start_state_get(void);

void send_sniff_msg(void);


#ifndef BT_TWS_SUPPORT
//���������Ƿ�ʹ��fastpower������sniffʱDIS��changemodeʱEN
void Bt_sniff_fastpower_dis(void);
void Bt_sniff_fastpower_en(void);
uint8_t Bt_sniff_fastpower_get(void);
#endif

//TWS����sniff����ʱADDA�Ƿ�׼���õı�־
//1bit:Master OK   2bit:Slave OK
void BtSniffADDAReadySet(uint8_t set);
uint8_t BtSniffADDAReadyGet();

uint8_t Bt_sniff_sniff_start_state_get(void);
#else

//ifndef BT_SNIFF_ENABLE
uint8_t Bt_sniff_sniff_start_state_get(void);


#endif//BT_SNIFF_ENABLE

#endif /*__BLUETOOTH_SNIFF_H__*/



