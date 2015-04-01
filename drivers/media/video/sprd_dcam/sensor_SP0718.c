/******************************************************************************
 ** File Name:    Sensor_SP0718.c                                         *
 ** Author:        superpix                                      	  *
 ** Date:           03/02/2011                                          *
 ** Copyright:    Spreadtrum All Rights Reserved.        *
 ** Description:   implementation of digital camera register interface       *
 ******************************************************************************

 ******************************************************************************
 **                        Edit History                                       *
 ** ------------------------------------------------------------------------- *
 ** DATE           NAME             DESCRIPTION                               *
 **                                  *
 ******************************************************************************/

#include "common/sensor_cfg.h"
#include "common/sensor_drv.h"
#include <linux/delay.h>
#include "common/sensor.h"
#include "common/jpeg_exif_header_k.h"

/**---------------------------------------------------------------------------*
 **                         Const variables                                   *
 **---------------------------------------------------------------------------*/
#define SP0718_I2C_ADDR_W    			0x21//0x42
#define SP0718_I2C_ADDR_R    			0x21//0x43
#define SP0718_I2C_ACK				0x0

typedef enum
{
    DCAMERA_FLICKER_50HZ = 0,
    DCAMERA_FLICKER_60HZ,
    FLICKER_MAX
}DCAMERA_FLICKER;

LOCAL uint32_t	s_sensor_mclk		 = 0;
LOCAL uint32_t	Antiflicker		 = DCAMERA_FLICKER_50HZ;
LOCAL uint32_t	SP0718_awb_mode	 	 = 0;      //whl130802

#define  SP0718_NORMAL_Y0ffset  	0x20
#define  SP0718_LOWLIGHT_Y0ffset  	0x25
//AE target
#define  SP0718_P1_0xeb  		0x78
#define  SP0718_P1_0xec  		0x6c
//HEQ
#define  SP0718_P1_0x10  		0x00//outdoor
#define  SP0718_P1_0x14  		0x20
#define  SP0718_P1_0x11  		0x00//nr
#define  SP0718_P1_0x15  		0x18
#define  SP0718_P1_0x12  		0x00//dummy
#define  SP0718_P1_0x16  		0x10
#define  SP0718_P1_0x13  		0x00//low
#define  SP0718_P1_0x17  		0x00
//AWB pre gain
#define  SP0718_P2_0x26  		0xc8
#define  SP0718_P2_0x27  		0xb6

/**---------------------------------------------------------------------------*
 **                         Macro define                                      *
 **---------------------------------------------------------------------------*/
#define    SP0718_DELAY_MS(ms)            {\
                                              uint32_t end = SCI_GetTickCount() + ms;\
                                              while (SCI_GetTickCount() < end)\
                                              ;\
                                          }

/**---------------------------------------------------------------------------*
 **                         Const variables                                   *
 **---------------------------------------------------------------------------*/
//  give the parameters

/**---------------------------------------------------------------------------*
 **                     Local Function Prototypes                             *
 **---------------------------------------------------------------------------*/
//LOCAL uint32_t SP0718_Power_On(uint32_t power_on);

