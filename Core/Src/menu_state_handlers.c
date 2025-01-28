#include "menu_state_handlers.h"
#include "fade.h"
#include "light_sen.h"
#include "r_encoder.h"

// Uchwyty do I2C i TIM – zdefiniowane w main.c, tutaj tylko extern
extern I2C_HandleTypeDef hi2c1;
extern TIM_HandleTypeDef htim3;

extern int8_t menuIndex;

/* --------------------------------------------------------------------------
   Zaawansowany debouncing przycisku:
   - 3 próbki w krótkich odstępach (np. co 20 ms)
   - jeśli >=2 z 3 próbek to stan wciśnięty (0), uznajemy przycisk za wciśnięty
   - lockout 50 ms (przez ten czas zwracamy stale "true")
   -------------------------------------------------------------------------- */
static bool CheckDebouncedButton(void)
{
    // Statyczne zmienne do zapamiętywania próbek
    static uint8_t  samples[3] = {1,1,1}; // 1=nie wciśnięty, 0=wciśnięty
    static uint8_t  idx            = 0;
    static uint32_t lastSampleTime = 0;
    static uint32_t lockoutUntil   = 0;

    // Parametry
    const uint32_t SAMPLE_INTERVAL = 20;    // co ile ms bierzemy nową próbkę
    const uint32_t LOCKOUT_MS      = 50;   // przez ile ms "trzymamy" wciśnięcie
    const int      DEBOUNCE_THRESHOLD = 2;  // min. liczba 0 w 3 próbkach

    uint32_t now = HAL_GetTick();

    // 1. Jeśli trwa lockout, zawsze zwracamy true (przycisk wciąż "wciśnięty")
    if (now < lockoutUntil)
    {
        return true;
    }

    // 2. Czy czas na kolejną próbkę?
    if ((now - lastSampleTime) < SAMPLE_INTERVAL)
    {
        // Nie czas na nową próbkę -> sprawdzamy istniejące
        int countPressed = 0;
        for (int i = 0; i < 3; i++)
        {
            if (samples[i] == 0) countPressed++;
        }
        return (countPressed >= DEBOUNCE_THRESHOLD);
    }

    // Czas na pobranie nowej próbki
    lastSampleTime = now;

    // Odczyt stanu pinu (0= wciśnięty, 1= niewciśnięty)
    samples[idx] = HAL_GPIO_ReadPin(ENCODER_BTN_GPIO_Port, ENCODER_BTN_Pin);
    idx = (idx + 1) % 3; // przesuwamy indeks w kółko

    // Liczymy ile z 3 próbek to 0
    int pressedCount = 0;
    for (int i = 0; i < 3; i++)
    {
        if (samples[i] == 0) pressedCount++;
    }

    // Jeśli co najmniej 2 z 3 to "0", przycisk wciśnięty:
    if (pressedCount >= DEBOUNCE_THRESHOLD)
    {
        // Ustawiamy lockout do now + 50 ms
        lockoutUntil = now + LOCKOUT_MS;
        return true;
    }

    // W przeciwnym razie nie wciśnięty
    return false;
}

/**
 * @brief Obsługa stanu MENU_STATE.
 * @param val - odczyt z enkodera (0=lewo,1=prawo, -1=brak)
 * @param now - aktualny czas (HAL_GetTick())
 */
