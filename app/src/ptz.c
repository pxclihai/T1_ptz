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
	

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);	//ʹ�ܶ�ʱ��3ʱ��
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB  | RCC_APB2Periph_AFIO, ENABLE);  //ʹ��GPIO�����AFIO���ù���ģ��ʱ��
	
	GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE); //Timer3������ӳ��  TIM3_CH2->PB5    
 
   //���ø�����Ϊ�����������,���TIM3 CH2��PWM���岨��	GPIOB.5
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; //TIM_CH2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //�����������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��GPIO
 
   //��ʼ��TIM3
	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ 
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
	
	//��ʼ��TIM3 Channel2 PWMģʽ	 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ2
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; //�������:TIM����Ƚϼ��Ը�
	TIM_OC2Init(TIM3, &TIM_OCInitStructure);  //����Tָ���Ĳ�����ʼ������TIM3 OC2

	TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);  //ʹ��TIM3��CCR2�ϵ�Ԥװ�ؼĴ���
 
	TIM_Cmd(TIM3, ENABLE);  //ʹ��TIM3
	

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