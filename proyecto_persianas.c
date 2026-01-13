/*
 * proyecto_persianas.c
 *
 *  Created on: Jan 10, 2026
 *      Author: alber
 */


#include "proyecto_persianas.h"
#include "stm32l4xx_hal.h"
#include "stm32l475e_iot01.h"



modos_persianas_t g_modo_actual = MODO_automatico;
static uint32_t last = 0;

//cambia de modo
void definir_modo(modos_persianas_t modo){
	g_modo_actual = modo;
}

modos_persianas_t leer_estado(){
	return g_modo_actual;
}


//funcion prueba oara ver que cambie de modo
//pequena maquina de estados
void modos_tarea(){
	uint32_t periodo;

    switch (g_modo_actual) {
		case MODO_automatico:
			periodo = 800;
			break;
		case MODO_manual:
			periodo = 200;
			break;
		case MODO_vacaciones:
			periodo = 80;
			break;
		default:
			periodo = 500;
			break;
	}

	if (HAL_GetTick() - last >= periodo) {
		last = HAL_GetTick();
		BSP_LED_Toggle(LED2);
	}

}
