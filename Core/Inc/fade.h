#ifndef FADE_H
#define FADE_H

#include "stm32f1xx_hal.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Kierunek fade
 *        FADE_IN:  z 100% do 0% (np. CCR: arr -> 0)
 *        FADE_OUT: z 0% do 100% (np. CCR: 0 -> arr)
 */
typedef enum
{
    FADE_IN,
    FADE_OUT
} FadeDirection_e;

/**
 * @brief Tryb pracy fade:
 *        FADE_MODE_SINGLE – jeden cykl (FADE_IN lub FADE_OUT) i koniec.
 *        FADE_MODE_PULSE  – cykliczne rozjaśnianie i przygaszanie
 *                           (in -> out -> in -> out w pętli).
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
    TIM_HandleTypeDef *htim;  // uchwyt timera
    uint32_t channel;         // kanał PWM

    bool isActive;            // czy proces trwa
    FadeMode_e mode;          // pojedynczy cykl czy pulse w kółko
    FadeDirection_e direction;// aktualny kierunek (FADE_IN / FADE_OUT)
    uint16_t steps;           // liczba kroków w jednym cyklu
    uint16_t currentStep;     // aktualny krok
    uint16_t arr;             // AutoReload timera (max licznika)

    uint32_t stepInterval;    // co ile ms robimy 1 krok
    uint32_t lastTime;        // ostatni moment (ms, z HAL_GetTick) wykonania kroku

    // Dla trybu PULSE:
    //   - totalTimeMs to czas FADE_IN,
    //   - i taki sam czas FADE_OUT.
    //   => sumarycznie jeden cykl "góra + dół" = 2 * totalTimeMs.
} LedFadeHandle_t;

/**
 * @brief Inicjuje pojedyncze rozjaśnianie lub przyciemnianie (FADE_MODE_SINGLE).
 * @param handle      obiekt stanu
 * @param htim        uchwyt timera
 * @param channel     kanał PWM
 * @param direction   FADE_IN (rozjaśnianie) lub FADE_OUT (przyciemnianie)
 * @param steps       ile kroków w całym cyklu
 * @param totalTimeMs czas (w ms) całego cyklu
 */
void LedFade_Start(LedFadeHandle_t *handle,
                   TIM_HandleTypeDef *htim,
                   uint32_t channel,
                   FadeDirection_e direction,
                   uint16_t steps,
                   uint32_t totalTimeMs);

/**
 * @brief Inicjuje tryb PULSE (rozjaśnianie -> przyciemnianie -> rozjaśnianie -> ...)
 * @param handle      obiekt stanu
 * @param htim        uchwyt timera
 * @param channel     kanał PWM
 * @param steps       ile kroków na rozjaśnianie i tyle samo na przyciemnianie
 * @param totalTimeMs czas (w ms) FADE_IN (oraz tyle samo na FADE_OUT)
 *                    => pełen cykl "góra + dół" = 2 * totalTimeMs
 */
void LedFade_PulseStart(LedFadeHandle_t *handle,
                        TIM_HandleTypeDef *htim,
                        uint32_t channel,
                        uint16_t steps,
                        uint32_t totalTimeMs);

/**
 * @brief Funkcja wywoływana cyklicznie (np. w pętli),
 *        obsługuje krok fade/pulse jeśli upłynął odpowiedni czas stepInterval.
 * @param handle  obiekt stanu fade
 * @return true, jeśli właśnie zakończono pojedynczy cykl w FADE_MODE_SINGLE.
 *         W trybie PULSE zwraca false (chyba że chcemy wykrywać np. wyjście z pętli).
 */
bool LedFade_Process(LedFadeHandle_t *handle);

#ifdef __cplusplus
}
#endif

#endif // FADE_H