#if 0
#ifdef DOUBLE_CAMERA_SUPPORT
/*__align(4)*/ SENSOR_REG_T SP0718_YVU_640X480[] = 
#else
/*__align(4)*/ const SENSOR_REG_T SP0718_YVU_640X480[] = 
#endif
#endif
SENSOR_REG_T SP0718_YVU_640X480[]=
{
    {0xfd,0x00},
    {0x1C,0x00},
    {0x31,0x10},
    {0x27,0xb3},//0xb3	//2x gain
    {0x1b,0x17},
    {0x26,0xaa},
    {0x37,0x02},
    {0x28,0x8f},
    {0x1a,0x73},
    {0x1e,0x1b},
    {0x21,0x06},  //blackout voltage
    {0x22,0x2a},  //colbias
    {0x0f,0x3f},
    {0x10,0x3e},
    {0x11,0x00},
    {0x12,0x01},
    {0x13,0x3f},
    {0x14,0x04},
    {0x15,0x30},
    {0x16,0x31},
    {0x17,0x01},
    {0x69,0x31},
    {0x6a,0x2a},
    {0x6b,0x33},
    {0x6c,0x1a},
    {0x6d,0x32},
    {0x6e,0x28},
    {0x6f,0x29},
    {0x70,0x34},
    {0x71,0x18},
    {0x36,0x00},//02 delete badframe
    {0xfd,0x01},
    {0x5d,0x51},//position
    {0xf2,0x19},
    //Blacklevel
    {0x1f,0x10},
    {0x20,0x1f},
    //pregain 
    {0xfd,0x02},
    {0x00,0x88},
    {0x01,0x88},
    //SI15_SP0718 24M 50Hz 15-8fps 
    //ae setting
    {0xfd,0x00},
    {0x03,0x01},
    {0x04,0xce},
    {0x06,0x00},
    {0x09,0x02},
    {0x0a,0xc4},
    {0xfd,0x01},
    {0xef,0x4d},
    {0xf0,0x00},
    {0x02,0x0c},
    {0x03,0x01},
    {0x06,0x47},
    {0x07,0x00},
    {0x08,0x01},
    {0x09,0x00},
    //Status   
    {0xfd,0x02},
    {0xbe,0x9c},
    {0xbf,0x03},
    {0xd0,0x9c},
    {0xd1,0x03},
    {0xfd,0x01},
    {0x5b,0x03},
    {0x5c,0x9c},
    //rpc
    {0xfd,0x01},
    {0xe0,0x40},////24//4c//48//4c//44//4c//3e//3c//3a//38//rpc_1base_max
    {0xe1,0x30},////24//3c//38//3c//36//3c//30//2e//2c//2a//rpc_2base_max
    {0xe2,0x2e},////24//34//30//34//2e//34//2a//28//26//26//rpc_3base_max
    {0xe3,0x2a},////24//2a//2e//2c//2e//2a//2e//26//24//22//rpc_4base_max
    {0xe4,0x2a},////24//2a//2e//2c//2e//2a//2e//26//24//22//rpc_5base_max
    {0xe5,0x28},////24//2c//2a//2c//28//2c//24//22//20//rpc_6base_max
    {0xe6,0x28},////24//2c//2a//2c//28//2c//24//22//20//rpc_7base_max
    {0xe7,0x26},////24//2a//28//2a//26//2a//22//20//20//1e//rpc_8base_max
    {0xe8,0x26},////24//2a//28//2a//26//2a//22//20//20//1e//rpc_9base_max
    {0xe9,0x26},////24//2a//28//2a//26//2a//22//20//20//1e//rpc_10base_max
    {0xea,0x26},////24//28//26//28//24//28//20//1f//1e//1d//rpc_11base_max
    {0xf3,0x26},////24//28//26//28//24//28//20//1f//1e//1d//rpc_12base_max
    {0xf4,0x26},////24//28//26//28//24//28//20//1f//1e//1d//rpc_13base_max
    //ae gain &status
    {0xfd,0x01},
    {0x04,0xe0},//rpc_max_indr
    {0x05,0x26},//1e//rpc_min_indr 
    {0x0a,0xa0},//rpc_max_outdr
    {0x0b,0x26},//rpc_min_outdr
    {0x5a,0x40},//dp rpc   
    {0xfd,0x02}, 
    {0xbc,0xa0},//rpc_heq_low
    {0xbd,0x80},//rpc_heq_dummy
    {0xb8,0x80},//mean_normal_dummy
    {0xb9,0x90},//mean_dummy_normal
    //ae target
    {0xfd,0x01}, 
    {0xeb,SP0718_P1_0xeb},//78 
    {0xec,SP0718_P1_0xec},//78
    {0xed,0x0a},	
    {0xee,0x10},
    //lsc       
    {0xfd,0x01},
    {0x26,0x30},
    {0x27,0x2c},
    {0x28,0x07},
    {0x29,0x08},
    {0x2a,0x40},
    {0x2b,0x03},
    {0x2c,0x00},
    {0x2d,0x00},
		      
    {0xa1,0x24},
    {0xa2,0x27},
    {0xa3,0x27},
    {0xa4,0x2b},
    {0xa5,0x1c},
    {0xa6,0x1a},
    {0xa7,0x1a},
    {0xa8,0x1a},
    {0xa9,0x18},
    {0xaa,0x1c},
    {0xab,0x17},
    {0xac,0x17},
    {0xad,0x08},
    {0xae,0x08},
    {0xaf,0x08},
    {0xb0,0x00},
    {0xb1,0x00},
    {0xb2,0x00},
    {0xb3,0x00},
    {0xb4,0x00},
    {0xb5,0x02},
    {0xb6,0x06},
    {0xb7,0x00},
    {0xb8,0x00},		       
    //DP       
    {0xfd,0x01},
    {0x48,0x09},
    {0x49,0x99},		        
    //awb       
    {0xfd,0x01},
    {0x32,0x05},
    {0xfd,0x00},
    {0xe7,0x03},
    {0xfd,0x02},
    {0x26,0xc8},
    {0x27,0xb6},
    {0xfd,0x00},
    {0xe7,0x00},
    {0xfd,0x02},
    {0x1b,0x80},
    {0x1a,0x80},
    {0x18,0x26},
    {0x19,0x28},
    {0xfd,0x02},
    {0x2a,0x00},
    {0x2b,0x08},
    {0x28,0xef},//0xa0//f8
    {0x29,0x20},
    //d65 90  e2 93
    {0x66,0x42},//0x59//0x60////0x58//4e//0x48
    {0x67,0x62},//0x74//0x70//0x78//6b//0x69
    {0x68,0xee},//0xd6//0xe3//0xd5//cb//0xaa
    {0x69,0x18},//0xf4//0xf3//0xf8//ed
    {0x6a,0xa6},//0xa5
    //indoor 91
    {0x7c,0x3b},//0x45//30//41//0x2f//0x44
    {0x7d,0x5b},//0x70//60//55//0x4b//0x6f
    {0x7e,0x15},//0a//0xed
    {0x7f,0x39},//23//0x28
    {0x80,0xaa},//0xa6
    //cwf   92 
    {0x70,0x3e},//0x38//41//0x3b
    {0x71,0x59},//0x5b//5f//0x55
    {0x72,0x31},//0x30//22//0x28
    {0x73,0x4f},//0x54//44//0x45
    {0x74,0xaa},
    //tl84  93 
    {0x6b,0x1b},//0x18//11
    {0x6c,0x3a},//0x3c//25//0x2f
    {0x6d,0x3e},//0x3a//35
    {0x6e,0x59},//0x5c//46//0x52
    {0x6f,0xaa},
    //f    94
    {0x61,0xea},//0x03//0x00//f4//0xed
    {0x62,0x03},//0x1a//0x25//0f//0f
    {0x63,0x6a},//0x62//0x60//52//0x5d
    {0x64,0x8a},//0x7d//0x85//70//0x75//0x8f
    {0x65,0x6a},//0xaa//6a
		      
    {0x75,0x80},
    {0x76,0x20},
    {0x77,0x00},
    {0x24,0x25},

    {0x20,0xd8},
    {0x21,0xa3},//82//
    {0x22,0xd0},//e3//bc
    {0x23,0x86},
    //outdoor r\b range
    {0x78,0xc3},//d8
    {0x79,0xba},//82
    {0x7a,0xa6},//e3
    {0x7b,0x99},//86
    //skin 
    {0x08,0x15},//
    {0x09,0x04},//
    {0x0a,0x20},//
    {0x0b,0x12},//
    {0x0c,0x27},//
    {0x0d,0x06},//
    {0x0e,0x63},//
    //wt th
    {0x3b,0x10},
    //gw
    {0x31,0x60},
    {0x32,0x60},
    {0x33,0xc0},
    {0x35,0x6f},
    // sharp
    {0xfd,0x02},
    {0xde,0x0f},
    {0xd2,0x02},
    {0xd3,0x06},
    {0xd4,0x06},
    {0xd5,0x06},
    {0xd7,0x20},//10//2x
    {0xd8,0x30},//24//1A//4x
    {0xd9,0x38},//28//8x
    {0xda,0x38},//16x
    {0xdb,0x08},//
    {0xe8,0x58},//48//
    {0xe9,0x48},
    {0xea,0x30},
    {0xeb,0x20},
    {0xec,0x48},//60//80
    {0xed,0x48},//50//60
    {0xee,0x30},
    {0xef,0x20},
    
    {0xf3,0x50},
    {0xf4,0x10},
    {0xf5,0x10},
    {0xf6,0x10},
    //dns       
    {0xfd,0x01},
    {0x64,0x44},//沿
    {0x65,0x22},
    {0x6d,0x04},//8//
    {0x6e,0x06},//8
    {0x6f,0x10},
    {0x70,0x10},
    {0x71,0x08},//0d//
    {0x72,0x12},//1b
    {0x73,0x1c},//20
    {0x74,0x24},
    {0x75,0x44},//[7:4]
    {0x76,0x02},//46
    {0x77,0x02},//33
    {0x78,0x02},
    {0x81,0x10},//18//2x//
    {0x82,0x20},//30//4x
    {0x83,0x30},//40//8x
    {0x84,0x48},//50//16x
    {0x85,0x0c},//12/8+reg0x81
    {0xfd,0x02},
    {0xdc,0x0f},	       
    //gamma    
    {0xfd,0x01},
    {0x8b,0x00},//00//00     
    {0x8c,0x0a},//0c//09     
    {0x8d,0x16},//19//17     
    {0x8e,0x1f},//25//24     
    {0x8f,0x2a},//30//33     
    {0x90,0x3c},//44//47     
    {0x91,0x4e},//54//58     
    {0x92,0x5f},//61//64     
    {0x93,0x6c},//6d//70     
    {0x94,0x82},//80//81     
    {0x95,0x94},//92//8f     
    {0x96,0xa6},//a1//9b     
    {0x97,0xb2},//ad//a5     
    {0x98,0xbf},//ba//b0     
    {0x99,0xc9},//c4//ba     
    {0x9a,0xd1},//cf//c4     
    {0x9b,0xd8},//d7//ce     
    {0x9c,0xe0},//e0//d7     
    {0x9d,0xe8},//e8//e1     
    {0x9e,0xef},//ef//ea     
    {0x9f,0xf8},//f7//f5     
    {0xa0,0xff},//ff//ff     
    //CCM      
    {0xfd,0x02},
    {0x15,0xd0},//b>th
    {0x16,0x95},//r<th
    //!F        
    {0xa0,0x80},//80
    {0xa1,0x00},//00
    {0xa2,0x00},//00
    {0xa3,0x00},//06
    {0xa4,0x8c},//8c
    {0xa5,0xf4},//ed
    {0xa6,0x0c},//0c
    {0xa7,0xf4},//f4
    {0xa8,0x80},//80
    {0xa9,0x00},//00
    {0xaa,0x30},//30
    {0xab,0x0c},//0c 
    //F        
    {0xac,0x8c},
    {0xad,0xf4},
    {0xae,0x00},
    {0xaf,0xed},
    {0xb0,0x8c},
    {0xb1,0x06},
    {0xb2,0xf4},
    {0xb3,0xf4},
    {0xb4,0x99},
    {0xb5,0x0c},
    {0xb6,0x03},
    {0xb7,0x0f},        
    //sat u     
    {0xfd,0x01},
    {0xd3,0x9c},//0x88//50
    {0xd4,0x98},//0x88//50
    {0xd5,0x8c},//50
    {0xd6,0x84},//50
    //sat v   
    {0xd7,0x9c},//0x88//50
    {0xd8,0x98},//0x88//50
    {0xd9,0x8c},//50
    {0xda,0x84},//50
    //auto_sat  
    {0xdd,0x30},
    {0xde,0x10},
    {0xd2,0x01},//autosa_en
    {0xdf,0xff},//a0//y_mean_th        
    //uv_th     
    {0xfd,0x01},
    {0xc2,0xaa},
    {0xc3,0xaa},
    {0xc4,0x66},
    {0xc5,0x66}, 
    //heq
    {0xfd,0x01},
    {0x0f,0xff},
    {0x10,SP0718_P1_0x10}, //out
    {0x14,SP0718_P1_0x14}, 
    {0x11,SP0718_P1_0x11}, //nr
    {0x15,SP0718_P1_0x15},  
    {0x12,SP0718_P1_0x12}, //dummy
    {0x16,SP0718_P1_0x16}, 
    {0x13,SP0718_P1_0x13}, //low 	
    {0x17,SP0718_P1_0x17},   	

    {0xfd,0x01},
    {0xcd,0x20},
    {0xce,0x1f},
    {0xcf,0x20},
    {0xd0,0x55},  
    //auto 
    {0xfd,0x01},
    {0xfb,0x33},
    //{0x32,0x15},
    {0x33,0xff},
    {0x34,0xe7},

    {0x35,0x40},	  	
 };