void HandleMenuState(int val, uint32_t now, Lcd_HandleTypeDef *lcd)
{
    // Zmienne do debounce (zdefiniowane w main.c)
    extern uint32_t lastEncMove;
    extern uint32_t lastBtnPress;

    // 1. Obrót enkodera (lewo/prawo)
    switch (val)
    {
    case 0:
    case 1:
        if ((now - lastEncMove) >= 500) // prosty debounce
        {
            lastEncMove = now;
            if (val == 0)
            {
                menuIndex--;
                if (menuIndex < 0) menuIndex = menuCount - 1;
            }
            else
            {
                menuIndex++;
                if (menuIndex >= menuCount) menuIndex = 0;
            }
            Menu_Display(lcd, menuIndex, false);
        }
        break;

    default:
        // -1 => brak obrotu
        break;
    }

    // 2. Wciśnięcie przycisku (zaawansowany debouncing)
    bool pressed = CheckDebouncedButton();
    if (pressed && ((now - lastBtnPress) >= 500))
    {
        lastBtnPress = now;

        // Reagujemy w zależności od menuIndex
        if (menuIndex == 1)
        {
        	currentSubMenuIndex = 0;
            gState = SUBMENU_ALARM;
            DisplayAlarmMenu(lcd, currentSubMenuIndex);
        }
        else if (menuIndex == 2)
        {
        	currentSubMenuIndex = 0;
            gState = SUBMENU_2;
            DisplaySubMenuON_OFF(lcd, currentSubMenuIndex, usb_OnOff);
        }
        else if (menuIndex == 3)
        {
        	currentSubMenuIndex = 0;
            gState = SUBMENU_2B;
            DisplaySubMenuON_OFF(lcd, currentSubMenuIndex, usb2_OnOff);
        }
        else if (menuIndex == 4)
        {
        	currentSubMenuIndex = 0;
            gState = SUBMENU_L_BULB;
            DisplaySubMenuON_OFF(lcd, currentSubMenuIndex, l_BulbOnOff);
        }
        else
        {
            // Pozostałe przypadki => OPTION_STATE
            Menu_ShowOption(lcd, menuIndex, &hi2c1);
            gState = OPTION_STATE;
        }
    }
}

/**
 * @brief Obsługa stanu OPTION_STATE (np. TIME lub SENSOR).
 */
void HandleOptionState(int val, uint32_t now, Lcd_HandleTypeDef *lcd)
{
    extern uint32_t lastTimeUpdate;
    extern uint32_t lastBtnPress;

    // 1. Obrót enkodera (lewo/prawo) – w tym stanie może nie być używany
    // 2. Wciśnięcie przycisku = powrót do MENU
    bool pressed = CheckDebouncedButton();
    if (pressed && ((now - lastBtnPress) >= 500))
    {
        lastBtnPress = now;
        Menu_Display(lcd, menuIndex, true);
        gState = MENU_STATE;
    }
    else
    {
        // Odświeżanie widoku co 1s (np. pomiar czujnika, wyświetlanie czasu)
        if ((now - lastTimeUpdate) >= 1000)
        {
            Menu_ShowOption(lcd, menuIndex, &hi2c1);
            lastTimeUpdate = now;
        }
    }
}

/**
 * @brief Obsługa stanu SUBMENU_2 – sterowanie USB1 (ON/OFF/BACK).
 */
void HandleSubMenu2State(int val, uint32_t now, Lcd_HandleTypeDef *lcd)
{
    extern uint32_t lastEncMove;
    extern uint32_t lastBtnPress;

    // Obrót enkodera
    if (val == 0 || val == 1)
    {
        if ((now - lastEncMove) >= 500)
        {
            lastEncMove = now;
            if (val == 0)
            {
            	currentSubMenuIndex--;
                if (currentSubMenuIndex < 0) currentSubMenuIndex = 2;
            }
            else
            {
            	currentSubMenuIndex++;
                if (currentSubMenuIndex > 2) currentSubMenuIndex = 0;
            }
            DisplaySubMenuON_OFF(lcd, currentSubMenuIndex, usb_OnOff);
        }
    }

    // Wciśnięcie przycisku
    bool pressed = CheckDebouncedButton();
    if (pressed && ((now - lastBtnPress) >= 500))
    {
        lastBtnPress = now;
        switch(currentSubMenuIndex)
        {
        case 0: // ON
            if (usb_OnOff != 1)
            {
                usb_OnOff = 1;
                HAL_GPIO_WritePin(USB1_EN_GPIO_Port, USB1_EN_Pin, GPIO_PIN_SET);
                DisplaySubMenuON_OFF(lcd, currentSubMenuIndex, usb_OnOff);
            }
            break;
        case 1: // OFF
            if (usb_OnOff != 2)
            {
                usb_OnOff = 2;
                HAL_GPIO_WritePin(USB1_EN_GPIO_Port, USB1_EN_Pin, GPIO_PIN_RESET);
                DisplaySubMenuON_OFF(lcd, currentSubMenuIndex, usb_OnOff);
            }
            break;
        case 2: // BACK
            gState = MENU_STATE;
            Menu_Display(lcd, menuIndex, true);
            break;
        }
    }
}

