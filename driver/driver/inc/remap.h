/**
  ******************************************************************************
  * @file    remap.h
  * @author  Peter Zhai
  * @version V1.0
  * @date    2017-10-27
  * @brief
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

/**
 * @addtogroup SYSTEM
 * @{
 * @defgroup remap remap.h
 * @{
 */

#ifndef __REMAP_H__
#define __REMAP_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus 

#include"debug.h"

#define REMAP_DBG(format, ...)		//DBG(format, ##__VA_ARGS__)

#define FLASH_ADDR 			0x0
#define TCM_SIZE   			12
#define SRAM_END_ADDR		0x20048000
#define TCM_SRAM_START_ADDR	0x20000000

extern uint32_t gSramEndAddr;
typedef enum
{
	ADDR_REMAP0=0,
	ADDR_REMAP1=1,
	ADDR_REMAP2
} ADDR_REMAP_ID;

typedef enum
{
	REMAP_UNALIGNED_ADDRESS = -1,
	REMAP_OK = 0
}REMAP_ERROR;

/**
 * @brief	ʹ��remap����
 * @param	num	 ��ADDR_REMAP1��ADDR_REMAP2��ָ��һ������
 * @param	src	 Դ��ַ������4KB����
 * @param	dst	 Ŀ�ĵ�ַ������4KB����
 * @param	size ��λ��KB������4KB����
 * @return	�����룬�����REMAP_ERROR
 * @note
 */
REMAP_ERROR Remap_AddrRemapSet(ADDR_REMAP_ID num, uint32_t src, uint32_t dst, uint32_t size);

/**
 * @brief	�ر�remap����
 * @param	num	 ��ADDR_REMAP1��ADDR_REMAP2��ָ��һ������
 * @return	�����룬�����REMAP_ERROR
 * @note
 */
void Remap_AddrRemapDisable(ADDR_REMAP_ID num);

/**
 * @brief	ʹ��TCM���ܣ���Ҫ�ǽ�flash����ӳ�䵽memcpy��
 * @param	StartAddr	Դ��ַ������4KB����
 * @param	size 		��λ��KB������4KB����
 * @return	�����룬�����REMAP_ERROR
 * @note
 */
REMAP_ERROR Remap_InitTcm(uint32_t StartAddr, uint32_t size);

/**
 * @brief	����TCM����
 * @param	None
 * @return	None
 * @note
 */
void Remap_DisableTcm(void);

#ifdef __cplusplus
}
#endif // __cplusplus 

#endif //__REMAP_H__
/**
 * @}
 * @}
 */
