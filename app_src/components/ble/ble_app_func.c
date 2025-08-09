
#include "type.h"
#include <string.h>
#include "ble_api.h"
#include "ble_app_func.h"
//#include "bt_app_func.h"
#include "bt_manager.h"
#include "app_config.h"

extern BT_CONFIGURATION_PARAMS		*btStackConfigParams;


#include "debug.h"
#if (BLE_SUPPORT == ENABLE)
//#ifndef BLE_NAME_TOFLASH
#if (defined(BT_TWS_SUPPORT) && (TWS_PAIRING_MODE != CFG_TWS_ROLE_RANDOM) && (TWS_PAIRING_MODE != CFG_TWS_PEER_MASTER) && (TWS_PAIRING_MODE != CFG_TWS_PEER_SLAVE))
extern int32_t BtDeviceBleNameSet(uint8_t* deviceName, uint8_t deviceLen);

//const uint8_t advertisement_data[] = {
//	0x02, 0x01, 0x02,		//flag:LE General Discoverable
//	0x03, 0x03, 0x00, 0xab,	//16bit service UUIDs
//};
const uint8_t advertisement_data[] = {
	0x02, 0x01, 0x1A,		//flag:LE General Discoverable
	16,	 0xff, 0xd9, 0x06, //flag + mac 6Bytes + mac 6bytes
};

#else

const uint8_t advertisement_data[] = {
	0x02, 0x01, 0x02,		//flag:LE General Discoverable
	0x03, 0x03, 0x00, 0xab,	//16bit service UUIDs
//	9,   0x09, 'B', 'P', '1', '0', '-','B','L','E',//
};

#endif
uint8_t gAdvertisementData[50]; 
uint8_t gResponseData[50];
	
const uint8_t profile_data[] =
{
	// 0x0001 PRIMARY_SERVICE-GAP_SERVICE
	0x0a, 0x00, 0x02, 0xf0, 0x01, 0x00, 0x00, 0x28, 0x00, 0x18,
	// 0x0002 CHARACTERISTIC-GAP_DEVICE_NAME-READ | WRITE | DYNAMIC
	0x0d, 0x00, 0x02, 0xf0, 0x02, 0x00, 0x03, 0x28, 0x0a, 0x03, 0x00, 0x00, 0x2a,
	// 0x0003 VALUE-GAP_DEVICE_NAME-READ | WRITE | DYNAMIC-'BP10-BLE'
	// READ_ANYBODY, WRITE_ANYBODY
	0x10, 0x00, 0x0a, 0xf1, 0x03, 0x00, 0x00, 0x2a, 0x42, 0x50, 0x31, 0x30, 0x2d, 0x42, 0x4c, 0x45,

	// 0x0004 PRIMARY_SERVICE-AB00
	0x0a, 0x00, 0x02, 0xf0, 0x04, 0x00, 0x00, 0x28, 0x00, 0xab,
	// 0x0005 CHARACTERISTIC-AB01-READ | WRITE | WRITE_WITHOUT_RESPONSE | DYNAMIC
	0x0d, 0x00, 0x02, 0xf0, 0x05, 0x00, 0x03, 0x28, 0x0e, 0x06, 0x00, 0x01, 0xab,
	// 0x0006 VALUE-AB01-READ | WRITE | WRITE_WITHOUT_RESPONSE | DYNAMIC-''
	// READ_ANYBODY, WRITE_ANYBODY
	0x08, 0x00, 0x0e, 0xf1, 0x06, 0x00, 0x01, 0xab,
	// 0x0007 CHARACTERISTIC-AB02-NOTIFY | DYNAMIC
	0x0d, 0x00, 0x02, 0xf0, 0x07, 0x00, 0x03, 0x28, 0x10, 0x08, 0x00, 0x02, 0xab,
	// 0x0008 VALUE-AB02-NOTIFY | DYNAMIC-''
	//
	0x08, 0x00, 0x00, 0xf1, 0x08, 0x00, 0x02, 0xab,
	// 0x0009 CLIENT_CHARACTERISTIC_CONFIGURATION
	// READ_ANYBODY, WRITE_ANYBODY
	0x0a, 0x00, 0x0e, 0xf1, 0x09, 0x00, 0x02, 0x29, 0x00, 0x00,
	// 0x000a CHARACTERISTIC-AB03-NOTIFY | DYNAMIC
	0x0d, 0x00, 0x02, 0xf0, 0x0a, 0x00, 0x03, 0x28, 0x10, 0x0b, 0x00, 0x03, 0xab,
	// 0x000b VALUE-AB03-NOTIFY | DYNAMIC-''
	//
	0x08, 0x00, 0x00, 0xf1, 0x0b, 0x00, 0x03, 0xab,
	// 0x000c CLIENT_CHARACTERISTIC_CONFIGURATION
	// READ_ANYBODY, WRITE_ANYBODY
	0x0a, 0x00, 0x0e, 0xf1, 0x0c, 0x00, 0x02, 0x29, 0x00, 0x00,

	// END
	0x00, 0x00,
}; // total size 74 bytes


