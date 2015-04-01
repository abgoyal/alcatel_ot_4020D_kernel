#ifndef	__HALBTC_OUT_SRC_H__
#define __HALBTC_OUT_SRC_H__

#define		NORMAL_EXEC					FALSE
#define		FORCE_EXEC						TRUE

#define		BTC_RF_A					PathA
#define		BTC_RF_B					PathB
#define		BTC_RF_C					PathC
#define		BTC_RF_D					PathD

#define		BTC_SMSP				SINGLEMAC_SINGLEPHY
#define		BTC_DMDP				DUALMAC_DUALPHY
#define		BTC_DMSP				DUALMAC_SINGLEPHY
#define		BTC_MP_UNKNOWN		0xff

// single Antenna definition
#define		BTC_ANT_PATH_WIFI			0
#define		BTC_ANT_PATH_BT				1
#define		BTC_ANT_PATH_PTA				2
// dual Antenna definition
#define		BTC_ANT_WIFI_AT_MAIN		0
#define		BTC_ANT_WIFI_AT_AUX			1
// coupler Antenna definition
#define		BTC_ANT_WIFI_AT_CPL_MAIN	0
#define		BTC_ANT_WIFI_AT_CPL_AUX		1

typedef enum _BTC_POWERSAVE_TYPE{
	BTC_PS_WIFI_NATIVE			= 0,	// wifi original power save behavior
	BTC_PS_LPS_ON				= 1,
	BTC_PS_LPS_OFF				= 2,
	BTC_PS_MAX
} BTC_POWERSAVE_TYPE, *PBTC_POWERSAVE_TYPE;

typedef enum _BTC_CHIP_INTERFACE{
	BTC_INTF_UNKNOWN	= 0,
	BTC_INTF_PCI			= 1,
	BTC_INTF_USB			= 2,
	BTC_INTF_SDIO		= 3,
	BTC_INTF_GSPI		= 4,
	BTC_INTF_MAX
} BTC_CHIP_INTERFACE, *PBTC_CHIP_INTERFACE;

typedef enum _BTC_CHIP_TYPE{
	BTC_CHIP_UNDEF		= 0,
	BTC_CHIP_CSR_BC4		= 1,
	BTC_CHIP_CSR_BC8		= 2,
	BTC_CHIP_RTL8723A 	= 3,
	BTC_CHIP_RTL8821	= 4,
	BTC_CHIP_RTL8723B 	= 5,
	BTC_CHIP_MAX
} BTC_CHIP_TYPE, *PBTC_CHIP_TYPE;

typedef enum _BTC_MSG_TYPE{
	BTC_MSG_INTERFACE			= 0x0,
	BTC_MSG_ALGORITHM			= 0x1,
	BTC_MSG_MAX
}BTC_MSG_TYPE;
extern u4Byte					GLBtcDbgType[];

// following is for BTC_MSG_INTERFACE
#define		INTF_INIT						BIT0
#define		INTF_NOTIFY						BIT2

// following is for BTC_ALGORITHM
#define		ALGO_BT_RSSI_STATE				BIT0
#define		ALGO_WIFI_RSSI_STATE				BIT1
#define		ALGO_BT_MONITOR				BIT2
#define		ALGO_TRACE						BIT3
#define		ALGO_TRACE_FW					BIT4
#define		ALGO_TRACE_FW_DETAIL			BIT5
#define		ALGO_TRACE_FW_EXEC				BIT6
#define		ALGO_TRACE_SW					BIT7
#define		ALGO_TRACE_SW_DETAIL			BIT8
#define		ALGO_TRACE_SW_EXEC				BIT9

// following is for wifi link status
#define		WIFI_STA_CONNECTED				BIT0
#define		WIFI_AP_CONNECTED				BIT1
#define		WIFI_HS_CONNECTED				BIT2
#define		WIFI_P2P_GO_CONNECTED			BIT3
#define		WIFI_P2P_GC_CONNECTED			BIT4

// following is for command line utility
#ifdef CONFIG_DEBUG
#define	BTC_PRINT(dbgtype, dbgflag, printstr)\
{\
	if (GLBtcDbgType[dbgtype] & dbgflag)\
	{\
		DbgPrint printstr;\
	}\
}