/**---------------------------------------------------------------------------*
 **                         Function Definitions                              *
 **---------------------------------------------------------------------------*/
LOCAL void SP0718_WriteReg( uint8_t  subaddr, uint8_t data )
{
#ifndef	_USE_DSP_I2C_
    Sensor_WriteReg_8bits(subaddr, data);
#else
    DSENSOR_IICWrite((uint16_t)subaddr, (uint16_t)data);
#endif
    SENSOR_TRACE("SENSOR: SP0718_WriteReg reg/value(%x,%x) !!", subaddr, data);
}

LOCAL uint8_t SP0718_ReadReg( uint8_t  subaddr)
{
    uint8_t value = 0;
	
#ifndef	_USE_DSP_I2C_
    value = Sensor_ReadReg( subaddr);
    SENSOR_TRACE( "SP0718 read ret is %X=%X",subaddr,value );
#else
    value = (uint16_t)DSENSOR_IICRead((uint16_t)subaddr);
#endif
    return value;
}

/******************************************************************************/
// Description: sensor probe function
// Author:     benny.zou
// Input:      none
// Output:     result
// Return:     0           successful
//             others      failed
// Note:       this function only to check whether sensor is work, not identify 
//              whitch it is!!
/******************************************************************************/
LOCAL uint32_t SP0718_Identify(void)
{
    uint8_t reg[2]    = {0x02, 0x02};
    uint8_t value[2]  = {0x71,0x71};
    uint8_t ret       = 0;
    uint32_t i;
    uint8_t err_cnt = 0;
    uint32_t nLoop = 1000;

    SP0718_WriteReg(0xfd,0x00);//change register page to 0 page
    for(i = 0; i<2; )
    {
        nLoop = 1000;
        ret = SP0718_ReadReg(reg[i]);    
	SENSOR_TRACE( "SP0718 read ret is %X",ret );
		
        if( ret != value[i])
        {
            err_cnt++;
            if(err_cnt>3)
            {
		SENSOR_TRACE( "SP0718 Fail" );
		return 0xFF;
            }
            else
            {
                //Masked by frank.yang,msleep() will cause a  Assert when called in boot precedure
                //msleep(10);
                while(nLoop--);
                continue;
            }
        }
        err_cnt = 0;
        i++;
    }
    SENSOR_TRACE( "SP0718 succ" );
    return (uint32_t)SENSOR_SUCCESS;
}

/*__align(4)*/ const SENSOR_REG_T SP0718_brightness_tab[][3]=
{
    {
	{0xfd,0x01},
	{0xdb,0xd0},//level -3
	{0xff,0xff},
    },

    {
	{0xfd,0x01},
	{0xdb,0xe0},//level -2
	{0xff,0xff},
    },

    {
	{0xfd,0x01},
	{0xdb,0xf0},//level -1
	{0xff,0xff},
    },

    {
	{0xfd,0x01},
	{0xdb,0x00},//level 0
	{0xff,0xff},
    },

    {
	{0xfd,0x01},
	{0xdb,0x10},//level +1
	{0xff,0xff},
    },

    {
	{0xfd,0x01},
	{0xdb,0x20},//level +2
	{0xff,0xff},
    },
    {		
	{0xfd,0x01},
	{0xdb,0x30},//level +3
	{0xff,0xff},
    },
};

LOCAL uint32_t set_brightness(uint32_t level)
{ 
    uint16_t i;
    SENSOR_REG_T* sensor_reg_ptr = (SENSOR_REG_T*)SP0718_brightness_tab[level];

    SENSOR_ASSERT(PNULL != sensor_reg_ptr);
    
    for(i = 0; (0xFF != sensor_reg_ptr[i].reg_addr) || (0xFF != sensor_reg_ptr[i].reg_value) ; i++)
    {
        SP0718_WriteReg(sensor_reg_ptr[i].reg_addr, sensor_reg_ptr[i].reg_value);
    } 
    return 0;
}

