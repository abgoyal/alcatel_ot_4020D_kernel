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

static int32_t ili9488_init(struct panel_spec *self)
{
	send_data_t send_cmd = self->info.mcu->ops->send_cmd;
	send_data_t send_data = self->info.mcu->ops->send_data;
	self->ops->panel_reset(self);
#if  1
	send_cmd(0xED);
	send_data(0x01);
	send_data(0xFE);

	send_cmd(0xEE);
	send_data(0xDE);
	send_data(0x21);
	mdelay(100);
	
	send_cmd(0xF1);
	send_data(0x01);

	send_cmd(0xDF);
	send_data(0x10);

	send_cmd(0xB7);
	send_data(0x00);

	send_cmd(0xC0);
	send_data(0x44);//48
	send_data(0x44);
	send_data(0x10);
	send_data(0x10);

	send_cmd(0xC2);
	send_data(0x44);
	send_data(0x44);
	send_data(0x44);

	send_cmd(0xC4);
	send_data(0x72);//72

	send_cmd(0xC6);
	send_data(0x00);//48
	send_data(0xE2);
	send_data(0xE2);
	send_data(0xE2);

	send_cmd(0xBF);
	send_data(0xAA);

	send_cmd(0xE0);
	send_data(0x00);
	send_data(0x00);
	send_data(0x07);
	send_data(0x00);
	send_data(0x19);
	send_data(0x00);
	send_data(0x3A);
	send_data(0x00);
	send_data(0x4E);
	send_data(0x00);
	send_data(0x5C);
	send_data(0x00);
	send_data(0x6E);
	send_data(0x00);
	send_data(0x85);
	send_data(0x00);
	send_data(0x93);
	send_data(0x00);
	send_data(0x9F);
	send_data(0x00);
	send_data(0xAA);
	send_data(0x00);
	send_data(0xB7);
	send_data(0x00);
	send_data(0xC0);
	send_data(0x00);
	send_data(0xC5);
	send_data(0x00);
	send_data(0xCC);
	send_data(0x00);
	send_data(0xD0);
	send_data(0x00);
	send_data(0xD9);
	send_data(0x00);
	send_data(0xF3);
	send_data(0x00);

	send_cmd(0xE1);
	send_data(0x00);
	send_data(0x00);
	send_data(0x07);
	send_data(0x00);
	send_data(0x19);
	send_data(0x00);
	send_data(0x3A);
	send_data(0x00);
	send_data(0x4E);
	send_data(0x00);
	send_data(0x5C);
	send_data(0x00);
	send_data(0x6E);
	send_data(0x00);
	send_data(0x85);
	send_data(0x00);
	send_data(0x93);
	send_data(0x00);
	send_data(0x9F);
	send_data(0x00);
	send_data(0xAA);
	send_data(0x00);
	send_data(0xB7);
	send_data(0x00);
	send_data(0xC0);
	send_data(0x00);
	send_data(0xC5);
	send_data(0x00);
	send_data(0xCC);
	send_data(0x00);
	send_data(0xD0);
	send_data(0x00);
	send_data(0xD9);
	send_data(0x00);
	send_data(0xF3);
	send_data(0x00);

	send_cmd(0xE2);
	send_data(0x00);
	send_data(0x00);
	send_data(0x07);
	send_data(0x00);
	send_data(0x19);
	send_data(0x00);
	send_data(0x3A);
	send_data(0x00);
	send_data(0x4E);
	send_data(0x00);
	send_data(0x5C);
	send_data(0x00);
	send_data(0x6E);
	send_data(0x00);
	send_data(0x85);
	send_data(0x00);
	send_data(0x93);
	send_data(0x00);
	send_data(0x9F);
	send_data(0x00);
	send_data(0xAA);
	send_data(0x00);
	send_data(0xB7);
	send_data(0x00);
	send_data(0xC0);
	send_data(0x00);
	send_data(0xC5);
	send_data(0x00);
	send_data(0xCC);
	send_data(0x00);
	send_data(0xD0);
	send_data(0x00);
	send_data(0xD9);
	send_data(0x00);
	send_data(0xF3);
	send_data(0x00);

	send_cmd(0xE3);
	send_data(0x00);
	send_data(0x00);
	send_data(0x07);
	send_data(0x00);
	send_data(0x19);
	send_data(0x00);
	send_data(0x3A);
	send_data(0x00);
	send_data(0x4E);
	send_data(0x00);
	send_data(0x5C);
	send_data(0x00);
	send_data(0x6E);
	send_data(0x00);
	send_data(0x85);
	send_data(0x00);
	send_data(0x93);
	send_data(0x00);
	send_data(0x9F);
	send_data(0x00);
	send_data(0xAA);
	send_data(0x00);
	send_data(0xB7);
	send_data(0x00);
	send_data(0xC0);
	send_data(0x00);
	send_data(0xC5);
	send_data(0x00);
	send_data(0xCC);
	send_data(0x00);
	send_data(0xD0);
	send_data(0x00);
	send_data(0xD9);
	send_data(0x00);
	send_data(0xF3);
	send_data(0x00);

	send_cmd(0xE4);
	send_data(0x00);
	send_data(0x00);
	send_data(0x07);
	send_data(0x00);
	send_data(0x19);
	send_data(0x00);
	send_data(0x3A);
	send_data(0x00);
	send_data(0x4E);
	send_data(0x00);
	send_data(0x5C);
	send_data(0x00);
	send_data(0x6E);
	send_data(0x00);
	send_data(0x85);
	send_data(0x00);
	send_data(0x93);
	send_data(0x00);
	send_data(0x9F);
	send_data(0x00);
	send_data(0xAA);
	send_data(0x00);
	send_data(0xB7);
	send_data(0x00);
	send_data(0xC0);
	send_data(0x00);
	send_data(0xC5);
	send_data(0x00);
	send_data(0xCC);
	send_data(0x00);
	send_data(0xD0);
	send_data(0x00);
	send_data(0xD9);
	send_data(0x00);
	send_data(0xF3);
	send_data(0x00);

	send_cmd(0xE5);
	send_data(0x00);
	send_data(0x00);
	send_data(0x07);
	send_data(0x00);
	send_data(0x19);
	send_data(0x00);
	send_data(0x3A);
	send_data(0x00);
	send_data(0x4E);
	send_data(0x00);
	send_data(0x5C);
	send_data(0x00);
	send_data(0x6E);
	send_data(0x00);
	send_data(0x85);
	send_data(0x00);
	send_data(0x93);
	send_data(0x00);
	send_data(0x9F);
	send_data(0x00);
	send_data(0xAA);
	send_data(0x00);
	send_data(0xB7);
	send_data(0x00);
	send_data(0xC0);
	send_data(0x00);
	send_data(0xC5);
	send_data(0x00);
	send_data(0xCC);
	send_data(0x00);
	send_data(0xD0);
	send_data(0x00);
	send_data(0xD9);
	send_data(0x00);
	send_data(0xF3);
	send_data(0x00); 


	send_cmd(0xE6);
	send_data(0x21);
	send_data(0x00);
	send_data(0x55);
	send_data(0x00);
	send_data(0x99);
	send_data(0x00);
	send_data(0x77);
	send_data(0x00);
	send_data(0x77);
	send_data(0x00);
	send_data(0x76);
	send_data(0x00);
	send_data(0x78);
	send_data(0x00);
	send_data(0x98);
	send_data(0x00);
	send_data(0xBB);
	send_data(0x00);
	send_data(0x99);
	send_data(0x00);
	send_data(0x66);
	send_data(0x00);
	send_data(0x54);
	send_data(0x00);
	send_data(0x45);
	send_data(0x00);
	send_data(0x34);
	send_data(0x00);
	send_data(0x44);
	send_data(0x00);
	send_data(0x34);
	send_data(0x00);
	send_cmd(0xE7);
	send_data(0x21);
	send_data(0x00);
	send_data(0x55);
	send_data(0x00);
	send_data(0x99);
	send_data(0x00);
	send_data(0x77);
	send_data(0x00);
	send_data(0x77);
	send_data(0x00);
	send_data(0x76);
	send_data(0x00);
	send_data(0x78);
	send_data(0x00);
	send_data(0x98);
	send_data(0x00);
	send_data(0xBB);
	send_data(0x00);
	send_data(0x99);
	send_data(0x00);
	send_data(0x66);
	send_data(0x00);
	send_data(0x54);
	send_data(0x00);
	send_data(0x45);
	send_data(0x00);
	send_data(0x34);
	send_data(0x00);
	send_data(0x44);
	send_data(0x00);
	send_data(0x34);
	send_data(0x00);
	send_cmd(0xE8);
	send_data(0x21);
	send_data(0x00);
	send_data(0x55);
	send_data(0x00);
	send_data(0x99);
	send_data(0x00);
	send_data(0x77);
	send_data(0x00);
	send_data(0x77);
	send_data(0x00);
	send_data(0x76);
	send_data(0x00);
	send_data(0x78);
	send_data(0x00);
	send_data(0x98);
	send_data(0x00);
	send_data(0xBB);
	send_data(0x00);
	send_data(0x99);
	send_data(0x00);
	send_data(0x66);
	send_data(0x00);
	send_data(0x54);
	send_data(0x00);
	send_data(0x45);
	send_data(0x00);
	send_data(0x34);
	send_data(0x00);
	send_data(0x44);
	send_data(0x00);
	send_data(0x34);
	send_data(0x00);
	send_cmd(0xE9);
	send_data(0xAA);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);

	send_cmd(0x00);
	send_data(0xAA);

	send_cmd(0xF3);
	send_data(0x00);

	send_cmd(0xF9);
	send_data(0x06);
	send_data(0x10);
	send_data(0x29);
	send_data(0x00);

	send_cmd(0xCF);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);

	send_cmd(0xF0);
	send_data(0x00);
	send_data(0x50);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);

	send_cmd(0x3B);
	send_data(0x00);

	send_cmd(0x35);
	send_data(0x00);

	send_cmd(0x44);
	send_data(0x00);
	send_data(0xC0);

	send_cmd(0x36);
	send_data(0x00);

	send_cmd(0x3A);
	send_data(0x66);

	send_cmd(0xB3);
	send_data(0x20);

	send_cmd(0x11);
	mdelay(120);
	send_cmd(0x29);
	mdelay(20);
