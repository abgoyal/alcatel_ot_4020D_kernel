/*
 * drivers/input/touchscreen/ft5x0x_ts.c
 *
 * FocalTech ft5x0x TouchScreen driver.
 *
 * Copyright (c) 2010  Focal tech Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * VERSION      	DATE			AUTHOR
 *    1.0		  2010-01-05			WenFS
 *
 * note: only support mulititouch	Wenfs 2010-10-01
 */

#include <linux/firmware.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <asm/uaccess.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>
//#include <linux/i2c/ft53x6_ts.h>
#include <mach/regulator.h>
#include <linux/input/mt.h>

#ifdef CONFIG_I2C_SPRD
#include <mach/i2c-sprd.h>
#endif
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#define TPD_PROXIMITY

#ifdef TPD_PROXIMITY
#include <linux/miscdevice.h>
#include <linux/wakelock.h>

#endif

#define APK_DEBUG
#define SPRD_AUTO_UPGRADE
#define SYSFS_DEBUG
#define FTS_CTL_IIC
#include "focaltech.h"
#include "focaltech_ex_fun.h"
#include "focaltech_ctl.h"


#define	USE_THREADED_IRQ	1
#define	TOUCH_VIRTUAL_KEYS
#define	MULTI_PROTOCOL_TYPE_B	1
#define	TS_MAX_FINGER		2

#define	FTS_PACKET_LENGTH	128
/*
static unsigned char FT5316_FW[]=
{
#include "FT_0513_app_o.i"
};

static unsigned char FT5306_FW[]=
{
#include "FT_0513_app.i"
};

static unsigned char *CTPM_FW = FT5306_FW;
*/
static int fw_size;

static struct ft5x0x_ts_data *g_ft5x0x_ts=NULL;
static struct i2c_client *this_client=NULL;

static unsigned char suspend_flag = 0;


 struct Upgrade_Info fts_updateinfo[] =
{
        {0x55,"FT5x06",TPD_MAX_POINTS_5,AUTO_CLB_NEED,50, 30, 0x79, 0x03, 1, 2000},
        {0x08,"FT5606",TPD_MAX_POINTS_5,AUTO_CLB_NEED,50, 30, 0x79, 0x06, 100, 2000},
	{0x0a,"FT5x16",TPD_MAX_POINTS_5,AUTO_CLB_NEED,50, 30, 0x79, 0x07, 1, 1500},
	{0x05,"FT6208",TPD_MAX_POINTS_2,AUTO_CLB_NONEED,60, 30, 0x79, 0x05, 10, 2000},
	{0x06,"FT6x06",TPD_MAX_POINTS_2,AUTO_CLB_NONEED,100, 30, 0x79, 0x08, 10, 2000},
	{0x55,"FT5x06i",TPD_MAX_POINTS_5,AUTO_CLB_NEED,50, 30, 0x79, 0x03, 1, 2000},
	{0x14,"FT5336",TPD_MAX_POINTS_5,AUTO_CLB_NEED,30, 30, 0x79, 0x11, 10, 2000},
	{0x13,"FT3316",TPD_MAX_POINTS_5,AUTO_CLB_NEED,30, 30, 0x79, 0x11, 10, 2000},
	{0x12,"FT5436i",TPD_MAX_POINTS_5,AUTO_CLB_NEED,30, 30, 0x79, 0x11, 10, 2000},
	{0x11,"FT5336i",TPD_MAX_POINTS_5,AUTO_CLB_NEED,30, 30, 0x79, 0x11, 10, 2000},
};
				
struct Upgrade_Info fts_updateinfo_curr;
#if 0//dennis
/* Attribute */
static ssize_t ft5x0x_show_suspend(struct device* cd,struct device_attribute *attr, char* buf);
static ssize_t ft5x0x_store_suspend(struct device* cd, struct device_attribute *attr,const char* buf, size_t len);
static ssize_t ft5x0x_show_version(struct device* cd,struct device_attribute *attr, char* buf);
static ssize_t ft5x0x_update(struct device* cd, struct device_attribute *attr, const char* buf, size_t len);
#endif

#ifdef TPD_PROXIMITY
static struct ft5x0x_ts_data *ft6306_data = NULL;
static struct wake_lock ps_wake_lock;
static char ps_data_state[1] = {1};

//#define FT53X6_DBG
#ifdef FT53X6_DBG
#define APS_ERR(fmt,arg...)           	printk("<<proximity>> "fmt"\n",##arg)
#define TPD_PROXIMITY_DEBUG(fmt,arg...) printk("<<proximity>> "fmt"\n",##arg)
#define TPD_PROXIMITY_DMESG(fmt,arg...) printk("<<proximity>> "fmt"\n",##arg)
#else
#define APS_ERR(fmt,arg...)
#define TPD_PROXIMITY_DEBUG(fmt,arg...)
#define TPD_PROXIMITY_DMESG(fmt,arg...)
#endif