#define	BTC_PRINT_F(dbgtype, dbgflag, printstr)\
{\
	if (GLBtcDbgType[dbgtype] & dbgflag)\
	{\
		DbgPrint("%s(): ", __FUNCTION__);\
		DbgPrint printstr;\
	}\
}

#define	BTC_PRINT_ADDR(dbgtype, dbgflag, printstr, _Ptr)\
{\
	if (GLBtcDbgType[dbgtype] & dbgflag)\
	{\
				int __i;						\
				pu1Byte	ptr = (pu1Byte)_Ptr;	\
				DbgPrint printstr;				\
				DbgPrint(" ");					\
				for( __i=0; __i<6; __i++ )		\
					DbgPrint("%02X%s", ptr[__i], (__i==5)?"":"-");		\
				DbgPrint("\n");							\
	}\
}

#define 	BTC_PRINT_DATA(dbgtype, dbgflag, _TitleString, _HexData, _HexDataLen)\
{\
	if (GLBtcDbgType[dbgtype] & dbgflag)\
	{\
		int __i;									\
		pu1Byte	ptr = (pu1Byte)_HexData;			\
		DbgPrint(_TitleString);					\
		for( __i=0; __i<(int)_HexDataLen; __i++ )	\
		{										\
			DbgPrint("%02X%s", ptr[__i], (((__i + 1) % 4) == 0)?"  ":" ");\
			if (((__i + 1) % 16) == 0)	DbgPrint("\n");\
		}										\
		DbgPrint("\n");							\
	}\
}

#else
#define	BTC_PRINT(dbgtype, dbgflag, printstr)
#define	BTC_PRINT_F(dbgtype, dbgflag, printstr)
#define	BTC_PRINT_ADDR(dbgtype, dbgflag, printstr, _Ptr)
#define 	BTC_PRINT_DATA(dbgtype, dbgflag, _TitleString, _HexData, _HexDataLen)
#endif

//Bryant Add
typedef enum _BTC_ANTENNA_POS{
	BTC_ANTENNA_AT_MAIN_PORT				= 0x1,
	BTC_ANTENNA_AT_AUX_PORT				= 0x2,
}BTC_ANTENNA_POS,*PBTC_ANTENNA_POS;

typedef struct _BTC_BOARD_INFO{
	// The following is some board information
	u1Byte				btChipType;
	u1Byte				pgAntNum;	// pg ant number
	u1Byte				btdmAntNum;	// ant number for btdm
	u1Byte				btdmAntPos;		//Bryant Add to indicate Antenna Position for (pgAntNum = 2) && (btdmAntNum =1)  (DPDT+1Ant case)
	BOOLEAN				bBtExist;
} BTC_BOARD_INFO, *PBTC_BOARD_INFO;

typedef enum _BTC_DBG_OPCODE{
	BTC_DBG_SET_COEX_NORMAL				= 0x0,
	BTC_DBG_SET_COEX_WIFI_ONLY				= 0x1,
	BTC_DBG_SET_COEX_BT_ONLY				= 0x2,
	BTC_DBG_MAX
}BTC_DBG_OPCODE,*PBTC_DBG_OPCODE;

typedef enum _BTC_RSSI_STATE{
	BTC_RSSI_STATE_HIGH						= 0x0,
	BTC_RSSI_STATE_MEDIUM					= 0x1,
	BTC_RSSI_STATE_LOW						= 0x2,
	BTC_RSSI_STATE_STAY_HIGH					= 0x3,
	BTC_RSSI_STATE_STAY_MEDIUM				= 0x4,
	BTC_RSSI_STATE_STAY_LOW					= 0x5,
	BTC_RSSI_MAX
}BTC_RSSI_STATE,*PBTC_RSSI_STATE;

typedef enum _BTC_WIFI_ROLE{
	BTC_ROLE_STATION						= 0x0,
	BTC_ROLE_AP								= 0x1,
	BTC_ROLE_IBSS							= 0x2,
	BTC_ROLE_HS_MODE						= 0x3,
	BTC_ROLE_MAX
}BTC_WIFI_ROLE,*PBTC_WIFI_ROLE;

