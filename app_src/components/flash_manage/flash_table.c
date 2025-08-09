#include <stdio.h>
#include <string.h>
#include "type.h"
#include "app_config.h"
#include "flash_table.h"
#include "spi_flash.h"
#include "flash_interface.h"
#include "debug.h"


#define MAX_PACKAGE_NAME_LEN		16

typedef struct _FlashTablePackageHeader
{
	uint8_t header[4];
	uint32_t len;
	uint32_t num;
	uint16_t crc;
}FlashTablePackageHeader;


#define ONE_PAGEAGE_LEN				32	
typedef struct _FlashTablePackage
{
	uint8_t id;
	uint8_t enable_flag;
	uint8_t name[MAX_PACKAGE_NAME_LEN];
	uint8_t offset[4];
	uint8_t check_sum;
	uint8_t	reserve[9];
} FlashTablePackage;


#define REMIND_PACKAGE_NAME				"remind"
#define USER_CONFIG_PACKAGE_NAME		"user_config"
#define BT_CONFIG_PACKAGE_NAME			"bt_config"
#define BT_DATA_PACKAGE_NAME			"bt_data"
#define EFFECT_DATA_PACKAGE_NAME		"effect_data"
#define BP_DATA_PACKAGE_NAME			"bp_data"
#define SYS_PARAMETER_PACKAGE_NAME		"sys_parameter"
#define DATA_DEM_PACKAGE_NAME			"data_dem"


struct
{
	uint32_t 	remind_addr;
	uint32_t 	effect_data_addr;
	uint32_t	bp_data_addr;
	uint32_t	bt_data_addr;
	uint32_t	user_config_addr;
	uint32_t	bt_config_addr;
	uint32_t	sys_parameter_addr;
	uint32_t	data_dem_addr;

	uint32_t	flash_capacity;
	bool		flash_table_valid;
}flash_table_info;

uint32_t flash_table_read(char * name)
{
	uint8_t len;
	uint32_t i;
	uint32_t addr;
	FlashTablePackage package;

	len = strlen(name);
	if(len > MAX_PACKAGE_NAME_LEN)
		len = MAX_PACKAGE_NAME_LEN;

	for(i=0;i<(FLASH_TABLE_SIZE/ONE_PAGEAGE_LEN);i++)
	{
		addr = flash_table_info.flash_capacity - FLASH_TABLE_OFFSET + (i * ONE_PAGEAGE_LEN);
		if(SpiFlashRead(addr,(uint8_t*)&package,ONE_PAGEAGE_LEN,10) != FLASH_NONE_ERR)
		{
			return 0;
		}
		if(memcmp(package.name,name,len) == 0)
		{
			int32_t addr;
			uint8_t check = 0;
			uint8_t *p = (uint8_t*)&package;

			addr = package.offset[3];
			addr <<= 8;
			addr |= package.offset[2];
			addr <<= 8;
			addr |= package.offset[1];
			addr <<= 8;
			addr |= package.offset[0];
#if 0
			addr += flash_table_info.flash_capacity;
#endif
			for(i=0;i<22;i++)
			{
				check +=  p[i];
			}
			//APP_DBG("\nflash_table_read = %x %x  %lx %s\n",check,package.check_sum,addr,package.name);
			if(addr < 0 || check != package.check_sum)
				return  0;
			else
				return addr;
		}
	}
	return 0;
}

void flash_table_check(void)
{
	uint32_t addr;
	FlashTablePackage package;

	flash_table_info.flash_table_valid = 0;

	addr = flash_table_info.flash_capacity - FLASH_TABLE_OFFSET;
	if(SpiFlashRead(addr,(uint8_t*)&package,ONE_PAGEAGE_LEN,10) != FLASH_NONE_ERR)
	{
		return;
	}	

	{
		uint8_t check = 0;
		uint8_t *p = (uint8_t*)&package;
		uint32_t i;

		for(i=0;i<22;i++)
		{
			check +=  p[i];
		}
		if(check != package.check_sum)
			return ;
	}	

	addr = flash_table_info.flash_capacity - FLASH_TABLE_OFFSET + FLASH_TABLE_SIZE - ONE_PAGEAGE_LEN;
	if(SpiFlashRead(addr,(uint8_t*)&package,ONE_PAGEAGE_LEN,10) != FLASH_NONE_ERR)
	{
		return;
	}	

	uint8_t *p = (uint8_t*)&package;
	if(memcmp(p+2,"do not use this",15) == 0)
	{
		flash_table_info.flash_table_valid = 1;
	}
}

bool flash_table_is_valid(void)
{
	return flash_table_info.flash_table_valid;
}

