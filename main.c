// PIC16F684 Configuration Bit Settings

// 'C' source line config statements

// CONFIG
#pragma config FOSC = INTOSCIO  				// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA4/OSC2/CLKOUT pin, I/O function on RA5/OSC1/CLKIN)
#pragma config WDTE = OFF       				// Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      				// Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      				// MCLR Pin Function Select bit (MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         				// Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        				// Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      				// Brown Out Detect (BOR disabled)
#pragma config IESO = OFF       				// Internal External Switchover bit (Internal External Switchover mode is disabled)
#pragma config FCMEN = OFF      				// Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <pic.h>
#include <pic16f684.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define _XTAL_FREQ 	   4000000
#define diode_freq     5
#define rattling   	   50
#define n_calibrate    10
#define deleyCelibrate 40000
#define countBlinkRes  3

#define FALSE (0)
#define TRUE  !FALSE
typedef char bool;

bool y_flag = FALSE;
bool d_flag = TRUE;
bool interFlag = FALSE;
unsigned int tim0count, tim01count, adcData, level;
unsigned long counter;
short int sensitivity = 20;

void interrupt timer0(void) {
    tim0count++;
    INTCONbits.TMR0IF = 0;                          		// clear flag
    return;
} 

int Calibrator() {
    int maxValue = 0;
    int arrData[n_calibrate]={};
    for(int i = 0; i < n_calibrate; i++) {
        arrData[i] = readADC();                     		// write 10-n values ADC
        if(arrData[i] > maxValue) {
        	maxValue = arrData[i];				// max value
        }
    }
    return maxvalueDat;
}

void Blink() {
    for(int i = 0; i < countBlinkRes; i++){
    PORTCbits.RC1 = 1;
    __delay_ms(200);
    PORTCbits.RC1 = 0;
    __delay_ms(500);
    }
}

void Blink_2() {
    for(int i = 0; i < countBlinkRes + 2; i++){
    PORTCbits.RC0 = 1;
    __delay_ms(200);
    PORTCbits.RC0 = 0;
    __delay_ms(500);
    }
}

void resetCalibrator() {
     Blink();
     level = Calibrator();
     Blink();
}

int readADC() {
     ADCON0bits.GO = 1;                         			// run ADC
     while(ADCON0bits.GO);                      			// waiting conversion to finish
     adcData = (unsigned int)ADRESH << 8;       			// shifting the values
     return adcData |= ADRESL;                			// log AND, get the entire value in type int
}

void reset() {
    INTCONbits.T0IF = 0;                         			// resetting all parameters
    PORTCbits.RC0 = 0;
    PORTCbits.RC1 = 0;
    INTCONbits.TMR0IE = 0;
    interFlag = FALSE;
    tim01count = 0;
}

void main(void) { 
     TRISA = 0b00000011;                         			// A0-A1 analog input
     PORTA = 0b11111100; 
     TRISC = 0b11111100;                         			// RC1-RC2 digital output 
     PORTC = 0b11111100;            
     ANSEL = 0b00000011;                         			// choose analog - digital pins
    
     INTCONbits.GIE = 1;                         			// enable all global interrupts
     INTCONbits.PEIE = 1;                        			// enable periferal int
     INTCONbits.TMR0IE = 0;                      			// enable timer interrupt
     INTCONbits.TMR0IF = 0;                      			// celar timer overflow flag
     INTCONbits.T0IF = 0;                        			// interrupt timer       
     TMR0 = 0;                                   			// register data timer
   
     OPTION_REGbits.T0CS = 0;                    			// T0CS - 0 - internal clock
     OPTION_REGbits.T0SE = 0;                    			// T0SE - 0 - edge selection
     OPTION_REGbits.PSA = 0;                     			// prescalar assigned to timer0
     OPTION_REGbits.PS0 = 1;                     			// select prescaler 1:256 PS0-PS2
     OPTION_REGbits.PS1 = 1; 
     OPTION_REGbits.PS2 = 1; 
                                                  		// ADCON0bits.CHS0 - CHS2 = 0 (standart) CHANNEL A0
     ADCON0bits.VCFG = 1;                         		// Vref on
     ADCON0bits.ADON = 1;                         		// ADC enable bit
     ADCON0bits.ADFM = 1;                         		// right justifield
     ADCON1bits.ADCS0 = 1;                        		// ADC clock source - internal Frc
     ADCON1bits.ADCS1 = 1;                        		// ADCS0 and ADCS1 = 1
   
     ei();
      __delay_us(1000);
      Blink_2();
      resetCalibrator();

      while(TRUE) {
          if ((readADC() >= (level + sensitivity)) && !interFlag) {   // compare with the level
              if(!interFlag) {                         		// checking the presence of the interrupt tim0 flag= 1;                          
                  INTCONbits.TMR0IE = 1;                  		// enter here only after pressing the button
                  interFlag = TRUE;    
              }
          }
          
          if(tim0count >= diode_freq) {                 		// swap led state with frequency 
                PORTCbits.RC0 = y_flag;
                tim01count++;
                if(tim01count > 3) {
                    PORTCbits.RC0 = !y_flag;
                    tim01count = 0;
                }
                tim0count = 0;
          } 
          while(!RC2) {                                		// button click processing
              counter++;
                  if(counter >= rattling) {            		// processing rattling button
                  reset();
              }
              if(counter >= deleyCelibrate) {           		// processing button reset
                  resetCalibrator();                  		// calibration
              }
          }
          counter = 0;                                		// reseting counter of press button
    }
return;
}