static u8 tpd_proximity_flag 			= 0;
static u8 tpd_proximity_suspend 		= 0;
static int tpd_enable_ps(int enable);

#endif


static unsigned char ft5x0x_read_fw_ver(void);

#ifdef CONFIG_HAS_EARLYSUSPEND
static void ft5x0x_ts_suspend(struct early_suspend *handler);
static void ft5x0x_ts_resume(struct early_suspend *handler);
#endif
//static int fts_ctpm_fw_update(void);
static int fts_ctpm_fw_upgrade_with_i_file(void);

struct ts_event {
	u16	x1;
	u16	y1;
	u16	x2;
	u16	y2;
	u16	x3;
	u16	y3;
	u16	x4;
	u16	y4;
	u16	x5;
	u16	y5;
	u16	pressure;
    u8  touch_point;
};

struct ft5x0x_ts_data {
	struct input_dev	*input_dev;
	struct i2c_client	*client;
	struct ts_event	event;
#if !USE_THREADED_IRQ
	struct work_struct	pen_event_work;
	struct workqueue_struct	*ts_workqueue;
#endif
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct work_struct       resume_work;
	struct workqueue_struct *ts_resume_workqueue;
	struct early_suspend	early_suspend;
#endif
	struct ft5x0x_ts_platform_data	*platform_data;
#ifdef TPD_PROXIMITY
	struct input_dev	*ps_input;
	struct workqueue_struct 	*ps_wq;
	struct work_struct 	ps_work;
#endif

};

#if 0//dennis
static DEVICE_ATTR(suspend, S_IRUGO | S_IWUSR, ft5x0x_show_suspend, ft5x0x_store_suspend);
static DEVICE_ATTR(update, S_IRUGO | S_IWUSR, ft5x0x_show_version, ft5x0x_update);

static ssize_t ft5x0x_show_suspend(struct device* cd,
				     struct device_attribute *attr, char* buf)
{
	ssize_t ret = 0;

	if(suspend_flag==1)
		sprintf(buf, "FT5x0x Suspend\n");
	else
		sprintf(buf, "FT5x0x Resume\n");

	ret = strlen(buf) + 1;

	return ret;
}

static ssize_t ft5x0x_store_suspend(struct device* cd, struct device_attribute *attr,
		       const char* buf, size_t len)
{
	unsigned long on_off = simple_strtoul(buf, NULL, 10);
	suspend_flag = on_off;

	if(on_off==1)
	{
		pr_info("FT5x0x Entry Suspend\n");
	#ifdef CONFIG_HAS_EARLYSUSPEND
		ft5x0x_ts_suspend(NULL);
	#endif
	}
	else
	{
		pr_info("FT5x0x Entry Resume\n");
	#ifdef CONFIG_HAS_EARLYSUSPEND
		ft5x0x_ts_resume(NULL);
	#endif
	}

	return len;
}

static ssize_t ft5x0x_show_version(struct device* cd,
				     struct device_attribute *attr, char* buf)
{
	ssize_t ret = 0;
	unsigned char uc_reg_value; 

	uc_reg_value = ft5x0x_read_fw_ver();

	sprintf(buf, "ft5x0x firmware id is V%x\n", uc_reg_value);

	ret = strlen(buf) + 1;

	return ret;
}

static ssize_t ft5x0x_update(struct device* cd, struct device_attribute *attr,
		       const char* buf, size_t len)
{
	unsigned long on_off = simple_strtoul(buf, NULL, 10);
	unsigned char uc_reg_value;

	uc_reg_value = ft5x0x_read_fw_ver();

	if(on_off==1)
	{
		pr_info("ft5x0x update, current firmware id is V%x\n", uc_reg_value);
		//fts_ctpm_fw_update();
		fts_ctpm_fw_upgrade_with_i_file();
	}

	return len;
}

static int ft5x0x_create_sysfs(struct i2c_client *client)
{
	int err;
	struct device *dev = &(client->dev);

	err = device_create_file(dev, &dev_attr_suspend);
	err = device_create_file(dev, &dev_attr_update);

	return err;
}
#endif
static int ft5x0x_i2c_rxdata(char *rxdata, int length)
{
	int ret = 0;

	struct i2c_msg msgs[] = {
		{
			.addr	= this_client->addr,
			.flags	= 0,
			.len	= 1,
			.buf	= rxdata,
		},
		{
			.addr	= this_client->addr,
			.flags	= I2C_M_RD,
			.len	= length,
			.buf	= rxdata,
		},
	};

	if (i2c_transfer(this_client->adapter, msgs, 2) != 2) {
		ret = -EIO;
		pr_err("msg %s i2c read error: %d\n", __func__, ret);
	}

	return ret;
}

