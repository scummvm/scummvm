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

#include "common/config-manager.h"
#include "common/debug.h"

#include "sludge/allfiles.h"
#include "sludge/backdrop.h"
#include "sludge/builtin.h"
#include "sludge/cursors.h"
#include "sludge/event.h"
#include "sludge/fonttext.h"
#include "sludge/freeze.h"
#include "sludge/floor.h"
#include "sludge/fileset.h"
#include "sludge/function.h"
#include "sludge/graphics.h"
#include "sludge/imgloader.h"
#include "sludge/language.h"
#include "sludge/moreio.h"
#include "sludge/newfatal.h"
#include "sludge/objtypes.h"
#include "sludge/people.h"
#include "sludge/region.h"
#include "sludge/savedata.h"
#include "sludge/sludge.h"
#include "sludge/sludger.h"
#include "sludge/sound.h"
#include "sludge/speech.h"
#include "sludge/sprites.h"
#include "sludge/sprbanks.h"
#include "sludge/statusba.h"
#include "sludge/variable.h"
#include "sludge/version.h"
#include "sludge/zbuffer.h"

namespace Sludge {

extern int numBIFNames;
extern Common::String *allBIFNames;
extern int numUserFunc;
extern Common::String *allUserFunc;

int selectedLanguage = 0;

int gameVersion;
FILETIME fileTime;

int numGlobals = 0;

extern Variable *launchResult;
extern Variable *globalVars;
extern VariableStack *noStack;

extern bool allowAnyFilename;

Common::File *openAndVerify(const Common::String &filename, char extra1, char extra2,
		const char *er, int &fileVersion) {
	Common::File *fp = new Common::File();
	if (!fp->open(filename)) {
		fatal("Can't open file", filename);
		return NULL;
	}
	bool headerBad = false;
	if (fp->readByte() != 'S')
		headerBad = true;
	if (fp->readByte() != 'L')
		headerBad = true;
	if (fp->readByte() != 'U')
		headerBad = true;
	if (fp->readByte() != 'D')
		headerBad = true;
	if (fp->readByte() != extra1)
		headerBad = true;
	if (fp->readByte() != extra2)
		headerBad = true;
	if (headerBad) {
		fatal(er, filename);
		return NULL;
	}
	char c;
	c = fp->readByte();
	while ((c = fp->readByte()))
		;

	int majVersion = fp->readByte();
	debugC(2, kSludgeDebugDataLoad, "majVersion %i", majVersion);
	int minVersion = fp->readByte();
	debugC(2, kSludgeDebugDataLoad, "minVersion %i", minVersion);
	fileVersion = majVersion * 256 + minVersion;

	Common::String txtVer = "";

	if (fileVersion > WHOLE_VERSION) {
		txtVer = Common::String::format(ERROR_VERSION_TOO_LOW_2, majVersion, minVersion);
		fatal(ERROR_VERSION_TOO_LOW_1, txtVer);
		return NULL;
	} else if (fileVersion < MINIM_VERSION) {
		txtVer = Common::String::format(ERROR_VERSION_TOO_HIGH_2, majVersion, minVersion);
		fatal(ERROR_VERSION_TOO_HIGH_1, txtVer);
		return NULL;
	}
	return fp;
}

void initSludge() {
	g_sludge->_timer.reset();
	g_sludge->_languageMan->init();
	g_sludge->_gfxMan->init();
	g_sludge->_resMan->init();
	g_sludge->_peopleMan->init();
	g_sludge->_floorMan->init();
	g_sludge->_objMan->init();
	g_sludge->_speechMan->init();
	initStatusBar();
	g_sludge->_evtMan->init();
	g_sludge->_txtMan->init();
	g_sludge->_cursorMan->init();

	g_sludge->_soundMan->init();
	if (!ConfMan.hasKey("mute") || !ConfMan.getBool("mute")) {
		g_sludge->_soundMan->initSoundStuff();
	}

	CustomSaveHelper::_saveEncoding = false;

	// global variables
	numGlobals = 0;
	launchResult = nullptr;

	allowAnyFilename = true;
	noStack = nullptr;
	numBIFNames = numUserFunc = 0;
	allUserFunc = allBIFNames = nullptr;
}

void killSludge() {
	killAllFunctions();
	g_sludge->_peopleMan->kill();
	g_sludge->_regionMan->kill();
	g_sludge->_floorMan->kill();
	g_sludge->_speechMan->kill();
	g_sludge->_languageMan->kill();
	g_sludge->_gfxMan->kill();
	g_sludge->_resMan->kill();
	g_sludge->_objMan->kill();
	g_sludge->_soundMan->killSoundStuff();
	g_sludge->_evtMan->kill();
	g_sludge->_txtMan->kill();
	g_sludge->_cursorMan->kill();

	// global variables
	numBIFNames = numUserFunc = 0;
	delete []allUserFunc;
	delete []allBIFNames;
}

bool initSludge(const Common::String &filename) {
	initSludge();

	Common::File *fp = openAndVerify(filename, 'G', 'E', ERROR_BAD_HEADER, gameVersion);
	if (!fp)
		return false;

	char c = fp->readByte();
	if (c) {
		numBIFNames = fp->readUint16BE();
		debugC(2, kSludgeDebugDataLoad, "numBIFNames %i", numBIFNames);
		allBIFNames = new Common::String[numBIFNames];
		if (!checkNew(allBIFNames))
			return false;

		for (int fn = 0; fn < numBIFNames; fn++) {
			allBIFNames[fn].clear();
			allBIFNames[fn] = readString(fp);
		}
		numUserFunc = fp->readUint16BE();
		debugC(2, kSludgeDebugDataLoad, "numUserFunc %i", numUserFunc);
		allUserFunc = new Common::String[numUserFunc];
		if (!checkNew(allUserFunc))
			return false;

		for (int fn = 0; fn < numUserFunc; fn++) {
			allUserFunc[fn].clear();
			allUserFunc[fn] = readString(fp);
		}

		if (gameVersion >= VERSION(1, 3)) {
			g_sludge->_resMan->readResourceNames(fp);
		}
	}

	int winWidth = fp->readUint16BE();
	debugC(2, kSludgeDebugDataLoad, "winWidth : %i", winWidth);
	int winHeight = fp->readUint16BE();
	debugC(2, kSludgeDebugDataLoad, "winHeight : %i", winHeight);
	g_sludge->_gfxMan->setWindowSize(winWidth, winHeight);

	int specialSettings = fp->readByte();
	debugC(2, kSludgeDebugDataLoad, "specialSettings : %i", specialSettings);
	g_sludge->_timer.setDesiredFPS(1000 / fp->readByte());

	readString(fp);  // Unused - was used for registration purposes.

	uint bytes_read = fp->read(&fileTime, sizeof(FILETIME));
	if (bytes_read != sizeof(FILETIME) && fp->err()) {
		debug(0, "Reading error in initSludge.");
	}

	Common::String dataFol = (gameVersion >= VERSION(1, 3)) ? readString(fp) : "";
	debugC(2, kSludgeDebugDataLoad, "dataFol : %s", dataFol.c_str());

	g_sludge->_languageMan->createTable(fp);

	if (gameVersion >= VERSION(1, 6)) {
		fp->readByte();
		// aaLoad
		fp->readByte();
		fp->readFloatLE();
		fp->readFloatLE();
	}

	Common::String checker = readString(fp);
	debugC(2, kSludgeDebugDataLoad, "checker : %s", checker.c_str());

	if (checker != "okSoFar")
		return fatal(ERROR_BAD_HEADER, filename);

	byte customIconLogo = fp->readByte();
	debugC(2, kSludgeDebugDataLoad, "Game icon type: %i", customIconLogo);

	if (customIconLogo & 1) {
		// There is an icon - read it!
		debugC(2, kSludgeDebugDataLoad, "There is an icon - read it!");

		// read game icon
		Graphics::Surface gameIcon;
		if (!ImgLoader::loadImage(fp, &gameIcon, false))
			return false;

	}

	if (customIconLogo & 2) {
		// There is an logo - read it!
		debugC(2, kSludgeDebugDataLoad, "There is an logo - read it!");

		// read game logo
		Graphics::Surface gameLogo;
		if (!ImgLoader::loadImage(fp, &gameLogo))
			return false;
	}

	numGlobals = fp->readUint16BE();
	debugC(2, kSludgeDebugDataLoad, "numGlobals : %i", numGlobals);

	globalVars = new Variable[numGlobals];
	if (!checkNew(globalVars))
		return false;

	// Get language selected by user
	g_sludge->_resMan->setData(fp);
	g_sludge->_languageMan->setLanguageID(g_sludge->getLanguageID());

	if (!dataFol.empty()) {
		Common::String dataFolder = encodeFilename(dataFol);
	}

	positionStatus(10, winHeight - 15);

	return true;
}

void displayBase() {
	g_sludge->_gfxMan->clear(); // Clear screen
	g_sludge->_gfxMan->drawBackDrop();// Draw Backdrop
	g_sludge->_gfxMan->drawZBuffer(g_sludge->_gfxMan->getCamX(), g_sludge->_gfxMan->getCamY(), false);
	g_sludge->_peopleMan->drawPeople();// Then add any moving characters...
	g_sludge->_gfxMan->displaySpriteLayers();
}

void sludgeDisplay() {
	displayBase();
	g_sludge->_speechMan->display();
	drawStatusBar();
	g_sludge->_cursorMan->displayCursor();
	g_sludge->_gfxMan->display();
}

} // End of namespace Sludge
