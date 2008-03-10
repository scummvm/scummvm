/* Residual - Virtual machine to run LucasArts' 3D adventure games
 * Copyright (C) 2003-2006 The ScummVM-Residual Team (www.scummvm.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#ifndef IMUSE_TABLES_H
#define IMUSE_TABLES_H

#include "common/sys.h"
#include "common/platform.h"
#include "common/debug.h"

#include "engine/lua.h"

#include "mixer/mixer.h"
#include "mixer/audiostream.h"

#include "engine/imuse/imuse_sndmgr.h"
#include "engine/imuse/imuse_mcmp_mgr.h"

struct ImuseTable {
	byte opcode;
	int16 soundId;
	byte atribPos;
	byte hookId;
	int16 fadeOut60TicksDelay;
	byte volume;
	byte pan;
	char filename[32];
};

#endif
