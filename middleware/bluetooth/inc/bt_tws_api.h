/**
 *******************************************************************************
 * @file    bt_tws_api.h
 * @author  Owen
 * @version V1.0.0
 * @date    19-Sep-2019
 * @brief   tws callback events and actions
 *******************************************************************************
 * @attention
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, MVSILICON SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
 * INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <h2><center>&copy; COPYRIGHT 2013 MVSilicon </center></h2>
 */

/**
* @addtogroup Bluetooth
* @{
* @defgroup bt_a2dp_api bt_a2dp_api.h
* @{
*/

#include "type.h"

#ifndef __BT_TWS_API_H__
#define __BT_TWS_API_H__
#ifdef CFG_APP_CONFIG
#include "app_config.h"
typedef unsigned char  U8;
typedef	int BOOL;
#else
#include "xatypes.h"
#endif

//#define TWS_CODE_BACKUP //ԭ��tws audioͬ��״̬Ǩ�ƴ������Σ��˺�򿪺�ָ�ִ�С�
#define TWS_DBG(format, ...)		printf(format, ##__VA_ARGS__)

typedef enum{
	BT_STACK_EVENT_TWS_NONE = 0,

	BT_STACK_EVENT_TWS_CONNECTED,

	BT_STACK_EVENT_TWS_DISCONNECT,

	BT_STACK_EVENT_TWS_DATA_IND,

	//master event
	BT_STACK_EVENT_TWS_MASTER_CONNECTED,
	
	BT_STACK_EVENT_TWS_MASTER_DISCONNECT,
		
	BT_STACK_EVENT_TWS_MASTER_DATA_IND,

	BT_STACK_EVENT_TWS_ACL_DATA_LINK,
	
	BT_STACK_EVENT_TWS_ACL_DATA_UNLINK,
	
	//slave event
	BT_STACK_EVENT_TWS_SLAVE_CONNECTED,
	
	BT_STACK_EVENT_TWS_SLAVE_DISCONNECT,
		
	BT_STACK_EVENT_TWS_SLAVE_DATA_IND,

	BT_STACK_EVENT_TWS_SLAVE_STREAM_START,

	BT_STACK_EVENT_TWS_SLAVE_STREAM_STOP,

	BT_STACK_EVENT_TWS_MUTE,
	
	//��ǰHOST��·�쳣,�ӳ���������
	BT_STACK_EVENT_TWS_CONNECT_DELAY,
	//��ǰTWS��·����,ȡ����������
	BT_STACK_EVENT_TWS_CONNECT_LINK_EXIST,
	
}BT_TWS_CALLBACK_EVENT;

typedef enum
{
	BT_TWS_MASTER = 0x00,
	BT_TWS_SLAVE,
	BT_TWS_UNKNOW = 0xff,
} BT_TWS_ROLE;

#ifndef TWS_PAIR_STATE
typedef enum
{
	TWS_PAIR_NONE = 0,
	TWS_PAIR_STOP,
	TWS_PAIR_START,			//��ʱ2s��INQUIRY
	TWS_PAIR_WAITING,		//slave�ȴ����
	TWS_PAIR_INQUIRY,		//
	TWS_PAIR_PAIRING,
	TWS_PAIR_PAIRED,
	TWS_PAIR_STOPPING,

	//unused
	TWS_PAIR_BLE_START,
	TWS_PAIR_BLE_CONNECTING,
	TWS_PAIR_BLE_CONNECTED,
	TWS_PAIR_BLE_STOPPING,
} TWS_PAIR_STATE;
#endif

typedef struct _BT_TWS_CALLBACK_PARAMS
{
	BT_TWS_ROLE					role;
	uint16_t 					paramsLen;
	bool						status;
	uint16_t		 			errorCode;

	union
	{
		uint8_t					*bd_addr;
		uint8_t					*twsData;
	}params;
}BT_TWS_CALLBACK_PARAMS;

typedef void (*BTTwsCallbackFunc)(BT_TWS_CALLBACK_EVENT event, BT_TWS_CALLBACK_PARAMS * param);

void BtTwsCallback(BT_TWS_CALLBACK_EVENT event, BT_TWS_CALLBACK_PARAMS * param);

typedef struct _twsAppFeatures
{
	uint32_t			twsSimplePairingCfg;	//0=both sides;  1=one side;
	uint32_t			twsRoleCfg;		//0=random; 1=master; 2=slave
	BTTwsCallbackFunc	twsAppCallback;
}TwsAppFeatures;

