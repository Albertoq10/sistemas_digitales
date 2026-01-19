/*
 *  display_oled.c
 *
 *  Created on: Jan 10, 2026
 *      Authors: Alberto and Marcos
 */


#include "display_oled.h"
#include "ssd1306.h"
#include <stdio.h>



void OLED_StatusUpdate(I2C_HandleTypeDef *hi2c1, uint8_t temp, modos_persianas_t modo, uint8_t p1, uint8_t p2)
{
    uint8_t lastTemp = 255;
    modos_persianas_t lastModo = (modos_persianas_t)255;
    uint8_t lastP1 = 255;
    uint8_t lastP2 = 255;

    if (temp == lastTemp && modo == lastModo && p1 == lastP1 && p2 == lastP2)
    	return;  // no cambia nada

    lastTemp = temp;
    lastModo = modo;
    lastP1 = p1;
    lastP2 = p2;

    const char *modoStr = "AUTO";
    if (modo == MODO_manual) modoStr = "MANUAL";
    else if (modo == MODO_vacaciones) modoStr = "VAC";

    char l1[20], l2[20], l3[22];
    snprintf(l1, sizeof(l1), "MODO: %s" "   %d C", modoStr, temp);
    snprintf(l2, sizeof(l2), "TEMP: %d C", temp);
    snprintf(l3, sizeof(l3), "P1: %3d %%" "P2: %3d %%", p1, p2);


    ssd1306_Fill(Black);
    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString(l1, Font_7x10, White);
    ssd1306_SetCursor(0, 11);
    ssd1306_WriteString(l3, Font_7x10, White);
    ssd1306_SetCursor(0, 22);
    ssd1306_WriteString(l3, Font_7x10, White);


    ssd1306_UpdateScreen(hi2c1);
}
