#include <avr/io.h>
#include <util/delay.h>
#include "../libs/nokia5110.h"

void LCD_write_byte(unsigned char dat, unsigned char command)
{
	unsigned char i;
	
	if (command == 1)
	LCD_DC_clr;
	else
	LCD_DC_set;

	for(i=0;i<8;i++)
	{
		if(dat&0x80)
		SDIN_set;
		else
		SDIN_clr;
		SCLK_clr;
		dat = dat << 1;
		SCLK_set;
	}
}

void LCD_init() 
{
	LCD_RST_clr;
	_delay_us(1);
	LCD_RST_set;
	SCE_clr;

	_delay_us(1);

	LCD_write_byte(0x21, 1);	// set LCD mode (00100PDVH)
	LCD_write_byte(0x13, 1);	// set bias voltage (old c8)
	LCD_write_byte(0x04, 1);	// temperature correction (old 06)
	LCD_write_byte(0x13, 1);	// 1:48
	LCD_write_byte(0xB8, 1);	// generator of hight voltage ?
	LCD_write_byte(0x20, 1);	// use bias command, vertical
	LCD_write_byte(0x0c, 1);	// set LCD mode,display normally
	LCD_clear();	            // clear the LCD
}

void LCD_clear() 
{
	unsigned int i;

	LCD_write_byte(0x0c, 1);
	LCD_write_byte(0x80, 1);

	for (i=0; i<504; i++)
	{
		LCD_write_byte(0, 0);
	}
}

void LCD_set_XY(unsigned char X, unsigned char Y)
{
	LCD_write_byte(0x40 | Y, 1);	// column
	LCD_write_byte(0x80 | X, 1);    // row
}

void LCD_print_big_num(unsigned char X,unsigned char Y,unsigned char number)
{
	LCD_set_XY(X,Y);

	for (uint8_t line=0; line<12; line++)
	LCD_write_byte(font12x16[number][line], 0);

	LCD_set_XY(X,Y+1);
	number++;

	for (uint8_t line=0; line<12; line++)
	LCD_write_byte(font12x16[number][line], 0);
}

void LCD_print_line(unsigned char X,unsigned char Y,unsigned char number)
{
	unsigned char line;

	LCD_set_XY(X,Y);

	for (line=0; line<18; line++)
	LCD_write_byte(strLines[number][line], 0);
}
