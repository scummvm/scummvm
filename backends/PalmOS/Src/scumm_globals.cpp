/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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
#include <PalmOS.h>

#include "start.h"
#include "globals.h"
#include "scumm_globals.h"

#include "scumm.h"

static void GlbInitAll() {
#ifndef DISABLE_SCUMM
	if (gVars->globals[GBVARS_SCUMM]) {
		CALL_INIT(IMuseDigital)
		CALL_INIT(NewGui)
		CALL_INIT(Akos)
		CALL_INIT(Bundle)
		CALL_INIT(Codec47)
		CALL_INIT(Gfx)
		CALL_INIT(Dialogs)
		CALL_INIT(Charset)
		CALL_INIT(Costume)
		CALL_INIT(PlayerV2)
	}
#endif
#ifndef DISABLE_SIMON
	if (gVars->globals[GBVARS_SIMON]) {
		CALL_INIT(Simon_Simon)
		CALL_INIT(Simon_Charset)
	}
#endif
#ifndef DISABLE_QUEEN
	if (gVars->globals[GBVARS_QUEEN]) {
		CALL_INIT(Queen_Restables)
	}
#endif
}

static void GlbReleaseAll() {
#ifndef DISABLE_SCUMM
	if (gVars->globals[GBVARS_SCUMM]) {
		CALL_RELEASE(IMuseDigital)
		CALL_RELEASE(NewGui)
		CALL_RELEASE(Akos)
		CALL_RELEASE(Bundle)
		CALL_RELEASE(Codec47)
		CALL_RELEASE(Gfx)
		CALL_RELEASE(Dialogs)
		CALL_RELEASE(Charset)
		CALL_RELEASE(Costume)
		CALL_RELEASE(PlayerV2)
	}
#endif
#ifndef DISABLE_SIMON
	if (gVars->globals[GBVARS_SIMON]) {
		CALL_RELEASE(Simon_Simon)
		CALL_RELEASE(Simon_Charset)
	}
#endif
#ifndef DISABLE_QUEEN
	if (gVars->globals[GBVARS_QUEEN]) {
		CALL_RELEASE(Queen_Restables)
	}
#endif
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
	gVars->globals[GBVARS_SCUMM] = GlbOpenInternal("Scumm-Globals");
	gVars->globals[GBVARS_SIMON] = GlbOpenInternal("Simon-Globals");
	gVars->globals[GBVARS_SKY  ] = GlbOpenInternal("Sky-Globals");
	gVars->globals[GBVARS_QUEEN] = GlbOpenInternal("Queen-Globals");

	GlbInitAll();
}

void GlbClose() {
	GlbReleaseAll();

	if (gVars->globals[GBVARS_SCUMM])
		DmCloseDatabase(gVars->globals[GBVARS_SCUMM]);
	if (gVars->globals[GBVARS_SIMON])
		DmCloseDatabase(gVars->globals[GBVARS_SIMON]);
	if (gVars->globals[GBVARS_SKY])
		DmCloseDatabase(gVars->globals[GBVARS_SKY]);
	if (gVars->globals[GBVARS_QUEEN])
		DmCloseDatabase(gVars->globals[GBVARS_QUEEN]);
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
