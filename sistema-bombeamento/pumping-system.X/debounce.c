#include "debounce.h"

void Debounce(unsigned port, unsigned pin, unsigned *bt_press, unsigned *filter, void (*ptr_f)(void))
{
    if (!CheckBit(port, pin))             // O bot�o est� pressionado ?
    {
        // sim,
        if (!(*bt_press))      // O bot�o j� foi tratado ?
        {            // n�o.
            if (*filter > 0)   // Fim do filtro do bot�o ?
            {
                (*filter)--;   // N�o, ent�o decrementa o filtro
            }
            else 
            {         // Sim, Faz a a��o do bot�o
                *bt_press = 1;   // Marca que o bot�o est� pressionado
                ptr_f();
            }
        }
    }
    else
    {               // bot�o liberado
        *filter = t_filter; // inicia o filtro do bot�o 
        *bt_press = 0;
    }
}