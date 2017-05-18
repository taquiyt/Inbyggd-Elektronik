/*	microwave_timer.c  */
/*	Taqui Shah and Viktor Vitek,
	Timer that ticks down after being set a time. 
	Much like a mircowave-timer.*/
/*	LCD two line 16 characthers display  
	Compiler CC5x - not ANSI-C
	Potensometer*/
#include "16F690.h"
#pragma config |= 0x00D4 
/*	I/O-pin definitions                               */ 
/*	change if you need a pin for a different purpose  */
#pragma bit RS  @ PORTB.4
#pragma bit EN  @ PORTB.6
#pragma bit D7  @ PORTC.3
#pragma bit D6  @ PORTC.2
#pragma bit D5  @ PORTC.1
#pragma bit D4  @ PORTC.0
/*	The right value for the potensometer*/ 
#define SCALE_FACTOR 49 //	This gives us 50-0 minutes range
/*	init fucntions  */
void delay(char); 
void ADinit(void);
void lcd_init(void); 
void lcd_putchar(char);	
long potentiometer(void);
char modulo(unsigned long dividend, long divisor);
void lcd_putline(char start, const char * text, char variable_1, char variable_2, char variable_3, char variable_4);


void main(void)
{
    /*	init of I/O-pin direction in/out definitions	*/
	ANSEL	= 0; 	//  PORTC digital I/O
	ANSELH	= 0;
	TRISC	= 0b1111.0000;  /* RC3,2,1,0 out*/
    TRISB.4 = 0;	/*	RB4, RB6 out*/
    TRISB.6 = 0;	
	TRISB.7 = 1;	/*Initialisering av knappen, weak-pullup fix*/
	OPTION.7= 0;
	WPUB.7	= 1;
	
	/*	init of lcd	and AD converter*/
	lcd_init();
	ADinit();
	/*	init of variables	*/
	int		temp_variable = 0;
	char	minute_tens_diget = '0',	minute_ones_diget = '0',	second_tens_diget = '0',	second_ones_diget = '0',	i;
	unsigned int	time_minutes = 0,	time_seconds = 1;
	
	/*	Before loop	*/
	
	time_minutes = potentiometer(); // Set start-time
	
	/*	*********	*/
	/*	main_loop	*/
	/*	*********	*/
	while(1){
	
		/*	LCD-clear	*/
		RS = 0;  
		lcd_putchar(1);  
		RS = 1;

		
		/*	************	*/
		/*	  Control      	*/
		/*	************	*/
		
		if(time_minutes!=0 || time_seconds!=0) //NAND, if both are true, then go to else (this statement outputs 0)
		{
		
			time_seconds--;
			
			/*	*******************	*/
			/*	int to char convert	*/
			/*  ******************* */
			
			/*	Seconds				*/
			second_ones_diget = modulo(time_seconds, 10) + '0'; // Modulo will get the ones diget. "+ '0'" so we start in the correct place in the ASCII-table.  
			
			second_tens_diget = '0'; // Correct char-value to start with.
			if(time_seconds >= 10) // If seconds is less than ten, then the tens diget will be 0.
			{
				temp_variable = time_seconds / 10; // But if it's equal or more than ten, then the tens diget will be correct. xy/10=x (Rounding correct because of the nature of int.)
				second_tens_diget += temp_variable; // Temporary variable because of slow processor. 
			}
			
			/*	Minutes				*/
			minute_ones_diget = modulo(time_minutes, 10) + '0'; // Same as seconds.
			
			minute_tens_diget = '0';
			if(time_minutes >= 10)
			{
				temp_variable = time_minutes / 10;
				minute_tens_diget += temp_variable;
			}
			if(time_minutes > 59)
			{
				time_minutes = 0;
			}
			
			//	Display numbers on LCD.
			lcd_putline(0, "%c%c:%c%c", minute_tens_diget, minute_ones_diget, second_tens_diget, second_ones_diget);	
			
			// Delay a second
			for(i = 0; i < 9; i++){ 
				delay(100);  
			}
			
			/* When the button is pressed it gets inside the if statement. Then it waits for the user to let go of the button*/
			if(PORTB.7 == 1) 
			{
				while(PORTB.7 == 1);
				time_minutes = potentiometer();
				time_seconds = 1; //time_seconds is one do to the decrementation that occurs in the begining of the loop.
			}
			
			//Checks if seconds is 0, if so count down new minute. This is last because of unsigned int can't be negative and we want to show 00
			if(time_seconds == 0 && time_minutes != 0)
			{
				time_seconds = 60;
				time_minutes--;
			}			
		}else{// If timer is 00:00, then done!
				lcd_putline(0, "DONE!", 0, 0, 0, 0); 
				while(PORTB.7 == 0); // Wait for button to restart.
				delay(10);
				while(PORTB.7 == 1);
				time_minutes = potentiometer();
				time_seconds = 1;
			}	
		}
		//Exit
		while(1) nop();
	}

	
	
