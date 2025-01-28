// Created by: Marcin Dziedzic
// lcd.c

#include "lcd.h"

/**
 * @brief Tabela adresów początkowych wierszy dla LCD 16-znakowego.
 */
const uint8_t ROW_16[] = {0x00, 0x40, 0x10, 0x50};

/**
 * @brief Tabela adresów początkowych wierszy dla LCD 20-znakowego.
 */
const uint8_t ROW_20[] = {0x00, 0x40, 0x14, 0x54};

/* ======================== Deklaracje funkcji statycznych ======================== */

/**
 * @brief Zapis bajtu danych do rejestru danych LCD.
 */
static void lcd_write_data(Lcd_HandleTypeDef * lcd, uint8_t data);

/**
 * @brief Zapis bajtu do rejestru poleceń LCD.
 */
static void lcd_write_command(Lcd_HandleTypeDef * lcd, uint8_t command);

/**
 * @brief Wystawienie określonej liczby bitów na magistralę danych i impuls linii EN.
 */
static void lcd_write(Lcd_HandleTypeDef * lcd, uint8_t data, uint8_t len);


/* ======================== Implementacje funkcji publicznych ======================== */

/**
 * @brief Tworzy uchwyt LCD (Lcd_HandleTypeDef) i wywołuje jego inicjalizację.
 * @param port[]   Tablica portów dla linii danych.
 * @param pin[]    Tablica pinów dla linii danych.
 * @param rs_port  Port linii RS.
 * @param rs_pin   Pin linii RS.
 * @param en_port  Port linii Enable.
 * @param en_pin   Pin linii Enable.
 * @param mode     Tryb pracy (4- lub 8-bitowy).
 * @return         Struktura Lcd_HandleTypeDef z ustawionymi parametrami.
 */
Lcd_HandleTypeDef Lcd_create(
    Lcd_PortType port[],
    Lcd_PinType pin[],
    Lcd_PortType rs_port,
    Lcd_PinType rs_pin,
    Lcd_PortType en_port,
    Lcd_PinType en_pin,
    Lcd_ModeTypeDef mode)
{
    Lcd_HandleTypeDef lcd;

    lcd.mode      = mode;
    lcd.en_pin    = en_pin;
    lcd.en_port   = en_port;
    lcd.rs_pin    = rs_pin;
    lcd.rs_port   = rs_port;
    lcd.data_pin  = pin;
    lcd.data_port = port;

    Lcd_init(&lcd);
    return lcd;
}

/**
 * @brief Inicjalizuje wyświetlacz LCD, domyślnie wyłącza kursor.
 * @param lcd Wskaźnik do struktury Lcd_HandleTypeDef.
 */
void Lcd_init(Lcd_HandleTypeDef * lcd)
{
    if (lcd->mode == LCD_4_BIT_MODE)
    {
        lcd_write_command(lcd, 0x33);
        lcd_write_command(lcd, 0x32);
        lcd_write_command(lcd, FUNCTION_SET | OPT_N);  // Tryb 4-bit
    }
    else
    {
        lcd_write_command(lcd, FUNCTION_SET | OPT_DL | OPT_N); // Tryb 8-bit
    }

    lcd_write_command(lcd, CLEAR_DISPLAY);                   // Czyszczenie ekranu
    lcd_write_command(lcd, DISPLAY_ON_OFF_CONTROL | OPT_D);  // LCD włączony, kursor wyłączony, brak migania
    lcd_write_command(lcd, ENTRY_MODE_SET | OPT_INC);         // Inkrementacja kursora
}

/**
 * @brief Wypisuje liczbę całkowitą na bieżącej pozycji kursora.
 * @param lcd    Wskaźnik do struktury Lcd_HandleTypeDef.
 * @param number Liczba całkowita do wyświetlenia.
 */
void Lcd_int(Lcd_HandleTypeDef * lcd, int number)
{
    char buffer[11];
    sprintf(buffer, "%d", number);
    Lcd_string(lcd, buffer);
}

/**
 * @brief Wyświetla liczbę zmiennoprzecinkową z zadaną precyzją po przecinku.
 * @param lcd      Wskaźnik do struktury Lcd_HandleTypeDef.
 * @param value    Wartość typu float do wyświetlenia.
 * @param decimals Liczba cyfr po przecinku.
 */
void Lcd_printFloat(Lcd_HandleTypeDef* lcd, float value, uint8_t decimals)
{
    // Obsługa znaku
    if (value < 0.0f)
    {
        Lcd_string(lcd, "-");
        value = -value;
    }

    // Część całkowita
    int32_t intPart = (int32_t)value;
    char buffer[16];
    sprintf(buffer, "%ld", (long)intPart);
    Lcd_string(lcd, buffer);

    // Separator (kropka)
    Lcd_string(lcd, ".");

    // Część ułamkowa
    float fractional = value - (float)intPart;
    while (decimals--)
    {
        fractional *= 10.0f;
    }
    int32_t fracPart = (int32_t)(fractional + 0.5f); // Zaokrąglenie
    sprintf(buffer, "%ld", (long)fracPart);
    Lcd_string(lcd, buffer);
}

