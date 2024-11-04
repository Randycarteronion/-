/********************************************************************************
  * 文 件 名: bsp_flame.c
  * 版 本 号: 初版
  * 修改作者: LC
  * 修改日期: 2023年04月06日
  * 功能介绍:          
  ******************************************************************************
  * 注意事项:
*********************************************************************************/


#include "bsp_flame.h"
#include "board.h"


 //DMA缓冲区
uint16_t gt_adc_val[ SAMPLES ][ CHANNEL_NUM ]; 


/******************************************************************
 * 函 数 名 称：ADC_DMA_Init
 * 函 数 说 明：初始化ADC+DMA功能
 * 函 数 形 参：无
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：无
******************************************************************/
void ADC_DMA_Init(void)
{
	/* DMA初始化功能结构体定义 */
	dma_single_data_parameter_struct dma_single_data_parameter;

	/* 使能引脚时钟 */
	rcu_periph_clock_enable(RCU_FLAME_GPIO_AO);                    
	rcu_periph_clock_enable(RCU_FLAME_GPIO_DO); 
	/* 使能ADC时钟 */
	rcu_periph_clock_enable(RCU_FLAME_ADC);                

	/* 使能DMA时钟 */
	rcu_periph_clock_enable(RCU_FLAME_DMA);
 
        /*        配置ADC时钟        */ 
    adc_clock_config(ADC_ADCCK_PCLK2_DIV4);        
   /*        配置DO为输入模式        */
    gpio_mode_set(PORT_FLAME_DO, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_FLAME_DO); 
        /*        配置AO为浮空模拟输入模式        */
    gpio_mode_set(PORT_FLAME_AO, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_FLAME_AO); // PC1 : ADC012_IN11 
 
        /*        配置ADC为独立模式        */
    adc_sync_mode_config(ADC_SYNC_MODE_INDEPENDENT);
        
    /*        使能连续转换模式        */
    adc_special_function_config(PORT_ADC, ADC_CONTINUOUS_MODE, ENABLE);
         
    /*        使能扫描模式        */
    adc_special_function_config(PORT_ADC, ADC_SCAN_MODE, ENABLE);
   
        /*        数据右对齐        */        
    adc_data_alignment_config(PORT_ADC, ADC_DATAALIGN_RIGHT);
    
    /*        ADC0设置为规则组  一共使用 CHANNEL_NUM 个通道                */  
    adc_channel_length_config(PORT_ADC, ADC_ROUTINE_CHANNEL, CHANNEL_NUM);
                
    /*        ADC规则通道配置：ADC0的通道11的扫描顺序为0；采样时间：15个周期                */  
        /*        DMA开启之后 gt_adc_val[x][0] = PC1的数据   */
    adc_routine_channel_config(PORT_ADC, 0, CHANNEL_ADC, ADC_SAMPLETIME_15);//PC1
 
        /*        ADC0设置为12位分辨率                */  
        adc_resolution_config(PORT_ADC, ADC_RESOLUTION_12B); 
        
        /*        ADC外部触发禁用, 即只能使用软件触发                */  
    adc_external_trigger_config(PORT_ADC, ADC_ROUTINE_CHANNEL, EXTERNAL_TRIGGER_DISABLE);
    
    /*        使能规则组通道每转换完成一个就发送一次DMA请求                */  
    adc_dma_request_after_last_enable(PORT_ADC);  
    
	/*        使能DMA请求                */  
	adc_dma_mode_enable(PORT_ADC);

	/*        使能ADC                */          
	adc_enable(PORT_ADC);

	/*        等待ADC稳定                */  
	delay_ms(1);
    
        /*        开启ADC自校准                */
    adc_calibration_enable(PORT_ADC); 
 
    /*        清除 DMA通道0 之前配置         */
    dma_deinit(PORT_DMA, CHANNEL_DMA);
   
	/*        DMA初始化配置         */
	dma_single_data_parameter.periph_addr = (uint32_t)(&ADC_RDATA(PORT_ADC));         //设置DMA传输的外设地址为ADC0基地址
	dma_single_data_parameter.periph_inc = DMA_PERIPH_INCREASE_DISABLE;                         //关闭外设地址自增
	dma_single_data_parameter.memory0_addr = (uint32_t)(gt_adc_val);                         //设置DMA传输的内存地址为 gt_adc_val数组
	dma_single_data_parameter.memory_inc = DMA_MEMORY_INCREASE_ENABLE;                         //开启内存地址自增（因为不止一个通道）
	dma_single_data_parameter.periph_memory_width = DMA_PERIPH_WIDTH_16BIT;             //传输的数据位 为 16位
	dma_single_data_parameter.direction = DMA_PERIPH_TO_MEMORY;                                         //DMA传输方向为 外设往内存
	dma_single_data_parameter.number = SAMPLES * CHANNEL_NUM;                                     //传输的数据长度为：每个通道采集30次 * 1个通道
	dma_single_data_parameter.priority = DMA_PRIORITY_HIGH;                                                 //设置高优先级
	dma_single_data_mode_init(PORT_DMA, CHANNEL_DMA, &dma_single_data_parameter);//将配置保存至DMA1的通道0
	
	/*        DMA通道外设选择                */
	/*        数据手册的195页根据PERIEN[2:0]值确定第三个参数，例是100 则为DMA_SUBPERI4          例是010 则为DMA_SUBPERI2         */
	/*        我们是ADC0功能，PERIEN[2:0]值为000，故为DMA_SUBPERI0                */
	dma_channel_subperipheral_select(PORT_DMA, CHANNEL_DMA, DMA_SUBPERI0);

	/*        使能DMA1通道0循环模式                */
	dma_circulation_enable(PORT_DMA, CHANNEL_DMA);

	/*        启动DMA1的通道0功能                */
	dma_channel_enable(PORT_DMA, CHANNEL_DMA); 
	
	/*        开启软件触发ADC转换                */
	adc_software_trigger_enable(PORT_ADC, ADC_ROUTINE_CHANNEL); 
}
 

