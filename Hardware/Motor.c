#include "stm32f10x.h"
#include "Motor.h"

/* 经测试所有轮子均可以正转 */
#define MOTOR_A_REVERSE  0
#define MOTOR_B_REVERSE  1
#define MOTOR_C_REVERSE  1
#define MOTOR_D_REVERSE  0

typedef struct
{
    GPIO_TypeDef *in1_port;  //In1端口
    uint16_t in1_pin;        //In1引脚
    GPIO_TypeDef *in2_port;
    uint16_t in2_pin;
    TIM_TypeDef *timer;      //定时器
    uint8_t channel;		 //通道 CHX
    uint8_t reverse;		 //是否反转
} Motor_Config;

/* 轮子对应编号：1=A左后，2=B左前，3=C右前，4=D右后 */
static const Motor_Config motors[4] =
{
    {GPIOB, GPIO_Pin_5,  GPIOB, GPIO_Pin_6, TIM3, 1, MOTOR_A_REVERSE},
    {GPIOB, GPIO_Pin_7,  GPIOB, GPIO_Pin_8, TIM3, 2, MOTOR_B_REVERSE},
    {GPIOA, GPIO_Pin_2,  GPIOA, GPIO_Pin_3, TIM2, 2, MOTOR_C_REVERSE},
    {GPIOA, GPIO_Pin_15, GPIOB, GPIO_Pin_9, TIM1, 1, MOTOR_D_REVERSE}
};

/*设定电机占空比*/
static void Motor_WritePWM(const Motor_Config *motor, uint16_t duty)
{
    if (motor->channel == 1)
    {
        TIM_SetCompare1(motor->timer, duty);
    }
    else
    {
        TIM_SetCompare2(motor->timer, duty);
    }
}

/*设置速度*/
void Motor_Set(uint8_t motor, int16_t speed)
{
    const Motor_Config *config;
    uint16_t duty;
    uint8_t forward;

    if (motor < 1 || motor > 4)
    {
        return;
    }

    if (speed > 100)  speed = 100;
    if (speed < -100) speed = -100;

    config = &motors[motor - 1];

    /* 改变方向前先关 PWM */
    Motor_WritePWM(config, 0);
    GPIO_ResetBits(config->in1_port, config->in1_pin);
    GPIO_ResetBits(config->in2_port, config->in2_pin);

    if (speed == 0)
    {
        return;
    }

    duty = (speed > 0) ? (uint16_t)speed : (uint16_t)(-speed);
    forward = (speed > 0);

    if (config->reverse)
    {
        forward = !forward;
    }

    if (forward)
    {
		//置高电平 使电机正转
        GPIO_SetBits(config->in1_port, config->in1_pin);
    }
    else
    {
        GPIO_SetBits(config->in2_port, config->in2_pin);
    }

    Motor_WritePWM(config, duty);
}

/*停止全部电机*/
void Motor_StopAll(void)
{
    Motor_Set(1, 0);
    Motor_Set(2, 0);
    Motor_Set(3, 0);
    Motor_Set(4, 0);
}


/*初始化*/
void Motor_Init(void)
{
    GPIO_InitTypeDef gpio;
    TIM_TimeBaseInitTypeDef timeBase;
    TIM_OCInitTypeDef output;

    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_AFIO |  //开启AFIO时钟，开启重定义功能
		RCC_APB2Periph_GPIOA |  RCC_APB2Periph_GPIOB |
        RCC_APB2Periph_TIM1,
        ENABLE);

    RCC_APB1PeriphClockCmd(
        RCC_APB1Periph_TIM2 |
        RCC_APB1Periph_TIM3,
        ENABLE);

    /*
	*PA15
	*I/O口电平：FT 输入容忍5V
	*引脚默认功能:JTDI：JTAG 的测试数据输入脚
	*重定义：
	*/
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

	//初始置低电平
    GPIO_ResetBits(GPIOA, GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_15);
    GPIO_ResetBits(GPIOB,
        GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 |
        GPIO_Pin_8 | GPIO_Pin_9);

    /* IN1/2方向引脚 */
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;  //推挽输出
    gpio.GPIO_Speed = GPIO_Speed_2MHz;

    gpio.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_15;
    GPIO_Init(GPIOA, &gpio);

    gpio.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 |
                    GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_Init(GPIOB, &gpio);

    /* 接PWM 引脚：PA1、PA6、PA7、PA8 */
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_6 |
                    GPIO_Pin_7 | GPIO_Pin_8;
    GPIO_Init(GPIOA, &gpio);

    /* 72 MHz / 72 / 100 = 10 kHz；占空比数值为 0~100 */
    TIM_TimeBaseStructInit(&timeBase);
    timeBase.TIM_Prescaler = 71;
    timeBase.TIM_Period = 99;

    TIM_TimeBaseInit(TIM1, &timeBase);
    TIM_TimeBaseInit(TIM2, &timeBase);
    TIM_TimeBaseInit(TIM3, &timeBase);

    TIM_OCStructInit(&output);
    output.TIM_OCMode = TIM_OCMode_PWM1;                //选择PWM模式
    output.TIM_OutputState = TIM_OutputState_Enable;
    output.TIM_OCPolarity = TIM_OCPolarity_High;		//极性
    output.TIM_Pulse = 0;

	//配置不同定时器的PWM
    TIM_OC1Init(TIM3, &output); /* A：PA6 */
    TIM_OC2Init(TIM3, &output); /* B：PA7 */
    TIM_OC2Init(TIM2, &output); /* C：PA1 */
    TIM_OC1Init(TIM1, &output); /* D：PA8 */

	//使能定时器
    TIM_Cmd(TIM1, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
    TIM_CtrlPWMOutputs(TIM1, ENABLE);

    Motor_StopAll();
}

void Motor_SetSides(int16_t left, int16_t right)
{
    Motor_Set(1, left);
    Motor_Set(2, left);
    Motor_Set(3, right);
    Motor_Set(4, right);
}