typedef enum _BTC_WIFI_BW_MODE{
	BTC_WIFI_BW_LEGACY					= 0x0,
	BTC_WIFI_BW_HT20					= 0x1,
	BTC_WIFI_BW_HT40					= 0x2,
	BTC_WIFI_BW_MAX
}BTC_WIFI_BW_MODE,*PBTC_WIFI_BW_MODE;

typedef enum _BTC_WIFI_TRAFFIC_DIR{
	BTC_WIFI_TRAFFIC_TX					= 0x0,
	BTC_WIFI_TRAFFIC_RX					= 0x1,
	BTC_WIFI_TRAFFIC_MAX
}BTC_WIFI_TRAFFIC_DIR,*PBTC_WIFI_TRAFFIC_DIR;

typedef enum _BTC_WIFI_PNP{
	BTC_WIFI_PNP_WAKE_UP					= 0x0,
	BTC_WIFI_PNP_SLEEP						= 0x1,
	BTC_WIFI_PNP_MAX
}BTC_WIFI_PNP,*PBTC_WIFI_PNP;

typedef enum _BTC_WIFI_DL_FW{
	BTC_WIFI_DLFW_BEGAIN					= 0x0,
	BTC_WIFI_DLFW_END						= 0x1,
	BTC_WIFI_DLFW_MAX
}BTC_WIFI_DLFW,*PBTC_WIFI_DLFW;

// defined for BFP_BTC_GET
typedef enum _BTC_GET_TYPE{
	// type BOOLEAN
	BTC_GET_BL_HS_OPERATION,
	BTC_GET_BL_HS_CONNECTING,
	BTC_GET_BL_WIFI_CONNECTED,
	BTC_GET_BL_WIFI_BUSY,
	BTC_GET_BL_WIFI_BUSY_TDMA,
	BTC_GET_BL_WIFI_SCAN,
	BTC_GET_BL_WIFI_LINK,
	BTC_GET_BL_WIFI_DHCP,
	BTC_GET_BL_WIFI_SOFTAP_IDLE,
	BTC_GET_BL_WIFI_SOFTAP_LINKING,
	BTC_GET_BL_WIFI_IN_EARLY_SUSPEND,
	BTC_GET_BL_WIFI_ROAM,
	BTC_GET_BL_WIFI_4_WAY_PROGRESS,
	BTC_GET_BL_WIFI_UNDER_5G,
	BTC_GET_BL_WIFI_AP_MODE_ENABLE,
	BTC_GET_BL_WIFI_ENABLE_ENCRYPTION,
	BTC_GET_BL_WIFI_UNDER_B_MODE,
	BTC_GET_BL_EXT_SWITCH,

	// type s4Byte
	BTC_GET_S4_WIFI_RSSI,
	BTC_GET_S4_HS_RSSI,

	// type u4Byte
	BTC_GET_U4_WIFI_BW,
	BTC_GET_U4_WIFI_TRAFFIC_DIRECTION,
	BTC_GET_U4_WIFI_FW_VER,
	BTC_GET_U4_WIFI_LINK_STATUS,
	BTC_GET_U4_BT_PATCH_VER,

	// type u1Byte
	BTC_GET_U1_WIFI_DOT11_CHNL,
	BTC_GET_U1_WIFI_CENTRAL_CHNL,
	BTC_GET_U1_WIFI_HS_CHNL,
	BTC_GET_U1_MAC_PHY_MODE,

	//===== for 1Ant ======
	BTC_GET_U1_LPS_MODE,
	BTC_GET_BL_BT_SCO_BUSY,

	//===== for test mode ==
	BTC_GET_DRIVER_TEST_CFG,
#if 0
	BTC_GET_U1_LPS,
	BTC_GET_U1_RPWM,
#endif
	BTC_GET_MAX
}BTC_GET_TYPE,*PBTC_GET_TYPE;

