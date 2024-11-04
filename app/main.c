/*
 * 立创开发板软硬件资料与相关扩展板软硬件资料官网全部开源
 * 开发板官网：www.lckfb.com
 * 技术支持常驻论坛，任何技术问题欢迎随时交流学习
 * 立创论坛：club.szlcsc.com
 * 关注bilibili账号：【立创开发板】，掌握我们的最新动态！
 * 不靠卖板赚钱，以培养中国工程师为己任
 * Change Logs:
 * Date           Author       Notes
 * 2023-11-02     LCKFB-yzh    first version
 */
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


