

#ifndef __OTG_DEVICE_STANDARD_H__
#define	__OTG_DEVICE_STANDARD_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus 

#include "type.h"

#define USB_DT_DEVICE					1
#define USB_DT_CONFIG					2
#define USB_DT_STRING					3
#define USB_DT_INTERFACE				4
#define USB_DT_ENDPOINT					5
#define USB_DT_DEVICE_QUALIFIER			6
#define USB_HID_REPORT					0x22

#define USB_REQ_GET_STATUS				0
#define USB_REQ_CLEAR_FEATURE			1
#define USB_REQ_SET_FEATURE				3
#define USB_REQ_SET_ADDRESS				5
#define USB_REQ_GET_DESCRIPTOR			6
#define USB_REQ_SET_DESCRIPTOR			7
#define USB_REQ_GET_CONFIGURATION		8
#define USB_REQ_SET_CONFIGURATION		9
#define USB_REQ_GET_INTERFACE			10
#define USB_REQ_SET_INTERFACE			11
#define USB_REQ_SYNCH_FRAME				12


// Max packet size. Fixed, user should not modify.
#define	DEVICE_FS_CONTROL_MPS		64
#define	DEVICE_FS_INT_IN_MPS		64
#define	DEVICE_FS_BULK_IN_MPS		64
#define	DEVICE_FS_BULK_OUT_MPS		64
#define	DEVICE_FS_ISO_IN_MPS		192
#define	DEVICE_FS_ISO_OUT_MPS		192

// Các endpoint cố định (không nên thay đổi)
#define	DEVICE_CONTROL_EP			    0x00	// Control endpoint
#define	DEVICE_INT_IN_EP1			    0x81	// Interrupt IN endpoint (bit 7 = IN)
#define	DEVICE_INT_IN_EP2			    0x82	// Interrupt IN endpoint (bit 7 = IN)

#define	DEVICE_BULK_IN_EP			    0x83	// Bulk IN
#define	DEVICE_BULK_OUT_EP			    0x03	// Bulk OUT


#define	DEVICE_ISO_IN_EP			    0x84	// Isochronous IN
#define	DEVICE_ISO_OUT_EP			    0x05	// Isochronous OUT


#define	DEVICE_HID_IN_EP			0x85
#define	DEVICE_FS_INT_IN_MPS		64

#define MSC_INTERFACE_NUM			0
#define AUDIO_ATL_INTERFACE_NUM		1
#define AUDIO_SRM_OUT_INTERFACE_NUM	2
#define AUDIO_SRM_IN_INTERFACE_NUM	3
#define HID_CTL_INTERFACE_NUM		4
#define HID_DATA_INTERFACE_NUM		5

#define USB_VID				0x8888
#define USB_PID_BASE		0x1717//����PID�������й���ֵ��ΪOffset
#define AUDIO_ONLY			0
#define MIC_ONLY			1
#define AUDIO_MIC			2

#ifdef USB_READER_EN
#define READER				3
#define AUDIO_READER		4
#define MIC_READER			5
#define AUDIO_MIC_READER	6
#endif

#define HID					7
#define USBPID(x)			(USB_PID_BASE + x)

//�������ߵ���
#define HID_DATA_FUN_EN	1
//MIC�ϴ���������
#define MIC_CH	2

void OTG_DeviceModeSel(uint8_t Mode,uint16_t UsbVid,uint16_t UsbPid);
void OTG_DeviceRequestProcess(void);

void hid_send_data(void);


#ifdef __cplusplus
}
#endif // __cplusplus 

#endif //__OTG_DEVICE_STANDARD_H__