static int ft5x0x_i2c_txdata(char *txdata, int length)
{
	int ret = 0;

	struct i2c_msg msg[] = {
		{
			.addr	= this_client->addr,
			.flags	= 0,
			.len	= length,
			.buf	= txdata,
		},
	};

	if (i2c_transfer(this_client->adapter, msg, 1) != 1) {
		ret = -EIO;
		pr_err("%s i2c write error: %d\n", __func__, ret);
	}

	return ret;
}

/***********************************************************************************************
Name	:	 ft5x0x_write_reg

Input	:	addr -- address
                     para -- parameter

Output	:

function	:	write register of ft5x0x

***********************************************************************************************/
static int ft5x0x_write_reg(u8 addr, u8 para)
{
	u8 buf[3];
	int ret = -1;

	buf[0] = addr;
	buf[1] = para;
	ret = ft5x0x_i2c_txdata(buf, 2);
	if (ret < 0) {
		pr_err("write reg failed! %#x ret: %d", buf[0], ret);
		return -1;
	}

	return 0;
}

/***********************************************************************************************
Name	:	ft5x0x_read_reg

Input	:	addr
                     pdata

Output	:

function	:	read register of ft5x0x

***********************************************************************************************/
static int ft5x0x_read_reg(u8 addr, u8 *pdata)
{
	int ret = 0;
	u8 buf[2] = {addr, 0};

	struct i2c_msg msgs[] = {
		{
			.addr	= this_client->addr,
			.flags	= 0,
			.len	= 1,
			.buf	= buf,
		},
		{
			.addr	= this_client->addr,
			.flags	= I2C_M_RD,
			.len	= 1,
			.buf	= buf+1,
		},
	};

	if (i2c_transfer(this_client->adapter, msgs, 2) != 2) {
		ret = -EIO;
		pr_err("msg %s i2c read error: %d\n", __func__, ret);
	}

	*pdata = buf[1];
	return ret;
}


#ifdef TOUCH_VIRTUAL_KEYS

static ssize_t virtual_keys_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{ 
    #if 0
	unsigned char uc_reg_value;

	ft5x0x_read_reg(FT5X0X_REG_CIPHER, &uc_reg_value);
      
	if (uc_reg_value == 0x0a || uc_reg_value == 0x0)
		return sprintf(buf,
        		 __stringify(EV_KEY) ":" __stringify(KEY_MENU)   ":133:1360:107:87"
	 		":" __stringify(EV_KEY) ":" __stringify(KEY_HOMEPAGE)   ":373:1360:107:87"
	 		":" __stringify(EV_KEY) ":" __stringify(KEY_BACK) ":627:1360:107:87"
			"\n");
	else
		return sprintf(buf,
         		__stringify(EV_KEY) ":" __stringify(KEY_MENU)   ":100:1020:80:65"
	 		":" __stringify(EV_KEY) ":" __stringify(KEY_HOMEPAGE)   ":280:1020:80:65"
	 		":" __stringify(EV_KEY) ":" __stringify(KEY_BACK) ":470:1020:80:65"
	 		"\n");
     #endif
	 
    return sprintf(buf,
	 		__stringify(EV_KEY) ":" __stringify(KEY_HOMEPAGE)   ":120:560:107:87"
			"\n");
	 
 }

static struct kobj_attribute virtual_keys_attr = {
    .attr = {
        .name = "virtualkeys.focaltech_ts",
        .mode = S_IRUGO,
    },
    .show = &virtual_keys_show,
};

static struct attribute *properties_attrs[] = {
    &virtual_keys_attr.attr,
    NULL
};

static struct attribute_group properties_attr_group = {
    .attrs = properties_attrs,
};

static void ft5x0x_ts_virtual_keys_init(void)
{
    int ret = 0;
    struct kobject *properties_kobj;

    pr_info("[FST] %s\n",__func__);

    properties_kobj = kobject_create_and_add("board_properties", NULL);
    if (properties_kobj)
        ret = sysfs_create_group(properties_kobj,
                     &properties_attr_group);
    if (!properties_kobj || ret)
        pr_err("failed to create board_properties\n");
}

#endif

/***********************************************************************************************
Name	:	 ft5x0x_read_fw_ver

Input	:	 void

Output	:	 firmware version

function	:	 read TP firmware version

***********************************************************************************************/
static unsigned char ft5x0x_read_fw_ver(void)
{
	unsigned char ver;
	ft5x0x_read_reg(FT5X0X_REG_FIRMID, &ver);
	return(ver);
}



static void ft5x0x_clear_report_data(struct ft5x0x_ts_data *ft5x0x_ts)
{
	int i;

	for(i = 0; i < TS_MAX_FINGER; i++) {
	#if MULTI_PROTOCOL_TYPE_B
		input_mt_slot(ft5x0x_ts->input_dev, i);
		input_mt_report_slot_state(ft5x0x_ts->input_dev, MT_TOOL_FINGER, false);
	#endif
		input_report_key(ft5x0x_ts->input_dev, BTN_TOUCH, 0);
	#if !MULTI_PROTOCOL_TYPE_B
		input_mt_sync(ft5x0x_ts->input_dev);
	#endif
	}
	input_sync(ft5x0x_ts->input_dev);
}