#else	
	send_cmd(0xED);
	send_data(0x01); 
	send_data(0xFE);
	send_cmd(0xEE);
	send_data(0xDE);
	send_data(0x21);

	mdelay(100);

	send_cmd(0xF3);   
	send_data(0x00); 


	send_cmd(0xF1);
	send_data(0x01); 
	send_cmd(0xDF);
	send_data(0x10);  

	send_cmd(0xC0);
	send_data(0x48);
	send_data(0x48);
	send_data(0x10);
	send_data(0x10);

	send_cmd(0xB4);
	send_data(0x0D);
	send_cmd(0xC4);
	send_data(0x72);//4//78//0x6A

	send_cmd(0xBF);
	send_data(0xAA);
	send_cmd(0xB0);
	send_data(0x0D);
	send_data(0x00); 
	send_data(0x0D);
	send_data(0x00);
	send_data(0x11);
	send_data(0x00);
	send_data(0x19);
	send_data(0x00);
	send_data(0x21);
	send_data(0x00);
	send_data(0x2D);
	send_data(0x00);
	send_data(0x3D);
	send_data(0x00); 
	send_data(0x5D);
	send_data(0x00);
	send_data(0x5D);
	send_data(0x00);

	send_cmd(0xB1);
	send_data(0x80);
	send_data(0x00); 
	send_data(0x8B);
	send_data(0x00); 
	send_data(0x96);
	send_data(0x00);

	send_cmd(0xB2);
	send_data(0x00);
	send_data(0x00); 
	send_data(0x02);
	send_data(0x00); 
	send_data(0x03);
	send_data(0x00);
	send_cmd(0xB3);
	send_data(0x00);
	send_data(0x00); 
	send_data(0x00);
	send_data(0x00);  
	send_data(0x00);  
	send_data(0x00);  
	send_data(0x00);  
	send_data(0x00);  
	send_data(0x00);  
	send_data(0x00);  
	send_data(0x00);  
	send_data(0x00);  
	send_data(0x00); 
	send_data(0x00); 
	send_data(0x00); 
	send_data(0x00); 
	send_data(0x00); 
	send_data(0x00); 
	send_data(0x00); 
	send_data(0x00); 
	send_data(0x00); 
	send_data(0x00); 
	send_data(0x00); 
	send_data(0x00); 
	send_cmd(0xB4);
	send_data(0x8B);
	send_data(0x00);
	send_data(0x96);
	send_data(0x00); 
	send_data(0xA1);
	send_data(0x00);
	send_cmd(0xB5);
	send_data(0x02);
	send_data(0x00); 
	send_data(0x03);
	send_data(0x00); 
	send_data(0x04);
	send_data(0x00);
	send_cmd(0xB6);
	send_data(0x00);
	send_data(0x00);
	send_cmd(0xB8);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_cmd(0xC2);
	send_data(0x0A);
	send_data(0x00);
	send_data(0x04);
	send_data(0x00);
	send_cmd(0xC7);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_cmd(0xC9);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_cmd(0xB7);
	send_data(0x3F);
	send_data(0x00);
	send_data(0x5E);
	send_data(0x00);
	send_data(0x9E);
	send_data(0x00);
	send_data(0x74);
	send_data(0x00);
	send_data(0x8C);
	send_data(0x00);
	send_data(0xAC);
	send_data(0x00);
	send_data(0xDC);
	send_data(0x00);
	send_data(0x70);
	send_data(0x00);
	send_data(0xB9);
	send_data(0x00);
	send_data(0xEC);
	send_data(0x00);
	send_data(0xDC);
	send_data(0x00); 

	send_cmd(0xE0);
	send_data(0x00);
	send_data(0x00);
	send_data(0x07);
	send_data(0x00);
	send_data(0x19);
	send_data(0x00);
	send_data(0x3A);
	send_data(0x00);
	send_data(0x4E);
	send_data(0x00);
	send_data(0x5C);
	send_data(0x00);
	send_data(0x6E);
	send_data(0x00);
	send_data(0x85);
	send_data(0x00);
	send_data(0x93);
	send_data(0x00);
	send_data(0x9F);
	send_data(0x00);
	send_data(0xAA);
	send_data(0x00);
	send_data(0xB7);
	send_data(0x00);
	send_data(0xC0);
	send_data(0x00);
	send_data(0xC5);
	send_data(0x00);
	send_data(0xCC);
	send_data(0x00);
	send_data(0xD0);
	send_data(0x00);
	send_data(0xD9);
	send_data(0x00);
	send_data(0xF3);
	send_data(0x00);

	send_cmd(0xE1);
	send_data(0x00);
	send_data(0x00);
	send_data(0x07);
	send_data(0x00);
	send_data(0x19);
	send_data(0x00);
	send_data(0x3A);
	send_data(0x00);
	send_data(0x4E);
	send_data(0x00);
	send_data(0x5C);
	send_data(0x00);
	send_data(0x6E);
	send_data(0x00);
	send_data(0x85);
	send_data(0x00);
	send_data(0x93);
	send_data(0x00);
	send_data(0x9F);
	send_data(0x00);
	send_data(0xAA);
	send_data(0x00);
	send_data(0xB7);
	send_data(0x00);
	send_data(0xC0);
	send_data(0x00);
	send_data(0xC5);
	send_data(0x00);
	send_data(0xCC);
	send_data(0x00);
	send_data(0xD0);
	send_data(0x00);
	send_data(0xD9);
	send_data(0x00);
	send_data(0xF3);
	send_data(0x00);

	send_cmd(0xE2);
	send_data(0x00);
	send_data(0x00);
	send_data(0x07);
	send_data(0x00);
	send_data(0x19);
	send_data(0x00);
	send_data(0x3A);
	send_data(0x00);
	send_data(0x4E);
	send_data(0x00);
	send_data(0x5C);
	send_data(0x00);
	send_data(0x6E);
	send_data(0x00);
	send_data(0x85);
	send_data(0x00);
	send_data(0x93);
	send_data(0x00);
	send_data(0x9F);
	send_data(0x00);
	send_data(0xAA);
	send_data(0x00);
	send_data(0xB7);
	send_data(0x00);
	send_data(0xC0);
	send_data(0x00);
	send_data(0xC5);
	send_data(0x00);
	send_data(0xCC);
	send_data(0x00);
	send_data(0xD0);
	send_data(0x00);
	send_data(0xD9);
	send_data(0x00);
	send_data(0xF3);
	send_data(0x00);

	send_cmd(0xE3);
	send_data(0x00);
	send_data(0x00);
	send_data(0x07);
	send_data(0x00);
	send_data(0x19);
	send_data(0x00);
	send_data(0x3A);
	send_data(0x00);
	send_data(0x4E);
	send_data(0x00);
	send_data(0x5C);
	send_data(0x00);
	send_data(0x6E);
	send_data(0x00);
	send_data(0x85);
	send_data(0x00);
	send_data(0x93);
	send_data(0x00);
	send_data(0x9F);
	send_data(0x00);
	send_data(0xAA);
	send_data(0x00);
	send_data(0xB7);
	send_data(0x00);
	send_data(0xC0);
	send_data(0x00);
	send_data(0xC5);
	send_data(0x00);
	send_data(0xCC);
	send_data(0x00);
	send_data(0xD0);
	send_data(0x00);
	send_data(0xD9);
	send_data(0x00);
	send_data(0xF3);
	send_data(0x00);

	send_cmd(0xE4);
	send_data(0x00);
	send_data(0x00);
	send_data(0x07);
	send_data(0x00);
	send_data(0x19);
	send_data(0x00);
	send_data(0x3A);
	send_data(0x00);
	send_data(0x4E);
	send_data(0x00);
	send_data(0x5C);
	send_data(0x00);
	send_data(0x6E);
	send_data(0x00);
	send_data(0x85);
	send_data(0x00);
	send_data(0x93);
	send_data(0x00);
	send_data(0x9F);
	send_data(0x00);
	send_data(0xAA);
	send_data(0x00);
	send_data(0xB7);
	send_data(0x00);
	send_data(0xC0);
	send_data(0x00);
	send_data(0xC5);
	send_data(0x00);
	send_data(0xCC);
	send_data(0x00);
	send_data(0xD0);
	send_data(0x00);
	send_data(0xD9);
	send_data(0x00);
	send_data(0xF3);
	send_data(0x00);

	send_cmd(0xE5);
	send_data(0x00);
	send_data(0x00);
	send_data(0x07);
	send_data(0x00);
	send_data(0x19);
	send_data(0x00);
	send_data(0x3A);
	send_data(0x00);
	send_data(0x4E);
	send_data(0x00);
	send_data(0x5C);
	send_data(0x00);
	send_data(0x6E);
	send_data(0x00);
	send_data(0x85);
	send_data(0x00);
	send_data(0x93);
	send_data(0x00);
	send_data(0x9F);
	send_data(0x00);
	send_data(0xAA);
	send_data(0x00);
	send_data(0xB7);
	send_data(0x00);
	send_data(0xC0);
	send_data(0x00);
	send_data(0xC5);
	send_data(0x00);
	send_data(0xCC);
	send_data(0x00);
	send_data(0xD0);
	send_data(0x00);
	send_data(0xD9);
	send_data(0x00);
	send_data(0xF3);
	send_data(0x00); 


	send_cmd(0xE6);
	send_data(0x21);
	send_data(0x00);
	send_data(0x55);
	send_data(0x00);
	send_data(0x99);
	send_data(0x00);
	send_data(0x77);
	send_data(0x00);
	send_data(0x77);
	send_data(0x00);
	send_data(0x76);
	send_data(0x00);
	send_data(0x78);
	send_data(0x00);
	send_data(0x98);
	send_data(0x00);
	send_data(0xBB);
	send_data(0x00);
	send_data(0x99);
	send_data(0x00);
	send_data(0x66);
	send_data(0x00);
	send_data(0x54);
	send_data(0x00);
	send_data(0x45);
	send_data(0x00);
	send_data(0x34);
	send_data(0x00);
	send_data(0x44);
	send_data(0x00);
	send_data(0x34);
	send_data(0x00);
	send_cmd(0xE7);
	send_data(0x21);
	send_data(0x00);
	send_data(0x55);
	send_data(0x00);
	send_data(0x99);
	send_data(0x00);
	send_data(0x77);
	send_data(0x00);
	send_data(0x77);
	send_data(0x00);
	send_data(0x76);
	send_data(0x00);
	send_data(0x78);
	send_data(0x00);
	send_data(0x98);
	send_data(0x00);
	send_data(0xBB);
	send_data(0x00);
	send_data(0x99);
	send_data(0x00);
	send_data(0x66);
	send_data(0x00);
	send_data(0x54);
	send_data(0x00);
	send_data(0x45);
	send_data(0x00);
	send_data(0x34);
	send_data(0x00);
	send_data(0x44);
	send_data(0x00);
	send_data(0x34);
	send_data(0x00);
	send_cmd(0xE8);
	send_data(0x21);
	send_data(0x00);
	send_data(0x55);
	send_data(0x00);
	send_data(0x99);
	send_data(0x00);
	send_data(0x77);
	send_data(0x00);
	send_data(0x77);
	send_data(0x00);
	send_data(0x76);
	send_data(0x00);
	send_data(0x78);
	send_data(0x00);
	send_data(0x98);
	send_data(0x00);
	send_data(0xBB);
	send_data(0x00);
	send_data(0x99);
	send_data(0x00);
	send_data(0x66);
	send_data(0x00);
	send_data(0x54);
	send_data(0x00);
	send_data(0x45);
	send_data(0x00);
	send_data(0x34);
	send_data(0x00);
	send_data(0x44);
	send_data(0x00);
	send_data(0x34);
	send_data(0x00);
	send_cmd(0xE9);
	send_data(0xAA);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_cmd(0x00);
	send_data(0xAA);

	send_cmd(0xCF);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);

	send_cmd(0xF0);
	send_data(0x00);
	send_data(0x50);
	send_data(0x00);
	send_data(0x00);
	send_data(0x00);

	send_cmd(0xF1);
	send_data(0x01);


	send_cmd(0xEE);
	send_data(0xDE);
	send_data(0x21);
	send_cmd(0xF3);
	send_data(0x00);


	send_cmd(0xB3);
	send_data(0x20);//41//01
	send_cmd(0x3B);
	send_data(0x08);
	send_cmd(0x35);
	send_data(0x00);
	send_cmd(0x44);
	send_data(0x00);
	send_data(0xc0);
	send_cmd(0x36);
	send_data(0x00);
	send_cmd(0x3A);
	send_data(0x66);

	send_cmd(0x11);
	mdelay(120);
	send_cmd(0x29);
	mdelay(50);
