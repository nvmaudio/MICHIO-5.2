/**
 **************************************************************************************
 * @file    bt_play_mode.h
 * @brief    
 *
 * @author  kk
 * @version V1.0.0
 *
 * $Created: 2017-3-17 13:06:47$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#ifndef __BT_PLAY_MODE_H__
#define __BT_PLAY_MODE_H__

#include "type.h"
#include "rtos_api.h"

typedef enum _BT_PLAYER_STATE
{
	BT_PLAYER_STATE_STOP = 0,    // ����
	BT_PLAYER_STATE_PLAYING,     // ����
	BT_PLAYER_STATE_PAUSED,       // ��ͣ
	BT_PLAYER_STATE_FWD_SEEK,
	BT_PLAYER_STATE_REV_SEEK,

	BT_PLAYER_STATE_ERROR = 0xff,
} BT_PLAYER_STATE;

/***********************************************************************************
 * ���� Bt Playģʽ ����״̬
 **********************************************************************************/
void SetBtPlayState(uint8_t state);

/***********************************************************************************
 * ��ȡ Bt Playģʽ ����״̬
 **********************************************************************************/
BT_PLAYER_STATE GetBtPlayState(void);

bool BtPlayInit(void);
bool BtPlayDeinit(void);
void BtPlayRun(uint16_t msgId);

#endif /*__BT_PLAY_MODE_H__*/



