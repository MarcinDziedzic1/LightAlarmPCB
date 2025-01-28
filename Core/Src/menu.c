// Created by: Marcin Dziedzic
// menu.c

#include "menu.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "light_sen.h"
#include "fade.h"

// Uchwyt timera do fade, zadeklarowany gdzie indziej
extern TIM_HandleTypeDef htim3;

// Definicja globalnych zmiennych (bez extern)
MenuState gState = MENU_STATE;    // start w głównym menu
int8_t menuIndex = 0;
int8_t currentSubMenuIndex = 0;
int8_t alarmSetIndex = 0;         // 0=day,1=month,2=year,3=hour,4=min,5=sec
int8_t lightSensorMode = 2;       // 1=ON, 2=OFF
int8_t sensorSubIndex;

int usb_OnOff   = 1;  // 1=ON, 2=OFF
int usb2_OnOff  = 1;  // 1=ON, 2=OFF
int l_BulbOnOff = 2;  // 1=ON, 2=OFF

// Globalna zmienna dla alarmu
AlarmData alarmData = {0, 0, 0, 0, 0, 0, 0}; // Inicjalizacja na zero

// Przykładowa tablica nazw pozycji w menu
static const char *menuItems[] = {
    "TIME ",
    "ALARM",
    "USB1 ",
    "USB2 ",
    "LIGHT_BULB",
    "LIGHT_SENSOR"
};
// Liczba pozycji
int menuCount = sizeof(menuItems) / sizeof(menuItems[0]);

/**
 * @brief Wyświetla główne menu w trybie scrollowalnym (po 2 pozycje).
 */
void Menu_Display(Lcd_HandleTypeDef *lcd, uint8_t index, bool forceRefresh)
{
    // Zmienne statyczne do zapamiętania poprzednich wartości
    static uint8_t oldIndex = 255;
    static uint8_t oldPage  = 255;

    // Bufory na wiersze LCD (16 znaków + 1 terminator)
    static char row0[17];
    static char row1[17];

    // Obliczenie numeru „strony” (po 2 elementy na stronę)
    uint8_t page = index / 2;
    uint8_t firstItem  = page * 2;
    uint8_t secondItem = firstItem + 1;

    // Funkcja pomocnicza do pełnego odświeżenia
    void RefreshWholePage(void)
    {
        Lcd_clear(lcd);

        // Wypełniamy oba wiersze spacjami
        memset(row0, ' ', 16);
        memset(row1, ' ', 16);
        row0[16] = '\0';
        row1[16] = '\0';

        // Uzupełniamy row0
        if (firstItem < menuCount)
        {
            bool selected = ((index % 2) == 0);
            if (selected)
                snprintf(row0, sizeof(row0), ">%s", menuItems[firstItem]);
            else
                snprintf(row0, sizeof(row0), " %s", menuItems[firstItem]);
        }

        // Uzupełniamy row1
        if (secondItem < menuCount)
        {
            bool selected = ((index % 2) == 1);
            if (selected)
                snprintf(row1, sizeof(row1), ">%s", menuItems[secondItem]);
            else
                snprintf(row1, sizeof(row1), " %s", menuItems[secondItem]);
        }

        // Wyświetlamy
        Lcd_cursor(lcd, 0, 0);
        Lcd_string(lcd, row0);
        Lcd_cursor(lcd, 1, 0);
        Lcd_string(lcd, row1);
    }

    // 1. forceRefresh => pełne odświeżenie ekranu
    if (forceRefresh)
    {
        oldPage  = 255;
        oldIndex = 255;
    }

    // 2. Sprawdzamy, czy zmieniła się strona
    if (page != oldPage)
    {
        RefreshWholePage();
        oldPage  = page;
        oldIndex = index;
    }
    else
    {
        // Ta sama strona, ale inny element?
        if (index != oldIndex)
        {
            uint8_t oldRow = (oldIndex % 2);
            uint8_t newRow = (index % 2);

            if (oldRow != newRow)
            {
                memset(row0, ' ', 16);
                row0[16] = '\0';
                if (firstItem < menuCount)
                {
                    bool selected = (newRow == 0);
                    if (selected)
                        snprintf(row0, sizeof(row0), ">%s", menuItems[firstItem]);
                    else
                        snprintf(row0, sizeof(row0), " %s", menuItems[firstItem]);
                }
                Lcd_cursor(lcd, 0, 0);
                Lcd_string(lcd, row0);

                memset(row1, ' ', 16);
                row1[16] = '\0';
                if (secondItem < menuCount)
                {
                    bool selected = (newRow == 1);
                    if (selected)
                        snprintf(row1, sizeof(row1), ">%s", menuItems[secondItem]);
                    else
                        snprintf(row1, sizeof(row1), " %s", menuItems[secondItem]);
                }
                Lcd_cursor(lcd, 1, 0);
                Lcd_string(lcd, row1);
            }
            oldIndex = index;
        }
    }
}

