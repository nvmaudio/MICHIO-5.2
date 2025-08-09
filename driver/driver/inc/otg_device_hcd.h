/**
 *****************************************************************************
 * @file     otg_device_hcd.h
 * @author   Owen
 * @version  V1.0.0
 * @date     27-03-2017
 * @brief    device module driver interface
 *****************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2013 MVSilicon </center></h2>
 */
 
/**
 * @addtogroup OTG
 * @{
 * @defgroup otg_device_hcd otg_device_hcd.h
 * @{
 */
 
#ifndef __OTG_DEVICE_HCD_H__
#define	__OTG_DEVICE_HCD_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include "type.h"


	
//MV �˵�FIFO��С
/*	
EP0,	64//
EP1,	64//bulk
EP2,	128//bulk
EP3,	256//iso
*/

// Max packet size. Fixed, user should not modify.
#define	DEVICE_FS_CONTROL_MPS		64	
#define	DEVICE_FS_INT_IN_MPS		64	
#define	DEVICE_FS_INT_OUT_MPS		64	
#define	DEVICE_FS_BULK_IN_MPS		64	
#define	DEVICE_FS_BULK_OUT_MPS		64	
#define	DEVICE_FS_ISO_IN_MPS		192//only use by ep4 5	
#define	DEVICE_FS_ISO_OUT_MPS_DRV_RECEIVE		288//only use by ep4 5 bkd 


typedef enum _DEVICE_TRANSFER_TYPE
{
    TYPE_SETUP 			= 0xFF,
    TYPE_CONTROL_IN		= 0xFE,
    TYPE_CONTROL_OUT	= 0xFD,
	TYPE_ISO_IN 		= 0x81,
	TYPE_ISO_OUT 		= 0x01,
	TYPE_BULK_IN 		= 0x82,
	TYPE_BULK_OUT 		= 0x02,
	TYPE_INT_IN 		= 0x83,
	TYPE_INT_OUT 		= 0x03,
} DEVICE_TRANSFER_TYPE;




typedef enum _OTG_DEVICE_ERR_CODE
{
	DEVICE_NONE_ERR = 0,
	DEVICE_UNLINK_ERR,
	
	CONTROL_SEND_ERR,
	CONTROL_RCV_ERR,
	CONTROL_SETUP_ERR,
	
	BULK_SEND_ERR,
	BULK_RCV_ERR0,
	BULK_RCV_ERR1,
	BULK_RCV_ERR2,

	INT_SEND_ERR,
	INT_RCV_ERR0,
	INT_RCV_ERR1,
	INT_RCV_ERR2,
}OTG_DEVICE_ERR_CODE;


/**
 * @brief  Device initial
 * @param  NONE
 * @return NONE
 */
void OTG_DeviceInit(void);


/**
 * @brief  Device software connect.
 * @param  NONE
 * @return NONE
 */
void OTG_DeviceConnect(void);


/**
 * @brief  Device software disconnect.
 * @param  NONE
 * @return NONE
 */
void OTG_DeviceDisConnect(void);


/**
 * @brief  ����STALLӦ��
 * @param  EndpointNum �˵��
 * @return NONE
 */
void OTG_DeviceStallSend(uint8_t EndpointNum);


/**
 * @brief  ��λĳ���˵�
 * @param  EndpointNum �˵��
 * @param  EndpointType �˵�����
 * @return NONE
 */
void OTG_DeviceEndpointReset(uint8_t EndpointNum, uint8_t EndpointType);


/**
 * @brief  �����豸��ַ
 * @brief  ���߸�λ�������豸��ַΪ0����������PC�˷���SetAddress��������ָ�����豸��ַ
 * @param  Address �豸��ַ
 * @return NONE
 */
void OTG_DeviceAddressSet(uint8_t Address);


/**
 * @brief  ��ȡ�����ϵ��¼�
 * @param  NONE
 * @return �����¼����룬USB_RESET-���߸�λ�¼���...
 */
uint8_t OTG_DeviceBusEventGet(void);


/**
 * @brief  �ӿ��ƶ˵㷢������
 * @param  Buf ���ݻ�����ָ��
 * @param  Len ���ݳ���
 * @param  TimeOut��ʱʱ�䣬ms
 * @return OTG_DEVICE_ERR_CODE
 */
OTG_DEVICE_ERR_CODE OTG_DeviceControlSend(uint8_t* Buf, uint32_t Len, uint32_t TimeOut);


/**
 * @brief  �ӿ��ƶ˵����OUT����
 * @param  Buf ���ݻ�����ָ��
 * @param  MaxLen ������ݳ���
 * @param  *pTransferLenʵ�ʽ��յ������ݳ���
 * @param  TimeOut��ʱʱ�䣬ms
 * @return OTG_DEVICE_ERR_CODE
 */
OTG_DEVICE_ERR_CODE OTG_DeviceControlReceive(uint8_t* Buf, uint32_t MaxLen, uint32_t *pTransferLen, uint32_t TimeOut);


