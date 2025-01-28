// Created by: Marcin Dziedzic
// light_sen.h

#ifndef LIGHT_SEN_H_
#define LIGHT_SEN_H_

#include <stdint.h>
#include "lcd.h"
#include "stm32f1xx_hal.h"

/**
 * @brief Adres sensora BH1750 po magistrali I2C.
 */
#define BH1750_ADDRESS 0x23

/**
 * @brief Inicjalizacja czujnika światła (BH1750).
 * @param hi2c Uchwyt (handler) do interfejsu I2C.
 */
void LightSen_Init(I2C_HandleTypeDef *hi2c);

/**
 * @brief Odczyt wartości natężenia światła (w luksach) z sensora BH1750.
 * @param hi2c Uchwyt do interfejsu I2C.
 * @return Zwraca wartość w luksach (uint16_t).
 */
uint16_t LightSen_ReadLux(I2C_HandleTypeDef *hi2c);

/**
 * @brief Wyświetlenie na LCD bieżącej wartości z czujnika światła.
 * @param lcd Wskaźnik do struktury obsługującej LCD.
 * @param lux Aktualna wartość luksów do wyświetlenia.
 */
void LightSen_DisplayLux(Lcd_HandleTypeDef *lcd, uint16_t lux);

#endif /* LIGHT_SEN_H_ */