//for bt tws link
typedef enum
{
	BT_TWS_STATE_NONE,
	BT_TWS_STATE_DISCONNECT,
	BT_TWS_STATE_CONNECTING,
	BT_TWS_STATE_WAITING_RS,//���ӳɹ���,�ȴ�role switch
	BT_TWS_STATE_WAITING_CONFIRM,
	BT_TWS_STATE_SNIFF,//����ʵ
	BT_TWS_STATE_CONNECTED,
} BT_TWS_STATE;

//for tws lmp cmd ���32��cmd
typedef enum{
	TWS_CMD_CONNECT,		//0
	TWS_CMD_DEVICE_OPEN,	//1
	TWS_CMD_HW_INIT,		//2
	TWS_CMD_BASE,			//3
	TWS_CMD_GETBASE,		//4
	TWS_CMD_BASE_LEN,		//5
	TWS_CMD_COMPENSATION,	//6
	TWS_CMD_SYNCED,			//7
	TWS_CMD_POWERDOWN,		//8
	TWS_CMD_ACTIVE,			//9
	TWS_CMD_TWS_SYNC_STATE,	//10

	TWS_CMD_LOCAL_MASTER,	//11
	TWS_CMD_LOCAL_SLAVE,	//12
	TWS_CMD_LOCAL_SYNC,		//13
	TWS_CMD_LOCAL_STOP,		//14
}TWS_AUDIO_CMD;

#define IS_CACHE(State) ((State >= TWS_HW_INIT) && (State <= TWS_IDLE))
//for tws audio link
typedef enum{
	TWS_DISCONNECT,
	TWS_CONNECT_JITTER,
	TWS_CONNECT,
/*********tws Audio fifo.data.net switch*******************/
	TWS_DISCONNECT_JITTER,
	TWS_HW_INIT,
	TWS_BASE_POINT,
	TWS_COMPENSATION,
	TWS_AUDIO,

	TWS_WAIT_SLAVE,
	TWS_IDLE,
	TWS_RE_INIT_CMD,
	TWS_RE_INIT_CMD_DONE,
	TWS_STOP_TRANSFER,
	TWS_SLAVE_SNED_READY,
	TWS_MASTER_SNED_READY,
	TWS_MASTER_ACTIVE,
	TWS_WAIT_TRANSFER,
	TWS_CONNECT_CONFIRM,
	TWS_ACTIVE_DISCONNECTION,

	TWS_BASE_RESYNC_CMD,//����,�����ɹ�����״̬
}TWS_SYNC_STATE;

//audio_mode
#define TWS_M_LR__S_LR		0//����:������,  �ӻ�:������
#define TWS_M_L__S_R		1//����:L����,  	  �ӻ�:R����
#define TWS_M_R__S_L		2//����:R����,   �ӻ�:L����
#define TWS_M_LR__S_MONO	3//����:������,  �ӻ�:L+R���

//TWS������ɫ
typedef enum
{
	TWS_ROLE_RANDOM = 0,
	TWS_ROLE_MASTER,
	TWS_ROLE_SLAVE,
}TWS_PAIRING_ROLE;

extern uint32_t tws_delay_frames;
#define TWS_SOURCE_FRAME0		(0x1000)
#define	TWS_ENCODE_FRAME0		(TWS_SOURCE_FRAME0 + tws_delay_frames)//������ʼ֡��


/***************************************************************************
 *bt tws link state
 ****************************************************************************/
void tws_link_state_set(BT_TWS_STATE state);

BT_TWS_STATE tws_link_state_get(void);

void tws_active_disconnection(void);

void tws_active_disconnection_set(uint8_t val);

uint8_t tws_active_disconnection_get(void);

//����֮�����ڽ����쳣���Ͽ�,��Ҫ�ϲ��ڶϿ�����л�������
U8 tws_reconnect_flag_get(void);

//tws��ǰ�Ľ�ɫ,����tws_sync��mv_lmp_send����ʹ�õ�role
uint8_t tws_cur_role_get(void);

/***************************************************************************
 * tws�ڴ����
 ****************************************************************************/
uint32_t tws_mem_size(uint32_t depth_frames,uint8_t audio_mode);
uint32_t tws_mem_set(uint8_t*mem);
void tws_delay_set(uint32_t delay_frames);

//bt task
/***************************************************************************
 * tws initializer
 ****************************************************************************/
BOOL twsInit(TwsAppFeatures *params);

//bt task
/***************************************************************************
 * tws �������̴���
 ****************************************************************************/
//peer-master/peer-slave/radom
void tws_start_pairing_radom(void);

void tws_stop_pairing_radom(void);

void tws_start_pairing(TWS_PAIRING_ROLE role);