/**
 * @brief Obsługa stanu SUBMENU_2B – sterowanie USB2 (ON/OFF/BACK).
 */
void HandleSubMenu2BState(int val, uint32_t now, Lcd_HandleTypeDef *lcd)
{
    extern uint32_t lastEncMove;
    extern uint32_t lastBtnPress;

    // Obrót enkodera
    if (val == 0 || val == 1)
    {
        if ((now - lastEncMove) >= 500)
        {
            lastEncMove = now;
            if (val == 0)
            {
            	currentSubMenuIndex--;
                if (currentSubMenuIndex < 0) currentSubMenuIndex = 2;
            }
            else
            {
            	currentSubMenuIndex++;
                if (currentSubMenuIndex > 2) currentSubMenuIndex = 0;
            }
            DisplaySubMenuON_OFF(lcd, currentSubMenuIndex, usb2_OnOff);
        }
    }

    // Wciśnięcie przycisku
    bool pressed = CheckDebouncedButton();
    if (pressed && ((now - lastBtnPress) >= 500))
    {
        lastBtnPress = now;
        switch(currentSubMenuIndex)
        {
        case 0: // ON
            if (usb2_OnOff != 1)
            {
                usb2_OnOff = 1;
                HAL_GPIO_WritePin(USB2_EN_GPIO_Port, USB2_EN_Pin, GPIO_PIN_SET);
                DisplaySubMenuON_OFF(lcd, currentSubMenuIndex, usb2_OnOff);
            }
            break;
        case 1: // OFF
            if (usb2_OnOff != 2)
            {
                usb2_OnOff = 2;
                HAL_GPIO_WritePin(USB2_EN_GPIO_Port, USB2_EN_Pin, GPIO_PIN_RESET);
                DisplaySubMenuON_OFF(lcd, currentSubMenuIndex, usb2_OnOff);
            }
            break;
        case 2: // BACK
            gState = MENU_STATE;
            Menu_Display(lcd, menuIndex, true);
            break;
        }
    }
}

/**
 * @brief Obsługa stanu SUBMENU_L_BULB – sterowanie żarówką (ON/OFF/BACK).
 */
void HandleSubMenuLBState(int val, uint32_t now, Lcd_HandleTypeDef *lcd)
{
    extern uint32_t lastEncMove;
    extern uint32_t lastBtnPress;
    extern LedFadeHandle_t g_fadeHandle;
    // Obrót enkodera
    if (val == 0 || val == 1)
    {
        if ((now - lastEncMove) >= 500)
        {
            lastEncMove = now;
            if (val == 0)
            {
            	currentSubMenuIndex--;
                if (currentSubMenuIndex < 0) currentSubMenuIndex = 2;
            }
            else
            {
            	currentSubMenuIndex++;
                if (currentSubMenuIndex > 2) currentSubMenuIndex = 0;
            }
            DisplaySubMenuON_OFF(lcd, currentSubMenuIndex, l_BulbOnOff);
        }
    }

    // Wciśnięcie przycisku
    bool pressed = CheckDebouncedButton();
    if (pressed && ((now - lastBtnPress) >= 500))
    {
        lastBtnPress = now;
        switch(currentSubMenuIndex)
        {
        case 0: // ON
            if (l_BulbOnOff != 1)
            {
                l_BulbOnOff = 1;
                LedFade_Start(&g_fadeHandle, &htim3, TIM_CHANNEL_4,
                              FADE_IN,    // kierunek
                              200,        // steps
                              1000);
                DisplaySubMenuON_OFF(lcd, currentSubMenuIndex, l_BulbOnOff);
            }
            break;
        case 1: // OFF
            if (l_BulbOnOff != 2)
            {
                l_BulbOnOff = 2;
                LedFade_Start(&g_fadeHandle, &htim3, TIM_CHANNEL_4,
                              FADE_OUT,    // kierunek
                              200,        // steps
                              1000);
                DisplaySubMenuON_OFF(lcd, currentSubMenuIndex, l_BulbOnOff);
            }
            break;
        case 2: // BACK
            gState = MENU_STATE;
            Menu_Display(lcd, menuIndex, true);
            break;
        }
    }
}

