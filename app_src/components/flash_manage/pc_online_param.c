#include <string.h>
#include "type.h"
#include "sys_param.h"
#include "spi_flash.h"
#include "flash_table.h"
#include "rtos_api.h"
#include "debug.h"
#include "bt_config.h"
#include "app_config.h"
#include "bt_app_ddb_info.h"

#ifdef CFG_FUNC_FLASH_PARAM_ONLINE_TUNING_EN

#define  CFG_PC_ONLINE_MAJOR_VERSION		(1)
#define  CFG_PC_ONLINE_MINOR_VERSION		(1)
#define  CFG_PC_ONLINE_PATCH_VERSION	    (0)

#define MAGIC_NUMBER_BT_ADDR				0xB1B5B681
#define MAGIC_NUMBER_BT_NAME				0xB1B5B682
#define MAGIC_NUMBER_BLE_ADDR				0xB1B5B683
#define MAGIC_NUMBER_BLE_NAME				0xB1B5B684

#include "otg_device_hcd.h"
//���õ������ߵķ���buf
extern uint8_t  hid_tx_buf[];
static bool  	hid_tx_flag = 0;

void Union_Effect_Send(uint8_t *buf, uint32_t len)
{
	if(len > 256)
		len = 256;
	memcpy(hid_tx_buf, buf, len);
	hid_tx_flag = TRUE;
}

enum
{

	PC_ONLINE_HANDSHAKE 		= 0x10,
	PC_ONLINE_HANDSHAKE_ACK 	= 0x11,

	PC_ONLINE_READ_DATA 		= 0x20,
	PC_ONLINE_READ_DATA_ACK 	= 0x21,
	PC_ONLINE_WRITE 			= 0x30,
	PC_ONLINE_WRITE_ACK 		= 0x31,
};

enum
{
	PC_ONLINE_READ_WRITE_SUCCESS = 0x00,

	PC_ONLINE_READ_OFFSET_ERROR = 0x01,
	PC_ONLINE_READ_CMD_LEN_ERROR = 0x02,

	PC_ONLINE_WRITE_FLASH_TABLE_ERROR = 0x03,
	PC_ONLINE_WRITE_OFFSET_ERROR,
	PC_ONLINE_WRITE_CMD_LEN_ERROR,

	PC_ONLINE_MEMORY_ERROR,
};

#define SET_HID_SEND_PC_ONLINE_CMD(tx_buf,cmd)	tx_buf[0] = 0xA5,\
												tx_buf[1] = 0x5A,\
												tx_buf[2] = 0x20,\
												tx_buf[3] = cmd

bool  FlashParamUsb_Tx(void)
{
	if(hid_tx_flag)
	{
		OTG_DeviceControlSend(hid_tx_buf,256,6);
		hid_tx_flag = FALSE;
		return TRUE;
	}
	return FALSE;
}

void PcOnlineReadWriteAck(uint8_t cmd,uint8_t *buf,uint8_t len,uint8_t error)
{
	uint8_t tx_buf[16];

	SET_HID_SEND_PC_ONLINE_CMD(tx_buf,cmd);

	//len[2B] + offset[4B] + data_len[2B] + data
	tx_buf[4] = 0x00;
	tx_buf[5] = 4+2+1;
	//offset ֱ�ӷ���ԭ����offset
	tx_buf[6] = buf[3];
	tx_buf[7] = buf[4];
	tx_buf[8] = buf[5];
	tx_buf[9] = buf[6];
	//data_len[2B]
	tx_buf[10] = 0x00;
	tx_buf[11] = len;
	//error����[1B]
	tx_buf[12] = error;
	Union_Effect_Send(tx_buf,13);
}

