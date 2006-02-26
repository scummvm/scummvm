/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
 * Copyright (C) 2002-2006 Chris Apers - PalmOS Backend
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
 * $URL$
 * $Id$
 *
 */

#include <PalmOS.h>

#include "palmdefs.h"
#include "globals.h"
#include "scumm_globals.h"

static void GlbInitAll() {
	if (gVars->globals[GBVARS_COMMON]) {
		CALL_INIT(ScummFont)
	}
	if (gVars->globals[GBVARS_ENGINE]) {
#ifndef DISABLE_SCUMM
#	ifndef DISABLE_SCUMM_7_8
		CALL_INIT(DimuseTables)
		CALL_INIT(DimuseCodecs)
		CALL_INIT(Codec47)
#	endif
		CALL_INIT(Akos)
		CALL_INIT(Gfx)
		CALL_INIT(Dialogs)
		CALL_INIT(Charset)
		CALL_INIT(Costume)
		CALL_INIT(PlayerV2)
		CALL_INIT(Scumm_md5table)
#endif
#ifndef DISABLE_SIMON
		CALL_INIT(Simon_Simon)
		CALL_INIT(Simon_Cursor)
		CALL_INIT(Simon_Charset)
#endif
#ifndef DISABLE_SKY
		CALL_INIT(Sky_Hufftext)
#endif
#ifndef DISABLE_SWORD1
		CALL_INIT(Sword1_fxList)
#endif
#ifndef DISABLE_QUEEN
		CALL_INIT(Queen_Talk)
		CALL_INIT(Queen_Display)
		CALL_INIT(Queen_Graphics)
		CALL_INIT(Queen_Restables)
		CALL_INIT(Queen_Musicdata)
#endif
	}
}

static void GlbReleaseAll() {
	if (gVars->globals[GBVARS_COMMON]) {
		CALL_RELEASE(ScummFont)

	}
	if (gVars->globals[GBVARS_SCUMM]) {
#ifndef DISABLE_SCUMM
#	ifndef DISABLE_SCUMM_7_8
		CALL_RELEASE(DimuseTables)
		CALL_RELEASE(DimuseCodecs)
		CALL_RELEASE(Codec47)
#	endif
		CALL_RELEASE(Akos)
		CALL_RELEASE(Gfx)
		CALL_RELEASE(Dialogs)
		CALL_RELEASE(Charset)
		CALL_RELEASE(Costume)
		CALL_RELEASE(PlayerV2)
		CALL_RELEASE(Scumm_md5table)
#endif
#ifndef DISABLE_SIMON
		CALL_RELEASE(Simon_Simon)
		CALL_RELEASE(Simon_Cursor)
		CALL_RELEASE(Simon_Charset)
#endif
#ifndef DISABLE_SKY
		CALL_RELEASE(Sky_Hufftext)
#endif
#ifndef DISABLE_SWORD1
		CALL_RELEASE(Sword1_fxList)
#endif
#ifndef DISABLE_QUEEN
		CALL_RELEASE(Queen_Talk)
		CALL_RELEASE(Queen_Display)
		CALL_RELEASE(Queen_Graphics)
		CALL_RELEASE(Queen_Restables)
		CALL_RELEASE(Queen_Musicdata)
#endif
	}
}

//TODO : use Boolean instead of void to check err
static DmOpenRef GlbOpenInternal(const Char *nameP) {
	LocalID dbID = DmFindDatabase(0, nameP);
	if (dbID) {
		UInt32 dbType, dbCreator;
		Err e = DmDatabaseInfo(0, dbID, 0, 0, 0, 0, 0, 0, 0, 0, 0, &dbType, &dbCreator);

		if (!e && dbType == 'GLBS' && dbCreator == appFileCreator)
			return DmOpenDatabase(0, dbID, dmModeReadOnly);
	}
	return NULL;
}

void GlbOpen() {
	gVars->globals[GBVARS_COMMON] = GlbOpenInternal("Glbs::Common");
	gVars->globals[GBVARS_ENGINE] = GlbOpenInternal("Glbs::Engine");
	GlbInitAll();
}

void GlbClose() {
	GlbReleaseAll();

	if (gVars->globals[GBVARS_COMMON])
		DmCloseDatabase(gVars->globals[GBVARS_COMMON]);
	if (gVars->globals[GBVARS_ENGINE])
		DmCloseDatabase(gVars->globals[GBVARS_ENGINE]);
}

void *GlbGetRecord(UInt16 index, UInt16 id) {
	if (gVars->globals[id]) {
		MemHandle recordH = DmQueryRecord(gVars->globals[id], index);
		if (recordH)
			return MemHandleLock(recordH);
	}
	return NULL;
}

void GlbReleaseRecord(UInt16 index, UInt16 id) {
	if (gVars->globals[id]) {
		MemHandle recordH = DmQueryRecord(gVars->globals[id], index);
		if (recordH)
			MemHandleUnlock(recordH);
	}
}