#endif
#if 0	
	send_cmd(0xE0);
	send_data(0x00);
	send_data(0x03);
	send_data(0x0C);
	send_data(0x09);
	send_data(0x17);
	send_data(0x09);
	send_data(0x3E);
	send_data(0x89);
	send_data(0x49);
	send_data(0x08);
	send_data(0x0D);
	send_data(0x0A);
	send_data(0x13);
	send_data(0x15);
	send_data(0x0F);

	send_cmd(0xE1);
	send_data(0x00);
	send_data(0x11);
	send_data(0x15);
	send_data(0x03);
	send_data(0x0F);
	send_data(0x05);
	send_data(0x2D);
	send_data(0x34);
	send_data(0x41);
	send_data(0x02);
	send_data(0x0B);
	send_data(0x0A);
	send_data(0x33);
	send_data(0x37);
	send_data(0x0F);

	send_cmd(0xc0);
	send_data(0x17);
	send_data(0x15);

	send_cmd(0xc1);
	send_data(0x41);

	send_cmd(0xc5);
	send_data(0x00);
	send_data(0x12);

	send_cmd(0xB1);
	send_data(0xA0);
	send_data(0x11);

	send_cmd(0xB0);
	send_data(0x00);

	send_cmd(0xB4);
	send_data(0x02);

	send_cmd(0xB6);
	send_data(0x02);
	send_data(0x22);

	send_cmd(0xE9);
	send_data(0x00);

	send_cmd(0xF7);
	send_data(0xA9);
	send_data(0x51);
	send_data(0x2C);
	send_data(0x82);

	send_cmd(0x35);
	send_data(0x00);

	send_cmd(0x3A);
	send_data(0x66);

	//mdelay(130);
	send_cmd(0x11);        // Sleep out
	mdelay(120);
	send_cmd(0x29);    // display on
	mdelay(35);