static int ft5x0x_update_data(void)
{
	struct ft5x0x_ts_data *data = i2c_get_clientdata(this_client);
	struct ts_event *event = &data->event;
	u8 buf[32] = {0};
	int ret = -1;
	int i;
	u16 x , y;
#ifdef TPD_PROXIMITY
			u8 proximity_status;
			u8 state;
#endif
		
#ifdef TPD_PROXIMITY
				if (tpd_proximity_flag == 1)
				{
					i2c_smbus_read_i2c_block_data(this_client, 0xB0, 1, &state);
					TPD_PROXIMITY_DEBUG("proxi_5206 0xB0 state value is 1131 0x%02X\n", state);
		
					if(!(state&0x01))
					{
						tpd_enable_ps(1);
					}
		
					i2c_smbus_read_i2c_block_data(this_client, 0x01, 1, &proximity_status);
					TPD_PROXIMITY_DEBUG("proxi_5206 0x01 value is 1139 0x%02X\n", proximity_status);
					
					if (proximity_status == 0xC0)
					{
						//tpd_proximity_detect = 0; 
						ps_data_state[0]=0;
					}
					else if(proximity_status == 0xE0)
					{
						//tpd_proximity_detect = 1;
						ps_data_state[0]=1;
					}
		
					TPD_PROXIMITY_DEBUG("tpd_proximity_detect 1149 = %d\n", ps_data_state[0]);
		
					input_report_abs(data->ps_input, ABS_DISTANCE, ps_data_state[0]);
					input_sync(data->ps_input);
				}  
#endif

	ret = ft5x0x_i2c_rxdata(buf, 31);

	if (ret < 0) {
		pr_err("%s read_data i2c_rxdata failed: %d\n", __func__, ret);
		return ret;
	}

	memset(event, 0, sizeof(struct ts_event));
	event->touch_point = buf[2] & 0x07;

	for(i = 0; i < TS_MAX_FINGER; i++) {
		if((buf[6*i+3] & 0xc0) == 0xc0)
			continue;
		x = (s16)(buf[6*i+3] & 0x0F)<<8 | (s16)buf[6*i+4];	
		y = (s16)(buf[6*i+5] & 0x0F)<<8 | (s16)buf[6*i+6];
		if((buf[6*i+3] & 0x40) == 0x0) {
		#if MULTI_PROTOCOL_TYPE_B
			input_mt_slot(data->input_dev, i);
			input_mt_report_slot_state(data->input_dev, MT_TOOL_FINGER, true);
		#endif
			input_report_abs(data->input_dev, ABS_MT_POSITION_X, x);
			input_report_abs(data->input_dev, ABS_MT_POSITION_Y, y);
			input_report_key(data->input_dev, BTN_TOUCH, 1);
		#if !MULTI_PROTOCOL_TYPE_B
			input_mt_sync(data->input_dev);
		#endif
		//	printk("===x%d = %d,y%d = %d \n",i, x, i, y);
		}
		else {
		#if MULTI_PROTOCOL_TYPE_B
			input_mt_slot(data->input_dev, i);
			input_mt_report_slot_state(data->input_dev, MT_TOOL_FINGER, false);
		#endif
			input_report_key(data->input_dev, BTN_TOUCH, 0);
		#if !MULTI_PROTOCOL_TYPE_B
			input_mt_sync(data->input_dev);
		#endif
		}
		
	}
	input_sync(data->input_dev);

	return 0;
}

#if !USE_THREADED_IRQ
static void ft5x0x_ts_pen_irq_work(struct work_struct *work)
{
	ft5x0x_update_data();
	enable_irq(this_client->irq);
}
#endif

static irqreturn_t ft5x0x_ts_interrupt(int irq, void *dev_id)
{
#if !USE_THREADED_IRQ
	struct ft5x0x_ts_data *ft5x0x_ts = (struct ft5x0x_ts_data *)dev_id;

	if (!work_pending(&ft5x0x_ts->pen_event_work)) {
		queue_work(ft5x0x_ts->ts_workqueue, &ft5x0x_ts->pen_event_work);
	}
#else
	ft5x0x_update_data();
#endif

	return IRQ_HANDLED;
}
#ifdef TPD_PROXIMITY
enum
{
       DISABLE_CTP_PS = 0,
       ENABLE_CTP_PS = 1,
       RESET_CTP_PS
};

static ssize_t show_proximity_sensor_status(struct device *dev, struct device_attribute *attr, char *buf)
{
	printk("%s : proximity_sensor_status = %d\n",__func__,ps_data_state[0]);
	return sprintf(buf, "0x%02x   \n",ps_data_state[0]);
}

