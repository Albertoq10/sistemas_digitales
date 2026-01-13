/*
 * proyecto_persianas.h
 *
 *  Created on: Jan 10, 2026
 *      Author: alber
 */

#ifndef APPLICATION_USER_PROYECTO_PERSIANAS_H_
#define APPLICATION_USER_PROYECTO_PERSIANAS_H_



#include <stdint.h>

// Modos de persianas
typedef enum {
	MODO_automatico = 0,
	MODO_manual = 1,
	MODO_vacaciones = 2


} modos_persianas_t;

void definir_modo(modos_persianas_t modo);
modos_persianas_t leer_estado();
void modos_tarea();


#endif /* APPLICATION_USER_PROYECTO_PERSIANAS_H_ */
