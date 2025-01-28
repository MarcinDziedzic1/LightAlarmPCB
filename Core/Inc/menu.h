// Created by: Marcin Dziedzic
// menu.h

#ifndef MENU_H
#define MENU_H

#include <stdint.h>
#include <stdbool.h>
#include "lcd.h"
#include "RTC.h"
#include "stm32f1xx_hal.h"

/**
 * @brief Enumeracja reprezentująca stany głównego menu i podmenu.
 */
typedef enum {
    MENU_STATE,            /**< Przeglądanie głównego menu */
    OPTION_STATE,          /**< Wybrano jedną z opcji "ENTER"/"EXIT" */
    SUBMENU_2,             /**< Sub-menu USB1 */
    SUBMENU_2B,            /**< Sub-menu USB2 */
    SUBMENU_L_BULB,        /**< Sub-menu lampki */
    SUBMENU_ALARM,         /**< Sub-menu: "SET", "L_SENSOR", "BACK" */
    SUBMENU_ALARM_SET,     /**< Ustawianie alarmu (dzień, miesiąc, rok, godzina, min, sek) */
    ALARM_TRIGGERED,       /**< Stan alarmu w trakcie wywołania */
    SUBMENU_ALARM_LSENSOR  /**< Obsługa czujnika światła (ON/OFF/BACK) */
} MenuState;

/**
 * @brief Struktura przechowująca dane alarmu.
 */
typedef struct {
    int8_t day;        /**< Dzień (1–31) */
    int8_t month;      /**< Miesiąc (1–12) */
    int8_t year;       /**< Rok (0–99) */
    int8_t hour;       /**< Godzina (0–23) */
    int8_t minute;     /**< Minuta (0–59) */
    int8_t second;     /**< Sekunda (0–59) */
    int8_t weekday;    /**< Dzień tygodnia (0=Sunday, ..., 6=Saturday) */
} AlarmData;

/**
 * @brief Globalna struktura przechowująca ustawienia alarmu.
 */
extern AlarmData alarmData;

/**
 * @brief Aktualny stan głównej maszyny stanów menu.
 */
extern MenuState gState;

/**
 * @brief Indeks wybranej opcji w menu głównym (0..menuCount-1).
 */
extern int8_t menuIndex;

/**
 * @brief Liczba pozycji w menu głównym (rozmiar tablicy menuItems).
 */
extern int menuCount;

/**
 * @brief Indeks w bieżącym sub-menu.
 */
extern int8_t currentSubMenuIndex;

/**
 * @brief Wybór ON/OFF dla USB (usb_OnOff): 0=brak, 1=ON, 2=OFF (oznaczone gwiazdką).
 */
extern int usb_OnOff;

/**
 * @brief Wybór ON/OFF dla USB2 (usb2_OnOff): 0=brak, 1=ON, 2=OFF.
 */
extern int usb2_OnOff;

/**
 * @brief Wybór ON/OFF (lampka): 0=brak, 1=ON, 2=OFF (oznaczone gwiazdką).
 */
extern int l_BulbOnOff;

/**
 * @brief Które pole w alarmie jest edytowane (0=day,1=month,2=year,3=hour,4=min,5=sec).
 */
extern int8_t alarmSetIndex;

/* -------------------- Deklaracje funkcji -------------------- */

/**
 * @brief Wyświetla menu alarmu: SET / L_SENSOR / BACK.
 * @param lcd       Wskaźnik do struktury LCD.
 * @param subIndex  Indeks aktualnie wybranej opcji (0..2).
 */
void DisplayAlarmMenu(Lcd_HandleTypeDef *lcd, int8_t subIndex);

/**
 * @brief Wyświetla ekran ustawiania alarmu (dzień, miesiąc, rok, godzina, min, sek).
 * @param lcd            Wskaźnik do struktury LCD.
 * @param alarmSetIndex  Indeks aktualnie edytowanej danej.
 * @param blinkOn        Czy dana ma się „mrugać” (true/false).
 */
void DisplayAlarmSet(Lcd_HandleTypeDef *lcd, int8_t alarmSetIndex, bool blinkOn);

/**
 * @brief Wyświetla komunikat ALARM! w pierwszym wierszu, oraz opcje STOP/SNOOZE w drugim.
 * @param lcd       Wskaźnik do struktury LCD.
 * @param subIndex  0=STOP, 1=SNOOZE (zaznaczone strzałką).
 */
void DisplayAlarmTriggered(Lcd_HandleTypeDef *lcd, int8_t subIndex);

/**
 * @brief Główne menu skrolowane, wyświetla 2 pozycje na ekranie i wyróżnia zaznaczoną.
 * @param lcd          Wskaźnik do struktury LCD.
 * @param index        Aktualny indeks menu (0..menuCount-1).
 * @param forceRefresh Jeśli true, wykonaj pełny odśwież ekranu (np. powrót z innego menu).
 */
void Menu_Display(Lcd_HandleTypeDef *lcd, uint8_t index, bool forceRefresh);

/**
 * @brief Wyświetla zawartość wybranej opcji menu (np. TIME, SENSOR, itp.).
 * @param lcd   Wskaźnik do struktury LCD.
 * @param index Indeks pozycji w głównym menu.
 * @param hi2c  Uchwyt do I2C (np. do sensora BH1750).
 */
void Menu_ShowOption(Lcd_HandleTypeDef *lcd, uint8_t index, I2C_HandleTypeDef *hi2c);

/**
 * @brief Wyświetla sub-menu ON/OFF/BACK, z podświetlaniem wybranej opcji strzałką.
 * @param lcd          Wskaźnik do struktury LCD.
 * @param subIndex     0=ON, 1=OFF, 2=BACK
 * @param device_OnOff Bieżący stan urządzenia (1=ON, 2=OFF).
 */
void DisplaySubMenuON_OFF(Lcd_HandleTypeDef *lcd, int8_t subIndex, int device_OnOff);

#endif /* MENU_H */
