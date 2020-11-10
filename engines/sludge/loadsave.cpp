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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/savefile.h"

#include "sludge/allfiles.h"
#include "sludge/backdrop.h"
#include "sludge/bg_effects.h"
#include "sludge/cursors.h"
#include "sludge/event.h"
#include "sludge/floor.h"
#include "sludge/fonttext.h"
#include "sludge/function.h"
#include "sludge/graphics.h"
#include "sludge/language.h"
#include "sludge/loadsave.h"
#include "sludge/moreio.h"
#include "sludge/newfatal.h"
#include "sludge/objtypes.h"
#include "sludge/people.h"
#include "sludge/region.h"
#include "sludge/savedata.h"
#include "sludge/sludge.h"
#include "sludge/sludger.h"
#include "sludge/sound.h"
#include "sludge/sprites.h"
#include "sludge/statusba.h"
#include "sludge/speech.h"
#include "sludge/variable.h"
#include "sludge/version.h"
#include "sludge/zbuffer.h"

namespace Sludge {

//----------------------------------------------------------------------
// From elsewhere
//----------------------------------------------------------------------

extern LoadedFunction *saverFunc;					// In function.cpp
extern LoadedFunction *allRunningFunctions;         // In sludger.cpp
extern int numGlobals;                              // In sludger.cpp
extern Variable *globalVars;                        // In sludger.cpp
extern FILETIME fileTime;                           // In sludger.cpp
extern bool allowAnyFilename;

//----------------------------------------------------------------------
// Save everything
//----------------------------------------------------------------------

bool handleSaveLoad() {
	if (!g_sludge->loadNow.empty()) {
		if (g_sludge->loadNow[0] == ':') {
			saveGame(g_sludge->loadNow.c_str() + 1);
			saverFunc->reg.setVariable(SVT_INT, 1);
		} else {
			if (!loadGame(g_sludge->loadNow))
				return false;
		}
		g_sludge->loadNow.clear();
	}
	return true;
}

bool saveGame(const Common::String &fname) {
	Common::OutSaveFile *fp = g_system->getSavefileManager()->openForSaving(fname);

	if (fp == NULL)
		return false;

	fp->writeString("SLUDSA");
	fp->writeByte(0);
	fp->writeByte(0);
	fp->writeByte(MAJOR_VERSION);
	fp->writeByte(MINOR_VERSION);

	if (!g_sludge->_gfxMan->saveThumbnail(fp))
		return false;

	fp->write(&fileTime, sizeof(FILETIME));

	// DON'T ADD ANYTHING NEW BEFORE THIS POINT!

	fp->writeByte(allowAnyFilename);
	fp->writeByte(false); // deprecated captureAllKeys
	fp->writeByte(true);
	g_sludge->_txtMan->saveFont(fp);

	// Save backdrop
	g_sludge->_gfxMan->saveBackdrop(fp);

	// Save event handlers
	g_sludge->_evtMan->saveHandlers(fp);

	// Save regions
	g_sludge->_regionMan->saveRegions(fp);

	g_sludge->_cursorMan->saveCursor(fp);

	// Save functions
	LoadedFunction *thisFunction = allRunningFunctions;
	int countFunctions = 0;
	while (thisFunction) {
		countFunctions++;
		thisFunction = thisFunction->next;
	}
	fp->writeUint16BE(countFunctions);

	thisFunction = allRunningFunctions;
	while (thisFunction) {
		saveFunction(thisFunction, fp);
		thisFunction = thisFunction->next;
	}

	for (int a = 0; a < numGlobals; a++) {
		globalVars[a].save(fp);
	}

	g_sludge->_peopleMan->savePeople(fp);

	g_sludge->_floorMan->save(fp);

	g_sludge->_gfxMan->saveZBuffer(fp);
	g_sludge->_gfxMan->saveLightMap(fp);

	g_sludge->_speechMan->save(fp);
	saveStatusBars(fp);
	g_sludge->_soundMan->saveSounds(fp);

	fp->writeUint16BE(CustomSaveHelper::_saveEncoding);

	blur_saveSettings(fp);

	g_sludge->_gfxMan->saveColors(fp);

	g_sludge->_gfxMan->saveParallax(fp);
	fp->writeByte(0);

	g_sludge->_languageMan->saveLanguageSetting(fp);

	g_sludge->_gfxMan->saveSnapshot(fp);

	fp->flush();
	fp->finalize();
	delete fp;

	clearStackLib();
	return true;
}

//----------------------------------------------------------------------
// Load everything
//----------------------------------------------------------------------

int ssgVersion;

bool loadGame(const Common::String &fname) {
	Common::InSaveFile *fp = g_system->getSavefileManager()->openForLoading(fname);
	FILETIME savedGameTime;

	while (allRunningFunctions)
		finishFunction(allRunningFunctions);

	if (fp == NULL)
		return false;

	bool headerBad = false;
	if (fp->readByte() != 'S')
		headerBad = true;
	if (fp->readByte() != 'L')
		headerBad = true;
	if (fp->readByte() != 'U')
		headerBad = true;
	if (fp->readByte() != 'D')
		headerBad = true;
	if (fp->readByte() != 'S')
		headerBad = true;
	if (fp->readByte() != 'A')
		headerBad = true;
	if (headerBad) {
		fatal(ERROR_GAME_LOAD_NO, fname);
		return NULL;
	}
	char c;
	c = fp->readByte();
	while ((c = fp->readByte()))
		;

	int majVersion = fp->readByte();
	int minVersion = fp->readByte();
	ssgVersion = VERSION(majVersion, minVersion);

	if (ssgVersion >= VERSION(1, 4)) {
		if (!g_sludge->_gfxMan->skipThumbnail(fp))
			return fatal(ERROR_GAME_LOAD_CORRUPT, fname);
	}

	uint32 bytes_read = fp->read(&savedGameTime, sizeof(FILETIME));
	if (bytes_read != sizeof(FILETIME) && fp->err()) {
		warning("Reading error in loadGame.");
	}

	if (savedGameTime.dwLowDateTime != fileTime.dwLowDateTime || savedGameTime.dwHighDateTime != fileTime.dwHighDateTime) {
		return fatal(ERROR_GAME_LOAD_WRONG, fname);
	}

	// DON'T ADD ANYTHING NEW BEFORE THIS POINT!

	if (ssgVersion >= VERSION(1, 4)) {
		allowAnyFilename = fp->readByte();
	}
	fp->readByte(); // deprecated captureAllKeys
	fp->readByte(); // updateDisplay (part of movie playing)

	g_sludge->_txtMan->loadFont(ssgVersion, fp);

	g_sludge->_regionMan->kill();

	g_sludge->_gfxMan->loadBackdrop(ssgVersion, fp);

	g_sludge->_evtMan->loadHandlers(fp);

	g_sludge->_regionMan->loadRegions(fp);

	if (!g_sludge->_cursorMan->loadCursor(fp)) {
		return false;
	}

	LoadedFunction *rFunc;
	LoadedFunction **buildList = &allRunningFunctions;

	int countFunctions = fp->readUint16BE();
	while (countFunctions--) {
		rFunc = loadFunction(fp);
		rFunc->next = NULL;
		(*buildList) = rFunc;
		buildList = &(rFunc->next);
	}

	for (int a = 0; a < numGlobals; a++) {
		globalVars[a].unlinkVar();
		globalVars[a].load(fp);
	}

	g_sludge->_peopleMan->loadPeople(fp);

	if (!g_sludge->_floorMan->load(fp)) {
		return false;
	}

	if (!g_sludge->_gfxMan->loadZBuffer(fp))
		return false;

	if (!g_sludge->_gfxMan->loadLightMap(ssgVersion, fp)) {
		return false;
	}

	g_sludge->_speechMan->load(fp);
	loadStatusBars(fp);
	g_sludge->_soundMan->loadSounds(fp);

	CustomSaveHelper::_saveEncoding = fp->readUint16BE();

	if (ssgVersion >= VERSION(1, 6)) {
		if (ssgVersion < VERSION(2, 0)) {
			// aaLoad
			fp->readByte();
			fp->readFloatLE();
			fp->readFloatLE();
		}

		blur_loadSettings(fp);
	}

	if (ssgVersion >= VERSION(1, 3)) {
		g_sludge->_gfxMan->loadColors(fp);

		// Read parallax layers
		while (fp->readByte()) {
			int im = fp->readUint16BE();
			int fx = fp->readUint16BE();
			int fy = fp->readUint16BE();

			if (!g_sludge->_gfxMan->loadParallax(im, fx, fy))
				return false;
		}

		g_sludge->_languageMan->loadLanguageSetting(fp);
	}

	g_sludge->_gfxMan->nosnapshot();
	if (ssgVersion >= VERSION(1, 4)) {
		if (fp->readByte()) {
			if (!g_sludge->_gfxMan->restoreSnapshot(fp))
				return false;
		}
	}

	delete fp;

	clearStackLib();
	return true;
}

} // End of namespace Sludge
