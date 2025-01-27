#ifndef LED_FADE_H
#define LED_FADE_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

/**
 * @brief Inicjalizacja PWM - uruchamia kanał PWM na zadanym Timerze i kanale.
 * @param htim    wskaźnik do zainicjalizowanego TIM_HandleTypeDef
 * @param channel kanał timera, np. TIM_CHANNEL_4
 */
void LedFade_Init(TIM_HandleTypeDef *htim, uint32_t channel);

/**
 * @brief Rozjaśnianie LED (fade in) w zadanym czasie całkowitym i liczbie kroków.
 *        Zaczyna od 0% duty cycle i kończy na 100%.
 * @param htim        wskaźnik do timera (np. &htim3)
 * @param channel     kanał timera (np. TIM_CHANNEL_4)
 * @param steps       liczba kroków (im więcej, tym płynniejsza zmiana)
 * @param totalTimeMs łączny czas w milisekundach
 */
void LedFade_In(TIM_HandleTypeDef *htim, uint32_t channel, uint16_t steps, uint32_t totalTimeMs);

/**
 * @brief Przygaszanie LED (fade out) w zadanym czasie całkowitym i liczbie kroków.
 *        Zaczyna od 100% duty cycle i kończy na 0%.
 * @param htim        wskaźnik do timera (np. &htim3)
 * @param channel     kanał timera (np. TIM_CHANNEL_4)
 * @param steps       liczba kroków (im więcej, tym płynniejsza zmiana)
 * @param totalTimeMs łączny czas w milisekundach
 */
void LedFade_Out(TIM_HandleTypeDef *htim, uint32_t channel, uint16_t steps, uint32_t totalTimeMs);

#endif /* LED_FADE_H */
