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
#include "sludge/graphics.h"
#include "sludge/language.h"
#include "sludge/loadsave.h"
#include "sludge/moreio.h"
#include "sludge/newfatal.h"
#include "sludge/objtypes.h"
#include "sludge/people.h"
#include "sludge/region.h"
#include "sludge/sludge.h"
#include "sludge/sludger.h"
#include "sludge/sound.h"
#include "sludge/sprites.h"
#include "sludge/statusba.h"
#include "sludge/speech.h"
#include "sludge/utf8.h"
#include "sludge/variable.h"
#include "sludge/version.h"
#include "sludge/zbuffer.h"

namespace Sludge {

//----------------------------------------------------------------------
// From elsewhere
//----------------------------------------------------------------------

extern LoadedFunction *allRunningFunctions;         // In sludger.cpp
extern const char *typeName[];                      // In variable.cpp
extern int numGlobals;                              // In sludger.cpp
extern Variable *globalVars;                        // In sludger.cpp
extern Floor *currentFloor;                          // In floor.cpp
extern FILETIME fileTime;                           // In sludger.cpp
extern byte brightnessLevel;               // "    "   "
extern byte fadeMode;                      // In transition.cpp
extern bool allowAnyFilename;
extern uint16 saveEncoding;                 // in savedata.cpp

//----------------------------------------------------------------------
// Globals (so we know what's saved already and what's a reference
//----------------------------------------------------------------------

struct stackLibrary {
	StackHandler *stack;
	stackLibrary *next;
};

int stackLibTotal = 0;
stackLibrary *stackLib = NULL;

//----------------------------------------------------------------------
// For saving and loading stacks...
//----------------------------------------------------------------------
void saveStack(VariableStack *vs, Common::WriteStream *stream) {
	int elements = 0;
	int a;

	VariableStack *search = vs;
	while (search) {
		elements++;
		search = search->next;
	}

	stream->writeUint16BE(elements);
	search = vs;
	for (a = 0; a < elements; a++) {
		saveVariable(&search->thisVar, stream);
		search = search->next;
	}
}

VariableStack *loadStack(Common::SeekableReadStream *stream, VariableStack **last) {
	int elements = stream->readUint16BE();
	int a;
	VariableStack *first = NULL;
	VariableStack **changeMe = &first;

	for (a = 0; a < elements; a++) {
		VariableStack *nS = new VariableStack;
		if (!checkNew(nS))
			return NULL;
		loadVariable(&(nS->thisVar), stream);
		if (last && a == elements - 1) {
			*last = nS;
		}
		nS->next = NULL;
		(*changeMe) = nS;
		changeMe = &(nS->next);
	}

	return first;
}

bool saveStackRef(StackHandler *vs, Common::WriteStream *stream) {
	stackLibrary *s = stackLib;
	int a = 0;
	while (s) {
		if (s->stack == vs) {
			stream->writeByte(1);
			stream->writeUint16BE(stackLibTotal - a);
			return true;
		}
		s = s->next;
		a++;
	}
	stream->writeByte(0);
	saveStack(vs->first, stream);
	s = new stackLibrary;
	stackLibTotal++;
	if (!checkNew(s))
		return false;
	s->next = stackLib;
	s->stack = vs;
	stackLib = s;
	return true;
}

void clearStackLib() {
	stackLibrary *k;
	while (stackLib) {
		k = stackLib;
		stackLib = stackLib->next;
		delete k;
	}
	stackLibTotal = 0;
}

StackHandler *getStackFromLibrary(int n) {
	n = stackLibTotal - n;
	while (n) {
		stackLib = stackLib->next;
		n--;
	}
	return stackLib->stack;
}

StackHandler *loadStackRef(Common::SeekableReadStream *stream) {
	StackHandler *nsh;

	if (stream->readByte()) {    // It's one we've loaded already...
		nsh = getStackFromLibrary(stream->readUint16BE());
		nsh->timesUsed++;
	} else {
		// Load the new stack

		nsh = new StackHandler;
		if (!checkNew(nsh))
			return NULL;
		nsh->last = NULL;
		nsh->first = loadStack(stream, &nsh->last);
		nsh->timesUsed = 1;

		// Add it to the library of loaded stacks

		stackLibrary *s = new stackLibrary;
		if (!checkNew(s))
			return NULL;
		s->stack = nsh;
		s->next = stackLib;
		stackLib = s;
		stackLibTotal++;
	}
	return nsh;
}

//----------------------------------------------------------------------
// For saving and loading variables...
//----------------------------------------------------------------------
bool saveVariable(Variable *from, Common::WriteStream *stream) {
	stream->writeByte(from->varType);
	switch (from->varType) {
		case SVT_INT:
		case SVT_FUNC:
		case SVT_BUILT:
		case SVT_FILE:
		case SVT_OBJTYPE:
			stream->writeUint32LE(from->varData.intValue);
			return true;

		case SVT_STRING:
			writeString(from->varData.theString, stream);
			return true;

		case SVT_STACK:
			return saveStackRef(from->varData.theStack, stream);

		case SVT_COSTUME:
			from->varData.costumeHandler->save(stream);
			return false;

		case SVT_ANIM:
			from->varData.animHandler->save(stream);
			return false;

		case SVT_NULL:
			return false;

		default:
			fatal("Can't save variables of this type:", (from->varType < SVT_NUM_TYPES) ? typeName[from->varType] : "bad ID");
	}
	return true;
}

bool loadVariable(Variable *to, Common::SeekableReadStream *stream) {
	to->varType = (VariableType)stream->readByte();
	switch (to->varType) {
		case SVT_INT:
		case SVT_FUNC:
		case SVT_BUILT:
		case SVT_FILE:
		case SVT_OBJTYPE:
			to->varData.intValue = stream->readUint32LE();
			return true;

		case SVT_STRING:
			to->varData.theString = createCString(readString(stream));
			return true;

		case SVT_STACK:
			to->varData.theStack = loadStackRef(stream);
			return true;

		case SVT_COSTUME:
			to->varData.costumeHandler = new Persona;
			if (!checkNew(to->varData.costumeHandler))
				return false;
			to->varData.costumeHandler->load(stream);
			return true;

		case SVT_ANIM:
			to->varData.animHandler = new PersonaAnimation;
			if (!checkNew(to->varData.animHandler))
				return false;
			to->varData.animHandler->load(stream);
			return true;

		default:
			break;
	}
	return true;
}

//----------------------------------------------------------------------
// For saving and loading functions
//----------------------------------------------------------------------
void saveFunction(LoadedFunction *fun, Common::WriteStream *stream) {
	int a;
	stream->writeUint16BE(fun->originalNumber);
	if (fun->calledBy) {
		stream->writeByte(1);
		saveFunction(fun->calledBy, stream);
	} else {
		stream->writeByte(0);
	}
	stream->writeUint32LE(fun->timeLeft);
	stream->writeUint16BE(fun->runThisLine);
	stream->writeByte(fun->cancelMe);
	stream->writeByte(fun->returnSomething);
	stream->writeByte(fun->isSpeech);
	saveVariable(&(fun->reg), stream);

	if (fun->freezerLevel) {
		fatal(ERROR_GAME_SAVE_FROZEN);
	}
	saveStack(fun->stack, stream);
	for (a = 0; a < fun->numLocals; a++) {
		saveVariable(&(fun->localVars[a]), stream);
	}
}

LoadedFunction *loadFunction(Common::SeekableReadStream *stream) {
	int a;

	// Reserve memory...

	LoadedFunction *buildFunc = new LoadedFunction;
	if (!checkNew(buildFunc))
		return NULL;

	// See what it was called by and load if we need to...

	buildFunc->originalNumber = stream->readUint16BE();
	buildFunc->calledBy = NULL;
	if (stream->readByte()) {
		buildFunc->calledBy = loadFunction(stream);
		if (!buildFunc->calledBy)
			return NULL;
	}

	buildFunc->timeLeft = stream->readUint32LE();
	buildFunc->runThisLine = stream->readUint16BE();
	buildFunc->freezerLevel = 0;
	buildFunc->cancelMe = stream->readByte();
	buildFunc->returnSomething = stream->readByte();
	buildFunc->isSpeech = stream->readByte();
	loadVariable(&(buildFunc->reg), stream);
	loadFunctionCode(buildFunc);

	buildFunc->stack = loadStack(stream, NULL);

	for (a = 0; a < buildFunc->numLocals; a++) {
		loadVariable(&(buildFunc->localVars[a]), stream);
	}

	return buildFunc;
}

//----------------------------------------------------------------------
// Save everything
//----------------------------------------------------------------------

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
	fp->writeUint16BE(g_sludge->_gfxMan->getCamX());
	fp->writeUint16BE(g_sludge->_gfxMan->getCamY());
	fp->writeFloatLE(g_sludge->_gfxMan->getCamZoom());

