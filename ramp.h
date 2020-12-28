#ifndef RAMP_H_
#define RAMP_H_


unsigned int get_ramp_table_size();
void init_ramp_settings();
void interpolate();
uint16_t ramp_translate( unsigned int x );
void ramp_menu();

#endif