/**
 * @brief Wyświetla zawartość wybranej opcji menu (TIME, ALARM, USB1, USB2, L_BULB, SENSOR).
 */
void Menu_ShowOption(Lcd_HandleTypeDef *lcd, uint8_t index, I2C_HandleTypeDef *hi2c)
{
    switch (index)
    {
    case 0: // TIME
    {
        RTC_TimeTypeDef now;
        RTC_ReadTime(&now);

        char buf[17];
        snprintf(buf, sizeof(buf), "%02d/%02d/%04d", now.day, now.month, (now.year + 2000));
        Lcd_cursor(lcd, 0, 0);
        Lcd_string(lcd, buf);

        snprintf(buf, sizeof(buf), "%02d:%02d:%02d", now.hours, now.minutes, now.seconds);
        Lcd_cursor(lcd, 1, 0);
        Lcd_string(lcd, buf);
        break;
    }
    case 1: // ALARM
    {
        currentSubMenuIndex = 0;
        gState = SUBMENU_ALARM;
        DisplayAlarmMenu(lcd, currentSubMenuIndex);
        break;
    }
    case 2: // USB1 => sub-menu
    {
        currentSubMenuIndex = 0;
        gState = SUBMENU_2;
        DisplaySubMenuON_OFF(lcd, currentSubMenuIndex, usb_OnOff);
        break;
    }
    case 3: // USB2 => sub-menu
    {
        currentSubMenuIndex = 0;
        gState = SUBMENU_2B;
        DisplaySubMenuON_OFF(lcd, currentSubMenuIndex, usb2_OnOff);
        break;
    }
    case 4: // L_BULB
    {
        currentSubMenuIndex = 0;
        gState = SUBMENU_L_BULB;
        DisplaySubMenuON_OFF(lcd, currentSubMenuIndex, l_BulbOnOff);
        break;
    }
    case 5: // SENSOR
    {
        uint16_t lux = LightSen_ReadLux(hi2c);
        LightSen_DisplayLux(lcd, lux);
        break;
    }
    default:
        break;
    }
}

/**
 * @brief Wyświetla sub-menu typu ON/OFF/BACK z wyróżnieniem opcji.
 */
void DisplaySubMenuON_OFF(Lcd_HandleTypeDef *lcd, int8_t subIndex, int device_OnOff)
{
    char row0[17];
    char row1[17];
    memset(row0, ' ', 16);
    memset(row1, ' ', 16);
    row0[16] = '\0';
    row1[16] = '\0';

    // ON
    char onLabel[8];
    if (subIndex == 0)
        snprintf(onLabel, sizeof(onLabel), ">ON ");
    else
        snprintf(onLabel, sizeof(onLabel), " ON ");
    if (device_OnOff == 1)
    {
        int len = strlen(onLabel);
        if (len < 6)
        {
            onLabel[len - 1] = '*';
        }
    }

    // OFF
    char offLabel[8];
    if (subIndex == 1)
        snprintf(offLabel, sizeof(offLabel), ">OFF ");
    else
        snprintf(offLabel, sizeof(offLabel), " OFF ");
    if (device_OnOff == 2)
    {
        int len = strlen(offLabel);
        if (len < 6)
        {
            offLabel[len - 1] = '*';
        }
    }

    // Pierwszy wiersz: ON i OFF
    strncpy(&row0[0], onLabel, strlen(onLabel));
    strncpy(&row0[6], offLabel, strlen(offLabel));

    // BACK
    char backLabel[8];
    if (subIndex == 2)
        snprintf(backLabel, sizeof(backLabel), ">BACK");
    else
        snprintf(backLabel, sizeof(backLabel), " BACK");

    strncpy(&row1[0], backLabel, strlen(backLabel));

    Lcd_cursor(lcd, 0, 0);
    Lcd_string(lcd, row0);
    Lcd_cursor(lcd, 1, 0);
    Lcd_string(lcd, row1);
}