/* *********************************** */
/*            FUNCTIONS                */
/* *********************************** */

char   modulo(unsigned long dividend, long divisor)
{
/* 
	Classic modulo function
	see more @https://en.wikipedia.org/wiki/Modulo_operation
*/
	while(dividend >= divisor)
	{
		dividend -= divisor;
	}
	return dividend;
}


/* 
	Most borrowed from the already done potentiometer function, which is the ADvalue variables.
	We've changed the SCALEFACTOR and we also added display functions to show different values to the LCD, in this function.
	Also included the button. To break the while loop if pressed.
*/ 

long potentiometer(void)
{
	unsigned long advalue, potens=0;
	char show_minute_tens=0, show_minute_ones=0;
	int  i;
	delay(100); 
	
	// Loop
	while(1)
	{
		show_minute_tens = '0';
		show_minute_ones = '0';
		// LED Sampling indicator
		// Now measure the Voltage [V]  
		GO = 1;         // start AD
		while(GO);    // wait for done
		advalue  = ADRESH * 256;    // read result 10 bit
		advalue += ADRESL;
		// 1024 -> 50000 
		// multiply with integer scalefactor
		advalue *= SCALE_FACTOR;  
		potens   = advalue / 1000; // Potens becomes 50 when voltage is 5V which indicates the amount of minutes
		
		show_minute_tens = modulo(potens, 10); // Modulo 10 tells us the first digit of the number potens
		show_minute_ones = potens / 10; // Tells us the second digit in the potens variable
		show_minute_tens+= '0';// Creates a char from the first digit
		show_minute_ones+= '0';// Creates a char from the second digit
		lcd_putline(0, "%c%c Antal Minuter", show_minute_ones, show_minute_tens, 0, 0); //displays the digits
		// Delay is created
		for(i = 0; i < 10; i++)
		{
			delay(10);
		}
		
		// Clears the screen
		RS = 0;  
		lcd_putchar(1);  
		RS = 1;
		
		// Breaks the loop when customer is finished picking amount of minutes
		if(PORTB.7 == 1)
		{
			while(PORTB.7 == 1); 
			break;
		}
	}
	return potens;
}
 

/* 
	Predone LCD-functions
*/ 

void ADinit(void)
{
	// AD setup 
	ANSEL.2 = 1; // RA2 AN2 analog configurated
	TRISA.2=1;   // AN2 input

	ADCON1 = 0b0.101.0000; // AD conversion clock 'fosc/16'

	/* 
	 1.x.xxxx.x.x  ADRESH:ADRESL is 10 bit right justified
	 x.0.xxxx.x.x  Vref is Vdd
	 x.x.0010.x.x  Channel (AN2) 
	 x.x.xxxx.0.x  Go/!Done - start from program later
	 x.x.xxxx.x.1  Enable AD-converter
	*/
	ADCON0 = 0b1.0.0010.0.1; 
}

void lcd_putchar(char data)
{
	// must set LCD-mode before calling this function!
	// RS = 1 LCD in character-mode
	// RS = 0 LCD in command-mode
	// upper Nybble
	D7 = data.7;
	D6 = data.6;
	D5 = data.5;
	D4 = data.4;
	EN = 0;
	nop();
	EN = 1;
	delay(5);
	// lower Nybble
	D7 = data.3;
	D6 = data.2;
	D5 = data.1;
	D4 = data.0;
	EN = 0;
	nop();
	EN = 1;
	delay(5);
}

