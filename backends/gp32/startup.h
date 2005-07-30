/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2004 The ScummVM project
 * Copyright (C) 2002 Ph0x - GP32 Backend
 * Copyright (C) 2003/2004 DJWillis - GP32 Backend
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

#ifndef STARTUP_H
#define STARTUP_H

/////////////////////////////////////////////////////////////////////////////
//
// ScummVM/GP32 - GP32 start-up header.
//                Not part of ScummVM backend as such.
//
/////////////////////////////////////////////////////////////////////////////

//	GP32 SDK Includes.
#include <gpdef.h>
#include <gpstdlib.h>
#include <gpfont.h>
#include <gpfont_port.h>
#include <gpfontres.dat>
#include <initval_port.h>

//	GP32 System startup.

unsigned int HEAPSTART;
unsigned int HEAPEND;
void InitializeFont (void);

#endif /* STARTUP_H */
