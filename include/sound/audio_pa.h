/*
 * audio_pa.h
 *
 * Copyright (C) 2012 SpreadTrum Ltd.
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
#ifndef __SOUND_AUDIO_PA
#define __SOUND_AUDIO_PA
#include <linux/platform_device.h>
typedef int (*audio_pa_callback)(int, void *);
typedef int (*audio_pa_init)(void);
typedef struct {
	audio_pa_init init;
	audio_pa_callback control;
} _audio_pa_callback, *paudio_pa_callback;
typedef struct {
	_audio_pa_callback speaker;
	_audio_pa_callback earpiece;
	_audio_pa_callback headset;
} _audio_pa_control, *paudio_pa_control;

extern paudio_pa_control audio_pa_amplifier;

typedef void (*audio_switch_hook_func)(void);
typedef int (*audio_switch_hook_init)(void);

struct audio_switch_hook {
    audio_switch_hook_init      init;
    audio_switch_hook_func      speaker;
    audio_switch_hook_func      earpiece;
};      
        
int audio_switch_hook_register(struct audio_switch_hook *hook);

#endif
