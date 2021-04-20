
# 1 "debounce.c"

# 2 "regoperations.h"
void SetBit(unsigned *port, unsigned pin);

void ClearBit(unsigned *port, unsigned pin);

void TougleBit(unsigned *port, unsigned pin);

int CheckBit(unsigned port, unsigned pin);

# 5 "debounce.h"
void Debounce(unsigned port, unsigned pin, unsigned *bt_press, unsigned *filter, void (*ptr_f)(void));

# 3 "debounce.c"
void Debounce(unsigned port, unsigned pin, unsigned *bt_press, unsigned *filter, void (*ptr_f)(void))
{
if (!CheckBit(port, pin))
{

if (!(*bt_press))
{
if (*filter > 0)
{
(*filter)--;
}
else
{
*bt_press = 1;
ptr_f();
}
}
}
else
{
*filter = 10;;
*bt_press = 0;
}
}
