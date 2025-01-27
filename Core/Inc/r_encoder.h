#ifndef R_ENCODER_H
#define R_ENCODER_H

#include "stm32f1xx_hal.h"

/**
 * @brief  Struktura przechowująca potrzebne informacje o enkoderze.
 */
typedef struct {
    TIM_HandleTypeDef *htim;       // uchwyt do timera w trybie Encoder
    GPIO_TypeDef      *btn_port;   // port przycisku
    uint16_t          btn_pin;     // pin przycisku
    int16_t           last_count;  // poprzednia wartość licznika
} REncoder_HandleTypeDef;

/**
 * @brief  Inicjalizacja enkodera oraz start timera w trybie Encoder.
 * @param  henc      - wskaźnik do struktury REncoder_HandleTypeDef.
 * @param  htim      - wskaźnik do uchwytu timera skonfigurowanego w trybie Encoder.
 * @param  btn_port  - port do którego podłączony jest przycisk enkodera.
 * @param  btn_pin   - pin przycisku enkodera.
 */
void REncoder_Init(REncoder_HandleTypeDef *henc,
                   TIM_HandleTypeDef *htim,
                   GPIO_TypeDef *btn_port,
                   uint16_t btn_pin);

/**
 * @brief  Odczyt aktualnego stanu enkodera i wykrycie zdarzeń.
 * @param  henc - wskaźnik do struktury REncoder_HandleTypeDef.
 * @retval Zwraca:
 *         - 1  -> Obrót w prawo
 *         - 0  -> Obrót w lewo
 *         - 2  -> Wciśnięcie przycisku
 *         - -1 -> Brak zdarzenia
 */
int REncoder_Update(REncoder_HandleTypeDef *henc);

#endif // R_ENCODER_H
