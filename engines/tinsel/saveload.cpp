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
 * $URL$
 * $Id$
 *
 * Save and restore scene and game.
 */

#include "tinsel/actors.h"
#include "tinsel/dw.h"
#include "tinsel/inventory.h"
#include "tinsel/rince.h"
#include "tinsel/savescn.h"
#include "tinsel/serializer.h"
#include "tinsel/timers.h"
#include "tinsel/tinlib.h"
#include "tinsel/tinsel.h"

#include "common/savefile.h"

namespace Tinsel {


/**
 * The current savegame format version.
 * Our save/load system uses an elaborate scheme to allow us to modify the
 * savegame while keeping full backward compatibility, in the sense that newer
 * ScummVM versions always are able to load old savegames.
 * In order to achieve that, we store a version in the savegame files, and whenever
 * the savegame layout is modified, the version is incremented.
 *
 * This roughly works by marking each savegame entry with a range of versions
 * for which it is valid; the save/load code iterates over all entries, but
 * only saves/loads those which are valid for the version of the savegame
 * which is being loaded/saved currently.
 */
#define CURRENT_VER 1
// TODO: Not yet used

/**
 * An auxillary macro, used to specify savegame versions. We use this instead
 * of just writing the raw version, because this way they stand out more to
 * the reading eye, making it a bit easier to navigate through the code.
 */
#define VER(x) x




//----------------- EXTERN FUNCTIONS --------------------

// in DOS_DW.C
extern void syncSCdata(Serializer &s);

// in DOS_MAIN.C
//char HardDriveLetter(void);

// in PCODE.C
extern void syncGlobInfo(Serializer &s);

// in POLYGONS.C
extern void syncPolyInfo(Serializer &s);

// in SAVESCN.CPP
extern void RestoreScene(SAVED_DATA *sd, bool bFadeOut);

//----------------- LOCAL DEFINES --------------------

struct SaveGameHeader {
	uint32 id;
	uint32 size;
	uint32 ver;
	char desc[SG_DESC_LEN];
	struct tm dateTime;
};

enum {
	SAVEGAME_ID = 0x44575399,	// = 'DWSc' = "DiscWorld ScummVM"
	SAVEGAME_HEADER_SIZE = 4 + 4 + 4 + SG_DESC_LEN + 7
};


//----------------- LOCAL GLOBAL DATA --------------------

static int	numSfiles = 0;
static SFILES	savedFiles[MAX_SFILES];

static bool NeedLoad = true;

static SAVED_DATA *srsd = 0;
static int RestoreGameNumber = 0;
static char *SaveSceneName = 0;
static const char *SaveSceneDesc = 0;
static int *SaveSceneSsCount = 0;
static char *SaveSceneSsData = 0;	// points to 'SAVED_DATA ssdata[MAX_NEST]'

static SRSTATE SRstate = SR_IDLE;

//------------- SAVE/LOAD SUPPORT METHODS ----------------

static void syncTime(Serializer &s, struct tm &t) {
	s.syncAsUint16LE(t.tm_year);
	s.syncAsByte(t.tm_mon);
	s.syncAsByte(t.tm_mday);
	s.syncAsByte(t.tm_hour);
	s.syncAsByte(t.tm_min);
	s.syncAsByte(t.tm_sec);
	if (s.isLoading()) {
		t.tm_wday = 0;
		t.tm_yday = 0;
		t.tm_isdst = 0;
	}
}

static bool syncSaveGameHeader(Serializer &s, SaveGameHeader &hdr) {
	s.syncAsUint32LE(hdr.id);
	s.syncAsUint32LE(hdr.size);
	s.syncAsUint32LE(hdr.ver);

	s.syncBytes((byte *)hdr.desc, SG_DESC_LEN);
	hdr.desc[SG_DESC_LEN - 1] = 0;

	syncTime(s, hdr.dateTime);

	int tmp = hdr.size - s.bytesSynced();
	// Perform sanity check
	if (tmp < 0 || hdr.id != SAVEGAME_ID || hdr.ver > CURRENT_VER || hdr.size > 1024)
		return false;
	// Skip over any extra bytes
	while (tmp-- > 0) {
		byte b = 0;
		s.syncAsByte(b);
	}
	return true;
}

static void syncSavedMover(Serializer &s, SAVED_MOVER &sm) {
	SCNHANDLE *pList[3] = { (SCNHANDLE *)&sm.WalkReels, (SCNHANDLE *)&sm.StandReels, (SCNHANDLE *)&sm.TalkReels };

	s.syncAsUint32LE(sm.MActorState);
	s.syncAsSint32LE(sm.actorID);
	s.syncAsSint32LE(sm.objx);
	s.syncAsSint32LE(sm.objy);
	s.syncAsUint32LE(sm.lastfilm);
	
	for (int pIndex = 0; pIndex < 3; ++pIndex) {
		SCNHANDLE *p = pList[pIndex];
		for (int i = 0; i < TOTAL_SCALES * 4; ++i)
			s.syncAsUint32LE(*p++);
	}
}

static void syncSavedActor(Serializer &s, SAVED_ACTOR &sa) {
	s.syncAsUint16LE(sa.actorID);
	s.syncAsUint16LE(sa.z);
	s.syncAsUint32LE(sa.bAlive);
	s.syncAsUint32LE(sa.presFilm);
	s.syncAsUint16LE(sa.presRnum);
	s.syncAsUint16LE(sa.presX);
	s.syncAsUint16LE(sa.presY);
}

extern void syncAllActorsAlive(Serializer &s);

static void syncNoScrollB(Serializer &s, NOSCROLLB &ns) {
	s.syncAsSint32LE(ns.ln);
	s.syncAsSint32LE(ns.c1);
	s.syncAsSint32LE(ns.c2);
}

static void syncSavedData(Serializer &s, SAVED_DATA &sd) {
	s.syncAsUint32LE(sd.SavedSceneHandle);
	s.syncAsUint32LE(sd.SavedBgroundHandle);
	for (int i = 0; i < MAX_MOVERS; ++i)
		syncSavedMover(s, sd.SavedMoverInfo[i]);
	for (int i = 0; i < MAX_SAVED_ACTORS; ++i)
		syncSavedActor(s, sd.SavedActorInfo[i]);

	s.syncAsSint32LE(sd.NumSavedActors);
	s.syncAsSint32LE(sd.SavedLoffset);
	s.syncAsSint32LE(sd.SavedToffset);
	for (int i = 0; i < MAX_INTERPRET; ++i)
		sd.SavedICInfo[i].syncWithSerializer(s);
	for (int i = 0; i < MAX_POLY; ++i)
		s.syncAsUint32LE(sd.SavedDeadPolys[i]);
	s.syncAsUint32LE(sd.SavedControl);
	s.syncAsUint32LE(sd.SavedMidi);
	s.syncAsUint32LE(sd.SavedLoop);
	s.syncAsUint32LE(sd.SavedNoBlocking);

	// SavedNoScrollData
	for (int i = 0; i < MAX_VNOSCROLL; ++i)
		syncNoScrollB(s, sd.SavedNoScrollData.NoVScroll[i]);
	for (int i = 0; i < MAX_HNOSCROLL; ++i)
		syncNoScrollB(s, sd.SavedNoScrollData.NoHScroll[i]);
	s.syncAsUint32LE(sd.SavedNoScrollData.NumNoV);
	s.syncAsUint32LE(sd.SavedNoScrollData.NumNoH);
}


/**
 * Called when saving a game to a new file.
 * Generates a new, unique, filename.
 */
static char *NewName(void) {
	static char result[FNAMELEN];
	int	i;
	int	ano = 1;	// Allocated number
	
	while (1) {
		Common::String fname = _vm->getSavegameFilename(ano);
		strcpy(result, fname.c_str());

		for (i = 0; i < numSfiles; i++)
			if (!strcmp(savedFiles[i].name, result))
				break;

		if (i == numSfiles)
			break;
		ano++;
	}

	return result;
}

/**
 * Interrogate the current DOS directory for saved game files.
 * Store the file details, ordered by time, in savedFiles[] and return
 * the number of files found).
 */
int getList(void) {
	// No change since last call?
	// TODO/FIXME: Just always reload this data? Be careful about slow downs!!!
	if (!NeedLoad)
		return numSfiles;

	int i;

	const Common::String pattern = _vm->getSavegamePattern();
	Common::StringList files = _vm->getSaveFileMan()->listSavefiles(pattern.c_str());

	numSfiles = 0;

	for (Common::StringList::const_iterator file = files.begin(); file != files.end(); ++file) {
		if (numSfiles >= MAX_SFILES)
			break;

		const Common::String &fname = *file;
		Common::InSaveFile *f = _vm->getSaveFileMan()->openForLoading(fname.c_str());
		if (f == NULL) {
			continue;
		}

		// Try to load save game header
		Serializer s(f, 0);
		SaveGameHeader hdr;
		bool validHeader = syncSaveGameHeader(s, hdr);
		delete f;
		if (!validHeader) {
			continue;	// Invalid header, or savegame too new -> skip it
			// TODO: In SCUMM, we still show an entry for the save, but with description
			// "incompatible version".
		}

		i = numSfiles;
#ifndef DISABLE_SAVEGAME_SORTING
		for (i = 0; i < numSfiles; i++) {
			if (difftime(mktime(&hdr.dateTime), mktime(&savedFiles[i].dateTime)) > 0) {
				Common::copy_backward(&savedFiles[i], &savedFiles[numSfiles], &savedFiles[numSfiles + 1]);
				break;
			} 
		}
#endif

		strncpy(savedFiles[i].name, fname.c_str(), FNAMELEN);
		strncpy(savedFiles[i].desc, hdr.desc, SG_DESC_LEN);
		savedFiles[i].desc[SG_DESC_LEN - 1] = 0;
		savedFiles[i].dateTime = hdr.dateTime;

		++numSfiles;
	}

	// Next getList() needn't do its stuff again
	NeedLoad = false;

	return numSfiles;
}


char *ListEntry(int i, letype which) {
	if (i == -1)
		i = numSfiles;

	assert(i >= 0);

	if (i < numSfiles)
		return which == LE_NAME ? savedFiles[i].name : savedFiles[i].desc;
	else
		return NULL;
}

static void DoSync(Serializer &s) {
	int	sg;

	syncSavedData(s, *srsd);
	syncGlobInfo(s);		// Glitter globals
	syncInvInfo(s);			// Inventory data

	// Held object
	if (s.isSaving())
		sg = WhichItemHeld();
	s.syncAsSint32LE(sg);
	if (s.isLoading())
		HoldItem(sg);

	syncTimerInfo(s);		// Timer data
	syncPolyInfo(s);		// Dead polygon data
	syncSCdata(s);			// Hook Scene and delayed scene

	s.syncAsSint32LE(*SaveSceneSsCount);

	if (*SaveSceneSsCount != 0) {
		SAVED_DATA *sdPtr = (SAVED_DATA *)SaveSceneSsData;
		for (int i = 0; i < *SaveSceneSsCount; ++i, ++sdPtr)
			syncSavedData(s, *sdPtr);
	}

	syncAllActorsAlive(s);
}

/**
 * DoRestore
 */
static bool DoRestore(void) {
	Common::InSaveFile *f;
	uint32 id;

	f = _vm->getSaveFileMan()->openForLoading(savedFiles[RestoreGameNumber].name);
	if (f == NULL) {
		return false;
	}

	Serializer s(f, 0);
	SaveGameHeader hdr;
	if (!syncSaveGameHeader(s, hdr)) {
		delete f;	// Invalid header, or savegame too new -> skip it
		return false;
	}

	DoSync(s);

	id = f->readSint32LE();
	if (id != (uint32)0xFEEDFACE)
		error("Incompatible saved game");

	bool failed = f->ioFailed();

	delete f;

	return !failed;
}

/**
 * DoSave
 */
static void DoSave(void) {
	Common::OutSaveFile *f;
	const char *fname;

	// Next getList() must do its stuff again
	NeedLoad = true;

	if (SaveSceneName == NULL)
		SaveSceneName = NewName();
	if (SaveSceneDesc[0] == 0)
		SaveSceneDesc = "unnamed";

	fname = SaveSceneName;

	f = _vm->getSaveFileMan()->openForSaving(fname);
	if (f == NULL)
		return;

	Serializer s(0, f);
	
	// Write out a savegame header
	SaveGameHeader hdr;
	hdr.id = SAVEGAME_ID;
	hdr.size = SAVEGAME_HEADER_SIZE;
	hdr.ver = CURRENT_VER;
	memcpy(hdr.desc, SaveSceneDesc, SG_DESC_LEN);
	hdr.desc[SG_DESC_LEN - 1] = 0;
	g_system->getTimeAndDate(hdr.dateTime);
	if (!syncSaveGameHeader(s, hdr) || f->ioFailed()) {
		goto save_failure;
	}

	DoSync(s);

	// Write out the special Id for Discworld savegames
	f->writeUint32LE(0xFEEDFACE);
	if (f->ioFailed())
		goto save_failure;

	f->finalize();
	delete f;
	return;

save_failure:
	delete f;
	_vm->getSaveFileMan()->removeSavefile(fname);
}

/**
 * ProcessSRQueue
 */
void ProcessSRQueue(void) {
	switch (SRstate) {
	case SR_DORESTORE:
		if (DoRestore()) {
			RestoreScene(srsd, false);
		}
		SRstate = SR_IDLE;
		break;

	case SR_DOSAVE:
		DoSave();
		SRstate = SR_IDLE;
		break;
	default:
		break;
	}
}


void RequestSaveGame(char *name, char *desc, SAVED_DATA *sd, int *pSsCount, SAVED_DATA *pSsData) {
	assert(SRstate == SR_IDLE);

	SaveSceneName = name;
	SaveSceneDesc = desc;
	SaveSceneSsCount = pSsCount;
	SaveSceneSsData = (char *)pSsData;
	srsd = sd;
	SRstate = SR_DOSAVE;
}

void RequestRestoreGame(int num, SAVED_DATA *sd, int *pSsCount, SAVED_DATA *pSsData) {
	assert(num >= 0);

	RestoreGameNumber = num;
	SaveSceneSsCount = pSsCount;
	SaveSceneSsData = (char *)pSsData;
	srsd = sd;
	SRstate = SR_DORESTORE;
}

} // end of namespace Tinsel