/*__align(4)*/ const SENSOR_REG_T SP0718_contrast_tab[][10]=
{
    {
	//level -3	
	{0xfd,0x01},
	{0x10,SP0718_P1_0x10}, //out
	{0x14,SP0718_P1_0x14}, 
	{0x11,SP0718_P1_0x11}, //nr
	{0x15,SP0718_P1_0x15},  
	{0x12,SP0718_P1_0x12}, //dummy
	{0x16,SP0718_P1_0x16}, 
	{0x13,SP0718_P1_0x13}, //low 	
	{0x17,SP0718_P1_0x17},  
	{0xff,0xff}
    },

    {
	//level -2
	{0xfd,0x01},
	{0x10,SP0718_P1_0x10}, //out
	{0x14,SP0718_P1_0x14}, 
	{0x11,SP0718_P1_0x11}, //nr
	{0x15,SP0718_P1_0x15},  
	{0x12,SP0718_P1_0x12}, //dummy
	{0x16,SP0718_P1_0x16}, 
	{0x13,SP0718_P1_0x13}, //low 	
	{0x17,SP0718_P1_0x17},  
	{0xff,0xff}
    },

    {
	//level -1
	{0xfd,0x01},
	{0x10,SP0718_P1_0x10}, //out
	{0x14,SP0718_P1_0x14}, 
	{0x11,SP0718_P1_0x11}, //nr
	{0x15,SP0718_P1_0x15},  
	{0x12,SP0718_P1_0x12}, //dummy
	{0x16,SP0718_P1_0x16}, 
	{0x13,SP0718_P1_0x13}, //low 	
	{0x17,SP0718_P1_0x17},  
	{0xff,0xff}
    },

    {
	//level 0
	{0xfd,0x01},
	{0x10,SP0718_P1_0x10}, //out
	{0x14,SP0718_P1_0x14}, 
	{0x11,SP0718_P1_0x11}, //nr
	{0x15,SP0718_P1_0x15},  
	{0x12,SP0718_P1_0x12}, //dummy
	{0x16,SP0718_P1_0x16}, 
	{0x13,SP0718_P1_0x13}, //low 	
	{0x17,SP0718_P1_0x17},  
	{0xff,0xff}
    },

    {
	//level +1
	{0xfd,0x01},
	{0x10,SP0718_P1_0x10+0x08}, //out
	{0x14,SP0718_P1_0x14+0x08}, 
	{0x11,SP0718_P1_0x11+0x08}, //nr
	{0x15,SP0718_P1_0x15+0x08},  
	{0x12,SP0718_P1_0x12+0x08}, //dummy
	{0x16,SP0718_P1_0x16+0x08}, 
	{0x13,SP0718_P1_0x13+0x08}, //low 	
	{0x17,SP0718_P1_0x17+0x08},  
	{0xff,0xff}
    },

    {
	//level +2
	{0xfd,0x01},
	{0x10,SP0718_P1_0x10+0x10}, //out
	{0x14,SP0718_P1_0x14+0x10}, 
	{0x11,SP0718_P1_0x11+0x10}, //nr
	{0x15,SP0718_P1_0x15+0x10},  
	{0x12,SP0718_P1_0x12+0x10}, //dummy
	{0x16,SP0718_P1_0x16+0x10}, 
	{0x13,SP0718_P1_0x13+0x10}, //low 	
	{0x17,SP0718_P1_0x17+0x10},  
	{0xff,0xff}
    },

    {
	//level +3
	{0xfd,0x01},
	{0x10,SP0718_P1_0x10+0x18}, //out
	{0x14,SP0718_P1_0x14+0x18}, 
	{0x11,SP0718_P1_0x11+0x18}, //nr
	{0x15,SP0718_P1_0x15+0x18},  
	{0x12,SP0718_P1_0x12+0x18}, //dummy
	{0x16,SP0718_P1_0x16+0x18}, 
	{0x13,SP0718_P1_0x13+0x18}, //low 	
	{0x17,SP0718_P1_0x17+0x18},  
	{0xff,0xff}
    }, 
};

LOCAL uint32_t set_contrast(uint32_t level)
{
    uint16_t i;    
    
    SENSOR_REG_T* sensor_reg_ptr = (SENSOR_REG_T*)SP0718_contrast_tab[level];

    SENSOR_ASSERT(PNULL != sensor_reg_ptr);
    
    for(i = 0; (0xFF != sensor_reg_ptr[i].reg_addr) || (0xFF != sensor_reg_ptr[i].reg_value) ; i++)
    {
        SP0718_WriteReg(sensor_reg_ptr[i].reg_addr, sensor_reg_ptr[i].reg_value);
    }
    return 0;
}


LOCAL uint32_t Sensor_Write_Regs( SENSOR_REG_T* sensor_reg_ptr )
{
    uint32_t i;
    
    for(i = 0; (0xFF != sensor_reg_ptr[i].reg_addr) && (0xFF != sensor_reg_ptr[i].reg_value) ; i++)
    {
        SP0718_WriteReg(sensor_reg_ptr[i].reg_addr, sensor_reg_ptr[i].reg_value);
    }
    return 0;
}


/*__align(4)*/ const SENSOR_REG_T SP0718_mode_tab[][4]=
{
#if 1
    {    
	{0xfd,0x00},
	{0xfd,0x00},
	{0xfd,0x00}, 
	{0xFF,0xFF},
    }
#else
//capture preview daylight
    {     
	{0xfd,0x00},
	{0xb2,SP0718_NORMAL_Y0ffset}, 
	{0xb3,0x1f}, //lum_set
	{0xFF,0xFF},	//数组结束标志
    },
//capture preview night
    {	    
	{0xfd,0x00},
	{0xb2,SP0718_LOWLIGHT_Y0ffset}, 
	{0xb3,0x1f}, //lum_set
	{0xFF,0xFF},	//数组结束标志 
    }
#endif
};  

////whl130802 add 
LOCAL uint32_t SP0718_awb_enable( )
{
    uint16_t awb_value=0;
    SP0718_WriteReg(0xfd,0x01);
    awb_value = SP0718_ReadReg(0x32);

    if(0x00==SP0718_awb_mode)
    {
	awb_value |= 0x10;
    }
    else if(0x01==SP0718_awb_mode)
    {
	awb_value &= 0xef;  
    }	

    SP0718_WriteReg(0x32, awb_value);
    return 0;
}

