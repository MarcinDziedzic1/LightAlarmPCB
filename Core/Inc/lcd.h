// Created by: Marcin Dziedzic
// lcd.h

#ifndef LCD_H_
#define LCD_H_

#include "stm32f1xx_hal.h"
#include "string.h"
#include "stdio.h"
#include "main.h"

/**
 * @brief Wybór rozmiaru wyświetlacza:
 *
 */
#define LCD16xN

/**
 * @brief Adresy początków wierszy dla różnych szerokości LCD.
 */
extern const uint8_t ROW_16[];

/**
 * @brief Rejestry i polecenia LCD.
 */
#define CLEAR_DISPLAY             0x01  /**< Wyczyść wyświetlacz */
#define RETURN_HOME               0x02  /**< Ustaw kursor w pozycji domowej */

#define ENTRY_MODE_SET            0x04
#define OPT_S                     0x01  /**< Przesunięcie całego wyświetlacza w prawo */
#define OPT_INC                   0x02  /**< Przesunięcie kursora w prawo (inkrementacja) */

#define DISPLAY_ON_OFF_CONTROL    0x08
#define OPT_D                     0x04  /**< Włączenie wyświetlacza */
#define OPT_C                     0x02  /**< Włączenie kursora */
#define OPT_B                     0x01  /**< Włączenie migania kursora */

#define CURSOR_DISPLAY_SHIFT      0x10  /**< Polecenie przesuwania kursora/ekranu */
#define OPT_SC                    0x08  /**< Przesunięcie całego wyświetlacza */
#define OPT_RL                    0x04  /**< Kierunek przesunięcia (prawo/lewo) */

#define FUNCTION_SET              0x20
#define OPT_DL                    0x10  /**< Ustawienie długości interfejsu danych (4/8 bitów) */
#define OPT_N                     0x08  /**< Ustawienie liczby wierszy wyświetlacza */
#define OPT_F                     0x04  /**< Wybór czcionki (matrycy znaków) */

#define SETCGRAM_ADDR             0x040 /**< Ustawienie adresu pamięci CGRAM */
#define SET_DDRAM_ADDR            0x80  /**< Ustawienie adresu pamięci DDRAM */

/**
 * @brief Makro do opóźnień bazujące na HAL (milisekundy).
 */
#define DELAY(X) HAL_Delay(X)

/**
 * @brief Tryb pracy bitów i rejestrów.
 */
#define LCD_NIB       4
#define LCD_BYTE      8
#define LCD_DATA_REG  1
#define LCD_COMMAND_REG 0

/**
 * @brief Skróty do definicji portów i pinów LCD.
 */
#define Lcd_PortType  GPIO_TypeDef*
#define Lcd_PinType   uint16_t

/**
 * @brief Tryb pracy LCD (4-bitowy lub 8-bitowy).
 */
typedef enum {
    LCD_4_BIT_MODE,
    LCD_8_BIT_MODE
} Lcd_ModeTypeDef;

/**
 * @brief Struktura opisująca uchwyty portów i pinów dla magistrali LCD.
 */
typedef struct {
    /**
     * @brief Tablica portów dla poszczególnych linii danych.
     */
    Lcd_PortType * data_port;
    /**
     * @brief Tablica pinów dla poszczególnych linii danych.
     */
    Lcd_PinType  * data_pin;

    /**
     * @brief Port i pin rejestru RS.
     */
    Lcd_PortType rs_port;
    Lcd_PinType  rs_pin;

    /**
     * @brief Port i pin linii EN (Enable).
     */
    Lcd_PortType en_port;
    Lcd_PinType  en_pin;

    /**
     * @brief Tryb pracy: 4- lub 8-bit.
     */
    Lcd_ModeTypeDef mode;
} Lcd_HandleTypeDef;

/**
 * @brief Inicjalizacja wyświetlacza LCD.
 * @param lcd Wskaźnik do struktury Lcd_HandleTypeDef.
 */
void Lcd_init(Lcd_HandleTypeDef * lcd);

/**
 * @brief Wyświetlenie liczby całkowitej.
 * @param lcd    Wskaźnik do struktury LCD.
 * @param number Liczba do wyświetlenia.
 */
void Lcd_int(Lcd_HandleTypeDef * lcd, int number);

/**
 * @brief Wyświetlenie liczby zmiennoprzecinkowej z zadaną liczbą miejsc po przecinku.
 * @param lcd      Wskaźnik do struktury LCD.
 * @param number   Liczba typu float do wyświetlenia.
 * @param decimals Liczba cyfr po przecinku.
 */
void Lcd_printFloat(Lcd_HandleTypeDef * lcd, float number, uint8_t decimals);

/**
 * @brief Wyświetlenie łańcucha znaków (string) na LCD.
 * @param lcd     Wskaźnik do struktury LCD.
 * @param string  Łańcuch znaków zakończony znakiem '\0'.
 */
void Lcd_string(Lcd_HandleTypeDef * lcd, char * string);

/**
 * @brief Ustawienie kursora na określonym wierszu i kolumnie.
 * @param lcd  Wskaźnik do struktury LCD.
 * @param row  Numer wiersza (0-based).
 * @param col  Numer kolumny (0-based).
 */
void Lcd_cursor(Lcd_HandleTypeDef * lcd, uint8_t row, uint8_t col);

/**
 * @brief Tworzenie obiektu (uchwytu) LCD w trybie 4- lub 8-bitowym.
 * @param port[]   Tablica portów dla linii danych (4 lub 8 elementów).
 * @param pin[]    Tablica pinów dla linii danych (4 lub 8 elementów).
 * @param rs_port  Port linii RS.
 * @param rs_pin   Pin linii RS.
 * @param en_port  Port linii Enable.
 * @param en_pin   Pin linii Enable.
 * @param mode     Tryb pracy (LCD_4_BIT_MODE lub LCD_8_BIT_MODE).
 * @return         Struktura Lcd_HandleTypeDef.
 */
Lcd_HandleTypeDef Lcd_create(Lcd_PortType port[], Lcd_PinType pin[],
                             Lcd_PortType rs_port, Lcd_PinType rs_pin,
                             Lcd_PortType en_port, Lcd_PinType en_pin,
                             Lcd_ModeTypeDef mode);

/**
 * @brief Definiowanie własnego znaku w pamięci CGRAM.
 * @param lcd    Wskaźnik do struktury LCD.
 * @param code   Kod znaku (0..7).
 * @param bitmap Tablica 8 bajtów definiująca kształt znaku.
 */
void Lcd_define_char(Lcd_HandleTypeDef * lcd, uint8_t code, uint8_t bitmap[]);

/**
 * @brief Wyczyść cały wyświetlacz LCD.
 * @param lcd Wskaźnik do struktury LCD.
 */
void Lcd_clear(Lcd_HandleTypeDef * lcd);

#endif /* LCD_H_ */
