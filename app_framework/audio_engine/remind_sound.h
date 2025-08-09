/**
 **************************************************************************************
 * @file    remind_sound.h
 * @brief   
 *
 * @author  pi
 * @version V1.0.0
 *
 * $Created: 2017-2-26 13:06:47$
 *
 * @Copyright (C) 2018, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#ifndef __REMIND_SOUND_SERVICE_H__
#define __REMIND_SOUND_SERVICE_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include "type.h"
#include "decoder.h"

#include "remind_sound_item.h"
enum
{
	MP2_WAIT_FOR_DECODE = 0xff,
	MP2_DECODE_IDLE = 0,
	MP2_DECODE_HEADER,
	MP2_DECODE_FRAME,
	MP2_DECODE_END,
};

typedef enum{
	REMIND_ITEM_IDLE,
	REMIND_ITEM_PREPARE,
	REMIND_ITEM_PLAY,
	REMIND_ITEM_UNMUTE,
	REMIND_ITEM_MUTE,
}REMIND_ITEM_STATE;
//��ʾ�����󣬼�¼��Ŀ�ַ�����
bool RemindSoundServiceItemRequest(char *SoundItem, uint32_t play_attribute);
void RemindSoundInit(void);
uint8_t RemindSoundIsPlay(void);
//void RemindSoundRun(SysModeState ModeState);
void RemindSoundClearPlay(void);
void RemindSoundClearSlavePlay(void);
uint8_t RemindSoundIsBlock(void);
bool RemindSoundIsMix(void);
uint16_t RemindDataGet(void* Buf, uint16_t Samples);
uint16_t RemindDataLenGet(void);
void RemindSoundAudioPlayEnd(void);
bool RemindSoundWaitingPlay(void);
void RemindSoundItemRequestDisable(void);

#ifndef SOUND_REMIND_0
	#define SOUND_REMIND_0 NULL
#endif
#ifndef SOUND_REMIND_1
	#define SOUND_REMIND_1 NULL
#endif
#ifndef SOUND_REMIND_2
	#define SOUND_REMIND_2 NULL
#endif
#ifndef SOUND_REMIND_3
	#define SOUND_REMIND_3 NULL
#endif
#ifndef SOUND_REMIND_4
	#define SOUND_REMIND_4 NULL
#endif
#ifndef SOUND_REMIND_5
	#define SOUND_REMIND_5 NULL
#endif
#ifndef SOUND_REMIND_6
	#define SOUND_REMIND_6 NULL
#endif
#ifndef SOUND_REMIND_7
	#define SOUND_REMIND_7 NULL
#endif
#ifndef SOUND_REMIND_8
	#define SOUND_REMIND_8 NULL
#endif
#ifndef SOUND_REMIND_9
	#define SOUND_REMIND_9 NULL
#endif
#ifndef SOUND_REMIND_CALLRING
	#define SOUND_REMIND_CALLRING NULL
#endif
#ifndef SOUND_REMIND_BTMODE
	#define SOUND_REMIND_BTMODE NULL
#endif
#ifndef SOUND_REMIND_CARDMODE
	#define SOUND_REMIND_CARDMODE NULL
#endif
#ifndef SOUND_REMIND_CONNECTE
	#define SOUND_REMIND_CONNECTE NULL
#endif
#ifndef SOUND_REMIND_DISCONNE
	#define SOUND_REMIND_DISCONNE NULL
#endif
#ifndef SOUND_REMIND_DLGUODI
	#define SOUND_REMIND_DLGUODI NULL
#endif
#ifndef SOUND_REMIND_FMMODE
	#define SOUND_REMIND_FMMODE NULL
#endif
#ifndef SOUND_REMIND_GUANJI
	#define SOUND_REMIND_GUANJI NULL
#endif
#ifndef SOUND_REMIND_GXIANMOD
	#define SOUND_REMIND_GXIANMOD NULL
#endif
#ifndef SOUND_REMIND_I2SMODE
	#define SOUND_REMIND_I2SMODE NULL
#endif
#ifndef SOUND_REMIND_KAIJI
	#define SOUND_REMIND_KAIJI NULL
#endif
#ifndef SOUND_REMIND_TWSCONNE
	#define SOUND_REMIND_TWSCONNE NULL
#endif
#ifndef SOUND_REMIND_RIGHTCHA
	#define SOUND_REMIND_RIGHTCHA NULL
#endif
#ifndef SOUND_REMIND_SHANGYIS
	#define SOUND_REMIND_SHANGYIS NULL
#endif
#ifndef SOUND_REMIND_PCMODE
	#define SOUND_REMIND_PCMODE NULL
#endif
#ifndef SOUND_REMIND_TZHOUMOD
	#define SOUND_REMIND_TZHOUMOD NULL
#endif
#ifndef SOUND_REMIND_UPANMODE
	#define SOUND_REMIND_UPANMODE NULL
#endif
#ifndef SOUND_REMIND_VOLMAX
	#define SOUND_REMIND_VOLMAX NULL
#endif
#ifndef SOUND_REMIND_AUXMODE
	#define SOUND_REMIND_AUXMODE NULL
#endif
#ifndef SOUND_REMIND_XIAYISOU
	#define SOUND_REMIND_XIAYISOU NULL
#endif

#ifndef SOUND_REMIND_BTPAIR
	#define SOUND_REMIND_BTPAIR NULL
#endif



#define REMIND_ATTR_NEED_MIX					0x80    // Cần trộn với nguồn hiện tại
#define REMIND_ATTR_NEED_HOLD_PLAY				0x40	// Khi gọi BlockPlay, cần giữ lại INTERRUPT_PLAY chưa kết thúc
#define REMIND_ATTR_NEED_CLEAR_INTTERRUPT_PLAY	0x20	// Xóa chế độ INTERRUPT_PLAY
#define REMIND_ATTR_NEED_MUTE_APP_SOURCE		0x10    // Khi phát âm báo, cần tắt tiếng nguồn APP hiện tại
#define REMIND_ATTR_CLEAR						0x08    // Xóa tất cả cờ nhắc nhở
#define REMIND_ATTR_MUTE						0x04	// Tắt tiếng (dùng cho thiết bị partner)

// Các mức độ ưu tiên (chọn 1 trong số các giá trị sau)
#define REMIND_PRIO_PARTNER						0x03	// Ưu tiên cao nhất: cho thông báo từ thiết bị partner, thường dùng trong chế độ kết hợp (TWS), không bị gián đoạn
#define REMIND_PRIO_SYS							0x02	// Ưu tiên hệ thống: phát theo thứ tự, không bị ngắt bởi thông báo cấp thấp, vẫn có thể bị dừng bởi thông báo partner
#define REMIND_PRIO_ORDER						0x01	// Phát theo thứ tự: cho phép ngắt khi có thông báo ưu tiên hơn hoặc khi vào chế độ đặc biệt
#define REMIND_PRIO_NORMAL						0x00	// Ưu tiên thường: có thể bị ngắt, thích hợp với thông báo bình thường



#define REMIND_PRIO_MASK						0x3
#ifdef __cplusplus
}
#endif//__cplusplus

#endif /* __REMIND_SOUND_SERVICE_H__ */

