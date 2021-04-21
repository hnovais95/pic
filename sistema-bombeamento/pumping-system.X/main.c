/*
 * File:   main.c
 * Author: Heitor Novais e Rafael Braga
 *
 * Created on 15 de Março de 2021, 23:44
 */

#define _XTAL_FREQ 10000000


/*******************************************************************
*   INCLUDES
*******************************************************************/
#include "configbits.h"
#include <xc.h>
#include "flexlcd.h"
#include "adc.h"
#include "regoperations.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "usertypes.h"
#include "debounce.h"
#include "system.h"        /* System funct/params, like osc/peripheral config */
#include "user.h"          /* User funct/params, such as InitApp */
#include "modbus.h"
#include <usart.h>
#include <stdint.h>        /* For uint8_t definition */


/*******************************************************************
*   DEFINES
*******************************************************************/
#define bt_operation_mode 0                 //LATBbits.LATB5
#define bt_on_off_pump 1                    //LATBbits.LATB2
#define bt_incr_min_lv1 3                   //LATBbits.LATB3
#define bt_incr_min_lv2 4                   //LATBbits.LATB4
#define led_operation_mode LATDbits.LATD0
#define led_test LATDbits.LATD1             //test programming logic
#define pump_relay 0                        //LATCbits.LATC0


/*******************************************************************
*   GLOBAL VARIABLES
*******************************************************************/
// Filters
int filter_bt_operation_mode = t_filter;
int filter_bt_on_off_pump = t_filter;
int filter_bt_min_lv1_incr = t_filter;
int filter_bt_min_lv2_incr = t_filter;

// Flags
int bt_operation_mode_press = 0;
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

// Auxiliary variables
int last_pump_status_received;
int last_operation_mode_received;


/*******************************************************************
*   CONSTANTS
*******************************************************************/
uint8 INCREMENT = 10;


/*******************************************************************
*   PROTOTYPES
*******************************************************************/
void interrupt isr(void);
void TouglePump();
void Automatic_Mode();
void Manual_Mode();
void Change_Mode();
void Min_Lv1_Incr();
void Min_Lv2_Incr();
void Read_Level();
void Sync_Registers();
void Receive_Message();
void Set_Display_Message();
void Show_Display(lcd display);
void Refresh_Screen();
void Initialize_Tanks();
void Configure_External_Interrupt();
void Configure_Timer_Interrupt();
void Configure_Registers();


/*******************************************************************
*   MAIN
*******************************************************************/
void main(void) {
    Configure_Registers();    
    adc_init();
    Lcd_Init();
    Lcd_Cmd(LCD_CURSOR_OFF);
    Initialize_Tanks();
    
    // Initialize modbus communication
    OpnUSART();
    ConfigInterrupts();
 
    while(1)
    {
        CLRWDT();
        
        last_pump_status_received = holdingReg[0];
        last_operation_mode_received = holdingReg[1];
        
        if (modbusMessage)
        {
            decodeIt();
            Receive_Message();
        }

        Read_Level();
        
        Debounce(PORTB, bt_operation_mode, &bt_operation_mode_press, &filter_bt_operation_mode, Change_Mode); // If button is pressed change operation mode
        
        if (operation == AUTOMATIC) Automatic_Mode(); else Manual_Mode();
        
        Debounce(PORTB, bt_incr_min_lv1, &bt_min_lv1_incr_press, &filter_bt_min_lv1_incr, Min_Lv1_Incr); // If button is pressed increase minimum tank 1 level
        Debounce(PORTB, bt_incr_min_lv2, &bt_min_lv2_incr_press, &filter_bt_min_lv2_incr, Min_Lv2_Incr); // If button is pressed increase minimum tank 2 level
        
        if (refresh_display) Refresh_Screen();
    }
    return;
}


/*******************************************************************
*   INTERRUPT FUNCTION
*******************************************************************/
void interrupt isr(void)
{   
    // Timer interrupt
    if (PIR1bits.TMR1IF == 1)
    {
        PIR1bits.TMR1IF = 0;
        TMR1L = 0x2C;   // reload TMR1L;
        TMR1H = 0xCF;   // reload TMR1H;
        count_screen++;
        count_refresh_display++;
                
        // Switches screen mode every t1 seconds
        if (count_screen == 100) // ~2s
        {            
            count_screen = 0;
            ++screen;
            if (screen == 4) screen = 0;
        }
        
        // Refresh display every t2 seconds
        if (count_refresh_display == 25) // ~500ms
        {
            count_refresh_display = 0;
            refresh_display = 1;
        }
    }
    
    // USART receive interrupt flag has been set
    if (PIR1bits.RCIF)
    {
        if ((!endOfMessage) && (!newMessage))
        {
            if (PIR1bits.TXIF)   // check if the TXREG is empty
            {
                received[z] = ReceiveBuffer;
                z++;
                timerCount = 0;
            }
        }

        if (newMessage) 
        {
            OpenTmr0();
            if (PIR1bits.TXIF)   // check if the TXREG is empty
            { 
                received[z] = ReceiveBuffer;
                z++;
                newMessage = 0;
                endOfMessage = 0;
                messageLength = 0;
                modbusMessage = 0;
                timerCount = 0;
                return;
            }
        }
        PIR1bits.RCIF = 0;
    }
    
    //TMR0 interrupt flag
    if (INTCONbits.TMR0IF)    
    { 
        modbusDelay();  //Resets timer for 1.04ms
        timerCount++;
        if (timerCount > 4)
        {
            endOfMessage = 1;
            newMessage = 1;
            messageLength = z;
            modbusMessage = 1;
            for (z = (messageLength); z != 125; z++)    //Clear rest of message
            {
              received[z] = 0;
            }
            z = 0;
            T0CONbits.TMR0ON = 0; //Close timer0
            timerCount = 0;
        }
        INTCONbits.TMR0IF = 0;  // Clear flag
    }
}


