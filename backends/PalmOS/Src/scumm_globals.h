/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
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

#ifndef __SCUMM_GLOBALS_H__
#define __SCUMM_GLOBALS_H__

#include "builder/enum_globals.h"

void *GlbGetRecord(UInt16 index, UInt16 id);
void GlbReleaseRecord(UInt16 index, UInt16 id);
void GlbOpen();
void GlbClose();


#define _GINIT(x) void initGlobals_##x() {
#define _GEND }
#define _GRELEASE(x) void releaseGlobals_##x() {

#define CALL_INIT(x) initGlobals_##x();
#define CALL_RELEASE(x)	releaseGlobals_##x();

#define _GSETPTR(var,index,format,id)	var = (format *)GlbGetRecord(index,id);
#define _GRELEASEPTR(index,id)			GlbReleaseRecord(index,id);

#define PROTO_GLOBALS(x)		void CALL_INIT(x);\
								void CALL_RELEASE(x);

#ifndef DISABLE_SCUMM
PROTO_GLOBALS(IMuseDigital)
PROTO_GLOBALS(NewGui)
PROTO_GLOBALS(Akos)
PROTO_GLOBALS(Bundle)
PROTO_GLOBALS(Codec47)
PROTO_GLOBALS(Gfx)
PROTO_GLOBALS(Dialogs)
PROTO_GLOBALS(Charset)
PROTO_GLOBALS(Costume)
PROTO_GLOBALS(PlayerV2)
#endif

#ifndef DISABLE_SIMON
PROTO_GLOBALS(Simon_Simon)
PROTO_GLOBALS(Simon_Charset)
#endif

#ifndef DISABLE_QUEEN
PROTO_GLOBALS(Queen_Restables)
#endif

#undef PROTO_GLOBALS

#endif