/**
 * @brief Obsługa stanu SUBMENU_ALARM – podmenu (SET/BACK).
 */
void HandleSubMenuAlarmState(int val, uint32_t now, Lcd_HandleTypeDef *lcd)
{
    extern uint32_t lastEncMove;
    extern uint32_t lastBtnPress;
    extern int8_t lightSensorMode; // 1=ON, 2=OFF
    extern int8_t sensorSubIndex;

    // Zakładam, że globalnie mamy: static int8_t currentSubMenuIndex = 0; (lub w inny sposób)

    // Obrót enkodera
    if (val == 0 || val == 1)
    {
        if ((now - lastEncMove) >= 500)
        {
            lastEncMove = now;
            if (val == 0) // enkoder w lewo
            {
                currentSubMenuIndex--;
                if (currentSubMenuIndex < 0) currentSubMenuIndex = 2; // zawinięcie do 2
            }
            else // enkoder w prawo
            {
                currentSubMenuIndex++;
                if (currentSubMenuIndex > 2) currentSubMenuIndex = 0; // zawinięcie do 0
            }
            DisplayAlarmMenu(lcd, currentSubMenuIndex);
        }
    }

    // Wciśnięcie przycisku
    bool pressed = CheckDebouncedButton();
    if (pressed && ((now - lastBtnPress) >= 500))
    {
        lastBtnPress = now;
        switch(currentSubMenuIndex)
        {
        case 0: // SET => przejście do stanu ustawiania czasu alarmu
            alarmSetIndex = 0;
            gState = SUBMENU_ALARM_SET;
            DisplayAlarmSet(lcd, alarmSetIndex, true);
            break;

        case 1: // L_Sensor => przejście do nowego stanu
            // np. SUBMENU_ALARM_LSENSOR
            gState = SUBMENU_ALARM_LSENSOR;
            sensorSubIndex = 0;  // (zainicjuj subIndex)
            DisplaySubMenuON_OFF(lcd, sensorSubIndex, lightSensorMode);
            break;

        case 2: // BACK => powrót do głównego menu
            gState = MENU_STATE;
            Menu_Display(lcd, menuIndex, true);
            break;
        }
    }
}


/**
 * @brief Obsługa stanu SUBMENU_ALARM_SET – edycja (day, month, year, hour, min, sec).
 */
