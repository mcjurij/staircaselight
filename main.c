#include <limits.h>
#include <stdio.h>
#include <stdbool.h>

#include <avr/io.h> 
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include <util/delay.h>
#include "lcd.h"
#include "i2c.h"
#include "intr.h"
#include "menu.h"
#include "ramp.h"
#include "eeprom.h"


static void init_global_settings()
{
    read_settings( &global_settings );

    if( global_settings.valid != 0xC7 )
    {
        lcd_print_str_l1( "writing defaults");
        global_settings.ramp_up_time = 10;
        global_settings.hold_time = 10;
        global_settings.ramp_down_time = 20;
        global_settings.hold_maximum = 100;
        global_settings.ramp_up_start = 0;
        init_ramp_settings();
        write_settings( &global_settings );
        lcd_print_str_l2( "to eeprom");
        _delay_ms(2000);
    }
}


#define MENU_TOP_MAX 8
const static char *menu_top[MENU_TOP_MAX] = {
    "IDLE",
    "RAMP UP TIME",
    "HOLD TIME",
    "HOLD INTENSITY",
    "RAMP DOWN TIME",
    "START INTENSITY",
    "EDIT RAMP?",
    "STORE?"
};

static int8_t menu_top_idx = 0;


int main(void)
{
    cli();
    
	i2c_init();
	lcd_init();
    
    
    DDRB &= ~(1 << PB1);    // input
    PORTB &= ~(1 << PB1);     // PB1 is in tri state
    DDRB |= (1 << PB0);     // output
    
    DDRC &= ~_BV(PC2);
    PORTC |= _BV(PC2);
    
    TCCR1A = 0;
    TCCR1B = 0;

    // pin change
    PCICR |= (1 << PCIE0);    // set PCIE0 to enable PCMSK0 scan
    PCICR |= (1 << PCIE1);    // enable PCMSK1 scan for PCINT10 PC2
    
    PCMSK0 = 0;
    //PCMSK0 |= (1 << PCINT1);  // set PCINT1 to trigger an interrupt on state change 

    PCMSK1 = 0;
    PCMSK1 |= _BV(PCINT10);   // detect pin change of PC2

    
    //TIMSK1 |= (1 << TOIE1);    // enable timer interrupt
    // overflow timer settings
    TCCR1B |= (1 << CS10);
    TIFR1 |= (1 << TOV1);

    init_switches();
    init_global_settings();
    
    lcd_clear();
    lcd_print_str_l1( "interpolate...");
    interpolate();
    char buf[40];
    sprintf( buf, "ramp size: %d", get_ramp_table_size());
    lcd_print_str_l2(buf);
    _delay_ms(2000);

#ifdef TEST_LED
	lcd_clear();
	lcd_write("TEST ON");
    PORTB |= (1 << PB0);
	_delay_ms(1000);
    lcd_write("TEST OFF");
    PORTB &= ~(1 << PB0);
    _delay_ms(1000);
    lcd_write("TEST ON");
    PORTB |= (1 << PB0);
	_delay_ms(1000);
    lcd_write("TEST OFF");
    PORTB &= ~(1 << PB0);
    _delay_ms(1000);
#endif
    
    lcd_clear();
    
    init_intr();

    lcd_clear();
    sprintf( buf, "U %d, H %d, D %d", global_settings.ramp_up_time, global_settings.hold_time, global_settings.ramp_down_time);
    lcd_print_str_l1( buf );
    sprintf( buf, "HI %d, RUI %d", global_settings.hold_maximum, global_settings.ramp_up_start);
    lcd_print_str_l2( buf );
    _delay_ms(2000);

    lcd_clear();
    lcd_print_str_l1( "GOING TO IDLE" );
    
    sei();    // enable interrupts
                
    
    global_menu_timeout = false;
    
	while(1)
    {
        _delay_ms(50);
        
        if( button_select_up_wait() )
        {
            if( menu_top_idx > 0 )
                menu_top_idx--;
        }
        
        if( button_select_dwn_wait() )
        {
            if( menu_top_idx < MENU_TOP_MAX-1 )
                menu_top_idx++;
        }
        
        if( switch_permanent() )
        {
            menu_top_idx = 0;
        }
        
        static int8_t old_menu_top_idx = 0;
        if( menu_top_idx != old_menu_top_idx )
        {
            lcd_clear();
            old_menu_top_idx = menu_top_idx;
            
            lcd_print_str_l1( menu_top[menu_top_idx] );
        }
        
        
        switch(menu_top_idx)
        {
        case 0:   // IDLE
            idle();
            break;
        
        case 1:  // RAMP UP TIME SECS
            menu_top_idx += menu_item_ramp_up_time( &global_settings.ramp_up_time );
            break;
        
        case 2:  // HOLD TIME SECS
            menu_top_idx += menu_item_hold_time( &global_settings.hold_time );
            break;
        
        case 3:  // HOLD INTENSITY
            menu_top_idx += menu_item_hold_intens( &global_settings.hold_maximum );
            break;
        
        case 4:  // RAMP DOWN TIME SECS
            menu_top_idx += menu_item_ramp_down_time( &global_settings.ramp_down_time );
            break;
        
        case 5:  // START INTENSITY
            menu_top_idx += menu_item_ramp_start_intens( &global_settings.ramp_up_start );
            break;
        
        case 6:  // EDIT RAMP?
            {
                bool edit_ramp = false;
                menu_top_idx += menu_item_yesno( &edit_ramp );
                if( edit_ramp )
                {
                    lcd_print_str_l1( "wait for timeout");
                    lcd_print_str_l2( "to leave");
                    _delay_ms(2000);
                    lcd_clear();
                    ramp_menu();
                    global_menu_timeout = false;
                    menu_top_idx = 7;
                }
            }
            break;
        
        case 7:  // STORE
            {
                bool store = false;
                menu_top_idx += menu_item_yesno( &store );
                if( store )
                {
                    lcd_print_str_l1( "writing settings");
                    write_settings( &global_settings );
                    interpolate();
                    compute_settings();
                    _delay_ms(1000);
                    lcd_clear();
                    menu_top_idx = 0;
                }
            }
            break;
        }
                
        if( global_menu_timeout )
        {
            global_menu_timeout = false;
            menu_top_idx = 0;
        }
        else
        {       
            if( menu_top_idx < 0 )
                menu_top_idx = 0;
            if( menu_top_idx > MENU_TOP_MAX-1 )
                menu_top_idx = MENU_TOP_MAX-1;
        }
    }
}