//
// list service handle ranges
//
#define ATT_SERVICE_GAP_SERVICE_START_HANDLE 		0x0001
#define ATT_SERVICE_GAP_SERVICE_END_HANDLE 			0x0003
#define ATT_SERVICE_AF00_START_HANDLE		 		0x0004
#define ATT_SERVICE_AF00_END_HANDLE 				0x0013

//
// list mapping between characteristics and handles
//
#define ATT_CHARACTERISTIC_GAP_DEVICE_NAME_01_VALUE_HANDLE 			0x0003


#define ATT_CHARACTERISTIC_AF01_01_VALUE_HANDLE 					0x0006
#define ATT_CHARACTERISTIC_AF02_01_VALUE_HANDLE 					0x0008
#define ATT_CHARACTERISTIC_AF02_01_CLIENT_CONFIGURATION_HANDLE 		0x0009
#define ATT_CHARACTERISTIC_AF03_01_VALUE_HANDLE 					0x000b
#define ATT_CHARACTERISTIC_AF04_01_VALUE_HANDLE 					0x000d
#define ATT_CHARACTERISTIC_AF04_01_CLIENT_CONFIGURATION_HANDLE 		0x000e
#define ATT_CHARACTERISTIC_AF05_01_VALUE_HANDLE 					0x0010
#define ATT_CHARACTERISTIC_AF06_01_VALUE_HANDLE 					0x0012
#define ATT_CHARACTERISTIC_AF06_01_CLIENT_CONFIGURATION_HANDLE 		0x0013

//
// list service handle ranges
//
//#define ATT_SERVICE_GAP_SERVICE_START_HANDLE 0x0001
//#define ATT_SERVICE_GAP_SERVICE_END_HANDLE 0x0003
#define ATT_SERVICE_AB00_START_HANDLE 0x0004
#define ATT_SERVICE_AB00_END_HANDLE 0x000e

//
// list mapping between characteristics and handles
//
#define ATT_CHARACTERISTIC_GAP_DEVICE_NAME_01_VALUE_HANDLE 0x0003
#define ATT_CHARACTERISTIC_AB01_01_VALUE_HANDLE 0x0006
#define ATT_CHARACTERISTIC_AB02_01_VALUE_HANDLE 0x0008
#define ATT_CHARACTERISTIC_AB02_01_CLIENT_CONFIGURATION_HANDLE 0x0009
#define ATT_CHARACTERISTIC_AB03_01_VALUE_HANDLE 0x000b
#define ATT_CHARACTERISTIC_AB03_01_CLIENT_CONFIGURATION_HANDLE 0x000c
#define ATT_CHARACTERISTIC_AB04_01_VALUE_HANDLE 0x000e


BLE_APP_CONTEXT			g_playcontrol_app_context;
GATT_SERVER_PROFILE		g_playcontrol_profile;
GAP_MODE				g_gap_mode;

