
#include <stdbool.h>
#include <stdio.h>
#include <util/delay.h>

#include "menu.h"

#include "lcd.h"
#include "ramp.h"
#include "eeprom.h"

#define RAMP_MAX 5700
static const ramp_values_t ramp_bases = {
    20, 40, 60, 80, 100,
    120, 140, 160, 180, 200,
    250, 300, 350, 400, 450, 500,
    600, 700, 800, 900, 1000,
    1200, 1400, 1600, 1800, 2000,
    2500, 3000, 3500, 4000,
    5000, RAMP_MAX
};

static const int ramp_steps[RAMP_VALS] = {
    1, 1, 2, 2, 5,
    5, 5, 5, 5, 10,
    10, 10, 10, 20, 25, 25,
    25, 25, 50, 100, 100,
    100, 200, 200, 200, 500,
    500, 500, 500, 500,
    500, 0
};

#define  TRANS_VALS  600

uint16_t ramp_table[ TRANS_VALS+1 ];
unsigned int ramp_table_size;

unsigned int get_ramp_table_size()
{
    return ramp_table_size;
}

void init_ramp_settings()
{
    for(int i = 0; i<RAMP_VALS; i++)
    {
        global_settings.ramp_values[i] = ramp_bases[i];
    }
}

void interpolate()
{
    double step = (double)RAMP_MAX / (double)TRANS_VALS;
    int n, k = 0;

    for( int i = 0; i < TRANS_VALS; i++)
        ramp_table[i] = RAMP_MAX;
    
    // first part starts with zero
    double m = (double)global_settings.ramp_values[0] / (double)ramp_bases[0];
    double x;
    
    for( x = 0.; x < (ramp_bases[0]-step); x+= step)
    {
        double y = m * x;
        
        ramp_table[k++] = (unsigned int)y;
    }
    
    for( n = 0; n < RAMP_VALS-1; n++)
    {
        m = (double)(global_settings.ramp_values[n+1] - global_settings.ramp_values[n]) / (double)(ramp_bases[n+1] - ramp_bases[n]);
        
        for( x = ramp_bases[n]; x < (ramp_bases[n+1]-step); x+= step)
        {
            double y = global_settings.ramp_values[n] + m * (x - ramp_bases[n]);
            
            ramp_table[k++] = (unsigned int)y;
        }
    }

    ramp_table_size = k;
}


uint16_t ramp_translate( unsigned int x )
{
    return ramp_table[x];
}

void ramp_menu()
{
    int curr_ramp_idx = 0;
    bool done = false;
    uint16_t cnt_for_idle = 0;
    lcd_clear();
    char buf[40];
    sprintf(buf, "%d", ramp_bases[curr_ramp_idx] );
    lcd_print_str_l1(buf);
    sprintf( buf, "%d", global_settings.ramp_values[curr_ramp_idx]);
    lcd_print_str_l2(buf);
    
    while(!done)
    {
        bool alt = false;
        
        if( button_select_up() )
        {
            if( curr_ramp_idx > 0 )
                curr_ramp_idx--;
            alt = true;
        }
        
        if( button_select_dwn() )
        {
            if( curr_ramp_idx < RAMP_VALS )
                curr_ramp_idx++;
            alt = true;
        }
        
        if( button_value_up() )
        {
            global_settings.ramp_values[curr_ramp_idx] += ramp_steps[curr_ramp_idx];
            alt = true;
        }
        
        if( button_value_dwn() )
        {
            if( ( global_settings.ramp_values[curr_ramp_idx] - ramp_steps[curr_ramp_idx] ) >= 0 )
                global_settings.ramp_values[curr_ramp_idx] -= ramp_steps[curr_ramp_idx];
            alt = true;
        }
        
        if( alt )
        {
            cnt_for_idle = 0;
            lcd_clear();
            sprintf(buf, "%d", ramp_bases[curr_ramp_idx] );
            lcd_print_str_l1(buf);
            
            // tcnt_fire = global_settings.ramp_values[curr_ramp_idx] + FIRE_SHORTEST;
            
            sprintf( buf, "%d", global_settings.ramp_values[curr_ramp_idx]);
            lcd_print_str_l2(buf);
        }
        else
            cnt_for_idle++;
        
        if( cnt_for_idle == 60000 )
        {
            done = true;
            lcd_print_str_l2( " - timeout -" );
            _delay_ms(2000);
            global_menu_timeout = true;
        }
    }
}