void HandleSubMenuAlarmSetState(int val, uint32_t now, Lcd_HandleTypeDef *lcd)
{
    extern uint32_t lastEncMove;
    extern uint32_t lastBtnPress;

    // Mruganie kursora co 500 ms
    static bool blinkOn = true;
    static uint32_t lastBlink = 0;

    // 1. Blink – co 500 ms odwracamy blinkOn i wywołujemy DisplayAlarmSet
    if (now - lastBlink >= 500)
    {
        blinkOn = !blinkOn;
        lastBlink = now;
        DisplayAlarmSet(lcd, alarmSetIndex, blinkOn);
    }

    // 2. Obrót enkodera – zmiana wartości
    if (val == 0 || val == 1)
    {
        if ((now - lastEncMove) >= 350)
        {
            lastEncMove = now;
            int dir = (val == 0) ? -1 : +1;

            switch(alarmSetIndex)
            {
            case 0: // day
                alarmData.day += dir;
                if (alarmData.day < 1)  alarmData.day = 31;
                if (alarmData.day > 31) alarmData.day = 1;
                break;

            case 1: // month
                alarmData.month += dir;
                if (alarmData.month < 1)  alarmData.month = 12;
                if (alarmData.month > 12) alarmData.month = 1;
                break;

            case 2: // year
                alarmData.year += dir;
                if (alarmData.year > 99) alarmData.year = 0;
                if (alarmData.year < 0)  alarmData.year = 99;
                break;

            case 3: // hour
                alarmData.hour += dir;
                if (alarmData.hour < 0)   alarmData.hour = 23;
                if (alarmData.hour > 23)  alarmData.hour = 0;
                break;

            case 4: // minute
                alarmData.minute += dir;
                if (alarmData.minute < 0)  alarmData.minute = 59;
                if (alarmData.minute > 59) alarmData.minute = 0;
                break;

            case 5: // second
                alarmData.second += dir;
                if (alarmData.second < 0)  alarmData.second = 59;
                if (alarmData.second > 59) alarmData.second = 0;
                break;
            }

            // Odśwież wyświetlacz po zmianie – z nowymi wartościami
            DisplayAlarmSet(lcd, alarmSetIndex, blinkOn);
        }
    }

    // 3. Wciśnięcie przycisku – przejście do kolejnego pola lub wyjście
    bool pressed = CheckDebouncedButton();
    if (pressed && ((now - lastBtnPress) >= 500))
    {
        lastBtnPress = now;
        alarmSetIndex++;
        if (alarmSetIndex > 5)
        {
            gState = SUBMENU_ALARM;
            DisplayAlarmMenu(lcd, 0);
        }
        else
        {
            blinkOn = true;
            DisplayAlarmSet(lcd, alarmSetIndex, blinkOn);
        }
    }
}

static void UpdateAlarmRow1(Lcd_HandleTypeDef *lcd, int8_t subIndex)
{
    char row1[17];
    memset(row1, ' ', 16);
    row1[16] = '\0';

    if (subIndex == 0)
        snprintf(row1, sizeof(row1), ">STOP   SNOOZE");
    else
        snprintf(row1, sizeof(row1), " STOP  >SNOOZE");

    // Nadpisujemy TYLKO drugi wiersz
    Lcd_cursor(lcd, 1, 0);
    Lcd_string(lcd, row1);
}


