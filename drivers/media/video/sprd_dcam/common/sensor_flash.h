#ifndef __SENSOR_FLASH__
#define __SENSOR_FLASH__
#include <linux/leds.h>
#include <linux/list.h>
#include <linux/device.h>
extern struct list_head leds_list;
int Sensor_SetFlash(int flash_mode);


#endif
