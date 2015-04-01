/*******************************************************************************
 * Source file : realtek_fm_ctl.c
 * Description : REALTEK_FM Receiver driver for linux.
 * Date        : 05/11/2011
 * 
 * Copyright (C) 2011 Spreadtum Inc.
 *
 ********************************************************************************
 * Revison
 2011-05-11  aijun.sun   initial version 
 *******************************************************************************/
#include <linux/miscdevice.h>
#include <linux/i2c.h>
#include <linux/sysfs.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/ioctl.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/suspend.h>
//#include <stdio.h>
//#include <stdlib.h>

#include <linux/kobject.h>
#include <linux/videodev2.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <media/v4l2-ioctl.h>

#include <mach/gpio.h>
#include <mach/board.h>

#include "realtek_fm_ctrl.h"
#include "rtl8723b_fm.h"

#define REALTEK_FM_WAKEUP_CHECK_TIME 100   //ms 
#define REALTEK_FM_WAKEUP_TIMEOUT 800   

#define REALTEK_FM_SEEK_CHECK_TIME 10 //ms

#define REALTEK_FM_TUNE_DELAY 50    //ms 

#define I2C_RETRY_DELAY 5
#define I2C_RETRIES 3

#define REALTEK_FM_DEV_NAME	"REALTEK_FM"
#define REALTEK_FM_I2C_NAME    REALTEK_FM_DEV_NAME
#define REALTEK_FM_I2C_ADDR    0x12


#define FM_START_FREQUENCY 87000000
#define FM_END_FREQUENCY 108000000


//randy for volume open fm
static u8 volume_open_fm;

#ifndef GPIO_FM_EN
#define GPIO_FM_EN -1
#endif 

#ifdef FM_VOLUME_ON



/*gordon for full search timeout begin*/
static void fm_volume_on_worker(struct work_struct *private_);
static DECLARE_DELAYED_WORK(fm_volume_on, fm_volume_on_worker);
/*gordon for full search timeout end*/
#endif

static void fm_show_snr_worker(struct work_struct *private_);
static DECLARE_DELAYED_WORK(fm_show_snr, fm_show_snr_worker);

extern int sprd_3rdparty_gpio_fm_enable;

struct mutex mutex;


struct realtek_fm_drv_data {
	struct i2c_client *client;
	int               opened_before_suspend;
	int				bg_play_enable; // enable/disable background play.

	atomic_t          fm_opened;
	atomic_t          fm_searching;
	int               current_freq;
	int			last_seek_freq;
	int               current_volume;
 	u8                muteOn;

	FM_MODULE *pFm;
	FM_MODULE FmModuleMemory;

	// Base interface module
	BASE_INTERFACE_MODULE *pBaseInterface;
	BASE_INTERFACE_MODULE BaseInterfaceModuleMemory;	
};

struct realtek_fm_drv_data *realtek_fm_dev_data = NULL;


static int realtek_fm_set_volume(struct realtek_fm_drv_data *cxt, u8 volume);



static int 
custom_i2c_read(
	BASE_INTERFACE_MODULE *pBaseInterface,
	unsigned char DeviceAddr,
	unsigned char *pReadingBytes,
	unsigned long ByteNum
	)
{
	struct realtek_fm_drv_data *cxt;
	int err;
	int tries;
	int kk;

	struct i2c_msg msgs[] = {
		{
			.addr = DeviceAddr,
			.flags =  I2C_M_RD,
			.len = ByteNum,
			.buf = pReadingBytes,
		},
	};


	err = 0;
	tries = 0;

	pBaseInterface->GetUserDefinedDataPointer(pBaseInterface, (void **)&cxt);	

	do {
		err = i2c_transfer(cxt->client->adapter, msgs, 1);
		if (err != 1)
			{
				msleep_interruptible(I2C_RETRY_DELAY);
				dev_err(&cxt->client->dev, "custom_i2c_read : %d Read transfer error\n", tries);
				dbg_info( "i2c_read err msg: DeviceAddr = %x\n",DeviceAddr);
				dbg_info( "i2c_read err msg: ByteNum = %ld\n",ByteNum);
				kk = 0;
				for(kk; kk < ByteNum; kk++)
					dbg_info( "i2c_read err msg: pReadingBytes = %x\n",*((unsigned char*)pReadingBytes + kk));
			}
	} while ((err != 1) && (++tries < I2C_RETRIES));

	if (err != 1) {
		dev_err(&cxt->client->dev, "custom_i2c_read : Read transfer error\n");
		err = -EIO;
	} else {
		err = 0;
	}

	return err;
}



static int
custom_i2c_write(
	BASE_INTERFACE_MODULE *pBaseInterface,
	unsigned char DeviceAddr,
	const unsigned char *pWritingBytes,
	unsigned long ByteNum
	)
{
	struct realtek_fm_drv_data *cxt;
	int err;
	int tries;
	int kk;
	
	struct i2c_msg msgs[] = { 
		{ 
			.addr = DeviceAddr,
	 		.flags = 0,
			.len = ByteNum, 
			.buf = (unsigned char*)pWritingBytes, 
		},
	};
		

	err = 0;
	tries = 0;

	pBaseInterface->GetUserDefinedDataPointer(pBaseInterface, (void **)&cxt);

	do {
		err = i2c_transfer(cxt->client->adapter, msgs, 1);
		if (err != 1)
		{
			msleep_interruptible(I2C_RETRY_DELAY);
			dev_err(&cxt->client->dev, "custom_i2c_write : %d write transfer error\n", tries);
			dbg_info( "i2c_write err msg: DeviceAddr = %x\n",DeviceAddr);
			dbg_info( "i2c_write err msg: ByteNum = %ld\n",ByteNum);
			kk = 0;
			for(kk; kk < ByteNum; kk++)
				dbg_info( "i2c_write err msg: pWritingBytes = %x\n",*((unsigned char*)pWritingBytes + kk));
		}
	} while ((err != 1) && (++tries < I2C_RETRIES));

	if (err != 1) {
		dev_err(&cxt->client->dev, "custom_i2c_write : write transfer error\n");
		err = -EIO;
	} else {
		err = 0;
	}

	return err;
}

/*

static ssize_t fm_set_volume_store(struct kobject *kobj, struct kobj_attribute *attr, char *buff, size_t len)				// 
{
	u8 volume = (u8)(*buff);

	if (realtek_fm_set_volume(realtek_fm_dev_data, volume) <0)
			return FM_FUNCTION_ERROR;
	return len;
}


static ssize_t fm_get_volume_show(struct kobject *kobj, struct kobj_attribute *attr, char *buff)		// 
{
		u8 volume =0;
		volume = realtek_fm_get_volume(realtek_fm_dev_data) ;

		if(volume <0 )
			return FM_FUNCTION_ERROR;
					
		*buff = volume;

		return 1;
}
*/
/*
static struct kobject *fm_volume_kobj = NULL;                                                                                           

static struct kobj_attribute realtek_fm_volume_attr =
        __ATTR(realtek_fm_volume, 0644, fm_get_volume_show, fm_set_volume_store);
*/