/***************************************************************************
 * tws start simple pairing
 * role: slave
 ****************************************************************************/
//soundbar slave
void tws_slave_start_simple_pairing(void);

void tws_slave_stop_simple_pairing(void);

void tws_slave_simple_pairing_ready(void);

void tws_slave_simple_pairing_end(void);

/***************************************************************************
 * tws connect�������
 ****************************************************************************/
void tws_connect(U8 *addr);

/***************************************************************************
 * tws link disconnect
 ****************************************************************************/
void tws_link_disconnect(void);

uint8_t *Get_Peer_Addr(void);

//TWS�����շ��ӿ�,�����������ݸ�ʽ����
void tws_master_send(uint8_t *buf,uint16_t len);

void tws_slave_send(uint8_t *buf,uint16_t len);

/***************************************************************************
 * tws avrcp���� -- L2CAP
 ****************************************************************************/
void tws_slave_send_cmd_play_pause(void);

void tws_slave_send_cmd_next(void);

void tws_slave_send_cmd_prev(void);

void tws_slave_send_cmd_sniff(void);

/***************************************************************************
 * tws ���ӳɹ��ڲ�״̬ͬ����ʼ��
 ****************************************************************************/
void tws_audio_init_sync(void);

/***************************************************************************
 * tws ����� -- LMP
 ****************************************************************************/
void tws_connect_confirm_cmd(void);

void tws_sync_powerdown_cmd(void);

//master ����
void tws_stop_transfer(void);

void tws_wait_transfer(void);

void tws_slave_send_ready(void);

void tws_master_send_ready(void);

void tws_master_active_cmd(void);

/*
 * func:CMD for tws reinit
 */
void tws_sync_reinit(void);


void tws_audio_connect_cmd(void);


/***************************************************************************
 * func:tws play set device, for interrupt
 ****************************************************************************/
void tws_device_sync_isr(uint8_t cmd);

void tws_audio_bt_state(BT_TWS_STATE event);

/*
 * func: get TWS audio link sync state
 * return :state
 */
uint8_t tws_audio_state_get(void);

/***************************************************************************
 * tws �ڲ�״̬ѭ��������
 ****************************************************************************/
void tws_audio_state_sync(void);

/*
 * p_m������Ƶ����
 * len ��Ƶ���ݵ�sample��
 * data_source ��Ƶ���ݵ�ͨ· app������ʾ��
 */
void tws_music_pcm_process(int16_t *p_m, uint32_t len, uint8_t data_source);

/*
 * func:��ȡTWS fifo ��ʱ֡���趨 ��λ:128��
 * return :TWS fifo ��ʱ֡���趨ֵ
 */
uint32_t tws_get_delay(void);

/*
 * func:TWS fifo ��ʱ֡���Ϳ�����չ��ʱ֡��  ��λ:128��
 * cmd����ͬ�������ӿ�ʼ׼��audio����
 * delay:��չ��ʱ��mute֡��  ��λ:128��
 * start_paly:TWS fifo ������չ��ʱ֡��
 */
void tws_start(uint32_t delay,uint32_t start_paly);


/*
 * func:��TWS fifo����֡��ȡ������
 * Buf: ����buf
 * Samples:�������� 128����
 * return :ʵ�ʲ�������
 */
uint16_t TWSDataGet(void* Buf, uint16_t Samples);

/*
 * func:��TWS fifo���ݲ�������
 * return :TWS fifo��������
 */
uint16_t TWSDataLenGet(void);


/*******************************************************************************
 **************************Audio Device Sync adjust********************************
 *********************************************************************************/
/*
 * func:TWS �����豸���fifo֡����
 * Len: = 128*n ��������
 * return: ʵ��set��������
 */
uint16_t TwsDeviceSinkSync(uint16_t Len);

//App:
BT_TWS_ROLE tws_get_role(void);
/**************************************************************************
 *  tws audio
 *************************************************************************/
bool tws_state_audiocore_assort(TWS_SYNC_STATE State, TWS_SYNC_STATE LastState);

//Tws Audio ��λ����
void tws_time_init(void);

uint16_t tws_device_space_len(void);



void tws_local_audio_set(uint16_t ItemRef, TWS_AUDIO_CMD CMD);

uint32_t SoundRemindItemGet(void);

uint8_t tws_local_audio_wait(void);

void RemindSoundSyncEnable(void);

bool RemindSoundSyncRequest(uint16_t ItemRef, TWS_AUDIO_CMD CMD);

void tws_local_audio_stop(uint16_t ItemRef);

void tws_device_open_isr(void);

#endif /*__BT_TWS_API_H__*/

/**
 * @}
 * @}
 */

