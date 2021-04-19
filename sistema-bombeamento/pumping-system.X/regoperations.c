#include "regoperations.h"

void SetBit(unsigned *port, unsigned pin)
{
    (*port) |= (1UL << pin);
}

void ClearBit(unsigned *port, unsigned pin)
{
    (*port) &= ~(1UL << pin);
}

void TougleBit(unsigned *port, unsigned pin)
{
    (*port) ^= (1UL << pin);
}

int CheckBit(unsigned port, unsigned pin)
{
    return ((port >> pin) & 1U);
}



