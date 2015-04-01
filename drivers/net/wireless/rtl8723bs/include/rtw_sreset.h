/******************************************************************************
 *
 * Copyright(c) 2007 - 2012 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/
#ifndef _RTW_SRESET_C_
#define _RTW_SRESET_C_

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>

struct sreset_priv {
	_mutex 	silentreset_mutex;
	u8 	silent_reset_inprogress;
	u8	Wifi_Error_Status;
	unsigned long last_tx_time;
	unsigned long last_tx_complete_time;
};

#ifdef CONFIG_RTL8192C
#include <rtl8192c_hal.h>
#endif
#ifdef CONFIG_RTL8192D
#include <rtl8192d_hal.h>
#endif
#ifdef CONFIG_RTL8723A
#include <rtl8723a_hal.h>
#endif
#ifdef CONFIG_RTL8188E
#include <rtl8188e_hal.h>
#endif
#ifdef CONFIG_RTL8723B
#include <rtl8723b_hal.h>
#endif

#define	WIFI_STATUS_SUCCESS 		0
#define	USB_VEN_REQ_CMD_FAIL 	BIT0
#define	USB_READ_PORT_FAIL 		BIT1
#define	USB_WRITE_PORT_FAIL		BIT2
#define	WIFI_MAC_TXDMA_ERROR 	BIT3
#define   WIFI_TX_HANG				BIT4
#define	WIFI_RX_HANG				BIT5
#define 	WIFI_IF_NOT_EXIST			BIT6

#if defined(DBG_CONFIG_ERROR_DETECT)
void sreset_init_value(_adapter *padapter);
void sreset_reset_value(_adapter *padapter);
u8 sreset_get_wifi_status(_adapter *padapter);
void sreset_set_wifi_error_status(_adapter *padapter, u32 status);
#else
static void sreset_init_value(_adapter *padapter){}
static void sreset_reset_value(_adapter *padapter){}
static u8 sreset_get_wifi_status(_adapter *padapter){return WIFI_STATUS_SUCCESS;}
static void sreset_set_wifi_error_status(_adapter *padapter, u32 status){}
#endif

#endif

