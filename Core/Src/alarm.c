#include "menu.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "light_sen.h"
#include "fade.h"
#include "menu_state_handlers.h"
#include "lcd.h"

extern TIM_HandleTypeDef htim3;
extern Lcd_HandleTypeDef lcd;
extern I2C_HandleTypeDef hi2c1;


extern int l_BulbOnOff;  // 1 = ON, 2 = OFF
extern int8_t currentSubMenuIndex;

bool alarmIsActive = false;
bool skipLamp = false;   // false = domyślnie włączymy lampę,
                         // true = jest jasno => nie włączamy.

// Funkcja, która zakłada, że now i alarmData
// są w tym samym dniu/miesiącu/roku.
// Zwraca dodatnią wartość, jeśli alarm w przyszłości,
// 0 jeśli aktualna sekunda == alarm sekunda,
// ujemną jeśli alarm już minął.
int TimeDiffSec(const RTC_TimeTypeDef *now, const AlarmData *a)
{
    // Zamieniamy HH:MM:SS na liczbę sekund od północy
    int nowSec   = now->hours   * 3600 + now->minutes   * 60 + now->seconds;
    int alarmSec = a->hour      * 3600 + a->minute      * 60 + a->second;

    return alarmSec - nowSec;
}
/**
 * @brief Sprawdza, czy aktualny czas RTC zgadza się z ustawionym alarmem.
 *        Jeśli tak – włącza lampkę (fade in).
 */


void CheckAlarmTrigger(const RTC_TimeTypeDef *rtc_info)
{
    extern bool alarmIsActive;
    extern int  lightSensorMode;  // 1=ON (czujnik używany), 2=OFF (ignoruj czujnik)
    extern bool skipLamp;         // flaga: czy pominąć włączenie lampki

    // Jeśli alarm już wyzwolony, to nic nie sprawdzaj
    if (alarmIsActive) return;

    // Sprawdzaj, czy day/month/year są takie same jak w alarmData
    if ((rtc_info->day   == alarmData.day)   &&
        (rtc_info->month == alarmData.month) &&
        (rtc_info->year  == alarmData.year))
    {
        // Obliczamy różnicę czasu (sekund) do alarmu w obrębie bieżącej doby
        int diff = TimeDiffSec(rtc_info, &alarmData);

        // 1. Jeśli włączony czujnik światła i do alarmu zostało 15 sek
        //    => czytamy czujnik i ustawiamy skipLamp w zależności od lux
        if (lightSensorMode == 1 && diff == 15)
        {
            uint16_t lux = LightSen_ReadLux(&hi2c1); // przykładowy odczyt
            if (lux > 100)
            {
                // jest wystarczająco jasno => pomijamy lampę
                skipLamp = true;
            }
            else
            {
                skipLamp = false;
            }
        }

        // 2. Jeśli diff == 0 => czas alarmu
        if (diff == 0)
        {
            // Alarm wyzwolony
            gState = ALARM_TRIGGERED;
            alarmIsActive = true;

            // Nie ustawiamy tu lampy, bo zrobimy to w HandleAlarmTriggered,
            // biorąc pod uwagę skipLamp.
        }
    }
}


void AlarmPreSet(void)
{
    // Odczyt aktualnego czasu z RTC
    RTC_TimeTypeDef now;
    RTC_ReadTime(&now);

    // Ustawiamy alarm na dzisiejszą datę i określoną godzinę
    alarmData.day = now.day;
    alarmData.month = now.month;
    alarmData.year = now.year; // np. 25 oznacza rok 2025 (zależnie od RTC)

    alarmData.hour = 12;
    alarmData.minute = 30;
    alarmData.second = 0;

    // Możemy ustawić dzień tygodnia, jeśli potrzebujemy go do logiki
    alarmData.weekday = now.weekday; // Jeśli RTC udostępnia dzień tygodnia
}
