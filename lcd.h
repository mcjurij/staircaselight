/*
 * lcd.h
 *
 * Created: 20.01.2018 12:51:11
 *  Author: Ulrich
 */ 

#ifndef __LCD_H__
#define __LCD_H__
#include <avr/pgmspace.h>

#define LCD_I2C_ADDR	0x4E

//Port Belegung am PCF8574 (1<<PORT)
#define LCD_RS			(1<<0)
#define LCD_RW			(1<<1)
#define LCD_E			(1<<2)
#define LCD_BACKLIGHT	(1<<3)

#define LCD_D4			(1<<4)
#define LCD_D5			(1<<5)
#define LCD_D6			(1<<6)
#define LCD_D7			(1<<7)

#define LCD_WIDTH 16
#define LCD_ADDR_LINE1	(0x80)
#define LCD_ADDR_LINE2	(0xC0)

void lcd_init(void);
void lcd_clear(void);
void lcd_home(void);
void lcd_print_str (char *str);
void lcd_write_P (const char *Buffer,...);

#define lcd_write(format, args...)   lcd_write_P(PSTR(format) , ## args)

void lcd_print_str_l1(const char *str);
void lcd_print_str_l2(const char *str);

#endif
