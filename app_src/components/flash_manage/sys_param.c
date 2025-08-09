#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "type.h"
#include "sys_param.h"
#include "spi_flash.h"
#include "flash_table.h"

#include "app_config.h"
#include "debug.h"
#include "app_message.h"
#include "key.h"
#include "ctrlvars.h"

#include "chip_info.h"


DATA_DEM 		data_dem;
SYS_PARAMETER 	sys_parameter;
SYS_PARAMETER 	sys_parameter_dem;



void SYS_PARAMETER_SetDefault(void)
{
	uint32_t addr = get_sys_parameter_addr();
	APP_DBG("[sys_parameter_init]   sys_parameter null > Flash!\n");

	memset(&sys_parameter, 0, sizeof(sys_parameter));
	sys_parameter.byte_check = 0x97;

	strcpy(sys_parameter.Board_name, "ANT-DSP");
	sys_parameter.Activate = 0;

	memcpy(sys_parameter.FW_Version,  (uint8_t[])	{0, 5, 2},	3);
	memcpy(sys_parameter.HID_Version, (uint8_t[])	{3, 0, 0},	3);

	sys_parameter.DAC0_EN = 1;
	sys_parameter.DACX_EN = 1;
	sys_parameter.I2S0_EN = 0;
	sys_parameter.I2S1_EN = 0;

	sys_parameter.Volume_tong 		= VOLUME_DEFAULT;         	
	sys_parameter.Volume_thong_bao 	= REMIND_VOLUME;     		
	sys_parameter.Nho_volume 		= 0;           				
	sys_parameter.Dongbo_volume 	= 0;        				

	strcpy(sys_parameter.BT_Name,  "NVM-Audio"); 				
	sys_parameter.Profile_BT 				= 2;              	
	sys_parameter.A2dp_AAC 					= 1;                
	sys_parameter.Bt_BackgroundType 		= 0;       			
	sys_parameter.Bt_SimplePairingEnable 	= 1;  				
	strcpy(sys_parameter.Bt_PinCode, 	"0000"); 				

	sys_parameter.Thong_bao_en = 1; 							
	memcpy(sys_parameter.Item_thong_bao, (uint8_t[]){1, 1, 1, 1, 1, 1, 1, 1},	8);

	sys_parameter.Key_en = 1; 									
	memcpy(sys_parameter.Key_matrix, ADKEY_TAB, sizeof(ADKEY_TAB));

	sys_parameter.Vr_en 				= ADC_VR_EN;          	
	sys_parameter.Vr_dac_line 			= 0;    				
	sys_parameter.Buoc_eq 				= 3;        			
	sys_parameter.Heso 					= 8;          			

	SpiFlashErase(SECTOR_ERASE, (addr / 4096), 1);
	SPI_FLASH_ERR_CODE ret = SpiFlashWrite(addr, (uint8_t *)&sys_parameter, sizeof(SYS_PARAMETER), 1);

	if (ret == FLASH_NONE_ERR)
	{
		APP_DBG("[sys_parameter_init]   Flash sys_parameter Done  !\n");
	}
}



void sys_parameter_init(void)
{
	uint32_t addr;

	addr = get_data_dem_addr();
	if (addr > 0)
	{
		SpiFlashRead(addr, (uint8_t *)&data_dem, sizeof(data_dem), 10);
		if (data_dem.byte_check != 0x97)
		{
			memset(&data_dem, 0, sizeof(data_dem));
			APP_DBG("[data_dem_init]   data_dem null > Flash!\n");
			data_dem.byte_check = 0x97;
			data_dem.mode = 2;
			data_dem.volume = 10;
			data_dem.remind_error = 0;
			SpiFlashErase(SECTOR_ERASE, (addr / 4096), 1);
			SpiFlashWrite(addr, (uint8_t *)&data_dem, sizeof(data_dem), 1);
		}
	}
	else
	{
		APP_DBG("[data_dem] addr error !\n");
		while (1);
	}

	addr = get_sys_parameter_addr();
	if (addr > 0)
	{
		SpiFlashRead(addr, (uint8_t *)&sys_parameter, sizeof(sys_parameter), 10);
	}
	else
	{
		APP_DBG("[sys_parameter] addr error !\n");
		while (1);
	}

	if (sys_parameter.byte_check != 0x97)
	{
		SYS_PARAMETER_SetDefault();
	}
	else
	{
		memcpy(ADKEY_TAB, sys_parameter.Key_matrix, sizeof(ADKEY_TAB));
		APP_DBG("[sys_parameter_init]   Key init oke!\n");
	}
}



void sys_parameter_Flash(void)
{
	uint32_t addr = get_sys_parameter_addr();
	SpiFlashErase(SECTOR_ERASE, addr / 4096, 1);
	SpiFlashWrite(addr, (uint8_t *)&sys_parameter, sizeof(SYS_PARAMETER), 1);
}
