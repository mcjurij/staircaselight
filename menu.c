#include <stdbool.h>
#include <stdio.h>

#include "menu.h"

#include <avr/sfr_defs.h>   // bit is clear

#include <util/delay.h>
#include "lcd.h"

bool global_menu_timeout;

#define DEBOUNCE_TIME  25

void init_switches()
{
    BUTTON_PORT |= _BV(SW_SELECT_UP);
    BUTTON_PORT |= _BV(SW_SELECT_DWN);
    BUTTON_PORT |= _BV(SW_VALUE_UP);
    BUTTON_PORT |= _BV(SW_VALUE_DWN);
    BUTTON_PORT |= _BV(SW_PERMANENT);
}


int button_select_up()
{
    /* the button is pressed when SW_SELECT_UP is clear */
    if( bit_is_clear(BUTTON_PIN, SW_SELECT_UP) )
    {
        _delay_ms(DEBOUNCE_TIME);
        if( bit_is_clear(BUTTON_PIN, SW_SELECT_UP) )
            return 1;
    }
    
    return 0;
}


int button_select_up_wait()
{
    if( button_select_up() )
    {
        while( bit_is_clear(BUTTON_PIN, SW_SELECT_UP) )
            ;
        return 1;
    }
    else
        return 0;
}


int button_select_dwn()
{
    if( bit_is_clear(BUTTON_PIN, SW_SELECT_DWN) )
    {
        _delay_ms(DEBOUNCE_TIME);
        if( bit_is_clear(BUTTON_PIN, SW_SELECT_DWN) )
            return 1;
    }
 
    return 0;
}


int button_select_dwn_wait()
{
    if( button_select_dwn() )
    {
        while( bit_is_clear(BUTTON_PIN, SW_SELECT_DWN) )
            ;
        return 1;
    }
    else
        return 0;
}


int button_value_up()
{
    if( bit_is_clear(BUTTON_PIN, SW_VALUE_UP) )
    {
        _delay_ms(DEBOUNCE_TIME);
        if( bit_is_clear(BUTTON_PIN, SW_VALUE_UP) )
            return 1;
    }
    
    return 0;
}


int button_value_dwn()
{
    if( bit_is_clear(BUTTON_PIN, SW_VALUE_DWN) )
    {
        _delay_ms(DEBOUNCE_TIME);
        if( bit_is_clear(BUTTON_PIN, SW_VALUE_DWN) )
            return 1;
    }
 
    return 0;
}


int switch_permanent()
{
    if( bit_is_clear(BUTTON_PIN, SW_PERMANENT) )
    {
        _delay_ms(DEBOUNCE_TIME);
        if( bit_is_clear(BUTTON_PIN, SW_PERMANENT) )
            return 1;
    }
 
    return 0;
}


static void lcd_print_uint16_l2( uint16_t u, const char *unit)
{
     char buf[40];
     
     sprintf(buf, "%u%s ", u, unit);
     lcd_print_str_l2( buf );
}


static int8_t menu_enter_value( uint16_t *value, uint16_t value_max, const char *unit)
{
    bool done = false;
    uint8_t  next_idx = 0;
    uint16_t cnt_for_idle = 0;
    lcd_print_uint16_l2( *value, unit);
    
    while(!done)
    {
        bool alt = false;
        
        if( button_select_up_wait() )
        {
            alt = true;
            done = true;
            next_idx = -1;
        }
        
        if( button_select_dwn_wait() )
        {
            alt = true;
            done = true;
            next_idx = 1;
        }
        
        if( button_value_up() )
        {
            alt = true;

            if( *value < value_max )
                (*value)++;
        }
        
        if( button_value_dwn() )
        {            
            alt = true;

            if( *value > 0 )
                (*value)--;
        }

        if( alt )
        {
            lcd_print_uint16_l2( *value, unit);
            cnt_for_idle = 0;
        }
        else
            cnt_for_idle++;

        if( cnt_for_idle == 50000 )
        {
            done = true;
            next_idx = 0;
            lcd_print_str_l2( " - timeout -" );
            _delay_ms(2000);
            global_menu_timeout = true;
        }
    }

    
    return next_idx;
}


int8_t menu_item_ramp_up_time( uint16_t *ramp_up_secs )
{
    return menu_enter_value( ramp_up_secs, 60, "s");
}

int8_t menu_item_hold_time( uint16_t *hold_secs )
{
    return menu_enter_value( hold_secs, 600, "s");
}

int8_t menu_item_ramp_down_time( uint16_t *ramp_dwn_secs )
{
    return menu_enter_value( ramp_dwn_secs, 300, "s");
}

int8_t menu_item_hold_intens( uint16_t *hold_maximum )
{
    return menu_enter_value( hold_maximum, 100, "%");
}

int8_t menu_item_ramp_start_intens( uint16_t *ramp_up_start )
{
    return menu_enter_value( ramp_up_start, 50, "%");
}


int8_t menu_item_yesno( bool *yes )
{
    bool done = false;
    uint8_t  next_idx = 0;
    uint16_t cnt_for_idle = 0;
    
    if( *yes )
        lcd_print_str_l2( ">YES< NO " );
    else
        lcd_print_str_l2( " YES >NO<" );
    
    while(!done)
    {
        bool alt = false;
        
        if( button_select_up_wait() )
        {
            alt = true;
            done = true;
            next_idx = -1;
        }
        
        if( button_select_dwn_wait() )
        {
            alt = true;
            done = true;
            next_idx = 1;
        }
        
        if( button_value_up() )
        {
            alt = true;
            *yes = !*yes;
        }
        
        if( button_value_dwn() )
        {            
            alt = true;
            *yes = !*yes;
        }

        if( alt )
        {
            if( *yes )
                lcd_print_str_l2( ">YES< NO " );
            else
                lcd_print_str_l2( " YES >NO<" );
            cnt_for_idle = 0;
        }
        else
            cnt_for_idle++;
        
        if( cnt_for_idle == 40000 )
        {
            done = true;
            next_idx = 0;
            *yes = false;
            lcd_print_str_l2( " - timeout -" );
            _delay_ms(2000);
            global_menu_timeout = true;
        }
    }

    
    return next_idx;
}
