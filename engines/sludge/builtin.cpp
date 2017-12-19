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
#include "common/savefile.h"

#include "sludge/allfiles.h"
#include "sludge/backdrop.h"
#include "sludge/bg_effects.h"
#include "sludge/builtin.h"
#include "sludge/cursors.h"
#include "sludge/event.h"
#include "sludge/floor.h"
#include "sludge/fonttext.h"
#include "sludge/freeze.h"
#include "sludge/graphics.h"
#include "sludge/language.h"
#include "sludge/loadsave.h"
#include "sludge/moreio.h"
#include "sludge/movie.h"
#include "sludge/newfatal.h"
#include "sludge/objtypes.h"
#include "sludge/people.h"
#include "sludge/region.h"
#include "sludge/savedata.h"
#include "sludge/sludger.h"
#include "sludge/sound.h"
#include "sludge/speech.h"
#include "sludge/sprbanks.h"
#include "sludge/sprites.h"
#include "sludge/statusba.h"
#include "sludge/sludge.h"
#include "sludge/utf8.h"
#include "sludge/zbuffer.h"

namespace Sludge {

int speechMode = 0;
SpritePalette pastePalette;

Variable *launchResult = NULL;

extern int lastFramesPerSecond, thumbWidth, thumbHeight;
extern bool allowAnyFilename;
extern bool captureAllKeys;
extern VariableStack *noStack;
extern StatusStuff  *nowStatus;
extern ScreenRegion *overRegion;
extern int numBIFNames, numUserFunc;

extern Common::String *allUserFunc;
extern Common::String *allBIFNames;

extern byte brightnessLevel;
extern byte fadeMode;
extern uint16 saveEncoding;

int paramNum[] = { -1, 0, 1, 1, -1, -1, 1, 3, 4, 1, 0, 0, 8, -1,    // SAY->MOVEMOUSE
                   -1, 0, 0, -1, -1, 1, 1, 1, 1, 4, 1, 1, 2, 1,// FOCUS->REMOVEREGION
                   2, 2, 0, 0, 2,                              // ANIMATE->SETSCALE
                   -1, 2, 1, 0, 0, 0, 1, 0, 3,                 // new/push/pop stack, status stuff
                   2, 0, 0, 3, 1, 0, 2,                        // delFromStack->completeTimers
                   -1, -1, -1, 2, 2, 0, 3, 1,                  // anim, costume, pO, setC, wait, sS, substring, stringLength
                   0, 1, 1, 0, 2,                              // dark, save, load, quit, rename
                   1, 3, 3, 1, 2, 1, 1, 3, 1, 0, 0, 2, 1,      // stackSize, pasteString, startMusic, defvol, vol, stopmus, stopsound, setfont, alignStatus, show x 2, pos'Status, setFloor
                   -1, -1, 1, 1, 2, 1, 1, 1, -1, -1, -1, 1, 1, // force, jump, peekstart, peekend, enqueue, getSavedGames, inFont, loopSound, removeChar, stopCharacter
                   1, 0, 3, 3, 1, 2, 1, 2, 2,                  // launch, howFrozen, pastecol, litcol, checksaved, float, cancelfunc, walkspeed, delAll
                   2, 3, 1, 2, 2, 0, 0, 1, 2, 3, 1, -1,        // extras, mixoverlay, pastebloke, getMScreenX/Y, setSound(Default/-)Volume, looppoints, speechMode, setLightMap
                   -1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1,           // think, getCharacterDirection, is(char/region/moving), deleteGame, renameGame, hardScroll, stringWidth, speechSpeed, normalCharacter
                   2, 1, 2, 1, 3, 1, 1, 2, 1,                  // fetchEvent, setBrightness, spin, fontSpace, burnString, captureAll, cacheSound, setSpinSpeed, transitionMode
                   1, 0, 0, 1, 0, 2, 1, 1, 1,                  // movie(Start/Abort/Playing), updateDisplay, getSoundCache, savedata, loaddata, savemode, freeSound
                   3, 0, 3, 3, 2, 1, 1,                        // setParallax, clearParallax, setBlankColour, setBurnColour, getPixelColour, makeFastArray, getCharacterScale
                   0, 2, 0,                                    // getLanguage, launchWith, getFramesPerSecond
                   3, 2, 2, 0, 0, 1,                           // readThumbnail, setThumbnailSize, hasFlag, snapshot, clearSnapshot, anyFilename
                   2, 1,                                       // regGet, fatal
                   4, 3, -1, 0,                                // chr AA, max AA, setBackgroundEffect, doBackgroundEffect
                   2,                                          // setCharacterAngleOffset
                   2, 5,                                       // setCharacterTransparency, setCharacterColourise
                   1,                                          // zoomCamera
                   1, 0, 0                                     // playMovie, stopMovie, pauseMovie
                 };

bool failSecurityCheck(const Common::String &fn) {
	if (fn.empty())
		return true;

	for (uint i = 0; i < fn.size(); ++i) {
		switch (fn[i]) {
			case ':':
			case '\\':
			case '/':
			case '*':
			case '?':
			case '"':
			case '<':
			case '>':
			case '|':
				fatal("Filenames may not contain the following characters: \n\n\\  /  :  \"  <  >  |  ?  *\n\nConsequently, the following filename is not allowed:", fn);
				return true;
		}
	}
	return false;
}

LoadedFunction *saverFunc;

typedef BuiltReturn (*builtInSludgeFunc)(int numParams, LoadedFunction *fun);
struct builtInFunctionData {
	builtInSludgeFunc func;
};

#define builtIn(a)          static BuiltReturn builtIn_ ## a (int numParams, LoadedFunction *fun)
#define UNUSEDALL           (void) (0 && sizeof(numParams) && sizeof (fun));


static BuiltReturn sayCore(int numParams, LoadedFunction *fun, bool sayIt) {
	int fileNum = -1;
	Common::String newText;
	int objT, p;
	killSpeechTimers();

	switch (numParams) {
		case 3:
			if (!getValueType(fileNum, SVT_FILE, fun->stack->thisVar))
				return BR_ERROR;
			trimStack(fun->stack);
			// fall through

		case 2:
			newText = getTextFromAnyVar(fun->stack->thisVar);
			trimStack(fun->stack);
			if (!getValueType(objT, SVT_OBJTYPE, fun->stack->thisVar))
				return BR_ERROR;
			trimStack(fun->stack);
			p = g_sludge->_speechMan->wrapSpeech(newText, objT, fileNum, sayIt);
			fun->timeLeft = p;
			//debugOut ("BUILTIN: sayCore: %s (%i)\n", newText, p);
			fun->isSpeech = true;
			return BR_KEEP_AND_PAUSE;
	}

	fatal("Function should have either 2 or 3 parameters");
	return BR_ERROR;
}

#pragma mark -
#pragma mark Built in functions

builtIn(say) {
	UNUSEDALL
	return sayCore(numParams, fun, true);
}

builtIn(think) {
	UNUSEDALL
	return sayCore(numParams, fun, false);
}

builtIn(freeze) {
	UNUSEDALL
	g_sludge->_gfxMan->freeze();
	freezeSubs();
	fun->freezerLevel = 0;
	return BR_CONTINUE;
}

builtIn(unfreeze) {
	UNUSEDALL
	g_sludge->_gfxMan->unfreeze();
	unfreezeSubs();
	return BR_CONTINUE;
}

builtIn(howFrozen) {
	UNUSEDALL
	setVariable(fun->reg, SVT_INT, g_sludge->_gfxMan->howFrozen());
	return BR_CONTINUE;
}

builtIn(setCursor) {
	UNUSEDALL
	PersonaAnimation  *aa = getAnimationFromVar(fun->stack->thisVar);
	g_sludge->_cursorMan->pickAnimCursor(aa);
	trimStack(fun->stack);
	return BR_CONTINUE;
}

builtIn(getMouseX) {
	UNUSEDALL
	setVariable(fun->reg, SVT_INT, g_sludge->_evtMan->mouseX() + g_sludge->_gfxMan->getCamX());
	return BR_CONTINUE;
}

builtIn(getMouseY) {
	UNUSEDALL
	setVariable(fun->reg, SVT_INT, g_sludge->_evtMan->mouseY() + g_sludge->_gfxMan->getCamY());
	return BR_CONTINUE;
}

builtIn(getMouseScreenX) {
	UNUSEDALL
	setVariable(fun->reg, SVT_INT, g_sludge->_evtMan->mouseX() * g_sludge->_gfxMan->getCamZoom());
	return BR_CONTINUE;
}

builtIn(getMouseScreenY) {
	UNUSEDALL
	setVariable(fun->reg, SVT_INT, g_sludge->_evtMan->mouseY() * g_sludge->_gfxMan->getCamZoom());
	return BR_CONTINUE;
}

builtIn(getStatusText) {
	UNUSEDALL
	makeTextVar(fun->reg, statusBarText());
	return BR_CONTINUE;
}

builtIn(getMatchingFiles) {
	UNUSEDALL
	Common::String newText = getTextFromAnyVar(fun->stack->thisVar);
	trimStack(fun->stack);
	unlinkVar(fun->reg);

	// Return value
	fun->reg.varType = SVT_STACK;
	fun->reg.varData.theStack = new StackHandler;
	if (!checkNew(fun->reg.varData.theStack))
		return BR_ERROR;
	fun->reg.varData.theStack->first = NULL;
	fun->reg.varData.theStack->last = NULL;
	fun->reg.varData.theStack->timesUsed = 1;
	if (!getSavedGamesStack(fun->reg.varData.theStack, newText))
		return BR_ERROR;
	return BR_CONTINUE;
}

builtIn(saveGame) {
	UNUSEDALL

	if (g_sludge->_gfxMan->isFrozen()) {
		fatal("Can't save game state while the engine is frozen");
	}

	g_sludge->loadNow = getTextFromAnyVar(fun->stack->thisVar);
	trimStack(fun->stack);

	Common::String aaaaa = encodeFilename(g_sludge->loadNow);
	g_sludge->loadNow.clear();
	if (failSecurityCheck(aaaaa))
		return BR_ERROR;      // Won't fail if encoded, how cool is that? OK, not very.

	g_sludge->loadNow = ":" + aaaaa;

	setVariable(fun->reg, SVT_INT, 0);
	saverFunc = fun;
	return BR_KEEP_AND_PAUSE;
}

builtIn(fileExists) {
	UNUSEDALL
	g_sludge->loadNow = getTextFromAnyVar(fun->stack->thisVar);
	trimStack(fun->stack);
	Common::String aaaaa = encodeFilename(g_sludge->loadNow);
	g_sludge->loadNow.clear();

	if (failSecurityCheck(aaaaa))
		return BR_ERROR;

	bool exist = false;

	Common::File fd;
	if (fd.open(aaaaa)) {
		exist = true;
		fd.close();
	} else {
		Common::InSaveFile *fp = g_system->getSavefileManager()->openForLoading(aaaaa);
		if (fp) {
			exist = true;
			delete fp;
		}
	}

	// Return value
	setVariable(fun->reg, SVT_INT, exist);
	return BR_CONTINUE;
}

builtIn(loadGame) {
	UNUSEDALL
	Common::String aaaaa = getTextFromAnyVar(fun->stack->thisVar);
	trimStack(fun->stack);
	g_sludge->loadNow.clear();
	g_sludge->loadNow = encodeFilename(aaaaa);

	if (g_sludge->_gfxMan->isFrozen()) {
		fatal("Can't load a saved game while the engine is frozen");
	}
	if (failSecurityCheck(g_sludge->loadNow))
		return BR_ERROR;
	Common::InSaveFile *fp = g_system->getSavefileManager()->openForLoading(g_sludge->loadNow);
	if (fp) {
		delete fp;
		return BR_KEEP_AND_PAUSE;
	}
	debug("not find sav file");

	g_sludge->loadNow.clear();
	return BR_CONTINUE;
}

//--------------------------------------
#pragma mark -
#pragma mark Background image - Painting

builtIn(blankScreen) {
	UNUSEDALL
	g_sludge->_gfxMan->blankAllScreen();
	return BR_CONTINUE;
}

builtIn(blankArea) {
	UNUSEDALL
	int x1, y1, x2, y2;
	if (!getValueType(y2, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(x2, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(y1, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(x1, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	g_sludge->_gfxMan->blankScreen(x1, y1, x2, y2);
	return BR_CONTINUE;
}

builtIn(darkBackground) {
	UNUSEDALL
	g_sludge->_gfxMan->darkScreen();
	return BR_CONTINUE;
}

builtIn(addOverlay) {
	UNUSEDALL
	int fileNumber, xPos, yPos;
	if (!getValueType(yPos, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(xPos, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(fileNumber, SVT_FILE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	g_sludge->_gfxMan->loadBackDrop(fileNumber, xPos, yPos);
	return BR_CONTINUE;
}

builtIn(mixOverlay) {
	UNUSEDALL
	int fileNumber, xPos, yPos;
	if (!getValueType(yPos, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(xPos, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(fileNumber, SVT_FILE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	g_sludge->_gfxMan->mixBackDrop(fileNumber, xPos, yPos);
	return BR_CONTINUE;
}

builtIn(pasteImage) {
	UNUSEDALL
	int x, y;
	if (!getValueType(y, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(x, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	PersonaAnimation  *pp = getAnimationFromVar(fun->stack->thisVar);
	trimStack(fun->stack);
	if (pp == NULL)
		return BR_CONTINUE;

	g_sludge->_cursorMan->pasteCursor(x, y, pp);
	return BR_CONTINUE;
}

#pragma mark -
#pragma mark Background Image - Scrolling

builtIn(setSceneDimensions) {
	UNUSEDALL
	int x, y;
	if (!getValueType(y, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(x, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (g_sludge->_gfxMan->killResizeBackdrop(x, y)) {
		g_sludge->_gfxMan->blankScreen(0, 0, x, y);
		return BR_CONTINUE;
	}
	fatal("Out of memory creating new backdrop.");
	return BR_ERROR;
}

builtIn(aimCamera) {
	UNUSEDALL
	int cameraX, cameraY;
	if (!getValueType(cameraY, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(cameraX, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);

	g_sludge->_gfxMan->aimCamera(cameraX, cameraY);

	return BR_CONTINUE;
}

builtIn(zoomCamera) {
	UNUSEDALL
	int z;
	if (!getValueType(z, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);

	g_sludge->_gfxMan->zoomCamera(z);

	return BR_CONTINUE;
}

#pragma mark -
#pragma mark Variables

builtIn(pickOne) {
	UNUSEDALL
	if (!numParams) {
		fatal("Built-in function should have at least 1 parameter");
		return BR_ERROR;
	}

	int i;

	i = g_sludge->getRandomSource()->getRandomNumber(numParams - 1);

	// Return value
	while (numParams--) {
		if (i == numParams)
			copyVariable(fun->stack->thisVar, fun->reg);
		trimStack(fun->stack);
	}
	return BR_CONTINUE;
}

builtIn(substring) {
	UNUSEDALL
	Common::String wholeString;
	int start, length;

	//debugOut ("BUILTIN: substring\n");

	if (!getValueType(length, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(start, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	wholeString = getTextFromAnyVar(fun->stack->thisVar);
	trimStack(fun->stack);

	UTF8Converter convert;
	convert.setUTF8String(wholeString);
	Common::U32String str32 = convert.getU32String();

	if ((int)str32.size() < start + length) {
		length = str32.size() - start;
		if ((int)str32.size() < start) {
			start = 0;
		}
	}
	if (length < 0) {
		length = 0;
	}

	int startoffset = convert.getOriginOffset(start);
	int endoffset = convert.getOriginOffset(start + length);

	Common::String newString(wholeString.begin() + startoffset, wholeString.begin() + endoffset);

	makeTextVar(fun->reg, newString);
	return BR_CONTINUE;
}

builtIn(stringLength) {
	UNUSEDALL
	Common::String newText = getTextFromAnyVar(fun->stack->thisVar);
	trimStack(fun->stack);
	setVariable(fun->reg, SVT_INT, g_sludge->_txtMan->stringLength(newText));
	return BR_CONTINUE;
}

builtIn(newStack) {
	UNUSEDALL
	unlinkVar(fun->reg);

	// Return value
	fun->reg.varType = SVT_STACK;
	fun->reg.varData.theStack = new StackHandler;
	if (!checkNew(fun->reg.varData.theStack))
		return BR_ERROR;
	fun->reg.varData.theStack->first = NULL;
	fun->reg.varData.theStack->last = NULL;
	fun->reg.varData.theStack->timesUsed = 1;
	while (numParams--) {
		if (!addVarToStack(fun->stack->thisVar, fun->reg.varData.theStack->first))
			return BR_ERROR;
		if (fun->reg.varData.theStack->last == NULL) {
			fun->reg.varData.theStack->last = fun->reg.varData.theStack->first;
		}
		trimStack(fun->stack);
	}
	return BR_CONTINUE;
}

// wait is exactly the same function, but limited to 2 parameters
#define builtIn_wait builtIn_newStack

builtIn(stackSize) {
	UNUSEDALL
	switch (fun->stack->thisVar.varType) {
		case SVT_STACK:
			// Return value
			setVariable(fun->reg, SVT_INT, stackSize(fun->stack->thisVar.varData.theStack));
			trimStack(fun->stack);
			return BR_CONTINUE;

		case SVT_FASTARRAY:
			// Return value
			setVariable(fun->reg, SVT_INT, fun->stack->thisVar.varData.fastArray->size);
			trimStack(fun->stack);
			return BR_CONTINUE;

		default:
			break;
	}
	fatal("Parameter isn't a stack or a fast array.");
	return BR_ERROR;
}

builtIn(copyStack) {
	UNUSEDALL
	if (fun->stack->thisVar.varType != SVT_STACK) {
		fatal("Parameter isn't a stack.");
		return BR_ERROR;
	}
	// Return value
	if (!copyStack(fun->stack->thisVar, fun->reg))
		return BR_ERROR;
	trimStack(fun->stack);
	return BR_CONTINUE;
}

builtIn(pushToStack) {
	UNUSEDALL
	if (fun->stack->next->thisVar.varType != SVT_STACK) {
		fatal("Parameter isn't a stack");
		return BR_ERROR;
	}

	if (!addVarToStack(fun->stack->thisVar, fun->stack->next->thisVar.varData.theStack->first))
		return BR_ERROR;

	if (fun->stack->next->thisVar.varData.theStack->first->next == NULL)
		fun->stack->next->thisVar.varData.theStack->last = fun->stack->next->thisVar.varData.theStack->first;

	trimStack(fun->stack);
	trimStack(fun->stack);
	return BR_CONTINUE;
}

builtIn(enqueue) {
	UNUSEDALL
	if (fun->stack->next->thisVar.varType != SVT_STACK) {
		fatal("Parameter isn't a stack");
		return BR_ERROR;
	}

	if (fun->stack->next->thisVar.varData.theStack->first == NULL) {
		if (!addVarToStack(fun->stack->thisVar, fun->stack->next->thisVar.varData.theStack->first))
			return BR_ERROR;

		fun->stack->next->thisVar.varData.theStack->last = fun->stack->next->thisVar.varData.theStack->first;
	} else {
		if (!addVarToStack(fun->stack->thisVar, fun->stack->next->thisVar.varData.theStack->last->next))
			return BR_ERROR;
		fun->stack->next->thisVar.varData.theStack->last = fun->stack->next->thisVar.varData.theStack->last->next;
	}

	trimStack(fun->stack);
	trimStack(fun->stack);
	return BR_CONTINUE;
}

builtIn(deleteFromStack) {
	UNUSEDALL
	if (fun->stack->next->thisVar.varType != SVT_STACK) {
		fatal("Parameter isn't a stack.");
		return BR_ERROR;
	}

	// Return value
	setVariable(fun->reg, SVT_INT, deleteVarFromStack(fun->stack->thisVar, fun->stack->next->thisVar.varData.theStack->first, false));

	// Horrible hacking because 'last' value might now be wrong!
	fun->stack->next->thisVar.varData.theStack->last = stackFindLast(fun->stack->next->thisVar.varData.theStack->first);

	trimStack(fun->stack);
	trimStack(fun->stack);
	return BR_CONTINUE;
}

builtIn(deleteAllFromStack) {
	UNUSEDALL
	if (fun->stack->next->thisVar.varType != SVT_STACK) {
		fatal("Parameter isn't a stack.");
		return BR_ERROR;
	}

	// Return value
	setVariable(fun->reg, SVT_INT, deleteVarFromStack(fun->stack->thisVar, fun->stack->next->thisVar.varData.theStack->first, true));

	// Horrible hacking because 'last' value might now be wrong!
	fun->stack->next->thisVar.varData.theStack->last = stackFindLast(fun->stack->next->thisVar.varData.theStack->first);

	trimStack(fun->stack);
	trimStack(fun->stack);
	return BR_CONTINUE;
}

builtIn(popFromStack) {
	UNUSEDALL
	if (fun->stack->thisVar.varType != SVT_STACK) {
		fatal("Parameter isn't a stack.");
		return BR_ERROR;
	}
	if (fun->stack->thisVar.varData.theStack->first == NULL) {
		fatal("The stack's empty.");
		return BR_ERROR;
	}

	// Return value
	copyVariable(fun->stack->thisVar.varData.theStack->first->thisVar, fun->reg);
	trimStack(fun->stack->thisVar.varData.theStack->first);
	trimStack(fun->stack);
	return BR_CONTINUE;
}

builtIn(peekStart) {
	UNUSEDALL
	if (fun->stack->thisVar.varType != SVT_STACK) {
		fatal("Parameter isn't a stack.");
		return BR_ERROR;
	}
	if (fun->stack->thisVar.varData.theStack->first == NULL) {
		fatal("The stack's empty.");
		return BR_ERROR;
	}

	// Return value
	copyVariable(fun->stack->thisVar.varData.theStack->first->thisVar, fun->reg);
	trimStack(fun->stack);
	return BR_CONTINUE;
}

builtIn(peekEnd) {
	UNUSEDALL
	if (fun->stack->thisVar.varType != SVT_STACK) {
		fatal("Parameter isn't a stack.");
		return BR_ERROR;
	}
	if (fun->stack->thisVar.varData.theStack->first == NULL) {
		fatal("The stack's empty.");
		return BR_ERROR;
	}

	// Return value
	copyVariable(fun->stack->thisVar.varData.theStack->last->thisVar, fun->reg);
	trimStack(fun->stack);
	return BR_CONTINUE;
}

builtIn(random) {
	UNUSEDALL
	int num;

	if (!getValueType(num, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;

	trimStack(fun->stack);
	if (num <= 0)
		num = 1;
	setVariable(fun->reg, SVT_INT, 0 /*rand() % num*/); //TODO:false value
	return BR_CONTINUE;
}

static bool getRGBParams(int &red, int &green, int &blue, LoadedFunction *fun) {
	if (!getValueType(blue, SVT_INT, fun->stack->thisVar))
		return false;
	trimStack(fun->stack);
	if (!getValueType(green, SVT_INT, fun->stack->thisVar))
		return false;
	trimStack(fun->stack);
	if (!getValueType(red, SVT_INT, fun->stack->thisVar))
		return false;
	trimStack(fun->stack);
	return true;
}

builtIn(setStatusColour) {
	UNUSEDALL
	int red, green, blue;

	if (!getRGBParams(red, green, blue, fun))
		return BR_ERROR;

	statusBarColour((byte)red, (byte)green, (byte)blue);
	return BR_CONTINUE;
}

builtIn(setLitStatusColour) {
	UNUSEDALL
	int red, green, blue;

	if (!getRGBParams(red, green, blue, fun))
		return BR_ERROR;

	statusBarLitColour((byte)red, (byte)green, (byte)blue);
	return BR_CONTINUE;
}

builtIn(setPasteColour) {
	UNUSEDALL
	int red, green, blue;

	if (!getRGBParams(red, green, blue, fun))
		return BR_ERROR;

	setFontColour(pastePalette, (byte)red, (byte)green, (byte)blue);
	return BR_CONTINUE;
}

builtIn(setBlankColour) {
	UNUSEDALL
	int red, green, blue;

	if (!getRGBParams(red, green, blue, fun))
		return BR_ERROR;

	g_sludge->_gfxMan->setBlankColor(red, green, blue);
	setVariable(fun->reg, SVT_INT, 1);
	return BR_CONTINUE;
}

builtIn(setBurnColour) {
	UNUSEDALL
	int red, green, blue;

	if (!getRGBParams(red, green, blue, fun))
		return BR_ERROR;

	g_sludge->_gfxMan->setBurnColor(red, green, blue);
	setVariable(fun->reg, SVT_INT, 1);
	return BR_CONTINUE;
}

builtIn(setFont) {
	UNUSEDALL
	int fileNumber, newHeight;
	if (!getValueType(newHeight, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	//              newDebug ("  Height:", newHeight);
	trimStack(fun->stack);
	Common::String newText = getTextFromAnyVar(fun->stack->thisVar);
	//              newDebug ("  Character supported:", newText);
	trimStack(fun->stack);
	if (!getValueType(fileNumber, SVT_FILE, fun->stack->thisVar))
		return BR_ERROR;
	//              newDebug ("  File:", fileNumber);
	trimStack(fun->stack);
	if (!g_sludge->_txtMan->loadFont(fileNumber, newText, newHeight))
		return BR_ERROR;
	//              newDebug ("  Done!");
	return BR_CONTINUE;
}

builtIn(inFont) {
	UNUSEDALL
	Common::String newText = getTextFromAnyVar(fun->stack->thisVar);
	trimStack(fun->stack);

	// Return value
	setVariable(fun->reg, SVT_INT, g_sludge->_txtMan->isInFont(newText));
	return BR_CONTINUE;
}

builtIn(pasteString) {
	UNUSEDALL
	Common::String newText = getTextFromAnyVar(fun->stack->thisVar);
	trimStack(fun->stack);
	int y, x;
	if (!getValueType(y, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(x, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (x == IN_THE_CENTRE)
		x = g_sludge->_gfxMan->getCenterX(g_sludge->_txtMan->stringWidth(newText));
	g_sludge->_txtMan->pasteStringToBackdrop(newText, x, y, pastePalette);
	return BR_CONTINUE;
}

builtIn(anim) {
	UNUSEDALL
	if (numParams < 2) {
		fatal("Built-in function anim() must have at least 2 parameters.");
		return BR_ERROR;
	}

	// First store the frame numbers and take 'em off the stack
	PersonaAnimation  *ba = createPersonaAnim(numParams - 1, fun->stack);

	// Only remaining paramter is the file number
	int fileNumber;
	if (!getValueType(fileNumber, SVT_FILE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);

	// Load the required sprite bank
	LoadedSpriteBank *sprBanky = g_sludge->_gfxMan->loadBankForAnim(fileNumber);
	if (!sprBanky)
		return BR_ERROR;    // File not found, fatal done already
	setBankFile(ba, sprBanky);

	// Return value
	newAnimationVariable(fun->reg, ba);

	return BR_CONTINUE;
}

builtIn(costume) {
	UNUSEDALL
	Persona *newPersona = new Persona;
	if (!checkNew(newPersona))
		return BR_ERROR;
	newPersona->numDirections = numParams / 3;
	if (numParams == 0 || newPersona->numDirections * 3 != numParams) {
		fatal("Illegal number of parameters (should be greater than 0 and divisible by 3)");
		return BR_ERROR;
	}
	int iii;
	newPersona->animation = new PersonaAnimation  *[numParams];
	if (!checkNew(newPersona->animation))
		return BR_ERROR;
	for (iii = numParams - 1; iii >= 0; iii--) {
		newPersona->animation[iii] = getAnimationFromVar(fun->stack->thisVar);
		trimStack(fun->stack);
	}

	// Return value
	newCostumeVariable(fun->reg, newPersona);
	return BR_CONTINUE;
}

builtIn(launch) {
	UNUSEDALL
	Common::String newTextA = getTextFromAnyVar(fun->stack->thisVar);

	Common::String newText = encodeFilename(newTextA);

	trimStack(fun->stack);
	if (newTextA[0] == 'h' && newTextA[1] == 't' && newTextA[2] == 't' && newTextA[3] == 'p' && (newTextA[4] == ':' || (newTextA[4] == 's' && newTextA[5] == ':'))) {

		// IT'S A WEBSITE!
		g_sludge->launchMe.clear();
		g_sludge->launchMe = newTextA;
	} else {
		Common::String gameDir = g_sludge->gamePath;
		gameDir += "/";
		g_sludge->launchMe.clear();
		g_sludge->launchMe = gameDir + newText;
		if (g_sludge->launchMe.empty())
			return BR_ERROR;
	}
	setVariable(fun->reg, SVT_INT, 1);
	launchResult = &fun->reg;

	return BR_KEEP_AND_PAUSE;
}

builtIn(pause) {
	UNUSEDALL
	int theTime;
	if (!getValueType(theTime, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (theTime > 0) {
		fun->timeLeft = theTime - 1;
		fun->isSpeech = false;
		return BR_KEEP_AND_PAUSE;
	}
	return BR_CONTINUE;
}

builtIn(completeTimers) {
	UNUSEDALL
	completeTimers();
	return BR_CONTINUE;
}

builtIn(callEvent) {
	UNUSEDALL
	int obj1, obj2;
	if (!getValueType(obj2, SVT_OBJTYPE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(obj1, SVT_OBJTYPE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);

	int fNum = g_sludge->_objMan->getCombinationFunction(obj1, obj2);

	// Return value
	if (fNum) {
		setVariable(fun->reg, SVT_FUNC, fNum);
		return BR_CALLAFUNC;
	}
	setVariable(fun->reg, SVT_INT, 0);
	return BR_CONTINUE;
}

builtIn(quitGame) {
	UNUSEDALL
	g_sludge->_evtMan->quitGame();
	return BR_CONTINUE;
}

#pragma mark -
#pragma mark Movie functions

// The old movie functions are deprecated and does nothing.
builtIn(_rem_movieStart) {
	UNUSEDALL
	trimStack(fun->stack);
	return BR_CONTINUE;
}

builtIn(_rem_movieAbort) {
	UNUSEDALL
	setVariable(fun->reg, SVT_INT, 0);
	return BR_CONTINUE;
}

builtIn(_rem_moviePlaying) {
	UNUSEDALL
	setVariable(fun->reg, SVT_INT, 0);
	return BR_CONTINUE;
}

builtIn(playMovie) {
	UNUSEDALL
	int fileNumber, r;

	if (movieIsPlaying)
		return BR_PAUSE;

	if (!getValueType(fileNumber, SVT_FILE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);

	r = playMovie(fileNumber);

	setVariable(fun->reg, SVT_INT, r);

	if (r && (!fun->next)) {
		restartFunction(fun);
		return BR_ALREADY_GONE;
	}
	return BR_CONTINUE;
}

builtIn(stopMovie) {
	UNUSEDALL

	stopMovie();

	setVariable(fun->reg, SVT_INT, 0);
	return BR_CONTINUE;
}

builtIn(pauseMovie) {
	UNUSEDALL

	pauseMovie();

	setVariable(fun->reg, SVT_INT, 0);
	return BR_CONTINUE;
}

#pragma mark -
#pragma mark Audio functions

builtIn(startMusic) {
	UNUSEDALL
	int fromTrack, musChan, fileNumber;
	if (!getValueType(fromTrack, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(musChan, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(fileNumber, SVT_FILE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!g_sludge->_soundMan->playMOD(fileNumber, musChan, fromTrack))
		return BR_CONTINUE;  //BR_ERROR;
	return BR_CONTINUE;
}

builtIn(stopMusic) {
	UNUSEDALL
	int v;
	if (!getValueType(v, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	g_sludge->_soundMan->stopMOD(v);
	return BR_CONTINUE;
}

builtIn(setMusicVolume) {
	UNUSEDALL
	int musChan, v;
	if (!getValueType(v, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(musChan, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	g_sludge->_soundMan->setMusicVolume(musChan, v);
	return BR_CONTINUE;
}

builtIn(setDefaultMusicVolume) {
	UNUSEDALL
	int v;
	if (!getValueType(v, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	g_sludge->_soundMan->setDefaultMusicVolume(v);
	return BR_CONTINUE;
}

builtIn(playSound) {
	UNUSEDALL
	int fileNumber;
	if (!getValueType(fileNumber, SVT_FILE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!g_sludge->_soundMan->startSound(fileNumber, false))
		return BR_CONTINUE;    // Was BR_ERROR
	return BR_CONTINUE;
}
builtIn(loopSound) {
	UNUSEDALL
	int fileNumber;

	if (numParams < 1) {
		fatal("Built-in function loopSound() must have at least 1 parameter.");
		return BR_ERROR;
	} else if (numParams < 2) {

		if (!getValueType(fileNumber, SVT_FILE, fun->stack->thisVar))
			return BR_ERROR;
		trimStack(fun->stack);
		if (!g_sludge->_soundMan->startSound(fileNumber, true))
			return BR_CONTINUE;     // Was BR_ERROR
		return BR_CONTINUE;
	} else {
		// We have more than one sound to play!

		int doLoop = 2;
		SoundList*s = NULL;
		SoundList*old = NULL;

		// Should we loop?
		if (fun->stack->thisVar.varType != SVT_FILE) {
			getValueType(doLoop, SVT_INT, fun->stack->thisVar);
			trimStack(fun->stack);
			numParams--;
		}
		while (numParams) {
			if (!getValueType(fileNumber, SVT_FILE, fun->stack->thisVar)) {
				fatal("Illegal parameter given built-in function loopSound().");
				return BR_ERROR;
			}
			s = new SoundList;
			if (!checkNew(s))
				return BR_ERROR;

			s->next = old;
			s->prev = NULL;
			s->sound = fileNumber;
			old = s;

			trimStack(fun->stack);
			numParams--;
		}
		while (s->next)
			s = s->next;
		if (doLoop > 1) {
			s->next = old;
			old->prev = s;
		} else if (doLoop) {
			s->next = s;
		}
		old->vol = -1;
		g_sludge->_soundMan->playSoundList(old);
		return BR_CONTINUE;
	}
}

builtIn(stopSound) {
	UNUSEDALL
	int v;
	if (!getValueType(v, SVT_FILE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	g_sludge->_soundMan->huntKillSound(v);
	return BR_CONTINUE;
}

builtIn(setDefaultSoundVolume) {
	UNUSEDALL
	int v;
	if (!getValueType(v, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	g_sludge->_soundMan->setDefaultSoundVolume(v);
	return BR_CONTINUE;
}

builtIn(setSoundVolume) {
	UNUSEDALL
	int musChan, v;
	if (!getValueType(v, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(musChan, SVT_FILE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	g_sludge->_soundMan->setSoundVolume(musChan, v);
	return BR_CONTINUE;
}

builtIn(setSoundLoopPoints) {
	UNUSEDALL
	int musChan, theEnd, theStart;
	if (!getValueType(theEnd, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(theStart, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(musChan, SVT_FILE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	g_sludge->_soundMan->setSoundLoop(musChan, theStart, theEnd);
	return BR_CONTINUE;
}

#pragma mark -
#pragma mark Extra room bits

builtIn(setFloor) {
	UNUSEDALL
	if (fun->stack->thisVar.varType == SVT_FILE) {
		int v;
		getValueType(v, SVT_FILE, fun->stack->thisVar);
		trimStack(fun->stack);
		if (!setFloor(v))
			return BR_ERROR;
	} else {
		trimStack(fun->stack);
		setFloorNull();
	}
	return BR_CONTINUE;
}

builtIn(showFloor) {
	UNUSEDALL
	drawFloor();
	return BR_CONTINUE;
}

builtIn(setZBuffer) {
	UNUSEDALL
	if (fun->stack->thisVar.varType == SVT_FILE) {
		int v;
		getValueType(v, SVT_FILE, fun->stack->thisVar);
		trimStack(fun->stack);
		if (!g_sludge->_gfxMan->setZBuffer(v))
			return BR_ERROR;
	} else {
		trimStack(fun->stack);
		g_sludge->_gfxMan->killZBuffer();
	}
	return BR_CONTINUE;
}

builtIn(setLightMap) {
	UNUSEDALL
	switch (numParams) {
		case 2:
			if (!getValueType(g_sludge->_gfxMan->_lightMapMode, SVT_INT, fun->stack->thisVar))
				return BR_ERROR;
			trimStack(fun->stack);
			g_sludge->_gfxMan->_lightMapMode %= LIGHTMAPMODE_NUM;
			// fall through

		case 1:
			if (fun->stack->thisVar.varType == SVT_FILE) {
				int v;
				getValueType(v, SVT_FILE, fun->stack->thisVar);
				trimStack(fun->stack);
				if (!g_sludge->_gfxMan->loadLightMap(v))
					return BR_ERROR;
				setVariable(fun->reg, SVT_INT, 1);
			} else {
				trimStack(fun->stack);
				g_sludge->_gfxMan->killLightMap();
				setVariable(fun->reg, SVT_INT, 0);
			}
			break;

		default:
			fatal("Function should have either 2 or 3 parameters");
			return BR_ERROR;
	}
	return BR_CONTINUE;
}

#pragma mark -
#pragma mark Objects

builtIn(setSpeechMode) {
	UNUSEDALL
	if (!getValueType(speechMode, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (speechMode < 0 || speechMode > 2) {
		fatal("Valid parameters are be SPEECHANDTEXT, SPEECHONLY or TEXTONLY");
		return BR_ERROR;
	}
	return BR_CONTINUE;
}

builtIn(somethingSpeaking) {
	UNUSEDALL
	int i = g_sludge->_speechMan->isThereAnySpeechGoingOn();
	if (i == -1) {
		setVariable(fun->reg, SVT_INT, 0);
	} else {
		setVariable(fun->reg, SVT_OBJTYPE, i);
	}
	return BR_CONTINUE;
}

builtIn(skipSpeech) {
	UNUSEDALL
	killSpeechTimers();
	return BR_CONTINUE;
}

builtIn(getOverObject) {
	UNUSEDALL
	if (overRegion)
		// Return value
		setVariable(fun->reg, SVT_OBJTYPE, overRegion->thisType->objectNum);
	else
		// Return value
		setVariable(fun->reg, SVT_INT, 0);
	return BR_CONTINUE;
}

builtIn(rename) {
	UNUSEDALL
	Common::String newText = getTextFromAnyVar(fun->stack->thisVar);
	int objT;
	trimStack(fun->stack);
	if (!getValueType(objT, SVT_OBJTYPE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	ObjectType *o = g_sludge->_objMan->findObjectType(objT);
	o->screenName.clear();
	o->screenName = newText;
	return BR_CONTINUE;
}

builtIn(getObjectX) {
	UNUSEDALL
	int objectNumber;
	if (!getValueType(objectNumber, SVT_OBJTYPE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);

	OnScreenPerson *pers = findPerson(objectNumber);
	if (pers) {
		setVariable(fun->reg, SVT_INT, pers->x);
	} else {
		ScreenRegion *la = getRegionForObject(objectNumber);
		if (la) {
			setVariable(fun->reg, SVT_INT, la->sX);
		} else {
			setVariable(fun->reg, SVT_INT, 0);
		}
	}
	return BR_CONTINUE;
}

builtIn(getObjectY) {
	UNUSEDALL
	int objectNumber;
	if (!getValueType(objectNumber, SVT_OBJTYPE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);

	OnScreenPerson *pers = findPerson(objectNumber);
	if (pers) {
		setVariable(fun->reg, SVT_INT, pers->y);
	} else {
		ScreenRegion *la = getRegionForObject(objectNumber);
		if (la) {
			setVariable(fun->reg, SVT_INT, la->sY);
		} else {
			setVariable(fun->reg, SVT_INT, 0);
		}
	}
	return BR_CONTINUE;
}

builtIn(addScreenRegion) {
	UNUSEDALL
	int sX, sY, x1, y1, x2, y2, di, objectNumber;
	if (!getValueType(di, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(sY, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(sX, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(y2, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(x2, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(y1, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(x1, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(objectNumber, SVT_OBJTYPE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (addScreenRegion(x1, y1, x2, y2, sX, sY, di, objectNumber))
		return BR_CONTINUE;
	return BR_ERROR;

}

builtIn(removeScreenRegion) {
	UNUSEDALL
	int objectNumber;
	if (!getValueType(objectNumber, SVT_OBJTYPE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	removeScreenRegion(objectNumber);
	return BR_CONTINUE;
}

builtIn(showBoxes) {
	UNUSEDALL
	showBoxes();
	return BR_CONTINUE;
}

builtIn(removeAllScreenRegions) {
	UNUSEDALL
	killAllRegions();
	return BR_CONTINUE;
}

builtIn(addCharacter) {
	UNUSEDALL
	Persona *p;
	int x, y, objectNumber;

	p = getCostumeFromVar(fun->stack->thisVar);
	if (p == NULL)
		return BR_ERROR;

	trimStack(fun->stack);
	if (!getValueType(y, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(x, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(objectNumber, SVT_OBJTYPE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (addPerson(x, y, objectNumber, p))
		return BR_CONTINUE;
	return BR_ERROR;
}

builtIn(hideCharacter) {
	UNUSEDALL
	int objectNumber;
	if (!getValueType(objectNumber, SVT_OBJTYPE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	setShown(false, objectNumber);
	return BR_CONTINUE;
}

builtIn(showCharacter) {
	UNUSEDALL
	int objectNumber;
	if (!getValueType(objectNumber, SVT_OBJTYPE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	setShown(true, objectNumber);
	return BR_CONTINUE;
}

builtIn(removeAllCharacters) {
	UNUSEDALL
	killSpeechTimers();
	killMostPeople();
	return BR_CONTINUE;
}

builtIn(setCharacterDrawMode) {
	UNUSEDALL
	int obj, di;
	if (!getValueType(di, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(obj, SVT_OBJTYPE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	setDrawMode(di, obj);
	return BR_CONTINUE;
}
builtIn(setCharacterTransparency) {
	UNUSEDALL
	int obj, x;
	if (!getValueType(x, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(obj, SVT_OBJTYPE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	setPersonTransparency(obj, x);
	return BR_CONTINUE;
}
builtIn(setCharacterColourise) {
	UNUSEDALL
	int obj, r, g, b, mix;
	if (!getValueType(mix, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(b, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(g, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(r, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(obj, SVT_OBJTYPE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	setPersonColourise(obj, r, g, b, mix);
	return BR_CONTINUE;
}

builtIn(setScale) {
	UNUSEDALL
	int val1, val2;
	if (!getValueType(val2, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(val1, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	setScale((int16)val1, (int16)val2);
	return BR_CONTINUE;
}

builtIn(stopCharacter) {
	UNUSEDALL
	int obj;
	if (!getValueType(obj, SVT_OBJTYPE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);

	// Return value
	setVariable(fun->reg, SVT_INT, stopPerson(obj));
	return BR_CONTINUE;
}

builtIn(pasteCharacter) {
	UNUSEDALL
	int obj;
	if (!getValueType(obj, SVT_OBJTYPE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);

	OnScreenPerson *thisPerson = findPerson(obj);
	if (thisPerson) {
		PersonaAnimation  *myAnim;
		myAnim = thisPerson->myAnim;
		if (myAnim != thisPerson->lastUsedAnim) {
			thisPerson->lastUsedAnim = myAnim;
			thisPerson->frameNum = 0;
			thisPerson->frameTick = myAnim->frames[0].howMany;
		}

		int fNum = myAnim->frames[thisPerson->frameNum].frameNum;
		g_sludge->_gfxMan->fixScaleSprite(thisPerson->x, thisPerson->y, myAnim->theSprites->bank.sprites[ABS(fNum)], myAnim->theSprites->bank.myPalette, thisPerson, 0, 0, fNum < 0);
		setVariable(fun->reg, SVT_INT, 1);
	} else {
		setVariable(fun->reg, SVT_INT, 0);
	}
	return BR_CONTINUE;
}

builtIn(animate) {
	UNUSEDALL
	int obj;
	PersonaAnimation  *pp = getAnimationFromVar(fun->stack->thisVar);
	if (pp == NULL)
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(obj, SVT_OBJTYPE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	animatePerson(obj, pp);
	setVariable(fun->reg, SVT_INT, timeForAnim(pp));
	return BR_CONTINUE;
}

builtIn(setCostume) {
	UNUSEDALL
	int obj;
	Persona *pp = getCostumeFromVar(fun->stack->thisVar);
	if (pp == NULL)
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(obj, SVT_OBJTYPE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	animatePerson(obj, pp);
	return BR_CONTINUE;
}

builtIn(floatCharacter) {
	UNUSEDALL
	int obj, di;
	if (!getValueType(di, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(obj, SVT_OBJTYPE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	setVariable(fun->reg, SVT_INT, floatCharacter(di, obj));
	return BR_CONTINUE;
}

builtIn(setCharacterWalkSpeed) {
	UNUSEDALL
	int obj, di;
	if (!getValueType(di, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(obj, SVT_OBJTYPE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	setVariable(fun->reg, SVT_INT, setCharacterWalkSpeed(di, obj));
	return BR_CONTINUE;
}

builtIn(turnCharacter) {
	UNUSEDALL
	int obj, di;
	if (!getValueType(di, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(obj, SVT_OBJTYPE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	setVariable(fun->reg, SVT_INT, turnPersonToFace(obj, di));
	return BR_CONTINUE;
}

builtIn(setCharacterExtra) {
	UNUSEDALL
	int obj, di;
	if (!getValueType(di, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(obj, SVT_OBJTYPE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	setVariable(fun->reg, SVT_INT, setPersonExtra(obj, di));
	return BR_CONTINUE;
}

builtIn(removeCharacter) {
	UNUSEDALL
	int objectNumber;
	if (!getValueType(objectNumber, SVT_OBJTYPE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	removeOneCharacter(objectNumber);
	return BR_CONTINUE;
}

static BuiltReturn moveChr(int numParams, LoadedFunction *fun, bool force, bool immediate) {
	switch (numParams) {
		case 3: {
			int x, y, objectNumber;

			if (!getValueType(y, SVT_INT, fun->stack->thisVar))
				return BR_ERROR;
			trimStack(fun->stack);
			if (!getValueType(x, SVT_INT, fun->stack->thisVar))
				return BR_ERROR;
			trimStack(fun->stack);
			if (!getValueType(objectNumber, SVT_OBJTYPE, fun->stack->thisVar))
				return BR_ERROR;
			trimStack(fun->stack);

			if (force) {
				if (forceWalkingPerson(x, y, objectNumber, fun, -1))
					return BR_PAUSE;
			} else if (immediate) {
				jumpPerson(x, y, objectNumber);
			} else {
				if (makeWalkingPerson(x, y, objectNumber, fun, -1))
					return BR_PAUSE;
			}
			return BR_CONTINUE;
		}

		case 2: {
			int toObj, objectNumber;
			ScreenRegion*reggie;

			if (!getValueType(toObj, SVT_OBJTYPE, fun->stack->thisVar))
				return BR_ERROR;
			trimStack(fun->stack);
			if (!getValueType(objectNumber, SVT_OBJTYPE, fun->stack->thisVar))
				return BR_ERROR;
			trimStack(fun->stack);
			reggie = getRegionForObject(toObj);
			if (reggie == NULL)
				return BR_CONTINUE;

			if (force) {
				if (forceWalkingPerson(reggie->sX, reggie->sY, objectNumber, fun, reggie->di))
					return BR_PAUSE;
			} else if (immediate) {
				jumpPerson(reggie->sX, reggie->sY, objectNumber);
			} else {
				if (makeWalkingPerson(reggie->sX, reggie->sY, objectNumber, fun, reggie->di))
					return BR_PAUSE;
			}
			return BR_CONTINUE;
		}

		default:
			fatal("Built-in function must have either 2 or 3 parameters.");
			return BR_ERROR;
	}
}

builtIn(moveCharacter) {
	UNUSEDALL
	return moveChr(numParams, fun, false, false);
}

builtIn(forceCharacter) {
	UNUSEDALL
	return moveChr(numParams, fun, true, false);
}

builtIn(jumpCharacter) {
	UNUSEDALL
	return moveChr(numParams, fun, false, true);
}

builtIn(clearStatus) {
	UNUSEDALL
	clearStatusBar();
	return BR_CONTINUE;
}

builtIn(removeLastStatus) {
	UNUSEDALL
	killLastStatus();
	return BR_CONTINUE;
}

builtIn(addStatus) {
	UNUSEDALL
	addStatusBar();
	return BR_CONTINUE;
}

builtIn(statusText) {
	UNUSEDALL
	Common::String newText = getTextFromAnyVar(fun->stack->thisVar);
	trimStack(fun->stack);
	setStatusBar(newText);
	return BR_CONTINUE;
}

builtIn(lightStatus) {
	UNUSEDALL
	int val;
	if (!getValueType(val, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	setLitStatus(val);
	return BR_CONTINUE;
}

builtIn(positionStatus) {
	UNUSEDALL
	int x, y;
	if (!getValueType(y, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(x, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	positionStatus(x, y);
	return BR_CONTINUE;
}

builtIn(alignStatus) {
	UNUSEDALL
	int val;
	if (!getValueType(val, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	nowStatus->alignStatus = (int16)val;
	return BR_CONTINUE;
}

static bool getFuncNumForCallback(int numParams, LoadedFunction *fun, int &functionNum) {
	switch (numParams) {
		case 0:
			functionNum = 0;
			break;

		case 1:
			if (!getValueType(functionNum, SVT_FUNC, fun->stack->thisVar))
				return false;
			trimStack(fun->stack);
			break;

		default:
			fatal("Too many parameters.");
			return false;
	}
	return true;
}

builtIn(onLeftMouse) {
	UNUSEDALL
	int functionNum;
	if (getFuncNumForCallback(numParams, fun, functionNum)) {
		g_sludge->_evtMan->setEventFunction(kLeftMouse, functionNum);
		return BR_CONTINUE;
	}
	return BR_ERROR;
}

builtIn(onLeftMouseUp) {
	UNUSEDALL
	int functionNum;
	if (getFuncNumForCallback(numParams, fun, functionNum)) {
		g_sludge->_evtMan->setEventFunction(kLeftMouseUp, functionNum);
		return BR_CONTINUE;
	}
	return BR_ERROR;
}

builtIn(onRightMouse) {
	UNUSEDALL
	int functionNum;
	if (getFuncNumForCallback(numParams, fun, functionNum)) {
		g_sludge->_evtMan->setEventFunction(kRightMouse, functionNum);
		return BR_CONTINUE;
	}
	return BR_ERROR;
}

builtIn(onRightMouseUp) {
	UNUSEDALL
	int functionNum;
	if (getFuncNumForCallback(numParams, fun, functionNum)) {
		g_sludge->_evtMan->setEventFunction(kRightMouseUp, functionNum);
		return BR_CONTINUE;
	}
	return BR_ERROR;
}

builtIn(onFocusChange) {
	UNUSEDALL
	int functionNum;
	if (getFuncNumForCallback(numParams, fun, functionNum)) {
		g_sludge->_evtMan->setEventFunction(kFocus, functionNum);
		return BR_CONTINUE;
	}
	return BR_ERROR;
}

builtIn(onMoveMouse) {
	UNUSEDALL
	int functionNum;
	if (getFuncNumForCallback(numParams, fun, functionNum)) {
		g_sludge->_evtMan->setEventFunction(kMoveMouse, functionNum);
		return BR_CONTINUE;
	}
	return BR_ERROR;
}

builtIn(onKeyboard) {
	UNUSEDALL
	int functionNum;
	if (getFuncNumForCallback(numParams, fun, functionNum)) {
		g_sludge->_evtMan->setEventFunction(kSpace, functionNum);
		return BR_CONTINUE;
	}
	return BR_ERROR;
}

builtIn(spawnSub) {
	UNUSEDALL
	int functionNum;
	if (getFuncNumForCallback(numParams, fun, functionNum)) {
		if (!startNewFunctionNum(functionNum, 0, NULL, noStack))
			return BR_ERROR;
		return BR_CONTINUE;
	}
	return BR_ERROR;
}

builtIn(cancelSub) {
	UNUSEDALL
	int functionNum;
	if (getFuncNumForCallback(numParams, fun, functionNum)) {
		bool killedMyself;
		cancelAFunction(functionNum, fun, killedMyself);
		if (killedMyself) {
			abortFunction(fun);
			return BR_ALREADY_GONE;
		}
		return BR_CONTINUE;
	}
	return BR_ERROR;
}

builtIn(stringWidth) {
	UNUSEDALL
	Common::String theText = getTextFromAnyVar(fun->stack->thisVar);
	trimStack(fun->stack);

	// Return value
	setVariable(fun->reg, SVT_INT, g_sludge->_txtMan->stringWidth(theText));
	return BR_CONTINUE;
}

builtIn(hardScroll) {
	UNUSEDALL
	int v;
	if (!getValueType(v, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	g_sludge->_gfxMan->hardScroll(v);
	return BR_CONTINUE;
}

builtIn(isScreenRegion) {
	UNUSEDALL
	int objectNumber;
	if (!getValueType(objectNumber, SVT_OBJTYPE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	setVariable(fun->reg, SVT_INT, getRegionForObject(objectNumber) != NULL);
	return BR_CONTINUE;
}

builtIn(setSpeechSpeed) {
	UNUSEDALL
	int number;
	if (!getValueType(number, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	g_sludge->_speechMan->setSpeechSpeed(number * 0.01);
	setVariable(fun->reg, SVT_INT, 1);
	return BR_CONTINUE;
}

builtIn(setFontSpacing) {
	UNUSEDALL
	int fontSpaceI;
	if (!getValueType(fontSpaceI, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	g_sludge->_txtMan->setFontSpace(fontSpaceI);
	trimStack(fun->stack);
	setVariable(fun->reg, SVT_INT, 1);
	return BR_CONTINUE;
}

builtIn(transitionLevel) {
	UNUSEDALL
	int number;
	if (!getValueType(number, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);

	if (number < 0)
		brightnessLevel = 0;
	else if (number > 255)
		brightnessLevel = 255;
	else
		brightnessLevel = number;

	setVariable(fun->reg, SVT_INT, 1);
	return BR_CONTINUE;
}

builtIn(captureAllKeys) {
	UNUSEDALL
	captureAllKeys = getBoolean(fun->stack->thisVar);
	trimStack(fun->stack);
	setVariable(fun->reg, SVT_INT, captureAllKeys);
	return BR_CONTINUE;
}

builtIn(spinCharacter) {
	UNUSEDALL
	int number, objectNumber;
	if (!getValueType(number, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(objectNumber, SVT_OBJTYPE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);

	OnScreenPerson *thisPerson = findPerson(objectNumber);
	if (thisPerson) {
		thisPerson->wantAngle = number;
		thisPerson->spinning = true;
		thisPerson->continueAfterWalking = fun;
		setVariable(fun->reg, SVT_INT, 1);
		return BR_PAUSE;
	} else {
		setVariable(fun->reg, SVT_INT, 0);
		return BR_CONTINUE;
	}
}

builtIn(getCharacterDirection) {
	UNUSEDALL
	int objectNumber;
	if (!getValueType(objectNumber, SVT_OBJTYPE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	OnScreenPerson *thisPerson = findPerson(objectNumber);
	if (thisPerson) {
		setVariable(fun->reg, SVT_INT, thisPerson->direction);
	} else {
		setVariable(fun->reg, SVT_INT, 0);
	}
	return BR_CONTINUE;
}

builtIn(isCharacter) {
	UNUSEDALL
	int objectNumber;
	if (!getValueType(objectNumber, SVT_OBJTYPE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	OnScreenPerson *thisPerson = findPerson(objectNumber);
	setVariable(fun->reg, SVT_INT, thisPerson != NULL);
	return BR_CONTINUE;
}

builtIn(normalCharacter) {
	UNUSEDALL
	int objectNumber;
	if (!getValueType(objectNumber, SVT_OBJTYPE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	OnScreenPerson *thisPerson = findPerson(objectNumber);
	if (thisPerson) {
		thisPerson->myAnim = thisPerson->myPersona->animation[thisPerson->direction];
		setVariable(fun->reg, SVT_INT, 1);
	} else {
		setVariable(fun->reg, SVT_INT, 0);
	}
	return BR_CONTINUE;
}

builtIn(isMoving) {
	UNUSEDALL
	int objectNumber;
	if (!getValueType(objectNumber, SVT_OBJTYPE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	OnScreenPerson *thisPerson = findPerson(objectNumber);
	if (thisPerson) {
		setVariable(fun->reg, SVT_INT, thisPerson->walking);
	} else {
		setVariable(fun->reg, SVT_INT, 0);
	}
	return BR_CONTINUE;
}

builtIn(fetchEvent) {
	UNUSEDALL
	int obj1, obj2;
	if (!getValueType(obj2, SVT_OBJTYPE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(obj1, SVT_OBJTYPE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);

	int fNum = g_sludge->_objMan->getCombinationFunction(obj1, obj2);

	// Return value
	if (fNum) {
		setVariable(fun->reg, SVT_FUNC, fNum);
	} else {
		setVariable(fun->reg, SVT_INT, 0);
	}
	return BR_CONTINUE;
}

builtIn(deleteFile) {
	UNUSEDALL

	Common::String namNormal = getTextFromAnyVar(fun->stack->thisVar);
	trimStack(fun->stack);
	Common::String nam = encodeFilename(namNormal);
	namNormal.clear();
	if (failSecurityCheck(nam))
		return BR_ERROR;
	setVariable(fun->reg, SVT_INT, remove(nam.c_str()));

	return BR_CONTINUE;
}

builtIn(renameFile) {
	UNUSEDALL
	Common::String temp;

	temp.clear();
	temp = getTextFromAnyVar(fun->stack->thisVar);
	Common::String newnam = encodeFilename(temp);
	trimStack(fun->stack);
	if (failSecurityCheck(newnam))
		return BR_ERROR;
	temp.clear();

	temp = getTextFromAnyVar(fun->stack->thisVar);
	Common::String nam = encodeFilename(temp);
	trimStack(fun->stack);
	if (failSecurityCheck(nam))
		return BR_ERROR;

	setVariable(fun->reg, SVT_INT, rename(nam.c_str(), newnam.c_str()));

	return BR_CONTINUE;
}

builtIn(cacheSound) {
	UNUSEDALL
	int fileNumber;
	if (!getValueType(fileNumber, SVT_FILE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (g_sludge->_soundMan->cacheSound(fileNumber) == -1)
		return BR_ERROR;
	return BR_CONTINUE;
}

builtIn(burnString) {
	UNUSEDALL
	Common::String newText = getTextFromAnyVar(fun->stack->thisVar);
	trimStack(fun->stack);
	int y, x;
	if (!getValueType(y, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(x, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (x == IN_THE_CENTRE)
		x = g_sludge->_gfxMan->getCenterX(g_sludge->_txtMan->stringWidth(newText));
	g_sludge->_txtMan->burnStringToBackdrop(newText, x, y, pastePalette);
	return BR_CONTINUE;
}

builtIn(setCharacterSpinSpeed) {
	UNUSEDALL
	int speed, who;
	if (!getValueType(speed, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(who, SVT_OBJTYPE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);

	OnScreenPerson *thisPerson = findPerson(who);

	if (thisPerson) {
		thisPerson->spinSpeed = speed;
		setVariable(fun->reg, SVT_INT, 1);
	} else {
		setVariable(fun->reg, SVT_INT, 0);
	}
	return BR_CONTINUE;
}

builtIn(setCharacterAngleOffset) {
	UNUSEDALL
	int angle, who;
	if (!getValueType(angle, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(who, SVT_OBJTYPE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);

	OnScreenPerson *thisPerson = findPerson(who);

	if (thisPerson) {
		thisPerson->angleOffset = angle;
		setVariable(fun->reg, SVT_INT, 1);
	} else {
		setVariable(fun->reg, SVT_INT, 0);
	}
	return BR_CONTINUE;
}

builtIn(transitionMode) {
	UNUSEDALL
	int n;
	if (!getValueType(n, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	fadeMode = n;
	trimStack(fun->stack);
	setVariable(fun->reg, SVT_INT, 1);
	return BR_CONTINUE;
}

// Removed function - does nothing
builtIn(_rem_updateDisplay) {
	UNUSEDALL
	trimStack(fun->stack);
	setVariable(fun->reg, SVT_INT, true);
	return BR_CONTINUE;
}

builtIn(getSoundCache) {
	UNUSEDALL
	fun->reg.varType = SVT_STACK;
	fun->reg.varData.theStack = new StackHandler;
	if (!checkNew(fun->reg.varData.theStack))
		return BR_ERROR;
	fun->reg.varData.theStack->first = NULL;
	fun->reg.varData.theStack->last = NULL;
	fun->reg.varData.theStack->timesUsed = 1;
	if (!g_sludge->_soundMan->getSoundCacheStack(fun->reg.varData.theStack))
		return BR_ERROR;
	return BR_CONTINUE;
}

builtIn(saveCustomData) {
	UNUSEDALL
	// saveCustomData (thisStack, fileName);
	Common::String fileNameB = getTextFromAnyVar(fun->stack->thisVar);

	Common::String fileName = encodeFilename(fileNameB);

	if (failSecurityCheck(fileName))
		return BR_ERROR;
	trimStack(fun->stack);

	if (fun->stack->thisVar.varType != SVT_STACK) {
		fatal("First parameter isn't a stack");
		return BR_ERROR;
	}
	if (!stackToFile(fileName, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	return BR_CONTINUE;
}

builtIn(loadCustomData) {
	UNUSEDALL

	Common::String newTextA = getTextFromAnyVar(fun->stack->thisVar);

	Common::String newText = encodeFilename(newTextA);

	if (failSecurityCheck(newText))
		return BR_ERROR;
	trimStack(fun->stack);

	unlinkVar(fun->reg);
	fun->reg.varType = SVT_STACK;
	fun->reg.varData.theStack = new StackHandler;
	if (!checkNew(fun->reg.varData.theStack))
		return BR_ERROR;
	fun->reg.varData.theStack->first = NULL;
	fun->reg.varData.theStack->last = NULL;
	fun->reg.varData.theStack->timesUsed = 1;
	if (!fileToStack(newText, fun->reg.varData.theStack))
		return BR_ERROR;
	return BR_CONTINUE;
}

builtIn(setCustomEncoding) {
	UNUSEDALL
	int n;
	if (!getValueType(n, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	saveEncoding = n;
	trimStack(fun->stack);
	setVariable(fun->reg, SVT_INT, 1);
	return BR_CONTINUE;
}

builtIn(freeSound) {
	UNUSEDALL
	int v;
	if (!getValueType(v, SVT_FILE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	g_sludge->_soundMan->huntKillFreeSound(v);
	return BR_CONTINUE;
}

builtIn(parallaxAdd) {
	UNUSEDALL
	if (g_sludge->_gfxMan->isFrozen()) {
		fatal("Can't set background parallax image while frozen");
		return BR_ERROR;
	} else {
		int wrapX, wrapY, v;
		if (!getValueType(wrapY, SVT_INT, fun->stack->thisVar))
			return BR_ERROR;
		trimStack(fun->stack);
		if (!getValueType(wrapX, SVT_INT, fun->stack->thisVar))
			return BR_ERROR;
		trimStack(fun->stack);
		if (!getValueType(v, SVT_FILE, fun->stack->thisVar))
			return BR_ERROR;
		trimStack(fun->stack);

		if (!g_sludge->_gfxMan->loadParallax(v, wrapX, wrapY))
			return BR_ERROR;
		setVariable(fun->reg, SVT_INT, 1);
	}
	return BR_CONTINUE;
}

builtIn(parallaxClear) {
	UNUSEDALL
	g_sludge->_gfxMan->killParallax();
	setVariable(fun->reg, SVT_INT, 1);
	return BR_CONTINUE;
}

builtIn(getPixelColour) {
	UNUSEDALL
	int x, y;
	if (!getValueType(y, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(x, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);

	unlinkVar(fun->reg);
	fun->reg.varType = SVT_STACK;
	fun->reg.varData.theStack = new StackHandler;
	if (!checkNew(fun->reg.varData.theStack))
		return BR_ERROR;
	fun->reg.varData.theStack->first = NULL;
	fun->reg.varData.theStack->last = NULL;
	fun->reg.varData.theStack->timesUsed = 1;
	if (!g_sludge->_gfxMan->getRGBIntoStack(x, y, fun->reg.varData.theStack))
		return BR_ERROR;

	return BR_CONTINUE;
}

builtIn(makeFastArray) {
	UNUSEDALL
	switch (fun->stack->thisVar.varType) {
		case SVT_STACK: {
			bool success = makeFastArrayFromStack(fun->reg, fun->stack->thisVar.varData.theStack);
			trimStack(fun->stack);
			return success ? BR_CONTINUE : BR_ERROR;
		}
			break;

		case SVT_INT: {
			int i = fun->stack->thisVar.varData.intValue;
			trimStack(fun->stack);
			return makeFastArraySize(fun->reg, i) ? BR_CONTINUE : BR_ERROR;
		}
			break;

		default:
			break;
	}
	fatal("Parameter must be a number or a stack.");
	return BR_ERROR;
}

builtIn(getCharacterScale) {
	UNUSEDALL
	int objectNumber;
	if (!getValueType(objectNumber, SVT_OBJTYPE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);

	OnScreenPerson *pers = findPerson(objectNumber);
	if (pers) {
		setVariable(fun->reg, SVT_INT, pers->scale * 100);
	} else {
		setVariable(fun->reg, SVT_INT, 0);
	}
	return BR_CONTINUE;
}

builtIn(getLanguageID) {
	UNUSEDALL
	setVariable(fun->reg, SVT_INT, g_sludge->getLanguageID());
	return BR_CONTINUE;
}

// Removed function
builtIn(_rem_launchWith) {
	UNUSEDALL

	trimStack(fun->stack);

	// To support some windows only games
	Common::String filename = getTextFromAnyVar(fun->stack->thisVar);
	trimStack(fun->stack);

	if (filename.hasSuffix(".exe")) {
		const Common::FSNode gameDataDir(ConfMan.get("path"));
		Common::FSList files;
		gameDataDir.getChildren(files, Common::FSNode::kListFilesOnly);

		for (Common::FSList::const_iterator file = files.begin(); file != files.end(); ++file) {
			Common::String fileName = file->getName();
			fileName.toLowercase();
			if (fileName.hasSuffix(".dat") || fileName == "data") {
				g_sludge->launchNext = file->getName();
				return BR_CONTINUE;
			}
		}
	}

	g_sludge->launchNext.clear();
	setVariable(fun->reg, SVT_INT, false);
	return BR_CONTINUE;
}

builtIn(getFramesPerSecond) {
	UNUSEDALL
	setVariable(fun->reg, SVT_INT, lastFramesPerSecond);
	return BR_CONTINUE;
}

builtIn(showThumbnail) {
	UNUSEDALL
	int x, y;
	if (!getValueType(y, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(x, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);

	// Encode the name!Encode the name!
	Common::String aaaaa = getTextFromAnyVar(fun->stack->thisVar);
	trimStack(fun->stack);
	Common::String file = encodeFilename(aaaaa);
	g_sludge->_gfxMan->showThumbnail(file, x, y);
	return BR_CONTINUE;
}

builtIn(setThumbnailSize) {
	UNUSEDALL
	if (!getValueType(thumbHeight, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(thumbWidth, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!g_sludge->_gfxMan->checkSizeValide(thumbWidth, thumbHeight)) {
		Common::String buff = Common::String::format("%i x %i", thumbWidth, thumbWidth);
		fatal("Invalid thumbnail size", buff);
		return BR_ERROR;
	}
	return BR_CONTINUE;
}

builtIn(hasFlag) {
	UNUSEDALL
	int objNum, flagIndex;
	if (!getValueType(flagIndex, SVT_INT, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!getValueType(objNum, SVT_OBJTYPE, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	ObjectType *objT = g_sludge->_objMan->findObjectType(objNum);
	if (!objT)
		return BR_ERROR;
	setVariable(fun->reg, SVT_INT, objT->flags & (1 << flagIndex));
	return BR_CONTINUE;
}

builtIn(snapshotGrab) {
	UNUSEDALL
	return g_sludge->_gfxMan->snapshot() ? BR_CONTINUE : BR_ERROR;
}

builtIn(snapshotClear) {
	UNUSEDALL
	g_sludge->_gfxMan->nosnapshot();
	return BR_CONTINUE;
}

builtIn(bodgeFilenames) {
	UNUSEDALL
	bool lastValue = allowAnyFilename;
	allowAnyFilename = getBoolean(fun->stack->thisVar);
	trimStack(fun->stack);
	setVariable(fun->reg, SVT_INT, lastValue);
	return BR_CONTINUE;
}

// Deprecated - does nothing.
builtIn(_rem_registryGetString) {
	UNUSEDALL
	trimStack(fun->stack);
	trimStack(fun->stack);
	setVariable(fun->reg, SVT_INT, 0);

	return BR_CONTINUE;
}

builtIn(quitWithFatalError) {
	UNUSEDALL
	Common::String mess = getTextFromAnyVar(fun->stack->thisVar);
	trimStack(fun->stack);
	fatal(mess);
	return BR_ERROR;
}

builtIn(_rem_setCharacterAA) {
	UNUSEDALL

	trimStack(fun->stack);
	trimStack(fun->stack);
	trimStack(fun->stack);
	trimStack(fun->stack);

	return BR_CONTINUE;
}

builtIn(_rem_setMaximumAA) {
	UNUSEDALL

	trimStack(fun->stack);
	trimStack(fun->stack);
	trimStack(fun->stack);

	return BR_CONTINUE;

}

builtIn(setBackgroundEffect) {
	UNUSEDALL
	bool done = blur_createSettings(numParams, fun->stack);
	setVariable(fun->reg, SVT_INT, done ? 1 : 0);
	return BR_CONTINUE;
}

builtIn(doBackgroundEffect) {
	UNUSEDALL
	bool done = blurScreen();
	setVariable(fun->reg, SVT_INT, done ? 1 : 0);
	return BR_CONTINUE;
}

#pragma mark -
#pragma mark Other functions

//-------------------------------------

} // End of namespace Sludge

#include "functionlist.h"

namespace Sludge {

BuiltReturn callBuiltIn(int whichFunc, int numParams, LoadedFunction *fun) {
	if (numBIFNames) {
		setFatalInfo((fun->originalNumber < numUserFunc) ? allUserFunc[fun->originalNumber] : "Unknown user function",
				(whichFunc < numBIFNames) ? allBIFNames[whichFunc] : "Unknown built-in function");
	}

	if (whichFunc < NUM_FUNCS) {
		if (paramNum[whichFunc] != -1) {
			if (paramNum[whichFunc] != numParams) {
				Common::String buff = Common::String::format("Built in function must have %i parameter%s", paramNum[whichFunc], (paramNum[whichFunc] == 1) ? "" : "s");
				Common::String msg = buff;
				fatal(msg);
				return BR_ERROR;
			}
		}

		if (builtInFunctionArray[whichFunc].func) {
			debugC(3, kSludgeDebugBuiltin,
					"Run built-in function %i : %s",
					whichFunc, (whichFunc < numBIFNames) ? allBIFNames[whichFunc].c_str() : "Unknown");
			return builtInFunctionArray[whichFunc].func(numParams, fun);
		}
	}

	fatal("Unknown / unimplemented built-in function.");
	return BR_ERROR;
}

} // End of namespace Sludge