#endif    
#if 0	
	if (1) { //  for test the lcd
	int i;
	send_cmd(0x2C); //Write data
	for (i = 0; i < 480*320/3; i++)
	send_data(0xff);
	for (i = 0; i < 480*320/3; i++)
	send_data(0x0000);
	for (i = 0; i < 480*320/3; i++)
	send_data(0xff0000);
	}
	send_cmd(0x29); //Display On 
	mdelay(120); //120ms
	send_cmd(0x2C); //Write data
#endif
	/*Begin by leihui .fixed for pr:730677 [Power on][Display] Happen to flash screen when power on*/
	{
		int i;
	//send_cmd(0x2C); //Write data
		for (i = 0; i < 480*320; i++)
		send_data(0x0000);
	}
	//send_cmd(0x29); //Display On 
	//mdelay(80); //120ms
	//send_cmd(0x2C); //Write data
	/*End by leihui .fixed for pr:730677 [Power on][Display] Happen to flash screen when power on*/

	//LCD_PRINT("ili9488_init: end\n");

	return 0;
}

static int32_t ili9488_set_window(struct panel_spec *self,
		uint16_t left, uint16_t top, uint16_t right, uint16_t bottom)
{
	send_data_t send_cmd = self->info.mcu->ops->send_cmd;
	send_data_t send_data = self->info.mcu->ops->send_data;

	pr_debug("ili9488_set_window:%d, %d, %d, %d\n", left, top, right, bottom);

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

static int32_t ili9488_invalidate(struct panel_spec *self)
{

	return self->ops->panel_set_window(self, 0, 0,
			self->width-1, self->height-1);

}

static int32_t ili9488_invalidate_rect(struct panel_spec *self,
				uint16_t left, uint16_t top,
				uint16_t right, uint16_t bottom)
{

	pr_debug("ili9488_invalidate_rect\n");

	return self->ops->panel_set_window(self, left, top,
			right, bottom);
}

static int32_t ili9488_set_direction(struct panel_spec *self, uint16_t direction)
{

	pr_debug("ili9488_set_direction\n");
	return 0;
}

static int32_t ili9488_enter_sleep(struct panel_spec *self, uint8_t is_sleep)
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
		ili9488_init(self);
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

static uint32_t ili9488_read_id(struct panel_spec *self)
{
	uint32_t read_value = 0;
	uint8_t ID[4]={0};
	int i;
	send_data_t send_cmd = self->info.mcu->ops->send_cmd;
	read_data_t read_data = self->info.mcu->ops->read_data;
	
	send_cmd(0xD4);
	for(i=0;i<4;i++){
		ID[i]=read_data();
		printk("[%s] ID[%d] = %x\n",__FUNCTION__,i,ID[i]);
	}
	if( ID[3] == 0x10 && ID[2]==0x53)  //ili9488 id :01 01 53 10 
		read_value = 0x9488;
	printk("[%s] lcd id = 0x%x\n",__FUNCTION__,read_value);
	return read_value;
}

extern int32_t lcm_reset();
static int32_t ili9488_reset(struct panel_spec *self)
{
    printk("%s() \n",__func__);
    lcm_reset();
}

static struct panel_operations lcd_ili9488_operations = {
	.panel_init            = ili9488_init,
	.panel_set_window      = ili9488_set_window,
	.panel_invalidate      = ili9488_invalidate,
	.panel_invalidate_rect = ili9488_invalidate_rect,
	.panel_set_direction   = ili9488_set_direction,
	.panel_enter_sleep     = ili9488_enter_sleep,
	.panel_readid          = ili9488_read_id,
	.panel_reset           = ili9488_reset,
};

static struct timing_mcu lcd_ili9488_timing[] = {
	[LCD_REGISTER_TIMING] = {
		.rcss = 50,//25,                // we should need long time to write and read operation ,if not ,we can see a tear scanline . 
		.rlpw = 50,//45,                // we can also modify register 0x44, you can scale the value from the lcd spec which metioned
		.rhpw = 90,            // the method.
		.wcss = 50,//30,
		.wlpw = 50,//15,
		.whpw = 50,//15,
	},
	[LCD_GRAM_TIMING] = {
		.rcss = 50,//25,  
		.rlpw = 50,//45,
		.rhpw = 90,
		.wcss = 50,//30,
		.wlpw = 50,//15,
		.whpw = 50,//15,
	}
};

static struct info_mcu lcd_ili9488_info = {
	.bus_mode  = LCD_BUS_8080,
	.bus_width = 18,
	.timing    = lcd_ili9488_timing,
	.ops       = NULL,
};

struct panel_spec lcd_ili9488_spec = {
	.width = 320,
	.height = 480,
	.mode = LCD_MODE_MCU,
	.direction = LCD_DIRECT_NORMAL,
	.info = {
		.mcu = &lcd_ili9488_info
	},
	.ops = &lcd_ili9488_operations,
};

struct panel_cfg lcd_ili9488 = {
	.lcd_cs = -1,
	.lcd_id = 0x9488,
	.lcd_name = "lcd_ili9488",
	.panel = &lcd_ili9488_spec,
};


static int __init lcd_ili9488_init(void)
{
	return sprd_register_panel(&lcd_ili9488);
}

subsys_initcall(lcd_ili9488_init);
