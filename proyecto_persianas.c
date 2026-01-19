/*
 * proyecto_persianas.c
 *
 *  Created on: Jan 10, 2026
 *      Authors: Alberto and Marcos
 *
 *  -logica del sistema de persianas
 *  - Tiene 3 modos: auto, manual y vacaciones
 *  - También guarda el "estado" (% actual) de cada persiana para poder mostrarlo en OLED
 */


#include "proyecto_persianas.h"
#include "stm32l4xx_hal.h"
#include "stm32l475e_iot01.h"
#include "servomotores_pwm.h"

extern ADC_HandleTypeDef hadc1;

modos_persianas_t g_modo_actual = MODO_automatico;//modo actual
uint32_t last = 0;

uint32_t vac_next_ms = 0;//para modo vacaciones
uint32_t last_change_ms = 0;
uint8_t  target_pct = 0;//%deseado

uint8_t g_p1 = 0, g_p2 = 0;

uint8_t persiana_pct(uint8_t p) {//devuelve el %
	return (p==1) ? g_p1 : (p==2)?g_p2:0;
}

void persiana_pct_actualizar(uint8_t p, uint8_t pct){//actualiza %
  if(p==1)
	  g_p1=pct;
  else if(p==2)
	  g_p2=pct;
}

//aqui ya se mueve servo y se guarada %
void persiana_set_pct_estado(uint8_t p, uint8_t pct)
{
  if (pct > 100)
	  pct = 100;
  persiana_set_pct(p, pct);     // driver
  persiana_pct_actualizar(p, pct);   // estado para OLED
}



/////////////////////////
//control
//cambia de modo
void definir_modo(modos_persianas_t modo){
	g_modo_actual = modo;
}

modos_persianas_t leer_estado(){
	return g_modo_actual;
}
//////////////////////////

//para elegir persiana y %apertura
void manual_set(uint8_t persiana, uint8_t pct)
{
    if (persiana != 1 && persiana != 2)
    	return;

    definir_modo(MODO_manual);
    persiana_set_pct_estado(persiana, pct);
}

//las dos persianas se mueven con el mismo %
void mover_persianas(uint8_t pct)
{
  persiana_set_pct_estado(1, pct);
  persiana_set_pct_estado(2, pct);
}


//modo auto
void tarea_modo_automatico(void)
{
  if (leer_estado() != MODO_automatico)
	  return;

  uint8_t luz = leer_nivel_luz(&hadc1);

  //% dependiendo cantidad de luz
  uint8_t desired;
  if (luz < 35)
	  desired = 0;
  else if (luz > 90)
	  desired = 50;
  else
	  desired = 100;

  uint32_t tiempo_actual = HAL_GetTick();

  //para evitar que se mueve demasiado
  if (desired != target_pct) {
      if (tiempo_actual - last_change_ms >= 5000) {//5 segundos, no bloqueante
        target_pct = desired;
        last_change_ms = tiempo_actual;
        mover_persianas(target_pct);
      }
    }
  }

//funciona como una semilla para que sea aletorio
void vacaciones_start(void)
{
  srand(HAL_GetTick());   //
}


//tarea
void tarea_modo_vacaciones(void)
{
  if (leer_estado() != MODO_vacaciones)
	  return;

  uint32_t tiempo_actual = HAL_GetTick();
  if ((int32_t)(tiempo_actual - vac_next_ms) < 0)
	  return;

  //rand aqui tambien funciona como aleatorio
  uint8_t persiana = (rand() % 2) + 1;     // 1 o 2
  uint8_t pct;

  switch (rand() % 4) {
    case 0: pct = 0;
    break;
    case 1: pct = 25;
    break;
    case 2: pct = 50;
    break;
    default: pct = 100;
    break;
  }

  persiana_set_pct_estado(persiana, pct);

  uint32_t dt = (rand() % 2) ? 2000 : 3000;
  vac_next_ms = tiempo_actual + dt;
}


//maquina de estados
//funcion prueba oara ver que cambie de modo
//pequena maquina de estados
void modos_tarea(){
	uint32_t periodo;

    switch (g_modo_actual) {
		case MODO_automatico:
			periodo = 2000;
			tarea_modo_automatico();
			break;
		case MODO_manual:
			periodo = 400;
			break;
		case MODO_vacaciones:
			periodo = 80;
			tarea_modo_vacaciones();
			break;
		default:
			periodo = 500;

			break;
	}

    //esto solo para ver el led parpadear
	if (HAL_GetTick() - last >= periodo) {
		last = HAL_GetTick();
		BSP_LED_Toggle(LED2);
	}

}
