// Created by: Marcin Dziedzic
// fade.c

#include "fade.h"
#include "stm32f1xx_hal.h"

/**
 * @brief Funkcja wewnętrzna inicjalizująca wspólne parametry fade.
 */
static void LedFade_InternalInit(LedFadeHandle_t *handle,
                                 TIM_HandleTypeDef *htim,
                                 uint32_t channel,
                                 FadeMode_e mode,
                                 FadeDirection_e direction,
                                 uint16_t steps,
                                 uint32_t totalTimeMs)
{
    handle->htim       = htim;
    handle->channel    = channel;
    handle->mode       = mode;
    handle->direction  = direction;
    handle->steps      = steps;
    handle->currentStep= 0;
    handle->isActive   = true;

    // Odczyt wartości AutoReload (ARR)
    handle->arr = __HAL_TIM_GET_AUTORELOAD(htim);

    // Czas pomiędzy kolejnymi krokami
    handle->stepInterval = (totalTimeMs / steps);
    handle->lastTime     = HAL_GetTick();

    // Uruchomienie PWM (o ile nie jest włączone)
    HAL_TIM_PWM_Start(htim, channel);

    // Wartość początkowa CCR w zależności od kierunku
    if (direction == FADE_IN)
    {
        // FADE_IN => start: CCR = ARR (wyłączona), dojdziemy do 0 (jasno)
        __HAL_TIM_SET_COMPARE(htim, channel, handle->arr);
    }
    else
    {
        // FADE_OUT => start: CCR = 0, pójdziemy do ARR (zgaszenie)
        __HAL_TIM_SET_COMPARE(htim, channel, 0);
    }
}

/**
 * @brief Rozpoczęcie pojedynczego rozjaśniania/przygaszania (FADE_MODE_SINGLE).
 */
void LedFade_Start(LedFadeHandle_t *handle,
                   TIM_HandleTypeDef *htim,
                   uint32_t channel,
                   FadeDirection_e direction,
                   uint16_t steps,
                   uint32_t totalTimeMs)
{
    LedFade_InternalInit(handle,
                         htim,
                         channel,
                         FADE_MODE_SINGLE,
                         direction,
                         steps,
                         totalTimeMs);
}

/**
 * @brief Uruchamia tryb pulsowania: FADE_IN (ARR->0) i FADE_OUT (0->ARR) w kółko.
 */
void LedFade_PulseStart(LedFadeHandle_t *handle,
                        TIM_HandleTypeDef *htim,
                        uint32_t channel,
                        uint16_t steps,
                        uint32_t totalTimeMs)
{
    LedFade_InternalInit(handle,
                         htim,
                         channel,
                         FADE_MODE_PULSE,
                         FADE_IN,     // startujemy od rozjaśniania
                         steps,
                         totalTimeMs);
}

/**
 * @brief Funkcja wywoływana cyklicznie (np. w pętli). Realizuje kolejne kroki fade.
 * @return true, jeśli właśnie zakończono FADE_MODE_SINGLE, w przeciwnym razie false.
 */
bool LedFade_Process(LedFadeHandle_t *handle)
{
    if (!handle->isActive)
    {
        return false; // Nic nie robimy, fade nieaktywny
    }

    uint32_t now = HAL_GetTick();

    // Czy minął czas na kolejny krok?
    if ((now - handle->lastTime) < handle->stepInterval)
    {
        return false;
    }

    // Aktualizacja "ostatniego" czasu
    handle->lastTime = now;

    // Obliczamy nowy CCR
    uint16_t newCompare = 0;
    if (handle->direction == FADE_IN)
    {
        // FADE_IN: currentStep=0 => CCR=ARR; currentStep=steps => CCR=0
        newCompare = handle->arr
                     - (handle->currentStep * (handle->arr / handle->steps));
    }
    else
    {
        // FADE_OUT: currentStep=0 => CCR=0; currentStep=steps => CCR=ARR
        newCompare = (handle->currentStep * (handle->arr / handle->steps));
    }

    // Ustawiamy CCR
    __HAL_TIM_SET_COMPARE(handle->htim, handle->channel, newCompare);

    // Następny krok
    handle->currentStep++;

    // Czy osiągnęliśmy koniec?
    if (handle->currentStep > handle->steps)
    {
        // Ustaw wartość docelową (dla pewności)
        if (handle->direction == FADE_IN)
        {
            __HAL_TIM_SET_COMPARE(handle->htim, handle->channel, 0);
        }
        else
        {
            __HAL_TIM_SET_COMPARE(handle->htim, handle->channel, handle->arr);
        }

        // Jeśli tryb pojedynczy -> kończymy
        if (handle->mode == FADE_MODE_SINGLE)
        {
            handle->isActive = false;
            return true;
        }
        else
        {
            // Tryb PULSE -> zmieniamy kierunek i zaczynamy od 0.
            if (handle->direction == FADE_IN)
            {
                handle->direction = FADE_OUT;
            }
            else
            {
                handle->direction = FADE_IN;
            }
            handle->currentStep = 0;
        }
    }

    return false;
}
