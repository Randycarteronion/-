#ifndef _BSP_FLAME_H_
#define _BSP_FLAME_H_
 
#include "gd32f4xx.h"
 
 
#define RCU_FLAME_GPIO_AO    RCU_GPIOC
#define RCU_FLAME_GPIO_DO    RCU_GPIOE


#define RCU_FLAME_DMA     	RCU_DMA1
#define PORT_DMA        	DMA1
#define CHANNEL_DMA     	DMA_CH0

#define RCU_FLAME_ADC     	RCU_ADC0
#define PORT_ADC        	ADC0
#define CHANNEL_ADC     	ADC_CHANNEL_11

#define PORT_FLAME_AO     GPIOC
#define GPIO_FLAME_AO     GPIO_PIN_1

#define PORT_FLAME_DO     GPIOE
#define GPIO_FLAME_DO     GPIO_PIN_1

 //采样次数
#define SAMPLES         30
//采样通道数 
#define CHANNEL_NUM     1


extern uint16_t gt_adc_val[ SAMPLES ][ CHANNEL_NUM ];  //DMA缓冲区
 

void ADC_DMA_Init(void);
unsigned int Get_Adc_Dma_Value(char CHx);
unsigned int Get_FLAME_Percentage_value(void);
unsigned char Get_FLAME_Do_value(void);

#endif