static ssize_t show_proximity_sensor_enable(struct device *dev, struct device_attribute *attr, char *buf)
{
	//unsigned char temp;
	printk("%s : tpd_proximity_flag = %d\n",__func__,tpd_proximity_flag);
	return sprintf(buf, "0x%02x   \n",tpd_proximity_flag);
}
static ssize_t store_proximity_sensor_enable(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int val;
    int ret;
    u8 state;
	
	if(buf != NULL && size != 0)
	{
		sscanf(buf,"%d",&val);
	//	printk("%s : val =%d\n",__func__,val);
		i2c_smbus_read_i2c_block_data(this_client, 0xB0, 1, &state);

		if(DISABLE_CTP_PS == val)
		{
			state &= 0x00;	
			tpd_proximity_flag = 0;

			printk("disable_CTP_PS buf=%d,size=%d,val=%d\n", *buf, size,val);


			msleep(200);

			wake_unlock(&ps_wake_lock);
		}

		else if(ENABLE_CTP_PS == val)
		{
			wake_lock(&ps_wake_lock);
			state |= 0x01;
		    tpd_proximity_flag = 1;
			printk("enable_CTP_PS buf=%d,size=%d,val=%d\n", *buf, size,val);

			if(ft6306_data)
			{
				input_report_abs(ft6306_data->ps_input, ABS_DISTANCE, ps_data_state[0]);
			}
		}
		ret = i2c_smbus_write_i2c_block_data(this_client, 0xB0, 1, &state);
	}
	return size;
}
static DEVICE_ATTR(enable, 0777, show_proximity_sensor_enable, store_proximity_sensor_enable);
static DEVICE_ATTR(status, 0777, show_proximity_sensor_status,NULL);
static struct attribute *msg2133_proximity_attributes[] = {
	&dev_attr_enable.attr,
	&dev_attr_status.attr,
	NULL
};

static struct attribute_group msg2133_proximity_attribute_group = {
	.attrs = msg2133_proximity_attributes
};


static int tpd_enable_ps(int enable)
{
	u8 state;
	int ret = -1;

	i2c_smbus_read_i2c_block_data(this_client, 0xB0, 1, &state);
	TPD_PROXIMITY_DEBUG("[proxi_5206]read: 999 0xb0's value is 0x%02X\n", state);
	if (enable){
	      #ifdef WAKE_LOCK_LPSENSOR
            	wake_lock(&pls_delayed_work_wake_lock);
            	#endif
		state |= 0x01;
		tpd_proximity_flag = 1;
		TPD_PROXIMITY_DEBUG("[proxi_5206]ps function is on\n");	
		
	}else{
	        #ifdef WAKE_LOCK_LPSENSOR
            	wake_unlock(&pls_delayed_work_wake_lock);
            	#endif
		state &= 0x00;	
		tpd_proximity_flag = 0;
		TPD_PROXIMITY_DEBUG("[proxi_5206]ps function is off\n");
		
	}

	ret = i2c_smbus_write_i2c_block_data(this_client, 0xB0, 1, &state);
	TPD_PROXIMITY_DEBUG("[proxi_5206]write: 0xB0's value is 0x%02X\n", state);
	return 0;
}


#endif

