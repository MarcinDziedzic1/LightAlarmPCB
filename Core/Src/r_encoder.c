// Created by: Marcin Dziedzic
// r_encoder.c
#include "r_encoder.h"

/**
 * @brief Inicjalizacja enkodera i uruchomienie timera w trybie Encoder.
 */
void REncoder_Init(REncoder_HandleTypeDef *henc,
                   TIM_HandleTypeDef *htim,
                   GPIO_TypeDef *btn_port,
                   uint16_t btn_pin)
{
    // Zapamiętujemy uchwyty
    henc->htim = htim;
    henc->btn_port = btn_port;
    henc->btn_pin = btn_pin;

    // Czyścimy poprzedni stan licznika
    henc->last_count = __HAL_TIM_GET_COUNTER(htim);

    // Start timera w trybie enkodera (dla obu kanałów)
    HAL_TIM_Encoder_Start(henc->htim, TIM_CHANNEL_ALL);
}

/**
 * @brief Sprawdza, czy nastąpił obrót w prawo/lewo lub wciśnięcie przycisku.
 */
int REncoder_Update(REncoder_HandleTypeDef *henc)
{
    // Pobieramy obecną wartość licznika
    int16_t current_count = __HAL_TIM_GET_COUNTER(henc->htim);

    // Wyliczamy różnicę od poprzedniego odczytu
    int16_t diff = current_count - henc->last_count;

    // Aktualizujemy last_count
    henc->last_count = current_count;

    // Jeśli różnica > 0, obrót w prawo
    if (diff > 0) {
        return 1;
    }
    // Jeśli różnica < 0, obrót w lewo
    else if (diff < 0) {
        return 0;
    }
    // Jeśli nie ma ruchu na enkoderze, to sprawdzamy stan przycisku
    else {
        // Założenie: aktywny stan niski na przycisku
        if (HAL_GPIO_ReadPin(henc->btn_port, henc->btn_pin) == GPIO_PIN_RESET) {
            return 2;
        }
    }

    // Brak zdarzenia
    return -1;
}
