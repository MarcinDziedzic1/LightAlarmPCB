// Created by: Marcin Dziedzic
// alarm.c

#include "menu.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "light_sen.h"
#include "fade.h"
#include "menu_state_handlers.h"
#include "lcd.h"

// Zewnętrzne deklaracje timerów, wyświetlacza, i2c
extern TIM_HandleTypeDef htim3;
extern Lcd_HandleTypeDef lcd;
extern I2C_HandleTypeDef hi2c1;

// Zmienne globalne
extern int l_BulbOnOff;         // 1 = ON, 2 = OFF
extern int8_t currentSubMenuIndex;

bool alarmIsActive = false;
bool skipLamp = false; // false = włączymy lampę, true = pominiemy ją (jest jasno)

/**
 * @brief Funkcja zakłada, że bieżący czas i struktura alarmData są w tym samym dniu.
 *        Zwraca dodatnią wartość, jeśli alarm w przyszłości,
 *        0 jeśli aktualna sekunda == alarm sekunda,
 *        ujemną jeśli alarm już minął.
 */
int TimeDiffSec(const RTC_TimeTypeDef *now, const AlarmData *a)
{
    // Zamiana HH:MM:SS na całkowitą liczbę sekund od północy
    int nowSec   = now->hours   * 3600 + now->minutes   * 60 + now->seconds;
    int alarmSec = a->hour      * 3600 + a->minute      * 60 + a->second;
    return alarmSec - nowSec;
}

/**
 * @brief Sprawdza, czy aktualny czas RTC zgadza się z ustawionym alarmem.
 *        Jeśli tak – ustawia stan ALARM_TRIGGERED.
 */
void CheckAlarmTrigger(const RTC_TimeTypeDef *rtc_info)
{
    extern bool alarmIsActive;
    extern int  lightSensorMode; // 1=ON (czujnik używany), 2=OFF (ignoruj czujnik)
    extern bool skipLamp;

    // Jeśli alarm już aktywny, nic nie robimy
    if (alarmIsActive) return;

    // Sprawdzamy, czy dzień/miesiąc/rok zgadza się z ustawieniami alarmu
    if ((rtc_info->day   == alarmData.day) &&
        (rtc_info->month == alarmData.month) &&
        (rtc_info->year  == alarmData.year))
    {
        // Różnica (w sekundach) między aktualnym czasem a alarmem
        int diff = TimeDiffSec(rtc_info, &alarmData);

        // Jeśli włączony czujnik światła i do alarmu zostało 15 sek, mierzymy natężenie światła
        if (lightSensorMode == 1 && diff == 15)
        {
            uint16_t lux = LightSen_ReadLux(&hi2c1);
            skipLamp = (lux > 100) ? true : false;
        }

        // Jeśli diff == 0 -> czas alarmu
        if (diff == 0)
        {
            gState = ALARM_TRIGGERED;
            alarmIsActive = true;
        }
    }
}

/**
 * @brief Funkcja ustawiająca domyślne parametry alarmu.
 */
void AlarmPreSet(void)
{
    RTC_TimeTypeDef now;
    RTC_ReadTime(&now);

    // Ustawiamy alarm na dzisiejszą datę, godzina 12:30:00
    alarmData.day    = now.day;
    alarmData.month  = now.month;
    alarmData.year   = now.year;
    alarmData.hour   = 12;
    alarmData.minute = 30;
    alarmData.second = 0;

    // Można ustawić dzień tygodnia, jeśli RTC go przechowuje
    alarmData.weekday = now.weekday;
}
