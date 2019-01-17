#include <sys.h>
//#include <wdg.h>
//#include <Uart_Config.h>
#include <delay.h>
#include <usart.h>
#include <led.h>
//#include <timer.h>
#include <key.h>
//#include <exti.h>
#include <GPIO_config.h>
//#include <MY_ADC.h>
//#include <DAC.h>
#include <dma.h>
//#include <spi.h>
#include <I2C.h>
//#include <flash.h>

unsigned int SR;
int main(void)
{
	unsigned char t=0,data[24];
	//unsigned char Rx_Buf[24];
	//unsigned short addr = 0xFF;
	struct dma_config_info conf_info;

	Stm32_Clock_Init(9); //系统时钟使能
	delay_init(72);	     //延时函数配置
	
	LED_Init();		  	 
	uart_init(72,9600);  //串口使能
	KEY_Init();
	//TIM1_PWM_Init(7999,8999);//1Hz
	//PWM_val = 1000;
	//ADC1_Init();
	//Tri_generater_Init(0x0C);
	DMA1_Init();
	//EXTI_Init();
	//IWDG_Init(7,0x00000FFF);
	//SPI1_Init();
	//SPI_Flash_Init();
	IIC1_Init();

	LED1 = LED0 = 0;
//	delay_ms(1000);
//	LED1 = LED0 = 1;

	conf_info.channel = DMA1_Channel4;
	conf_info.cpar = &(USART1->DR);
	conf_info.ccr = 0x00000090;

	while(1)
	{
		t = KEY_Scan(0);
		if(t == 1)
		{
			//read addr = 0~9
			t = E2PROM_Read(0,data,20);
			//USART1->DR = t;
			if(t == 0)
			{
				//conf_info.cmar = Rx_Buf;
				conf_info.cndtr = 20;
				conf_info.cmar = data;
				DMA1_Config(conf_info);
				USART1->CR3 |= 1<<7;
				while((DMA1->ISR & 0x00002000) != 0x00002000);
				USART1->CR3 &= ~(1<<7);
			}
			else
			{
				USART1->DR = t;
				SR = I2C1->SR2;
				SR <<= 16;
				SR |= I2C1->SR1;
				I2C1->CR1 = 0; 
				LED1 = !LED1;
				
				conf_info.cndtr = 4;
				conf_info.cmar = &SR;
				DMA1_Config(conf_info);
				USART1->CR3 |= 1<<7;
				while((DMA1->ISR & 0x00002000) != 0x00002000);
				USART1->CR3 &= ~(1<<7);
			}
			data[0]=0;
		}
		else if(t == 2)
		{
			//write addr = 0
			data[0] = 0;
			data[1] = 'D';
			t = IIC1_WriteBytes(0xA0,data,2);
			delay_ms(8);
			
			USART1->DR = t;
			SR = I2C1->SR2;
			SR <<= 16;
			SR |= I2C1->SR1;
			LED1 = !LED1;
			
			conf_info.cndtr = 4;
			conf_info.cmar = &SR;
			DMA1_Config(conf_info);
			USART1->CR3 |= 1<<7;
			while((DMA1->ISR & 0x00002000) != 0x00002000);
			USART1->CR3 &= ~(1<<7);			
//			USART1->DR = t;
//			//write addr = 1
			data[0] = 1;
			data[1] = 'T';
			t = IIC1_WriteBytes(0xA0,data,2);
			delay_ms(8);
			//USART1->DR = t;
			//write addr = 2
			data[0] = 2;
			data[1] = 'C';
			t = IIC1_WriteBytes(0xA0,data,2);
			delay_ms(8);
			//USART1->DR = t;
		}
	} 
	//return 0;
}

