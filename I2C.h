#ifndef I2C_H
#define I2C_H
#include <sys.h>
#include <led.h>
#include <GPIO_config.h>
//return 0:done
//return 1:can't send start conditon
//return 2:slave didn't ACK before finishing
//return 3:didn't receive enough data
//return 4:input argument illegal
//return 5:AF is set
#define I2C_DEBUG 0

//FLAG in SR1
#define AF 	  0x0400 	//ACK failure
#define ARLO  0x0200 	//Arbitrition lost
#define BERR  0x0100 	//Buss error: a wrong start/stop condition
#define TxE   0x0080 	//Data register empty(transmitter)
#define RxNE  0x0040 	//Data register not empty(receiver)
#define STOPF 0x0010 	//Stop detection(slave mode)
#define ADD10 0x0008 	//10-bit header sent(master mode)
#define BTF   0x0004 	//Byte transfer finish
#define ADDR  0x0002 	//Address sent(master mode)/matched(slave mode)
#define SB    0x0001 	//Start bit(master mode)
//Flag in SR2
#define DUALF 0x0080 	//Dual address flag(slave mode)
#define TRA   0x0004 	//Transmitter/receiver
#define BUSY  0x0002 	//Bus busy: SDA=0 or SCL=0
#define MSL   0x0001 	//Master/slave

//extern unsigned int SR;
void IIC1_Init(void);
void IIC2_Init(void);

unsigned char E2PROM_Read(unsigned short addr,unsigned char *data,unsigned char data_num);
//unsigned char IIC2_ReadBytes(unsigned char addr,unsigned char *data,unsigned char data_num);
unsigned char IIC1_WriteBytes(unsigned char addr,unsigned char *data,unsigned char data_num);

#endif