LOCAL uint32_t set_preview_mode(uint32_t preview_mode)
{
    switch (preview_mode)
    {
	case DCAMERA_ENVIRONMENT_NORMAL: 
	case DCAMERA_ENVIRONMENT_SUNNY:
	{
	    SP0718_WriteReg(0xfd,0x01);
	    SP0718_WriteReg(0xcd,SP0718_NORMAL_Y0ffset);
	    SP0718_WriteReg(0xce,0x1f);
	    if(Antiflicker== DCAMERA_FLICKER_50HZ)
	    {
		SENSOR_TRACE("normal mode 50hz\r\n");
		//capture preview daylight 24M 50hz 15-8FPS   
		SP0718_WriteReg(0xfd,0x00);
		SP0718_WriteReg(0x03,0x01);
		SP0718_WriteReg(0x04,0xce);
		SP0718_WriteReg(0x06,0x00);
		SP0718_WriteReg(0x09,0x02);
		SP0718_WriteReg(0x0a,0xc4);
		SP0718_WriteReg(0xfd,0x01);
		SP0718_WriteReg(0xef,0x4d);
		SP0718_WriteReg(0xf0,0x00);
		SP0718_WriteReg(0x02,0x0c);
		SP0718_WriteReg(0x03,0x01);
		SP0718_WriteReg(0x06,0x47);
		SP0718_WriteReg(0x07,0x00);
		SP0718_WriteReg(0x08,0x01);
		SP0718_WriteReg(0x09,0x00);
		//Status                 
		SP0718_WriteReg(0xfd,0x02);
		SP0718_WriteReg(0xbe,0x9c);
		SP0718_WriteReg(0xbf,0x03);
		SP0718_WriteReg(0xd0,0x9c);
		SP0718_WriteReg(0xd1,0x03);
		SP0718_WriteReg(0xfd,0x01);
		SP0718_WriteReg(0x5b,0x03);
		SP0718_WriteReg(0x5c,0x9c);
		SP0718_WriteReg(0xfd,0x01);
		SP0718_awb_enable();  //whl130802
	    }else{
		SENSOR_TRACE("normal mode 60hz\r\n");
			//capture preview daylight 24M 60hz 15-8FPS    
		SP0718_WriteReg(0xfd,0x00);
		SP0718_WriteReg(0x03,0x01);
		SP0718_WriteReg(0x04,0x80);
		SP0718_WriteReg(0x06,0x00);
		SP0718_WriteReg(0x09,0x02);
		SP0718_WriteReg(0x0a,0xc9);
		SP0718_WriteReg(0xfd,0x01);
		SP0718_WriteReg(0xef,0x40);
		SP0718_WriteReg(0xf0,0x00);
		SP0718_WriteReg(0x02,0x0f);
		SP0718_WriteReg(0x03,0x01);
		SP0718_WriteReg(0x06,0x3a);
		SP0718_WriteReg(0x07,0x00);
		SP0718_WriteReg(0x08,0x01);
		SP0718_WriteReg(0x09,0x00);
		//Status                   
		SP0718_WriteReg(0xfd,0x02);
		SP0718_WriteReg(0xbe,0xc0);
		SP0718_WriteReg(0xbf,0x03);
		SP0718_WriteReg(0xd0,0xc0);
		SP0718_WriteReg(0xd1,0x03);
		SP0718_WriteReg(0xfd,0x01);
		SP0718_WriteReg(0x5b,0x03);
		SP0718_WriteReg(0x5c,0xc0);
		SP0718_WriteReg(0xfd,0x01);
		SP0718_awb_enable(); //whl130802
	    }
	    break;
	}	
	case DCAMERA_ENVIRONMENT_NIGHT:
	{
	    SP0718_WriteReg(0xfd,0x01);
	    SP0718_WriteReg(0xcd,SP0718_LOWLIGHT_Y0ffset);
	    SP0718_WriteReg(0xce,0x1f);
	    if(Antiflicker== DCAMERA_FLICKER_50HZ)
	    {
		SENSOR_TRACE("night mode 50hz\r\n");
		//capture preview night 24M 50hz 15-6FPS  
		SP0718_WriteReg(0xfd,0x00);
		SP0718_WriteReg(0x03,0x01);
		SP0718_WriteReg(0x04,0xce);
		SP0718_WriteReg(0x06,0x00);
		SP0718_WriteReg(0x09,0x02);
		SP0718_WriteReg(0x0a,0xc4);
		SP0718_WriteReg(0xfd,0x01);
		SP0718_WriteReg(0xef,0x4d);
		SP0718_WriteReg(0xf0,0x00);
		SP0718_WriteReg(0x02,0x10);
		SP0718_WriteReg(0x03,0x01);
		SP0718_WriteReg(0x06,0x47);
		SP0718_WriteReg(0x07,0x00);
		SP0718_WriteReg(0x08,0x01);
		SP0718_WriteReg(0x09,0x00);
		//Statu                  
		SP0718_WriteReg(0xfd,0x02);
		SP0718_WriteReg(0xbe,0xd0);
		SP0718_WriteReg(0xbf,0x04);
		SP0718_WriteReg(0xd0,0xd0);
		SP0718_WriteReg(0xd1,0x04);
		SP0718_WriteReg(0xfd,0x01);
		SP0718_WriteReg(0x5b,0x04);
		SP0718_WriteReg(0x5c,0xd0);
		SP0718_WriteReg(0xfd,0x01);
		SP0718_awb_enable();  //whl130802
	    }else{
		SENSOR_TRACE("night mode 60hz\r\n");
		//capture preview night 24M 60hz 15-6FPS 
		SP0718_WriteReg(0xfd,0x00);
		SP0718_WriteReg(0x03,0x01);
		SP0718_WriteReg(0x04,0x80);
		SP0718_WriteReg(0x06,0x00);
		SP0718_WriteReg(0x09,0x02);
		SP0718_WriteReg(0x0a,0xc9);
		SP0718_WriteReg(0xfd,0x01);
		SP0718_WriteReg(0xef,0x40);
		SP0718_WriteReg(0xf0,0x00);
		SP0718_WriteReg(0x02,0x14);
		SP0718_WriteReg(0x03,0x01);
		SP0718_WriteReg(0x06,0x3a);
		SP0718_WriteReg(0x07,0x00);
		SP0718_WriteReg(0x08,0x01);
		SP0718_WriteReg(0x09,0x00);
	//Status                 
		SP0718_WriteReg(0xfd,0x02);
		SP0718_WriteReg(0xbe,0x00);
		SP0718_WriteReg(0xbf,0x05);
		SP0718_WriteReg(0xd0,0x00);
		SP0718_WriteReg(0xd1,0x05);
		SP0718_WriteReg(0xfd,0x01);
		SP0718_WriteReg(0x5b,0x05);
		SP0718_WriteReg(0x5c,0x00);
		SP0718_WriteReg(0xfd,0x01);
		SP0718_awb_enable();   //whl130802			
	    }
	    break;
	}
		
	default:
	{
	    break;
	}
    }
    msleep(100);
    return 0;
}


/*__align(4)*/ const SENSOR_REG_T SP0718_image_effect_tab[][5]=	
{
    // effect normal
    {
	{0xfd, 0x01},
	{0x66, 0x00},
	{0x67, 0x80},
	{0x68, 0x80},
	{0xff, 0xff}
    },
    //effect BLACKWHITE//GRAYSCALE
    {
	{0xfd, 0x01},
	{0x66, 0x20},
	{0x67, 0x80},
	{0x68, 0x80},
	{0xff, 0xff}
    },
    // effect RED pink
    {
	{0xfd, 0x01},
	{0x66, 0x10},
	{0x67, 0xb0},
	{0x68, 0x90},
	{0xff, 0xff}
    },
    // effect GREEN//SEPIAGREEN
    {
	{0xfd, 0x01},
	{0x66, 0x10},
	{0x67, 0x20},
	{0x68, 0x20},
	{0xff, 0xff}
    },
    // effect  BLUE//SEPIABLUE
    {
	{0xfd, 0x01},
	{0x66, 0x10},
	{0x67, 0x20},
	{0x68, 0xf0},
	{0xff, 0xff}
    },
    // effect  YELLOW//SEPIAYELLOW
    {
	{0xfd, 0x01},
	{0x66, 0x10},
	{0x67, 0xb0},
	{0x68, 0x00},
	{0xff, 0xff}

    },  
    // effect NEGATIVE//COLORINV
    {	     
	{0xfd, 0x01},
	{0x66, 0x04},
	{0x67, 0x80},
	{0x68, 0x80},
	{0xff, 0xff}
    },    
    //effect ANTIQUE//SEPIA
    {
	{0xfd, 0x01},
	{0x66, 0x10},
	{0x67, 0xc0},
	{0x68, 0x20},
	{0xff, 0xff}
    },
};

LOCAL uint32_t set_image_effect(uint32_t effect_type)
{
    uint16_t i;
    
    SENSOR_REG_T* sensor_reg_ptr = (SENSOR_REG_T*)SP0718_image_effect_tab[effect_type];

    SENSOR_ASSERT(PNULL != sensor_reg_ptr);

    for(i = 0; (0xFF != sensor_reg_ptr[i].reg_addr) || (0xFF != sensor_reg_ptr[i].reg_value) ; i++)
    {
        SP0718_WriteReg(sensor_reg_ptr[i].reg_addr, sensor_reg_ptr[i].reg_value);
    }
    return 0;
}

LOCAL uint32_t SP0718_ae_awb_enable(uint32_t ae_enable, uint32_t awb_enable)
{
    uint16_t ae_value = 0 , awb_value=0;

    SP0718_WriteReg(0xfd,0x01);
    ae_value = SP0718_ReadReg(0x32);
    awb_value = SP0718_ReadReg(0x32);

    if(0x01==ae_enable)
    {
	ae_value |= 0x05;       
    }else if(0x00==ae_enable)
    {
	ae_value &= 0xf8;
    }

    if(0x01==awb_enable)
    {
	awb_value |= 0x10;//0x08 pzt 2012-7-20
    }else if(0x00==awb_enable)
    {
	awb_value &= 0xef;  //0xf7;
    }	
	
    SP0718_WriteReg(0x32, ae_value);
    SP0718_WriteReg(0x32, awb_value);
	return 0;
}

LOCAL uint32_t SP0718_before_snapshot(uint32_t para)
{
    return 0;
}

LOCAL uint32_t SP0718_after_snapshot(uint32_t para)
{
    return 0;
}

