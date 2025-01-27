#include "light_sen.h"

/**
 * Inicjalizacja sensora BH1750
 * @param hi2c Wskaźnik do handlera I2C
 */
void LightSen_Init(I2C_HandleTypeDef *hi2c) {
    uint8_t cmd = 0x10; // 1 lx rozdzielczości, czas 120 ms
    HAL_I2C_Master_Transmit(hi2c, BH1750_ADDRESS << 1, &cmd, 1, HAL_MAX_DELAY);
}

/**
 * Odczyt wartości natężenia światła w luksach
 * @param hi2c Wskaźnik do handlera I2C
 * @return Odczytana wartość w luksach
 */
uint16_t LightSen_ReadLux(I2C_HandleTypeDef *hi2c) {
    uint8_t buff[2] = {0};
    uint16_t lux = 0;

    // Odczyt danych z sensora
    HAL_I2C_Master_Receive(hi2c, BH1750_ADDRESS << 1, buff, 2, HAL_MAX_DELAY);

    // Konwersja wartości do luksów
    lux = ((buff[0] << 8) | buff[1]) / 1.2;
    return lux;
}

/**
 * Wyświetlenie wartości natężenia światła na ekranie LCD
 * @param lcd Wskaźnik do struktury LCD
 * @param lux Wartość natężenia światła w luksach
 */
void LightSen_DisplayLux(Lcd_HandleTypeDef *lcd, uint16_t lux) {
    char buffer[16];
    sprintf(buffer, "Lux: %u lx", lux);
    Lcd_clear(lcd);
    Lcd_string(lcd, buffer);
}
