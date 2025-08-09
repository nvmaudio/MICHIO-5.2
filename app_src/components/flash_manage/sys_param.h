#ifndef _SYS_PARAM_H__
#define _SYS_PARAM_H__

#include "sys_param_typedef.h"


#define BT_NAME						"BP10_BT"
#define BLE_NAME						"BP10_BT_Ble"
#define BT_TRIM_ECO0				0x16 
#define BT_TRIM						0x14 


#define BT_TX_POWER_LEVEL			22
#define BT_PAGE_TX_POWER_LEVEL		16 


enum
{
	USE_NULL_RING = 0,
	USE_NUMBER_REMIND_RING = 1,
	USE_LOCAL_AND_PHONE_RING = 2,
	USE_ONLY_LOCAL_RING = 3,
};


enum
{
	BT_BACKGROUND_FAST_POWER_ON_OFF = 0,
	BT_BACKGROUND_POWER_ON = 1,
	BT_BACKGROUND_DISABLE = 2,
};

#define BT_NAME_SIZE				sizeof(((SYS_PARAMETER *)0)->BT_Name)
#define BLE_NAME_SIZE				sizeof(((SYS_PARAMETER *)0)->BT_Name)

extern SYS_PARAMETER sys_parameter;
extern DATA_DEM data_dem;

extern void sys_parameter_init(void);
extern void sys_parameter_Flash(void);


#endif


