/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2004 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef BACKENDS_INTERN_H
#define BACKENDS_INTERN_H

#include "common/system.h"

/* Factory functions. This means we don't have to include the headers for
 * all backends.
 */
extern OSystem *OSystem_SDL_create(int gfx_driver);
extern OSystem *OSystem_NULL_create();
extern OSystem *OSystem_MorphOS_create(int gfx_driver, bool full_screen);
extern OSystem *OSystem_Dreamcast_create();
extern OSystem *OSystem_WINCE3_create();
extern OSystem *OSystem_X11_create();
extern OSystem *OSystem_MAC_create(int gfx_mode, bool full_screen);
extern OSystem *OSystem_GP32_create(int gfx_mode, bool full_screen); //ph0x 
extern OSystem *OSystem_PALMOS_create(int gfx_mode);


#ifdef _WIN32_WCE
#define SAMPLES_PER_SEC_OLD 11025
#define SAMPLES_PER_SEC_NEW 22050
#else
//#define SAMPLES_PER_SEC 11025
#define SAMPLES_PER_SEC 22050
//#define SAMPLES_PER_SEC 44100
#endif

#endif
