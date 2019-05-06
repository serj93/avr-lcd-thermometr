//Atmega8A - используемый микроконтроллер

// В настройках проекта обязательно правильно укажите свою тактовую частоту
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h> // здесь описаны режимы сна
#include <util/delay.h>
#include <stdio.h>

//объявим  библиотеку
#include "libs/nokia5110.h"
#include "libs/ds18s20.h"

void showTemperature(void);
void printTemp(char *temp, uint8_t start_line, uint8_t shift);
void startADC(void);
void stopADC(void);
void LCD_turn_off_LED(void);
void LCD_turn_on_LED(void);

// Общие переменные ------------------------------\

volatile uint8_t isHomeSensor=0, isStreetSensor=0;

TSDS18x20 DS18x20_h, DS18x20_s;
TSDS18x20 *pDS18x20_h = &DS18x20_h;
TSDS18x20 *pDS18x20_s = &DS18x20_s;

// Hold Vcc value
volatile float vcc = 0.0f;
//------------------------------------------------/

ISR(INT0_vect)
{
	showTemperature();
}

ISR(INT1_vect)
{
	startADC();
}

ISR(ADC_vect) //ADC End of Conversion interrupt 
{
	unsigned char adc_data;
	char temp[5];

	adc_data = ADC>>2; //read 8 bit value
	vcc = 1.265 * 255 / adc_data;

	dtostrf(vcc,5,2,temp);

	LCD_turn_on_LED();
	LCD_init();

	// X, Y, nymber_of_line
	LCD_print_line(66,1,3);
	// *Char, Y, X, Type (t/v)
	printDigits(temp,2,0,'v');

	_delay_ms(3000);
	LCD_clear();
	_delay_ms(500);
	LCD_turn_off_LED();

	// Switch off ADC
	stopADC();

	// Power_down LCD
	LCD_write_byte(0x24, 1);
}

void startADC(void)
{
	//Set the Band Gap voltage as the ADC input (Vref = AREF pin; Vmeasured = Vbg (1.3V))
	ADMUX = 0xE;
	// Enable Interrupt, Start conversation; Fdivision = 32  
    ADCSRA = (1<<ADEN)|(1<<ADIE)|(1<<ADSC)|5;
}

void stopADC(void) 
{
	ADCSRA = 0x00;
}

void showTemperature()
{
	char temp1[5],temp2[5];
	uint8_t start_line = 0;

	if(isStreetSensor) 
	{
		// Initiate a temperature conversion and get the temperature reading
		if (DS18x20_MeasureTemperature(pDS18x20_s))
		{
			dtostrf(DS18x20_TemperatureValue(pDS18x20_s),3,0,temp1);
		}		
	}
		
	if(isHomeSensor) 
	{
		// Initiate a temperature conversion and get the temperature reading
		if(DS18x20_MeasureTemperature(pDS18x20_h))
		{
			dtostrf(DS18x20_TemperatureValue(pDS18x20_h),3,0,temp2);
		}		
	}

	LCD_turn_on_LED();
	LCD_init();
	
	if(isStreetSensor) 
	{
		// X, Y, nymber_of_line
		LCD_print_line(0,start_line,0);
		start_line += 1;
		// Temp, Y, X
		printDigits(temp1, start_line, 0, 't');
		start_line += 2;
	}


	if(isHomeSensor) 
	{
		// X, Y, nymber_of_line
		LCD_print_line(0,start_line,1);
		start_line += 1;
		// Temp, Y, X
		printDigits(temp2, start_line, 0, 't');
	}

	if (!isHomeSensor && !isStreetSensor) 
	{
		// X, Y, nymber_of_line
		LCD_print_line(32,2,2);
	}

	_delay_ms(3000);
	LCD_clear();
	_delay_ms(500);
	LCD_turn_off_LED();
	
	// Power_down LCD
	LCD_write_byte(0x24, 1);
}

void printDigits(char *temp, uint8_t start_line, uint8_t shift, char type)
{
	uint8_t index, point;

	for(index=0; index<5; index++)
	{
		switch(temp[index]) {
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				point = ((uint8_t)temp[index]-48)*2;
				LCD_print_big_num(shift, start_line, point);
				shift += 12;
				break;
			case '-':
				LCD_print_big_num(shift,start_line,20);
				shift += 12;
				break;
			case '.':
				LCD_print_big_num(shift,start_line,24);
				shift += 6;
				break;
			default:
				shift += 12;
				break;
		}
	}

	if(type == 't') {
		shift = 48;
		LCD_print_big_num(shift,start_line,22);
	}
	if(type == 'v') LCD_print_big_num(shift,start_line,26);
}

void LCD_turn_off_LED() 
{
	PORTB |= _BV(PB0); 
}

void LCD_turn_on_LED() 
{
	PORTB &= ~_BV(PB0); 
}

void AVR_Init()
{
	DDRB = 0x3F; // LCD + PB0 LED
	PORTB |= _BV(PB0); // Switch off LED

	DDRD = 0x00;
	PORTD = 0xFF;

	//прерывание по логическому 0 на INT0 и на INT1
	MCUCR &= ~(_BV(ISC00)|_BV(ISC01)|_BV(ISC10)|_BV(ISC11));  
	GICR |= _BV(INT0)|_BV(INT1);
}

int main(void)
{
	AVR_Init();
	
	// Init DS18B20 sensor (return 0 - Ok, 1 - Fail)
	if(!DS18x20_Init(pDS18x20_h,&PORTC,PC0)) {
		// Enable flug
		isHomeSensor = 1;
		
		// Set DS18B20 resolution to 12 bits.
		DS18x20_SetResolution(pDS18x20_h,CONF_RES_12b);
		DS18x20_WriteScratchpad(pDS18x20_h);
	}

	// Init DS18B20 sensor (return 0 - Ok, 1 - Fail)
	if(!DS18x20_Init(pDS18x20_s,&PORTC,PC1)) {
		// Enable flug
		isStreetSensor = 1;
		
		// Set DS18B20 resolution to 12 bits.
		DS18x20_SetResolution(pDS18x20_h,CONF_RES_12b);
		DS18x20_WriteScratchpad(pDS18x20_s);
	}
	
	// Show temperature
	showTemperature();
	// Check voltage
	startADC();

	_delay_ms(500);

	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sei();
	
    while(1)
    {
		sleep_enable(); // разрешаем сон
        sleep_cpu(); 	// спать!
    }
}
