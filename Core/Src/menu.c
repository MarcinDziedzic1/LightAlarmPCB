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
int8_t currentSubMenuIndex = 0;
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
void Menu_Display(Lcd_HandleTypeDef *lcd, uint8_t index, bool forceRefresh)
{
    // Zmienne statyczne, żeby pamiętać poprzednie wartości między wywołaniami
    static uint8_t oldIndex = 255;  // lub inna wartość, która nie wystąpi normalnie
    static uint8_t oldPage  = 255;

    // Bufory na wiersze LCD (po 16 znaków + terminator)
    static char row0[17];
    static char row1[17];

    // Każda „strona” wyświetla 2 elementy: w wierszu 0 i w wierszu 1
    uint8_t page = index / 2;         // np. index=0..1 => page=0; index=2..3 => page=1
    uint8_t firstItem  = page * 2;    // np. page=0 => item0,1; page=1 => item2,3
    uint8_t secondItem = firstItem + 1;

    // Funkcja pomocnicza do pełnego odświeżenia ekranu
    void RefreshWholePage(void)
    {
        Lcd_clear(lcd);  // czyścimy cały ekran

        // Przygotowujemy wiersze
        memset(row0, ' ', 16);
        memset(row1, ' ', 16);
        row0[16] = '\0';
        row1[16] = '\0';

        // Wypełniamy row0
        if (firstItem < menuCount)
        {
            bool selected = ((index % 2) == 0); // czy górny wiersz ma strzałkę?
            if (selected)
                snprintf(row0, sizeof(row0), ">%s", menuItems[firstItem]);
            else
                snprintf(row0, sizeof(row0), " %s", menuItems[firstItem]);
        }

        // Wypełniamy row1
        if (secondItem < menuCount)
        {
            bool selected = ((index % 2) == 1); // czy dolny wiersz ma strzałkę?
            if (selected)
                snprintf(row1, sizeof(row1), ">%s", menuItems[secondItem]);
            else
                snprintf(row1, sizeof(row1), " %s", menuItems[secondItem]);
        }

        // Wyświetlamy wiersze
        Lcd_cursor(lcd, 0, 0);
        Lcd_string(lcd, row0);
        Lcd_cursor(lcd, 1, 0);
        Lcd_string(lcd, row1);
    }

    // 1. Jeśli forceRefresh == true -> wykonaj pełne odświeżenie
    //    (np. właśnie wróciliśmy z innego menu)
    if (forceRefresh)
    {
        // Wymuszamy także reset poprzednich wartości, by uniknąć
        // niepotrzebnych optymalizacji.
        oldPage  = 255;
        oldIndex = 255;
    }

    // 2. Sprawdź, czy zmieniła się strona (page), albo to „pierwsze” wywołanie
    if (page != oldPage)
    {
        // Tak -> pełne odświeżenie
        RefreshWholePage();
        oldPage  = page;
        oldIndex = index;
    }
    else
    {
        // Ta sama strona, sprawdź, czy zmienił się index w obrębie tej strony
        if (index != oldIndex)
        {
            // Z której linii przenosimy strzałkę?
            uint8_t oldRow = (oldIndex % 2);
            uint8_t newRow = (index % 2);

            // Jeżeli „strzałka” (>) przenosi się z góry na dół, lub odwrotnie:
            if (oldRow != newRow)
            {
                // Odśwież TYLKO row0 i row1, ale bez clear,
                // tak by zmienić, gdzie stoi '>'
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
                }
                {
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
            }
            // Gdyby oldRow == newRow, tu nic nie robimy (to raczej się nie zdarza
            // przy 2 elementach na stronę)
            oldIndex = index;
        }
    }
}
/**
 * @brief Wyświetla zawartość wybranej opcji menu
 *        (np. TIME, SENSOR, USB1 => subMenu2, L_BULB => subMenuLB, itd.).
 */
