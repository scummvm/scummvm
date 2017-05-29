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
#include "allfiles.h"
#include "sprites.h"
#include "fonttext.h"
#include "newfatal.h"
#include "variable.h"
#include "CommonCode/version.h"
#include "language.h"
#include "moreio.h"
#include "sludger.h"
#include "people.h"
#include "talk.h"
#include "objtypes.h"
#include "backdrop.h"
#include "region.h"
#include "floor.h"
#include "zbuffer.h"
#include "cursors.h"
#include "statusba.h"
#include "sound.h"
#include "fileset.h"
#include "debug.h"
#include "loadsave.h"

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
extern char *fontOrderString;                       // "    "   "
extern FILETIME fileTime;                           // In sludger.cpp
extern int speechMode;                              // "    "   "
extern int lightMapNumber;                          // In backdrop.cpp
extern unsigned int sceneWidth, sceneHeight;        // "    "   "
extern int cameraX, cameraY;                        // "    "   "
extern float cameraZoom;
extern unsigned char brightnessLevel;               // "    "   "
extern short fontSpace;                             // in textfont.cpp
extern unsigned char fadeMode;                      // In transition.cpp
extern bool captureAllKeys;
extern bool allowAnyFilename;
extern unsigned short saveEncoding;                 // in savedata.cpp
extern unsigned char currentBurnR, currentBurnG, currentBurnB;
extern unsigned int currentBlankColour;             // in backdrop.cpp
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

	put2bytes(elements, stream);
	search = vs;
	for (a = 0; a < elements; ++a) {
		saveVariable(&search->thisVar, stream);
		search = search->next;
	}
}

variableStack *loadStack(Common::SeekableReadStream *stream,
		variableStack **last) {
	int elements = get2bytes(stream);
	int a;
	variableStack *first = NULL;
	variableStack * * changeMe = &first;

	for (a = 0; a < elements; ++a) {
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
			putch(1, stream);
			put2bytes(stackLibTotal - a, stream);
			return true;
		}
		s = s->next;
		++a;
	}
	putch(0, stream);
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

	if (getch(stream)) {    // It's one we've loaded already...
		stackDebug((stackfp, "loadStackRef (duplicate, get from library)\n"));

		nsh = getStackFromLibrary(get2bytes(stream));
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
#if DEBUG_STACKINESS
	{
		char *str = getTextFromAnyVar(*from);
		stackDebug((stackfp, "in saveVariable, type %d, %s\n", from->varType, str));
		delete str;
	}
#endif

	putch(from->varType, stream);
	switch (from->varType) {
	case SVT_INT:
	case SVT_FUNC:
	case SVT_BUILT:
	case SVT_FILE:
	case SVT_OBJTYPE:
		put4bytes(from->varData.intValue, stream);
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
		fatal("Can't save variables of this type:",
				(from->varType < SVT_NUM_TYPES) ?
						typeName[from->varType] : "bad ID");
	}
	return true;
}

