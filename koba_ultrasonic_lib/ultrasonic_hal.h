#ifndef ULTRASONIC_HAL_H
#define ULTRASONIC_HAL_H

#include <stdint.h>
#include <stdbool.h>

// El usuario debe implementar estas funciones en su proyecto.

void ultrasonic_hal_trigger_setHigh();
void ultrasonic_hal_trigger_setLow();
void ultrasonic_hal_setTrigger_timeout(ultrasonic_t *us, uint32_t microseconds);
void ultrasonic_hal_echo_timeout(ultrasonic_t *us);
void ultrasonic_hal_echo_setRisingEdge();
void ultrasonic_hal_echo_setFallingEdge();
bool ultrasonic_hal_isTriggerPinHigh();

#endif // ULTRASONIC_HAL_H
