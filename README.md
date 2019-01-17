# demo-of-I2C-interface-of-STM32F103
I'm trying to use I2C hardware interface of STM32F103RCT6, which does not satisfy anybody. 
I know using polling is not efficient. It's just for learning and figuring out what happen in a communication.
These functions are verified with 24C02. I didn't use them to read less bytes so it probably has other proplems.
BTW, simulate I2C communication with Keil uVision5 can be helpful but not enough. 
And there are a few problems with the simulator I think. For example, if I reset PE when return, function E2PROM_Read() can't generate another start condition when I call it again.
note: I2C.c & I2C.h encode to GB2312. exam_main.c encodes to UTF-8.