void FlashSn_Rx(uint8_t *buf,uint16_t buf_len)
{

	switch (buf[0])
	{
	case PC_ONLINE_HANDSHAKE:
		if(buf[3] == 0x81)
		{
			uint8_t tx_buf[16];

			SET_HID_SEND_PC_ONLINE_CMD(tx_buf,PC_ONLINE_HANDSHAKE_ACK);

			//"OK"  + CHIPID +  Vxx.xx.xx + Э��汾
			tx_buf[4] = 0x00;
			tx_buf[5] = 2+1+3+3;
			//OK
			tx_buf[6] = 'O';
			tx_buf[7] = 'K';
			//CHIPID
			tx_buf[8] = CFG_SDK_VER_CHIPID;
			//SDK �汾
			tx_buf[9] = CFG_SDK_MAJOR_VERSION;
			tx_buf[10] = CFG_SDK_MINOR_VERSION;
			tx_buf[11] = CFG_SDK_PATCH_VERSION;
			//Э��汾
			tx_buf[12] = CFG_PC_ONLINE_MAJOR_VERSION;
			tx_buf[13] = CFG_PC_ONLINE_MINOR_VERSION;
			tx_buf[14] = CFG_PC_ONLINE_PATCH_VERSION;

			Union_Effect_Send(tx_buf,15);
		}
		else
		{
			uint8_t tx_buf[16];

			SET_HID_SEND_PC_ONLINE_CMD(tx_buf,PC_ONLINE_HANDSHAKE_ACK);

			//"NG" + error����[1B]
			tx_buf[4] = 0x00;
			tx_buf[5] = 2+1;
			//NG
			tx_buf[6] = 'N';
			tx_buf[7] = 'G';
			//error����[1B]
			tx_buf[8] = 0x01;

			Union_Effect_Send(tx_buf,9);
		}
		break;

	case PC_ONLINE_READ_DATA:
		if(buf[1] == 6)
		{
			uint8_t *p_tx_buf = osPortMalloc(256);
			uint32_t offset = buf[3]<<24 | buf[4]<<16 | buf[5]<<8 | buf[6];
			uint16_t len = buf[7]<<8 | buf[8];

			if(p_tx_buf == NULL)
			{
				PcOnlineReadWriteAck(PC_ONLINE_READ_DATA_ACK,buf,0,PC_ONLINE_MEMORY_ERROR);
				break;
			}

			SET_HID_SEND_PC_ONLINE_CMD(p_tx_buf,PC_ONLINE_READ_DATA_ACK);
			//len[2B] + offset[4B] + data_len[2B] + data
			p_tx_buf[4] = 0x00;
			p_tx_buf[5] = 4+2;
			//offset ֱ�ӷ���ԭ����offset
			p_tx_buf[6] = buf[3];
			p_tx_buf[7] = buf[4];
			p_tx_buf[8] = buf[5];
			p_tx_buf[9] = buf[6];
			if(offset == MAGIC_NUMBER_BT_ADDR || offset == MAGIC_NUMBER_BLE_ADDR)
			{
				p_tx_buf[10] = 0;
				p_tx_buf[11] = BT_ADDR_SIZE;
				if(offset == MAGIC_NUMBER_BT_ADDR)
					memcpy(&p_tx_buf[12],BTDB_CONFIG_ADDR, BT_ADDR_SIZE);
				else
					memcpy(&p_tx_buf[12],BTDB_CONFIG_ADDR + BT_ADDR_SIZE, BT_ADDR_SIZE);
			}
			else if(offset == MAGIC_NUMBER_BT_NAME)
			{
				p_tx_buf[10] = 0;
				p_tx_buf[11] = BT_NAME_SIZE;
				memcpy(&p_tx_buf[12],sys_parameter.BT_Name,BT_NAME_SIZE);
			}
			else if(offset == MAGIC_NUMBER_BLE_NAME)
			{
				p_tx_buf[10] = 0;
				p_tx_buf[11] = BLE_NAME_SIZE;
				memcpy(&p_tx_buf[12],sys_parameter.BT_Name,BLE_NAME_SIZE);
			}
			else
			{
				extern char __data_lmastart;
				if(offset < &__data_lmastart || len > (256-12))
				{
					osPortFree(p_tx_buf);
					PcOnlineReadWriteAck(PC_ONLINE_READ_DATA_ACK,buf,0,PC_ONLINE_READ_OFFSET_ERROR);
					break;
				}
				p_tx_buf[10] = 0;
				p_tx_buf[11] = len;
				memcpy(&p_tx_buf[12],offset,len);
			}
			p_tx_buf[5] += p_tx_buf[11];
			Union_Effect_Send(p_tx_buf,p_tx_buf[5] + 6);
			osPortFree(p_tx_buf);
		}
		else
		{
			PcOnlineReadWriteAck(PC_ONLINE_READ_DATA_ACK,buf,0,PC_ONLINE_READ_CMD_LEN_ERROR);
		}
		break;

	case PC_ONLINE_WRITE:
		{
			uint32_t offset = buf[3]<<24 | buf[4]<<16 | buf[5]<<8 | buf[6];
			uint16_t len = buf[7]<<8 | buf[8];
//			DBG("offset: %x  len: %d\n", offset, len);
			if(offset == MAGIC_NUMBER_BT_ADDR || offset == MAGIC_NUMBER_BLE_ADDR)
			{
				DBG("PC: %x %x %x %x %x %x\n", buf[9],buf[10],buf[11],buf[12],buf[13],buf[14]);
				if(len != BT_ADDR_SIZE)
				{
					PcOnlineReadWriteAck(PC_ONLINE_WRITE_ACK,buf,len,PC_ONLINE_WRITE_CMD_LEN_ERROR);
				}
				else
				{
					uint32_t addr = BTDB_CONFIG_ADDR;
					uint8_t *p_buf = osPortMalloc(4096);
					if(p_buf == NULL)
					{
						PcOnlineReadWriteAck(PC_ONLINE_WRITE_ACK,buf,0,PC_ONLINE_MEMORY_ERROR);
						break;
					}
					SpiFlashRead(addr,p_buf,4096,10);
					SpiFlashErase(SECTOR_ERASE, (addr/4096), 1);
					if(offset == MAGIC_NUMBER_BLE_ADDR)
						memcpy(&p_buf[0+BT_ADDR_SIZE],&buf[9], BT_ADDR_SIZE);
					else
						memcpy(&p_buf[0],&buf[9], BT_ADDR_SIZE);
					SpiFlashWrite(addr, p_buf, 4096, 1);
					PcOnlineReadWriteAck(PC_ONLINE_WRITE_ACK,buf,len,PC_ONLINE_READ_WRITE_SUCCESS);
					osPortFree(p_buf);
				}
			}
			else if(offset == MAGIC_NUMBER_BT_NAME || offset == MAGIC_NUMBER_BLE_NAME)
			{
				uint32_t addr = get_sys_parameter_addr();
				DBG("PC: %s\n", &buf[9]);
				if(addr == 0)
				{
					PcOnlineReadWriteAck(PC_ONLINE_WRITE_ACK,buf,0,PC_ONLINE_WRITE_FLASH_TABLE_ERROR);
					break;
				}
				if(offset == MAGIC_NUMBER_BT_NAME)
				{
					if(len > BT_NAME_SIZE)
						len = BT_NAME_SIZE;
					memset(sys_parameter.BT_Name,0,BT_NAME_SIZE);
					memcpy(sys_parameter.BT_Name,&buf[9],len);
				}
				else
				{
					if(len > BLE_NAME_SIZE)
						len = BLE_NAME_SIZE;
					if(len == BLE_NAME_SIZE)
						len--;

					memset(sys_parameter.BT_Name,0,BLE_NAME_SIZE);
					memcpy(sys_parameter.BT_Name,&buf[9],len);
				}

				SpiFlashErase(SECTOR_ERASE, (addr/4096), 1);
				SpiFlashWrite(addr, (uint8_t*)&sys_parameter, sizeof(SYS_PARAMETER), 1);
				PcOnlineReadWriteAck(PC_ONLINE_WRITE_ACK,buf,len,PC_ONLINE_READ_WRITE_SUCCESS);
			}
			else
			{
				extern char __data_lmastart;
				uint8_t *p_buf;

				//offset����codeλ��
				if(offset < &__data_lmastart || len > (256-12))
				{
					PcOnlineReadWriteAck(PC_ONLINE_READ_DATA_ACK,buf,0,PC_ONLINE_WRITE_OFFSET_ERROR);
					break;
				}
				if((offset/4096) != ((offset+len)/4096)) //����flashͬһ��BLOCK����Ҫ��2�β������ݲ�֧��
				{
					PcOnlineReadWriteAck(PC_ONLINE_READ_DATA_ACK,buf,0,PC_ONLINE_WRITE_CMD_LEN_ERROR);
					break;
				}
				p_buf = osPortMalloc(4096);
				if(p_buf == NULL)
				{
					PcOnlineReadWriteAck(PC_ONLINE_WRITE_ACK,buf,0,PC_ONLINE_MEMORY_ERROR);
					break;
				}
				SpiFlashRead((offset/4096)*4096,p_buf,4096,10); //4K�����ȡ����
				SpiFlashErase(SECTOR_ERASE, (offset/4096), 1);	//����BLOCK
				//д��offset�����4K����λ�õ�ƫ��
				memcpy(p_buf + (offset - ((offset/4096)*4096)),&buf[9], len);

				SpiFlashWrite((offset/4096)*4096, p_buf, 4096, 1);
				PcOnlineReadWriteAck(PC_ONLINE_WRITE_ACK,buf,len,PC_ONLINE_READ_WRITE_SUCCESS);
				osPortFree(p_buf);
			}
		}
		break;

	default:
		break;
	}
}

#endif