int16_t app_att_read(uint16_t con_handle, uint16_t attribute_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size);
int16_t app_att_write(uint16_t con_handle, uint16_t attribute_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
int16_t att_read(uint16_t con_handle, uint16_t attribute_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size);
int16_t att_write(uint16_t con_handle, uint16_t attribute_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
int16_t gap_att_write(uint16_t con_handle, uint16_t attribute_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);

int8_t InitBlePlaycontrolProfile(void)
{
	uint8_t adv_len = 0;
	uint8_t offset = 0;
	//register ble callback funciton
	BleAppCallBackRegister(BLEStackCallBackFunc);

	memcpy(g_playcontrol_app_context.ble_device_addr, btStackConfigParams->ble_LocalDeviceAddr, 6);

#ifdef BT_TWS_SUPPORT
#if (TWS_PAIRING_MODE == CFG_TWS_ROLE_SLAVE)
	g_playcontrol_app_context.ble_device_role = CENTRAL_DEVICE;
#else
	g_playcontrol_app_context.ble_device_role = PERIPHERAL_DEVICE;
#endif

#else
	g_playcontrol_app_context.ble_device_role = PERIPHERAL_DEVICE;
#endif

	g_playcontrol_profile.profile_data 	= (uint8_t *)profile_data;//g_profile_data;
	g_playcontrol_profile.attr_read		= att_read;
	g_playcontrol_profile.attr_write	= att_write;

	// set advertising interval params
	SetAdvIntervalParams(0x0020, 0x0100);

	// set gap mode
	g_gap_mode.broadcase_mode		= NON_BROADCAST_MODE;
	g_gap_mode.discoverable_mode	= GENERAL_DISCOVERABLE_MODE;
	g_gap_mode.connectable_mode		= UNDIRECTED_CONNECTABLE_MODE;
	g_gap_mode.bondable_mode		= NON_BONDABLE_MODE;
	SetGapMode(g_gap_mode);
#ifndef BLE_NAME_TOFLASH
	adv_len = sizeof(advertisement_data);
	memcpy(&gAdvertisementData[0], (uint8_t *)advertisement_data, adv_len);
#else
	offset = adv_len = sizeof(advertisement_data);
	memcpy(&gAdvertisementData[0], (uint8_t *)advertisement_data, offset);
	gAdvertisementData[offset] = (strlen(btStackConfigParams->ble_LocalDeviceName)+1);
	gAdvertisementData[offset+1] = 0x09;
	memcpy(&gAdvertisementData[offset+2], btStackConfigParams->ble_LocalDeviceName, strlen(btStackConfigParams->ble_LocalDeviceName));
	adv_len += (2+strlen(btStackConfigParams->ble_LocalDeviceName));
#endif
	SetAdvertisingData((uint8_t *)gAdvertisementData, adv_len);

	return 0;
}

/************************************************************************************
 * �˺����޸�BLE�㲥����:����BLE����
 * ע��:�˺������ڷ�����Э��ջ�����е��ȴ���, ��Ϊ�������е���vTaskDelay��ʱ����
 * ��׿�ֻ���ʹ��APP: nRF Connect ���˶�BLE�㲥���ݵĸ���,����APP���ܴ���BLE�㲥���ݸ��²���ʱ����
 ************************************************************************************/
void BleAdvUpdate(uint8_t *nameStr)
{
	uint8_t adv_len = 0;
	uint8_t offset = 0;
	uint16_t name_len = 0;
	
#ifdef BLE_NAME_TOFLASH
	if(nameStr == NULL)
		return;
	
	name_len = strlen(nameStr);
	if((name_len == 0)||(name_len >= (31-sizeof(advertisement_data))))
		return;

	DisableAdvertising();
	vTaskDelay(20);
	memset(btStackConfigParams->ble_LocalDeviceName, 0, BLE_NAME_SIZE);
	strcpy((void *)btStackConfigParams->ble_LocalDeviceName, nameStr);
	memset(gAdvertisementData, 0, 50);
	offset = adv_len = sizeof(advertisement_data);
	memcpy(&gAdvertisementData[0], (uint8_t *)advertisement_data, offset);
	gAdvertisementData[offset] = (strlen(btStackConfigParams->ble_LocalDeviceName)+1);
	gAdvertisementData[offset+1] = 0x09;
	memcpy(&gAdvertisementData[offset+2], btStackConfigParams->ble_LocalDeviceName, strlen(btStackConfigParams->ble_LocalDeviceName));
	adv_len += (2+strlen(btStackConfigParams->ble_LocalDeviceName));
    SetAdvertisingData((uint8_t *)gAdvertisementData, adv_len);
	EnableAdvertising();
#endif
}


void BleAdvSet(void)
{
	uint8_t adv_len = 0;
	uint8_t offset = 0;

	#ifndef BLE_NAME_TOFLASH
		adv_len = sizeof(advertisement_data);
		memcpy(&gAdvertisementData[0], (uint8_t *)advertisement_data, adv_len);
	#else
		offset = adv_len = sizeof(advertisement_data);
		memcpy(&gAdvertisementData[0], (uint8_t *)advertisement_data, offset);
		gAdvertisementData[offset] = (strlen(btStackConfigParams->ble_LocalDeviceName)+1);
		gAdvertisementData[offset+1] = 0x09;
		memcpy(&gAdvertisementData[offset+2], btStackConfigParams->ble_LocalDeviceName, strlen(btStackConfigParams->ble_LocalDeviceName));
		adv_len += (2+strlen(btStackConfigParams->ble_LocalDeviceName));
	#endif
    SetAdvertisingData((uint8_t *)gAdvertisementData, adv_len);
}

int8_t UninitBlePlaycontrolProfile(void)
{
	return 0;
}

int16_t att_read(uint16_t con_handle, uint16_t attribute_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size)
{
    if( (attribute_handle >= ATT_SERVICE_GAP_SERVICE_START_HANDLE) && (attribute_handle <= ATT_SERVICE_GAP_SERVICE_END_HANDLE))
	{
    	switch(attribute_handle)
		{
			case ATT_CHARACTERISTIC_GAP_DEVICE_NAME_01_VALUE_HANDLE:
				if(buffer)
				{
					memcpy(buffer, btStackConfigParams->ble_LocalDeviceName, strlen((char*)btStackConfigParams->ble_LocalDeviceName));
					return (int16_t)strlen((char*)btStackConfigParams->ble_LocalDeviceName);
				}
				return 0;
				
	        default:
	            return 0;
		}
	}
    else if( (attribute_handle >= ATT_SERVICE_AB00_START_HANDLE) && (attribute_handle <= ATT_SERVICE_AB00_END_HANDLE))
	{
    	return app_att_read(con_handle, attribute_handle, offset, buffer, buffer_size);
	}
	else
	{
		
	}

    return 0;
}




int16_t att_write(uint16_t con_handle, uint16_t attribute_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    if( (attribute_handle >= ATT_SERVICE_GAP_SERVICE_START_HANDLE) && (attribute_handle <= ATT_SERVICE_GAP_SERVICE_END_HANDLE))
	{
    	//BT_DBG("att_write attribute_handle:%u\n",attribute_handle);
    	switch(attribute_handle)
    	{
			case ATT_CHARACTERISTIC_GAP_DEVICE_NAME_01_VALUE_HANDLE:
				if((buffer)&&(buffer_size))
				{
					//BT_DBG("name: %s\n", buffer);

					if(buffer_size > BT_NAME_SIZE)
						buffer_size = BT_NAME_SIZE;

					#ifdef BLE_NAME_TOFLASH
					BtDeviceBleNameSet(buffer, buffer_size);

					{
						uint8_t offset = 0;
						uint8_t adv_len = 0;
						offset = adv_len = sizeof(advertisement_data);

						offset = adv_len = sizeof(advertisement_data);
						memcpy(&gAdvertisementData[0], (uint8_t *)advertisement_data, offset);
						gAdvertisementData[offset] = (strlen(btStackConfigParams->ble_LocalDeviceName)+1);
						gAdvertisementData[offset+1] = 0x09;
						memcpy(&gAdvertisementData[offset+2], btStackConfigParams->ble_LocalDeviceName, strlen(btStackConfigParams->ble_LocalDeviceName));
						adv_len += (2+strlen(btStackConfigParams->ble_LocalDeviceName));
						
						SetAdvertisingData((uint8_t *)gAdvertisementData, adv_len);
					}
					#endif
				}
				return 0;
				
			default:
				return 0;
    	}
	}
    else if( (attribute_handle >= ATT_SERVICE_AB00_START_HANDLE) && (attribute_handle <= ATT_SERVICE_AB00_END_HANDLE))
	{
    	return app_att_write(con_handle, attribute_handle, transaction_mode, offset, buffer, buffer_size);
	}
	else
	{
		//δ֪���
	}
    return 0;
}



int16_t app_att_read(uint16_t con_handle, uint16_t attribute_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size)
{
//	BT_DBG("app_att_read for handle %02x,%d,%d,0x%08lx\n", attribute_handle,offset,buffer_size,buffer);
	switch(attribute_handle)
	{
		case ATT_CHARACTERISTIC_AB01_01_VALUE_HANDLE:
			//BT_DBG("ATT_CHARACTERISTIC_AB01_01_VALUE_HANDLE:\n");
			if(buffer == 0)//���´���Ŀ�����ݵĳ���
			{

			}
			else
			{

			}
			break;

		case ATT_CHARACTERISTIC_AB02_01_VALUE_HANDLE:
			//BT_DBG("ATT_CHARACTERISTIC_AB02_01_VALUE_HANDLE:\n");
			if(buffer == 0)//���´���Ŀ�����ݵĳ���
			{

			}
			else
			{

			}
			break;

		case ATT_CHARACTERISTIC_AB03_01_VALUE_HANDLE:
			//BT_DBG("ATT_CHARACTERISTIC_AB03_01_VALUE_HANDLE:\n");
			if(buffer == 0)//���´���Ŀ�����ݵĳ���
			{

			}
			else
			{

			}
			break;

		case ATT_CHARACTERISTIC_AF06_01_CLIENT_CONFIGURATION_HANDLE:
			if(buffer == 0)//���´���Ŀ�����ݵĳ���
			{

			}
			else
			{
				return 2;
			}
			break;

		default:
			return 0;
	}
	return 0;
}

int16_t app_att_write(uint16_t con_handle, uint16_t attribute_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
//    BT_DBG("app_att_write for handle %02x\n", attribute_handle);
	switch(attribute_handle)
	{
		case ATT_CHARACTERISTIC_AB01_01_VALUE_HANDLE:
			BT_DBG("ATT_CHARACTERISTIC_AB01_01_VALUE_HANDLE:\n");
			BT_DBG("write len:%d\n", buffer_size);
			break;

		case ATT_CHARACTERISTIC_AB02_01_VALUE_HANDLE:
			//BT_DBG("ATT_CHARACTERISTIC_AB02_01_VALUE_HANDLE:\n");
			break;

		case ATT_CHARACTERISTIC_AB03_01_VALUE_HANDLE:
			//BT_DBG("ATT_CHARACTERISTIC_AB03_01_VALUE_HANDLE:\n");
			break;
		case ATT_CHARACTERISTIC_AF05_01_VALUE_HANDLE:
			break;

		case ATT_CHARACTERISTIC_AF06_01_CLIENT_CONFIGURATION_HANDLE:

			if(buffer_size == 2)
			{
			}

			break;

		default:
			return 0;
	}
	return 0;
}

#if (defined(BT_TWS_SUPPORT)&&(TWS_PAIRING_MODE == CFG_TWS_ROLE_MASTER))
void ble_advertisement_data_update(void)
{
	uint8_t offset = 0;
	uint8_t adv_len = 0;

	if(g_playcontrol_app_context.ble_device_role == CENTRAL_DEVICE)
		return;

	offset = adv_len = sizeof(advertisement_data);
//	memcpy(&gAdvertisementData[0], (uint8_t *)advertisement_data, adv_len);
	gAdvertisementData[0] = 0x02;
	gAdvertisementData[1] = 0x01;
	gAdvertisementData[2] = 0x1A;

	gAdvertisementData[3] = 16; //len
#ifdef TWS_FILTER_USER_DEFINED
	gAdvertisementData[3] += 6; //len
#endif
	gAdvertisementData[4] = 0xff;
	gAdvertisementData[5] = 0xd9;
	gAdvertisementData[6] = 0x06;

	//tws:ble info
	gAdvertisementData[offset] = btManager.twsEnterPairingFlag; //flag:0=normal; 1=pairing
	//local device mac addr
	memcpy(&gAdvertisementData[offset+1], btManager.btDevAddr, 6);
	printf("btDevAddr: %02x:%02x:%02x:%02x:%02x:%02x\n", \
			btManager.btDevAddr[0], \
			btManager.btDevAddr[1], \
			btManager.btDevAddr[2], \
			btManager.btDevAddr[3], \
			btManager.btDevAddr[4], \
			btManager.btDevAddr[5]);
	//tws device mac addr
	memcpy(&gAdvertisementData[offset+7], btManager.btTwsDeviceAddr, 6);
	printf("btTwsDeviceAddr: %02x:%02x:%02x:%02x:%02x:%02x\n", \
			btManager.btTwsDeviceAddr[0], \
			btManager.btTwsDeviceAddr[1], \
			btManager.btTwsDeviceAddr[2], \
			btManager.btTwsDeviceAddr[3], \
			btManager.btTwsDeviceAddr[4], \
			btManager.btTwsDeviceAddr[5]);
	adv_len += 13;

#ifdef TWS_FILTER_USER_DEFINED
	//tws filter infor
	memcpy(&gAdvertisementData[offset+13], btManager.TwsFilterInfor, 6);
	adv_len += 6;
#endif
	DisableAdvertising();
	if(GetBtManager()->twsState == BT_TWS_STATE_NONE)
		SetAdvertisingData((uint8_t *)gAdvertisementData, adv_len);
	EnableAdvertising();
}
#endif

//ble scan params
//default params
#define BLE_SCAN_TYPE_DEFAULT			0x01//0=passive scan;1=active scan;
#define BLE_SCAN_INTERVAL_DEFAULT		0xA0
#define BLE_SCAN_WINDOW_DEFAULT			0x80
#define BLE_OWN_ADDR_TYPE_DEFAULT		0x00//0=public addr type;1=random addr type;
#define BLE_SCAN_FILTER_POLICY_DEFAULT	0x00//0=scan policy accept all;1=scan policy accept whitelist;
//sniff params
#define BLE_SCAN_TYPE_SNIFF				0x00//0=passive scan;1=active scan;
#define BLE_SCAN_INTERVAL_SNIFF			0x640
#define BLE_SCAN_WINDOW_SNIFF			0x80
#define BLE_OWN_ADDR_TYPE_SNIFF			0x00
#define BLE_SCAN_FILTER_POLICY_SNIFF	0x00


void BleScanParamConfig_Default(void)
{
	BleScanParamSet(BLE_SCAN_TYPE_DEFAULT, BLE_SCAN_INTERVAL_DEFAULT, BLE_SCAN_WINDOW_DEFAULT, BLE_OWN_ADDR_TYPE_DEFAULT, BLE_SCAN_FILTER_POLICY_DEFAULT);
}
void BleScanParamConfig_Sniff(void)
{
	BleScanParamSet(BLE_SCAN_TYPE_SNIFF, BLE_SCAN_INTERVAL_SNIFF, BLE_SCAN_WINDOW_SNIFF, BLE_OWN_ADDR_TYPE_SNIFF, BLE_SCAN_FILTER_POLICY_SNIFF);
}

#endif

