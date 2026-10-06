#include "stm32f10x.h"                  // Device header
#include "Delay.h"
//#define SCL_Port	GPIOA				//宏定义
//#define SCL_Pin 	GPIO_Pin_11

//SCL写入
void MyI2C_W_SCL(uint8_t BitValue)
{
	GPIO_WriteBit(GPIOA,GPIO_Pin_11,(BitAction)BitValue);
	Delay_us(10);
}

//SDA写入
void MyI2C_W_SDA(uint8_t BitValue)
{
	GPIO_WriteBit(GPIOA,GPIO_Pin_12,(BitAction)BitValue);
	Delay_us(10);
}

//SDA读取
uint8_t MyI2C_R_SDA(void)
{
	uint8_t BitValue;
	BitValue=GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_12);
	Delay_us(10);
	return BitValue;
}

void MyI2C_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	//SCL&SDA
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11|GPIO_Pin_12 ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	GPIO_SetBits(GPIOA,GPIO_Pin_11|GPIO_Pin_12);	//将SDA和SCL置高电平
	
	
}

//开启SCL和SDA
void MyI2C_Start(void)
{
	//开启SCL&SDA
	MyI2C_W_SDA(1);
	MyI2C_W_SCL(1);
	//拉低SDA
	MyI2C_W_SDA(0);
	//拉低SCL
	MyI2C_W_SCL(0);
	
}
void MyI2C_Stop(void)
{
	//先拉低SDA
	MyI2C_W_SDA(0);
	//再释放SCL
	MyI2C_W_SCL(1);
	//释放SDA
	MyI2C_W_SDA(1);
}

void MyI2C_SendByte(uint8_t Byte)
{
	uint8_t i;
	for(i=0;i<8;i++)
	{
		MyI2C_W_SDA(!!(Byte & (0x80 >> i)));; //0x80右移i位
		MyI2C_W_SCL(1);
		MyI2C_W_SCL(0);
	}
//	//先写入字节最高位
//	MyI2C_W_SDA(Byte&0x80); //按位与,取出最高位
//	//释放SCL
//	MyI2C_W_SCL(1);
//	//拉低SCL
//	MyI2C_W_SCL(0);
//	
//	//写入字节次高位
//	MyI2C_W_SDA(Byte&0x40);
//	//释放SCL
//	MyI2C_W_SCL(1);
//	//拉低SCL
//	MyI2C_W_SCL(0);
//	
//	MyI2C_W_SDA(Byte&0x20);
//	MyI2C_W_SCL(1);
//	MyI2C_W_SCL(0);
//	
//	...以此类推，写入八位
}

uint8_t MyI2C_RecieveByte(void)
{
	uint8_t i,Byte=0x00;
	//从机写入数据
	MyI2C_W_SDA(1);
	//主机读取数据
	for(i=0;i<8;i++)
	{
		MyI2C_W_SCL(1);
		if(MyI2C_R_SDA()==1){Byte|=(0x80>>i);} //把数据放到对应位
		MyI2C_W_SCL(0);	
	}
	return Byte;
	
	
}


//发送应答
void MyI2C_SendAck(uint8_t AckBit)
{
	//主机发送应答
	MyI2C_W_SDA(AckBit); 
	//SCL高电平，从机读取应答
	MyI2C_W_SCL(1);
	MyI2C_W_SCL(0);
	
}

uint8_t MyI2C_RecieveAck(void)
{
	uint8_t AckBit;
	//从机发送应答
	MyI2C_W_SDA(1);
	//主机读取应答
	MyI2C_W_SCL(1);
	AckBit = MyI2C_R_SDA(); //把数据放到对应位
	MyI2C_W_SCL(0);	
	
	return AckBit;
	
	
}


