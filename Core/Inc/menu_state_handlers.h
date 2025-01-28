// Created by: Marcin Dziedzic
// menu_state_handlers.h

#ifndef MENU_STATE_HANDLERS_H
#define MENU_STATE_HANDLERS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "lcd.h"
#include "menu.h"
#include "RTC.h"
#include <stdbool.h>

/**
 * @brief Prototypy funkcji obsługujących poszczególne stany menu.
 */

void HandleMenuState(int val, uint32_t now, Lcd_HandleTypeDef *lcd);
void HandleOptionState(int val, uint32_t now, Lcd_HandleTypeDef *lcd);
void HandleSubMenu2State(int val, uint32_t now, Lcd_HandleTypeDef *lcd);
void HandleSubMenu2BState(int val, uint32_t now, Lcd_HandleTypeDef *lcd);
void HandleSubMenuLBState(int val, uint32_t now, Lcd_HandleTypeDef *lcd);
void HandleSubMenuAlarmState(int val, uint32_t now, Lcd_HandleTypeDef *lcd);
void HandleSubMenuAlarmSetState(int val, uint32_t now, Lcd_HandleTypeDef *lcd);
void HandleAlarmTriggered(int val, uint32_t now, Lcd_HandleTypeDef *lcd);
void HandleSubMenuAlarmLSensorState(int val, uint32_t now, Lcd_HandleTypeDef *lcd);

#ifdef __cplusplus
}
#endif

#endif /* MENU_STATE_HANDLERS_H */
