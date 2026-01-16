#include "display_oled.h"
#include "ssd1306.h"
#include <stdio.h>



void OLED_StatusUpdate(I2C_HandleTypeDef *hi2c1, uint8_t temp, modos_persianas_t modo)
{
    static uint8_t lastTemp = 255;
    static modos_persianas_t lastModo = (modos_persianas_t)255;


    if (temp == lastTemp && modo == lastModo)
    	return;  // no cambia nada

    lastTemp = temp;
    lastModo = modo;

    const char *modoStr = "AUTO";
    if (modo == MODO_manual) modoStr = "MANUAL";
    else if (modo == MODO_vacaciones) modoStr = "VAC";

    char l1[20], l2[20];
    snprintf(l1, sizeof(l1), "MODO: %s", modoStr);
    snprintf(l2, sizeof(l2), "TEMP: %d C", temp);

    ssd1306_Fill(Black);
    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString(l1, Font_7x10, White);
    ssd1306_SetCursor(0, 12);
    ssd1306_WriteString(l2, Font_7x10, White);
    ssd1306_SetCursor(0, 24);
    ssd1306_WriteString("WiFi Server", Font_7x10, White);

    ssd1306_UpdateScreen(hi2c1);
}
