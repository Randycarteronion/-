/********************************************************************************
  * �� �� ��: bsp_flame.c
  * �� �� ��: ����
  * �޸�����: LC
  * �޸�����: 2023��04��06��
  * ���ܽ���:          
  ******************************************************************************
  * ע������:
*********************************************************************************/


#include "bsp_flame.h"
#include "board.h"


 //DMA������
uint16_t gt_adc_val[ SAMPLES ][ CHANNEL_NUM ]; 


/******************************************************************
 * �� �� �� �ƣ�ADC_DMA_Init
 * �� �� ˵ ������ʼ��ADC+DMA����
 * �� �� �� �Σ���
 * �� �� �� �أ���
 * ��       �ߣ�LC
 * ��       ע����
******************************************************************/
void ADC_DMA_Init(void)
{
	/* DMA��ʼ�����ܽṹ�嶨�� */
	dma_single_data_parameter_struct dma_single_data_parameter;

	/* ʹ������ʱ�� */
	rcu_periph_clock_enable(RCU_FLAME_GPIO_AO);                    
	rcu_periph_clock_enable(RCU_FLAME_GPIO_DO); 
	/* ʹ��ADCʱ�� */
	rcu_periph_clock_enable(RCU_FLAME_ADC);                

	/* ʹ��DMAʱ�� */
	rcu_periph_clock_enable(RCU_FLAME_DMA);
 
        /*        ����ADCʱ��        */ 
    adc_clock_config(ADC_ADCCK_PCLK2_DIV4);        
   /*        ����DOΪ����ģʽ        */
    gpio_mode_set(PORT_FLAME_DO, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_FLAME_DO); 
        /*        ����AOΪ����ģ������ģʽ        */
    gpio_mode_set(PORT_FLAME_AO, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_FLAME_AO); // PC1 : ADC012_IN11 
 
        /*        ����ADCΪ����ģʽ        */
    adc_sync_mode_config(ADC_SYNC_MODE_INDEPENDENT);
        
    /*        ʹ������ת��ģʽ        */
    adc_special_function_config(PORT_ADC, ADC_CONTINUOUS_MODE, ENABLE);
         
    /*        ʹ��ɨ��ģʽ        */
    adc_special_function_config(PORT_ADC, ADC_SCAN_MODE, ENABLE);
   
        /*        �����Ҷ���        */        
    adc_data_alignment_config(PORT_ADC, ADC_DATAALIGN_RIGHT);
    
    /*        ADC0����Ϊ������  һ��ʹ�� CHANNEL_NUM ��ͨ��                */  
    adc_channel_length_config(PORT_ADC, ADC_ROUTINE_CHANNEL, CHANNEL_NUM);
                
    /*        ADC����ͨ�����ã�ADC0��ͨ��11��ɨ��˳��Ϊ0������ʱ�䣺15������                */  
        /*        DMA����֮�� gt_adc_val[x][0] = PC1������   */
    adc_routine_channel_config(PORT_ADC, 0, CHANNEL_ADC, ADC_SAMPLETIME_15);//PC1
 
        /*        ADC0����Ϊ12λ�ֱ���                */  
        adc_resolution_config(PORT_ADC, ADC_RESOLUTION_12B); 
        
        /*        ADC�ⲿ��������, ��ֻ��ʹ���������                */  
    adc_external_trigger_config(PORT_ADC, ADC_ROUTINE_CHANNEL, EXTERNAL_TRIGGER_DISABLE);
    
    /*        ʹ�ܹ�����ͨ��ÿת�����һ���ͷ���һ��DMA����                */  
    adc_dma_request_after_last_enable(PORT_ADC);  
    
	/*        ʹ��DMA����                */  
	adc_dma_mode_enable(PORT_ADC);

	/*        ʹ��ADC                */          
	adc_enable(PORT_ADC);

	/*        �ȴ�ADC�ȶ�                */  
	delay_ms(1);
    
        /*        ����ADC��У׼                */
    adc_calibration_enable(PORT_ADC); 
 
    /*        ��� DMAͨ��0 ֮ǰ����         */
    dma_deinit(PORT_DMA, CHANNEL_DMA);
   
	/*        DMA��ʼ������         */
	dma_single_data_parameter.periph_addr = (uint32_t)(&ADC_RDATA(PORT_ADC));         //����DMA����������ַΪADC0����ַ
	dma_single_data_parameter.periph_inc = DMA_PERIPH_INCREASE_DISABLE;                         //�ر������ַ����
	dma_single_data_parameter.memory0_addr = (uint32_t)(gt_adc_val);                         //����DMA������ڴ��ַΪ gt_adc_val����
	dma_single_data_parameter.memory_inc = DMA_MEMORY_INCREASE_ENABLE;                         //�����ڴ��ַ��������Ϊ��ֹһ��ͨ����
	dma_single_data_parameter.periph_memory_width = DMA_PERIPH_WIDTH_16BIT;             //���������λ Ϊ 16λ
	dma_single_data_parameter.direction = DMA_PERIPH_TO_MEMORY;                                         //DMA���䷽��Ϊ �������ڴ�
	dma_single_data_parameter.number = SAMPLES * CHANNEL_NUM;                                     //��������ݳ���Ϊ��ÿ��ͨ���ɼ�30�� * 1��ͨ��
	dma_single_data_parameter.priority = DMA_PRIORITY_HIGH;                                                 //���ø����ȼ�
	dma_single_data_mode_init(PORT_DMA, CHANNEL_DMA, &dma_single_data_parameter);//�����ñ�����DMA1��ͨ��0
	
	/*        DMAͨ������ѡ��                */
	/*        �����ֲ��195ҳ����PERIEN[2:0]ֵȷ������������������100 ��ΪDMA_SUBPERI4          ����010 ��ΪDMA_SUBPERI2         */
	/*        ������ADC0���ܣ�PERIEN[2:0]ֵΪ000����ΪDMA_SUBPERI0                */
	dma_channel_subperipheral_select(PORT_DMA, CHANNEL_DMA, DMA_SUBPERI0);

	/*        ʹ��DMA1ͨ��0ѭ��ģʽ                */
	dma_circulation_enable(PORT_DMA, CHANNEL_DMA);

	/*        ����DMA1��ͨ��0����                */
	dma_channel_enable(PORT_DMA, CHANNEL_DMA); 
	
	/*        �����������ADCת��                */
	adc_software_trigger_enable(PORT_ADC, ADC_ROUTINE_CHANNEL); 
}
 

