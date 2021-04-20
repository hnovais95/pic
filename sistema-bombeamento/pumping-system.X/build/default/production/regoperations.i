
# 1 "regoperations.c"

# 2 "regoperations.h"
void SetBit(unsigned *port, unsigned pin);

void ClearBit(unsigned *port, unsigned pin);

void TougleBit(unsigned *port, unsigned pin);

int CheckBit(unsigned port, unsigned pin);

# 3 "regoperations.c"
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

