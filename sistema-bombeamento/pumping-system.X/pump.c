#include "pump.h"

void Power_On_Pump() 
{
    SetBit(&PORTC, 0);
}

void Power_Off_Pump() 
{
    ClearBit(&PORTC, 0);
}

void Tougle_Pump() 
{
    TougleBit(&PORTC, 0);
}

__bit Status_Pump()
{
    return CheckBit(PORTC, 0);
}
