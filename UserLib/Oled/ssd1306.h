#ifndef __SSD1306_H__
#define __SSD1306_H__

#include "stm32f1xx_hal.h"
#include "oledfont.h"

// 声明全局的 I2C 句柄指针
extern I2C_HandleTypeDef *oled_i2c;

// 添加初始化函数声明
HAL_StatusTypeDef OLED_Init(I2C_HandleTypeDef *hi2c);

HAL_StatusTypeDef OLED_WR_CMD(uint8_t cmd);
void OLED_WR_DATA(uint8_t data);
void OLED_Clear(void);
void OLED_Display_On(void);
void OLED_Display_Off(void);
void OLED_Set_Pos(uint8_t x, uint8_t y);
void OLED_On(void);
void OLED_ShowNum(uint8_t x, uint8_t y, unsigned int num, uint8_t len, uint8_t size2, uint8_t Color_Turn);
void OLED_Showdecimal(uint8_t x, uint8_t y, float num, uint8_t z_len, uint8_t f_len, uint8_t size2, uint8_t Color_Turn);
void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr, uint8_t Char_Size, uint8_t Color_Turn);
void OLED_ShowString(uint8_t x, uint8_t y, char *chr, uint8_t Char_Size, uint8_t Color_Turn);
void OLED_ShowCHinese(uint8_t x, uint8_t y, uint8_t no, uint8_t Color_Turn);
void OLED_ShowCHinese_Array(uint8_t x, uint8_t y, uint8_t Color_Turn, uint8_t num, const uint8_t *indices);
void OLED_DrawBMP(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t *BMP, uint8_t Color_Turn);
void OLED_HorizontalShift(uint8_t direction);
void OLED_Some_HorizontalShift(uint8_t direction, uint8_t start, uint8_t end);
void OLED_VerticalAndHorizontalShift(uint8_t direction);
void OLED_DisplayMode(uint8_t mode);
void OLED_IntensityControl(uint8_t intensity);

#endif // __SSD1306_H__