	fp->writeByte(brightnessLevel);
	g_sludge->_gfxMan->saveHSI(fp);

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
		saveVariable(&globalVars[a], fp);
	}

	g_sludge->_peopleMan->savePeople(fp);

	if (currentFloor->numPolygons) {
		fp->writeByte(1);
		fp->writeUint16BE(currentFloor->originalNum);
	} else {
		fp->writeByte(0);
	}

	g_sludge->_gfxMan->saveZBuffer(fp);
	g_sludge->_gfxMan->saveLightMap(fp);

	fp->writeByte(fadeMode);

	g_sludge->_speechMan->save(fp);
	saveStatusBars(fp);
	g_sludge->_soundMan->saveSounds(fp);

	fp->writeUint16BE(saveEncoding);

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

	int camerX = fp->readUint16BE();
	int camerY = fp->readUint16BE();
	float camerZ;
	if (ssgVersion >= VERSION(2, 0)) {
		camerZ = fp->readFloatLE();
	} else {
		camerZ = 1.0;
	}

	brightnessLevel = fp->readByte();

	g_sludge->_gfxMan->loadHSI(fp, 0, 0, true);
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
		unlinkVar(globalVars[a]);
		loadVariable(&globalVars[a], fp);
	}

	g_sludge->_peopleMan->loadPeople(fp);

	if (fp->readByte()) {
		if (!setFloor(fp->readUint16BE()))
			return false;
	} else
		setFloorNull();

	if (!g_sludge->_gfxMan->loadZBuffer(fp))
		return false;

	if (!g_sludge->_gfxMan->loadLightMap(ssgVersion, fp)) {
		return false;
	}

	fadeMode = fp->readByte();
	g_sludge->_speechMan->load(fp);
	loadStatusBars(fp);
	g_sludge->_soundMan->loadSounds(fp);

	saveEncoding = fp->readUint16BE();

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

	g_sludge->_gfxMan->setCamera(camerX, camerY, camerZ);

	clearStackLib();
	return true;
}

} // End of namespace Sludge