//@ Chenfeng for adding AWB & AE functions
/*__align(4)*/ const SENSOR_REG_T SP0718_awb_tab[][7]=
{
//AUTO  // AUTO 3000K~7000K 
    {
	{0xfd,0x02},                      
	{0x26,SP0718_P2_0x26},		                  
	{0x27,SP0718_P2_0x27},                      
	{0xfd,0x01}, 		
	{0x32,0x15},   
	{0xfd,0x00},  
	{0xff,0xff} 
    },    
//INCANDESCENCE:  //2800K~3000K  
    {
	{0xfd,0x01}, 
	{0x32,0x05},                 
	{0xfd,0x02},                 
	{0x26,0x75},		             
	{0x27,0xe2},		             
	{0xfd,0x00}, 
	{0xff,0xff} 
    },
//U30 //3000K
    {
	{0xfd,0x01}, 
	{0x32,0x05},                 
	{0xfd,0x02},                 
	{0x26,0x7b},		             
	{0x27,0xd3},		             
	{0xfd,0x00},   	
	{0xff,0xff} 
    },  
//CWF  //4150K
    {
	{0xfd,0x01}, 
	{0x32,0x05},                 
	{0xfd,0x02},                 
	{0x26,0x9c},		             
	{0x27,0xc0},		             
	{0xfd,0x00},  		
	{0xff,0xff} 
    },    
//FLUORESCENT://4200K~5000K   
    {
	{0xfd,0x01}, 
	{0x32,0x05},                 
	{0xfd,0x02},                 
	{0x26,0x91},		             
	{0x27,0xc8},		             
	{0xfd,0x00}, 	
	{0xff,0xff} 
    },
//SUN: //DAYLIGHT//6500K   
    {
	{0xfd,0x01}, 
	{0x32,0x05},                 
	{0xfd,0x02},                 
	{0x26,0xc8},		             
	{0x27,0x89},		             
	{0xfd,0x00}, 		
	{0xff,0xff} 
    },
//CLOUD: //7000K 
    {
	{0xfd,0x01}, 
	{0x32,0x05},                 
	{0xfd,0x02},                 
	{0x26,0xdc},		             
	{0x27,0xe5},		             
	{0xfd,0x00}, 		
	{0xff,0xff} 
    },	
};

LOCAL uint32_t set_SP0718_awb(uint32_t mode)
{
    uint8_t awb_en_value;
    uint16_t i;
	
    SENSOR_REG_T* sensor_reg_ptr = (SENSOR_REG_T*)SP0718_awb_tab[mode];

    SP0718_WriteReg(0xfd,0x01);
    awb_en_value = SP0718_ReadReg(0x32);

    SENSOR_ASSERT(mode < 7);
    SENSOR_ASSERT(PNULL != sensor_reg_ptr);
	
    for(i = 0; (0xFF != sensor_reg_ptr[i].reg_addr) || (0xFF != sensor_reg_ptr[i].reg_value); i++)
    {
	if(0x32 == sensor_reg_ptr[i].reg_addr)
       	{
	    if(mode == 0)
	    {
		SP0718_awb_mode = 0;    //whl130802
		SP0718_WriteReg(0x32, awb_en_value |0x10 );//0x08
	    }else{
		SP0718_awb_mode = 1;   //whl130802
              	SP0718_WriteReg(0x32, awb_en_value &0xef );//0xf7
	    }
       	}else{
           SP0718_WriteReg(sensor_reg_ptr[i].reg_addr, sensor_reg_ptr[i].reg_value);
	}
    }
	msleep(100); 
	return 0;
}

#define AE_ENABLE  (0x32)
LOCAL uint32_t set_SP0718_ae_enable(uint32_t enable)
{
    unsigned char ae_value;
    SP0718_WriteReg(0xfd,0x01);
    ae_value=SP0718_ReadReg(AE_ENABLE);

    if(0x00==enable)
    {
        ae_value &= 0xf8;
        SP0718_WriteReg(AE_ENABLE,ae_value);
    }else if(0x01==enable){
        ae_value|=0x05;
        SP0718_WriteReg(AE_ENABLE,ae_value);
    }

    return 0;
}

/******************************************************************************/
// Description: anti 50/60 hz banding flicker
// Global resource dependence: 
// Author:
// Note:
//		
/******************************************************************************/
LOCAL uint32_t set_SP0718_anti_flicker(uint32_t mode)
{ 
    switch(mode)
    {
	case DCAMERA_FLICKER_50HZ:			
	    Antiflicker = DCAMERA_FLICKER_50HZ;
	    SENSOR_TRACE( " set_SP0718_anti_flicker  50hz \r\n" );
	// SP0718  24M  50HZ  8-15FPS 
	//ae setting
	    SP0718_WriteReg(0xfd,0x00);
	    SP0718_WriteReg(0x03,0x01);
	    SP0718_WriteReg(0x04,0xce);
	    SP0718_WriteReg(0x06,0x00);
	    SP0718_WriteReg(0x09,0x02);
	    SP0718_WriteReg(0x0a,0xc4);
	    SP0718_WriteReg(0xfd,0x01);
	    SP0718_WriteReg(0xef,0x4d);
	    SP0718_WriteReg(0xf0,0x00);
	    SP0718_WriteReg(0x02,0x0c);
	    SP0718_WriteReg(0x03,0x01);
	    SP0718_WriteReg(0x06,0x47);
	    SP0718_WriteReg(0x07,0x00);
	    SP0718_WriteReg(0x08,0x01);
	    SP0718_WriteReg(0x09,0x00);
	//Status                 
	    SP0718_WriteReg(0xfd,0x02);
	    SP0718_WriteReg(0xbe,0x9c);
	    SP0718_WriteReg(0xbf,0x03);
	    SP0718_WriteReg(0xd0,0x9c);
	    SP0718_WriteReg(0xd1,0x03);
	    SP0718_WriteReg(0xfd,0x01);
	    SP0718_WriteReg(0x5b,0x03);
	    SP0718_WriteReg(0x5c,0x9c);  
	    SP0718_WriteReg(0xfd,0x01);
	    SP0718_awb_enable(); //whl130802
	    break;
            
        case DCAMERA_FLICKER_60HZ:
	    Antiflicker = DCAMERA_FLICKER_60HZ;
	    SENSOR_TRACE( " set_SP0718_anti_flicker  60hz \r\n" );
	// SP0718  24M  60HZ  8-15FPS 
	//ae setting
	    SP0718_WriteReg(0xfd,0x00);
	    SP0718_WriteReg(0x03,0x01);
	    SP0718_WriteReg(0x04,0x80);
	    SP0718_WriteReg(0x06,0x00);
	    SP0718_WriteReg(0x09,0x02);
	    SP0718_WriteReg(0x0a,0xc9);
	    SP0718_WriteReg(0xfd,0x01);
	    SP0718_WriteReg(0xef,0x40);
	    SP0718_WriteReg(0xf0,0x00);
	    SP0718_WriteReg(0x02,0x0f);
	    SP0718_WriteReg(0x03,0x01);
	    SP0718_WriteReg(0x06,0x3a);
	    SP0718_WriteReg(0x07,0x00);
	    SP0718_WriteReg(0x08,0x01);
	    SP0718_WriteReg(0x09,0x00);
	//Status                   
	    SP0718_WriteReg(0xfd,0x02);
	    SP0718_WriteReg(0xbe,0xc0);
	    SP0718_WriteReg(0xbf,0x03);
	    SP0718_WriteReg(0xd0,0xc0);
	    SP0718_WriteReg(0xd1,0x03);
	    SP0718_WriteReg(0xfd,0x01);
	    SP0718_WriteReg(0x5b,0x03);
	    SP0718_WriteReg(0x5c,0xc0);
	    SP0718_WriteReg(0xfd,0x01);
	    SP0718_awb_enable();    //whl130802
	    break;
            
        default:
            break;
    }
    
    msleep(100); 
    return 0;
}

