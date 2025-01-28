#ifndef MENU_H
#define MENU_H

#include <stdint.h>
#include <stdbool.h>
#include "lcd.h"
#include "RTC.h"
#include "stm32f1xx_hal.h"

// ----- Definicja stanów -----
typedef enum {
    MENU_STATE,             // przeglądanie głównego menu
    OPTION_STATE,           // wybrano jedną z opcji ENTER/EXIT
    SUBMENU_2, 		        // sub-menu USB1
	SUBMENU_2B,		        // sub-menu USB2
    SUBMENU_L_BULB,         // sub-menu lampki
	SUBMENU_ALARM,          // Nowy sub-menu: "SET" / "BACK"
	SUBMENU_ALARM_SET,		// sub-menu do ustawiania alarmu
	ALARM_TRIGGERED 		// menu wyświetlane przy wywołaniu alarmu
} MenuState;


typedef struct {
    int8_t day;        // Dzień (1-31)
    int8_t month;      // Miesiąc (1-12)
    int8_t year;       // Rok (0-99)
    int8_t hour;       // Godzina (0-23)
    int8_t minute;     // Minuta (0-59)
    int8_t second;     // Sekunda (0-59)
    int8_t weekday;    // Dzień tygodnia (0=Sunday, ..., 6=Saturday)
} AlarmData;

extern AlarmData alarmData; // Globalna zmienna alarmu
// ----- Zmienne globalne -----

/**
 * @brief Aktualny stan maszyny stanów.
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
 * @brief Indeks w sub-menu.
 */
extern int8_t currentSubMenuIndex;
/**
 * @brief Wybór ON/OFF (usb): 0 = brak, 1 = ON, 2 = OFF (oznaczenie gwiazdką).
 */
extern int usb_OnOff;

/**
 * @brief Wybór ON/OFF (usb2): 0 = brak, 1 = ON, 2 = OFF (oznaczenie gwiazdką).
 */
extern int usb2_OnOff;
/**
 * @brief Wybór ON/OFF (lampka): 0 = brak, 1 = ON, 2 = OFF (oznaczenie gwiazdką).
 */
extern int l_BulbOnOff;


extern int8_t alarmSetIndex;        // Które pole edytujemy (0=day,1=month,2=year,3=hour,4=min,5=sec)
// ----- Deklaracje funkcji -----


void DisplayAlarmMenu(Lcd_HandleTypeDef *lcd, int8_t subIndex);
void DisplayAlarmSet(Lcd_HandleTypeDef *lcd, int8_t alarmSetIndex, bool blinkOn);
void DisplayAlarmTriggered(Lcd_HandleTypeDef *lcd, int8_t subIndex);

/**
 * @brief Wyświetla główne menu w trybie „scrollowalnym”.
 *        Wyświetla dwie pozycje (górną i dolną) – strona zależna od menuIndex / 2.
 *        Wyróżnia aktualnie zaznaczoną pozycję (menuIndex % 2).
 *
 * @param lcd   wskaźnik do struktury Lcd_HandleTypeDef
 * @param index aktualny menuIndex
 */
void Menu_Display(Lcd_HandleTypeDef *lcd, uint8_t index,  bool forceRefresh);

/**
 * @brief Wyświetla zawartość wybranej opcji menu (np. TIME, SENSOR, SUBMENU_2, itp.).
 *
 * @param lcd   wskaźnik do LCD
 * @param index numer pozycji w menu
 * @param hi2c  wskaźnik do I2C (np. do sensora BH1750)
 */
void Menu_ShowOption(Lcd_HandleTypeDef *lcd, uint8_t index, I2C_HandleTypeDef *hi2c);

/**
 * @brief Wyświetla sub-menu: ON/OFF/BACK.
 */
void DisplaySubMenuON_OFF(Lcd_HandleTypeDef *lcd, int8_t subIndex, int device_OnOff);

#endif /* MENU_H */
