
#include <avr/interrupt.h>
#include <util/delay.h>
#include <limits.h>

#include "lcd.h"
#include "intr.h"
#include "menu.h"
#include "ramp.h"

//#define FIRE_SHORTEST  UINT_MAX-6000
#define FIRE_SHORTEST  UINT_MAX-6035


static uint16_t tcnt_fire;


static uint8_t ramp_phase = 0, ramp_next_phase = 0;

static float ramp_x, ramp_step, ramp_step_up, ramp_step_dwn, ramp_step_up_retrigger;
static uint16_t ramp_cycles, ramp_cycles_up, ramp_cycles_dwn, ramp_cycles_hold;
static float ramp_hold_max, ramp_up_start;

static uint8_t intr_phase = 0;


void compute_settings()
{
    ramp_cycles_up = 50 * global_settings.ramp_up_time;

    ramp_hold_max = (float)global_settings.hold_maximum/100. * get_ramp_table_size();
    ramp_up_start = (float)global_settings.ramp_up_start/100. * get_ramp_table_size();
    
    ramp_step_up = (ramp_hold_max-ramp_up_start) / (float)ramp_cycles_up;
    ramp_step_up_retrigger = ramp_hold_max / (float)ramp_cycles_up;
    ramp_cycles_dwn = 50 * global_settings.ramp_down_time;
    ramp_step_dwn = ramp_hold_max / (float)ramp_cycles_dwn;
    
    ramp_cycles_hold = 50 * global_settings.hold_time;
}


void init_intr()
{
    // tcnt_fire = FIRE_SHORTEST;
    
    compute_settings();

    ramp_phase = RAMP_PHASE_IDLE;
    ramp_next_phase = RAMP_PHASE_IDLE;
    ramp_x = ramp_up_start;
    ramp_step = ramp_step_up;
    ramp_cycles = ramp_cycles_up;
}


void print_ramp_phase()
{
    const char *s;

    switch( get_ramp_phase() )
    {
    case 0:
        s = "RAMP UP   ";
        break;
    case 1:
        s = "HOLD      ";
        break;
    case 2:
        s = "RAMP DOWN ";
        break;
    case 3:
        s = "IDLE      ";
        break;
   
    case 4:
        s = "PERMANENT ";
        break;
    default:
        s = "???       ";
    }
    
    lcd_print_str_l1(s);
}


void idle()
{
    static uint8_t old_ramp_phase = RAMP_PHASE_IDLE;

    if( ramp_phase != old_ramp_phase )
    {
        lcd_clear();
        print_ramp_phase();
        old_ramp_phase = ramp_phase;
    }
    
    if( switch_permanent() )
    {
        PCMSK0 = 0;
        TIMER_DISABLE;
        PORTB |= (1 << PB0);       // triac on
        ramp_phase = ramp_next_phase = RAMP_PHASE_PERM;
    }
    else
    {
        if( ramp_phase == RAMP_PHASE_PERM )
        {
            PCMSK0 = 0;
            PORTB &= ~(1 << PB0);  // triac off
            ramp_phase = ramp_next_phase = RAMP_PHASE_IDLE; // go to idle
        }
    }
}


uint8_t get_ramp_phase()
{
    return ramp_phase;
}


ISR(TIMER1_OVF_vect)
{
    PORTB |= (1 << PB0);       // fire triac
    _delay_us( 3. );
    PCMSK0 |= (1 << PCINT1);   // turn on pin change intr
    PORTB &= ~(1 << PB0);
}


ISR(PCINT0_vect)
{
    if( (PINB & (1 << PINB1)) )    // rising edge
    {   
        PCMSK0 = 0;             // turn off pin change intr
        _delay_us( 8. );

        TCNT1 = tcnt_fire;
        
        switch( intr_phase )
        {
        case 0:   // upper half of the sine
            ramp_x += ramp_step;
            tcnt_fire = ramp_translate( (unsigned int) ramp_x ) + FIRE_SHORTEST;
            intr_phase = 1;
            ramp_cycles--;
            if( ramp_cycles == 0 )
                intr_phase = 3;
            break;
            
        case 1:  // lower half of the sine
            intr_phase = 0;
             _delay_us( 2. );
            break;
            
        case 3:
            switch( ramp_next_phase )
            {
            case RAMP_PHASE_UP:
                ramp_x = ramp_up_start;
                ramp_cycles = ramp_cycles_up;
                ramp_step = ramp_step_up;
                intr_phase = 0;
                ramp_phase = RAMP_PHASE_UP;
                ramp_next_phase = RAMP_PHASE_HOLD;
                break;
                
            case RAMP_PHASE_HOLD:
                ramp_cycles = ramp_cycles_hold;
                ramp_step = 0.;
                intr_phase = 0;
                ramp_phase = RAMP_PHASE_HOLD;
                ramp_next_phase = RAMP_PHASE_DOWN;
                break;
                
            case RAMP_PHASE_DOWN:
                ramp_cycles = ramp_cycles_dwn;
                ramp_step = -ramp_step_dwn;
                intr_phase = 0;
                ramp_phase = RAMP_PHASE_DOWN;
                ramp_next_phase = RAMP_PHASE_IDLE;
                break;

            case RAMP_PHASE_IDLE:      // go to idle by shutting off everything 
                TIMER_DISABLE;         // turn off overflow intr
                PCMSK0 = 0;            // turn off pin change intr
                PORTB &= ~(1 << PB0);  // turn off triac
                ramp_phase = RAMP_PHASE_IDLE;
                break;
            }
            break;            
        }
    }
}


ISR(PCINT1_vect)
{   
    if (PINC & _BV(PC2))
    {
        if( ramp_phase == RAMP_PHASE_IDLE )  // IDLE
        {
            ramp_x = ramp_up_start;
            ramp_cycles = ramp_cycles_up;
            ramp_step = ramp_step_up;
            intr_phase = 0;
            ramp_phase = RAMP_PHASE_UP;
            ramp_next_phase = RAMP_PHASE_HOLD;
            
            tcnt_fire = ramp_translate( (unsigned int) ramp_x ) + FIRE_SHORTEST;
            TCNT1 = tcnt_fire;
            PCMSK0 |= (1 << PCINT1);   // turn on pin change intr
            TIMER_ENABLE;
        }
        else if( ramp_phase == RAMP_PHASE_HOLD )   // re-trigger => prolong hold phase
        {
            ramp_cycles = ramp_cycles_hold;
        }
        else if( ramp_phase == RAMP_PHASE_DOWN )   // re-trigger => ramp up again
        {
            uint16_t skip_cycles = (uint16_t) (ramp_x / ramp_step_up_retrigger );
            ramp_cycles = ramp_cycles_up - skip_cycles;
            ramp_step = ramp_step_up_retrigger;
            
            tcnt_fire = ramp_translate( (unsigned int) ramp_x ) + FIRE_SHORTEST;
            TCNT1 = tcnt_fire;
            intr_phase = 0;
            ramp_phase = RAMP_PHASE_UP;
            ramp_next_phase = RAMP_PHASE_HOLD;
        }
    }
    
}
