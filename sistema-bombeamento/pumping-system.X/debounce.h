#include "regoperations.h"

#define t_filter 10;

void Debounce(unsigned port, unsigned pin, unsigned *bt_press, unsigned *filter, void (*ptr_f)(void));