/******************************************************************************/
// Description: set video mode
// Global resource dependence: 
// Author:
// Note:
//		 
/******************************************************************************/
/*__align(4)*/ const SENSOR_REG_T SP0718_video_mode_nand_tab[][9]=
{
#if 1
    {		
	{0xfd, 0x00},
	{0xfd, 0x00},
	{0xfd, 0x00},
	{0xfd, 0x00},
	{0xfd, 0x00},
	{0xfd, 0x00},
	{0xfd, 0x00},
	{0xfd, 0x00},
	{0xff, 0xff},
    },

#else
    {
        {0x01,0x6a},
	{0x02,0x32},
	{0x0f,0x20},
	{0xe2,0x00},
	{0xe3,0x96},
	{0xe8,0x04},
	{0xe9,0x1a},
	{0xec,0x20},
	{0xff, 0xff} 
    },    
    //video mode
    {
        {0x01,0x6a},
	{0x02,0x70},
	{0x0f,0x00},
	{0xe2,0x00},
	{0xe3,0x96},
	{0xe8,0x02},
	{0xe9,0x58},
	{0xec,0x20},
	{0xff, 0xff}      
    }
#endif
};

/******************************************************************************/
// Description: set video mode
// Global resource dependence: 
// Author:
// Note:
//		 
/******************************************************************************/
/*__align(4)*/ const SENSOR_REG_T SP0718_video_mode_nor_tab_50hz[][40]=
{
//Video preview
    {
	//capture preview daylight 24M 50hz 15-8FPS  
	//ae setting
	{0xfd,0x00},
	{0x03,0x01},
	{0x04,0xce},
	{0x06,0x00},
	{0x09,0x02},
	{0x0a,0xc4},
	{0xfd,0x01},
	{0xef,0x4d},
	{0xf0,0x00},
	{0x02,0x0c},
	{0x03,0x01},
	{0x06,0x47},
	{0x07,0x00},
	{0x08,0x01},
	{0x09,0x00},
	//Status   
	{0xfd,0x02},
	{0xbe,0x9c},
	{0xbf,0x03},
	{0xd0,0x9c},
	{0xd1,0x03},
	{0xfd,0x01},
	{0x5b,0x03},
	{0x5c,0x9c},
	{0xfd,0x01},
	//{0x32,0x15},    //whl130802
	{0xFF,0xFF},	//数组结束标志
    },    
//Video record
    {
	//Video record daylight 24M 50hz 14-14FPS
	{0xfd,0x00},
	{0x03,0x01},
	{0x04,0xb0},
	{0x06,0x00},
	{0x09,0x03},
	{0x0a,0x31},
	{0xfd,0x01},
	{0xef,0x48},
	{0xf0,0x00},
	{0x02,0x07},
	{0x03,0x01},
	{0x06,0x42},
	{0x07,0x00},
	{0x08,0x01},
	{0x09,0x00},
	//Status
	{0xfd,0x02},
	{0xbe,0xf8},
	{0xbf,0x01},
	{0xd0,0xf8},
	{0xd1,0x01},
	{0xfd,0x01},
	{0x5b,0x01},
	{0x5c,0xf8},
	{0xfd,0x01},
	//{0x32,0x15},    //whl130802
	{0xFF,0xFF},	//数组结束标志
    },
// UPCC  mode
    {
	//Video record daylight 24M 50hz 14-14FPS 
	{0xfd,0x00},
	{0x03,0x01},
	{0x04,0xb0},
	{0x06,0x00},
	{0x09,0x03},
	{0x0a,0x31},
	{0xfd,0x01},
	{0xef,0x48},
	{0xf0,0x00},
	{0x02,0x07},
	{0x03,0x01},
	{0x06,0x42},
	{0x07,0x00},
	{0x08,0x01},
	{0x09,0x00},
	//Status
	{0xfd,0x02},
	{0xbe,0xf8},
	{0xbf,0x01},
	{0xd0,0xf8},
	{0xd1,0x01},
	{0xfd,0x01},
	{0x5b,0x01},
	{0x5c,0xf8},
	{0xfd,0x01},
	//{0x32,0x15},    //whl130802
	{0xFF,0xFF},	//数组结束标志
    }  
};    


/*__align(4)*/ const SENSOR_REG_T SP0718_video_mode_nor_tab_60hz[][40]=
{
//Video preview
    {
	//capture preview daylight 24M 60Hz 15-8FPS    
	{0xfd,0x00},
	{0x03,0x01},
	{0x04,0x80},
	{0x06,0x00},
	{0x09,0x02},
	{0x0a,0xc9},
	{0xfd,0x01},
	{0xef,0x40},
	{0xf0,0x00},
	{0x02,0x0f},
	{0x03,0x01},
	{0x06,0x3a},
	{0x07,0x00},
	{0x08,0x01},
	{0x09,0x00},
	//Status
	{0xfd,0x02},
	{0xbe,0xc0},
	{0xbf,0x03},
	{0xd0,0xc0},
	{0xd1,0x03},
	{0xfd,0x01},
	{0x5b,0x03},
	{0x5c,0xc0},
	{0xfd,0x01},
	//{0x32,0x15},   //whl130802
	{0xFF,0xFF},	//数组结束标志
    },    
//Video record
    {
	//Video record daylight 24M 60Hz 14-14FPS 
	{0xfd,0x00},
	{0x03,0x01},
	{0x04,0x68},
	{0x06,0x00},
	{0x09,0x03},
	{0x0a,0x31},
	{0xfd,0x01},
	{0xef,0x3c},
	{0xf0,0x00},
	{0x02,0x08},
	{0x03,0x01},
	{0x06,0x36},
	{0x07,0x00},
	{0x08,0x01},
	{0x09,0x00},
	//Status
	{0xfd,0x02},
	{0xbe,0xe0},
	{0xbf,0x01},
	{0xd0,0xe0},
	{0xd1,0x01},
	{0xfd,0x01},
	{0x5b,0x01},
	{0x5c,0xe0},
	{0xfd,0x01},
	//{0x32,0x15},   //whl130802
	{0xFF,0xFF},	//数组结束标志
    },
// UPCC  mode
    {	
	//Video record daylight 24M 60Hz 14-14FPS 
	{0xfd,0x00},
	{0x03,0x01},
	{0x04,0x68},
	{0x06,0x00},
	{0x09,0x03},
	{0x0a,0x31},
	{0xfd,0x01},
	{0xef,0x3c},
	{0xf0,0x00},
	{0x02,0x08},
	{0x03,0x01},
	{0x06,0x36},
	{0x07,0x00},
	{0x08,0x01},
	{0x09,0x00},
	//Status
	{0xfd,0x02},
	{0xbe,0xe0},
	{0xbf,0x01},
	{0xd0,0xe0},
	{0xd1,0x01},
	{0xfd,0x01},
	{0x5b,0x01},
	{0x5c,0xe0},
	{0xfd,0x01},
	//{0x32,0x15},   //whl130802
	{0xFF,0xFF},	//数组结束标志	
    }  
};


LOCAL uint32_t set_SP0718_video_mode(uint32_t mode)
{
    uint8_t data=0x00;
    uint16_t i;
    SENSOR_REG_T* sensor_reg_ptr = PNULL;
    uint8_t tempregval = 0;
    SENSOR_TRACE(" xg:set_SP0718_video_mode ,%d,%d\r\n",mode,Antiflicker);
   // SENSOR_ASSERT(mode <=DCAMERA_MODE_MAX);

    if(Antiflicker ==DCAMERA_FLICKER_50HZ )
    {
	sensor_reg_ptr = (SENSOR_REG_T*)SP0718_video_mode_nor_tab_50hz[mode];
    }else{
	sensor_reg_ptr = (SENSOR_REG_T*)SP0718_video_mode_nor_tab_60hz[mode];
    }

    SENSOR_ASSERT(PNULL != sensor_reg_ptr);

    for(i = 0; (0xFF != sensor_reg_ptr[i].reg_addr) || (0xFF != sensor_reg_ptr[i].reg_value); i++)
    {
    	SP0718_WriteReg(sensor_reg_ptr[i].reg_addr, sensor_reg_ptr[i].reg_value);
    	     
    }

    SP0718_awb_enable(); //whl130802
    return 0;
}

