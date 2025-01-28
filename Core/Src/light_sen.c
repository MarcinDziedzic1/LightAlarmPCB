// Created by: Marcin Dziedzic
// light_sen.c

#include "light_sen.h"

/**
 * @brief Inicjalizacja sensora BH1750.
 * @param hi2c Wskaźnik do handlera I2C.
 */
void LightSen_Init(I2C_HandleTypeDef *hi2c)
{
    uint8_t cmd = 0x10; // Rozdzielczość 1 lx, czas 120 ms
    HAL_I2C_Master_Transmit(hi2c, BH1750_ADDRESS << 1, &cmd, 1, HAL_MAX_DELAY);
}

/**
 * @brief Odczyt wartości natężenia światła w luksach.
 * @param hi2c Wskaźnik do handlera I2C.
 * @return Odczytana wartość w luksach (uint16_t).
 */
uint16_t LightSen_ReadLux(I2C_HandleTypeDef *hi2c)
{
    uint8_t buff[2] = {0};
    uint16_t lux = 0;

    // Odczyt danych z sensora
    HAL_I2C_Master_Receive(hi2c, BH1750_ADDRESS << 1, buff, 2, HAL_MAX_DELAY);

    // Konwersja wartości do luksów
    lux = ((buff[0] << 8) | buff[1]) / 1.2;
    return lux;
}

/**
 * @brief Wyświetlenie wartości natężenia światła na wyświetlaczu LCD.
 * @param lcd Wskaźnik do struktury obsługi LCD.
 * @param lux Wartość natężenia światła w luksach.
 */
void LightSen_DisplayLux(Lcd_HandleTypeDef *lcd, uint16_t lux)
{
    // 1) Wyczyść drugi wiersz (16 spacji)
    Lcd_cursor(lcd, 1, 0);
    Lcd_string(lcd, "               "); // 16 spacji = wyczyszczenie

    // 2) Bufor na wartość lux (maks 5 cyfr + '\0')
    char valStr[6];
    sprintf(valStr, "%u", lux);

    // 3) W kolumnie 0 wyświetl "Lux: "
    Lcd_cursor(lcd, 0, 0);
    Lcd_string(lcd, "Lux: ");

    // 4) W kolumnie 5 wyświetl zmierzoną wartość lux
    Lcd_cursor(lcd, 0, 5);
    Lcd_string(lcd, valStr);

    // (Opcjonalnie) Nadpisanie spacjami, jeśli nowa liczba jest krótsza
    int len = strlen(valStr);
    for (int i = len; i < 5; i++)
    {
        Lcd_cursor(lcd, 0, 5 + i);
        Lcd_string(lcd, " ");
    }

    // 5) W kolumnie 10 zawsze wyświetl "lx"
    Lcd_cursor(lcd, 0, 10);
    Lcd_string(lcd, "lx");
}