bool loadVariable(variable *to, Common::SeekableReadStream *stream) {
	to->varType = (variableType) getch(stream);
	switch (to->varType) {
	case SVT_INT:
	case SVT_FUNC:
	case SVT_BUILT:
	case SVT_FILE:
	case SVT_OBJTYPE:
		to->varData.intValue = get4bytes(stream);
		return true;

	case SVT_STRING:
		to->varData.theString = readString(stream);
		return true;

	case SVT_STACK:
		to->varData.theStack = loadStackRef(stream);
#if DEBUG_STACKINESS
		{
			char *str = getTextFromAnyVar(*to);
			stackDebug((stackfp, "just loaded %s\n", str));
			delete str;
		}
#endif
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
	put2bytes(fun->originalNumber, stream);
	if (fun->calledBy) {
		putch(1, stream);
		saveFunction(fun->calledBy, stream);
	} else {
		putch(0, stream);
	}
	put4bytes(fun->timeLeft, stream);
	put2bytes(fun->runThisLine, stream);
	putch(fun->cancelMe, stream);
	putch(fun->returnSomething, stream);
	putch(fun->isSpeech, stream);
	saveVariable(&(fun->reg), stream);

	if (fun->freezerLevel) {
		fatal(ERROR_GAME_SAVE_FROZEN);
	}
	saveStack(fun->stack, stream);
	for (a = 0; a < fun->numLocals; ++a) {
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

	buildFunc->originalNumber = get2bytes(stream);
	buildFunc->calledBy = NULL;
	if (getch(stream)) {
		buildFunc->calledBy = loadFunction(stream);
		if (!buildFunc->calledBy)
			return NULL;
	}

	buildFunc->timeLeft = get4bytes(stream);
	buildFunc->runThisLine = get2bytes(stream);
	buildFunc->freezerLevel = 0;
	buildFunc->cancelMe = getch(stream);
	buildFunc->returnSomething = getch(stream);
	buildFunc->isSpeech = getch(stream);
	loadVariable(&(buildFunc->reg), stream);
	loadFunctionCode(buildFunc);

	buildFunc->stack = loadStack(stream, NULL);

	for (a = 0; a < buildFunc->numLocals; ++a) {
		loadVariable(&(buildFunc->localVars[a]), stream);
	}

	return buildFunc;
}

//----------------------------------------------------------------------
// Save everything
//----------------------------------------------------------------------

bool saveGame(char *fname) {
#if 0
	int a;

	FILE *fp = fopen(fname, "wb");
	if (fp == NULL) return false;

	fprintf(fp, "SLUDSA");
	fputc(0, fp);
	fputc(0, fp);
	fputc(MAJOR_VERSION, fp);
	fputc(MINOR_VERSION, fp);

	if (! saveThumbnail(fp)) return false;

	fwrite(&fileTime, sizeof(FILETIME), 1, fp);

	// DON'T ADD ANYTHING NEW BEFORE THIS POINT!

	fputc(allowAnyFilename, fp);
	fputc(captureAllKeys, fp);
	fputc(true, fp);// updateDisplay
	fputc(fontTableSize > 0, fp);

	if (fontTableSize > 0) {
		put2bytes(loadedFontNum, fp);
		put2bytes(fontHeight, fp);
		writeString(fontOrderString, fp);
	}
	putSigned(fontSpace, fp);

	// Save backdrop
	put2bytes(cameraX, fp);
	put2bytes(cameraY, fp);
	putFloat(cameraZoom, fp);

	fputc(brightnessLevel, fp);
	saveHSI(fp);

	// Save event handlers
	saveHandlers(fp);

	// Save regions
	saveRegions(fp);

	saveAnim(mouseCursorAnim, fp);
	put2bytes(mouseCursorFrameNum, fp);

	// Save functions
	loadedFunction *thisFunction = allRunningFunctions;
	int countFunctions = 0;
	while (thisFunction) {
		countFunctions ++;
		thisFunction = thisFunction->next;
	}
	put2bytes(countFunctions, fp);

	thisFunction = allRunningFunctions;
	while (thisFunction) {
		saveFunction(thisFunction, fp);
		thisFunction = thisFunction->next;
	}

	for (a = 0; a < numGlobals; ++a) {
		saveVariable(&globalVars[a], fp);
	}

	savePeople(fp);

	if (currentFloor->numPolygons) {
		fputc(1, fp);
		put2bytes(currentFloor->originalNum, fp);
	} else fputc(0, fp);

	if (zBuffer.tex) {
		fputc(1, fp);
		put2bytes(zBuffer.originalNum, fp);
	} else fputc(0, fp);

	if (lightMap.data) {
		fputc(1, fp);
		put2bytes(lightMapNumber, fp);
	} else fputc(0, fp);

	fputc(lightMapMode, fp);
	fputc(speechMode, fp);
	fputc(fadeMode, fp);
	saveSpeech(speech, fp);
	saveStatusBars(fp);
	saveSounds(fp);

	put2bytes(saveEncoding, fp);

	blur_saveSettings(fp);

	put2bytes(currentBlankColour, fp);
	fputc(currentBurnR, fp);
	fputc(currentBurnG, fp);
	fputc(currentBurnB, fp);

	saveParallaxRecursive(parallaxStuff, fp);
	fputc(0, fp);

	fputc(languageNum, fp);     // Selected language

	saveSnapshot(fp);

	fclose(fp);
#endif
	clearStackLib();
	return true;
}

//----------------------------------------------------------------------
// Load everything
//----------------------------------------------------------------------

int ssgVersion;

bool loadGame(char *fname) {
#if 0
	FILE *fp;
	FILETIME savedGameTime;
	int a;

	while (allRunningFunctions) finishFunction(allRunningFunctions);

	fp = openAndVerify(fname, 'S', 'A', ERROR_GAME_LOAD_NO, ssgVersion);
	if (fp == NULL) return false;

	if (ssgVersion >= VERSION(1, 4)) {
		if (! skipThumbnail(fp)) return fatal(ERROR_GAME_LOAD_CORRUPT, fname);
	}

	size_t bytes_read = fread(&savedGameTime, sizeof(FILETIME), 1, fp);
	if (bytes_read != sizeof(FILETIME) && ferror(fp)) {
		debugOut("Reading error in loadGame.\n");
	}

	if (savedGameTime.dwLowDateTime != fileTime.dwLowDateTime ||
			savedGameTime.dwHighDateTime != fileTime.dwHighDateTime) {
		return fatal(ERROR_GAME_LOAD_WRONG, fname);
	}

	// DON'T ADD ANYTHING NEW BEFORE THIS POINT!

	if (ssgVersion >= VERSION(1, 4)) {
		allowAnyFilename = fgetc(fp);
	}
	captureAllKeys = fgetc(fp);
	fgetc(fp);  // updateDisplay (part of movie playing)

	bool fontLoaded = fgetc(fp);
	int fontNum;
	char *charOrder;
	if (fontLoaded) {
		fontNum = get2bytes(fp);
		fontHeight = get2bytes(fp);

		if (ssgVersion < VERSION(2, 2)) {
			int x;
			charOrder = new char[257];
			if (! checkNew(charOrder)) return false;

			for (int a = 0; a < 256; ++a) {
				x = fgetc(fp);
				charOrder[x] = a;
			}
			charOrder[256] = 0;
		} else {
			charOrder = readString(fp);
		}
	}
	loadFont(fontNum, charOrder, fontHeight);
	delete [] charOrder;

	fontSpace = getSigned(fp);

	killAllPeople();
	killAllRegions();

	int camerX = get2bytes(fp);
	int camerY = get2bytes(fp);
	float camerZ;
	if (ssgVersion >= VERSION(2, 0)) {
		camerZ = getFloat(fp);
	} else {
		camerZ = 1.0;
	}

	brightnessLevel = fgetc(fp);

	loadHSI(fp, 0, 0, true);
	loadHandlers(fp);
	loadRegions(fp);

	mouseCursorAnim = new personaAnimation;
	if (! checkNew(mouseCursorAnim)) return false;
	if (! loadAnim(mouseCursorAnim, fp)) return false;
	mouseCursorFrameNum = get2bytes(fp);

	loadedFunction *rFunc;
	loadedFunction * * buildList = &allRunningFunctions;

	int countFunctions = get2bytes(fp);
	while (countFunctions --) {
		rFunc = loadFunction(fp);
		rFunc->next = NULL;
		(* buildList) = rFunc;
		buildList = &(rFunc->next);
	}

	for (a = 0; a < numGlobals; ++a) {
		unlinkVar(globalVars[a]);
		loadVariable(&globalVars[a], fp);
	}

	loadPeople(fp);

	if (fgetc(fp)) {
		if (! setFloor(get2bytes(fp))) return false;
	} else setFloorNull();

	if (fgetc(fp)) {
		if (! setZBuffer(get2bytes(fp))) return false;
	}

	if (fgetc(fp)) {
		if (! loadLightMap(get2bytes(fp))) return false;
	}

	if (ssgVersion >= VERSION(1, 4)) {
		lightMapMode = fgetc(fp) % 3;
	}

	speechMode = fgetc(fp);
	fadeMode = fgetc(fp);
	loadSpeech(speech, fp);
	loadStatusBars(fp);
	loadSounds(fp);

	saveEncoding = get2bytes(fp);

	if (ssgVersion >= VERSION(1, 6)) {
		if (ssgVersion < VERSION(2, 0)) {
			// aaLoad
			fgetc(fp);
			getFloat(fp);
			getFloat(fp);
		}

		blur_loadSettings(fp);
	}

	if (ssgVersion >= VERSION(1, 3)) {
		currentBlankColour = get2bytes(fp);
		currentBurnR = fgetc(fp);
		currentBurnG = fgetc(fp);
		currentBurnB = fgetc(fp);

		// Read parallax layers
		while (fgetc(fp)) {
			int im = get2bytes(fp);
			int fx = get2bytes(fp);
			int fy = get2bytes(fp);

			if (! loadParallax(im, fx, fy)) return false;
		}

		int selectedLanguage = fgetc(fp);
		if (selectedLanguage != languageNum) {
			// Load the saved language!
			languageNum = selectedLanguage;
			setFileIndices(NULL, gameSettings.numLanguages, languageNum);
		}
	}

	nosnapshot();
	if (ssgVersion >= VERSION(1, 4)) {
		if (fgetc(fp)) {
			if (! restoreSnapshot(fp)) return false;
		}
	}

	fclose(fp);

	cameraX = camerX;
	cameraY = camerY;
	cameraZoom = camerZ;
#endif

	clearStackLib();
	return true;
}

} // End of namespace Sludge