/******************************************************************
 * �� �� �� �ƣ�Get_Adc_Dma_Value
 * �� �� ˵ ������DMA��������ݽ���ƽ��ֵ��������
 * �� �� �� �Σ�CHx �ڼ���ɨ�������
 * �� �� �� �أ���Ӧɨ���ADCֵ
 * ��       �ߣ�LC
 * ��       ע����
******************************************************************/
unsigned int Get_Adc_Dma_Value(char CHx)
{
        unsigned char i = 0;
        unsigned int AdcValue = 0;
    
    /* ��Ϊ�ɼ� SAMPLES �Σ���ѭ�� SAMPLES �� */
        for(i=0; i< SAMPLES; i++)
        {
        /*    �ۼ�    */
                AdcValue+=gt_adc_val[i][CHx];
        }
    /* ��ƽ��ֵ */
        AdcValue=AdcValue / SAMPLES;
    
        return AdcValue;
}

/******************************************************************
 * �� �� �� �ƣ�Get_FLAME_Percentage_value
 * �� �� ˵ ������ȡ����AOֵ�����ҷ��ذٷֱ�
 * �� �� �� �Σ���
 * �� �� �� �أ����ذٷֱ�
 * ��       �ߣ�LC
 * ��       ע����
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
 * �� �� �� �ƣ�Get_FLAME_Do_value
 * �� �� ˵ ������ȡ����DOֵ������0����1
 * �� �� �� �Σ���
 * �� �� �� �أ�
 * ��       �ߣ�LC
 * ��       ע����
******************************************************************/
unsigned char Get_FLAME_Do_value(void)
{
    return gpio_input_bit_get(PORT_FLAME_DO,GPIO_FLAME_DO);
}