// defined for BFP_BTC_SET
typedef enum _BTC_SET_TYPE{
	// type BOOLEAN
	BTC_SET_BL_BT_DISABLE,
	BTC_SET_BL_BT_TRAFFIC_BUSY,
	BTC_SET_BL_BT_LIMITED_DIG,
	BTC_SET_BL_FORCE_TO_ROAM,
	BTC_SET_BL_TO_REJ_AP_AGG_PKT,
	BTC_SET_BL_BT_CTRL_AGG_SIZE,
	BTC_SET_BL_INC_SCAN_DEV_NUM,

	// type u1Byte
	BTC_SET_U1_RSSI_ADJ_VAL_FOR_AGC_TABLE_ON,
	BTC_SET_U1_AGG_BUF_SIZE,
	BTC_SET_U1_RSSI_ADJ_VAL_FOR_1ANT_COEX_TYPE,
	BTC_SET_UI_SCAN_SIG_COMPENSATION,

	// type trigger some action
	BTC_SET_ACT_GET_BT_RSSI,
	BTC_SET_ACT_AGGREGATE_CTRL,

	//===== for 1Ant ======
	// type u1Byte
	BTC_SET_U1_LPS_VAL,
	BTC_SET_U1_RPWM_VAL,
	// type trigger some action
	BTC_SET_ACT_LEAVE_LPS,
	BTC_SET_ACT_ENTER_LPS,
	BTC_SET_ACT_NORMAL_LPS,
	BTC_SET_ACT_DISABLE_LOW_POWER,
	BTC_SET_ACT_UPDATE_RAMASK,
	// BT Coex related
	BTC_SET_ACT_CTRL_BT_INFO,
	BTC_SET_ACT_CTRL_BT_COEX,
	//=================
	BTC_SET_MAX
}BTC_SET_TYPE,*PBTC_SET_TYPE;

typedef enum _BTC_DBG_DISP_TYPE{
	BTC_DBG_DISP_COEX_STATISTICS				= 0x0,
	BTC_DBG_DISP_BT_LINK_INFO				= 0x1,
	BTC_DBG_DISP_BT_FW_VER					= 0x2,
	BTC_DBG_DISP_FW_PWR_MODE_CMD			= 0x3,
	BTC_DBG_DISP_MAX
}BTC_DBG_DISP_TYPE,*PBTC_DBG_DISP_TYPE;

typedef enum _BTC_NOTIFY_TYPE_IPS{
	BTC_IPS_LEAVE							= 0x0,
	BTC_IPS_ENTER							= 0x1,
	BTC_IPS_MAX
}BTC_NOTIFY_TYPE_IPS,*PBTC_NOTIFY_TYPE_IPS;
typedef enum _BTC_NOTIFY_TYPE_LPS{
	BTC_LPS_DISABLE							= 0x0,
	BTC_LPS_ENABLE							= 0x1,
	BTC_LPS_MAX
}BTC_NOTIFY_TYPE_LPS,*PBTC_NOTIFY_TYPE_LPS;
typedef enum _BTC_NOTIFY_TYPE_SCAN{
	BTC_SCAN_FINISH							= 0x0,
	BTC_SCAN_START							= 0x1,
	BTC_SCAN_MAX
}BTC_NOTIFY_TYPE_SCAN,*PBTC_NOTIFY_TYPE_SCAN;
typedef enum _BTC_NOTIFY_TYPE_ASSOCIATE{
	BTC_ASSOCIATE_FINISH						= 0x0,
	BTC_ASSOCIATE_START						= 0x1,
	BTC_ASSOCIATE_MAX
}BTC_NOTIFY_TYPE_ASSOCIATE,*PBTC_NOTIFY_TYPE_ASSOCIATE;
typedef enum _BTC_NOTIFY_TYPE_MEDIA_STATUS{
	BTC_MEDIA_DISCONNECT					= 0x0,
	BTC_MEDIA_CONNECT						= 0x1,
	BTC_MEDIA_MAX
}BTC_NOTIFY_TYPE_MEDIA_STATUS,*PBTC_NOTIFY_TYPE_MEDIA_STATUS;
typedef enum _BTC_NOTIFY_TYPE_SPECIAL_PACKET{
	BTC_PACKET_UNKNOWN					= 0x0,
	BTC_PACKET_DHCP							= 0x1,
	BTC_PACKET_ARP							= 0x2,
	BTC_PACKET_EAPOL						= 0x3,
	BTC_PACKET_MAX
}BTC_NOTIFY_TYPE_SPECIAL_PACKET,*PBTC_NOTIFY_TYPE_SPECIAL_PACKET;
typedef enum _BTC_NOTIFY_TYPE_STACK_OPERATION{
	BTC_STACK_OP_NONE					= 0x0,
	BTC_STACK_OP_INQ_PAGE_PAIR_START		= 0x1,
	BTC_STACK_OP_INQ_PAGE_PAIR_FINISH	= 0x2,
	BTC_STACK_OP_MAX
}BTC_NOTIFY_TYPE_STACK_OPERATION,*PBTC_NOTIFY_TYPE_STACK_OPERATION;

