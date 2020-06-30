/*
 * File:   main.c
 * Author: heitor
 *
 * Created on 15 de Junho de 2020, 20:38
 */

// PIC16F628A Configuration Bit Settings

// 'C' source line config statements

// CONFIG
#pragma config FOSC = INTOSCIO  // Oscillator Selection bits (INTOSC oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = ON       // RA5/MCLR/VPP Pin Function Select bit (RA5/MCLR/VPP pin function is MCLR)
#pragma config BOREN = ON       // Brown-out Detect Enable bit (BOD enabled)
#pragma config LVP = ON         // Low-Voltage Programming Enable bit (RB4/PGM pin has PGM function, low-voltage programming enabled)
#pragma config CPD = OFF        // Data EE Memory Code Protection bit (Data memory code protection off)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.
#include <xc.h>

#define _XTAL_FREQ 4000000

void main(void) {
    TRISA = 0xff;
    TRISB = 0x00;
    PORTA = 0b00001111;
    PORTB = 0b00001111;
    
    while(1) {
        PORTBbits.RB0 = PORTAbits.RA1;
        PORTBbits.RB1 = PORTAbits.RA2;
        PORTBbits.RB2 = PORTAbits.RA3;
        PORTBbits.RB3 = PORTAbits.RA4;
        __delay_ms(500);
    }
    
    return;
}
