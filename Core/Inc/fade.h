// Created by: Marcin Dziedzic
// fade.h

#ifndef FADE_H
#define FADE_H

#include "stm32f1xx_hal.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Kierunek fade:
 *        - FADE_IN:  z 100% do 0% (np. CCR: arr -> 0)
 *        - FADE_OUT: z 0% do 100% (np. CCR: 0 -> arr)
 */
typedef enum
{
    FADE_IN,
    FADE_OUT
} FadeDirection_e;

/**
 * @brief Tryb pracy fade:
 *        - FADE_MODE_SINGLE: jeden cykl (FADE_IN lub FADE_OUT) i koniec.
 *        - FADE_MODE_PULSE: cykliczne rozjaśnianie i przygaszanie
 *                           (FADE_IN -> FADE_OUT -> FADE_IN -> OUT...).
 */
typedef enum
{
    FADE_MODE_SINGLE,
    FADE_MODE_PULSE
} FadeMode_e;

/**
 * @brief Struktura przechowująca stan procesu fade/pulse (bez HAL_Delay).
 */
typedef struct
{
    TIM_HandleTypeDef *htim;  /**< Uchwyty timera */
    uint32_t channel;         /**< Kanał PWM */

    bool isActive;            /**< Czy proces trwa? */
    FadeMode_e mode;          /**< Pojedynczy cykl czy pulsowanie w kółko */
    FadeDirection_e direction;/**< Aktualny kierunek (FADE_IN / FADE_OUT) */
    uint16_t steps;           /**< Liczba kroków w jednym cyklu */
    uint16_t currentStep;     /**< Aktualny krok */
    uint16_t arr;             /**< AutoReload timera (maks licznika) */

    uint32_t stepInterval;    /**< Co ile ms wykonujemy 1 krok */
    uint32_t lastTime;        /**< Ostatni moment (ms, z HAL_GetTick) wykonania kroku */

    /**
     *  Dla trybu PULSE:
     *   - totalTimeMs to czas FADE_IN,
     *   - i taki sam czas FADE_OUT.
     *   => sumarycznie jeden cykl (góra + dół) = 2 * totalTimeMs.
     */
} LedFadeHandle_t;

/**
 * @brief Rozpoczęcie pojedynczego rozjaśniania lub przyciemniania (FADE_MODE_SINGLE).
 * @param handle      Obiekt stanu
 * @param htim        Uchwyt timera
 * @param channel     Kanał PWM
 * @param direction   FADE_IN (rozjaśnianie) lub FADE_OUT (przyciemnianie)
 * @param steps       Ilość kroków w całym cyklu
 * @param totalTimeMs Czas (ms) całego cyklu
 */
void LedFade_Start(LedFadeHandle_t *handle,
                   TIM_HandleTypeDef *htim,
                   uint32_t channel,
                   FadeDirection_e direction,
                   uint16_t steps,
                   uint32_t totalTimeMs);

/**
 * @brief Uruchamia tryb PULSE (rozjaśnianie -> przyciemnianie -> rozjaśnianie -> ...).
 * @param handle      Obiekt stanu
 * @param htim        Uchwyt timera
 * @param channel     Kanał PWM
 * @param steps       Ilość kroków na rozjaśnianie i tyle samo na przyciemnianie
 * @param totalTimeMs Czas (ms) FADE_IN (oraz tyle samo na FADE_OUT),
 *                    sumarycznie cykl "góra + dół" = 2 * totalTimeMs
 */
void LedFade_PulseStart(LedFadeHandle_t *handle,
                        TIM_HandleTypeDef *htim,
                        uint32_t channel,
                        uint16_t steps,
                        uint32_t totalTimeMs);

/**
 * @brief Funkcja wywoływana cyklicznie (np. w pętli głównej),
 *        obsługuje krok fade/pulse jeśli upłynął odpowiedni czas stepInterval.
 * @param handle  Obiekt stanu fade
 * @return true, jeśli zakończono pojedynczy cykl (FADE_MODE_SINGLE).
 *         W trybie PULSE zwraca false (chyba że chcemy wykryć inny koniec).
 */
bool LedFade_Process(LedFadeHandle_t *handle);

#ifdef __cplusplus
}
#endif

#endif // FADE_H
