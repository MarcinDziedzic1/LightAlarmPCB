#include "RTC.h"
#include <stdio.h> // opcjonalnie do debug/printf

/*
   PCF85063AT (obudowa SO8) ma 7-bitowy adres 0x51.
   W zapisie do funkcji HAL_I2C:
    - write = (0x51 << 1) + 0 = 0xA2
    - read  = (0x51 << 1) + 1 = 0xA3
*/
#define PCF85063A_WRITE_ADDR   0xA2
#define PCF85063A_READ_ADDR    0xA3

/* Używane rejestry wg dokumentacji:
   - 0x04 => Seconds (BCD, bit7=OS)
   - 0x05 => Minutes
   - 0x06 => Hours   (24h => bity [5:0])
   - 0x07 => Days
   - 0x08 => Weekdays (0..6, nie jest BCD)
   - 0x09 => Months
   - 0x0A => Years (0..99)
*/

/* Wskaźnik do uchwytu I2C */
static I2C_HandleTypeDef *rtc_i2c = NULL;

/* Funkcje pomocnicze do konwersji BCD <-> DEC */
static uint8_t bcd2dec(uint8_t bcd)
{
    return (uint8_t)((bcd >> 4) * 10 + (bcd & 0x0F));
}

static uint8_t dec2bcd(uint8_t dec)
{
    return (uint8_t)(((dec / 10) << 4) | (dec % 10));
}

/* -------------------------------------------------------
 * RTC_Init:
 *  Zapamiętuje uchwyt do I2C.
 * ------------------------------------------------------- */
void RTC_Init(I2C_HandleTypeDef *hi2c)
{
    rtc_i2c = hi2c;
}

/* -------------------------------------------------------
 * RTC_SetTime:
 *  Ustawienie czasu w rejestrach 0x04..0x0A.
 *  1) Wystarczy wywołać HAL_I2C_Mem_Write z adresem startowym = 0x04
 *  2) Podajemy 7 bajtów: [sec, min, hour, day, weekday, month, year].
 * ------------------------------------------------------- */
void RTC_SetTime(const RTC_TimeTypeDef *time)
{
    if (rtc_i2c == NULL) return;

    if (time->seconds > 59 || time->minutes > 59 || time->hours > 23 ||
           time->day < 1 || time->day > 31 || time->month < 1 || time->month > 12 ||
           time->year > 99) {
           return; // Nie ustawiaj, jeśli dane są nieprawidłowe
       }

    uint8_t buffer[7];

    // sekundy (bit7=OS=0)
    buffer[0] = dec2bcd(time->seconds) & 0x7F;
    // minuty
    buffer[1] = dec2bcd(time->minutes) & 0x7F;
    // godziny (24h => bity [5:0])
    buffer[2] = dec2bcd(time->hours) & 0x3F;
    // dzień
    buffer[3] = dec2bcd(time->day) & 0x3F;
    // dzień tygodnia (0..6) - tutaj nie jest BCD, wystarczy maskować ewentualnie do 0..7
    buffer[4] = (time->weekday & 0x07);
    // miesiąc
    buffer[5] = dec2bcd(time->month) & 0x1F;
    // rok (0..99)
    buffer[6] = dec2bcd(time->year);

    // Zapis do rejestrów 0x04..0x0A (7 bajtów)
    HAL_I2C_Mem_Write(rtc_i2c,
                      PCF85063A_WRITE_ADDR, // 0xA2
                      0x04,
                      I2C_MEMADD_SIZE_8BIT,
                      buffer,
                      7,
                      100);
}

/* -------------------------------------------------------
 * RTC_ReadTime:
 *   Zalecany przez dokumentację:
 *   1) START + (SlaveAddrWrite=0xA2)
 *   2) Wysłanie adresu rejestru=0x04
 *   3) RESTART + (SlaveAddrRead=0xA3)
 *   4) Odczyt 7 bajtów
 *   5) STOP
 * ------------------------------------------------------- */
void RTC_ReadTime(RTC_TimeTypeDef *time)
{
    if (rtc_i2c == NULL) return;

    HAL_StatusTypeDef ret;
    uint8_t regPointer[1] = {0x04};
    uint8_t buffer[7]     = {0};

    // Krok 1) i 2): START + Write(0xA2), wyślij '0x04' (adres rejestru sekundy)
    ret = HAL_I2C_Master_Transmit(rtc_i2c,
                                  PCF85063A_WRITE_ADDR,
                                  regPointer,
                                  1,
                                  100);
    if (ret != HAL_OK)
    {
        // Błąd
        return;
    }

    // Krok 3) i 4): RESTART + Read(0xA3), odczyt 7 bajtów
    ret = HAL_I2C_Master_Receive(rtc_i2c,
                                 PCF85063A_READ_ADDR,
                                 buffer,
                                 7,
                                 100);
    if (ret != HAL_OK)
    {
        // Błąd
        return;
    }
    // Krok 5): STOP generuje się automatycznie po zakończeniu transmisji.

    // Dekodowanie BCD -> DEC
    time->seconds = bcd2dec(buffer[0] & 0x7F);
    time->minutes = bcd2dec(buffer[1] & 0x7F);
    time->hours   = bcd2dec(buffer[2] & 0x3F);
    time->day     = bcd2dec(buffer[3] & 0x3F);
    time->weekday = (buffer[4] & 0x07);
    time->month   = bcd2dec(buffer[5] & 0x1F);
    time->year    = bcd2dec(buffer[6]);
}