typedef u1Byte
(*BFP_BTC_R1)(
	IN 	PVOID			pBtcContext,
	IN	u4Byte			RegAddr
	);
typedef u2Byte
(*BFP_BTC_R2)(
	IN 	PVOID			pBtcContext,
	IN	u4Byte			RegAddr
	);
typedef u4Byte
(*BFP_BTC_R4)(
	IN 	PVOID			pBtcContext,
	IN	u4Byte			RegAddr
	);
typedef VOID
(*BFP_BTC_W1)(
	IN 	PVOID			pBtcContext,
	IN	u4Byte			RegAddr,
	IN	u1Byte			Data
	);
typedef VOID
(*BFP_BTC_W1_BIT_MASK)(
	IN 	PVOID			pBtcContext,
	IN	u4Byte			regAddr,
	IN	u1Byte			bitMask,
	IN	u1Byte			data1b
	);
typedef VOID
(*BFP_BTC_W2)(
	IN 	PVOID			pBtcContext,
	IN	u4Byte			RegAddr,
	IN	u2Byte			Data
	);
typedef VOID
(*BFP_BTC_W4)(
	IN 	PVOID			pBtcContext,
	IN	u4Byte			RegAddr,
	IN	u4Byte			Data
	);
typedef VOID
(*BFP_BTC_SET_BB_REG)(
	IN 	PVOID			pBtcContext,
	IN	u4Byte			RegAddr,
	IN	u4Byte			BitMask,
	IN	u4Byte			Data
	);
typedef u4Byte
(*BFP_BTC_GET_BB_REG)(
	IN 	PVOID			pBtcContext,
	IN	u4Byte			RegAddr,
	IN	u4Byte			BitMask
	);
typedef VOID
(*BFP_BTC_SET_RF_REG)(
	IN 	PVOID			pBtcContext,
	IN	u1Byte			eRFPath,
	IN	u4Byte			RegAddr,
	IN	u4Byte			BitMask,
	IN	u4Byte			Data
	);
typedef u4Byte
(*BFP_BTC_GET_RF_REG)(
	IN 	PVOID			pBtcContext,
	IN	u1Byte			eRFPath,
	IN	u4Byte			RegAddr,
	IN	u4Byte			BitMask
	);
typedef VOID
(*BFP_BTC_SET_BT_REG)(
	IN 	PVOID			pBtcContext,
	IN	u1Byte			RegType,
	IN	u4Byte			RegAddr,
	IN	u4Byte			Data
	);
typedef VOID
(*BFP_BTC_FILL_H2C)(
	IN 	PVOID			pBtcContext,
	IN	u1Byte 			elementId,
	IN	u4Byte 			cmdLen,
	IN	pu1Byte			pCmdBuffer
	);

typedef	BOOLEAN
(*BFP_BTC_GET)(
	IN	PVOID			pBtCoexist,
	IN	u1Byte			getType,
	OUT	PVOID			pOutBuf
	);

typedef	BOOLEAN
(*BFP_BTC_SET)(
	IN	PVOID			pBtCoexist,
	IN	u1Byte			setType,
	OUT	PVOID			pInBuf
	);
typedef VOID
(*BFP_BTC_DISP_DBG_MSG)(
	IN	PVOID			pBtCoexist,
	IN	u1Byte			dispType
	);

