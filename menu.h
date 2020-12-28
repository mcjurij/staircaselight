#ifndef _MENU_H_
#define _MENU_H_

#include <stdbool.h>

#include <avr/io.h>

#define BUTTON_PORT   PORTD  /* PORTx - register for button output */
#define BUTTON_PIN    PIND   /* PINx - register for button input */
#define SW_SELECT_UP  PD1
#define SW_SELECT_DWN PD0
#define SW_VALUE_UP   PD3
#define SW_VALUE_DWN  PD2
#define SW_PERMANENT  PD4

void init_switches();

int button_select_up();
int button_select_up_wait();
int button_select_dwn();
int button_select_dwn_wait();
int button_value_up();
int button_value_dwn();
int switch_permanent();

extern bool global_menu_timeout;

int8_t menu_item_ramp_up_time( uint16_t *ramp_up_secs );
int8_t menu_item_hold_time( uint16_t *hold_secs );
int8_t menu_item_ramp_down_time( uint16_t *ramp_dwn_secs );
int8_t menu_item_hold_intens( uint16_t *hold_maximum );
int8_t menu_item_ramp_start_intens( uint16_t *ramp_up_start );
int8_t menu_item_yesno( bool *yes );

#endif