void flash_table_init(void)
{
	SPI_FLASH_INFO *ret = SpiFlashInfoGet();
	uint8_t			buf[2];
	uint8_t			valid;

	memset(&flash_table_info,0,sizeof(flash_table_info));
	valid = 0;
	
	if(SpiFlashRead(FLASH_CAPACTITY_ADDR,buf,1,10) == FLASH_NONE_ERR)
	{
		switch(buf[0])
		{
			case 4:
			case 8:
			case 16:
			case 32:
			case 64:
			case 128:
				flash_table_info.flash_capacity = (1024 * 1024 * buf[0])/8;
				valid = 1;
				break;
			default:
				flash_table_info.flash_capacity = ret->Capacity;
				break;
		}
	}
	else
		flash_table_info.flash_capacity = ret->Capacity;

	if( valid == 0 &&
		flash_table_info.flash_capacity != 0x100000 &&
		flash_table_info.flash_capacity != 0x200000 &&
		flash_table_info.flash_capacity != 0x300000 &&
		flash_table_info.flash_capacity != 0x400000
	  )
	{
		flash_table_info.flash_capacity = FLASH_DEFAULT_CAPACTITY;
	}

	flash_table_check();
	flash_table_info.remind_addr 		= flash_table_read(REMIND_PACKAGE_NAME);
	flash_table_info.effect_data_addr 	= flash_table_read(EFFECT_DATA_PACKAGE_NAME);//Sam add
	flash_table_info.bp_data_addr 		= flash_table_read(BP_DATA_PACKAGE_NAME);
	flash_table_info.bt_data_addr 		= flash_table_read(BT_DATA_PACKAGE_NAME);
	flash_table_info.user_config_addr 	= flash_table_read(USER_CONFIG_PACKAGE_NAME);
	flash_table_info.bt_config_addr 	= flash_table_read(BT_CONFIG_PACKAGE_NAME);
	flash_table_info.sys_parameter_addr	= flash_table_read(SYS_PARAMETER_PACKAGE_NAME);
	flash_table_info.data_dem_addr		= flash_table_read(DATA_DEM_PACKAGE_NAME);

	if(flash_table_info.remind_addr == 0)
		flash_table_info.remind_addr = flash_table_info.flash_capacity - REMIND_DATA_OFFSET;
	if(flash_table_info.effect_data_addr == 0)
		flash_table_info.effect_data_addr = flash_table_info.flash_capacity - EFFECT_DATA_OFFSET;
	if(flash_table_info.bp_data_addr == 0)
		flash_table_info.bp_data_addr = flash_table_info.flash_capacity - BP_DATA_OFFSET;
	if(flash_table_info.bt_data_addr == 0)
		flash_table_info.bt_data_addr = flash_table_info.flash_capacity - BT_DATA_OFFSET;
	if(flash_table_info.sys_parameter_addr == 0)
		flash_table_info.sys_parameter_addr = flash_table_info.flash_capacity - SYS_PARA_OFFSET;
#if 1
	if(flash_table_info.user_config_addr == 0)
		flash_table_info.user_config_addr = flash_table_info.flash_capacity - USER_CONFIG_OFFSET;
	if(flash_table_info.bt_config_addr == 0)
		flash_table_info.bt_config_addr = flash_table_info.flash_capacity - BT_CONFIG_OFFSET;
#else
	if(flash_table_info.user_config_addr == 0)
		flash_table_info.user_config_addr = ret->Capacity - USER_CONFIG_OFFSET;
	if(flash_table_info.bt_config_addr == 0)
		flash_table_info.bt_config_addr = ret->Capacity - BT_CONFIG_OFFSET;
#endif

	APP_DBG("read flash Capacity = 0x%lx 0x%lx\n",ret->Capacity,flash_table_info.flash_capacity);
	APP_DBG("Data Size = 0x%lx,FlashTable: %d\n",SDK_DATA_SIZE,flash_table_info.flash_table_valid);
	APP_DBG("remind_addr = 0x%lx\n",flash_table_info.remind_addr);
	APP_DBG("effect_addr = 0x%lx\n", flash_table_info.effect_data_addr);
	APP_DBG("bp_data_addr = 0x%lx\n",flash_table_info.bp_data_addr);
	APP_DBG("bt_data_addr = 0x%lx\n",flash_table_info.bt_data_addr);
	APP_DBG("sys_parameter_addr = 0x%lx\n",flash_table_info.sys_parameter_addr);
	APP_DBG("user_config_addr = 0x%lx\n",flash_table_info.user_config_addr);
	APP_DBG("bt_config_addr = 0x%lx\n",flash_table_info.bt_config_addr);
	APP_DBG("data_dem_addr = 0x%lx\n",flash_table_info.data_dem_addr);
}

uint32_t get_remind_addr(void)
{
	return flash_table_info.remind_addr;
}

uint32_t get_effect_data_addr(void)
{
	return flash_table_info.effect_data_addr;
}

uint32_t get_bp_data_addr(void)
{
	return flash_table_info.bp_data_addr;
}

uint32_t get_bt_data_addr(void)
{
	return flash_table_info.bt_data_addr;
}

uint32_t get_user_config_addr(void)
{
	return flash_table_info.user_config_addr;
}

uint32_t get_bt_config_addr(void)
{
	return flash_table_info.bt_config_addr;
}

uint32_t get_sys_parameter_addr(void)
{
	return flash_table_info.sys_parameter_addr;
}

uint32_t get_data_dem_addr(void)
{
	return flash_table_info.data_dem_addr;
}