typedef struct _BTC_BT_INFO{
	BOOLEAN					bBtDisabled;
	u1Byte					rssiAdjustForAgcTableOn;
	u1Byte					rssiAdjustFor1AntCoexType;
	BOOLEAN					bPreBtCtrlAggBufSize;
	BOOLEAN					bBtCtrlAggBufSize;
	BOOLEAN					bRejectAggPkt;
	BOOLEAN					bIncreaseScanDevNum;
	u1Byte					preAggBufSize;
	u1Byte					aggBufSize;
	BOOLEAN					bBtBusy;
	BOOLEAN					bLimitedDig;
	u2Byte					btHciVer;
	u2Byte					btRealFwVer;
	u1Byte					btFwVer;

	BOOLEAN					bBtDisableLowPwr;
	BOOLEAN					bBtCtrlLps;
	BOOLEAN					bBtLpsOn;
	BOOLEAN					bForceToRoam;	// for 1Ant solution
	u1Byte					lpsVal;
	u1Byte					rpwmVal;
	u4Byte					raMask;
} BTC_BT_INFO, *PBTC_BT_INFO;

typedef struct _BTC_STACK_INFO{
	BOOLEAN					bProfileNotified;
	u2Byte					hciVersion;	// stack hci version
	u1Byte					numOfLink;
	BOOLEAN					bBtLinkExist;
	BOOLEAN					bScoExist;
	BOOLEAN					bAclExist;
	BOOLEAN					bA2dpExist;
	BOOLEAN					bHidExist;
	BOOLEAN					bPanExist;
	BOOLEAN					bUnknownAclExist;
	s1Byte					minBtRssi;
	u1Byte					numOfHid;
} BTC_STACK_INFO, *PBTC_STACK_INFO;

typedef struct _BTC_BT_LINK_INFO{
	BOOLEAN					bBtLinkExist;
	BOOLEAN					bScoExist;
	BOOLEAN					bScoOnly;
	BOOLEAN					bA2dpExist;
	BOOLEAN					bA2dpOnly;
	BOOLEAN					bHidExist;
	BOOLEAN					bHidOnly;
	BOOLEAN					bPanExist;
	BOOLEAN					bPanOnly;
} BTC_BT_LINK_INFO, *PBTC_BT_LINK_INFO;

typedef struct _BTC_STATISTICS{
	u4Byte					cntBind;
	u4Byte					cntInitHwConfig;
	u4Byte					cntInitCoexDm;
	u4Byte					cntIpsNotify;
	u4Byte					cntLpsNotify;
	u4Byte					cntScanNotify;
	u4Byte					cntConnectNotify;
	u4Byte					cntMediaStatusNotify;
	u4Byte					cntSpecialPacketNotify;
	u4Byte					cntBtInfoNotify;
	u4Byte					cntPeriodical;
	u4Byte					cntStackOperationNotify;
	u4Byte					cntDbgCtrl;
} BTC_STATISTICS, *PBTC_STATISTICS;

typedef struct _BTC_COEXIST{
	BOOLEAN				bBinded;		// make sure only one adapter can bind the data context
	PVOID				Adapter;		// default adapter
	BTC_BOARD_INFO		boardInfo;
	BTC_BT_INFO			btInfo;		// some bt info referenced by non-bt module
	BTC_STACK_INFO		stackInfo;
	BTC_BT_LINK_INFO	btLinkInfo;
	BTC_CHIP_INTERFACE	chipInterface;

	BOOLEAN				bInitilized;
	BOOLEAN				bStopCoexDm;
	BOOLEAN				bManualControl;
	pu1Byte				cliBuf;
	BTC_STATISTICS		statistics;
	u1Byte				pwrModeVal[10];

	// function pointers
	// io related
	BFP_BTC_R1			fBtcRead1Byte;
	BFP_BTC_W1			fBtcWrite1Byte;
	BFP_BTC_W1_BIT_MASK	fBtcWrite1ByteBitMask;
	BFP_BTC_R2			fBtcRead2Byte;
	BFP_BTC_W2			fBtcWrite2Byte;
	BFP_BTC_R4			fBtcRead4Byte;
	BFP_BTC_W4			fBtcWrite4Byte;
	// read/write bb related
	BFP_BTC_SET_BB_REG	fBtcSetBbReg;
	BFP_BTC_GET_BB_REG	fBtcGetBbReg;

	// read/write rf related
	BFP_BTC_SET_RF_REG	fBtcSetRfReg;
	BFP_BTC_GET_RF_REG	fBtcGetRfReg;

	//write bt reg related
	BFP_BTC_SET_BT_REG	fBtcSetBtReg;

	// fill h2c related
	BFP_BTC_FILL_H2C		fBtcFillH2c;
	// other
	BFP_BTC_DISP_DBG_MSG	fBtcDispDbgMsg;
	// normal get/set related
	BFP_BTC_GET			fBtcGet;
	BFP_BTC_SET			fBtcSet;
} BTC_COEXIST, *PBTC_COEXIST;