static void ft5x0x_ts_reset(void)
{
	//struct ft5x0x_ts_platform_data *pdata = g_ft5x0x_ts->platform_data;

	gpio_direction_output(GPIO_TOUCH_RESET, 1);
	msleep(1);
	gpio_set_value(GPIO_TOUCH_RESET, 0);
	msleep(10);
	gpio_set_value(GPIO_TOUCH_RESET, 1);
	msleep(200);
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void ft5x0x_ts_suspend(struct early_suspend *handler)
{
    int ret=-1;
	int count =5;
	pr_info("==%s==\n", __FUNCTION__);
#ifdef TPD_PROXIMITY
	if (tpd_proximity_flag == 1)
	{
		tpd_proximity_suspend = 0;	
		return;
	}
	else
	{
		tpd_proximity_suspend = 1;
	}
#endif

	ret = ft5x0x_write_reg(FT5X0X_REG_PMODE, PMODE_HIBERNATE);
	while(ret != 0 && count != 0) {
			//printk("trying to enter hibernate again. ret = %d\n", ret);
			msleep(10);
			ret = ft5x0x_write_reg(FT5X0X_REG_PMODE, PMODE_HIBERNATE);
			count--;
	}
	disable_irq(this_client->irq);
	ft5x0x_clear_report_data(g_ft5x0x_ts);
}

static void ft5x0x_ts_resume(struct early_suspend *handler)
{
	struct ft5x0x_ts_data  *ft5x0x_ts = (struct ft5x0x_ts_data *)i2c_get_clientdata(this_client);

#ifdef TPD_PROXIMITY	
	if (tpd_proximity_suspend == 0)
	{
		return;
	}
	else
	{
		tpd_proximity_suspend = 0;
	}
#endif	

	queue_work(ft5x0x_ts->ts_resume_workqueue, &ft5x0x_ts->resume_work);
}

static void ft5x0x_ts_resume_work(struct work_struct *work)
{
	pr_info("==%s==\n", __FUNCTION__);

	ft5x0x_ts_reset();
	ft5x0x_write_reg(FT5X0X_REG_PERIODACTIVE, 7);
	msleep(200);
	enable_irq(this_client->irq);
}
#endif

static void ft5x0x_ts_hw_init(struct ft5x0x_ts_data *ft5x0x_ts)
{
	struct ft5x0x_ts_platform_data *pdata = ft5x0x_ts->platform_data;
	struct regulator* reg_vdd;
	printk(KERN_INFO "%s [irq=%d];[rst=%d]\n",__func__,pdata->irq_gpio_number,pdata->reset_gpio_number);
	gpio_request(GPIO_TOUCH_IRQ, "ts_irq_pin");
	gpio_request(GPIO_TOUCH_RESET, "ts_rst_pin");
	gpio_direction_output(GPIO_TOUCH_RESET, 1);
	gpio_direction_input(GPIO_TOUCH_IRQ);
	//vdd power on
	reg_vdd = regulator_get(&ft5x0x_ts->client->dev, REGU_NAME_TP);
	regulator_set_voltage(reg_vdd, 2800000, 2800000);
	regulator_enable(reg_vdd);

	reg_vdd = regulator_get(&ft5x0x_ts->client->dev, REGU_NAME_TP1);
        regulator_set_voltage(reg_vdd, 1800000, 1800000);
        regulator_enable(reg_vdd);
	msleep(100);
	ft5x0x_ts_reset();
}
static int ft5x0x_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct ft5x0x_ts_data *ft5x0x_ts;
	struct input_dev *input_dev;
	struct ft5x0x_ts_platform_data *pdata = client->dev.platform_data;
	int err = 0;
	unsigned char uc_reg_value;
	u8 chip_id,i;

	pr_info("[FST] %s: probe\n",__func__);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		err = -ENODEV;
		goto exit_check_functionality_failed;
	}

	ft5x0x_ts = kzalloc(sizeof(*ft5x0x_ts), GFP_KERNEL);
	if (!ft5x0x_ts)	{
		err = -ENOMEM;
		goto exit_alloc_data_failed;
	}

	g_ft5x0x_ts = ft5x0x_ts;
	ft5x0x_ts->platform_data = pdata;
	this_client = client;
	ft5x0x_ts->client = client;
	ft5x0x_ts_hw_init(ft5x0x_ts);
	i2c_set_clientdata(client, ft5x0x_ts);
	client->irq = gpio_to_irq(GPIO_TOUCH_IRQ);

	#ifdef CONFIG_I2C_SPRD
	sprd_i2c_ctl_chg_clk(client->adapter->nr, 400000);
	#endif

	err = ft5x0x_read_reg(FT5X0X_REG_CIPHER, &uc_reg_value);
	if (err < 0)
	{
		pr_err("[FST] read chip id error %x\n", uc_reg_value);
		err = -ENODEV;
		goto exit_alloc_data_failed;
	}
       
	/* set report rate, about 70HZ */
	ft5x0x_write_reg(FT5X0X_REG_PERIODACTIVE, 7);
#if !USE_THREADED_IRQ
	INIT_WORK(&ft5x0x_ts->pen_event_work, ft5x0x_ts_pen_irq_work);

	ft5x0x_ts->ts_workqueue = create_singlethread_workqueue(dev_name(&client->dev));
	if (!ft5x0x_ts->ts_workqueue) {
		err = -ESRCH;
		goto exit_create_singlethread;
	}
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
	INIT_WORK(&ft5x0x_ts->resume_work, ft5x0x_ts_resume_work);
	ft5x0x_ts->ts_resume_workqueue = create_singlethread_workqueue("ft5x0x_ts_resume_work");
	if (!ft5x0x_ts->ts_resume_workqueue) {
		err = -ESRCH;
		goto exit_create_singlethread;
	}
#endif
	input_dev = input_allocate_device();
	if (!input_dev) {
		err = -ENOMEM;
		dev_err(&client->dev, "[FST] failed to allocate input device\n");
		goto exit_input_dev_alloc_failed;
	}
#ifdef TPD_PROXIMITY
		
		ft5x0x_ts->ps_input = input_allocate_device();
		if (!ft5x0x_ts->ps_input) {
			dev_err(&client->dev, "%s: failed to allocate input device\n", __func__);
			goto exit_input_ps_dev_alloc_failed;
		}
#endif
#ifdef TOUCH_VIRTUAL_KEYS
	ft5x0x_ts_virtual_keys_init();
