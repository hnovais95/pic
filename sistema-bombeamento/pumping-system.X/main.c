/*
 * File:   main.c
 * Author: heitor
 *
 * Created on 28 de Junho de 2020, 15:26
 */

// CONFIG
#pragma config FOSC = EXTRC        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

#include <xc.h>
#include <stdio.h>
#include <stdbool.h>
#include "config.h"
#include "adc.h"
#include "lcd.h"

void PowerOnPump() {
    PORTCbits.RC0 = 1;
}

void PowerOffPump() {
    PORTCbits.RC0 = 0;
}

void main(void) {  
    TRISC = 0x00;
    TRISD = 0x00;
    TRISE = 0x00;
    
    ADC_Initialize();
    Lcd_Start();
    
    while(1)
    {
        int adcTank1 = ADC_Read(0);
        int adcTank2 = ADC_Read(1);
        float lvTank1 = (adcTank1 / 10.23); // %
        float lvTank2 = (adcTank2 / 10.23); // %
        
        bool isFull = lvTank2 > 95 ? true : false;      
        if(lvTank1 > 20 && lvTank2 < 80)
            PowerOnPump();
        else if (lvTank1 <= 20 || isFull)
            PowerOffPump();
        
        char strLvTank1 [10], strLvTank2 [10];
        sprintf(strLvTank1, "Lv1: %d%%", (int)lvTank1);
        sprintf(strLvTank2, "Lv2: %d%%", (int)lvTank2);
        
        Lcd_Clear();
        Lcd_Set_Cursor(1,1);
        Lcd_Print_String(strLvTank1);
        Lcd_Set_Cursor(2,1);
        Lcd_Print_String(strLvTank2);
        __delay_ms(100);
    }
    return;
}
