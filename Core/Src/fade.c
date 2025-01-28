#include "fade.h"
#include "stm32f1xx_hal.h"

/**
 * @brief Funkcja wewnętrzna: ustawia wstępne parametry fade
 *        (ARR, stepInterval, PWM start).
 */
static void LedFade_InternalInit(LedFadeHandle_t *handle,
                                 TIM_HandleTypeDef *htim,
                                 uint32_t channel,
                                 FadeMode_e mode,
                                 FadeDirection_e direction,
                                 uint16_t steps,
                                 uint32_t totalTimeMs)
{
    handle->htim = htim;
    handle->channel = channel;
    handle->mode = mode;
    handle->direction = direction;
    handle->steps = steps;
    handle->currentStep = 0;
    handle->isActive = true;

    // Odczytujemy AutoReload
    handle->arr = __HAL_TIM_GET_AUTORELOAD(htim);

    // Obliczamy odstęp między krokami.
    // Dla FADE_IN/FADE_OUT => totalTimeMs to czas jednego przejścia.
    handle->stepInterval = (totalTimeMs / steps);

    // Czas startu
    handle->lastTime = HAL_GetTick();

    // Uruchamiamy PWM, jeśli nie jest
    HAL_TIM_PWM_Start(htim, channel);

    // W zależności od direction, ustawiamy wartość początkową.
    // FADE_IN => start: CCR=arr (wyłączona), schodzimy do 0 (pełna jasność).
    // FADE_OUT => start: CCR=0, idziemy do arr.
    if (direction == FADE_IN)
    {
        __HAL_TIM_SET_COMPARE(htim, channel, handle->arr);
    }
    else
    {
        __HAL_TIM_SET_COMPARE(htim, channel, 0);
    }
}

void LedFade_Start(LedFadeHandle_t *handle,
                   TIM_HandleTypeDef *htim,
                   uint32_t channel,
                   FadeDirection_e direction,
                   uint16_t steps,
                   uint32_t totalTimeMs)
{
    LedFade_InternalInit(handle, htim, channel,
                         FADE_MODE_SINGLE,
                         direction,
                         steps,
                         totalTimeMs);
}

/**
 * @brief Uruchamia tryb pulsowania:
 *        najpierw FADE_IN (arr->0) w totalTimeMs,
 *        potem FADE_OUT (0->arr) w totalTimeMs,
 *        i tak w kółko.
 */
void LedFade_PulseStart(LedFadeHandle_t *handle,
                        TIM_HandleTypeDef *htim,
                        uint32_t channel,
                        uint16_t steps,
                        uint32_t totalTimeMs)
{
    LedFade_InternalInit(handle, htim, channel,
                         FADE_MODE_PULSE,
                         FADE_IN,  // startujemy od rozjaśniania
                         steps,
                         totalTimeMs);
}

/**
 * @brief Funkcja, którą wywołujemy cyklicznie (np. w pętli głównej).
 *        Realizuje kolejne kroki rozjaśniania/przygaszania, bez blokowania.
 * @return true, jeśli FADE_MODE_SINGLE właśnie się zakończył,
 *         false - w przeciwnym wypadku.
 */
bool LedFade_Process(LedFadeHandle_t *handle)
{
    if (!handle->isActive)
    {
        return false; // nic się nie dzieje
    }

    uint32_t now = HAL_GetTick();

    // Sprawdzamy, czy upłynął czas na kolejny krok
    if ((now - handle->lastTime) < handle->stepInterval)
    {
        return false;
    }

    // Zaktualizuj lastTime
    handle->lastTime = now;

    // Oblicz nowy CCR
    uint16_t newCompare = 0;
    if (handle->direction == FADE_IN)
    {
        // currentStep=0 => CCR=arr; currentStep=steps => CCR=0
        newCompare = handle->arr
                     - (handle->currentStep * (handle->arr / handle->steps));
    }
    else // FADE_OUT
    {
        // currentStep=0 => CCR=0; currentStep=steps => CCR=arr
        newCompare = (handle->currentStep * (handle->arr / handle->steps));
    }

    __HAL_TIM_SET_COMPARE(handle->htim, handle->channel, newCompare);

    // Kolejny krok
    handle->currentStep++;

    // Czy doszliśmy do końca?
    if (handle->currentStep > handle->steps)
    {
        // Ustaw ostateczną wartość
        if (handle->direction == FADE_IN)
        {
            __HAL_TIM_SET_COMPARE(handle->htim, handle->channel, 0);
        }
        else
        {
            __HAL_TIM_SET_COMPARE(handle->htim, handle->channel, handle->arr);
        }

        // FADE_MODE_SINGLE -> kończymy
        if (handle->mode == FADE_MODE_SINGLE)
        {
            handle->isActive = false;
            return true; // właśnie się zakończyło
        }
        else
        {
            // FADE_MODE_PULSE -> zmieniamy kierunek i zaczynamy od 0.
            if (handle->direction == FADE_IN)
            {
                handle->direction = FADE_OUT;
            }
            else
            {
                handle->direction = FADE_IN;
            }
            handle->currentStep = 0;
            // Pomiń return true, bo w pulse chcemy ciągły cykl.
        }
    }

    return false; // wciąż trwa lub pulsuje dalej
}