void lcd_putline(char start, const char * text,char variable_1, char variable_2, char variable_3, char variable_4)
{
/* 
	Changde to 4 variables.
*/
	RS = 0;  // LCD in command-mode
	lcd_putchar(start);  // move to text position
	RS = 1;  // LCD in character-mode
	char i, k, counter=0, variable=0,a,m,b;

	for(i = 0 ; ; i++)
	{
		k = text[i];
		if( k == '\0') return;
		if( k == '%')           // insert variable in string
		{
			i++;
			k = text[i];
			switch(counter) {

				case 0:
					variable = variable_1;
					break;
				case 1:
					variable = variable_2;
					break;
				case 2:
					variable = variable_3;
					break;
				case 3:
					variable = variable_4;
					break;
				/* We have 4 case statements */

			};
			counter++;
			switch(k)
			{
				case 'd':         // %d  signed 8bit
				if( variable.7 ==1) lcd_putchar('-');
				else lcd_putchar(' ');
				if( variable > 127) variable = -variable;  // no break!
				case 'u':         // %u unsigned 8bit
					a = variable/100;
					lcd_putchar('0'+a); // print 100's
					b = variable%100;
					a = b/10;
					lcd_putchar('0'+a); // print 10's
					a = b%10;
					lcd_putchar('0'+a); // print 1's
					break;
				case 'b':         // %b BINARY 8bit
					for( m = 0 ; m < 8 ; m++ )
					{
						if (variable.7 == 1) lcd_putchar('1');
						else lcd_putchar('0');
						variable = rl(variable);
					}
					break;
				case 'c':         // %c  'char'
					lcd_putchar(variable);
					break;
				case '%':
					lcd_putchar('%');
					break;
				default:          // not implemented
					lcd_putchar('!');
			}
		}else	lcd_putchar(k); // found end of string
	}
	return;  
}

void lcd_init( void ) // must be run once before using the display
{
  delay(40);  // give LCD time to settle
  RS = 0;     // LCD in command-mode
  lcd_putchar(0b0011.0011); /* LCD starts in 8 bit mode          */
  lcd_putchar(0b0011.0010); /* change to 4 bit mode              */
  lcd_putchar(0b00101000);  /* two line (8+8 chars in the row)   */ 
  lcd_putchar(0b00001100);  /* display on, cursor off, blink off */
  lcd_putchar(0b00000001);  /* display clear                     */
  lcd_putchar(0b00000110);  /* increment mode, shift off         */
  RS = 1;    // LCD in character-mode
             // initialization is done!
}


/* 
  Borrowed predone delay funtion.
*/

void delay(char millisec)
/* 
  Delays a multiple of 1 milliseconds at 4 MHz (16F690 internal clock)
  using the TMR0 timer 
*/
{
    OPTION = 2;  /* prescaler divide by 8        */
    do  {
        TMR0 = 0;
        while ( TMR0 < 125)   /* 125 * 8 = 1000  */
            ;
    } while ( -- millisec > 0);
}

/* *********************************** */
/*            HARDWARE                 */
/* *********************************** */

/*
         ___________  ___________
        |           \/           |
  +5V---|Vdd     16F690       Vss|---GND
        |RA5        RA0/AN0/(PGD)|
        |RA4            RA1/(PGC)|
        |RA3/!MCLR/(Vpp)  RA2/INT|->- Potentiometer
        |RC5/CCP              RC0|->-D4
        |RC4                  RC1|->-D5
  D7 -<-|RC3                  RC2|->-D6
        |RC6                  RB4|->- RS
        |RC7               RB5/Rx|
Butt -<-|RB7/Tx               RB6|->- EN
        |________________________| 

*/

/*
           LCD two lines, Line length 16 characters
           Internal ic: HD44780A00		   
           _______________
          |               |
          |         Vss  1|--- GND  
          |         Vdd  2|--- +5V
          |    Contrast  3|-<- Pot
          |          RS  4|-<- RB4
          |      RD/!WR  5|--- 0, GND
          |          EN  6|-<- RB6
          |          D0  7|
          |          D1  8|
          |          D2  9|
          |          D3 10|
          |          D4 11|-<- RC0
          |          D5 12|-<- RC1 
          |          D6 13|-<- RC2
          |          D7 14|-<- RC3 
          |_______________|						  
*/