/*__align(4)*/ const SENSOR_REG_T SP0718_ev_tab[][10]=
{   
    {
	{0xfd,0x01},   
	{0xeb,SP0718_P1_0xeb-0x18},			
	{0xec,SP0718_P1_0xec-0x18},//level -3				
	{0xff,0xff},        
    },
    {
	{0xfd,0x01},   
	{0xeb,SP0718_P1_0xeb-0x10},			
	{0xec,SP0718_P1_0xec-0x10},//level -2			
	{0xff,0xff}, 			     
    },
    {
	{0xfd,0x01},   
	{0xeb,SP0718_P1_0xeb-0x08},			
	{0xec,SP0718_P1_0xec-0x08},//level -1			
	{0xff,0xff}, 			   
    },
    {
	{0xfd,0x01},   
	{0xeb,SP0718_P1_0xeb},			
	{0xec,SP0718_P1_0xec},//level 0				
	{0xff,0xff}, 			    
    },
    {
	{0xfd,0x01},   
	{0xeb,SP0718_P1_0xeb+0x08},			
	{0xec,SP0718_P1_0xec+0x08},//level 1			
	{0xff,0xff}, 			    
    },
    {
	{0xfd,0x01},   
	{0xeb,SP0718_P1_0xeb+0x10},			
	{0xec,SP0718_P1_0xec+0x10},//level 2				
	{0xff,0xff}, 			      
    },
    {
	{0xfd,0x01},   
	{0xeb,SP0718_P1_0xeb+0x18},			
	{0xec,SP0718_P1_0xec+0x18},//level 3						
	{0xff,0xff},    
    },
};

LOCAL uint32_t set_SP0718_ev(uint32_t level)
{
    uint16_t i; 
    SENSOR_REG_T* sensor_reg_ptr = (SENSOR_REG_T*)SP0718_ev_tab[level];

    SENSOR_ASSERT(PNULL != sensor_reg_ptr);
    SENSOR_ASSERT(level < 7);
 
    for(i = 0; (0xFF != sensor_reg_ptr[i].reg_addr) ||(0xFF != sensor_reg_ptr[i].reg_value) ; i++)
    {
        SP0718_WriteReg(sensor_reg_ptr[i].reg_addr, sensor_reg_ptr[i].reg_value);
    }
    return 0;
}

LOCAL SENSOR_REG_TAB_INFO_T s_SP0718_resolution_Tab_YUV[]=
{   
	// COMMON INIT
	{ ADDR_AND_LEN_OF_ARRAY(SP0718_YVU_640X480),	640,	480,	24,	SENSOR_IMAGE_FORMAT_YUV422 },

	// YUV422 PREVIEW 1
	{ 			 PNULL,  	  0,    640,    480,    24,     SENSOR_IMAGE_FORMAT_YUV422 },
	{ 			 PNULL,           0,      0,      0,     0,   			         0 },   
	{ 			 PNULL,           0,      0,      0,     0,        			 0 },   
	{ 			 PNULL,           0,      0,      0,     0,            			 0 },   

	// YUV422 PREVIEW 2 
	{ 			 PNULL,           0,      0,      0,     0,         			 0 },   
	{ 		  	 PNULL,           0,      0,      0,     0,      		         0 },   
	{ 			 PNULL,           0,      0,      0,     0,       			 0 },   
	{ 			 PNULL,           0,      0,      0,     0,        			 0 }
};

LOCAL SENSOR_IOCTL_FUNC_TAB_T s_SP0718_ioctl_func_tab = 
{
	// Internal 
	PNULL,
	PNULL,  //  SP0718_Power_On,  // PNULL 
	PNULL,
	SP0718_Identify,

	PNULL,          // write register
	PNULL,          // read  register   
	PNULL,
	PNULL,

	// External
	PNULL,//set_SP0718_ae_enable, //set_ae_enable,
	PNULL,//hmirror_enable, //set_hmirror_enable,
	PNULL,//vmirror_enable, //set_vmirror_enable,

	set_brightness,
	set_contrast,
	PNULL,// set_sharpness,
	PNULL,//set_saturation,

	set_preview_mode,	
	set_image_effect,

	PNULL,//GC0309_BeforeSnapshot, 
	PNULL, //GC0309_After_Snapshot,
	
	PNULL,
	PNULL,
	PNULL,
	PNULL,
	PNULL,
	PNULL,
	PNULL,
	PNULL,
	set_SP0718_awb,    
	PNULL, //read_ev_value,
	PNULL, //write_ev_value,
	set_SP0718_ev, //read_gain_value,
	PNULL, //write_gain_value,
	PNULL, //read_gain_scale,
	PNULL, //set_frame_rate,
	PNULL,
	PNULL,
	set_SP0718_anti_flicker, //PNULL,
	set_SP0718_video_mode,   
	PNULL, 
};

 SENSOR_INFO_T g_SP0718_yuv_info =
{
	SP0718_I2C_ADDR_W,				// salve i2c write address
	SP0718_I2C_ADDR_R, 				// salve i2c read address
	
	0,								// bit0: 0: i2c register value is 8 bit, 1: i2c register value is 16 bit
									// bit2: 0: i2c register addr  is 8 bit, 1: i2c register addr  is 16 bit
									// other bit: reseved
	SENSOR_HW_SIGNAL_PCLK_P|\
	SENSOR_HW_SIGNAL_VSYNC_P|\
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
	SENSOR_LOW_PULSE_RESET,		// reset pulse level
	100,								// reset pulse width(ms)
	SENSOR_HIGH_LEVEL_PWDN,			// 1: high level valid; 0: low level valid
	2,
	{{0x02, 0x71},
	{0x02, 0x71}},
	
	SENSOR_AVDD_2800MV,				// voltage of avdd	
	
	640,							// max width of source image
	480,							// max height of source image
	"SP0718",						// name of sensor												
	SENSOR_IMAGE_FORMAT_YUV422,		// define in SENSOR_IMAGE_FORMAT_E enum,
									// if set to SENSOR_IMAGE_FORMAT_MAX here, image format depent on SENSOR_REG_TAB_INFO_T
	SENSOR_IMAGE_PATTERN_YUV422_YUYV,	// pattern of input image form sensor;			

	s_SP0718_resolution_Tab_YUV,	// point to resolution table information structure
	&s_SP0718_ioctl_func_tab,		// point to ioctl function table
			
	PNULL,							// information and table about Rawrgb sensor
	PNULL,							// extend information about sensor	
	SENSOR_AVDD_2800MV,                     // iovdd
	SENSOR_AVDD_1800MV,                      // dvdd   //SENSOR_AVDD_1800MV
	3,								//skip frame num before preview
	0,								//skip frame num before capture
	0,			//skip frame num during preview
	2			//skip frame num during video preview
};

struct sensor_drv_cfg sensor_sp0718 = {
    .sensor_pos = CONFIG_DCAM_SENSOR_POS_SP0718,
    .sensor_name = "sp0718",
    .driver_info = &g_SP0718_yuv_info,
};

static int __init sensor_sp0718_init(void)
{
    return dcam_register_sensor_drv(&sensor_sp0718);
}
subsys_initcall(sensor_sp0718_init);
