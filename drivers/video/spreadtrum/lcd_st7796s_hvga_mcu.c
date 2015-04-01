/*
 * Copyright (C) 2012 Spreadtrum Communications Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/regulator/consumer.h>
#include <mach/regulator.h>
#include "lcdpanel.h"

static int32_t st7796s_init(struct panel_spec *self)
{
	send_data_t send_cmd = self->info.mcu->ops->send_cmd;
	send_data_t send_data = self->info.mcu->ops->send_data;

    self->ops->panel_reset(self);
	#include "st7796s_init_data.h"
#if 0	
	if (1) { //  for test the lcd
		int i;
		send_cmd(0x2C); //Write data
		for (i = 0; i < 480*320/3; i++)
			send_data(0xff);
		for (i = 0; i < 480*320/3; i++)
			send_data(0xff00);
		for (i = 0; i < 480*320/3; i++)
			send_data(0xff0000);
	}
	send_cmd(0x29); //Display On 
	LCD_DelayMS(120); //120ms
	send_cmd(0x2C); //Write data
	LCD_DelayMS(12000); //120ms
#endif	
    {
        int i;
        send_cmd(0x2C); //Write data
        for (i = 0; i < 480*320; i++)
			    send_data(0x0000);
    }
	return 0;
}

static int32_t st7796s_set_window(struct panel_spec *self,
		uint16_t left, uint16_t top, uint16_t right, uint16_t bottom)
{
	send_data_t send_cmd = self->info.mcu->ops->send_cmd;
	send_data_t send_data = self->info.mcu->ops->send_data;

	pr_debug("st7796s_set_window:%d, %d, %d, %d\n", left, top, right, bottom);

	send_cmd(0x2A);
	send_data((left >> 8));
	send_data((left & 0xFF));
	send_data((right >> 8));
	send_data((right & 0xFF));

	send_cmd(0x2B);
	send_data((top >> 8));
	send_data((top & 0xFF));
	send_data((bottom >> 8));
	send_data((bottom & 0xFF));

	send_cmd(0x2C);

	return 0;
}

static int32_t st7796s_invalidate(struct panel_spec *self)
{

	return self->ops->panel_set_window(self, 0, 0,
			self->width-1, self->height-1);

}

static int32_t st7796s_invalidate_rect(struct panel_spec *self,
				uint16_t left, uint16_t top,
				uint16_t right, uint16_t bottom)
{

	pr_debug("st7796s_invalidate_rect\n");

	return self->ops->panel_set_window(self, left, top,
			right, bottom);
}

static int32_t st7796s_set_direction(struct panel_spec *self, uint16_t direction)
{

	pr_debug("st7796s_set_direction\n");
	return 0;
}

static int32_t st7796s_enter_sleep(struct panel_spec *self, uint8_t is_sleep)
{
	send_data_t send_cmd = self->info.mcu->ops->send_cmd;
	send_data_t send_data = self->info.mcu->ops->send_data;

	if(is_sleep) {
		/*send_cmd(0x10);*/
		/* LCD_DelayMS(120);*/

		send_cmd(0x28);
		mdelay(150);

		send_cmd(0x10);
		mdelay(120);

	}
	else {
		st7796s_init(self);
		#if 0
		/*send_cmd(0x11);
		LCD_DelayMS(120);*/
		send_cmd(0x11); // (SLPOUT)
		mdelay(120); // 100ms
		send_cmd(0x29); // (DISPON)
		mdelay(100); // 100ms
		#endif
	}

	return 0;
}

static uint32_t st7796s_read_id(struct panel_spec *self)
{
	uint32_t read_value = 0;
	send_data_t send_cmd = self->info.mcu->ops->send_cmd;
	read_data_t read_data = self->info.mcu->ops->read_data;
	
	//send_cmd(0x04);
	send_cmd(0xD3);

	read_data();
	read_data();
	//read_value += read_data()<< 16;
	read_value += read_data()<< 8;
	read_value += read_data();

	printk("[%s] read_value = 0x%x\n",__FUNCTION__,read_value);	
	return read_value;
}

extern int32_t lcm_reset();
static int32_t st7796s_reset(struct panel_spec *self)
{
    printk("%s() \n",__func__);
    lcm_reset();
}

static struct panel_operations lcd_st7796s_operations = {
	.panel_init            = st7796s_init,
	.panel_set_window      = st7796s_set_window,
	.panel_invalidate      = st7796s_invalidate,
	.panel_invalidate_rect = st7796s_invalidate_rect,
	.panel_set_direction   = st7796s_set_direction,
	.panel_enter_sleep     = st7796s_enter_sleep,
	.panel_readid          = st7796s_read_id,
	.panel_reset           = st7796s_reset,
};

static struct timing_mcu lcd_st7796s_timing[] = {
	[LCD_REGISTER_TIMING] = {
		.rcss = 35,
		.rlpw = 55,
		.rhpw = 100,
		.wcss = 50,
		.wlpw = 25,
		.whpw = 25,
	},
	[LCD_GRAM_TIMING] = {
		.rcss = 35,
		.rlpw = 55,
		.rhpw = 100,
		.wcss = 40,
		.wlpw = 25,
		.whpw = 25,
	}
};

static struct info_mcu lcd_st7796s_info = {
	.bus_mode  = LCD_BUS_8080,
	.bus_width = 18,
	.timing    = lcd_st7796s_timing,
	.ops       = NULL,
};

struct panel_spec lcd_st7796s_spec = {
	.width = 320,
	.height = 480,
	.mode = LCD_MODE_MCU,
	.direction = LCD_DIRECT_NORMAL,
	.info = {
		.mcu = &lcd_st7796s_info
	},
	.ops = &lcd_st7796s_operations,
};

struct panel_cfg lcd_st7796s = {
	.lcd_cs = -1,
	.lcd_id = 0x7796,
	.lcd_name = "lcd_st7796s",
	.panel = &lcd_st7796s_spec,
};


static int __init lcd_st7796s_init(void)
{
	return sprd_register_panel(&lcd_st7796s);
}

subsys_initcall(lcd_st7796s_init);
