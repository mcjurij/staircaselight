#ifndef EEPROM_H_
#define EEPROM_H_

#define RAMP_VALS 32

typedef int ramp_values_t[RAMP_VALS];
    
typedef struct
{
    uint8_t valid;              // valid if 0xC7
    uint16_t ramp_up_time;      // in sec
    uint16_t hold_time;         // in sec
    uint16_t ramp_down_time;    // in sec
    uint16_t hold_maximum;      // min 51, max 100 = 100%
    uint16_t ramp_up_start;     // min 0, max 50 = 50%
    ramp_values_t ramp_values;
} settings_t;

void write_settings(settings_t *settings);

void read_settings(settings_t *settings);

extern settings_t global_settings;

#endif