/******************************************************************
 * 函 数 名 称：Get_Adc_Dma_Value
 * 函 数 说 明：对DMA保存的数据进行平均值计算后输出
 * 函 数 形 参：CHx 第几个扫描的数据
 * 函 数 返 回：对应扫描的ADC值
 * 作       者：LC
 * 备       注：无
******************************************************************/
unsigned int Get_Adc_Dma_Value(char CHx)
{
        unsigned char i = 0;
        unsigned int AdcValue = 0;
    
    /* 因为采集 SAMPLES 次，故循环 SAMPLES 次 */
        for(i=0; i< SAMPLES; i++)
        {
        /*    累加    */
                AdcValue+=gt_adc_val[i][CHx];
        }
    /* 求平均值 */
        AdcValue=AdcValue / SAMPLES;
    
        return AdcValue;
}

/******************************************************************
 * 函 数 名 称：Get_FLAME_Percentage_value
 * 函 数 说 明：读取火焰AO值，并且返回百分比
 * 函 数 形 参：无
 * 函 数 返 回：返回百分比
 * 作       者：LC
 * 备       注：无
******************************************************************/
unsigned int Get_FLAME_Percentage_value(void)
{
    int adc_max = 4095;
    int adc_new = 0;
    int Percentage_value = 0;
    
    adc_new = Get_Adc_Dma_Value(0);
    
    Percentage_value = (1-((float)adc_new/adc_max)) * 100;
    return Percentage_value;
}
/******************************************************************
 * 函 数 名 称：Get_FLAME_Do_value
 * 函 数 说 明：读取火焰DO值，返回0或者1
 * 函 数 形 参：无
 * 函 数 返 回：
 * 作       者：LC
 * 备       注：无
******************************************************************/
unsigned char Get_FLAME_Do_value(void)
{
    return gpio_input_bit_get(PORT_FLAME_DO,GPIO_FLAME_DO);
}
