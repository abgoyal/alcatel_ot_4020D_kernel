/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <linux/delay.h>
#include "common/sensor.h"
#include "common/jpeg_exif_header_k.h"
#include "common/sensor_drv.h"
#include "common/sensor_flash.h"

/**---------------------------------------------------------------------------*
 ** 						Compiler Flag									  *
 **---------------------------------------------------------------------------*/
#ifdef	 __cplusplus
	extern	 "C"
	{
#endif
/**---------------------------------------------------------------------------*
 ** 						   Macro Define
 **---------------------------------------------------------------------------*/
#define GC0328_I2C_ADDR_W	0x21 // 0x42 --> 0x42 / 2
#define GC0328_I2C_ADDR_R		0x21 // 0x43 --> 0x43 / 2

#define SENSOR_GAIN_SCALE		16 
static uint32_t g_flash_mode_en = 0;
/**---------------------------------------------------------------------------*
 ** 					Local Function Prototypes							  *
 **---------------------------------------------------------------------------*/
LOCAL uint32_t set_gc0328_ae_enable(uint32_t enable);
LOCAL uint32_t set_hmirror_enable(uint32_t enable);
LOCAL uint32_t set_vmirror_enable(uint32_t enable);
LOCAL uint32_t set_preview_mode(uint32_t preview_mode);
LOCAL uint32_t GC0328_Identify(uint32_t param);
LOCAL uint32_t _GC0328_PowerOn(uint32_t power_on);
LOCAL uint32_t GC0328_BeforeSnapshot(uint32_t param);
LOCAL uint32_t GC0328_After_Snapshot(uint32_t param);
LOCAL uint32_t set_brightness(uint32_t level);
LOCAL uint32_t set_contrast(uint32_t level);
LOCAL uint32_t set_sharpness(uint32_t level);
LOCAL uint32_t set_saturation(uint32_t level);
LOCAL uint32_t set_image_effect(uint32_t effect_type);
LOCAL uint32_t read_ev_value(uint32_t value);
LOCAL uint32_t write_ev_value(uint32_t exposure_value);
LOCAL uint32_t read_gain_value(uint32_t value);
LOCAL uint32_t write_gain_value(uint32_t gain_value);
LOCAL uint32_t read_gain_scale(uint32_t value);
LOCAL uint32_t set_frame_rate(uint32_t param);
LOCAL uint32_t set_gc0328_ev(uint32_t level);
LOCAL uint32_t set_gc0328_awb(uint32_t mode);
LOCAL uint32_t set_gc0328_anti_flicker(uint32_t mode);
LOCAL uint32_t set_gc0328_video_mode(uint32_t mode);
LOCAL uint32_t _gc0328_flash(uint32_t param);
/**---------------------------------------------------------------------------*
 ** 						Local Variables 								 *
 **---------------------------------------------------------------------------*/
 typedef enum
{
        FLICKER_50HZ = 0,
        FLICKER_60HZ,
        FLICKER_MAX
}FLICKER_E;

SENSOR_REG_T gc0328_YUV_640X480[]=
{
	{0xfe , 0x80},
	{0xfe , 0x80},
	{0xfc , 0x16},
	{0xfc , 0x16},
	{0xfc , 0x16},
	{0xfc , 0x16},
	{0xf1 , 0x00},
	{0xf2 , 0x00},
	{0xfe , 0x00},
	{0x4f , 0x00},
	{0x03 , 0x00},
	{0x04 , 0xc0},
	{0x42 , 0x00},
	{0x77 , 0x5a},
	{0x78 , 0x40},
	{0x79 , 0x56},

	{0xfe , 0x00},
	{0x0d , 0x01},
	{0x0e , 0xe8},
	{0x0f , 0x02},
	{0x10 , 0x88},
	{0x09 , 0x00},
	{0x0a , 0x00},
	{0x0b , 0x00},
	{0x0c , 0x00},
	{0x16 , 0x00},
	{0x17 , 0x14},
	{0x18 , 0x0e},
	{0x19 , 0x06},

	{0x1b , 0x48},
	{0x1f , 0xC8},
	{0x20 , 0x01},
	{0x21 , 0x78},
	{0x22 , 0xb0},
	{0x23 , 0x04},
	{0x24 , 0x11}, 
	{0x26 , 0x00},
	{0x50 , 0x01}, //crop mode

	//global gain for range 
	{0x70 , 0x45},
	
	/////////////banding/////////////
	{0x05 , 0x02},//hb
	{0x06 , 0x2c},//
	{0x07 , 0x00},//vb
	{0x08 , 0xb8},//
	{0xfe , 0x01},//
	{0x29 , 0x00},//anti-flicker step [11:8]
	{0x2a , 0x60},//anti-flicker step [7:0]
	{0x2b , 0x02},//exp level 0  14.28fps
	{0x2c , 0xa0},//             
	{0x2d , 0x03},//exp level 1  12.50fps
	{0x2e , 0x00},//             
	{0x2f , 0x03},//exp level 2  10.00fps
	{0x30 , 0xc0},//             
	{0x31 , 0x05},//exp level 3  7.14fps
	{0x32 , 0x40},//
	{0xfe , 0x00},//

	///////////////AWB//////////////
	{0xfe , 0x01},
	{0x50 , 0x00},
	{0x4f , 0x00},
	{0x4c , 0x01},
	{0x4f , 0x00},
	{0x4f , 0x00},
	{0x4f , 0x00},
	{0x4f , 0x00},
	{0x4f , 0x00}, 
	{0x4d , 0x30},
	{0x4e , 0x04},	
	{0x4e , 0x04}, 
	{0x4e , 0x04},	
	{0x4e , 0x04},
	{0x4e , 0x04},	
	{0x4e , 0x04}, 
	{0x4e , 0x04},	
	{0x4e , 0x04},
	{0x4e , 0x04},	
	{0x4e , 0x04}, 
	{0x4e , 0x04},	
	{0x4e , 0x04},
	{0x4e , 0x04}, 
	{0x4e , 0x04},	
	{0x4e , 0x04},
	{0x4e , 0x04},
	{0x4d , 0x40},
	{0x4e , 0x04},	
	{0x4e , 0x04}, 
	{0x4e , 0x04},	
	{0x4e , 0x04},
	{0x4e , 0x04},	
	{0x4e , 0x04}, 
	{0x4e , 0x04},	
	{0x4e , 0x04},
	{0x4e , 0x04},	
	{0x4e , 0x04}, 
	{0x4e , 0x04},	
	{0x4e , 0x04},
	{0x4e , 0x04}, 
	{0x4e , 0x04},	
	{0x4e , 0x04},
	{0x4e , 0x04},
	{0x4d , 0x50},
	{0x4e , 0x04},	
	{0x4e , 0x04}, 
	{0x4e , 0x04},	
	{0x4e , 0x04},
	{0x4e , 0x04},	
	{0x4e , 0x04}, 
	{0x4e , 0x04},	
	{0x4e , 0x04},
	{0x4e , 0x04},	
	{0x4e , 0x04}, 
	{0x4e , 0x04},	
	{0x4e , 0x04},
	{0x4e , 0x04}, 
	{0x4e , 0x04},	
	{0x4e , 0x04},
	{0x4e , 0x04},
	{0x4d , 0x60},
	{0x4e , 0x04},	
	{0x4e , 0x04}, 
	{0x4e , 0x04},	
	{0x4e , 0x04},
	{0x4e , 0x04},	
	{0x4e , 0x04}, 
	{0x4e , 0x04},	
	{0x4e , 0x04},
	{0x4e , 0x04},	
	{0x4e , 0x04}, 
	{0x4e , 0x04},	
	{0x4e , 0x04},
	{0x4e , 0x04}, 
	{0x4e , 0x04},	
	{0x4e , 0x04},
	{0x4e , 0x04},
	{0x4d , 0x70},
	{0x4e , 0x04},	
	{0x4e , 0x04}, 
	{0x4e , 0x04},	
	{0x4e , 0x04},
	{0x4e , 0x04},	
	{0x4e , 0x04}, 
	{0x4e , 0x04},	
	{0x4e , 0x04},
	{0x4e , 0x04},	
	{0x4e , 0x04}, 
	{0x4e , 0x04},	
	{0x4e , 0x04},
	{0x4e , 0x04}, 
	{0x4e , 0x04},	
	{0x4e , 0x04},
	{0x4e , 0x04},
	{0x4f , 0x01},
	{0x50 , 0x88},
	{0xfe , 0x00},

	//////////// BLK//////////////////////
	{0xfe , 0x00}, 
	{0x27 , 0xb7},
	{0x28 , 0x7F},
	{0x29 , 0x20},
	{0x33 , 0x20},
	{0x34 , 0x20},
	{0x35 , 0x20},
	{0x36 , 0x20},
	{0x32 , 0x08},
	{0x3b , 0x00}, 
	{0x3c , 0x00},
	{0x3d , 0x00},
	{0x3e , 0x00},
	{0x47 , 0x00},
	{0x48 , 0x00}, 

	//////////// block enable/////////////
	{0x40 , 0x7f}, 
	{0x41 , 0x26}, 
	{0x42 , 0xfb},
	{0x44 , 0x02}, //yuv
	{0x45 , 0x00},
	{0x46 , 0x02},
	{0x4f , 0x01},
	{0x4b , 0x01},
	{0x50 , 0x01}, 

	/////DN & EEINTP/////
	{0x7e , 0x0a}, 
	{0x7f , 0x03}, 
	{0x81 , 0x15}, 
	{0x82 , 0x90},
	{0x83 , 0x02},
	{0x84 , 0xe5},
	{0x90 , 0x2c}, 
	{0x92 , 0x02},
	{0x94 , 0x02},
	{0x95 , 0x33},     //35

	////////////YCP///////////
	{0xd1 , 0x22},//24  0x30 for front
	{0xd2 , 0x22},//24  0x30 for front
	{0xd3 , 0x42},  //40
	{0xdd , 0xd3},
	{0xde , 0x38},
	{0xe4 , 0x88},
	{0xe5 , 0x40}, 
	{0xd7 , 0x0e}, 

	///////////rgb gamma ////////////
	{0xfe , 0x00},
	{0xbf , 0x0e},
	{0xc0 , 0x1c},
	{0xc1 , 0x34},
	{0xc2 , 0x48},
	{0xc3 , 0x5a},
	{0xc4 , 0x6e},
	{0xc5 , 0x80},
	{0xc6 , 0x9c},
	{0xc7 , 0xb4},
	{0xc8 , 0xc7},
	{0xc9 , 0xd7},
	{0xca , 0xe3},
	{0xcb , 0xed},
	{0xcc , 0xf2},
	{0xcd , 0xf8},
	{0xce , 0xfd},
	{0xcf , 0xff},

	/////////////Y gamma//////////
	{0xfe , 0x00},
	{0x63 , 0x00},
	{0x64 , 0x05},
	{0x65 , 0x0b},
	{0x66 , 0x19},
	{0x67 , 0x2e},
	{0x68 , 0x40},
	{0x69 , 0x54},
	{0x6a , 0x66},
	{0x6b , 0x86},
	{0x6c , 0xa7},
	{0x6d , 0xc6},
	{0x6e , 0xe4},
	{0x6f , 0xff},
	
	//////////////ASDE/////////////
	{0xfe , 0x01},
	{0x18 , 0x02},
	{0xfe , 0x00},
	{0x97 , 0x30},
	{0x98 , 0x00},
	{0x9b , 0x60},
	{0x9c , 0x60},
	{0xa4 , 0x50},
	{0xa8 , 0x80},
	{0xaa , 0x40},
	{0xa2 , 0x23},
	{0xad , 0x28},
	
	//////////////abs///////////
	{0xfe , 0x01},
	{0x9c , 0x00}, 
	{0x9e , 0xc0}, 
	{0x9f , 0x40},	
	
	////////////// AEC////////////
	{0xfe , 0x01},
	{0x08 , 0xa0},
	{0x09 , 0xe8},
	{0x10 , 0x08},
	{0x11 , 0x11},
	{0x12 , 0x11},
	{0x13 , 0x4a},
	{0x15 , 0xfc},
	{0x18 , 0x02},
	{0x21 , 0xf0},
	{0x22 , 0x60},
	{0x23 , 0x30},
	{0x25 , 0x00},
	{0x24 , 0x14},
	{0x3d , 0x80},
	{0x3e , 0x40},

	////////////////AWB///////////
	{0xfe , 0x01},
	{0x51 , 0x88},
	{0x52 , 0x12},
	{0x53 , 0x80},
	{0x54 , 0x60},
	{0x55 , 0x01},
	{0x56 , 0x02},
	{0x58 , 0x00},
	{0x5b , 0x02},
	{0x5e , 0xa4},
	{0x5f , 0x8a},
	{0x61 , 0xdc},
	{0x62 , 0xdc},
	{0x70 , 0xfc},
	{0x71 , 0x10},
	{0x72 , 0x30},
	{0x73 , 0x0b},
	{0x74 , 0x0b},
	{0x75 , 0x01},
	{0x76 , 0x00},
	{0x77 , 0x40},
	{0x78 , 0x70},
	{0x79 , 0x00},
	{0x7b , 0x00},
	{0x7c , 0x71},
	{0x7d , 0x00},
	{0x80 , 0x70},
	{0x81 , 0x58},
	{0x82 , 0x98},
	{0x83 , 0x60},
	{0x84 , 0x58},
	{0x85 , 0x50},
	{0xfe , 0x00},	
	
	////////////////LSC////////////////
	{0xfe , 0x01},
	{0xc0 , 0x10},
	{0xc1 , 0x0b},
	{0xc2 , 0x06},
	{0xc6 , 0x11},
	{0xc7 , 0x0b},
	{0xc8 , 0x09},
	{0xba , 0x30},
	{0xbb , 0x20},
	{0xbc , 0x1d},
	{0xb4 , 0x23},
	{0xb5 , 0x19},
	{0xb6 , 0x15},
	{0xc3 , 0x09},
	{0xc4 , 0x00},
	{0xc5 , 0x0c},
	{0xc9 , 0x00},
	{0xca , 0x00},
	{0xcb , 0x00},
	{0xbd , 0x09},
	{0xbe , 0x00},
	{0xbf , 0x03},
	{0xb7 , 0x1a},
	{0xb8 , 0x05},
	{0xb9 , 0x03},
	{0xa8 , 0x07},
	{0xa9 , 0x00},
	{0xaa , 0x02},
	{0xab , 0x04},
	{0xac , 0x00},
	{0xad , 0x02},
	{0xae , 0x0a},
	{0xaf , 0x00},
	{0xb0 , 0x00},
	{0xb1 , 0x08},
	{0xb2 , 0x00},
	{0xb3 , 0x00},
	{0xa4 , 0x00},
	{0xa5 , 0x00},
	{0xa6 , 0x00},
	{0xa7 , 0x00},
	{0xa1 , 0x3c},
	{0xa2 , 0x50},
	{0xfe , 0x00},
	
	///////////////CCT ///////////
	{0xb1 , 0x12},
	{0xb2 , 0xf5},
	{0xb3 , 0xfe},
	{0xb4 , 0xe0},
	{0xb5 , 0x15},
	{0xb6 , 0xc8},
	
	/*   /////skin CC for front //////
	{0xb1 , 0x00},
	{0xb2 , 0x00},
	{0xb3 , 0x00},
	{0xb4 , 0xf0},
	{0xb5 , 0x00},
	{0xb6 , 0x00},
	*/

	///////////////AWB////////////////
	{0xfe , 0x01},
	{0x50 , 0x00},
	{0xfe , 0x01}, 
	{0x4f , 0x00},
	{0x4c , 0x01},
	{0x4f , 0x00},
	{0x4f , 0x00},
	{0x4f , 0x00}, 
	{0x4d , 0x34},
	{0x4e , 0x04},
	{0x4e , 0x04},
	{0x4e , 0x02},
	{0x4e , 0x02},
	{0x4d , 0x44},
	{0x4e , 0x04},
	{0x4e , 0x04},
	{0x4e , 0x04},
	{0x4d , 0x53},
	{0x4e , 0x08},  //00
	{0x4e , 0x04},
	{0x4e , 0x04},
	{0x4e , 0x04},
	{0x4d , 0x63},
	{0x4e , 0x08}, 
	{0x4e , 0x04},
	{0x4d , 0x72},
	{0x4e , 0x20},
	{0x4d , 0x82},
	{0x4e , 0x20},
	{0x4f , 0x01}, 
	{0x50 , 0x88}, 
	/////////output//////// 
	{0xfe , 0x00},  
	{0xf1 , 0x07}, 
	{0xf2 , 0x01}, 
};

LOCAL SENSOR_REG_TAB_INFO_T s_GC0328_resolution_Tab_YUV[]=
{
        // COMMON INIT
        {ADDR_AND_LEN_OF_ARRAY(gc0328_YUV_640X480), 640, 480, 24, SENSOR_IMAGE_FORMAT_YUV422},

        // YUV422 PREVIEW 1	
        {PNULL, 0, 640, 480,24, SENSOR_IMAGE_FORMAT_YUV422},
        {PNULL, 0, 0, 0, 0, 0},
        {PNULL, 0, 0, 0, 0, 0},
        {PNULL, 0, 0, 0, 0, 0},

        // YUV422 PREVIEW 2 
        {PNULL, 0, 0, 0, 0, 0},
        {PNULL, 0, 0, 0, 0, 0},
        {PNULL, 0, 0, 0, 0, 0},
        {PNULL, 0, 0, 0, 0, 0}
};

LOCAL SENSOR_IOCTL_FUNC_TAB_T s_GC0328_ioctl_func_tab = 
{
        // Internal 
        PNULL,
        _GC0328_PowerOn,//PNULL,
        PNULL,
        GC0328_Identify,

        PNULL,			// write register
        PNULL,			// read  register	
        PNULL,
        PNULL,

        // External
        set_gc0328_ae_enable,
        PNULL,//set_hmirror_enable,
        PNULL,//set_vmirror_enable,

        set_brightness,
        set_contrast,
        set_sharpness,
        set_saturation,

        set_preview_mode,	
        set_image_effect,

        GC0328_BeforeSnapshot,	//	GC0328_BeforeSnapshot,
        GC0328_After_Snapshot,		//GC0328_After_Snapshot,

        _gc0328_flash,

        read_ev_value,
        write_ev_value,
        read_gain_value,
        write_gain_value,
        read_gain_scale,
        set_frame_rate,	
        PNULL,
        PNULL,
        set_gc0328_awb,
        PNULL,
        PNULL,
        set_gc0328_ev,
        PNULL,
        PNULL,
        PNULL,
        PNULL,
        PNULL,
        set_gc0328_anti_flicker,
        set_gc0328_video_mode,
        PNULL,
        PNULL
};

/**---------------------------------------------------------------------------*
 ** 						Global Variables								  *
 **---------------------------------------------------------------------------*/
 SENSOR_INFO_T g_GC0328_yuv_info =
{
        GC0328_I2C_ADDR_W,				// salve i2c write address
        GC0328_I2C_ADDR_R, 				// salve i2c read address

        0,								// bit0: 0: i2c register value is 8 bit, 1: i2c register value is 16 bit
        							// bit2: 0: i2c register addr  is 8 bit, 1: i2c register addr  is 16 bit
        							// other bit: reseved
        SENSOR_HW_SIGNAL_PCLK_P|\
        SENSOR_HW_SIGNAL_VSYNC_N|\
        SENSOR_HW_SIGNAL_HSYNC_P,		// bit0: 0:negative; 1:positive -> polarily of pixel clock
        							// bit2: 0:negative; 1:positive -> polarily of horizontal synchronization signal
        							// bit4: 0:negative; 1:positive -> polarily of vertical synchronization signal
        							// other bit: reseved											
        									
        // preview mode
        SENSOR_ENVIROMENT_NORMAL|\
        SENSOR_ENVIROMENT_NIGHT|\
        SENSOR_ENVIROMENT_SUNNY,		

        // image effect
        SENSOR_IMAGE_EFFECT_NORMAL|\
        SENSOR_IMAGE_EFFECT_BLACKWHITE|\
        SENSOR_IMAGE_EFFECT_RED|\
        SENSOR_IMAGE_EFFECT_GREEN|\
        SENSOR_IMAGE_EFFECT_BLUE|\
        SENSOR_IMAGE_EFFECT_YELLOW|\
        SENSOR_IMAGE_EFFECT_NEGATIVE|\
        SENSOR_IMAGE_EFFECT_CANVAS,

        // while balance mode
        0,

        7,								// bit[0:7]: count of step in brightness, contrast, sharpness, saturation
        							// bit[8:31] reseved

        SENSOR_LOW_PULSE_RESET,			// reset pulse level
        100,								// reset pulse width(ms)

        SENSOR_HIGH_LEVEL_PWDN,			// 1: high level valid; 0: low level valid	

        2,								// count of identify code
        {{0xf0, 0x9d},						// supply two code to identify sensor.
        {0xf0, 0x9d}},						// for Example: index = 0-> Device id, index = 1 -> version id	
        							
        SENSOR_AVDD_2800MV,				// voltage of avdd	

        640,							// max width of source image
        480,							// max height of source image
        "GC0328",						// name of sensor												

        SENSOR_IMAGE_FORMAT_YUV422,		// define in SENSOR_IMAGE_FORMAT_E enum,
        							// if set to SENSOR_IMAGE_FORMAT_MAX here, image format depent on SENSOR_REG_TAB_INFO_T
        SENSOR_IMAGE_PATTERN_YUV422_YUYV,	// pattern of input image form sensor;			

        s_GC0328_resolution_Tab_YUV,	// point to resolution table information structure
        &s_GC0328_ioctl_func_tab,		// point to ioctl function table
        	
        PNULL,							// information and table about Rawrgb sensor
        PNULL,							// extend information about sensor	
        SENSOR_AVDD_1800MV,                     // iovdd
        SENSOR_AVDD_1800MV,                      // dvdd
        2,
        0,
        0,
        2
};
/**---------------------------------------------------------------------------*
 ** 							Function  Definitions
 **---------------------------------------------------------------------------*/
LOCAL void GC0328_WriteReg( uint8_t  subaddr, uint8_t data )
{	
#ifndef	_USE_DSP_I2C_
        Sensor_WriteReg_8bits(subaddr, data);
#else
        DSENSOR_IICWrite((uint16_t)subaddr, (uint16_t)data);
#endif

        SENSOR_TRACE("SENSOR: GC0328_WriteReg reg/value(%x,%x) !!\n", subaddr, data);
}
LOCAL uint8_t GC0328_ReadReg( uint8_t  subaddr)
{
        uint8_t value = 0;

#ifndef	_USE_DSP_I2C_
        value = Sensor_ReadReg( subaddr);
#else
        value = (uint16_t)DSENSOR_IICRead((uint16_t)subaddr);
#endif

        SENSOR_TRACE("SENSOR: GC0328_ReadReg reg/value(%x,%x) !!\n", subaddr, value);
        return value;
}

LOCAL uint32_t _GC0328_PowerOn(uint32_t power_on)
{
	SENSOR_AVDD_VAL_E dvdd_val = g_GC0328_yuv_info.dvdd_val;
	SENSOR_AVDD_VAL_E avdd_val = g_GC0328_yuv_info.avdd_val;
	SENSOR_AVDD_VAL_E iovdd_val = g_GC0328_yuv_info.iovdd_val;
	BOOLEAN power_down = g_GC0328_yuv_info.power_down_level;
	BOOLEAN reset_level = g_GC0328_yuv_info.reset_pulse_level;
	//uint32_t reset_width=g_GC0328_yuv_info.reset_pulse_width;

	printk("SENSOR: _GC0328_Power_On_start(1:on, 0:off): %d \n", power_on);

	if (SENSOR_TRUE == power_on) {
		Sensor_PowerDown(power_down);
		// Open power
		//Sensor_SetVoltage(dvdd_val, avdd_val, iovdd_val);
		Sensor_SetVoltage(SENSOR_AVDD_CLOSED, avdd_val, iovdd_val);
		msleep(20);
		Sensor_SetMCLK(SENSOR_DEFALUT_MCLK);
		msleep(10);
		Sensor_PowerDown(!power_down);
		// Reset sensor
		Sensor_Reset(reset_level);
	} else {
		Sensor_PowerDown(power_down);
		Sensor_SetMCLK(SENSOR_DISABLE_MCLK);
		Sensor_SetVoltage(SENSOR_AVDD_CLOSED, SENSOR_AVDD_CLOSED,
				  SENSOR_AVDD_CLOSED);
	}
	printk("SENSOR: _GC0328_Power_On(1:on, 0:off): %d \n", power_on);
	return SENSOR_SUCCESS;
}
LOCAL uint32_t GC0328_Identify(uint32_t param)
{
#define GC0328_PID_VALUE	0x9d	
#define GC0328_PID_ADDR		0xf0
#define GC0328_VER_VALUE	0x9d	
#define GC0328_VER_ADDR		0xf0	

        uint32_t i;
        uint32_t nLoop;
        uint8_t ret;
        uint32_t err_cnt = 0;
        uint8_t reg[2] 	= {0xf0, 0xf0};
        uint8_t value[2] 	= {0x9d, 0x9d};

        SENSOR_TRACE("GC0328_Identify");
        for(i = 0; i<2; ) {
                nLoop = 1000;
                ret = GC0328_ReadReg(reg[i]);
		printk("GC0328_Identify ret = %d \n", ret);
                if( ret != value[i]) {
                        err_cnt++;
                        if(err_cnt>3) {
                                SENSOR_PRINT_HIGH("It is not GC0328\n");
                                return SENSOR_FAIL;
                        } else {
                                while(nLoop--);
                                continue;
                        }
                }
                err_cnt = 0;
                i++;
        }

        SENSOR_TRACE("GC0328_Identify: it is GC0328\n");
        return (uint32_t)SENSOR_SUCCESS;
}

LOCAL uint32_t set_gc0328_ae_enable(uint32_t enable)
{
        SENSOR_TRACE("set_gc0328_ae_enable: enable = %d\n", enable);
        return 0;
}

/******************************************************************************/
// Description: set brightness 
// Global resource dependence: 
// Author:
// Note:
//		level  must smaller than 8
/******************************************************************************/
SENSOR_REG_T gc0328_brightness_tab[][2]=
{
   	 {{0xd5, 0xd0}, {0xff, 0xff}},
	{{0xd5, 0xe0}, {0xff, 0xff}},
	{{0xd5, 0xf0}, {0xff, 0xff}},
	{{0xd5, 0xfd}, {0xff, 0xff}},
	{{0xd5, 0x20}, {0xff, 0xff}},
	{{0xd5, 0x30}, {0xff, 0xff}},
	{{0xd5, 0x40}, {0xff, 0xff}},
};

LOCAL uint32_t set_brightness(uint32_t level)
{
        uint16_t i;
        SENSOR_REG_T* sensor_reg_ptr = (SENSOR_REG_T*)gc0328_brightness_tab[level];

        if(level>6)
                return 0;
	
        for(i = 0; (0xFF != sensor_reg_ptr[i].reg_addr) && (0xFF != sensor_reg_ptr[i].reg_value); i++) {
                GC0328_WriteReg(sensor_reg_ptr[i].reg_addr, sensor_reg_ptr[i].reg_value);
        }
        SENSOR_TRACE("set_brightness: level = %d\n", level);
        return 0;
}

SENSOR_REG_T GC0328_ev_tab[][4]=
{   
         {{0xfe, 0x01}, {0x13, 0x38}, {0xfe, 0x00}, {0xff, 0xff}},
	    {{0xfe, 0x01}, {0x13, 0x40}, {0xfe, 0x00}, {0xff, 0xff}},
	    {{0xfe, 0x01}, {0x13, 0x48}, {0xfe, 0x00}, {0xff, 0xff}},
	    {{0xfe, 0x01}, {0x13, 0x4a}, {0xfe, 0x00}, {0xff, 0xff}},
	    {{0xfe, 0x01}, {0x13, 0x58}, {0xfe, 0x00}, {0xff, 0xff}},
	    {{0xfe, 0x01}, {0x13, 0x60}, {0xfe, 0x00}, {0xff, 0xff}},
	    {{0xfe, 0x01}, {0x13, 0x68}, {0xfe, 0x00}, {0xff, 0xff}},  
};

LOCAL uint32_t set_gc0328_ev(uint32_t level)
{
        uint16_t i; 
        SENSOR_REG_T* sensor_reg_ptr = (SENSOR_REG_T*)GC0328_ev_tab[level];

        if(level>6)
                return 0;

        for(i = 0; (0xFF != sensor_reg_ptr[i].reg_addr) ||(0xFF != sensor_reg_ptr[i].reg_value) ; i++) {
                GC0328_WriteReg(sensor_reg_ptr[i].reg_addr, sensor_reg_ptr[i].reg_value);
        }

        SENSOR_TRACE("SENSOR: set_ev: level = %d\n", level);
        return 0;
}

/******************************************************************************/
// Description: anti 50/60 hz banding flicker
// Global resource dependence: 
// Author:
// Note:
//		level  must smaller than 8
/******************************************************************************/
LOCAL uint32_t set_gc0328_anti_flicker(uint32_t param )
{
        switch (param) {
        case FLICKER_50HZ:
              GC0328_WriteReg(0xfe,0x00);
		GC0328_WriteReg(0x05,0x02);
		GC0328_WriteReg(0x06,0x2c);
		GC0328_WriteReg(0x07,0x00);
		GC0328_WriteReg(0x08,0xb8);
		GC0328_WriteReg(0xfe,0x01);
		GC0328_WriteReg(0x29,0x00);
		GC0328_WriteReg(0x2a,0x60);
		GC0328_WriteReg(0x2b,0x02);
		GC0328_WriteReg(0x2c,0xa0);
		GC0328_WriteReg(0x2d,0x03);
		GC0328_WriteReg(0x2e,0xc0);
		GC0328_WriteReg(0x2f,0x04);
		GC0328_WriteReg(0x30,0x80);
		GC0328_WriteReg(0x31,0x05);
		GC0328_WriteReg(0x32,0x40);
		GC0328_WriteReg(0xfe,0x00);
                break;
        case FLICKER_60HZ:
              GC0328_WriteReg(0xfe,0x00);
		GC0328_WriteReg(0x05,0x02);
		GC0328_WriteReg(0x06,0x4c);
		GC0328_WriteReg(0x07,0x00);
		GC0328_WriteReg(0x08,0x88);
		GC0328_WriteReg(0xfe,0x01);
		GC0328_WriteReg(0x29,0x00);
		GC0328_WriteReg(0x2a,0x4e);
		GC0328_WriteReg(0x2b,0x02);
		GC0328_WriteReg(0x2c,0x70);
		GC0328_WriteReg(0x2d,0x03);
		GC0328_WriteReg(0x2e,0x0c);
		GC0328_WriteReg(0x2f,0x03);
		GC0328_WriteReg(0x30,0xa8);
		GC0328_WriteReg(0x31,0x05);
		GC0328_WriteReg(0x32,0x2e);
		GC0328_WriteReg(0xfe,0x00);
                break;
        default:
              GC0328_WriteReg(0xfe,0x00);
		GC0328_WriteReg(0x05,0x02);
		GC0328_WriteReg(0x06,0x2c);
		GC0328_WriteReg(0x07,0x00);
		GC0328_WriteReg(0x08,0xb8);
		GC0328_WriteReg(0xfe,0x01);
		GC0328_WriteReg(0x29,0x00);
		GC0328_WriteReg(0x2a,0x60);
		GC0328_WriteReg(0x2b,0x02);
		GC0328_WriteReg(0x2c,0xa0);
		GC0328_WriteReg(0x2d,0x03);
		GC0328_WriteReg(0x2e,0xc0);
		GC0328_WriteReg(0x2f,0x04);
		GC0328_WriteReg(0x30,0x80);
		GC0328_WriteReg(0x31,0x05);
		GC0328_WriteReg(0x32,0x40);
		GC0328_WriteReg(0xfe,0x00);
                break;
        }
        return 0;
}

/******************************************************************************/
// Description: set video mode
// Global resource dependence: 
// Author:
// Note:
//		 
/******************************************************************************/
LOCAL uint32_t set_gc0328_video_mode(uint32_t mode)
{
   
      return 0;

    if(0 == mode)
               GC0328_WriteReg(0xec,0x20);
        else if(1 == mode)
               GC0328_WriteReg(0xec,0x00);
        SENSOR_TRACE("SENSOR: GC0328_ReadReg(0xec) = %x\n", GC0328_ReadReg(0xec));
        SENSOR_TRACE("SENSOR: set_video_mode: mode = %d\n", mode);
        return 0;
}
/******************************************************************************/
// Description: set wb mode 
// Global resource dependence: 
// Author:
// Note:
//		
/******************************************************************************/
SENSOR_REG_T GC0328_awb_tab[][5]=
{
		   	//AUTO
		   	{
				{0x77,0x57},
				{0x78,0x4d},
				{0x79,0x45},
				{0x42, 0xfe},
				{0xff, 0xff}
			},
			//INCANDESCENCE:
			{
				{0x42, 0xfd},    // Disable AWB
				{0x77, 0x48},
				{0x78, 0x40},
				{0x79, 0x5c},
				{0xff, 0xff}
			},
			//U30
			{
				{0x42, 0xfd},   // Disable AWB
				{0x77, 0x40},
				{0x78, 0x54},
				{0x79, 0x70},
				{0xff, 0xff}
			},
			//CWF  //
			{
				{0x42, 0xfd},   // Disable AWB
				{0x77, 0x40},
				{0x78, 0x54},
				{0x79, 0x70},
				{0xff, 0xff}
			},
			//FLUORESCENT:
			{
				{0x42, 0xfd},   // Disable AWB
				{0x77, 0x40},
				{0x78, 0x42},
				{0x79, 0x50},
				{0xff, 0xff} 
			},
			//SUN:
			{
				{0x42, 0xfd},   // Disable AWB
				{0x77, 0x50},
				{0x78, 0x45},
				{0x79, 0x40},
				{0xff, 0xff} 
			},
		            //CLOUD:
			{
				{0x42, 0xfd},   // Disable AWB
				{0x77, 0x5a},
				{0x78, 0x42},
				{0x79, 0x40},
				{0xff, 0xff}
			},
	};
	
LOCAL uint32_t set_gc0328_awb(uint32_t mode)
{
        uint8_t awb_en_value;
        uint16_t i;
        SENSOR_REG_T* sensor_reg_ptr = (SENSOR_REG_T*)GC0328_awb_tab[mode];
        
        awb_en_value = GC0328_ReadReg(0x22);	

        if(mode>6)
                return 0;


	    for(i = 0; (0xFF != sensor_reg_ptr[i].reg_addr) || (0xFF != sensor_reg_ptr[i].reg_value) ; i++)
	    {
	    	GC0328_WriteReg(sensor_reg_ptr[i].reg_addr, sensor_reg_ptr[i].reg_value);
	    }
        SENSOR_TRACE("SENSOR: set_awb_mode: mode = %d\n", mode);

        return 0;
}

SENSOR_REG_T gc0328_contrast_tab[][2]=
{
		{
			{0xd3,0x34}, 	{0xff,0xff},
		},

		{
			{0xd3,0x38}, 	{0xff,0xff},
		},

		{
			{0xd3,0x3d}, 	{0xff,0xff},
		},

		{
			{0xd3,0x42}, 	{0xff,0xff},
		},

		{
			{0xd3,0x44}, 	{0xff,0xff},
		},

		{
			{0xd3,0x48}, 	{0xff,0xff},
		},

		{
			{0xd3,0x50}, 	{0xff,0xff},
		},
};
LOCAL uint32_t set_contrast(uint32_t level)
{
        uint16_t i;
        SENSOR_REG_T* sensor_reg_ptr;

        sensor_reg_ptr = (SENSOR_REG_T*)gc0328_contrast_tab[level];

        if(level>6)
                return 0;

        for(i = 0; (0xFF != sensor_reg_ptr[i].reg_addr) && (0xFF != sensor_reg_ptr[i].reg_value); i++) {
                GC0328_WriteReg(sensor_reg_ptr[i].reg_addr, sensor_reg_ptr[i].reg_value);
        }

        SENSOR_TRACE("set_contrast: level = %d\n", level);
        return 0;
}
LOCAL uint32_t set_sharpness(uint32_t level)
{
        return 0;
}
LOCAL uint32_t set_saturation(uint32_t level)
{
        return 0;
}

/******************************************************************************/
// Description: set brightness 
// Global resource dependence: 
// Author:
// Note:
//		level  must smaller than 8
/******************************************************************************/
LOCAL uint32_t set_preview_mode(uint32_t preview_mode)
{
        SENSOR_TRACE("set_preview_mode: preview_mode = %d\n", preview_mode);

        set_gc0328_anti_flicker(0);
        switch (preview_mode) {
        case DCAMERA_ENVIRONMENT_NORMAL: 
               // GC0328_WriteReg(0xec,0x20);
                break;
        case DCAMERA_ENVIRONMENT_NIGHT:
               // GC0328_WriteReg(0xec,0x30);
                break;
        case DCAMERA_ENVIRONMENT_SUNNY:
              //  GC0328_WriteReg(0xec,0x10);
                break;
        default:
                break;
        }
        SENSOR_Sleep(10);
        return 0;
}
	
SENSOR_REG_T GC0328_image_effect_tab[][6]=	
{
		// effect normal
			{
				{0x43,0x00},
				{0x95,0x43},    //33  45
				{0xd3,0x42},
				{0xda,0x00},
				{0xdb,0x00},
				{0xff,0xff}
			},
			//effect BLACKWHITE
			{
				{0x43,0x02},
				{0xd3,0x40},
				{0xda,0x00},
				{0xdb,0x00},
				{0xff,0xff},
				{0xff,0xff}
			},
			// effect RED pink
			{
				{0x43,0x02},
				{0x95,0x88},
				{0xd3,0x40},
				{0xda,0x10},
				{0xdb,0x50},
				{0xff, 0xff},
			},
			// effect GREEN
			{
				{0x43,0x02},
				{0x95,0x88},
				{0xd3,0x40},
				{0xda,0xc0},
				{0xdb,0xc0},
				{0xff, 0xff},
			},
			// effect  BLUE
			{
				{0x43,0x02},
				{0xd3,0x40},
				{0xda,0x50},
				{0xdb,0xe0},
				{0xff, 0xff},
				{0xff, 0xff}
			},
			// effect  YELLOW
			{
				{0x43,0x02},
				{0x95,0x88},
				{0xd3,0x40},
				{0xda,0x80},
				{0xdb,0x20},
				{0xff, 0xff}
			},
			// effect NEGATIVE
			{
				{0x43,0x01},
				{0xd3,0x40},
				{0xda,0x00},
				{0xdb,0x00},
				{0xff, 0xff},
				{0xff, 0xff}
			},
			//effect ANTIQUE
			{
				{0x43,0x02},
				{0xd3,0x40},
				{0xda,0xd2},
				{0xdb,0x28},
				{0xff, 0xff},
				{0xff, 0xff}
			},
};
LOCAL uint32_t set_image_effect(uint32_t effect_type)
{
        uint16_t i;
        SENSOR_REG_T* sensor_reg_ptr = (SENSOR_REG_T*)GC0328_image_effect_tab[effect_type];
        if(effect_type>7)
                return 0;

        for(i = 0; (0xFF != sensor_reg_ptr[i].reg_addr) || (0xFF != sensor_reg_ptr[i].reg_value) ; i++) {
                Sensor_WriteReg_8bits(sensor_reg_ptr[i].reg_addr, sensor_reg_ptr[i].reg_value);
        }
        SENSOR_TRACE("-----------set_image_effect: effect_type = %d------------\n", effect_type);
        return 0;
}


LOCAL uint32_t GC0328_After_Snapshot(uint32_t param)
{
	printk("SENSOR: set flash on beforeSnapShot, g_flash_mode = %d \n",
	       g_flash_mode_en);
	if (g_flash_mode_en) {
		Sensor_SetFlash(0x10);
	}
	
	//Sensor_SetMCLK(24);
	
	//GC0328_WriteReg(0x41,GC0328_ReadReg(0x41) | 0xf7);
	//SENSOR_Sleep(200);
	return 0;
    
}

LOCAL uint32_t GC0328_BeforeSnapshot(uint32_t param)
{

    uint16_t shutter = 0x00;
    uint16_t temp_reg = 0x00;
    uint16_t temp_r =0x00;
    uint16_t temp_g =0x00;
    uint16_t temp_b =0x00;    
    BOOLEAN b_AEC_on;
    

    SENSOR_TRACE("GC0328_BeforeSnapshot ");   

	printk("SENSOR: set flash on beforeSnapShot, g_flash_mode = %d \n",
	       g_flash_mode_en);
	if (g_flash_mode_en) {
		//Sensor_SetFlash(0x11);  // high light
		Sensor_SetFlash(1);
	}
#if 0	
    	if(GC0328_ReadReg(0X41)  & 0x08 == 0x08)  //AEC on
    		b_AEC_on = SENSOR_TRUE;
    	else
    		b_AEC_on = SENSOR_FALSE;

	temp_reg = GC0328_ReadReg(0xdb);
	temp_r = GC0328_ReadReg(0xcd);
	temp_g = GC0328_ReadReg(0xce);
	temp_b = GC0328_ReadReg(0xcf);

	shutter = (GC0328_ReadReg(0x03)<<8)  | (GC0328_ReadReg(0x04)&0x00ff) ;
	shutter = shutter /2;

	if(b_AEC_on)
		GC0328_WriteReg(0x41,GC0328_ReadReg(0x41) & 0xc5); //0x01);
	SENSOR_Sleep(300); 

///12m
	Sensor_SetMCLK(12);
	
	GC0328_WriteReg(0x03,shutter/256);
	GC0328_WriteReg(0x04,shutter & 0x00ff);	
   	//SENSOR_TRACE("GC0328_BeforeSnapshot, temp_r=%x,temp_reg=%x, final = %x ",temp_r,temp_reg, temp_r*temp_reg/ 0x80);    

	temp_r = (temp_r*temp_reg) / 0x80;
	temp_g = (temp_g*temp_reg) / 0x80;
	temp_b = (temp_b*temp_reg) / 0x80;
	if(b_AEC_on)
	{
		GC0328_WriteReg(0xcd, temp_r);
		GC0328_WriteReg(0xce, temp_g);
		GC0328_WriteReg(0xcf , temp_b);
	}
   	//SENSOR_TRACE("GC0328_BeforeSnapshot, temp_r=%x,temp_g=%x, temp_b = %x ",temp_r,temp_g,temp_b);    

	SENSOR_Sleep(300); 
#endif
    	return 0;
    
}


LOCAL uint32_t _gc0328_flash(uint32_t param)
{
	g_flash_mode_en = param;
	Sensor_SetFlash(param);
	return 0;
}

LOCAL uint32_t read_ev_value(uint32_t value)
{
        return 0;
}
LOCAL uint32_t write_ev_value(uint32_t exposure_value)
{
        return 0;	
}
LOCAL uint32_t read_gain_value(uint32_t value)
{
        return 0;
}
LOCAL uint32_t write_gain_value(uint32_t gain_value)
{	
        return 0;
}
LOCAL uint32_t read_gain_scale(uint32_t value)
{
        return SENSOR_GAIN_SCALE;
}
LOCAL uint32_t set_frame_rate(uint32_t param)  
{
        //GC0328_WriteReg( 0xd8, uint8_t data );
        return 0;
}

struct sensor_drv_cfg sensor_gc0328 = {
        .sensor_pos = CONFIG_DCAM_SENSOR_POS_GC0328,
        .sensor_name = "gc0328",
        .driver_info = &g_GC0328_yuv_info,
};

static int __init sensor_gc0328_init(void)
{
        return dcam_register_sensor_drv(&sensor_gc0328);
}

subsys_initcall(sensor_gc0328_init);

