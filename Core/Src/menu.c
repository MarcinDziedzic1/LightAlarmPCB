#include "menu.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "light_sen.h"
#include "fade.h"

// Uchwyt timera do fade, zadeklarowany gdzie indziej
extern TIM_HandleTypeDef htim3;

// Definicja globalnych zmiennych (bez extern)
MenuState gState = MENU_STATE;  // start w głównym menu
int8_t menuIndex = 0;
int8_t subMenu2Index = 0;
int8_t subMenu2BIndex = 0;
int8_t subMenuLBIndex = 0;
int8_t subMenuAlarmIndex = 0;  // 0=SET, 1=BACK
int8_t alarmSetIndex = 0;      // 0=day,1=month,2=year,3=hour,4=min,5=sec

int usb_OnOff   = 1;  // 1=ON, 2=OFF
int usb2_OnOff   = 1;  // 1=ON, 2=OFF
int l_BulbOnOff = 2;  // 1=ON, 2=OFF

// Globalna zmienna dla alarmu
AlarmData alarmData = {0, 0, 0, 0, 0, 0, 0}; // Inicjalizacja na zero

// Przykładowa tablica nazw pozycji w menu
static const char *menuItems[] = {
    "TIME ",     // index=0
    "ALARM",    // index=1
    "USB1 ",     // index=2
    "USB2 ",     // index=3
    "LIGHT_BULB",    // index=4
    "LIGHT_SENSOR"     // index=5
    // Możesz dodać więcej elementów
};
// Liczba pozycji
int menuCount = sizeof(menuItems) / sizeof(menuItems[0]);

/**
 * @brief Wyświetla główne menu w trybie scrollowalnym (2 wiersze naraz).
 *        Wyróżnia jedną z tych dwóch pozycji zależnie od (index % 2).
 */
