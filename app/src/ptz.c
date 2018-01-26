#include "ptz.h"
#include <easyflash.h>
#include <rthw.h>
#include <rtthread.h>
#include <stm32f10x_conf.h>
#include <finsh.h>
ptz_t ptz;
motor_t motor_axial;motor_t motor_radial;
void ptz_control(motor_t *motor ,mode_t mode,u16 pos);
void ptz_auto(motor_t *motor ,mode_t mode,u16 min,u16 max);
void TIM3_PWM_Init(u16 arr,u16 psc)
{  
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);	//使能定时器3时钟
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB  | RCC_APB2Periph_AFIO, ENABLE);  //使能GPIO外设和AFIO复用功能模块时钟
	
	GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE); //Timer3部分重映射  TIM3_CH2->PB5    
 
   //设置该引脚为复用输出功能,输出TIM3 CH2的PWM脉冲波形	GPIOB.5
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; //TIM_CH2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIO
 
   //初始化TIM3
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值 
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
	
	//初始化TIM3 Channel2 PWM模式	 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //选择定时器模式:TIM脉冲宽度调制模式2
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; //输出极性:TIM输出比较极性高
	TIM_OC2Init(TIM3, &TIM_OCInitStructure);  //根据T指定的参数初始化外设TIM3 OC2

	TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);  //使能TIM3在CCR2上的预装载寄存器
 
	TIM_Cmd(TIM3, ENABLE);  //使能TIM3
	

}
u8 stepper_axial_init()
{
		TIM3_PWM_Init(9999,72);
}
u8 stepper_axial_position(u16 pos)
{
	  TIM_SetCompare2(TIM3,pos);	
}

u8 setpper_loop(motor_t *motor)
{
	
	//rt_kprintf("The CPU usage%d\n",motor.run_status);
	 switch(motor->run_status)
	 {
		 case POSITION:
			 motor->run_status = JUDGE;
		   break;
		 case JUDGE:
			 if(motor->cur_pos < motor->set_pos)
			 {
				  motor->run_status = LEFT;
			 }
			 else if(motor->cur_pos > motor->set_pos)
			 {
				  motor->run_status = RIGHT;
			 }
			 else
			 {
				  motor->run_status = STOP;
			 }
			 break;
		 case LEFT:
			motor->cur_pos++;
			motor->set_position(motor->cur_pos);
			motor->run_status = WAIT;
			 
		   if( motor->run_step == motor->set_pos)
			 {
				 motor->run_status = STOP;
			 }
			 break;
		 case RIGHT:
			motor->cur_pos--;
			motor->set_position(motor->cur_pos);
			motor->run_status = WAIT;
			 
		   if( motor->cur_pos == motor->set_pos)
			 {
				 motor->run_status = STOP;
			 }
			 break;
		 case WAIT:
			 motor->time_count++;
		   if(motor->time_count > motor_axial.turn_speed)
			 {
				 motor->run_status = JUDGE;
				 motor->time_count = 0;
			 }
			 
			 
		   break;
		 case STOP:
			// if(motor.cur_angle != )
			 break;
		 case HOME:
			 motor->set_position(motor->home_vaule);
		   motor->cur_pos = motor->home_vaule;
		   motor->run_status = STOP;
		   
			 break;
		 case AUTO:
			 if(motor->turn_dir == 0)
			 {
				 motor->cur_pos ++;
				 if(motor->cur_pos > motor->auto_max)
				 {
						motor->turn_dir = 1;
				 }
				 
		   }
			 else
			 {
				  motor->cur_pos --;
				 if(motor->cur_pos < motor->auto_min)
				 {
						motor->turn_dir = 0;
				 }
			 }
			
		   
			 motor->set_position(motor->cur_pos);
			 motor->run_status = AUTO_WAIT;
			 break;
		 case AUTO_WAIT:
			 	motor->time_count++;
		   if(motor->time_count > motor_axial.turn_speed)
			 {
				 motor->run_status = AUTO;
				 motor->time_count = 0;
			 }
			 break;
		 
		 
	 }
}

void ptz_init()
{
	
	uint32_t ptz_para = NULL;
  char *c_ptz_para, c_new_ptz_para[11] = {0};
	
//  c_ptz_para = ef_get_env("motor_axial_speed");
//  RT_ASSERT(c_ptz_para);
//  motor_axial.turn_speed = atol(c_ptz_para);
	
  motor_axial.motor_init     = stepper_axial_init;
	motor_axial.set_position =   stepper_axial_position;
	motor_axial.home_vaule  = 2000;
	motor_axial.turn_speed  = 10;
	motor_axial.motor_init();
	
	ptz.motor_loop          = setpper_loop;
	ptz.motor_control       = ptz_control;
	ptz.motor_auto        = ptz_auto;
	motor_axial.run_status  = STOP;
	ptz.motor_control(&motor_axial,HOME,0);

//	ptz.motor_control(&motor_axial,POSITION,1500);
	//motor_axial
	
}
void ptz_control(motor_t *motor ,mode_t mode,u16 pos)
{
	motor->run_status = mode;
  motor->set_pos    = pos;	
}
void ptz_auto(motor_t *motor ,mode_t mode,u16 min,u16 max)
{
	motor->run_status  = mode;
  motor->auto_max    = max;
 	motor->auto_min    = min;
}
void ptz_loop()
{
	ptz.motor_loop(&motor_axial);
		
}