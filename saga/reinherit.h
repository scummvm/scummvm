/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
/*
 Description:   
 
    Main Header File

 Notes: 
*/

#ifndef SAGA_REINHERIT_H_
#define SAGA_REINHERIT_H_

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/rect.h"

#include "base/engine.h"

/*
 * Implementation conditionals
\*--------------------------------------------------------------------------*/
#define R_ENV_LINUX
#include "sys_interface.h"

namespace Saga {

#define R_MAXPATH 512

/* For debug message processing */
#define R_DEBUG_NONE 0
#define R_DEBUG_INFO 1
#define R_DEBUG_VERBOSE 2
#define R_DEBUG_PARANOID 3

#define R_MEMFAIL_MSG "Memory allocation error."

/*
 * Define opaque types
\*--------------------------------------------------------------------------*/

/* r_rscfile */
typedef struct R_RSCFILE_CONTEXT_tag R_RSCFILE_CONTEXT;

/* r_script */
typedef struct R_SEMAPHORE_tag R_SEMAPHORE;

/*
 * Define common data types
\*--------------------------------------------------------------------------*/

#ifndef HAVE_UCHAR
typedef unsigned char uchar;
#endif
#ifndef HAVE_ULONG
typedef unsigned long ulong;
#endif
#ifndef HAVE_UINT
typedef unsigned int uint;
#endif

typedef Common::Point R_POINT;
typedef Common::Rect R_RECT;


struct R_COLOR {

	int red;
	int green;
	int blue;
	int alpha;

};

struct R_SURFACE {

	uchar *buf;
	int buf_w;
	int buf_h;
	int buf_pitch;

	int bpp;

	R_RECT clip_rect;

	void *impl_src;

};

struct R_SOUNDBUFFER {

	const uchar *res_data;
	size_t res_len;

	long s_freq;
	int s_samplebits;
	int s_stereo;
	int s_signed;

	const uchar *s_buf;
	size_t s_buf_len;

};

#define R_RGB_RED   0x00FF0000UL
#define R_RGB_GREEN 0x0000FF00UL
#define R_RGB_BLUE  0x000000FFUL

struct SAGA_COLOR {

	byte r;
	byte g;
	byte b;

};

#define SAGA_COLOR_LEN 3

struct PALENTRY {

	byte red;
	byte green;
	byte blue;

};

enum R_ERRORCODE {

	R_STOP = -3,
	R_MEM = -2,
	R_FAILURE = -1,
	R_SUCCESS = 0

};


/*
 * r_main.c
\*--------------------------------------------------------------------------*/

void R_Shutdown(int param);

/*
 * r_transitions.c
\*--------------------------------------------------------------------------*/
int TRANSITION_Dissolve(uchar * dst_img,
    int dst_w,
    int dst_h,
    int dst_p, const uchar * src_img, int src_p, int flags, double percent);

/*--------------------------------------------------------------------------*\
 * System specific routines
\*--------------------------------------------------------------------------*/

/*
 * System : Sound
\*--------------------------------------------------------------------------*/
int SYSSOUND_Init(int enabled);
int SYSSOUND_Shutdown(void);

int SYSSOUND_Play(int sound_rn, int channel);
int SYSSOUND_Pause(int channel);
int SYSSOUND_Resume(int channel);
int SYSSOUND_Stop(int channel);

int SYSSOUND_PlayVoice(R_SOUNDBUFFER *);
int SYSSOUND_PauseVoice(void);
int SYSSOUND_ResumeVoice(void);
int SYSSOUND_StopVoice(void);

/*
 * System : Music
\*--------------------------------------------------------------------------*/
enum SYSMUSIC_FLAGS {

	R_MUSIC_LOOP = 0x01
};

int SYSMUSIC_Init(int enabled);
int SYSMUSIC_Shutdown(void);

int SYSMUSIC_Play(ulong music_rn, uint flags);
int SYSMUSIC_Pause(void);
int SYSMUSIC_Resume(void);
int SYSMUSIC_Stop(void);

/*
 * System : Graphics
\*--------------------------------------------------------------------------*/
#define R_PAL_ENTRIES 256

struct R_SYSGFX_INIT {

	int backbuf_w;
	int backbuf_h;
	int backbuf_bpp;

	int screen_w;
	int screen_h;
	int screen_bpp;

	int fullscreen;

};

int SYSGFX_Init(R_SYSGFX_INIT *);

R_SURFACE *SYSGFX_GetScreenSurface(void);
R_SURFACE *SYSGFX_GetBackBuffer(void);

int SYSGFX_LockSurface(R_SURFACE * surface);
int SYSGFX_UnlockSurface(R_SURFACE * surface);

R_SURFACE *SYSGFX_CreateSurface(int w, int h, int bpp);
R_SURFACE *SYSGFX_FormatToDisplay(R_SURFACE * surface);
int SYSGFX_DestroySurface(R_SURFACE * surface);

int SYSGFX_GetWhite(void);
int SYSGFX_GetBlack(void);
int SYSGFX_MatchColor(unsigned long colormask);
int SYSGFX_SetPalette(R_SURFACE * surface, PALENTRY * pal);
int SYSGFX_GetCurrentPal(PALENTRY * src_pal);

int SYSGFX_PalToBlack(R_SURFACE * surface, PALENTRY * src_pal, double percent);

int SYSGFX_BlackToPal(R_SURFACE * surface, PALENTRY * src_pal, double percent);

/*
 * System : Input 
\*--------------------------------------------------------------------------*/
int SYSINPUT_Init(void);
int SYSINPUT_ProcessInput(void);
int SYSINPUT_GetMousePos(int *mouse_x, int *mouse_y);
int SYSINPUT_HideMouse(void);
int SYSINPUT_ShowMouse(void);

/*** sys_signal.c ***/
int ITESYS_CheckSignal(void);

} // End of namespace Saga

#endif				/* SAGA_REINHERIT_H_ */
