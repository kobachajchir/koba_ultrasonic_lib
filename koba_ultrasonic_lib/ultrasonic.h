#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef void (*PrintfMethod)(const char* message);
	
#define ULTRA_MS_ECHO_TIMEOUT   (20U)
#define ULTRA_MIN_INTERVAL_MS   (60U)
#define ULTRA_TRIGGER_PULSE_US  (10U)

#define DO_TRIGGER flags.bitmap.bit0
#define TRIGGER_FINISH flags.bitmap.bit1
#define CALCULATE_DISTANCE flags.bitmap.bit2
#define TIMEDOUT flags.bitmap.bit3
#define ECHO_RISING flags.bitmap.bit4
#define TRIGGER_ALLOWED flags.bitmap.bit5
#define ECHO_RECEIVED flags.bitmap.bit6
#define NEW_RESULT flags.bitmap.bit7

#define ALL_FLAGS flags.byte

typedef union {
	struct {
		uint8_t bit0: 1;  // Bit 0 - Parte del nibble bajo (bitL)
		uint8_t bit1: 1;  // Bit 1 - Parte del nibble bajo (bitL)
		uint8_t bit2: 1;  // Bit 2 - Parte del nibble bajo (bitL)
		uint8_t bit3: 1;  // Bit 3 - Parte del nibble bajo (bitL)
		uint8_t bit4: 1;  // Bit 4 - Parte del nibble alto (bitH)
		uint8_t bit5: 1;  // Bit 5 - Parte del nibble alto (bitH)
		uint8_t bit6: 1;  // Bit 6 - Parte del nibble alto (bitH)
		uint8_t bit7: 1;  // Bit 7 - Parte del nibble alto (bitH)
	} bitmap;
	struct {
		uint8_t bitL: 4;  // Nibble bajo (bits 0-3)
		uint8_t bitH: 4;  // Nibble alto (bits 4-7)
	} nibbles;
	uint8_t byte;
} Byte_Flag;

// Estados del sensor
typedef enum {
	ULTRA_IDLE,
	ULTRA_TRIGGERING,
	ULTRA_WAIT_RISING,
	ULTRA_WAIT_FALLING,
	ULTRA_CALCULATING,
	ULTRA_DONE,
	ULTRA_TIMEOUT
} ultra_state_t;

typedef struct {
	// Variables
	volatile uint32_t echo_init_time;
	volatile uint32_t echo_finish_time;
	volatile uint32_t distance_mm;
	// Banderas
	volatile Byte_Flag flags;
	// Estado
	volatile ultra_state_t state;
	volatile bool debugMode;
	PrintfMethod print_method;
} ultrasonic_t;

void ultrasonic_init_flags(ultrasonic_t *us);

// Inicializa la estructura interna
void ultrasonic_init(ultrasonic_t *us, PrintfMethod printMethod);

// Comienza una medición (pone la lógica para TRIGGER)
bool ultrasonic_start(ultrasonic_t *us);

// Debe ser llamada desde interrupciones o loop para actualizar la máquina de estados
void ultrasonic_update(ultrasonic_t *us);

// Función para calcular la distancia, si deseas una genérica
void ultrasonic_calculate_distance(ultrasonic_t *us);

// Devuelve la distancia en mm
uint32_t ultrasonic_get_distance(const ultrasonic_t *us);

// Función para establecer método de impresión
void ultrasonic_set_print_method(ultrasonic_t *us, PrintfMethod print_method);

// Función de impresión para la librería
void ultrasonic_print(ultrasonic_t *us, const char* message);

bool ultrasonic_timeout_clear(ultrasonic_t *us, bool state);

void ultrasonic_set_debug_mode(ultrasonic_t *us, bool newState);

#endif // ULTRASONIC_H
