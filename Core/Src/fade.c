#include "fade.h"
#include "stm32f1xx_hal.h"
#include <stdint.h>

/**
 * @brief Rozpoczęcie pracy kanału PWM na zadanym timerze i kanale.
 */
void LedFade_Init(TIM_HandleTypeDef *htim, uint32_t channel)
{
    // Uruchamiamy PWM na wskazanym kanale
    HAL_TIM_PWM_Start(htim, channel);

    // Na start można ustawić PWM na 0% duty cycle (opcjonalnie)
    uint16_t arr = __HAL_TIM_GET_AUTORELOAD(htim);
    __HAL_TIM_SET_COMPARE(htim, channel, arr);
}

/**
 * @brief Funkcja rozjaśniania (fade-in): 0% -> 100% w zadanym czasie.
 */
void LedFade_In(TIM_HandleTypeDef *htim, uint32_t channel, uint16_t steps, uint32_t totalTimeMs)
{
    uint16_t arr = __HAL_TIM_GET_AUTORELOAD(htim);
    uint16_t stepSize = arr / steps;
    uint32_t stepDelay = totalTimeMs / steps;

    // Zaczynamy od arr (lampka fizycznie WYŁĄCZONA) i schodzimy do 0 (lampka WŁĄCZONA).
    for (uint16_t i = 0; i <= steps; i++)
    {
        uint16_t compareVal = arr - (i * stepSize);
        __HAL_TIM_SET_COMPARE(htim, channel, compareVal);
        HAL_Delay(stepDelay);
    }
}


/**
 * @brief Funkcja przygaszania (fade-out): 100% -> 0% w zadanym czasie.
 */
void LedFade_Out(TIM_HandleTypeDef *htim, uint32_t channel, uint16_t steps, uint32_t totalTimeMs)
{
    uint16_t arr = __HAL_TIM_GET_AUTORELOAD(htim);
    uint16_t stepSize = arr / steps;
    uint32_t stepDelay = totalTimeMs / steps;

    // Zaczynamy od 0 (fizycznie WŁĄCZONA) i idziemy do arr (WYŁĄCZONA).
    for (uint16_t i = 0; i <= steps; i++)
    {
        uint16_t compareVal = 0 + (i * stepSize);
        __HAL_TIM_SET_COMPARE(htim, channel, compareVal);
        HAL_Delay(stepDelay);
    }
}

