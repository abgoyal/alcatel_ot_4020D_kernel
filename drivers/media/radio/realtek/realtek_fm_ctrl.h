
#ifndef __REALTEK_FM_H
#define __REALTEK_FM_H


#define REALTEK_FM_DEBUG 1
#ifdef REALTEK_FM_DEBUG
#define dbg_info(args...) printk(args)
#else
#define dbg_info(args...)
#endif


#define FM_FUNCTION_SUCCESS 0 
#define FM_FUNCTION_ERROR -EINVAL
#define FM_FUNCTION_FULL_SEARCH_ERROR -EAGAIN

//#define	TROUT_FM_DEV_NAME	"trout_fm"
#define	TROUT_FM_DEV_NAME	"radio0"

/* seek direction */
#define Trout_SEEK_DIR_UP          0
#define Trout_SEEK_DIR_DOWN        1

/** The following define the IOCTL command values via the ioctl macros */
#define	Trout_FM_IOCTL_BASE     'R'
#define	Trout_FM_IOCTL_ENABLE		 _IOW(Trout_FM_IOCTL_BASE, 0, int)
#define Trout_FM_IOCTL_GET_ENABLE  _IOW(Trout_FM_IOCTL_BASE, 1, int)
#define Trout_FM_IOCTL_SET_TUNE    _IOW(Trout_FM_IOCTL_BASE, 2, int)
#define Trout_FM_IOCTL_GET_FREQ    _IOW(Trout_FM_IOCTL_BASE, 3, int)
#define Trout_FM_IOCTL_SEARCH      _IOW(Trout_FM_IOCTL_BASE, 4, int[4])
#define Trout_FM_IOCTL_STOP_SEARCH _IOW(Trout_FM_IOCTL_BASE, 5, int)
#define Trout_FM_IOCTL_MUTE        _IOW(Trout_FM_IOCTL_BASE, 6, int)
#define Trout_FM_IOCTL_SET_VOLUME  _IOW(Trout_FM_IOCTL_BASE, 7, int)
#define Trout_FM_IOCTL_GET_VOLUME  _IOW(Trout_FM_IOCTL_BASE, 8, int)

//chenq add a ioctl cmd for deal i2s work,in songkun mail,2013-01-17
#define Trout_FM_IOCTL_CONFIG  _IOW(Trout_FM_IOCTL_BASE, 9, int)
//chenq add end



#endif



