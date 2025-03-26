/*
 * ultrasonic.c
 *
 * Implementación de la máquina de estados y lógica de cálculo para el HC-SR04
 * usando un struct 'ultrasonic_t' que guarda la info interna.
 *
 * Esta versión asume que la parte de hardware (poner TRIGGER en HIGH/LOW,
 * leer ECHO, etc.) se define en 'ultrasonic_hal.c'.
 */

#include "ultrasonic.h"
#include "ultrasonic_hal.h"  // Funciones de bajo nivel (setTriggerHigh, setTriggerLow, etc.)
#include <stddef.h>

void ultrasonic_set_debug_mode(ultrasonic_t *us, bool newState){
	us->debugMode = newState;
}

void ultrasonic_init_flags(ultrasonic_t *us){
	us->echo_init_time    = 0;
	us->echo_finish_time  = 0;
	us->distance_mm       = 0;
	us->ALL_FLAGS = 0;
	us->TRIGGER_ALLOWED = 1;
	us->state = ULTRA_IDLE;
	ultrasonic_hal_echo_setRisingEdge();
	if(us->print_method != NULL){
		if(us->debugMode){
			ultrasonic_print(us, "LIB DEBUG - HCSR04 Flags Reset");	
		}
	}
}

// Inicializa la estructura interna
void ultrasonic_init(ultrasonic_t *us, PrintfMethod printMethod)
{
	ultrasonic_init_flags(us);	
	us->debugMode = true;
	if(printMethod == NULL){
		us->print_method = NULL;
		}else{
		us->print_method = printMethod; // Asignación directa
		if(us->debugMode){
			ultrasonic_print(us, "LIB DEBUG - HCSR04 Iniciado");
		}
	}
}

bool ultrasonic_timeout_clear(ultrasonic_t *us, bool state){
	if(us->state == ULTRA_TIMEOUT){
		ultrasonic_init_flags(us);
		us->debugMode = state;
		if(us->debugMode){	
			ultrasonic_print(us, "LIB DEBUG - HCSR04 TMDOUT Cleared");
		}
		return true;
	}
	return false;
}

// Pide iniciar una medición (cambia al estado de TRIGGERING)
bool ultrasonic_start(ultrasonic_t *us)
{
	// Solo se inicia la medición si el sensor está en reposo o finalizado
	if ((us->state == ULTRA_IDLE || us->state == ULTRA_DONE)) {
		// Comprobar que el trigger está permitido
		if (us->TRIGGER_ALLOWED) {
			us->state = ULTRA_TRIGGERING;
			if(us->debugMode){
				ultrasonic_print(us, "LIB DEBUG - HCSR04 Start: TRIGGER_ALLOWED true");
			}
			return true;
			} else {
			if(us->debugMode){
				ultrasonic_print(us, "LIB DEBUG - HCSR04 Start failed: TRIGGER_ALLOWED false");
			}
		}
		} else {
			if(us->debugMode){	
				ultrasonic_print(us, "LIB DEBUG - HCSR04 Start failed: wrong state");
			}
	}
	return false;
}

// Función principal de la máquina de estados
void ultrasonic_update(ultrasonic_t *us)
{
	switch (us->state)
	{
		case ULTRA_IDLE:
		// Espera a que ultrasonic_start() lo saque de este estado
		break;
		case ULTRA_TRIGGERING:
		{
			// Ya no se utiliza la rama DO_TRIGGER porque el ISR se encarga de encender el trigger
			if(us->TRIGGER_FINISH){ // Activado desde el ISR
				if(us->debugMode){
					ultrasonic_print(us, "LIB DEBUG - TRIGGER_FINISH lib");
				}
// 				us->state = ULTRA_WAIT_RISING;
// 				us->ECHO_RISING = 1; // Indica que se ha enviado el trigger y se espera ECHO
// 				us->TRIGGER_FINISH = 0; // Limpiamos la bandera para evitar transiciones repetidas
			}
			break;
		}
		// Los demás casos permanecen igual...
		case ULTRA_WAIT_RISING:
		{
			if (!us->ECHO_RISING) {
				us->state = ULTRA_WAIT_FALLING;
				ultrasonic_hal_echo_setFallingEdge();
				if(us->debugMode){
					ultrasonic_print(us, "LIB DEBUG - ECHO RISING bajo");
				}
			}
			if (us->TIMEDOUT) {
				us->state = ULTRA_TIMEOUT;
			}
			break;
		}
		case ULTRA_WAIT_FALLING:
		{
			if(us->ECHO_RECEIVED){
				us->state = ULTRA_CALCULATING;
				if(us->debugMode){
					ultrasonic_print(us, "LIB DEBUG - CALCULANDO");
				}
				us->CALCULATE_DISTANCE = 1;
			}
			if (us->TIMEDOUT) {
				us->state = ULTRA_TIMEOUT;
			}
			break;
		}
		case ULTRA_CALCULATING:
		{
			if (us->CALCULATE_DISTANCE) {
				ultrasonic_calculate_distance(us);
				us->CALCULATE_DISTANCE = 0;
				if(us->debugMode){
					ultrasonic_print(us, "LIB DEBUG - Calculado, pasando a estado listo");
				}
				us->state = ULTRA_DONE;
			}
			break;
		}
		case ULTRA_DONE:
		{
			// Permanece en DONE hasta la siguiente medición
			break;
		}
		case ULTRA_TIMEOUT:
			us->TIMEDOUT = 1;
		break;
	}
}

// Calcula la distancia en milímetros según el tiempo de pulso
void ultrasonic_calculate_distance(ultrasonic_t *us)
{
    uint32_t pulse_duration = (uint32_t)(us->echo_finish_time - us->echo_init_time);
    us->distance_mm = (uint32_t)(pulse_duration * 34UL) / 400UL;
	us->NEW_RESULT = 1;
}

// Función para que el usuario lea la distancia calculada
uint32_t ultrasonic_get_distance(const ultrasonic_t *us)
{
    return us->distance_mm;
}
// Función para establecer método de impresión
void ultrasonic_set_print_method(ultrasonic_t *us, PrintfMethod print_method) {
	us->print_method = print_method;
}

// Función de impresión para la librería
void ultrasonic_print(ultrasonic_t *us, const char* message) {
	if (us->print_method != NULL) {
		us->print_method(message);
	}
}

