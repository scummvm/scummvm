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

// Main Header File

#ifndef SAGA_REINHERIT_H_
#define SAGA_REINHERIT_H_

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/rect.h"

#include "base/engine.h"

namespace Saga {

#define R_MAXPATH 512

#define R_MEMFAIL_MSG "Memory allocation error."

// Define opaque types

// r_rscfile
typedef struct R_RSCFILE_CONTEXT_tag R_RSCFILE_CONTEXT;

// r_script
typedef struct R_SEMAPHORE_tag R_SEMAPHORE;

// Define common data types

typedef Common::Point R_POINT;
typedef Common::Rect R_RECT;

struct R_COLOR {
	int red;
	int green;
	int blue;
	int alpha;
};

struct R_SURFACE {
	byte *buf;
	int buf_w;
	int buf_h;
	int buf_pitch;
	R_RECT clip_rect;
	void *impl_src;
};

#define R_RGB_RED   0x00FF0000UL
#define R_RGB_GREEN 0x0000FF00UL
#define R_RGB_BLUE  0x000000FFUL

struct PALENTRY {
	byte red;
	byte green;
	byte blue;
};

enum R_ERRORCODE {
	R_MEM = -2,
	R_FAILURE = -1,
	R_SUCCESS = 0
};

// r_transitions.c
int TRANSITION_Dissolve(byte *dst_img, int dst_w, int dst_h,
						int dst_p, const byte *src_img, int src_p, int flags, double percent);

// System specific routines

// System : Graphics
#define R_PAL_ENTRIES 256

int GFX_Init(OSystem *system, int width, int height);
R_SURFACE *GFX_GetBackBuffer();
int GFX_GetWhite();
int GFX_GetBlack();
int GFX_MatchColor(unsigned long colormask);
int GFX_SetPalette(R_SURFACE *surface, PALENTRY *pal);
int GFX_GetCurrentPal(PALENTRY *src_pal);
int GFX_PalToBlack(R_SURFACE *surface, PALENTRY *src_pal, double percent);
int GFX_BlackToPal(R_SURFACE *surface, PALENTRY *src_pal, double percent);

// System : Input 
int SYSINPUT_ProcessInput(void);
R_POINT SYSINPUT_GetMousePos();

} // End of namespace Saga

#endif