#endif
	ft5x0x_ts->input_dev = input_dev;

	__set_bit(ABS_MT_TOUCH_MAJOR, input_dev->absbit);
	__set_bit(ABS_MT_POSITION_X, input_dev->absbit);
	__set_bit(ABS_MT_POSITION_Y, input_dev->absbit);
	__set_bit(ABS_MT_WIDTH_MAJOR, input_dev->absbit);
	//__set_bit(KEY_MENU,  input_dev->keybit);
	//__set_bit(KEY_BACK,  input_dev->keybit);
	__set_bit(KEY_HOMEPAGE,  input_dev->keybit);
	__set_bit(BTN_TOUCH, input_dev->keybit);
    set_bit(INPUT_PROP_DIRECT, input_dev->propbit);
#if MULTI_PROTOCOL_TYPE_B
	input_mt_init_slots(input_dev, TS_MAX_FINGER);
#endif

	input_set_abs_params(input_dev,ABS_MT_POSITION_X, 0, 320, 0, 0);
	input_set_abs_params(input_dev,ABS_MT_POSITION_Y, 0, 480, 0, 0);
	input_set_abs_params(input_dev,ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
	input_set_abs_params(input_dev,ABS_MT_WIDTH_MAJOR, 0, 255, 0, 0);

  set_bit(EV_SYN, input_dev->evbit);
	set_bit(EV_ABS, input_dev->evbit);
	set_bit(EV_KEY, input_dev->evbit);

	input_dev->name = FOCALTECH_TS_NAME;
	err = input_register_device(input_dev);
	if (err) {
		dev_err(&client->dev,
		"[FST] ft5x0x_ts_probe: failed to register input device: %s\n",
		dev_name(&client->dev));
		goto exit_input_register_device_failed;
	}
#ifdef TPD_PROXIMITY
	
		ft5x0x_ts->ps_input->name = "p_sensor";
		
		set_bit(EV_ABS, ft5x0x_ts->ps_input->evbit);
		input_set_abs_params(ft5x0x_ts->ps_input, ABS_DISTANCE, 0,1, 0, 0);
		err = input_register_device(ft5x0x_ts->ps_input);
		if (err){
			dev_err(&client->dev, "Register input device fail\n");
			goto exit_ps_input_register_device;
		}
#endif
#if USE_THREADED_IRQ
	err = request_threaded_irq(client->irq, NULL, ft5x0x_ts_interrupt, 
		IRQF_TRIGGER_FALLING | IRQF_ONESHOT, client->name, ft5x0x_ts);
#else
	err = request_irq(client->irq, ft5x0x_ts_interrupt,
		IRQF_TRIGGER_FALLING | IRQF_ONESHOT, client->name, ft5x0x_ts);
#endif
	if (err < 0) {
		dev_err(&client->dev, "[FST] ft5x0x_probe: request irq failed %d\n",err);
		goto exit_irq_request_failed;
	}
#ifdef TPD_PROXIMITY
	
	wake_lock_init(&ps_wake_lock, WAKE_LOCK_SUSPEND, "ps_wakelock");
	ft6306_data = ft5x0x_ts;
	
	err= sysfs_create_group(&ft5x0x_ts->ps_input->dev.kobj, &msg2133_proximity_attribute_group);
	if(err < 0)
	{
		printk("Failed to create devices file:msg2133_proximity_attribute_group\n");
	}
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
	ft5x0x_ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	ft5x0x_ts->early_suspend.suspend = ft5x0x_ts_suspend;
	ft5x0x_ts->early_suspend.resume	= ft5x0x_ts_resume;
	register_early_suspend(&ft5x0x_ts->early_suspend);
#endif

       #if 1//find chip info  dennis add
      		i2c_smbus_read_i2c_block_data(client,FT_REG_CHIP_ID,1,&chip_id);
		printk("%s chip_id = %x\n", __func__, chip_id);
		for(i=0;i<sizeof(fts_updateinfo)/sizeof(struct Upgrade_Info);i++)
                {
                    if(chip_id==fts_updateinfo[i].CHIP_ID)
                    {
                        memcpy(&fts_updateinfo_curr, &fts_updateinfo[i], sizeof(struct Upgrade_Info));
                        break;
                    }
                }
                if(i>=sizeof(fts_updateinfo)/sizeof(struct Upgrade_Info))
                {
                        memcpy(&fts_updateinfo_curr, &fts_updateinfo[0], sizeof(struct Upgrade_Info));
                }
       #endif

#ifdef SYSFS_DEBUG	
fts_create_sysfs(client);
#endif

#ifdef FTS_CTL_IIC	
if (ft_rw_iic_drv_init(client) < 0)	
{
	dev_err(&client->dev, "%s:[FTS] create fts control iic driver failed\n",	__func__);
}
#endif

#ifdef SPRD_AUTO_UPGRADE
	printk("********************Enter CTP Auto Upgrade********************\n");
	fts_ctpm_auto_upgrade(client);
#endif   

#ifdef APK_DEBUG
	ft5x0x_create_apk_debug_channel(client);
#endif
	return 0;

exit_irq_request_failed:
#ifndef TPD_PROXIMITY
	input_unregister_device(input_dev);
#else
    input_unregister_device(ft5x0x_ts->ps_input);
exit_ps_input_register_device:
    input_unregister_device(input_dev);
	
#endif
exit_input_register_device_failed:
#ifndef TPD_PROXIMITY
	input_free_device(input_dev);
#else
    input_free_device(ft5x0x_ts->ps_input);
    ft5x0x_ts->ps_input=NULL;

exit_input_ps_dev_alloc_failed:
     input_free_device(input_dev);
	 input_dev=NULL;	
#endif
exit_input_dev_alloc_failed:
exit_create_singlethread:
exit_chip_check_failed:
	kfree(ft5x0x_ts);
exit_alloc_data_failed:
exit_check_functionality_failed:
	ft5x0x_ts = NULL;
	i2c_set_clientdata(client, ft5x0x_ts);
	return err;
}

