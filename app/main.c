
#include "board.h"
#include "bsp_flame.h"

int main(void)
{
    board_init();
	bsp_uart_init();
	
    ADC_DMA_Init();
    while(1)
    {
        printf("flame = %d%%\r\n",Get_FLAME_Percentage_value());
        delay_ms(500);
    }
}


