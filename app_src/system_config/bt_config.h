
///////////////////////////////////////////////////////////////////////////////
//               Mountain View Silicon Tech. Inc.
//  Copyright 2012, Mountain View Silicon Tech. Inc., Shanghai, China
//                       All rights reserved.
//  Filename: bt_config.h
//  maintainer: keke
///////////////////////////////////////////////////////////////////////////////
#ifndef __BT_DEVICE_CFG_H__
#define __BT_DEVICE_CFG_H__
#include "type.h"
#include "app_config.h"

#define ENABLE						TRUE
#define DISABLE						FALSE


#define BtTrimECO0                              0x16
#define BtTrim                                  0x14 
#define bt_TxPowerLevel                         23
#define bt_PagePowerLevel                       20

#define TwsVolSyncEnable	                    TRUE
#define bt_CallinRingType	                    2
#define bt_ReconnectionEnable                   TRUE
#define bt_ReconnectionTryCounts                5
#define bt_ReconnectionInternalTime             3
#define bt_BBLostReconnectionEnable             1   
#define bt_BBLostTryCounts                      90
#define bt_BBLostInternalTime                   5
#define bt_TwsReconnectionEnable                TRUE
#define bt_TwsReconnectionTryCounts             3
#define bt_TwsReconnectionInternalTime          3
#define bt_TwsBBLostReconnectionEnable          TRUE
#define bt_TwsBBLostTryCounts                   3
#define bt_TwsBBLostInternalTime                5
#define bt_TwsPairingWhenPhoneConnectedSupport  0
#define bt_TwsConnectedWhenActiveDisconSupport  0


#ifdef CFG_APP_BT_MODE_EN

		/*
		*      Tần số của TWS
		* Giải thích: Giá trị này là 11.2986, với tần số xung nhịp hệ thống là 360M, tần số TWS cũng phải được cấu hình sao cho phù hợp.
		* 112896 * 28 = 3161088   (tương đương 316M)
		* 112896 * 31 = 3499776   (tương đương 349M)
		* Tần số của hệ thống 360M sẽ sử dụng giá trị 3499776.
		* Nếu bạn muốn tắt TWS, có thể cấu hình lại tần số hệ thống 360M thông qua hàm SystemClockInit() và thay đổi giá trị Clock_PllLock(288000) thành 360000.
		*/

	#define SYS_CORE_DPLL_FREQ            3161088	//3161088  // 3499776


	#define BT_TWS_SUPPORT

	//#define BT_MULTI_LINK_SUPPORT
		#if ( defined(CFG_RES_AUDIO_SPDIFOUT_EN) || defined(LOSSLESS_DECODER_HIGH_RESOLUTION) )
			#undef 	BT_TWS_SUPPORT
		#endif