void Menu_ShowOption(Lcd_HandleTypeDef *lcd, uint8_t index, I2C_HandleTypeDef *hi2c)
{
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
    case 3: // USB2 => sub-menuB
    {
    	currentSubMenuIndex = 0;
    	gState = SUBMENU_2B;
        DisplaySubMenuON_OFF(lcd, currentSubMenuIndex, usb2_OnOff);
    	break;
    }
    case 4:
    {
    	currentSubMenuIndex = 0;
    	gState = SUBMENU_L_BULB;
    	DisplaySubMenuON_OFF(lcd, currentSubMenuIndex, l_BulbOnOff);
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
            onLabel[len - 1] = '*';
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

void DisplayAlarmMenu(Lcd_HandleTypeDef *lcd, int8_t subIndex)
{
	// Wyświetlamy w pierwszym wierszu SET, w drugim BACK

    char row0[17];
    char row1[17];
    memset(row0, ' ', 16);
    memset(row1, ' ', 16);
    row0[16] = '\0';
    row1[16] = '\0';

    // SET
    if (subIndex == 0)
        snprintf(row0, sizeof(row0), ">SET      ");
    else
        snprintf(row0, sizeof(row0), " SET      ");

    // BACK
    if (subIndex == 1)
        snprintf(row1, sizeof(row1), ">BACK     ");
    else
        snprintf(row1, sizeof(row1), " BACK     ");

    Lcd_cursor(lcd, 0, 0);
    Lcd_string(lcd, row0);
    Lcd_cursor(lcd, 1, 0);
    Lcd_string(lcd, row1);
}

void DisplayAlarmSet(Lcd_HandleTypeDef *lcd, int8_t setIndex, bool blinkOn)
{
    // Indeksy: 0=day,1=month,2=year,3=hour,4=minute,5=sec

    // 1. Przygotowujemy stringi do wyświetlenia
    // Górny wiersz: "DD/MM/YYYY"
    char row0[17];
    snprintf(row0, sizeof(row0), "%02d/%02d/%04d",
             alarmData.day, alarmData.month, 2000 + alarmData.year);

    // Dolny wiersz: "HH:MM:SS"
    char row1[17];
    snprintf(row1, sizeof(row1), "%02d:%02d:%02d",
             alarmData.hour, alarmData.minute, alarmData.second);

    // 2. Jeśli blinkOn = false, chowamy aktualnie edytowaną daną
    //    (zamieniamy ją na spacje):
    if (!blinkOn)
    {
        switch(setIndex)
        {
            case 0: // day
                // row0[0..1] to dwie cyfry "DD"
                row0[0] = ' ';
                row0[1] = ' ';
                break;
            case 1: // month
                // row0[3..4] to "MM"
                row0[3] = ' ';
                row0[4] = ' ';
                break;
            case 2: // year
                // row0[6..9] to "YYYY" (czyli np. 2025)
                row0[6] = ' ';
                row0[7] = ' ';
                row0[8] = ' ';
                row0[9] = ' ';
                break;
            case 3: // hour
                // row1[0..1] to "HH"
                row1[0] = ' ';
                row1[1] = ' ';
                break;
            case 4: // minute
                // row1[3..4] to "MM"
                row1[3] = ' ';
                row1[4] = ' ';
                break;
            case 5: // second
                // row1[6..7] to "SS"
                row1[6] = ' ';
                row1[7] = ' ';
                break;
        }
    }

    // 3. Wyświetlamy (bez czyszczenia całego LCD)
    Lcd_cursor(lcd, 0, 0);
    Lcd_string(lcd, row0);

    Lcd_cursor(lcd, 1, 0);
    Lcd_string(lcd, row1);
}

void DisplayAlarmTriggered(Lcd_HandleTypeDef *lcd, int8_t subIndex)
{
    // Bufory na dwie linie
    char row0[17];
    char row1[17];
    memset(row0, ' ', 16);
    memset(row1, ' ', 16);
    row0[16] = '\0';
    row1[16] = '\0';

    // W pierwszej linii od 5. znaku: "ALARM!"
    // (np. "     ALARM!")
    snprintf(row0 + 5, sizeof(row0) - 5, "ALARM!");

    // W drugiej linii mamy 2 opcje: STOP i SNOOZE.
    // Jeśli subIndex == 0, to '>' stoi przy STOP.
    // Jeśli subIndex == 1, to '>' stoi przy SNOOZE.
    if (subIndex == 0)
    {
        snprintf(row1, sizeof(row1), ">STOP   SNOOZE");
    }
    else
    {
        snprintf(row1, sizeof(row1), " STOP  >SNOOZE");
    }

    // Wyświetlamy (bez czyszczenia całego LCD)
    Lcd_cursor(lcd, 0, 0);
    Lcd_string(lcd, row0);

    Lcd_cursor(lcd, 1, 0);
    Lcd_string(lcd, row1);
}

