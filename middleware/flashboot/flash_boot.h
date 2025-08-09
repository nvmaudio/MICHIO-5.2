#ifndef __FLASH_BOOT_H__
#define __FLASH_BOOT_H__

#include "app_config.h"
#include "flash_table.h"
/*
�汾˵������ǰΪV3.0.6�汾
���ڣ�2021��11��10��
*/
#define FLASH_BOOT_EN      1

//TX PIN
#define BOOT_UART_TX_OFF	0//�رմ���
#define BOOT_UART_TX_A0		1
#define BOOT_UART_TX_A1		2
#define BOOT_UART_TX_A6		3
#define BOOT_UART_TX_A10	4
#define BOOT_UART_TX_A19	5
#define BOOT_UART_TX_A25	6
#ifdef CFG_UART_TX_PORT
	#if CFG_UART_TX_PORT == 0
		#define BOOT_UART_TX_PIN	BOOT_UART_TX_A6
	#elif CFG_UART_TX_PORT == 1
		#define BOOT_UART_TX_PIN	BOOT_UART_TX_A10
	#elif CFG_UART_TX_PORT == 2
		#define BOOT_UART_TX_PIN	BOOT_UART_TX_A25
	#elif CFG_UART_TX_PORT == 3
		#define BOOT_UART_TX_PIN	BOOT_UART_TX_A0	
	#elif CFG_UART_TX_PORT == 4
		#define BOOT_UART_TX_PIN	BOOT_UART_TX_A1 
	#else
		#define BOOT_UART_TX_PIN	BOOT_UART_TX_A6
	#endif
#else
	#define BOOT_UART_TX_PIN	BOOT_UART_TX_OFF
#endif

//����������
#define BOOT_UART_BAUD_RATE_9600	0
#define BOOT_UART_BAUD_RATE_115200	1
#define BOOT_UART_BAUD_RATE_256000	2
#define BOOT_UART_BAUD_RATE_512000	3
#define BOOT_UART_BAUD_RATE_1000000	4
#define BOOT_UART_BAUD_RATE_1500000	5
#define BOOT_UART_BAUD_RATE_3000000	6
#ifdef CFG_UART_BANDRATE
	#if CFG_UART_BANDRATE == 9600
		#define BOOT_UART_BAUD_RATE		BOOT_UART_BAUD_RATE_9600
	#elif CFG_UART_BANDRATE == 115200
		#define BOOT_UART_BAUD_RATE		BOOT_UART_BAUD_RATE_115200
	#elif CFG_UART_BANDRATE == 256000
		#define BOOT_UART_BAUD_RATE		BOOT_UART_BAUD_RATE_256000	
	#elif CFG_UART_BANDRATE == 512000
		#define BOOT_UART_BAUD_RATE		BOOT_UART_BAUD_RATE_512000	
	#elif CFG_UART_BANDRATE == 1000000
		#define BOOT_UART_BAUD_RATE		BOOT_UART_BAUD_RATE_1000000	
	#elif CFG_UART_BANDRATE == 1500000
		#define BOOT_UART_BAUD_RATE		BOOT_UART_BAUD_RATE_1500000	
	#elif CFG_UART_BANDRATE == 3000000
		#define BOOT_UART_BAUD_RATE		BOOT_UART_BAUD_RATE_3000000	
	#else
		#define BOOT_UART_BAUD_RATE		BOOT_UART_BAUD_RATE_512000
	#endif
#else
	#define BOOT_UART_BAUD_RATE		BOOT_UART_BAUD_RATE_512000
#endif

#define BOOT_UART_CONFIG	((BOOT_UART_BAUD_RATE<<4)+BOOT_UART_TX_PIN)


/*  JUDGEMENT_STANDARD˵��
 * �ָ�4bit���4bit��
 *   ��4bit��
 *      ΪF��code���汾������
 *      Ϊ5��code��CRC��������
 *   ��4bit:
 *     ΪF��������code��Ҫ�õ����ռ伴�������ռ�
 *     Ϊ5ʱ���ʶ����codeǰȫ������оƬ���ݣ���������ȫƬ����������flash��0��ַ��ʼflashbootռ�ÿռ䲻�����Լ����8K��������
 * ���磺0x5F ��Ϊ�Ƚ�CODE CRC�ж��Ƿ���Ҫ����������ʱ�����ֲ�����������ȫ����
 */
#define JUDGEMENT_STANDARD		0x55

///�����ӿڶ���
#define	SD_OFF				0x00
#define SD_A15A16A17		0x1
#define SD_A20A21A22		0x2
#if CFG_RES_CARD_GPIO == SDIO_A15_A16_A17
#define SD_PORT				SD_A15A16A17
#else
#define SD_PORT				SD_A20A21A22
#endif

#define UDisk_OFF			0x00
#define UDisk_ON			0x4

#define PCTOOL_OFF			0x00
#define PCTOOL_ON			0x08

#define	BTTOOL_OFF			0X00
#define BTTOOL_ON			0X10

#ifdef CFG_FUNC_BT_OTA_EN
#define UP_PORT				(BTTOOL_OFF + PCTOOL_ON + UDisk_ON + SD_PORT + BTTOOL_ON)
#else
#define UP_PORT				(BTTOOL_OFF + PCTOOL_ON + UDisk_ON + SD_PORT)//����Ӧ�������������Щ�����ӿ�
#endif

#if FLASH_BOOT_EN
extern const unsigned char flash_data[];
#endif

#define USER_CODE_RUN_START		0 	//����������ֱ�����пͻ�����
#define UPDAT_OK				1 	//���������������ɹ�
#define NEEDLESS_UPDAT			2	//���������󣬵���������

#endif


#define BOOT_UART_CONFIG			((BOOT_UART_BAUD_RATE<<4)+BOOT_UART_TX_PIN)


#if FLASH_BOOT_EN
extern const unsigned char flash_data[];
#endif


