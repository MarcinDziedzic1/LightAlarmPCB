// This code was performed by: Marcin Dziedzic
// alarm.h

#ifndef INC_ALARM_H_
#define INC_ALARM_H_

/**
 * @brief Sprawdza, czy alarm został wyzwolony na podstawie aktualnego czasu.
 * @param rtc_info  Wskaźnik do struktury z aktualnym czasem.
 */
void CheckAlarmTrigger(const RTC_TimeTypeDef *rtc_info);

/**
 * @brief Funkcja przygotowawcza dla alarmu (ustawienia domyślne itp.).
 */
void AlarmPreSet(void);

#endif /* INC_ALARM_H_ */
