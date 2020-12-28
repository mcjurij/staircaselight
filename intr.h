#ifndef INTR_H_
#define INTR_H_


#include "eeprom.h"

extern settings_t global_settings;

#define TIMER_ENABLE   TIMSK1 |= (1 << TOIE1)
#define TIMER_DISABLE  TIMSK1 &= ~(1 << TOIE1)


/*
 ramp phases:
 0: ramp up
 1: hold
 2: ramp down
 3: idle
 4: permanent
*/

#define RAMP_PHASE_UP     0
#define RAMP_PHASE_HOLD   1
#define RAMP_PHASE_DOWN   2
#define RAMP_PHASE_IDLE   3
#define RAMP_PHASE_PERM   4

void compute_settings();
void init_intr();

void print_ramp_phase();
void idle();
uint8_t get_ramp_phase();

#endif
