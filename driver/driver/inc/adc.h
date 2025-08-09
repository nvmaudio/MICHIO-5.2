/**
 *******************************************************************************
 * @file    adc.h
 * @brief	Giao diện trình điều khiển mô-đun SarAdc
 *
 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2014-12-26 14:01:05$
 * @Copyright (C) 2014, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 *******************************************************************************
 */

/**
 * @addtogroup ADC
 * @{
 * @defgroup adc adc.h
 * @{
 */

#ifndef __ADC_H__
#define __ADC_H__

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

	// Bộ chia tần số xung clock cho ADC
	typedef enum __ADC_DC_CLK_DIV
	{
		CLK_DIV_NONE = 0, // Không chia
		CLK_DIV_2,		  // Chia 2
		CLK_DIV_4,		  // Chia 4
		CLK_DIV_8,		  // Chia 8
		CLK_DIV_16,		  // Chia 16
		CLK_DIV_32,		  // Chia 32
		CLK_DIV_64,		  // Chia 64
		CLK_DIV_128,	  // Chia 128
		CLK_DIV_256,	  // Chia 256
		CLK_DIV_512,	  // Chia 512
		CLK_DIV_1024,	  // Chia 1024
		CLK_DIV_2048,	  // Chia 2048
		CLK_DIV_4096	  // Chia 4096

	} ADC_DC_CLK_DIV;

	// Các kênh đầu vào ADC
	typedef enum __ADC_DC_CHANNEL_SEL
	{
		ADC_CHANNEL_VIN = 0,	 /** Kênh 0 */
		ADC_CHANNEL_VBK,		 /** Kênh 1 */
		ADC_CHANNEL_VDD1V2,		 /** Kênh 2 */
		ADC_CHANNEL_VCOM,		 /** Kênh 3 */
		ADC_CHANNEL_GPIOA20_A23, /** Kênh 4 */
		ADC_CHANNEL_GPIOA21_A24, /** Kênh 5 */
		ADC_CHANNEL_GPIOA22_A25, /** Kênh 6 */
		ADC_CHANNEL_GPIOA26,	 /** Kênh 7 */
		ADC_CHANNEL_GPIOA27,	 /** Kênh 8 */
		ADC_CHANNEL_GPIOA28,	 /** Kênh 9 */
		ADC_CHANNEL_GPIOA29,	 /** Kênh 10 */
		ADC_CHANNEL_GPIOA30,	 /** Kênh 11 */
		ADC_CHANNEL_GPIOA31,	 /** Kênh 12 */
		ADC_CHANNEL_GPIOB0,		 /** Kênh 13 */
		ADC_CHANNEL_GPIOB1,		 /** Kênh 14 */
		ADC_CHANNEL_POWERKEY	 /** Kênh 15 (phím nguồn) */

	} ADC_DC_CHANNEL_SEL;

	// Lựa chọn điện áp tham chiếu cho ADC
	typedef enum __ADC_VREF
	{
		ADC_VREF_VDD,	// Vref = VDD
		ADC_VREF_VDDA,	// Vref = VDDA
		ADC_VREF_Extern // Vref ngoài (ngoại vi cấp)

	} ADC_VREF;

	// Chế độ chuyển đổi của ADC
	typedef enum __ADC_CONV
	{
		ADC_CON_SINGLE,	 // Chế độ chuyển đổi đơn (Single Conversion)
		ADC_CON_CONTINUA // Chế độ chuyển đổi liên tục (Continuous Conversion)
	} ADC_CONV;

	/**
	 * @brief  Kích hoạt ADC
	 * @param  Không có
	 * @return Không có
	 */
	void ADC_Enable(void);

	/**
	 * @brief  Vô hiệu hóa ADC
	 * @param  Không có
	 * @return Không có
	 */
	void ADC_Disable(void);

	/**
	 * @brief  Bật kênh PowerKey của ADC
	 * @param  Không có
	 * @return Không có
	 */
	void ADC_PowerkeyChannelEnable(void);

	/**
	 * @brief  Tắt kênh PowerKey của ADC
	 * @param  Không có
	 * @return Không có
	 */
	void ADC_PowerkeyChannelDisable(void);

	/**
	 * @brief  Hiệu chuẩn ADC, cần thực hiện trước khi bắt đầu chuyển đổi ADC
	 * @param  Không có
	 * @return Không có
	 */
	void ADC_Calibration(void);

	/**
	 * @brief  Thiết lập hệ số chia tần số xung clock ADC
	 * @param  Div: hệ số chia xung
	 * @return Không có
	 */
	void ADC_ClockDivSet(ADC_DC_CLK_DIV Div);

	/**
	 * @brief  Lấy hệ số chia tần số xung clock ADC
	 * @param  Không có
	 * @return Giá trị chia xung
	 */
	ADC_DC_CLK_DIV ADC_ClockDivGet(void);

	/**
	 * @brief  Cấu hình nguồn điện áp tham chiếu cho ADC
	 * @param  Mode: lựa chọn nguồn Vref
	 *         - 2: Vref ngoài
	 *         - 1: VDDA
	 *         - 0: 2*VMID
	 * @return Không có
	 */
	void ADC_VrefSet(ADC_VREF Mode);

	/**
	 * @brief  Thiết lập chế độ chuyển đổi của ADC
	 * @param  Mode: chế độ chuyển đổi
	 *         - 0: chế độ đơn (Single Mode)
	 *         - 1: chế độ liên tục (Continuous Mode, chỉ hỗ trợ 1 kênh với DMA)
	 * @return Không có
	 */
	void ADC_ModeSet(ADC_CONV Mode);

	/**
	 * @brief  Lấy dữ liệu ADC từ chế độ đơn
	 * @param  ChannalNum: số kênh ADC
	 * @return Giá trị ADC [0 - 4095]
	 */
	int16_t ADC_SingleModeDataGet(uint32_t ChannalNum);

	/**
	 * @brief  Bắt đầu chế độ lấy mẫu đơn (Single Mode) của ADC
	 *         (ADC 单次采样模式启动)
	 * @param  ChannalNum: số kênh ADC cần lấy mẫu
	 * @return Không có
	 * @note   Cần sử dụng cùng với các hàm:
	 *         - ADC_SingleModeDataStart
	 *         - ADC_SingleModeDataConvertionState
	 *         - ADC_SingleModeDataOut3
	 */
	void ADC_SingleModeDataStart(uint32_t ChannalNum);

	/**
	 * @brief  Kiểm tra quá trình chuyển đổi ADC đã hoàn thành chưa
	 * @param  Không có
	 * @return Trạng thái chuyển đổi
	 *         - TRUE: đã hoàn thành
	 *         - FALSE: chưa hoàn thành
	 * @note Phải dùng kết hợp với ADC_SingleModeDataStart và ADC_SingleModeDataOut
	 */
	bool ADC_SingleModeDataConvertionState(void);

	/**
	 * @brief  Lấy dữ liệu ADC sau khi chuyển đổi ở chế độ đơn
	 * @param  Không có
	 * @return Giá trị ADC [0 - 4095]
	 * @note Phải dùng kết hợp với ADC_SingleModeDataStart và ADC_SingleModeDataConvertionState
	 */
	int16_t ADC_SingleModeDataOut(void);

	/**
	 * @brief  Bật chế độ DMA cho ADC
	 * @param  Không có
	 * @return Không có
	 * @note   Trước khi bật cần thiết lập DMA đúng kênh.
	 *         Ở chế độ DMA, ADC chỉ hỗ trợ một kênh duy nhất.
	 */
	void ADC_DMAEnable(void);

	/**
	 * @brief  Tắt chế độ DMA cho ADC
	 * @param  Không có
	 * @return Không có
	 * @note   Trước khi bật cần thiết lập DMA đúng kênh.
	 *         Ở chế độ DMA, ADC chỉ hỗ trợ một kênh duy nhất.
	 */
	void ADC_DMADisable(void);

	/**
	 * @brief  Bắt đầu chế độ chuyển đổi liên tục (Continuous Mode)
	 * @param  ChannalNum: kênh ADC cần lấy dữ liệu
	 * @return Không có
	 */
	void ADC_ContinuModeStart(uint32_t ChannalNum);

	/**
	 * @brief  MCU lấy dữ liệu ADC ở chế độ chuyển đổi liên tục
	 * @param  Không có
	 * @return Giá trị ADC [0 - 4095]
	 */
	uint16_t ADC_ContinuModeDataGet(void);

	/**
	 * @brief  Dừng chế độ chuyển đổi liên tục của ADC
	 * @param  Không có
	 * @return Không có
	 */
	void ADC_ContinuModeStop(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __ADC_H__
/**
 * @}
 * @}
 */
