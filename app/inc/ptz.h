#ifndef __PTZ_H__
#define __PTZ_H__
#include <stm32f10x_conf.h>


extern void ptz_init();
extern void ptz_loop();
typedef enum {
	  LEFT,
	  RIGHT,
	  JUDGE,
	  POSITION,
  	WAIT,
  	STOP,
	  HOME,
    AUTO,
	  AUTO_WAIT,
} mode_t;
typedef struct motor{
	
	
    mode_t  run_status ;
  	u16 run_step;
    u16 time_count;
	  u8  turn_dir;
	  u16 cur_pos;
	  u16 set_pos;
	  u16 home_vaule;
	  u16 limit_max;
	  u16 limit_min;
	  u16 auto_min;
	  u16 auto_max;
	  u16 turn_speed;
	  u16 acc_speed;
	  u16 dec_speed;
	  u8 (*motor_init)();
    
	  u8 (*set_position) (u16 pos);
	  void (*continuous_trotation)(u16 minAngle,u16 maxAngle);
}motor_t;

typedef struct _ptz{
    
	void (*motor_control)(motor_t *,mode_t,u16);
  u8 (*motor_loop)(motor_t *);
	void (*motor_auto)(motor_t *,mode_t,u16,u16);
	 
}ptz_t;
extern  motor_t motor_axial;
extern	motor_t motor_radial;
extern ptz_t ptz;
#endif