/**
 * @brief Wyświetla menu alarmu (SET / L_Sensor / BACK).
 */
void DisplayAlarmMenu(Lcd_HandleTypeDef *lcd, int8_t subIndex)
{
    char row0[17];
    char row1[17];
    memset(row0, ' ', 16);
    memset(row1, ' ', 16);
    row0[16] = '\0';
    row1[16] = '\0';

    // subIndex = 0 => >SET    LSensor
    // subIndex = 1 =>  SET   >LSensor
    // subIndex = 2 =>  SET    LSensor, dolny wiersz => >BACK

    if (subIndex == 0)
    {
        snprintf(row0, sizeof(row0), ">SET    LSensor");
    }
    else if (subIndex == 1)
    {
        snprintf(row0, sizeof(row0), " SET   >LSensor");
    }
    else
    {
        snprintf(row0, sizeof(row0), " SET    LSensor");
    }

    // Dolny wiersz
    if (subIndex == 2)
    {
        snprintf(row1, sizeof(row1), ">BACK   ");
    }
    else
    {
        snprintf(row1, sizeof(row1), " BACK   ");
    }

    Lcd_cursor(lcd, 0, 0);
    Lcd_string(lcd, row0);
    Lcd_cursor(lcd, 1, 0);
    Lcd_string(lcd, row1);
}

/**
 * @brief Wyświetla ekran do ustawiania alarmu (DD/MM/YYYY i HH:MM:SS).
 */
void DisplayAlarmSet(Lcd_HandleTypeDef *lcd, int8_t setIndex, bool blinkOn)
{
    char row0[17];
    snprintf(row0, sizeof(row0), "%02d/%02d/%04d      ",
             alarmData.day, alarmData.month, 2000 + alarmData.year);

    char row1[17];
    snprintf(row1, sizeof(row1), "%02d:%02d:%02d",
             alarmData.hour, alarmData.minute, alarmData.second);

    // Mruganie (jeśli blinkOn = false, ukrywamy aktualnie edytowaną wartość)
    if (!blinkOn)
    {
        switch (setIndex)
        {
        case 0: // day
            row0[0] = ' ';
            row0[1] = ' ';
            break;
        case 1: // month
            row0[3] = ' ';
            row0[4] = ' ';
            break;
        case 2: // year
            row0[6] = ' ';
            row0[7] = ' ';
            row0[8] = ' ';
            row0[9] = ' ';
            break;
        case 3: // hour
            row1[0] = ' ';
            row1[1] = ' ';
            break;
        case 4: // minute
            row1[3] = ' ';
            row1[4] = ' ';
            break;
        case 5: // second
            row1[6] = ' ';
            row1[7] = ' ';
            break;
        }
    }

    Lcd_cursor(lcd, 0, 0);
    Lcd_string(lcd, row0);
    Lcd_cursor(lcd, 1, 0);
    Lcd_string(lcd, row1);
}

/**
 * @brief Wyświetla komunikat ALARM! (wiersz 0) oraz STOP/SNOOZE (wiersz 1).
 */
void DisplayAlarmTriggered(Lcd_HandleTypeDef *lcd, int8_t subIndex)
{
    char row0[17];
    char row1[17];
    memset(row0, ' ', 16);
    memset(row1, ' ', 16);
    row0[16] = '\0';
    row1[16] = '\0';

    snprintf(row0 + 5, sizeof(row0) - 5, "ALARM!");

    if (subIndex == 0)
    {
        snprintf(row1, sizeof(row1), ">STOP   SNOOZE");
    }
    else
    {
        snprintf(row1, sizeof(row1), " STOP  >SNOOZE");
    }

    Lcd_cursor(lcd, 0, 0);
    Lcd_string(lcd, row0);
    Lcd_cursor(lcd, 1, 0);
    Lcd_string(lcd, row1);
}
