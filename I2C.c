#include <I2C.h>

#if I2C_DEBUG
#include <usart.h>
#endif

/*void I2C1_ER_IRQHandler(void)
{
	SR = I2C1->SR2;
	SR <<= 16;
	SR |= I2C1->SR1;
	LED0 = 1;
	I2C1->CR1 = 0;
}*/
void I2C1_EV_IRQHandler(void)
{
	if((I2C2->SR1 & ADDR) == ADDR) 		//ADDR = 1
		I2C2->SR2;
	if((I2C2->SR1 & STOPF) == STOPF)	//STOPF = 1
		I2C2->CR1 = 0x0401;				//ACK & PE
	/*if((I2C2->SR1 & SB) == SB)*/
	if((I2C2->SR1 & BTF) == BTF){
		I2C1->CR1 |= 0x0200;	//STOP
		I2C1->CR2 &= 0xF7FF;	//DMAEN=0
	}
}
void IIC1_Init(void)
{
	GPIOB_Enable();
	GPIO_config(GPIOB,6,AF_OD);//I2C1_SCL
	GPIO_config(GPIOB,7,AF_OD);//I2C1_SDA
	
	RCC->APB1ENR |= 1<<21;
	
	I2C1->OAR1 = 0x4000;
	I2C1->CR2 = 0x0024;	//36MHz，不开启中断
	I2C1->CCR = 180;	//36MHz下，36M/360 = 100kHz，占空比0.5
	I2C1->TRISE = 72;	//标准模式的总线规范
	
	//MY_NVIC_Init(1,2,I2C1_ER_IRQn,2);
}
//由于没有上下拉，这里配置为推挽
void IIC2_Init(void)
{
	GPIOB_Enable();
	GPIO_config(GPIOB,10,AF_PP);//I2C2_SCL
	GPIO_config(GPIOB,11,AF_PP);//I2C2_SDA
	
	RCC->APB1ENR |= 1<<22;

	I2C2->OAR1 = 0x40B0;	//7bits slave mode, slave address = 0xB0
	I2C2->CR2 = 0x0224;	//36MHz，不开启出错中断，开启事件中断使能
	I2C2->CCR = 180;	//36MHz下，36M/360 = 100kHz，占空比0.5
	I2C2->TRISE = 72;	//标准模式的总线规范
	//听说官方推荐设置最高优先级中断，要不就最高优先级DMA
	//MY_NVIC_Init(0,0,I2C2_EV_IRQn ,2);
}
unsigned char IIC1_DMA_Read(unsigned short addr,unsigned char *data,unsigned char data_num)
{

}
//Read data from 24C02
unsigned char E2PROM_Read(unsigned short addr,unsigned char *data,unsigned char data_num)
{
	unsigned short times=0;
	unsigned char offset = 0;
	
	if(data_num == 0 || addr & 0xF800)
		return 4;
	
	I2C1->CR1 = 0x0001;	//Enable I2C1
	
	//choose chip & write ROM address
	I2C1->CR1 |= 0x0500;	//start & ACK
	//check for SB == 1
	while((I2C1->SR1 & 0x0001) != 0x0001 && times != 720)
		++times;
	if(times != 720)
		times = 0;
	else
		return 1;//can't send start condition within reasonable time
	//send equipment address & check for ADDR == 1
	I2C1->DR = (unsigned char)(0xA0 + (addr>>7)) & 0xFE;	//writing address & slave read
	while((I2C1->SR1 & 0x0002) != 0x0002 && times != 1500)	//720 also work here
		++times;
	if(times != 1500)
		times = 0;
	else
		return 2;//slave didn't ACK
	//clear ADDR & send ROM address
	if((I2C1->SR2 & 0x0005) == 0x0005)	//read SR1 & SR2 to clear ADDR
		I2C1->DR = addr;
	while((I2C1->SR1 & 0x0004) != 0x0004 && times != 7200)
		++times;//wait for addr transit finish
	if(times != 7200)
		times=0;
	else
		return 3;//can't send ROM_addr

	//read data
	//send equipment address
	I2C1->CR1 |= 0x0500;	//re-start & ACK
	//check for SB == 1
	while((I2C1->SR1 & 0x0001) != 0x0001 && times != 720)
		++times;
	if(times != 720)
		times = 0;
	else
		return 4;//can't send start condition within reasonable time
	//send equipment address & check for ADDR == 1
	I2C1->DR = (unsigned char)(0xA0 + (addr>>7)) | 0x01;	//writing address & slave write
	while((I2C1->SR1 & 0x0002) != 0x0002 && times != 1500)	//720 also work here
		++times;
	if(times != 1500)
		times = 0;
	else
		return 5;//slave didn't ACK
	if((I2C1->SR2 & 0x0005) == 0x0001)	//Master Receiver
	{
		for(;offset < data_num;++offset)
		{
			if(offset == (data_num-1))
				I2C1->CR1 = 0x0201;	//NACK & STOP
			while((I2C1->SR1 & 0x0040) != 0x0040 && times != 7200)
				++times;
			if(times != 7200)
				*(data + offset) = I2C1->DR;
			else
			{
				/*data[0] = offset;//data_num>4
				data[1] = (I2C1->SR1)>>8;
				data[2] = (unsigned char)I2C1->SR1;
				data[3] = I2C1->SR2;
				data[4] = (unsigned char)I2C1->SR2;*/
				I2C1->CR1 = 0x0201;
				return 6;//
			}
			times = 0;
		}
	}
	//I2C1->CR1 &= 0xFFFE;	//disable I2C1
	return 0;	//done well
}
//主接收模式，读出 data_num 个数据
//需要 7bits 从设备地址
/*unsigned char IIC2_ReadBytes(unsigned char addr,unsigned char *data,unsigned char data_num)
{
	unsigned short times=0;
	unsigned char offset = 0;
	
	if(data_num == 0)
		return 4;
	
	I2C2->CR1 = 0x0001;	//Enable I2C2
	addr |= 0x01;	//slave write
	I2C2->CR1 |= 0x0100;	//start
	//check for SB == 1
	while((I2C2->SR1 & 0x0001) != 0x0001 && times != 1500)//720 also work here! unbelievale
		++times;
	if(times != 1500)
		times = 0;
	else
		return 1;//can't send start condition within reasonable time
	//send address & check for ADDR == 1
	I2C2->DR = addr;	//writing address
	while((I2C2->SR1 & 0x0002) != 0x0002 && times != 1500)
		++times;
	if(times != 1500)
		times = 0;
	else
		return 2;//slave didn't ACK
	
	if(I2C2->SR2 & 0x0001)	//read SR1 & SR2 to clear ADDR
	{
		I2C2->CR1 |= 0x0400;
		for(;offset < data_num;++offset)
		{
			if(offset == (data_num-1))
				I2C2->CR1 = 0x0201;	//NACK & STOP
			while((I2C2->SR1 & 0x0040) != 0x0040 && times != 7200)
				++times;
			if(times != 7200)
				*(data + offset) = I2C2->DR;
			else
			{
				#if I2C_DEBUG
				USART1->DR = offset;
				#endif
				I2C2->CR1 = 0x0201;
				return 3;//
			}
			times = 0;
		}
	}
	//I2C2->CR1 &= 0xFFFE;
	return 0;	//done well
}*/
//主发送模式
//注意：EEPROM写数据和下一次操作之间，要间隔一段时间
//需要7bits从设备地址
unsigned char IIC1_WriteBytes(unsigned char addr,unsigned char *data,unsigned char data_num)
{
	unsigned short times=0;
	unsigned char offset = 0;

	if(data_num == 0)
		return 4;

	addr &= 0xFE;		//slave: receiver
	I2C1->CR1 = 0x0001;	//Enable I2C1
	I2C1->CR1 |= 0x0500;//start & ACK
	
	//check for SB==1
	while((I2C1->SR1 & 0x0001) != 0x0001 && times != 720)
		++times;
	if(times != 720)
		times = 0;
	else
		return 1;//time out
	//send address & check for ADDR == 1
	I2C1->DR = addr;
	while((I2C1->SR1 & 0x0002) != 0x0002 && times != 8000)
		++times;
	if(times != 8000)
		times = 0;
	else
		return 2;//slave didn't ACK
	I2C1->SR1;
	if(I2C1->SR2 & 0x0004)	//read SR1 & SR2 to clear ADDR
	{
		while((I2C1->SR1 & 0x0080) == 0 && times != 8000) //check for TxE = 1
			++times;
		for(;(offset < data_num) && (times != 8000);++offset)
		{
			times = 0;
			I2C1->DR = *(data + offset);
			/*if(offset == (data_num-1))
				I2C1->CR1 = 0x0201;*/	//STOP
			while((I2C1->SR1 & 0x0084) == 0 && (times != 8000)) //when TxE=1 & BTF!=1
			    ++times;
		}
		if(times == 8000){
			I2C1->CR1 = 0x0201;
			//I2C1->CR1 = 0;
			return 2;
		}
		times = 0;
		while((I2C1->SR1 & 0x0004) == 0 && times !=8000)//BTF=1
			++times;
		I2C1->CR1 = 0x0201;//stop
		if(times == 8000)
			return 2;
		return 0;
	}
	//I2C1->CR1 = 0;
	//return 6;
}