void
custom_wait_ms(
	BASE_INTERFACE_MODULE *pBaseInterface,
	unsigned long nMinDelayTime
	)
{
	unsigned long usec;
	
	do {
		usec = (nMinDelayTime > 8000) ? 8000 : nMinDelayTime;
		msleep(usec);
		nMinDelayTime -= usec;
		} while (nMinDelayTime > 0);

	return;

}



static void realtek_fm_gpio_en(struct realtek_fm_drv_data *cxt,bool turn_on)
{

	if(turn_on){
		printk("FM in chip on\n");
		if(GPIO_FM_EN){
       		gpio_direction_output(GPIO_FM_EN, 1);
			mdelay(100);
		}
    }else{
		printk("FM in chip off\n");
		if(GPIO_FM_EN){
       		gpio_direction_output(GPIO_FM_EN, 0);
			mdelay(150);
		}
	}
}



//NOTES:
//This function is private call by 'probe function', so not add re-entry
//protect.
static int
realtek_fm_power_on(
	struct realtek_fm_drv_data *cxt
	)
{
	unsigned long data;
	unsigned int times;
	
	dbg_info( "+FM power on\n");

	/* turn on fm enable*/
	realtek_fm_gpio_en(cxt,true);

	//I2C_write_bitsB(addr_fm, hex2dec('010d'), 0, 0, 0); % en_sleep LDO12H, 0: disable, 1: enable
	if(cxt->pFm->SetRegMaskBits(cxt->pFm, 0x010d, 0, 0, 0) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

	msleep(1);
	//% I2C_write_bitsB(addr_fm, hex2dec('0101'), 1, 1, 1); %% rfafe_en, 0: turn off LDOD, 1: turn on LDOD
	if(cxt->pFm->SetRegMaskBits(cxt->pFm, 0x0101, 1, 1, 1) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

	msleep(1);
	//% I2C_write_bitsB(addr_fm, hex2dec('0101'), 2, 2, 1); %% ldoa_en, 0: turn off LDOA, 1: turn on LDOA
	if(cxt->pFm->SetRegMaskBits(cxt->pFm, 0x0101, 2, 2, 1) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

	msleep(1);
	//% I2C_write_bitsB(addr_fm, hex2dec('0101'), 3, 3, 0); %% RFC_ISO_ON, 0: disable, 1: enable
	if(cxt->pFm->SetRegMaskBits(cxt->pFm, 0x0101, 3, 3, 0) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

	msleep(1);
	//% I2C_write_bitsB(addr_fm, hex2dec('0101'), 4, 4, 0); %% ISO_ON, 0: disable, 1: enable
	if(cxt->pFm->SetRegMaskBits(cxt->pFm, 0x0101, 4, 4, 0) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

	msleep(1);
	//% I2C_write_bitsB(addr_fm, hex2dec('0101'), 5, 5, 1); %% RFC software rstn 
	if(cxt->pFm->SetRegMaskBits(cxt->pFm, 0x0101, 5, 5, 1) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

	msleep(10);	
	
	//rfc_write(hex2dec('0'), hex2dec('a000'));
	if(cxt->pFm->SetRfcReg2Bytes(cxt->pFm, 0, 0xa000) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

	msleep(10);	

	//rfc_write(hex2dec('27'), hex2dec('9800'));
	if(cxt->pFm->SetRfcReg2Bytes(cxt->pFm, 0x27, 0x9800) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

	if(cxt->pFm->SetRfcReg2Bytes(cxt->pFm, 0x04, 0x1354) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

	msleep(10);	

	times = 0;
	do{
		times++;
		msleep(5);
		if(cxt->pFm->GetRegMaskBits(cxt->pFm, 0x0120, 7, 0, &data) != FUNCTION_SUCCESS)
			goto error_status_set_registers;
	}while( (data!=0xff) && (times<200) );

	if(data!=0xff)
	{
		dbg_info( "FM power on : timeout err BB Reg.0x0120 = 0x%x\n", data);
		goto error_status_set_registers;
	}
	
	//I2C_write_bitsB(addr_fm, hex2dec('0101'), 0, 0, 1); % i2c_clk_sel, 0: ring 12M, 1: gated_40M
	if(cxt->pFm->SetRegMaskBits(cxt->pFm, 0x0101, 0, 0, 1) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

	msleep(10);

	//I2C_write_bitsB(addr_fm, hex2dec('010d'), 3, 3, 0); % ck_12m_en, 0: disable, 1: enable
	if(cxt->pFm->SetRegMaskBits(cxt->pFm, 0x010d, 3, 3, 0) != FUNCTION_SUCCESS)
		goto error_status_set_registers;
	
	msleep(10);	

	dbg_info( "-FM power on\n");


	return FM_FUNCTION_SUCCESS;

error_status_set_registers:

	dbg_info( "-FM power on : err\n");


	return FM_FUNCTION_ERROR;

}



static int
realtek_fm_close(
	struct realtek_fm_drv_data *cxt
	)
{
	if(volume_open_fm)
	{
		volume_open_fm--;	//randy test
		dbg_info( "dont close now\n");
		return FM_FUNCTION_SUCCESS;
	}
	
	atomic_cmpxchg(&cxt->fm_opened, 1, 0);

	dbg_info( "+FM close\n");
#if 0
	//%% turn on 12M Ring osc.
	//I2C_write_bitsB(addr_fm, hex2dec('010d'), 3, 3, 1); % ck_12m_en, 0: disable, 1: enable
	if(cxt->pFm->SetRegMaskBits(cxt->pFm, 0x010d, 3, 3, 1) != FUNCTION_SUCCESS)
		goto error_status_set_registers;	

	//%% set CLK to 12MHz.
	//I2C_write_bitsB(addr_fm, hex2dec('0101'), 0, 0, 0); % i2c_clk_sel, 0: ring 12M, 1: gated_40M
	if(cxt->pFm->SetRegMaskBits(cxt->pFm, 0x0101, 0, 0, 0) != FUNCTION_SUCCESS)
		goto error_status_set_registers;
	
	//%%%% set FM into sleep mode %%%%
	//I2C_write_bitsB(addr_fm, hex2dec('0101'), 5, 5, 0); %% RFC software rstn 
	if(cxt->pFm->SetRegMaskBits(cxt->pFm, 0x0101, 5, 5, 0) != FUNCTION_SUCCESS)
		goto error_status_set_registers;	

	
	msleep(100);

	//I2C_write_bitsB(addr_fm, hex2dec('0101'), 3, 3, 1); %% RFC_ISO_ON, 0: disable, 1: enable
	if(cxt->pFm->SetRegMaskBits(cxt->pFm, 0x0101, 3, 3, 1) != FUNCTION_SUCCESS)
		goto error_status_set_registers;	

	msleep(100);

	//I2C_write_bitsB(addr_fm, hex2dec('0101'), 2, 2, 0); %% ldoa_en, 0: turn off LDOA, 1: turn on LDOA
	if(cxt->pFm->SetRegMaskBits(cxt->pFm, 0x0101, 2, 2, 0) != FUNCTION_SUCCESS)
		goto error_status_set_registers;	

	msleep(100);
	
	//I2C_write_bitsB(addr_fm, hex2dec('0101'), 1, 1, 0); %% rfafe_en, 0: turn off LDOD, 1: turn on LDOD
	if(cxt->pFm->SetRegMaskBits(cxt->pFm, 0x0101, 1, 1, 0) != FUNCTION_SUCCESS)
		goto error_status_set_registers;	

	msleep(100);

	//%% enable LDO12H sleep mode.
	//I2C_write_bitsB(addr_fm, hex2dec('010d'), 0, 0, 1); % en_sleep LDO12H, 0: disable, 1: enable
	if(cxt->pFm->SetRegMaskBits(cxt->pFm, 0x010d, 0, 0, 1) != FUNCTION_SUCCESS)
		goto error_status_set_registers;	

	//I2C_write_bitsB(addr_fm, hex2dec('0001'), 0, 0, 0); %% disable FM
	if(cxt->pFm->SetRegMaskBits(cxt->pFm, 0x0001, 0, 0, 0) != FUNCTION_SUCCESS)
		goto error_status_set_registers;	

	//%% hold BB software rst.
	//I2C_write_bitsB(addr_fm, hex2dec('0002'), 0, 0, 1);
	if(cxt->pFm->SetRegMaskBits(cxt->pFm, 0x0002, 0, 0, 1) != FUNCTION_SUCCESS)
		goto error_status_set_registers;	
	
#endif

#ifdef FM_SHOW_SNR_DEBUG
	cancel_delayed_work(&fm_show_snr);
#endif

#if 0
	unsigned long Value;

	for( i = 1; i <= 0x0120; i++)
	{
		cxt->pFm->GetRegMaskBits(cxt->pFm, i, 15, 0, &Value);
		dbg_info( "BB	0x%x	0x%x\n", i , Value);
	}

	cxt->pFm->SetRegMaskBits(cxt->pFm, 0x006c, 0, 0, 0);
	for( i = 0; i <= 0xff; i++)
	{
		cxt->pFm->GetRfcReg2Bytes(cxt->pFm, i, &Value);
		dbg_info( "RFC	0x%x	0x%x\n", i , Value);
	}
#endif	
	/* turn off fm enable */
	realtek_fm_gpio_en(cxt,false);

	//realtek_fm_set_volume(cxt, 0);	//randy close ,set volume off
	
	dbg_info( "-FM close\n");

	return FM_FUNCTION_SUCCESS;


}



//
// Notes: Before this call, device must be power on.
//
static int
realtek_fm_open(
	struct realtek_fm_drv_data *cxt
	)
{
	int ret;
	int chipid;
	unsigned long data;

	ret = -EINVAL;
	
	if (atomic_read(&cxt->fm_opened)) {
		dev_err(&cxt->client->dev,
			"FM open: already opened, ignore this operation\n");
		//return FM_FUNCTION_ERROR;
		volume_open_fm++;			//randy test
		//atomic_cmpxchg(&cxt->fm_opened, 1, 0);
		//realtek_fm_gpio_en(cxt,false);
		
		return FM_FUNCTION_SUCCESS;
	}
	
	ret = realtek_fm_power_on(cxt);
	if (ret < 0)
		return FM_FUNCTION_ERROR;	

	rtl8723b_fm_Get_Chipid(cxt->pFm, &chipid);
	
	dbg_info( "+FM open 2014.03.14 for ffos 2014-6-30 :: Chip ID = 0x%x\n", chipid);

	if(rtl8723b_fm_Initialize(cxt->pFm))
	{
		msleep(5);
		if(rtl8723b_fm_Initialize(cxt->pFm))  	goto error;
	}
	//+ register dump check
	cxt->pFm->GetRfcReg2Bytes(cxt->pFm, 0x00, &data) ;            
	if(data != 0xa000)
	{
		msleep(5);
		cxt->pFm->GetRfcReg2Bytes(cxt->pFm, 0x00, &data) ;
		if((data != 0xa000))
		{
			dbg_info( " FM open :::err Rfc Reg.0x00=0x%x\n", data);
			goto error;
		}
	}
	
	cxt->pFm->GetRfcReg2Bytes(cxt->pFm, 0x21, &data);
	if(data != 0xcd53)
	{
		msleep(5);
		cxt->pFm->GetRfcReg2Bytes(cxt->pFm, 0x21, &data);
		if(data != 0xcd53)
		{
			dbg_info( " FM open :::err Rfc Reg.0x21=0x%x\n", data);
			goto error;
		}
	}


	cxt->pFm->GetRegMaskBits(cxt->pFm, 0x0001, 7, 0, &data);
	if(data != 0x1)
	{
		msleep(5);
		cxt->pFm->GetRegMaskBits(cxt->pFm, 0x0001, 7, 0, &data);
		if(data != 0x1)
		{
			dbg_info( " FM open :::err BB Reg.0x0001=0x%x\n", data);
			goto error;
		}
	}

	cxt->pFm->GetRegMaskBits(cxt->pFm, 0x0002, 7, 0, &data);
	if(data != 0x0)
	{
		msleep(5);
		cxt->pFm->GetRegMaskBits(cxt->pFm, 0x0002, 7, 0, &data);
		if(data != 0x0)
		{
			dbg_info( " FM open :::err BB Reg.0x0002=0x%x\n", data);
			goto error;
		}
	}
	//- register dump check

	if(rtl8723b_fm_SetBand(cxt->pFm, FM_BAND_87_108MHz) )	goto error;
	
	if(rtl8723b_fm_Set_Channel_Space( cxt->pFm,FM_CHANNEL_SPACE_100kHz ))	goto error;

	if(rtl8723b_fm_SetTune_Hz(cxt->pFm, cxt->current_freq*100*1000)!= FM_FUNCTION_SUCCESS)	goto error;

	cxt->pFm->pBaseInterface->WaitMs(cxt->pFm->pBaseInterface,  100);
	
	atomic_cmpxchg(&cxt->fm_opened, 0, 1);

	cxt->current_volume = 14;
	
	//if(realtek_fm_set_volume(cxt, 0)) goto error;

	dbg_info( "-FM open: FM is opened\n");
	
	if(realtek_fm_set_volume(cxt, 12)) goto error;

        //rfc_write(hex2dec('23'), hex2dec('5bf6')); %% power on DAC
	if(cxt->pFm->SetRfcReg2Bytes(cxt->pFm, 0x23, 0x5bf6))                goto error;

#ifdef FM_SHOW_SNR_DEBUG
	schedule_delayed_work(&fm_show_snr, HZ/4);
#endif

	return FM_FUNCTION_SUCCESS;

error:

	dev_err(&cxt->client->dev, "-FM open : err\n");
	
	return FM_FUNCTION_ERROR;
	
}



//
//Notes:Set FM tune, the frequency 100KHz unit.
//
static int
realtek_fm_set_tune(
	struct realtek_fm_drv_data *cxt,
	u16 frequency
	)
{
	if (!atomic_read(&cxt->fm_opened)) {
		
		dev_err(&cxt->client->dev, "FM set_tune: FM not open\n");
		
		return FM_FUNCTION_ERROR;
	}

	dbg_info( "+FM set_tune %d\n", frequency);
	
	if(rtl8723b_fm_SetTune_Hz(cxt->pFm, frequency*100*1000)!= FM_FUNCTION_SUCCESS)	goto error;

	cxt->current_freq = frequency;

	dbg_info( "-FM set_tune\n");

	return FM_FUNCTION_SUCCESS;
	
error:

	dev_err(&cxt->client->dev, "-FM set tune: err...\n");
	
	return FM_FUNCTION_ERROR;
}



//
// NOTES: Get tune frequency, with 100KHz unit.
 //
static int
realtek_fm_get_frequency(
	struct realtek_fm_drv_data *cxt
	)
{
	u16 frequency;
	int FrequencyHz;
	
	frequency = 0;
	
	if (!atomic_read(&cxt->fm_opened)) {

		dev_err(&cxt->client->dev, "FM get_frequency: FM not open\n");

		return FM_FUNCTION_ERROR;
	}

	dbg_info( "+FM get_frequency\n");
	
	if(rtl8723b_fm_GetFrequencyHz(cxt->pFm, &FrequencyHz)!= FM_FUNCTION_SUCCESS)	goto error;

	frequency = FrequencyHz/100000;

	dbg_info( "-FM get_frequency = %d\n", frequency);

	return frequency;

error:

	dev_err(&cxt->client->dev, "-FM get_frequency : err\n");

	return FM_FUNCTION_ERROR;
	
}



//NOTES:
//Stop fm search and clear search status 
 //
static int
realtek_fm_stop_search(
	struct realtek_fm_drv_data *cxt
	)
{
	dbg_info( "+FM stop_search\n");


	if (atomic_read(&cxt->fm_searching)) {

		atomic_cmpxchg(&cxt->fm_searching, 1, 0);

		if(rtl8723b_fm_Seek( cxt->pFm, FM_SEEK_DISABLE))	goto error;

	}

	dbg_info( "-FM stop_search\n");

	return FM_FUNCTION_SUCCESS;

error:

	dev_err(&cxt->client->dev, "-FM stop_search : err\n");

	return FM_FUNCTION_ERROR;
}


//gordon

static void fm_volume_on_worker(struct work_struct *private_)
{
#if 1 
	struct realtek_fm_drv_data *cxt = realtek_fm_dev_data;

	if (!atomic_read(&cxt->fm_opened)) {

		dev_err(&cxt->client->dev, "fm_volume_on_worker: FM not open\n");

		return;
	}

	dbg_info( "+FM volume on\n");

	if(cxt->current_volume != 0)
	{
		if(rtl8723b_fm_SetMute(cxt->pFm, 0))            goto error;
	}

	dbg_info( "-FM volume on success\n");

	return;

error:

	dbg_info( "-FM volume on : err\n");
#endif
}


static void fm_show_snr_worker(struct work_struct *private_)
{
	int FrequencyHz1;
	long SnrDbNum1;
	long SnrDbDen1;
	unsigned long RFGain, RXBBGain;
	int rssi;
	
	struct realtek_fm_drv_data *cxt = realtek_fm_dev_data;

	if (!atomic_read(&cxt->fm_opened)) {

		dev_err(&cxt->client->dev, "fm_show_snr_worker: FM not open\n");

		return;
	}

	if(rtl8723b_fm_GetFrequencyHz(cxt->pFm, &FrequencyHz1))		goto error;
	if(rtl8723b_fm_GetSNR(cxt->pFm, &SnrDbNum1, &SnrDbDen1))		goto error;
	if(rtl8723b_fm_GetRSSI(cxt->pFm, &rssi))		goto error;

	if(cxt->pFm->GetReadOnlyRegMaskBits(cxt->pFm, 21, FM_ADDR_DEBUG_0, 2, 0, &RFGain))		goto error;
	if(cxt->pFm->GetReadOnlyRegMaskBits(cxt->pFm, 21, FM_ADDR_DEBUG_0, 7, 3, &RXBBGain))		goto error;

	dbg_info( " Show Freq:%d, SNR:%ld, RF Gain:%ld, RXBB Gain:%ld, rssi=%d\n", FrequencyHz1/100000, SnrDbNum1*10/SnrDbDen1, RFGain, RXBBGain, rssi);

	if(FrequencyHz1 == 60*1000*1000)
	{
		dbg_info( " ERR : Freq = 60MHz, #### FM RE-OPEN ####\n");
		dbg_info( " +===============================\n");
		if(realtek_fm_close(cxt))		goto error;
		if(realtek_fm_open(cxt))		goto error; 
		dbg_info( " -===============================\n");
	}

	cancel_delayed_work(&fm_show_snr);
	schedule_delayed_work(&fm_show_snr, HZ/2);

error:
	return;
}


// NOTES: 
// Search frequency from current frequency, if a channel is found, the 
// frequency will be read out.
// This function is timeout call. If no channel is found when time out, a error 
// code will be given. The caller can retry search(skip "do seek")   to get 
// channel. 
//
static int
realtek_fm_full_search(
	struct realtek_fm_drv_data *cxt, 
	u16 frequency, 
	u8 seek_dir,
	u32 time_out,
	u16 *freq_found
	)
{
	int FrequencyHz1;
	int FrequencyHz2;
	int FrequencyTest;

	unsigned long SeekTuneComplete;

	int Status;
	ulong   jiffies_comp;
	u8      is_timeout;
   	
	long SnrDbNum1, SnrDbNum2 ;
	long SnrDbDen1,SnrDbDen2;
	unsigned long RFGain, RXBBGain;

	if (!atomic_read(&cxt->fm_opened)) {

		dev_err(&cxt->client->dev, "FM Full search: FM not open\n");

		return FM_FUNCTION_ERROR;
	}

	if (atomic_read(&cxt->fm_searching)){

		dev_err(&cxt->client->dev, "FM Full search :  busy searching!!!\n");
		
		return -EBUSY;	
	}
	
	dbg_info( "+FM full_search\n");	

	atomic_cmpxchg(&cxt->fm_searching, 0, 1);

	if(cxt->current_volume != 0)
	{
		if(rtl8723b_fm_SetMute(cxt->pFm, 1))		goto error;
	}

	msleep(30);	
	
	if(cxt->pFm->GetReadOnlyRegMaskBits(cxt->pFm, 21, FM_ADDR_DEBUG_0, 2, 0, &RFGain))	goto error;

	dbg_info( " ###FM full_search : RFGain = %ld###\n", RFGain);	
	
	if(cxt->pFm->SetRegMaskBits(cxt->pFm, 0x006c, 0, 0, 0))		goto error;
	
	if(cxt->pFm->SetRfcReg2Bytes(cxt->pFm, 0x0f, 0x4000))	goto error;

	switch(RFGain)
	{
		case 7:
		if(cxt->pFm->SetRfcReg2Bytes(cxt->pFm, 0x06, 0xfe00))	goto error;
		break;
		
		case 6:
		if(cxt->pFm->SetRfcReg2Bytes(cxt->pFm, 0x06, 0xe030))	goto error;
		break;

		case 5:
		if(cxt->pFm->SetRfcReg2Bytes(cxt->pFm, 0x06, 0x8018))	goto error;
		break;

		case 4:
		if(cxt->pFm->SetRfcReg2Bytes(cxt->pFm, 0x06, 0x43f8))	goto error;
		break;
		
		case 0:
		if(cxt->pFm->SetRfcReg2Bytes(cxt->pFm, 0x00, 0xa000))	goto error;
		if(cxt->pFm->SetRfcReg2Bytes(cxt->pFm, 0x0f, 0x4000))	goto error;
		if(cxt->pFm->SetRfcReg2Bytes(cxt->pFm, 0x06, 0xfe00))	goto error;
		break;
		
		default:
		if(cxt->pFm->SetRfcReg2Bytes(cxt->pFm, 0x06, 0x13f8))	goto error;
		break;
	}

	if(cxt->pFm->SetRegMaskBits(cxt->pFm, 0x006c, 0, 0, 1))		goto error;
  
	if (frequency > 0)
	{
		if(rtl8723b_fm_SetTune_Hz(cxt->pFm,  frequency*100*1000 )!= FM_FUNCTION_SUCCESS)	goto error;
	}

	if(seek_dir==RTL_SEEK_DIR_UP)
	{	
		if(rtl8723b_fm_SetSeekDirection( cxt->pFm, FM_SEEK_DIR_UP ))	goto error;
	}
	else
	{
		if(rtl8723b_fm_SetSeekDirection( cxt->pFm, FM_SEEK_DIR_DOWN ))	goto error;
	}

	jiffies_comp = jiffies;

reseek:	
	
	
	if(rtl8723b_fm_Seek( cxt->pFm, FM_SEEK_ENABLE ))	goto error;


	SeekTuneComplete = 0;

	do
	{

#ifdef FM_VOLUME_ON	
		//gordon
		//dbg_info( "gordon before schedule\n");		
		cancel_delayed_work(&fm_volume_on);
		schedule_delayed_work(&fm_volume_on, HZ/2);
#endif
		if (atomic_read(&cxt->fm_searching) == 0)
			break;
        
		msleep_interruptible(REALTEK_FM_SEEK_CHECK_TIME);
		if (signal_pending(current))
			break;

		if(cxt->pFm->GetReadOnlyRegMaskBits(cxt->pFm, 10, FM_ADDR_DEBUG_1, 5, 5, &SeekTuneComplete)) goto error;

		if(rtl8723b_fm_GetFrequencyHz(cxt->pFm, &FrequencyHz1)!= FM_FUNCTION_SUCCESS)	goto error;

		if(rtl8723b_fm_GetSNR(cxt->pFm, &SnrDbNum1, &SnrDbDen1)!= FM_FUNCTION_SUCCESS)	goto error;

		if(cxt->pFm->GetReadOnlyRegMaskBits(cxt->pFm, 21, FM_ADDR_DEBUG_0, 2, 0, &RFGain))		goto error;
		if(cxt->pFm->GetReadOnlyRegMaskBits(cxt->pFm, 21, FM_ADDR_DEBUG_0, 7, 3, &RXBBGain))	goto error;

		dbg_info( " ***FM full_search-ING  ...  : #SeekTuneComplete=%ld, freq out =%dHz, SNR=%ld, RFGain =%ld, RXBBGain=%ld***\n", SeekTuneComplete, FrequencyHz1, SnrDbNum1*10/SnrDbDen1, RFGain, RXBBGain);		
		
		is_timeout = time_after(jiffies, jiffies_comp + msecs_to_jiffies(7500));
		
	}while( (SeekTuneComplete==0) && (!is_timeout) );

	if(SeekTuneComplete==0)
	{
		dbg_info( " FM full_search : seek timeout      ERROR!!!\n");
	}

	if( SeekTuneComplete==1 )
	{
		Status = FM_FUNCTION_SUCCESS;
	}
	else
	{
		Status = FM_FUNCTION_FULL_SEARCH_ERROR;
	}

	msleep(2);
		
	if(rtl8723b_fm_GetFrequencyHz(cxt->pFm, &FrequencyHz1)!= FM_FUNCTION_SUCCESS)	goto error;

	if(rtl8723b_fm_SetTune_Hz(cxt->pFm, FrequencyHz1)!= FM_FUNCTION_SUCCESS)	goto error;

	if(rtl8723b_fm_GetFrequencyHz(cxt->pFm, &FrequencyTest)!= FM_FUNCTION_SUCCESS)	goto error;

	dbg_info( " FM full_search : *** bef  rtl8723b_fm_SetTune_Hz freq = %d, after rtl8723b_fm_SetTune_Hz freq = %d***\n", FrequencyHz1, FrequencyTest);

	if(rtl8723b_fm_GetSNR(cxt->pFm, &SnrDbNum1, &SnrDbDen1)!= FM_FUNCTION_SUCCESS)	goto error;

	*freq_found = FrequencyHz1/100000;

#if 1
	if(Status == FM_FUNCTION_SUCCESS)
	{

		if(SnrDbNum1 < 896)
		{
			dbg_info( " FM full_search : SnrDbNum1 < 896 ..............so RE-SEEK\n");
			goto reseek;
		}

		dbg_info( " FM full_search :                         a. Freq:%d, SNR:%ld\n", FrequencyHz1/100000, SnrDbNum1*10/SnrDbDen1);

		if(seek_dir==RTL_SEEK_DIR_UP)
			FrequencyHz2 = FrequencyHz1+100000;
		else
			FrequencyHz2 = FrequencyHz1-100000;

		if(rtl8723b_fm_SetTune_Hz(cxt->pFm, FrequencyHz2)!= FM_FUNCTION_SUCCESS)	goto error;
			
		if(rtl8723b_fm_GetSNR(cxt->pFm, &SnrDbNum2, &SnrDbDen2)!= FM_FUNCTION_SUCCESS)	goto error;

		dbg_info( " FM full_search :                      b. Freq:%d, SNR:%ld\n", FrequencyHz2/100000, SnrDbNum2*10/SnrDbDen2);

		if(SnrDbNum1 <= SnrDbNum2 + 128)
		{
			dbg_info( " FM full_search :   snr1_%ld <= snr2_%ld+0.5...............so RE-SEEK\n", SnrDbNum1*10/SnrDbDen1, SnrDbNum2*10/SnrDbDen2);

			if(rtl8723b_fm_SetTune_Hz(cxt->pFm, FrequencyHz1)!= FM_FUNCTION_SUCCESS)	goto error;
		
			goto reseek;
		}
		else
		{
			if(rtl8723b_fm_SetTune_Hz(cxt->pFm, FrequencyHz1)!= FM_FUNCTION_SUCCESS)	goto error;
			
		}

		if(  ((cxt->last_seek_freq+1) == *freq_found) || ((cxt->last_seek_freq-1) == *freq_found) )
		{
			dbg_info( " FM full_search : Diff (last_seek_freq = %d, freq_found = %d) =100kHz , so RE-SEEK\n",cxt->last_seek_freq, *freq_found);
			goto reseek;
		}
		else
		{
			cxt->last_seek_freq = *freq_found;
		}


	}
#endif
 
	if(cxt->pFm->SetRegMaskBits(cxt->pFm, 0x006c, 0, 0, 0))		goto error;
	
	if(cxt->pFm->SetRfcReg2Bytes(cxt->pFm, 0x0f, 0x0000))	goto error;

	if(cxt->pFm->SetRegMaskBits(cxt->pFm, 0x006c, 0, 0, 1))		goto error;
	
	cxt->current_freq = *freq_found;

	if(cxt->current_volume != 0)
	{
	    rtl8723b_fm_SetMute(cxt->pFm, 0);	
	}	
	
	atomic_cmpxchg(&cxt->fm_searching, 1, 0);

	dbg_info( 
		"-FM full_search 1 : current freq=%d, dir=%d, timeout=%d\n", frequency, seek_dir, time_out);

	dbg_info( 
		"-FM full_search 2 : ***Status = %d, freq find out =%dHz***\n",Status, *freq_found);

	return Status;

error:

	cxt->pFm->SetRfcReg2Bytes(cxt->pFm, 0x0f, 0x0000);

	cxt->pFm->SetRegMaskBits(cxt->pFm, 0x006c, 0, 0, 1);
	
	if(cxt->current_volume != 0)
	{
	    rtl8723b_fm_SetMute(cxt->pFm, 0);	
	}
	
	dev_err(&cxt->client->dev, "-FM full_search : err\n");	


	atomic_cmpxchg(&cxt->fm_searching, 1, 0);	

	return FM_FUNCTION_ERROR;
}


#if 1
#define VOLUME_LEVEL_LEN		16
unsigned long VOLUME_LEVEL[VOLUME_LEVEL_LEN] = 
{
	0,
	60,
	70,
	83,
	98,
	115,
	136,
	161,
	189,
	224,
	264,
	311,
	367,
	432,
	510,
	601,
};
#else

#define VOLUME_LEVEL_LEN		8
unsigned long VOLUME_LEVEL[VOLUME_LEVEL_LEN] = 
{
	0,
	//60,
	//70,
	83,
	//98,
	115,
	//136,
	161,
	//189,
	224,
	//264,
	311,
	//367,
	432,
	//510,
	601,
};

#endif

int
realtek_fm_set_volume(
	struct realtek_fm_drv_data *cxt,
	u8 volume
	)
{
	int i;
	
	if (!atomic_read(&cxt->fm_opened)) {

		dev_err(&cxt->client->dev, "FM set_volume: FM not open\n");

		return FM_FUNCTION_ERROR;
	}

	if(volume > 15 || volume < 0)
		return FM_FUNCTION_ERROR;
		
	dbg_info( "+FM set_volume : volume = %d\n", volume);



	if( cxt->current_volume >= volume)    //volume down
	{
		for( i = cxt->current_volume;  i >= volume ; i--)
		{
			if(cxt->pFm->SetRegMaskBits(cxt->pFm, 0x0024, 11, 0, VOLUME_LEVEL[i])) goto error;

		}
		
	}
	else if( cxt->current_volume < volume)  //volume up
	{
		for( i = cxt->current_volume+1;  i <= volume ; i++)
		{
			if(cxt->pFm->SetRegMaskBits(cxt->pFm, 0x0024, 11, 0, VOLUME_LEVEL[i])) goto error;

		}		

	}

	cxt->current_volume = volume;

	dbg_info( "-FM set_volume : volume = %d\n", volume);


	return FM_FUNCTION_SUCCESS;
	
error:

	dbg_info( "-FM set_volume : err\n");

	return FM_FUNCTION_ERROR;	

}



int
realtek_fm_get_volume(
	struct realtek_fm_drv_data *cxt
	)
{
	unsigned long Value;
	unsigned long volume;
	
	if (!atomic_read(&cxt->fm_opened)) {

		dev_err(&cxt->client->dev, "FM get_volume: FM not open\n");

		return FM_FUNCTION_ERROR;
	}

	dbg_info( "+FM get_volume\n");
	
	if(cxt->pFm->GetRegMaskBits(cxt->pFm, 0x0024, 11, 0, &Value)) goto error;

	for(volume = 0 ; volume < VOLUME_LEVEL_LEN ; volume++)
	{
		if(Value == VOLUME_LEVEL[volume])
			break;
	}

	if(volume == VOLUME_LEVEL_LEN)
	{
		dbg_info( " FM get_volume err : find no correct volume level\n");
	}
	
	dbg_info( "-FM get_volume = %ld\n",  volume);
	
	return volume;

error:

	dbg_info( "-FM get_volume : err\n");

	return FM_FUNCTION_ERROR;	
	
}


#define TROUT_FM_VERSION "V0.1"
#define DRIVER_NAME "radio-trout"
#define CARD_NAME "Trout FM Receiver"


#define FM_FREQ_CONVERSION (625)
#define V4L2_CID_PRIVATE_FM_AUDIO (V4L2_CID_BASE + 10)



struct fmdev {
	struct v4l2_ctrl_handler ctrl_handler; /* V4L2 ctrl framwork handler*/
};

static struct video_device *s_radio = NULL;



static int
_fm_open(struct file *file)
{
	return realtek_fm_open(realtek_fm_dev_data);
}



static int
_fm_release(
	struct file *file
	)
{
	return realtek_fm_close(realtek_fm_dev_data);
}



static int
_fm_querycap(
	struct file *file,
	void *priv,
	struct v4l2_capability *cap
	)
{
	strlcpy(cap->driver, DRIVER_NAME, sizeof(cap->driver));
	strlcpy(cap->card, CARD_NAME, sizeof(cap->card));
	cap->version = KERNEL_VERSION(0, 1, 0);
	cap->capabilities = V4L2_CAP_RADIO |
			    V4L2_CAP_TUNER |
			    V4L2_CAP_HW_FREQ_SEEK;
	return 0;
}



static int
_fm_get_tuner(
	struct file *file,
	void *priv,
	struct v4l2_tuner *tuner
	)
{
	strlcpy(tuner->name, "FM", sizeof(tuner->name));
	tuner->type = V4L2_TUNER_RADIO;
	tuner->rangelow = 1400000;
	tuner->rangehigh = 1728000;
	memset(tuner->reserved, 0, sizeof(tuner->reserved));
	return -EINVAL;
}



static int
_fm_set_tuner(
	struct file *file,
	void *priv,
	struct v4l2_tuner *tuner
	)
{
	return -EINVAL;
}



static int
_fm_get_freq(
	struct file *file,
	void *priv,
	struct v4l2_frequency *freq
	)
{
	u16 frequency = 0;

	frequency = realtek_fm_get_frequency(realtek_fm_dev_data);
	//dbg_info( "randy :get freq :%d \n *100KHz",frequency);
	
	if(frequency < 0 ) goto error;
		
	freq->frequency = (((u32)frequency)*100)*16;
	//dbg_info( "randy :get freq to hal :%d \n",freq->frequency);
	
	freq->type = V4L2_TUNER_RADIO;

	memset(freq->reserved, 0, sizeof(freq->reserved));

	return FM_FUNCTION_SUCCESS;

error:

	return FM_FUNCTION_ERROR;

}



static int
_fm_set_freq(
	struct file *file,
	void *priv,
	struct v4l2_frequency *freq
	)
{
//	dbg_info( "randy :freq :%d \n",freq->frequency);
	int frequency = ((freq->frequency) * 625)/1000/1000;
//	dbg_info( "randy: after covn freq :%d *100KHz\n",frequency);

	if(realtek_fm_set_tune(realtek_fm_dev_data, frequency) !=FUNCTION_SUCCESS)
		goto error;

	return FM_FUNCTION_SUCCESS;
	
error:
	
	return FM_FUNCTION_ERROR;

}

static int
_fm_set_volume(	struct file *file,
			void *priv,
			struct v4l2_volume *vol
					)
{
		u8 volume = vol->volume;
		if (realtek_fm_set_volume(realtek_fm_dev_data, volume) <0)
						return FM_FUNCTION_ERROR;
				return FM_FUNCTION_SUCCESS;
}


static int
_fm_get_volume(	struct file *file,
		void *priv,
		struct v4l2_volume *vol
		)
{

	u8 volume =0;

	volume = realtek_fm_get_volume(realtek_fm_dev_data) ;

	if(volume <0 )
		return FM_FUNCTION_ERROR;
	vol->volume = volume;

	vol->type = V4L2_TUNER_RADIO;

	return FM_FUNCTION_SUCCESS;

}



static int 
_fm_seek(
	struct file *file,
	void *priv,
	struct v4l2_hw_freq_seek *seek
	)
{
	u8 direction = seek->seek_upward;
	u16 freq_found = 0;

	//seek start frequency = 0 => from CURRENT fm hw frequency
	if(realtek_fm_full_search(realtek_fm_dev_data, 0, direction, 300, &freq_found) != FUNCTION_SUCCESS)
		goto error;

	seek->reserved[0] = (freq_found * 100)*16;

	return FM_FUNCTION_SUCCESS;

error:

	return FM_FUNCTION_ERROR;
	
}

static ssize_t fm_set_volume_store(struct kobject *kobj, struct kobj_attribute *attr, char *buff, size_t len)               //   
{
	    //u8 volume = (u8)(*buff);
		u8 volume = simple_strtoul(buff,NULL,10);
	
		if (realtek_fm_set_volume(realtek_fm_dev_data, volume) <0)
		      return FM_FUNCTION_ERROR;
	   return len; 
}


static ssize_t fm_get_volume_show(struct kobject *kobj, struct kobj_attribute *attr, char *buff)        //   
{                                                                                                                                              
	    u8 volume =0;
        volume = realtek_fm_get_volume(realtek_fm_dev_data) ;

        if(volume <0 )
			return FM_FUNCTION_ERROR;
							     
		//itoa(volume,buff,10);
		snprintf(buff,1,"%d",volume);

       return 1;
}

static struct kobject *fm_volume_kobj = NULL;                                                                                           

static struct kobj_attribute realtek_fm_volume_attr =
        __ATTR(realtek_fm_volume, 0644, fm_get_volume_show, fm_set_volume_store);


static int
_fm_control(
	struct v4l2_ctrl *ctrl
	)
{
	dbg_info("fm control\n");

	switch (ctrl->id) 
	{
		case V4L2_CID_PRIVATE_FM_AUDIO:
		break;
		case V4L2_CID_AUDIO_VOLUME:
			dbg_info("volume: %d\n",(u16)ctrl->val);
			return realtek_fm_set_volume(realtek_fm_dev_data, (u8)ctrl->val);
		default:
		break;
	}
	return 0;
}


static const struct v4l2_ctrl_ops trout_ctrl_ops = {
	.s_ctrl = _fm_control,
};

static const struct v4l2_file_operations trout_fops = {
	.owner = THIS_MODULE,
	.release = _fm_release,
	.open = _fm_open,
	.unlocked_ioctl = video_ioctl2,
};

static const struct v4l2_ioctl_ops trout_ioctl_ops = {
	.vidioc_querycap = _fm_querycap,
	.vidioc_g_tuner = _fm_get_tuner,
	.vidioc_s_tuner = _fm_set_tuner,
	.vidioc_g_frequency = _fm_get_freq,
	.vidioc_s_frequency = _fm_set_freq,
	.vidioc_s_hw_freq_seek = _fm_seek,
	.vidioc_s_volume = _fm_set_volume,		//add
	.vidioc_g_volume = _fm_get_volume,		//add
};



static int register_v4l2_device(void)
{
	int ret;
	struct fmdev *fmdev = NULL;
	struct video_device *radio = video_device_alloc();

	if (!radio)
	{
		dbg_info("register_v4l2_device : Could not allocate video_device");
		return -EINVAL;
	}
	strlcpy(radio->name, DRIVER_NAME, sizeof(radio->name));
	radio->fops = &trout_fops;
	radio->release = video_device_release;
	radio->ioctl_ops = &trout_ioctl_ops;
	if (video_register_device(radio, VFL_TYPE_RADIO, -1))
	{
		dbg_info("register_v4l2_device : Could not register video_device");
		video_device_release(radio);
		return -EINVAL;
	}
	s_radio = radio;

	fmdev = (struct fmdev *)kzalloc(sizeof(struct fmdev), GFP_KERNEL);
	if (!fmdev)
	{
		dbg_info("register_v4l2_device : Could not allocate fmdev");
		return -EINVAL;
	}
	video_set_drvdata(radio, fmdev);
	radio->ctrl_handler = &fmdev->ctrl_handler;
	ret = v4l2_ctrl_handler_init(&fmdev->ctrl_handler, 2);	
	if (ret < 0)
	{
		dbg_info("register_v4l2_device : Failed to int v4l2_ctrl_handler");
		v4l2_ctrl_handler_free(&fmdev->ctrl_handler);
		return -EINVAL;
	}
	v4l2_ctrl_new_std(&fmdev->ctrl_handler, &trout_ctrl_ops,
		V4L2_CID_AUDIO_VOLUME, 0, 14, 1, 12);	
	v4l2_ctrl_new_std(&fmdev->ctrl_handler, &trout_ctrl_ops,
		V4L2_CID_PRIVATE_FM_AUDIO, 0, 1, 1, 0);


	dbg_info("register_v4l2_device : Registered Realtek FM Receiver.");
	return 0;
}



static int unregister_v4l2_device(void)
{
	struct fmdev *fmdev;
	dbg_info("unregister_v4l2_device \n");

	if(s_radio)
	{
		fmdev = video_get_drvdata(s_radio);
		if (fmdev)
		{
			v4l2_ctrl_handler_free(&fmdev->ctrl_handler);
			kfree(fmdev);
		}
		video_unregister_device(s_radio);
		s_radio = NULL;
	}
	return 0;
}



static void fm_gpio_init(void)
{

	if(GPIO_FM_EN > 0)
	{
		printk("FM: FM EN is %d\n", GPIO_FM_EN);
		gpio_request(GPIO_FM_EN, "fm_en");
	}
	else
	{
		printk("fm enable pin is not set, please check\n");
	}
}



static void fm_gpio_deinit(void)
{
	if(GPIO_FM_EN > 0){
    	gpio_free(GPIO_FM_EN);
    }else{
		printk("fm enable pin is not set, please check\n");
	}
}


static int __devexit realtek_fm_remove(struct i2c_client *client)
{
	struct realtek_fm_drv_data  *cxt = i2c_get_clientdata(client);

	realtek_fm_close(cxt);
	
	fm_gpio_deinit();

	unregister_v4l2_device();

	kfree(cxt);

	dbg_info( "%s\n", __func__);

	return 0;    
}



static int realtek_fm_probe(
	struct i2c_client *client,
	const struct i2c_device_id *id
	)
{
	struct realtek_fm_drv_data *cxt;
	int ret;


	ret = -EINVAL;
	cxt = NULL;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "realtek_fm driver: client is not i2c capable.\n");
		ret = -ENODEV;
		goto i2c_functioality_failed;
	}

	cxt = kzalloc(sizeof(struct realtek_fm_drv_data), GFP_KERNEL);
	if (cxt == NULL) {
		dev_err(&client->dev, "Can't alloc memory for module data.\n");
		ret = -ENOMEM;
		goto alloc_data_failed;
	}

	mutex_init(&mutex);

	cxt->client = client;
	i2c_set_clientdata(client, cxt);

	atomic_set(&cxt->fm_searching, 0);
	atomic_set(&cxt->fm_opened, 0);

	BuildBaseInterface(
		&cxt->pBaseInterface,
		&cxt->BaseInterfaceModuleMemory,
		9,
		8,
		custom_i2c_read,
		custom_i2c_write,
		custom_wait_ms
		);

	BuildRtl8723bFmModule(
		&cxt->pFm,
		&cxt->FmModuleMemory,
		cxt->pBaseInterface,
		REALTEK_FM_I2C_ADDR
		);

	cxt->pBaseInterface->SetUserDefinedDataPointer(cxt->pBaseInterface, cxt);

	fm_gpio_init();
			
	ret = register_v4l2_device();
	if(ret < 0)
	{
		dev_err(&client->dev, "realtekfm v4l2 device register failed.\n");
		goto misc_register_failed;
	}

	cxt->opened_before_suspend = 0;
	cxt->bg_play_enable = 1;
	cxt->current_freq = 870; /* init current frequency, search may use it. */
	cxt->last_seek_freq = 0;
	cxt->current_volume = 0;
    
	realtek_fm_dev_data = cxt;
	

    fm_volume_kobj = kobject_create_and_add("realtek_fm", kernel_kobj);
	 if (fm_volume_kobj == NULL) {
			ret = -ENOMEM;
			dbg_info("register sysfs failed. ret = %d\n", ret);
			goto misc_register_failed;
		 }

	ret = sysfs_create_file(fm_volume_kobj, &realtek_fm_volume_attr.attr);
	if (ret) {
	        dbg_info("create sysfs failed. ret = %d\n", ret);
		goto misc_register_failed;
	 }
	
	return ret; 

misc_register_failed:
	unregister_v4l2_device();
	kfree(cxt);
alloc_data_failed:
i2c_functioality_failed:
	dev_err(&client->dev,"realtek_fm driver init failed.\n");
	return ret;
}


static const struct i2c_device_id realtek_fm_i2c_id[] = { 
    { REALTEK_FM_I2C_NAME, 0 }, 
    { }, 
};

MODULE_DEVICE_TABLE(i2c, realtek_fm_i2c_id);

static struct i2c_driver realtek_fm_i2c_driver = {
	.driver = {
		.name = REALTEK_FM_I2C_NAME,
	},
	.probe    =  realtek_fm_probe,
	.remove   =  __devexit_p(realtek_fm_remove),
 	.id_table =  realtek_fm_i2c_id,
};


static int __init realtek_fm_driver_init(void)
{
	int  ret = 0;        

	dbg_info("REALTEK_FM driver: init\n");
	ret =  i2c_add_driver(&realtek_fm_i2c_driver);

	return ret;
}

static void __exit realtek_fm_driver_exit(void)
{

	dbg_info("REALTEK_FM driver exit\n");

	i2c_del_driver(&realtek_fm_i2c_driver);
 	return;
}

module_init(realtek_fm_driver_init);
module_exit(realtek_fm_driver_exit);

MODULE_DESCRIPTION("TROUT v4l2 FM radio driver");
MODULE_AUTHOR("Spreadtrum Inc - Jesse.Ji");
MODULE_LICENSE("GPL");
MODULE_VERSION(TROUT_FM_VERSION);