extern BTC_COEXIST				GLBtCoexist;

BOOLEAN
EXhalbtcoutsrc_InitlizeVariables(
	IN	PVOID		Adapter
	);
VOID
EXhalbtcoutsrc_InitHwConfig(
	IN	PBTC_COEXIST		pBtCoexist
	);
VOID
EXhalbtcoutsrc_InitCoexDm(
	IN	PBTC_COEXIST		pBtCoexist
	);
VOID
EXhalbtcoutsrc_IpsNotify(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	u1Byte			type
	);
VOID
EXhalbtcoutsrc_LpsNotify(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	u1Byte			type
	);
VOID
EXhalbtcoutsrc_ScanNotify(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	u1Byte			type
	);
VOID
EXhalbtcoutsrc_ConnectNotify(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	u1Byte			action
	);
VOID
EXhalbtcoutsrc_MediaStatusNotify(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	RT_MEDIA_STATUS	mediaStatus
	);
VOID
EXhalbtcoutsrc_SpecialPacketNotify(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	u1Byte			pktType
	);
VOID
EXhalbtcoutsrc_BtInfoNotify(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	pu1Byte			tmpBuf,
	IN	u1Byte			length
	);
VOID
EXhalbtcoutsrc_StackOperationNotify(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	u1Byte			type
	);
VOID
EXhalbtcoutsrc_HaltNotify(
	IN	PBTC_COEXIST		pBtCoexist
	);
VOID
EXhalbtcoutsrc_SwitchGntBt(
	IN	PBTC_COEXIST		pBtCoexist
	);
VOID
EXhalbtcoutsrc_PnpNotify(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	u1Byte			pnpState
	);
VOID
EXhalbtcoutsrc_DownloadFwNotify(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	u1Byte			DLFwState
	);
VOID
EXhalbtcoutsrc_Periodical(
	IN	PBTC_COEXIST		pBtCoexist
	);
VOID
EXhalbtcoutsrc_DbgControl(
	IN	PBTC_COEXIST			pBtCoexist,
	IN	u1Byte				opCode,
	IN	u1Byte				opLen,
	IN	pu1Byte				pData
	);
VOID
EXhalbtcoutsrc_StackUpdateProfileInfo(VOID);

VOID
EXhalbtcoutsrc_SetHciVersion(
	IN	u2Byte	hciVersion
	);
VOID
EXhalbtcoutsrc_SetBtPatchVersion(
	IN	u2Byte	btHciVersion,
	IN	u2Byte	btPatchVersion
	);
VOID
EXhalbtcoutsrc_UpdateMinBtRssi(
	IN	s1Byte	btRssi
	);
VOID
EXhalbtcoutsrc_SetBtExist(
	IN	BOOLEAN		bBtExist
	);
VOID
EXhalbtcoutsrc_SetChipType(
	IN	u1Byte		chipType
	);
VOID
EXhalbtcoutsrc_SetAntNum(
	IN	u1Byte		type,
	IN	u1Byte		antNum
	);
VOID
EXhalbtcoutsrc_DisplayBtCoexInfo(
	IN	PBTC_COEXIST		pBtCoexist
	);
#endif
