#ifndef RTC_H
#define RTC_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

/**
 * @brief Struktura przechowująca dane o czasie
 *        (w formacie dziesiętnym po konwersji z/do BCD).
 */
typedef struct {
    uint8_t seconds;  /*!< 0..59 */
    uint8_t minutes;  /*!< 0..59 */
    uint8_t hours;    /*!< 0..23 (24h) */
    uint8_t day;      /*!< 1..31 */
    uint8_t weekday;  /*!< 0..6  (0 = niedziela, 1 = poniedziałek, ...) */
    uint8_t month;    /*!< 1..12 */
    uint8_t year;     /*!< 0..99 => 2000..2099 */
} RTC_TimeTypeDef;

/**
 * @brief Inicjalizuje obsługę RTC, zapamiętując uchwyt do I2C.
 * @param hi2c: wskaźnik do zainicjalizowanego interfejsu I2C (np. hi2c1).
 */
void RTC_Init(I2C_HandleTypeDef *hi2c);

/**
 * @brief Ustawia czas w układzie PCF85063AT.
 * @param time: wskaźnik do struktury z czasem (w formacie dziesiętnym).
 */
void RTC_SetTime(const RTC_TimeTypeDef *time);

/**
 * @brief  Zalecany przez dokumentację sposób odczytu czasu (sek..rok).
 * @param  time: wskaźnik do struktury, w której zostanie zwrócony czas (w formacie dziesiętnym).
 */
void RTC_ReadTime(RTC_TimeTypeDef *time);

#endif /* RTC_H */