/**
 * @brief  Giao diện xử lý gói SETUP từ host USB
 * @param  Buf      Con trỏ đến vùng đệm dữ liệu nhận được
 * @param  MaxLen   Độ dài tối đa của dữ liệu, gói SETUP luôn có độ dài cố định là 8 byte
 * @param  *pTransferLen   Con trỏ đến biến lưu độ dài dữ liệu thực sự đã nhận
 * @return OTG_DEVICE_ERR_CODE  Mã lỗi trả về (nếu có)
 */
OTG_DEVICE_ERR_CODE OTG_DeviceSetupReceive(uint8_t* Buf, uint32_t MaxLen ,uint32_t *pTransferLen);



/**
 * @brief  ��BULK IN�˵㷢������
 * @param  EpNum �˵��
 * @param  Buf ���ݻ�����ָ��
 * @param  Len ���ݳ���
 * @param  TimeOut��ʱʱ�䣬ms
 * @return OTG_DEVICE_ERR_CODE
 */
OTG_DEVICE_ERR_CODE OTG_DeviceBulkSend(uint8_t EpNum, uint8_t* Buf, uint32_t Len, uint32_t TimeOut);


/**
 * @brief  ��BULK OUT�˵��������
 * @param  EpNum �˵��
 * @param  Buf ���ݻ�����ָ��
 * @param  MaxLen ������ݳ���
 * @param  *pTransferLenʵ�ʽ��յ������ݳ���
 * @param  TimeOut��ʱʱ�䣬ms
 * @return OTG_DEVICE_ERR_CODE
 */
OTG_DEVICE_ERR_CODE OTG_DeviceBulkReceive(uint8_t EpNum, uint8_t* Buf, uint16_t MaxLen, uint32_t *pTransferLen ,uint32_t TimeOut);


/**
 * @brief  ��ISO IN�˵㷢������
 * @param  EpNum �˵��
 * @param  Buf ���ݻ�����ָ��
 * @param  Len ���ݳ��ȣ����ܴ���DEVICE_FS_ISO_IN_MPS�ֽ�
 * @return OTG_DEVICE_ERR_CODE
 */
OTG_DEVICE_ERR_CODE OTG_DeviceISOSend(uint8_t EpNum, uint8_t* Buf, uint32_t Len);


/**
 * @brief  ��ISO OUT�˵��������
 * @param  EpNum �˵��
 * @param  Buf ���ݻ�����ָ��
 * @param  MaxLen ������ݳ���
 * @param  *pTransferLenʵ�ʽ��յ������ݳ���
 * @return OTG_DEVICE_ERR_CODE
 */
OTG_DEVICE_ERR_CODE OTG_DeviceISOReceive(uint8_t EpNum, uint8_t* Buf, uint32_t MaxLen ,uint32_t *pTransferLen);


/**
 * @brief  Gửi dữ liệu qua endpoint INT IN
 *         (Interrupt IN endpoint - thiết bị gửi dữ liệu về cho host).
 * 
 * @param  EpNum   Số hiệu endpoint (endpoint number).
 * @param  Buf     Con trỏ đến bộ đệm dữ liệu cần gửi.
 * @param  Len     Độ dài dữ liệu cần gửi, không được vượt quá DEVICE_FS_BULK_IN_MPS byte.
 * @param  TimeOut Thời gian chờ (timeout), đơn vị mili giây (ms).
 * 
 * @return Mã lỗi kiểu OTG_DEVICE_ERR_CODE.
 */
OTG_DEVICE_ERR_CODE OTG_DeviceInterruptSend(uint8_t EpNum, uint8_t* Buf, uint32_t Len, uint32_t TimeOut);


/**
 * @brief  ��INT OUT�˵��������
 * @param  EpNum �˵��
 * @param  Buf ���ݻ�����ָ��
 * @param  MaxLen ������ݳ���
 * @param  *pTransferLenʵ�ʽ��յ������ݳ���
 * @param  TimeOut��ʱʱ�䣬ms
 * @return OTG_DEVICE_ERR_CODE
 */
OTG_DEVICE_ERR_CODE OTG_DeviceInterruptReceive(uint8_t EpNum, uint8_t* Buf, uint16_t MaxLen, uint32_t *pTransferLen ,uint32_t TimeOut);


/**
 * @brief  DEVICEģʽ��ʹ��ĳ���˵��ж�
 * @param  Pipe Pipeָ��
 * @param  Func �жϻص�����ָ��
 * @return NONE
 */
void OTG_EndpointInterruptEnable(uint8_t EpNum, FPCALLBACK Func);


/**
 * @brief  DEVICEģʽ�½�ֹĳ���˵��ж�
 * @param  EpNum �˵��
 * @return NONE
 */
void OTG_EndpointInterruptDisable(uint8_t EpNum);

/**
 * @brief  OTG PowerDown
 * @param  NONE
 * @return NONE
 */
void OTG_PowerDown(void);

/**
 * @brief  OTG Pause detect & Backup reg &Powerdown
 * @param  NONE
 * @return NONE
 */
void OTG_DeepSleepBackup(void);

/**
 * @brief  OTG resume Reg/PowerOn & Detect
 * @param  NONE
 * @return NONE
 */
void OTG_WakeupResume(void);

#ifdef __cplusplus
}
#endif//__cplusplus

#endif //__OTG_DEVICE_HCD_H__
/**
 * @}
 * @}
 */
