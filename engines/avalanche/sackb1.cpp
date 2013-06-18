/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

/*
  €ﬂ‹ €ﬂ‹ ‹ﬂﬂ‹  ﬂ€ﬂ €ﬂﬂ  ‹ﬂ ﬂ€ﬂ      ‹ﬂ€ﬂ‹  ﬂ€ﬂ €‹  € ‹€ﬂﬂ  ﬂ€ﬂ €ﬂ‹ €ﬂﬂ €
  €ﬂ  €€  €  € ‹ €  €ﬂﬂ ﬂ‹   €      €  €  €  €  € €‹€  ﬂﬂﬂ‹  €  €€  €ﬂﬂ €
  ﬂ   ﬂ ﬂ  ﬂﬂ   ﬂﬂ  ﬂﬂﬂ   ﬂ  ﬂ      ﬂ  ﬂ  ﬂ ﬂﬂﬂ ﬂ  ﬂﬂ  ﬂﬂﬂ   ﬂ  ﬂ ﬂ ﬂﬂﬂ ﬂﬂﬂ

                 SACKBLASTER-1    The temporary mod player. */

/* This is SackBlaster version 1.0, using Mark J. Cox's MODOBJ routines.
  When Cameron finishes his mod player I'll use his routines, DV. However,
  this will do for the time being. */

#define __sackb1_implementation__


#include "sackb1.h"


/*$L v:MOD-obj.OBJ*/            /* Link in Object file */
/*$F+*/                 /* force calls to be 'far'*/

namespace Avalanche {

extern void modvolume(integer v1, integer v2, integer v3, integer v4); /*Can do while playing*/
extern void moddevice(integer &device);
extern void modsetup(integer &status, integer device, integer mixspeed, integer pro, integer loop, string &stri);
extern void modstop();
extern void modinit();
/*$F-*/

void sb_start(string md) {
	integer dev, mix, stat, pro, loop;

	modinit();
	dev = 7; /* Sound Blaster */
	mix = 10000;    /*use 10000 normally */
	pro = 0;  /*Leave at 0*/
	loop = 4; /*4 means mod will play forever*/
	modvolume(255, 255, 255, 255);  /* Full volume */
	modsetup(stat, dev, mix, pro, loop, md);
}

void sb_stop() {
	;
	modstop();
}

void sb_link() {   /* At the moment, this does nothing. */
	;
}

} // End of namespace Avalanche.