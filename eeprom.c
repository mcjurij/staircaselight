
#include <avr/eeprom.h>

#include "eeprom.h"

settings_t ee_settings __attribute__ ((section (".eeprom")));

settings_t global_settings;


void write_settings(settings_t *settings)
{
    settings->valid = 0xC7;
    
    eeprom_busy_wait();
    eeprom_write_block((const void *)settings, (void *)&ee_settings, sizeof(settings_t));
}


void read_settings(settings_t *settings)
{
    eeprom_busy_wait();
    eeprom_read_block( settings, &ee_settings, sizeof(settings_t));
}
