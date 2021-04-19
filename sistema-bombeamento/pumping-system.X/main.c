/*
 * File:   main.c
 * Author: Heitor Novais e Rafael Braga
 *
 * Created on 15 de Março de 2021, 23:44
 */

/*******************************************************************
*   INCLUDES
*******************************************************************/
#include <xc.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "config.h"
#include "adc.h"
#include "lcd.h"
#include "pump.h"
#include "usertypes.h"
#include "debounce.h"


/*******************************************************************
*   DEFINES
*******************************************************************/
#define bt_operation_mode PORTBbits.RB0
#define bt_on_off_pump PORTBbits.RB1
#define bt_incr_min_lv1 PORTBbits.RB3
#define bt_incr_min_lv2 PORTBbits.RB4
#define led_operation_mode PORTDbits.RD0



/*******************************************************************
*   GLOBAL VARIABLES
*******************************************************************/
// Filters
int filter_bt_on_off_pump = t_filter;
int filter_bt_min_lv1_incr = t_filter;
int filter_bt_min_lv2_incr = t_filter;

// Flags
int bt_on_off_pump_press = 0;
int bt_min_lv1_incr_press = 0;
int bt_min_lv2_incr_press = 0;
volatile __bit refresh_display = 0;

// Counters
volatile int count_screen = 0;
volatile unsigned int count_refresh_display = 0;

// User types
volatile operation_mode operation = AUTOMATIC;
volatile screen_mode screen = CURRENT_LEVEL;
volatile tank tank1, tank2;
volatile lcd display;



/*******************************************************************
*   CONSTANTS
*******************************************************************/
uint8 INCREMENT = 10;



/*******************************************************************
*   PROTOTYPES
*******************************************************************/
void Set_Display_Message();
void Min_Lv1_Incr();
void Min_Lv2_Incr();
void Automatic_Mode();
void Manual_Mode();
void Read_Level();
void Refresh_Screen();
void Initialize_Tanks();
void Configure_External_Interrupt();
void Configure_Timer_Interrupt();
void Configure_Registers();



/*******************************************************************
*   INTERRUPT FUNCTION
*******************************************************************/
void interrupt isr(void)
{   
    // Timer interrupt
    if (INTCONbits.TMR0IF == 1)
    {
        INTCONbits.TMR0IF = 0;        
        TMR0 = 131;
        count_screen++;
        count_refresh_display++;
                
        // Switches screen mode every t1 seconds
        if (count_screen == 500) // ~2s
        {            
            count_screen = 0;
            ++screen;
            if (screen == 3) screen = 0;
        }
        
        // Refresh display every t2 seconds
        if (count_refresh_display == 125) // ~500ms
        {
            count_refresh_display = 0;
            refresh_display = 1;
        }
    }
    
    // Pin RB0 interrupt
    if (INTCONbits.INTF == 1)
    {
        INTCONbits.INTF = 0;
        led_operation_mode = ~led_operation_mode;
        
        if (operation == AUTOMATIC) operation = MANUAL; else operation = AUTOMATIC;
        if ((operation == MANUAL) && (Status_Pump() == 1)) Power_Off_Pump(); // If switched to manual mode and pump is on, turn off the pump
    }
}



/*******************************************************************
*   MAIN
*******************************************************************/
void main(void) {
    Configure_Registers();    
    ADC_Initialize();
    Lcd_Start();
    
    Initialize_Tanks();    
 
    while(1)
    {
        CLRWDT();
        
        Read_Level();
        
        if (operation == AUTOMATIC) Automatic_Mode(); else Manual_Mode();
        
        Debounce(PORTB, 3, &bt_min_lv1_incr_press, &filter_bt_min_lv1_incr, Min_Lv1_Incr); // If button is pressed increase minimum tank 1 level
        Debounce(PORTB, 4, &bt_min_lv2_incr_press, &filter_bt_min_lv2_incr, Min_Lv2_Incr); // If button is pressed increase minimum tank 2 level
        
        if (refresh_display) Refresh_Screen();
    }
    return;
}



/*******************************************************************
*   FUNCTIONS
*******************************************************************/
void Set_Display_Message()
{
    if (screen == CURRENT_LEVEL)
    {
        sprintf(display.line0, "CURRENT LV1: %d%%", (int)tank1.level);
        sprintf(display.line1, "CURRENT LV2: %d%%", (int)tank2.level);
    }
    else if (screen == OPERATION_MODE)
    {
        strcpy(display.line0, "OPERATION MODE:");
        strcpy(display.line1, operation == AUTOMATIC ? "AUTOMATIC" : "MANUAL");
    }
    else if (screen == MIN_LEVEL)
    {
        sprintf(display.line0, "LV_MIN_1: %d%%", (int)tank1.min_level);
        sprintf(display.line1, "LV_MIN_2: %d%%", (int)tank2.min_level);
    }
}


void Min_Lv1_Incr()
{
    screen = MIN_LEVEL;
    Refresh_Screen();
    TMR0 = 131;
    count_screen = -250;

    if ((tank1.min_level + INCREMENT) <= 100) tank1.min_level += INCREMENT;
    else tank1.min_level = 0; // Increments from zero
}


void Min_Lv2_Incr()
{
    screen = MIN_LEVEL;
    Refresh_Screen();
    TMR0 = 131;
    count_screen = -250;

    if ((tank2.min_level + INCREMENT) <= 100) tank2.min_level += INCREMENT;
    else tank2.min_level = 0; // Increments from zero 
}


void Automatic_Mode()
{
    bool isFull = tank2.level >= tank2.max_level ? true : false;      
    if ((tank1.level > tank1.min_level) && (tank2.level < tank2.min_level)) Power_On_Pump();
    else if (tank1.level <= tank1.min_level || isFull) Power_Off_Pump();
}


void Manual_Mode()
{
    Debounce(PORTB, 1, &bt_on_off_pump_press, &filter_bt_on_off_pump, Tougle_Pump);
}


void Read_Level()
{
    int adcTank1 = ADC_Read(0);
    int adcTank2 = ADC_Read(1);
    
    tank1.level = (adcTank1 / 10.23); // %
    tank2.level = (adcTank2 / 10.23); // %
}


void Refresh_Screen()
{
    Set_Display_Message();
    Show_Display(display);
    refresh_display = 0;
}


void Initialize_Tanks()
{
    // Tank 1
    tank1.level = 0;
    tank1.min_level = 10;
    tank1.max_level = 100;
    // Tank 2
    tank2.level = 0;
    tank2.min_level = 50;
    tank2.max_level = 90;    
}


void Configure_External_Interrupt()
{    
    INTCONbits.INTE = 1;
    INTCONbits.INTF = 0;
}


void Configure_Timer_Interrupt()
{   
    INTCONbits.TMR0IE = 1;
    INTCONbits.TMR0IF = 0;    
    OPTION_REG = 0b11000101; // 8bit and 1:64 Prescale
    TMR0 = 131;
}

void Configure_Registers()
{
    TRISB = 0xFF;
    TRISC = 0x00;
    TRISD = 0x00;
    TRISE = 0x00;
    
    ei();
    Configure_External_Interrupt();
    Configure_Timer_Interrupt();
}