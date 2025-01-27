#ifndef LIGHT_SEN_H_
#define LIGHT_SEN_H_

#include <stdint.h>
#include "lcd.h"
#include "stm32f1xx_hal.h"

// Adres I2C sensora BH1750
#define BH1750_ADDRESS 0x23

// Funkcje API sensora
void LightSen_Init(I2C_HandleTypeDef *hi2c);
uint16_t LightSen_ReadLux(I2C_HandleTypeDef *hi2c);
void LightSen_DisplayLux(Lcd_HandleTypeDef *lcd, uint16_t lux);

#endif /* LIGHT_SEN_H_ */