void Menu_Display(Lcd_HandleTypeDef *lcd, uint8_t index)
{
    Lcd_clear(lcd);

    // Każda „strona” ma 2 elementy
    // Strona = index / 2
    uint8_t page = index / 2;
    uint8_t firstItem  = page * 2;
    uint8_t secondItem = firstItem + 1;

    // Bufory na 2 wiersze
    char row0[17];
    char row1[17];
    memset(row0, ' ', 16);
    memset(row1, ' ', 16);
    row0[16] = '\0';
    row1[16] = '\0';

    // Wiersz 0 (firstItem)
    if (firstItem < menuCount)
    {
        bool selected = ((index % 2) == 0);
        if (selected)
            snprintf(row0, sizeof(row0), ">%s", menuItems[firstItem]);
        else
            snprintf(row0, sizeof(row0), " %s", menuItems[firstItem]);
    }

    // Wiersz 1 (secondItem)
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

/**
 * @brief Wyświetla zawartość wybranej opcji menu
 *        (np. TIME, SENSOR, USB1 => subMenu2, L_BULB => subMenuLB, itd.).
 */
void Menu_ShowOption(Lcd_HandleTypeDef *lcd, uint8_t index, I2C_HandleTypeDef *hi2c)
{
    // Czyścimy LCD
    Lcd_clear(lcd);

    // Tu decydujemy, co się stanie dla danej pozycji w menu.
    switch (index)
    {
    case 0: // TIME
    {
        // Pobierz aktualny czas z RTC i wyświetl
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
    	subMenuAlarmIndex = 0;
    	gState = SUBMENU_ALARM;
    	DisplayAlarmMenu(lcd, subMenuAlarmIndex);
    	break;
    }
    case 2: // USB1 => sub-menu
    {
        subMenu2Index = 0;
        gState = SUBMENU_2;
        DisplaySubMenu2(lcd, subMenu2Index, usb_OnOff);
        break;
    }
    case 3: // USB2 => sub-menuB
    {
    	subMenu2BIndex = 0;
    	gState = SUBMENU_2B;
        DisplaySubMenu2B(lcd, subMenu2BIndex, usb2_OnOff);
    	break;
    }
    case 4:
    {
    	subMenuLBIndex = 0;
    	gState = SUBMENU_L_BULB;
    	DisplayLBulbMenu(lcd, subMenuLBIndex, l_BulbOnOff);
    	break;
    }
    case 5: // SENSOR
    {
    	// Odczyt sensora BH1750
        uint16_t lux = LightSen_ReadLux(hi2c);
    	LightSen_DisplayLux(lcd, lux);
    	break;

    }
    default:
        // Nic
        break;
    }
}

/**
 * @brief Wyświetla sub-menu USB1 (ON/OFF/BACK).
 */
void DisplaySubMenu2(Lcd_HandleTypeDef *lcd, int8_t subIndex, int usb_OnOff)
{
    Lcd_clear(lcd);

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
    if (usb_OnOff == 1)
    {
        int len = strlen(onLabel);
        if (len < 6)
            onLabel[len - 1] = '*';
    }

    // OFF
    char offLabel[8];
    if (subIndex == 1)
        snprintf(offLabel, sizeof(offLabel), ">OFF ");
    else
        snprintf(offLabel, sizeof(offLabel), " OFF ");
    if (usb_OnOff == 2)
    {
        int len = strlen(offLabel);
        if (len < 6)
            offLabel[len - 1] = '*';
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

void DisplaySubMenu2B(Lcd_HandleTypeDef *lcd, int8_t subIndex, int usb2_OnOff)
{
    Lcd_clear(lcd);

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
    if (usb2_OnOff == 1)
    {
        int len = strlen(onLabel);
        if (len < 6)
            onLabel[len - 1] = '*';
    }

    // OFF
    char offLabel[8];
    if (subIndex == 1)
        snprintf(offLabel, sizeof(offLabel), ">OFF ");
    else
        snprintf(offLabel, sizeof(offLabel), " OFF ");
    if (usb2_OnOff == 2)
    {
        int len = strlen(offLabel);
        if (len < 6)
            offLabel[len - 1] = '*';
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
 * @brief Wyświetla sub-menu lampki (ON/OFF/BACK).
 */
void DisplayLBulbMenu(Lcd_HandleTypeDef *lcd, int8_t subIndex, int l_BulbOnOff)
{
    Lcd_clear(lcd);

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
    if (l_BulbOnOff == 1)
    {
        int len = strlen(onLabel);
        if (len < 6)
            onLabel[len - 1] = '*';
    }

    // OFF
    char offLabel[8];
    if (subIndex == 1)
        snprintf(offLabel, sizeof(offLabel), ">OFF ");
    else
        snprintf(offLabel, sizeof(offLabel), " OFF ");
    if (l_BulbOnOff == 2)
    {
        int len = strlen(offLabel);
        if (len < 6)
            offLabel[len - 1] = '*';
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
void AlarmSet(void)
{
    // Odczyt aktualnego czasu z RTC
    RTC_TimeTypeDef now;
    RTC_ReadTime(&now);

    // Ustawiamy alarm na dzisiejszą datę i określoną godzinę
    alarmData.day = now.day;
    alarmData.month = now.month;
    alarmData.year = now.year; // np. 25 oznacza rok 2025 (zależnie od RTC)

    // Przykładowy czas alarmu: 19:58:00
    alarmData.hour = 19;
    alarmData.minute = 58;
    alarmData.second = 0;

    // Możemy ustawić dzień tygodnia, jeśli potrzebujemy go do logiki
    alarmData.weekday = now.weekday; // Jeśli RTC udostępnia dzień tygodnia
}
void DisplayAlarmMenu(Lcd_HandleTypeDef *lcd, int8_t subIndex)
{
//	RTC_TimeTypeDef now;
//	RTC_ReadTime(&now);
//
//	// Ustawiamy alarm na "dzisiejszą" datę, godz. 12:30:00
//	alarmData.day   = now.day;
//	alarmData.month = now.month;
//	alarmData.year  = now.year; // np. 25 oznacza 2025, zależnie od RTC
//	alarmData.hour  = 19;
//	alarmData.minute   = 58;
//	alarmData.second   = 0;

	// Wyświetlamy np. w pierwszym wierszu SET, w drugim BACK
    Lcd_clear(lcd);

    char row0[17];
    char row1[17];
    memset(row0, ' ', 16);
    memset(row1, ' ', 16);
    row0[16] = '\0';
    row1[16] = '\0';

    // SET
    if (subIndex == 0)
        snprintf(row0, sizeof(row0), ">SET ");
    else
        snprintf(row0, sizeof(row0), " SET ");

    // BACK
    if (subIndex == 1)
        snprintf(row1, sizeof(row1), ">BACK");
    else
        snprintf(row1, sizeof(row1), " BACK");

    Lcd_cursor(lcd, 0, 0);
    Lcd_string(lcd, row0);
    Lcd_cursor(lcd, 1, 0);
    Lcd_string(lcd, row1);
}

void DisplayAlarmSet(Lcd_HandleTypeDef *lcd, int8_t setIndex, bool blinkOn)
{
    // Indeksy: 0=day,1=month,2=year,3=hour,4=min,5=sec

    Lcd_clear(lcd);

    // Przygotowujemy stringi
    // Górny: "DD/MM/YYYY"
    char row0[17];
    snprintf(row0, sizeof(row0), "%02d/%02d/%04d", alarmData.day, alarmData.month, 2000 + alarmData.year);

    // Dolny: "HH:MM:SS"
    char row1[17];
    snprintf(row1, sizeof(row1), "%02d:%02d:%02d", alarmData.hour, alarmData.minute, alarmData.second);

    // Jeśli blinkOn = false i akurat setIndex = 0 => chowamy "DD"
    // Jeżeli setIndex=1 => chowamy "MM"
    // itd.
    if (!blinkOn)
    {
        switch(setIndex)
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
                // row0[6..9], bo "DD/MM/YYYY" = 10 znaków
                row0[6] = ' ';
                row0[7] = ' ';
                row0[8] = ' ';
                row0[9] = ' ';
                break;
            case 3: // hour
                row1[0] = ' ';
                row1[1] = ' ';
                break;
            case 4: // min
                row1[3] = ' ';
                row1[4] = ' ';
                break;
            case 5: // sec
                row1[6] = ' ';
                row1[7] = ' ';
                break;
        }
    }

    // Wyświetlamy
    Lcd_cursor(lcd, 0, 0);
    Lcd_string(lcd, row0);
    Lcd_cursor(lcd, 1, 0);
    Lcd_string(lcd, row1);
}
