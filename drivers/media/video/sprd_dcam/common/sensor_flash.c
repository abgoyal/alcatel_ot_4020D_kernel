#include "sensor_flash.h"

#define LED_FLASH_DEV_NAME "flash-led"

static struct led_classdev *led_flash = NULL;

static int sensor_find_ledflash(void)
{
	struct led_classdev *led_temp;
	list_for_each_entry(led_temp, &leds_list, node){
		if(!strncmp(led_temp->name,LED_FLASH_DEV_NAME,sizeof(LED_FLASH_DEV_NAME)))
		{
			led_flash = led_temp;
			return 0;
		}
	}
	return -1;
}

int Sensor_SetFlash(int flash_mode)
{
	if(led_flash == NULL)
	{
		if(sensor_find_ledflash() !=0)
		{
			printk("Error of get flash devices!\n");
			return -1;
		}
	}

	led_brightness_set(led_flash,flash_mode);
	
	return 0;
}
