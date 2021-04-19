#include "debounce.h"

void Debounce(unsigned port, unsigned pin, unsigned *bt_press, unsigned *filter, void (*ptr_f)(void))
{
    if (!CheckBit(port, pin))             // O botão está pressionado ?
    {
        // sim,
        if (!(*bt_press))      // O botão já foi tratado ?
        {            // não.
            if (*filter > 0)   // Fim do filtro do botão ?
            {
                (*filter)--;   // Não, então decrementa o filtro
            }
            else 
            {         // Sim, Faz a ação do botão
                *bt_press = 1;   // Marca que o botão está pressionado
                ptr_f();
            }
        }
    }
    else
    {               // botão liberado
        *filter = t_filter; // inicia o filtro do botão 
        *bt_press = 0;
    }
}