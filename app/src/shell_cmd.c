#include <easyflash.h>
#include <rthw.h>
#include <rtthread.h>
#include <stm32f10x_conf.h>
#include <finsh.h>
#include "ptz.h"

void setmotor(uint8_t argc, char **argv) {
    uint8_t i;
	  u16  value;
    char c_value = NULL;
   // char *value = &c_value;
   
    if (argc == 1) {
        
    }
		else if (argc == 3) {
			  if( strcmp(argv[1],"pos") == 0)
				{
						
					sscanf(argv[2],"%d",&value);
					ptz.motor_control(&motor_axial,POSITION,value);
				}
				else if( strcmp(argv[1],"speed") == 0 )
				{
			      /* set and store the boot count number to Env */
						sscanf(argv[2],"%d",&value);
						motor_axial.turn_speed  = value;
				//		ef_set_env("motor_axial_speed", argv[2]);
				//		ef_save_env();
				}
				else 
				{
					rt_kprintf("para error\n");
				}
    }
		else if (argc == 4) 
	 { 
		if( strcmp(argv[1],"auto") == 0 )
				{
					u16  min,max;
					sscanf(argv[2],"%d",&min);
					sscanf(argv[3],"%d",&max);
					ptz.motor_auto(&motor_axial,AUTO,min,max);
				}
		}
		else {
          
    }
	
}
MSH_CMD_EXPORT(setmotor, set motor para);