#endif

	#define BT_SUPPORT			        ENABLE
	#define BLE_SUPPORT					DISABLE
	#if (BLE_SUPPORT == ENABLE)
		#define BLE_NAME_TOFLASH
	#endif

	#define BT_ADDR_SIZE				6
	#define BT_LSTO_DFT					8000
	#define BT_PAGE_TIMEOUT				8000

	#define	CFG_PARA_BT_SYNC				
	#define CFG_PARA_HFP_SYNC				
	
	#define BT_A2DP_SUPPORT				ENABLE 
	#define BT_SPP_SUPPORT				DISABLE	

	#if CFG_RES_MIC_SELECT
		#define BT_HFP_SUPPORT			DISABLE
	#endif

	#define BT_AUTO_ENTER_PLAY_MODE
	//#define POWER_ON_BT_ACCESS_MODE_SET
	#define BT_USER_VISIBILITY_STATE
	#define  RECON_ADD


	#ifdef BT_MULTI_LINK_SUPPORT
		#define BT_LINK_DEV_NUM				2
		#define BT_DEVICE_NUMBER			2
		#define BT_SCO_NUMBER				2

		#if (BT_LINK_DEV_NUM == 2)
		#define LAST_PLAY_PRIORITY			
		//#define BT_LINK_2DEV_ACCESS_DIS_CON		
		#endif
	#else
		#define BT_LINK_DEV_NUM				1
		#define BT_DEVICE_NUMBER			1
		#define BT_SCO_NUMBER				1
	#endif


	#ifdef BT_DEVICE_NUMBER
		#if ((BT_DEVICE_NUMBER != 2)&&(BT_DEVICE_NUMBER != 1))
		#error Conflict: BT_DEVICE_NUMBER setting error
		#endif
	#endif

	#ifdef BT_SCO_NUMBER
		#if ((BT_SCO_NUMBER != 2)&&(BT_SCO_NUMBER != 1)&&(BT_SCO_NUMBER > BT_DEVICE_NUMBER))
		#error Conflict: BT_SCO_NUMBER setting error
		#endif
	#endif



	#if BT_A2DP_SUPPORT == ENABLE
		#include "bt_a2dp_api.h"
		#define BT_AUDIO_AAC_ENABLE

		#if defined(BT_PROFILE_BQB_ENABLE) && !defined(BT_AUDIO_AAC_ENABLE)
			#define BT_AUDIO_AAC_ENABLE
		#endif
		#if defined(BT_AUDIO_AAC_ENABLE) && !defined(USE_AAC_DECODER)
			#define USE_AAC_DECODER
		#endif

		#include "bt_avrcp_api.h"
		#define BT_AVRCP_VOLUME_SYNC			ENABLE
		#define BT_AVRCP_PLAYER_SETTING			DISABLE
		#define BT_AVRCP_SONG_PLAY_STATE		DISABLE
		#define BT_AVRCP_SONG_TRACK_INFOR		DISABLE
		#define BT_AUTO_PLAY_MUSIC				DISABLE

	#endif /* BT_A2DP_SUPPORT == ENABLE */


	#if BT_HFP_SUPPORT == ENABLE
		#include "bt_hfp_api.h"
		#define BT_HFP_SUPPORT_WBS				ENABLE
		#define BT_HFP_AUDIO_DATA				HFP_AUDIO_DATA_mSBC
		//#define BT_HFP_BATTERY_SYNC
		#define BT_HFP_MODE_DISABLE
		#define BT_HFP_AEC_ENABLE
		#define BT_REMOTE_AEC_DISABLE			

		#define BT_HFP_MIC_PGA_GAIN				15  //ADC PGA GAIN +18db(0~31, 0:max, 31:min)
		#define BT_HFP_MIC_PGA_GAIN_BOOST_SEL	2
		#define BT_HFP_MIC_DIGIT_GAIN			4095
		#define BT_HFP_INPUT_DIGIT_GAIN			1100

		#define BT_HFP_AEC_ECHO_LEVEL			4 //Echo suppression level: 0(min)~5(max)
		#define BT_HFP_AEC_NOISE_LEVEL			2 //Noise suppression level: 0(min)~5(max)

		#define BT_HFP_AEC_MAX_DELAY_BLK		32
		#define BT_HFP_AEC_DELAY_BLK			4 
		//#define BT_HFP_AEC_DELAY_BLK			14 
		#define BT_HFP_CALL_DURATION_DISP
	#endif



	#ifdef BT_TWS_SUPPORT
		#include "bt_tws_api.h"

		#define BT_TWS_LINK_NUM				1		
		#define CFG_TWS_ONLY_IN_BT_MODE		DISABLE 
		#define TWS_FIFO_FRAMES				120 	//@Framesize=128, tương ứng với đường truyền âm nhạc qua TWS.
		#define TWS_DELAY_FRAMES			120		//56*2	// Độ trễ từ 112ms ~ 325ms; độ trễ của âm nhạc trong chế độ TWS, delay bằng N*2.9ms@44100Hz, giá trị này phải lớn hơn 25 và nhỏ hơn số lượng frame trong FIFO.
		//#define CFG_CMD_DELAY				10      //ms, thời gian trễ khi gửi lệnh CMD, lệnh gửi đi mất khoảng 5ms, nếu thời gian gửi dài hơn 7~20ms thì có thể gây hiện tượng pop noise, tốt nhất là 10ms.
	


		//#define TWS_FILTER_NAME						
		#define TWS_FILTER_USER_DEFINED					//max:  6  bytes
		#ifdef TWS_FILTER_USER_DEFINED
			#define TWS_FILTER_INFOR			"TWS-MV"
		#endif

		#define CFG_TWS_ROLE_RANDOM			0	
		#define CFG_TWS_ROLE_MASTER			1	
		#define CFG_TWS_ROLE_SLAVE			2	
		#define CFG_TWS_PEER_MASTER			3	
		#define CFG_TWS_PEER_SLAVE			4
	
		//#define CFG_TWS_SOUNDBAR_APP

		#ifdef CFG_TWS_SOUNDBAR_APP
			#if (BLE_SUPPORT ==	DISABLE)
				#undef 	BLE_SUPPORT
				#define BLE_SUPPORT  	ENABLE
			#endif

			#define BT_SNIFF_ENABLE
			#define TWS_PAIRING_MODE						CFG_TWS_ROLE_MASTER
			//#define TWS_PAIRING_MODE						CFG_TWS_ROLE_SLAVE
			#define TWS_SIMPLE_PAIRING_SUPPORT				ENABLE	// DISABLE
			//#define TWS_SOUNDBAR_MASTER_DISCONNECT_ENABLE
			#define CFG_TWS_ROLE_SLAVE_TEST
		#else  
			#define TWS_PAIRING_MODE			CFG_TWS_PEER_MASTER
		#endif

		#if(TWS_PAIRING_MODE != CFG_TWS_PEER_SLAVE)
			#define CFG_AUTO_ENTER_TWS_SLAVE_MODE
		#endif	

		//#define TWS_SLAVE_MODE_SWITCH_EN
		//#define TWS_POWEROFF_MODE_SYNC

	#else
		#define BT_TWS_LINK_NUM				0
	#endif

#endif /*__BT_DEVICE_CFG_H__*/

