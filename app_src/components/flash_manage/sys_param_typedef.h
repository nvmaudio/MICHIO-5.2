#ifndef __SYS_PARAMETER_H__
#define __SYS_PARAMETER_H__


typedef struct _SYS_PARAMETER_
{
	uint8_t         byte_check;

	char 			Board_name[40];
	uint8_t         Activate;	
	uint8_t 		FW_Version[3];
	uint8_t 		HID_Version[3];

	uint8_t 		DAC0_EN;
	uint8_t 		DACX_EN;
	uint8_t 		I2S0_EN;
	uint8_t 		I2S1_EN;
	
	uint8_t 		Volume_tong;
	uint8_t 		Volume_thong_bao;
	uint8_t 		Nho_volume;
	uint8_t 		Dongbo_volume;

	char 			BT_Name[40];
	uint8_t 		Profile_BT;
	uint8_t 		A2dp_AAC;
	uint8_t 		Bt_BackgroundType;
	uint8_t 		Bt_SimplePairingEnable;
	char 			Bt_PinCode[6];

	uint8_t 		Thong_bao_en;
	uint8_t 		Item_thong_bao[8]; 			// BTmode AUXmode PCmode connect disconnect tws_connect Volmax BTpair

	uint8_t 		Key_en;
	uint8_t 		Key_matrix[11][5];

	uint8_t 		Vr_en;
	uint8_t 		Vr_dac_line;
	uint8_t 		Buoc_eq;
	uint8_t 		Heso;

} SYS_PARAMETER;

 

typedef struct _DATA_DEM_
{
	uint8_t 		byte_check;

	uint8_t 		mode;
	uint8_t 		volume;
	uint8_t 		remind_error;
} DATA_DEM;


#endif