void HandleAlarmTriggered(int val, uint32_t now, Lcd_HandleTypeDef *lcd)
{
    extern bool alarmIsActive;
    extern bool skipLamp; // jeśli true => pomijamy włączenie lampy
    extern uint32_t lastEncMove;
    extern uint32_t lastBtnPress;
    extern LedFadeHandle_t g_fadeHandle;

    // Zapamiętujemy „stary” subIndex, żeby wykryć zmianę
    static int8_t oldSubMenuIndex = -1;

    // Flaga: czy pierwszy raz wchodzimy w stan ALARM_TRIGGERED?
    static bool firstCall = true;

    // Jeśli lampa jest wyłączona, włącz i zrób fade-in (tylko raz)
    if (l_BulbOnOff == 2) // lampka jest wyłączona
        {
            if (!skipLamp)
            {
                l_BulbOnOff = 1; // włączona
                LedFade_Start(&g_fadeHandle, &htim3, TIM_CHANNEL_4,
                              FADE_IN,    // kierunek
                              200,        // steps
                              1000);
            }
        }

    // 1. Pierwsze wejście w stan -> rysujemy ALARM! (wiersz 0) i wiersz 1
    if (firstCall)
    {
    	currentSubMenuIndex = 0;
        // Rysujemy cały ekran alarmu
        // (bez Lcd_clear – w samej funkcji DisplayAlarmTriggered)
        DisplayAlarmTriggered(lcd, currentSubMenuIndex);

        firstCall = false;
        // Zapamiętujemy, że wyświetliliśmy subIndex
        oldSubMenuIndex = currentSubMenuIndex;
    }

    // 2. Obsługa enkodera
    if ((val == 0 || val == 1) && ((now - lastEncMove) >= 350))
    {
        lastEncMove = now;
        int dir = (val == 0) ? -1 : +1;

        // Zmiana subIndex w zakresie 0..1
        currentSubMenuIndex += dir;
        if (currentSubMenuIndex < 0) currentSubMenuIndex = 1;
        if (currentSubMenuIndex > 1) currentSubMenuIndex = 0;

        // Jeśli subIndex się zmienił -> odśwież TYLKO DRUGI wiersz
        if (currentSubMenuIndex != oldSubMenuIndex)
        {
            UpdateAlarmRow1(lcd, currentSubMenuIndex);
            oldSubMenuIndex = currentSubMenuIndex;
        }
    }

    // 3. Obsługa przycisku STOP / SNOOZE
    bool pressed = CheckDebouncedButton();
    if (pressed && ((now - lastBtnPress) >= 500))
    {
        lastBtnPress = now;

        if (currentSubMenuIndex == 0)
        {
            // STOP -> wyłącz lampę
        	if (l_BulbOnOff == 1)
        	{
        		l_BulbOnOff = 2;
        		LedFade_Start(&g_fadeHandle, &htim3, TIM_CHANNEL_4,
        		              FADE_OUT,    // kierunek
        		              200,        // steps
        		              1000);
        	}
            alarmIsActive = false;
            // Przy wyjściu ze stanu -> zresetuj firstCall, by kolejnym
            // razem znowu wyświetlić „ALARM!”
            firstCall = true;

            gState = MENU_STATE;
            Menu_Display(lcd, menuIndex, true);
        }
        else
        {
            // SNOOZE -> +5min
            alarmData.minute += 5;
            if (alarmData.minute >= 60)
            {
                alarmData.minute -= 60;
                alarmData.hour++;
                if (alarmData.hour > 23)
                {
                    alarmData.hour = 0;
                    alarmData.day += 1;
                }
            }
            if (l_BulbOnOff == 1)
            {
            	l_BulbOnOff = 2;
            	LedFade_Start(&g_fadeHandle, &htim3, TIM_CHANNEL_4,
            	              FADE_OUT,    // kierunek
            	              200,        // steps
            	              1000);
            }


            alarmIsActive = false;
            firstCall = true;

            gState = MENU_STATE;
            Menu_Display(lcd, menuIndex, true);
        }
        return;
    }
}

void HandleSubMenuAlarmLSensorState(int val, uint32_t now, Lcd_HandleTypeDef *lcd)
{
    extern uint32_t lastEncMove;
    extern uint32_t lastBtnPress;

    extern int8_t lightSensorMode;
    extern int8_t sensorSubIndex;  // 0=ON, 1=OFF, 2=BACK

    // --- 1. Obsługa enkodera (góra/dół) ---
    if (val == 0 || val == 1)
    {
        if ((now - lastEncMove) >= 350)
        {
            lastEncMove = now;
            if (val == 0) // W lewo
            {
                sensorSubIndex--;
                if (sensorSubIndex < 0) sensorSubIndex = 2;
            }
            else // W prawo
            {
                sensorSubIndex++;
                if (sensorSubIndex > 2) sensorSubIndex = 0;
            }
            DisplaySubMenuON_OFF(lcd, sensorSubIndex, lightSensorMode);
        }
    }

    // --- 2. Obsługa przycisku (zatwierdzenie) ---
    bool pressed = CheckDebouncedButton();
    if (pressed && ((now - lastBtnPress) >= 500))
    {
        lastBtnPress = now;

        switch(sensorSubIndex)
        {
        case 0: // ON
            lightSensorMode = 1;
            DisplaySubMenuON_OFF(lcd, sensorSubIndex, lightSensorMode);
            // Zostajemy w tym samym submenu,
            // dopóki user nie wybierze BACK
            break;

        case 1: // OFF
            lightSensorMode = 2;
            DisplaySubMenuON_OFF(lcd, sensorSubIndex, lightSensorMode);
            // Podobnie, zostajemy w tym samym submenu
            break;

        case 2: // BACK
            gState = SUBMENU_ALARM; // wracamy do menu alarmu
            DisplayAlarmMenu(lcd, currentSubMenuIndex);
            break;
        }
    }
}