/**
 * @brief Wypisuje łańcuch znaków na LCD od bieżącej pozycji kursora.
 * @param lcd    Wskaźnik do struktury Lcd_HandleTypeDef.
 * @param string Łańcuch znaków zakończony '\0'.
 */
void Lcd_string(Lcd_HandleTypeDef * lcd, char * string)
{
    for (uint8_t i = 0; i < strlen(string); i++)
    {
        lcd_write_data(lcd, string[i]);
    }
}

/**
 * @brief Ustawia kursor w określonym wierszu i kolumnie.
 * @param lcd Wskaźnik do struktury Lcd_HandleTypeDef.
 * @param row Numer wiersza (0-based).
 * @param col Numer kolumny (0-based).
 */
void Lcd_cursor(Lcd_HandleTypeDef * lcd, uint8_t row, uint8_t col)
{
#ifdef LCD20xN
    lcd_write_command(lcd, SET_DDRAM_ADDR + ROW_20[row] + col);
#endif

#ifdef LCD16xN
    lcd_write_command(lcd, SET_DDRAM_ADDR + ROW_16[row] + col);
#endif
}

/**
 * @brief Czyści wyświetlacz LCD.
 * @param lcd Wskaźnik do struktury Lcd_HandleTypeDef.
 */
void Lcd_clear(Lcd_HandleTypeDef * lcd)
{
    lcd_write_command(lcd, CLEAR_DISPLAY);
}

/**
 * @brief Definiuje własny znak w CGRAM na podstawie tablicy 8 bajtów.
 * @param lcd    Wskaźnik do struktury Lcd_HandleTypeDef.
 * @param code   Kod znaku (0..7).
 * @param bitmap Tablica bajtów opisująca kształt.
 */
void Lcd_define_char(Lcd_HandleTypeDef * lcd, uint8_t code, uint8_t bitmap[])
{
    lcd_write_command(lcd, SETCGRAM_ADDR + (code << 3));
    for (uint8_t i = 0; i < 8; ++i)
    {
        lcd_write_data(lcd, bitmap[i]);
    }
}


/* ======================== Implementacje funkcji statycznych ======================== */

/**
 * @brief Zapis bajtu do rejestru poleceń LCD (RS=0).
 * @param lcd     Wskaźnik do struktury Lcd_HandleTypeDef.
 * @param command Wartość polecenia.
 */
static void lcd_write_command(Lcd_HandleTypeDef * lcd, uint8_t command)
{
    HAL_GPIO_WritePin(lcd->rs_port, lcd->rs_pin, LCD_COMMAND_REG);

    if (lcd->mode == LCD_4_BIT_MODE)
    {
        lcd_write(lcd, (command >> 4), LCD_NIB);
        lcd_write(lcd, command & 0x0F, LCD_NIB);
    }
    else
    {
        lcd_write(lcd, command, LCD_BYTE);
    }
}

/**
 * @brief Zapis bajtu do rejestru danych LCD (RS=1).
 * @param lcd  Wskaźnik do struktury Lcd_HandleTypeDef.
 * @param data Bajt danych do wyświetlenia.
 */
static void lcd_write_data(Lcd_HandleTypeDef * lcd, uint8_t data)
{
    HAL_GPIO_WritePin(lcd->rs_port, lcd->rs_pin, LCD_DATA_REG);

    if (lcd->mode == LCD_4_BIT_MODE)
    {
        lcd_write(lcd, data >> 4, LCD_NIB);
        lcd_write(lcd, data & 0x0F, LCD_NIB);
    }
    else
    {
        lcd_write(lcd, data, LCD_BYTE);
    }
}

/**
 * @brief Wystawia określoną liczbę bitów na linie danych i generuje impuls na linii EN.
 * @param lcd  Wskaźnik do struktury Lcd_HandleTypeDef.
 * @param data Dane do wystawienia.
 * @param len  Długość (4 lub 8 bitów).
 */
static void lcd_write(Lcd_HandleTypeDef * lcd, uint8_t data, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++)
    {
        HAL_GPIO_WritePin(lcd->data_port[i], lcd->data_pin[i], (data >> i) & 0x01);
    }

    HAL_GPIO_WritePin(lcd->en_port, lcd->en_pin, 1);
    DELAY(1);
    HAL_GPIO_WritePin(lcd->en_port, lcd->en_pin, 0); // Zapis danych przy opadającym zboczu
}