/*******************************************************************
*   FUNCTIONS
*******************************************************************/
void Tougle_Pump()
{
    TougleBit(&LATC, pump_relay);
}

void Automatic_Mode()
{
    bool isFull = tank2.level >= tank2.max_level ? true : false;      
    if ((tank1.level > tank1.min_level) && (tank2.level < tank2.min_level)) 
    {
        SetBit(&LATC, pump_relay);
    }
    else if (tank1.level <= tank1.min_level || isFull) 
    {
        ClearBit(&LATC, pump_relay);
    }
}

void Manual_Mode()
{
    Debounce(PORTB, bt_on_off_pump, &bt_on_off_pump_press, &filter_bt_on_off_pump, Tougle_Pump);
}

void Change_Mode()
{    
    led_operation_mode = ~led_operation_mode;    
    if (operation == AUTOMATIC) operation = MANUAL; 
    else operation = AUTOMATIC;
    
    holdingReg[1] = operation;
}

void Min_Lv1_Incr()
{
    screen = MIN_LEVEL;
    Refresh_Screen();    
    TMR1L = 0x2C; // LSB 
    TMR1H = 0xCF; // HSB
    count_screen = -250;

    if ((tank1.min_level + INCREMENT) <= 100) tank1.min_level += INCREMENT;
    else tank1.min_level = 0; // Increments from zero
    
    holdingReg[2] = tank1.min_level;
}

void Min_Lv2_Incr()
{
    screen = MIN_LEVEL;
    Refresh_Screen();
    TMR1L = 0x2C; // LSB 
    TMR1H = 0xCF; // HSB
    count_screen = -250;

    if ((tank2.min_level + INCREMENT) <= 100) tank2.min_level += INCREMENT;
    else tank2.min_level = 0; // Increments from zero
    
    holdingReg[3] = tank2.min_level;
}

void Read_Level()
{
    int adcTank1 = adc_amostra(0);
    int adcTank2 = adc_amostra(1);
    
    tank1.level = (adcTank1 / 10.23); // %
    tank2.level = (adcTank2 / 10.23); // %
    
    holdingReg[4] = tank1.level;
    holdingReg[5] = tank2.level;
}

void Sync_Registers()
{    
    // Sync pump status
    if ((last_pump_status_received == holdingReg[0]) && (holdingReg[0] != CheckBit(LATC, pump_relay)))
        holdingReg[0] = CheckBit(LATC, pump_relay);
    
    // Sync operation mode
    if ((last_operation_mode_received == holdingReg[1]) && (holdingReg[1] != operation))
        holdingReg[1] = operation;
}

void Receive_Message()
{
    Sync_Registers();
    if (holdingReg[0] == 1) SetBit(&LATC, pump_relay);
    else ClearBit(&LATC, pump_relay);
    operation = holdingReg[1]; 
    tank1.min_level = holdingReg[2];
    tank2.min_level = holdingReg[3];
}

void Set_Display_Message()
{
    if (screen == CURRENT_LEVEL)
    {
        sprintf(display.line0, "CURRENT LV1: %d%%  ", (int)tank1.level);
        sprintf(display.line1, "CURRENT LV2: %d%%  ", (int)tank2.level);
    }
    else if (screen == OPERATION_MODE)
    {
        strcpy(display.line0, "OPERATION MODE: ");
        strcpy(display.line1, operation == AUTOMATIC ? "AUTOMATIC       " : "MANUAL          ");
    }
    else if (screen == MIN_LEVEL)
    {
        sprintf(display.line0, "LV_MIN_1: %d%%   ", (int)tank1.min_level);
        sprintf(display.line1, "LV_MIN_2: %d%%   ", (int)tank2.min_level);
    }
    else if (screen == MODBUS)
    {         
        strcpy(display.line0, "MODBUS:         ");
        sprintf(display.line1, "%02X %02X %02X %02X     ", holdingReg[0], holdingReg[1], holdingReg[2], holdingReg[3]);
    }
}

void Show_Display(lcd display)
{ 
    Lcd_Out(1, 0, display.line0);
    Lcd_Out(2, 0, display.line1);
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
    tank1.min_level = 0;
    tank1.max_level = 100;
    // Tank 2
    tank2.level = 0;
    tank2.min_level = 0;
    tank2.max_level = 90;    
}

void Configure_External_Interrupt()
{
    /*INTCON2bits.INTEDG0 = 0; // 0 = Interrupt on falling edge
    INTCONbits.INT0IF = 0; // The INT0 external interrupt occurred (must be cleared in software) 
    INTCONbits.INT0IE = 1; // 1 = Enables the INT0 external interrupt*/
}

void Configure_Timer_Interrupt()
{
    PIE1bits.TMR1IE = 1;
    PIR1bits.TMR1IF = 0;
    TMR1L = 0x2C; // LSB 
    TMR1H = 0xCF; // HSB
    T1CON = 0b00100001; // timer, prescaler 1:4, internal clock;
}

void Configure_Registers()
{
    INTCON2bits.RBPU = 0; // 0 = PORTB pull-ups are enabled by individual port latch values 
    
    // Input or Output
    TRISA = 0xff;
    TRISB = 0x3f;
    TRISC = 0x00;
    TRISD = 0x00;
    TRISE = 0x00;
    // Clear ports
    PORTA = 0; LATA = 0;
    PORTB = 0; LATB = 0;
    PORTC = 0; LATC = 0;
    PORTD = 0; LATD = 0;
    PORTE = 0; LATE = 0;    
    
    PEIE = 1;
    ei();
    Configure_External_Interrupt();
    Configure_Timer_Interrupt();
}