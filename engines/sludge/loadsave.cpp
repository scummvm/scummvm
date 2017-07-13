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
#include "sludge/sprites.h"
#include "sludge/fonttext.h"
#include "sludge/newfatal.h"
#include "sludge/variable.h"
#include "sludge/language.h"
#include "sludge/moreio.h"
#include "sludge/sludger.h"
#include "sludge/people.h"
#include "sludge/talk.h"
#include "sludge/objtypes.h"
#include "sludge/backdrop.h"
#include "sludge/region.h"
#include "sludge/floor.h"
#include "sludge/zbuffer.h"
#include "sludge/cursors.h"
#include "sludge/statusba.h"
#include "sludge/sound.h"
#include "sludge/fileset.h"
#include "sludge/loadsave.h"
#include "sludge/bg_effects.h"
#include "sludge/thumbnail.h"
#include "sludge/utf8.h"
#include "sludge/version.h"

namespace Sludge {

//----------------------------------------------------------------------
// From elsewhere
//----------------------------------------------------------------------

extern loadedFunction *allRunningFunctions;         // In sludger.cpp
extern char *typeName[];                            // In variable.cpp
extern int numGlobals;                              // In sludger.cpp
extern variable *globalVars;                        // In sludger.cpp
extern flor *currentFloor;                          // In floor.cpp
extern zBufferData zBuffer;                         // In zbuffer.cpp
extern speechStruct *speech;                        // In talk.cpp
extern personaAnimation *mouseCursorAnim;           // In cursor.cpp
extern int mouseCursorFrameNum;                     // "    "   "
extern int loadedFontNum, fontHeight, fontTableSize;    // In fonttext.cpp
extern int numFontColours;                          // "    "   "
extern UTF8Converter fontOrder;                       // "    "   "
extern FILETIME fileTime;                           // In sludger.cpp
extern int speechMode;                              // "    "   "
extern int lightMapNumber;                          // In backdrop.cpp
extern uint sceneWidth, sceneHeight;        // "    "   "
extern int cameraX, cameraY;                        // "    "   "
extern float cameraZoom;
extern byte brightnessLevel;               // "    "   "
extern int16 fontSpace;                             // in textfont.cpp
extern byte fadeMode;                      // In transition.cpp
extern bool captureAllKeys;
extern bool allowAnyFilename;
extern uint16 saveEncoding;                 // in savedata.cpp
extern byte currentBurnR, currentBurnG, currentBurnB;
extern uint currentBlankColour;             // in backdrop.cpp
extern parallaxLayer *parallaxStuff;                //      "
extern int lightMapMode;                    //      "
extern int languageNum;

//----------------------------------------------------------------------
// Globals (so we know what's saved already and what's a reference
//----------------------------------------------------------------------

struct stackLibrary {
	stackHandler *stack;
	stackLibrary *next;
};

int stackLibTotal = 0;
stackLibrary *stackLib = NULL;

//----------------------------------------------------------------------
// For saving and loading stacks...
//----------------------------------------------------------------------
void saveStack(variableStack *vs, Common::WriteStream *stream) {
	int elements = 0;
	int a;

	variableStack *search = vs;
	while (search) {
		elements++;
		search = search->next;
	}

	stackDebug((stackfp, "  stack contains %d elements\n", elements));

	stream->writeUint16BE(elements);
	search = vs;
	for (a = 0; a < elements; a++) {
		saveVariable(&search->thisVar, stream);
		search = search->next;
	}
}

variableStack *loadStack(Common::SeekableReadStream *stream, variableStack **last) {
	int elements = stream->readUint16BE();
	int a;
	variableStack *first = NULL;
	variableStack **changeMe = &first;

	for (a = 0; a < elements; a++) {
		variableStack *nS = new variableStack;
		if (!checkNew(nS))
			return NULL;
		loadVariable(&(nS->thisVar), stream);
		if (last && a == elements - 1) {
			stackDebug((stackfp, "Setting last to %p\n", nS));
			*last = nS;
		}
		nS->next = NULL;
		(*changeMe) = nS;
		changeMe = &(nS->next);
	}

	return first;
}

bool saveStackRef(stackHandler *vs, Common::WriteStream *stream) {
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

stackHandler *getStackFromLibrary(int n) {
	n = stackLibTotal - n;
	while (n) {
		stackLib = stackLib->next;
		n--;
	}
	return stackLib->stack;
}

stackHandler *loadStackRef(Common::SeekableReadStream *stream) {
	stackHandler *nsh;

	if (stream->readByte()) {    // It's one we've loaded already...
		stackDebug((stackfp, "loadStackRef (duplicate, get from library)\n"));

		nsh = getStackFromLibrary(stream->readUint16BE());
		nsh->timesUsed++;
	} else {
		stackDebug((stackfp, "loadStackRef (new one)\n"));

		// Load the new stack

		nsh = new stackHandler;
		if (!checkNew(nsh))
			return NULL;
		nsh->last = NULL;
		nsh->first = loadStack(stream, &nsh->last);
		nsh->timesUsed = 1;
		stackDebug((stackfp, "  first = %p\n", nsh->first));
		if (nsh->first)
			stackDebug((stackfp, "  first->next = %p\n", nsh->first->next));
		stackDebug((stackfp, "  last = %p\n", nsh->last));
		if (nsh->last)
			stackDebug((stackfp, "  last->next = %p\n", nsh->last->next));

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
bool saveVariable(variable *from, Common::WriteStream *stream) {
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
			saveCostume(from->varData.costumeHandler, stream);
			return false;

		case SVT_ANIM:
			saveAnim(from->varData.animHandler, stream);
			return false;

		case SVT_NULL:
			return false;

		default:
			fatal("Can't save variables of this type:", (from->varType < SVT_NUM_TYPES) ? typeName[from->varType] : "bad ID");
	}
	return true;
}

bool loadVariable(variable *to, Common::SeekableReadStream *stream) {
	to->varType = (variableType)stream->readByte();
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
			to->varData.costumeHandler = new persona;
			if (!checkNew(to->varData.costumeHandler))
				return false;
			loadCostume(to->varData.costumeHandler, stream);
			return true;

		case SVT_ANIM:
			to->varData.animHandler = new personaAnimation;
			if (!checkNew(to->varData.animHandler))
				return false;
			loadAnim(to->varData.animHandler, stream);
			return true;

		default:
			break;
	}
	return true;
}

//----------------------------------------------------------------------
// For saving and loading functions
//----------------------------------------------------------------------
void saveFunction(loadedFunction *fun, Common::WriteStream *stream) {
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

loadedFunction *loadFunction(Common::SeekableReadStream *stream) {
	int a;

	// Reserve memory...

	loadedFunction *buildFunc = new loadedFunction;
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

	if (!saveThumbnail(fp))
		return false;

	fp->write(&fileTime, sizeof(FILETIME));

	// DON'T ADD ANYTHING NEW BEFORE THIS POINT!

	fp->writeByte(allowAnyFilename);
	fp->writeByte(captureAllKeys);
	fp->writeByte(true);
	fp->writeByte(fontTableSize > 0);

	if (fontTableSize > 0) {
		fp->writeUint16BE(loadedFontNum);
		fp->writeUint16BE(fontHeight);
		writeString(fontOrder.getUTF8String(), fp);
	}
	fp->writeSint16LE(fontSpace);

	// Save backdrop
	fp->writeUint16BE(cameraX);
	fp->writeUint16BE(cameraY);
	fp->writeFloatLE(cameraZoom);

	fp->writeByte(brightnessLevel);
	saveHSI(fp);

	// Save event handlers
	saveHandlers(fp);

	// Save regions
	saveRegions(fp);

	saveAnim(mouseCursorAnim, fp);
	fp->writeUint16BE(mouseCursorFrameNum);

	// Save functions
	loadedFunction *thisFunction = allRunningFunctions;
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

	savePeople(fp);

	if (currentFloor->numPolygons) {
		fp->writeByte(1);
		fp->writeUint16BE(currentFloor->originalNum);
	} else {
		fp->writeByte(0);
	}

	if (zBuffer.numPanels > 0) {
		fp->writeByte(1);
		fp->writeUint16BE(zBuffer.originalNum);
	} else {
		fp->writeByte(0);
	}

	if (lightMap.getPixels()) {
		fp->writeByte(1);
		fp->writeUint16BE(lightMapNumber);
	} else {
		fp->writeByte(0);
	}

	fp->writeByte(lightMapMode);
	fp->writeByte(speechMode);
	fp->writeByte(fadeMode);
	saveSpeech(speech, fp);
	saveStatusBars(fp);
	saveSounds(fp);

	fp->writeUint16BE(saveEncoding);

	blur_saveSettings(fp);

	fp->writeUint16BE(currentBlankColour);
	fp->writeByte(currentBurnR);
	fp->writeByte(currentBurnG);
	fp->writeByte(currentBurnB);

	saveParallaxRecursive(parallaxStuff, fp);
	fp->writeByte(0);

	fp->writeByte(languageNum); // Selected language

	saveSnapshot(fp);

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
	ssgVersion = majVersion * 256 + minVersion;


	if (ssgVersion >= VERSION(1, 4)) {
		if (!skipThumbnail(fp))
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
	captureAllKeys = fp->readByte();
	fp->readByte();  // updateDisplay (part of movie playing)

	bool fontLoaded = fp->readByte();
	int fontNum;
	Common::String charOrder = "";
	if (fontLoaded) {
		fontNum = fp->readUint16BE();
		fontHeight = fp->readUint16BE();

		if (ssgVersion < VERSION(2, 2)) {
			char *tmp = new char[257];
			for (int a = 0; a < 256; a++) {
				int x = fp->readByte();
				tmp[x] = a;
			}
			tmp[256] = 0;
			charOrder = tmp;
			delete []tmp;
		} else {
			charOrder = readString(fp);
		}
	}
	loadFont(fontNum, charOrder, fontHeight);

	fontSpace = fp->readSint16LE();

	killAllPeople();
	killAllRegions();

	int camerX = fp->readUint16BE();
	int camerY = fp->readUint16BE();
	float camerZ;
	if (ssgVersion >= VERSION(2, 0)) {
		camerZ = fp->readFloatLE();
	} else {
		camerZ = 1.0;
	}

	brightnessLevel = fp->readByte();

	loadHSI(fp, 0, 0, true);
	loadHandlers(fp);
	loadRegions(fp);

	mouseCursorAnim = new personaAnimation;
	if (!checkNew(mouseCursorAnim))
		return false;
	if (!loadAnim(mouseCursorAnim, fp))
		return false;
	mouseCursorFrameNum = fp->readUint16BE();

	loadedFunction *rFunc;
	loadedFunction **buildList = &allRunningFunctions;

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

	loadPeople(fp);

	if (fp->readByte()) {
		if (!setFloor(fp->readUint16BE()))
			return false;
	} else
		setFloorNull();

	if (fp->readByte()) {
		if (!setZBuffer(fp->readUint16BE()))
			return false;
	}

	if (fp->readByte()) {
		if (!loadLightMap(fp->readUint16BE()))
			return false;
	}

	if (ssgVersion >= VERSION(1, 4)) {
		lightMapMode = fp->readByte() % 3;
	}

	speechMode = fp->readByte();
	fadeMode = fp->readByte();
	loadSpeech(speech, fp);
	loadStatusBars(fp);
	loadSounds(fp);

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
		currentBlankColour = fp->readUint16BE();
		currentBurnR = fp->readByte();
		currentBurnG = fp->readByte();
		currentBurnB = fp->readByte();

		// Read parallax layers
		while (fp->readByte()) {
			int im = fp->readUint16BE();
			int fx = fp->readUint16BE();
			int fy = fp->readUint16BE();

			if (!loadParallax(im, fx, fy))
				return false;
		}

		int selectedLanguage = fp->readByte();
		if (selectedLanguage != languageNum) {
			// Load the saved language!
			languageNum = selectedLanguage;
			setFileIndices(NULL, gameSettings.numLanguages, languageNum);
		}
	}

	nosnapshot();
	if (ssgVersion >= VERSION(1, 4)) {
		if (fp->readByte()) {
			if (!restoreSnapshot(fp))
				return false;
		}
	}

	delete fp;

	cameraX = camerX;
	cameraY = camerY;
	cameraZoom = camerZ;

	clearStackLib();
	return true;
}

} // End of namespace Sludge