static int __devexit ft5x0x_ts_remove(struct i2c_client *client)
{
	struct ft5x0x_ts_data *ft5x0x_ts = i2c_get_clientdata(client);

	pr_info("==ft5x0x_ts_remove=\n");
	
	#ifdef SYSFS_DEBUG
	fts_release_sysfs(client);
	#endif
	#ifdef FTS_CTL_IIC	
	ft_rw_iic_drv_exit();
	#endif
	#ifdef APK_DEBUG
	ft5x0x_release_apk_debug_channel();
	#endif
	
#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&ft5x0x_ts->early_suspend);
#endif
#if defined(TPD_PROXIMITY)
		input_unregister_device(ft5x0x_ts->ps_input);
		input_free_device(ft5x0x_ts->ps_input);
		ft6306_data = NULL;
		this_client = NULL;
#endif

	free_irq(client->irq, ft5x0x_ts);
	input_unregister_device(ft5x0x_ts->input_dev);
	input_free_device(ft5x0x_ts->input_dev);
#if !USE_THREADED_IRQ
	cancel_work_sync(&ft5x0x_ts->pen_event_work);
	destroy_workqueue(ft5x0x_ts->ts_workqueue);
#endif
#ifdef CONFIG_HAS_EARLYSUSPEND
	cancel_work_sync(&ft5x0x_ts->resume_work);
	destroy_workqueue(ft5x0x_ts->ts_resume_workqueue);
#endif
	kfree(ft5x0x_ts);
	ft5x0x_ts = NULL;
	i2c_set_clientdata(client, ft5x0x_ts);

	return 0;
}

static const struct i2c_device_id ft5x0x_ts_id[] = {
	{ FOCALTECH_TS_NAME, 0 },{ }
};

static int ft5x0x_suspend(struct i2c_client *client, pm_message_t mesg)
{
	int ret = -1;
	int count = 5;

	//PRINT_INFO("ft5x0x_suspend\n");

#ifdef TPD_PROXIMITY
	if (tpd_proximity_flag == 1)
	{
		tpd_proximity_suspend = 0;	
		return 0;
	}
	else
	{
		tpd_proximity_suspend = 1;
	}
#endif

	ret = ft5x0x_write_reg(FT5X0X_REG_PMODE, PMODE_HIBERNATE);
	while(ret != 0 && count != 0) {
			//PRINT_ERR("trying to enter hibernate again. ret = %d\n", ret);
			msleep(10);
			ret = ft5x0x_write_reg(FT5X0X_REG_PMODE, PMODE_HIBERNATE);
			count--;
	}
	return 0;
}
static int ft5x0x_resume(struct i2c_client *client)
{
	//PRINT_INFO("ft5x0x_resume\n");

#ifdef TPD_PROXIMITY	
	if (tpd_proximity_suspend == 0)
	{
		return 0;
	}
	else
	{
		tpd_proximity_suspend = 0;
	}
#endif	

	return 0;
}

MODULE_DEVICE_TABLE(i2c, ft5x0x_ts_id);

static struct i2c_driver ft5x0x_ts_driver = {
	.probe		= ft5x0x_ts_probe,
	.remove		= __devexit_p(ft5x0x_ts_remove),
	.id_table	= ft5x0x_ts_id,
	.driver	= {
		.name	= FOCALTECH_TS_NAME,
		.owner	= THIS_MODULE,
	},
//	.suspend = ft5x0x_suspend,
//	.resume = ft5x0x_resume,
};

static int __init ft5x0x_ts_init(void)
{
	return i2c_add_driver(&ft5x0x_ts_driver);
}

static void __exit ft5x0x_ts_exit(void)
{
	i2c_del_driver(&ft5x0x_ts_driver);
}

module_init(ft5x0x_ts_init);
module_exit(ft5x0x_ts_exit);

MODULE_AUTHOR("<wenfs@Focaltech-systems.com>");
MODULE_DESCRIPTION("FocalTech ft5x0x TouchScreen driver");
MODULE_LICENSE("GPL");
