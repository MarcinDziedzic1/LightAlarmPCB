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

extern int l_BulbOnOff;  // 1 = ON, 2 = OFF
extern int8_t currentSubMenuIndex;

bool alarmIsActive = false;
/**
 * @brief Sprawdza, czy aktualny czas RTC zgadza się z ustawionym alarmem.
 *        Jeśli tak – włącza lampkę (fade in).
 */


void CheckAlarmTrigger(const RTC_TimeTypeDef *rtc_info)
{
    // Sprawdzamy, czy alarm nie został już wyzwolony
    if (!alarmIsActive)
    {
        if ((rtc_info->day     == alarmData.day)    &&
            (rtc_info->month   == alarmData.month)  &&
            (rtc_info->year    == alarmData.year)   &&
            (rtc_info->hours   == alarmData.hour)   &&
            (rtc_info->minutes == alarmData.minute) &&
            (rtc_info->seconds == alarmData.second))
        {
            // Alarm wyzwolony!
            gState = ALARM_TRIGGERED;   // Przechodzimy do obsługi alarmu
            alarmIsActive = true;       // Zaznaczamy, że alarm jest już aktywny
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
