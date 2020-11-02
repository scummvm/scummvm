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
#define SYMBIAN_USE_SYSTEM_REMOVE

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
#include "sludge/function.h"
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
#include "sludge/zbuffer.h"

namespace Sludge {

Variable *launchResult = NULL;

extern bool allowAnyFilename;
extern VariableStack *noStack;
extern StatusStuff  *nowStatus;
extern int numBIFNames, numUserFunc;

extern Common::String *allUserFunc;
extern Common::String *allBIFNames;

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
			default:
				break;
		}
	}
	return false;
}

extern LoadedFunction *saverFunc;

typedef BuiltReturn (*builtInSludgeFunc)(int numParams, LoadedFunction *fun);
struct builtInFunctionData {
	builtInSludgeFunc func;
	int paramNum;
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
			if (!fun->stack->thisVar.getValueType(fileNum, SVT_FILE))
				return BR_ERROR;
			trimStack(fun->stack);
			// fall through

		case 2:
			newText = fun->stack->thisVar.getTextFromAnyVar();
			trimStack(fun->stack);
			if (!fun->stack->thisVar.getValueType(objT, SVT_OBJTYPE))
				return BR_ERROR;
			trimStack(fun->stack);
			p = g_sludge->_speechMan->wrapSpeech(newText, objT, fileNum, sayIt);
			fun->timeLeft = p;
			//debugOut ("BUILTIN: sayCore: %s (%i)\n", newText, p);
			fun->isSpeech = true;
			return BR_KEEP_AND_PAUSE;

		default:
			break;
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
	fun->reg.setVariable(SVT_INT, g_sludge->_gfxMan->howFrozen());
	return BR_CONTINUE;
}

builtIn(setCursor) {
	UNUSEDALL
	PersonaAnimation *aa = fun->stack->thisVar.getAnimationFromVar();
	g_sludge->_cursorMan->pickAnimCursor(aa);
	trimStack(fun->stack);
	return BR_CONTINUE;
}

builtIn(getMouseX) {
	UNUSEDALL
	fun->reg.setVariable(SVT_INT, g_sludge->_evtMan->mouseX() + g_sludge->_gfxMan->getCamX());
	return BR_CONTINUE;
}

builtIn(getMouseY) {
	UNUSEDALL
	fun->reg.setVariable(SVT_INT, g_sludge->_evtMan->mouseY() + g_sludge->_gfxMan->getCamY());
	return BR_CONTINUE;
}

builtIn(getMouseScreenX) {
	UNUSEDALL
	fun->reg.setVariable(SVT_INT, g_sludge->_evtMan->mouseX() * g_sludge->_gfxMan->getCamZoom());
	return BR_CONTINUE;
}

builtIn(getMouseScreenY) {
	UNUSEDALL
	fun->reg.setVariable(SVT_INT, g_sludge->_evtMan->mouseY() * g_sludge->_gfxMan->getCamZoom());
	return BR_CONTINUE;
}

builtIn(getStatusText) {
	UNUSEDALL
	fun->reg.makeTextVar(statusBarText());
	return BR_CONTINUE;
}

builtIn(getMatchingFiles) {
	UNUSEDALL
	Common::String newText = fun->stack->thisVar.getTextFromAnyVar();
	trimStack(fun->stack);
	fun->reg.unlinkVar();

	// Return value
	fun->reg.varType = SVT_STACK;
	fun->reg.varData.theStack = new StackHandler;
	if (!checkNew(fun->reg.varData.theStack))
		return BR_ERROR;
	fun->reg.varData.theStack->first = NULL;
	fun->reg.varData.theStack->last = NULL;
	fun->reg.varData.theStack->timesUsed = 1;
	if (!fun->reg.varData.theStack->getSavedGamesStack(newText))
		return BR_ERROR;
	return BR_CONTINUE;
}

builtIn(saveGame) {
	UNUSEDALL

	if (g_sludge->_gfxMan->isFrozen()) {
		fatal("Can't save game state while the engine is frozen");
	}

	g_sludge->loadNow = fun->stack->thisVar.getTextFromAnyVar();
	trimStack(fun->stack);

	Common::String aaaaa = encodeFilename(g_sludge->loadNow);
	g_sludge->loadNow.clear();
	if (failSecurityCheck(aaaaa))
		return BR_ERROR;      // Won't fail if encoded, how cool is that? OK, not very.

	g_sludge->loadNow = ":" + aaaaa;

	fun->reg.setVariable(SVT_INT, 0);
	saverFunc = fun;
	return BR_KEEP_AND_PAUSE;
}

builtIn(fileExists) {
	UNUSEDALL
	g_sludge->loadNow = fun->stack->thisVar.getTextFromAnyVar();
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
	fun->reg.setVariable(SVT_INT, exist);
	return BR_CONTINUE;
}

builtIn(loadGame) {
	UNUSEDALL
	Common::String aaaaa = fun->stack->thisVar.getTextFromAnyVar();
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
	if (!fun->stack->thisVar.getValueType(y2, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(x2, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(y1, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(x1, SVT_INT))
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
	if (!fun->stack->thisVar.getValueType(yPos, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(xPos, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(fileNumber, SVT_FILE))
		return BR_ERROR;
	trimStack(fun->stack);
	g_sludge->_gfxMan->loadBackDrop(fileNumber, xPos, yPos);
	return BR_CONTINUE;
}

builtIn(mixOverlay) {
	UNUSEDALL
	int fileNumber, xPos, yPos;
	if (!fun->stack->thisVar.getValueType(yPos, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(xPos, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(fileNumber, SVT_FILE))
		return BR_ERROR;
	trimStack(fun->stack);
	g_sludge->_gfxMan->mixBackDrop(fileNumber, xPos, yPos);
	return BR_CONTINUE;
}

builtIn(pasteImage) {
	UNUSEDALL
	int x, y;
	if (!fun->stack->thisVar.getValueType(y, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(x, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	PersonaAnimation *pp = fun->stack->thisVar.getAnimationFromVar();
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
	if (!fun->stack->thisVar.getValueType(y, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(x, SVT_INT))
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
	if (!fun->stack->thisVar.getValueType(cameraY, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(cameraX, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);

	g_sludge->_gfxMan->aimCamera(cameraX, cameraY);

	return BR_CONTINUE;
}

builtIn(zoomCamera) {
	UNUSEDALL
	int z;
	if (!fun->stack->thisVar.getValueType(z, SVT_INT))
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
			fun->reg.copyFrom(fun->stack->thisVar);
		trimStack(fun->stack);
	}
	return BR_CONTINUE;
}

builtIn(substring) {
	UNUSEDALL
	Common::String wholeString;
	int start, length;

	//debugOut ("BUILTIN: substring\n");

	if (!fun->stack->thisVar.getValueType(length, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(start, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	wholeString = fun->stack->thisVar.getTextFromAnyVar();
	trimStack(fun->stack);

	Common::U32String str32 = wholeString.decode(Common::kUtf8);

	if ((int)str32.size() < start + length) {
		length = str32.size() - start;
		if ((int)str32.size() < start) {
			start = 0;
		}
	}
	if (length < 0) {
		length = 0;
	}

	Common::String newString = str32.substr(start, length).encode(Common::kUtf8);

	fun->reg.makeTextVar(newString);
	return BR_CONTINUE;
}

builtIn(stringLength) {
	UNUSEDALL
	Common::String newText = fun->stack->thisVar.getTextFromAnyVar();
	trimStack(fun->stack);
	fun->reg.setVariable(SVT_INT, g_sludge->_txtMan->stringLength(newText));
	return BR_CONTINUE;
}

builtIn(newStack) {
	UNUSEDALL
	fun->reg.unlinkVar();

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
			fun->reg.setVariable(SVT_INT, fun->stack->thisVar.varData.theStack->getStackSize());
			trimStack(fun->stack);
			return BR_CONTINUE;

		case SVT_FASTARRAY:
			// Return value
			fun->reg.setVariable(SVT_INT, fun->stack->thisVar.varData.fastArray->size);
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
	if (!fun->reg.copyStack(fun->stack->thisVar))
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
	fun->reg.setVariable(SVT_INT, deleteVarFromStack(fun->stack->thisVar, fun->stack->next->thisVar.varData.theStack->first, false));

	// Horrible hacking because 'last' value might now be wrong!
	VariableStack *nextFirstStack = fun->stack->next->thisVar.varData.theStack->first;
	fun->stack->next->thisVar.varData.theStack->last = (nextFirstStack == NULL) ? NULL : nextFirstStack->stackFindLast();

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
	fun->reg.setVariable(SVT_INT, deleteVarFromStack(fun->stack->thisVar, fun->stack->next->thisVar.varData.theStack->first, true));

	// Horrible hacking because 'last' value might now be wrong!
	VariableStack *nextFirstStack = fun->stack->next->thisVar.varData.theStack->first;
	fun->stack->next->thisVar.varData.theStack->last = (nextFirstStack == NULL) ? NULL : nextFirstStack->stackFindLast();

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
	fun->reg.copyFrom(fun->stack->thisVar.varData.theStack->first->thisVar);
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
	fun->reg.copyFrom(fun->stack->thisVar.varData.theStack->first->thisVar);
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
	fun->reg.copyFrom(fun->stack->thisVar.varData.theStack->last->thisVar);
	trimStack(fun->stack);
	return BR_CONTINUE;
}

builtIn(random) {
	UNUSEDALL
	int num;

	if (!fun->stack->thisVar.getValueType(num, SVT_INT))
		return BR_ERROR;

	trimStack(fun->stack);
	if (num <= 0)
		num = 1;
	fun->reg.setVariable(SVT_INT, g_sludge->getRandomSource()->getRandomNumber(num - 1));
	return BR_CONTINUE;
}

static bool getRGBParams(int &red, int &green, int &blue, LoadedFunction *fun) {
	if (!fun->stack->thisVar.getValueType(blue, SVT_INT))
		return false;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(green, SVT_INT))
		return false;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(red, SVT_INT))
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

	g_sludge->_txtMan->setPasterColor((byte)red, (byte)green, (byte)blue);
	return BR_CONTINUE;
}

builtIn(setBlankColour) {
	UNUSEDALL
	int red, green, blue;

	if (!getRGBParams(red, green, blue, fun))
		return BR_ERROR;

	g_sludge->_gfxMan->setBlankColor(red, green, blue);
	fun->reg.setVariable(SVT_INT, 1);
	return BR_CONTINUE;
}

builtIn(setBurnColour) {
	UNUSEDALL
	int red, green, blue;

	if (!getRGBParams(red, green, blue, fun))
		return BR_ERROR;

	g_sludge->_gfxMan->setBurnColor(red, green, blue);
	fun->reg.setVariable(SVT_INT, 1);
	return BR_CONTINUE;
}

builtIn(setFont) {
	UNUSEDALL
	int fileNumber, newHeight;
	if (!fun->stack->thisVar.getValueType(newHeight, SVT_INT))
		return BR_ERROR;
	//              newDebug ("  Height:", newHeight);
	trimStack(fun->stack);
	Common::String newText = fun->stack->thisVar.getTextFromAnyVar();
	//              newDebug ("  Character supported:", newText);
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(fileNumber, SVT_FILE))
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
	Common::String newText = fun->stack->thisVar.getTextFromAnyVar();
	trimStack(fun->stack);

	// Return value
	fun->reg.setVariable(SVT_INT, g_sludge->_txtMan->isInFont(newText));
	return BR_CONTINUE;
}

builtIn(pasteString) {
	UNUSEDALL
	Common::String newText = fun->stack->thisVar.getTextFromAnyVar();
	trimStack(fun->stack);
	int y, x;
	if (!fun->stack->thisVar.getValueType(y, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(x, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (x == IN_THE_CENTRE)
		x = g_sludge->_gfxMan->getCenterX(g_sludge->_txtMan->stringWidth(newText));
	g_sludge->_txtMan->pasteStringToBackdrop(newText, x, y);
	return BR_CONTINUE;
}

builtIn(anim) {
	UNUSEDALL
	if (numParams < 2) {
		fatal("Built-in function anim() must have at least 2 parameters.");
		return BR_ERROR;
	}

	// First store the frame numbers and take 'em off the stack
	PersonaAnimation *ba = new PersonaAnimation(numParams - 1, fun->stack);

	// Only remaining paramter is the file number
	int fileNumber;
	if (!fun->stack->thisVar.getValueType(fileNumber, SVT_FILE))
		return BR_ERROR;
	trimStack(fun->stack);

	// Load the required sprite bank
	LoadedSpriteBank *sprBanky = g_sludge->_gfxMan->loadBankForAnim(fileNumber);
	if (!sprBanky) {
		delete ba;
		return BR_ERROR;    // File not found, fatal done already
	}
	ba->theSprites = sprBanky;

	// Return value
	fun->reg.makeAnimationVariable(ba);

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
		newPersona->animation[iii] = fun->stack->thisVar.getAnimationFromVar();
		trimStack(fun->stack);
	}

	// Return value
	fun->reg.makeCostumeVariable(newPersona);
	return BR_CONTINUE;
}

builtIn(launch) {
	UNUSEDALL
	Common::String newTextA = fun->stack->thisVar.getTextFromAnyVar();

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
	fun->reg.setVariable(SVT_INT, 1);
	launchResult = &fun->reg;

	return BR_KEEP_AND_PAUSE;
}

builtIn(pause) {
	UNUSEDALL
	int theTime;
	if (!fun->stack->thisVar.getValueType(theTime, SVT_INT))
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
	if (!fun->stack->thisVar.getValueType(obj2, SVT_OBJTYPE))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(obj1, SVT_OBJTYPE))
		return BR_ERROR;
	trimStack(fun->stack);

	int fNum = g_sludge->_objMan->getCombinationFunction(obj1, obj2);

	// Return value
	if (fNum) {
		fun->reg.setVariable(SVT_FUNC, fNum);
		return BR_CALLAFUNC;
	}
	fun->reg.setVariable(SVT_INT, 0);
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
	fun->reg.setVariable(SVT_INT, 0);
	return BR_CONTINUE;
}

builtIn(_rem_moviePlaying) {
	UNUSEDALL
	fun->reg.setVariable(SVT_INT, 0);
	return BR_CONTINUE;
}

builtIn(playMovie) {
	UNUSEDALL
	int fileNumber, r;

	if (movieIsPlaying)
		return BR_PAUSE;

	if (!fun->stack->thisVar.getValueType(fileNumber, SVT_FILE))
		return BR_ERROR;
	trimStack(fun->stack);

	r = playMovie(fileNumber);

	fun->reg.setVariable(SVT_INT, r);

	if (r && (!fun->next)) {
		restartFunction(fun);
		return BR_ALREADY_GONE;
	}
	return BR_CONTINUE;
}

builtIn(stopMovie) {
	UNUSEDALL

	stopMovie();

	fun->reg.setVariable(SVT_INT, 0);
	return BR_CONTINUE;
}

builtIn(pauseMovie) {
	UNUSEDALL

	pauseMovie();

	fun->reg.setVariable(SVT_INT, 0);
	return BR_CONTINUE;
}

#pragma mark -
#pragma mark Audio functions

builtIn(startMusic) {
	UNUSEDALL
	int fromTrack, musChan, fileNumber;
	if (!fun->stack->thisVar.getValueType(fromTrack, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(musChan, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(fileNumber, SVT_FILE))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!g_sludge->_soundMan->playMOD(fileNumber, musChan, fromTrack))
		return BR_CONTINUE;  //BR_ERROR;
	return BR_CONTINUE;
}

builtIn(stopMusic) {
	UNUSEDALL
	int v;
	if (!fun->stack->thisVar.getValueType(v, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	g_sludge->_soundMan->stopMOD(v);
	return BR_CONTINUE;
}

builtIn(setMusicVolume) {
	UNUSEDALL
	int musChan, v;
	if (!fun->stack->thisVar.getValueType(v, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(musChan, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	g_sludge->_soundMan->setMusicVolume(musChan, v);
	return BR_CONTINUE;
}

builtIn(setDefaultMusicVolume) {
	UNUSEDALL
	int v;
	if (!fun->stack->thisVar.getValueType(v, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	g_sludge->_soundMan->setDefaultMusicVolume(v);
	return BR_CONTINUE;
}

builtIn(playSound) {
	UNUSEDALL
	int fileNumber;
	if (!fun->stack->thisVar.getValueType(fileNumber, SVT_FILE))
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

		if (!fun->stack->thisVar.getValueType(fileNumber, SVT_FILE))
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
			fun->stack->thisVar.getValueType(doLoop, SVT_INT);
			trimStack(fun->stack);
			numParams--;
		}
		while (numParams) {
			if (!fun->stack->thisVar.getValueType(fileNumber, SVT_FILE)) {
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
	if (!fun->stack->thisVar.getValueType(v, SVT_FILE))
		return BR_ERROR;
	trimStack(fun->stack);
	g_sludge->_soundMan->huntKillSound(v);
	return BR_CONTINUE;
}

builtIn(setDefaultSoundVolume) {
	UNUSEDALL
	int v;
	if (!fun->stack->thisVar.getValueType(v, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	g_sludge->_soundMan->setDefaultSoundVolume(v);
	return BR_CONTINUE;
}

builtIn(setSoundVolume) {
	UNUSEDALL
	int musChan, v;
	if (!fun->stack->thisVar.getValueType(v, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(musChan, SVT_FILE))
		return BR_ERROR;
	trimStack(fun->stack);
	g_sludge->_soundMan->setSoundVolume(musChan, v);
	return BR_CONTINUE;
}

builtIn(setSoundLoopPoints) {
	UNUSEDALL
	int musChan, theEnd, theStart;
	if (!fun->stack->thisVar.getValueType(theEnd, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(theStart, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(musChan, SVT_FILE))
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
		fun->stack->thisVar.getValueType(v, SVT_FILE);
		trimStack(fun->stack);
		if (!g_sludge->_floorMan->setFloor(v))
			return BR_ERROR;
	} else {
		trimStack(fun->stack);
		g_sludge->_floorMan->setFloorNull();
	}
	return BR_CONTINUE;
}

builtIn(showFloor) {
	UNUSEDALL
	g_sludge->_floorMan->drawFloor();
	return BR_CONTINUE;
}

builtIn(setZBuffer) {
	UNUSEDALL
	if (fun->stack->thisVar.varType == SVT_FILE) {
		int v;
		fun->stack->thisVar.getValueType(v, SVT_FILE);
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
			if (!fun->stack->thisVar.getValueType(g_sludge->_gfxMan->_lightMapMode, SVT_INT))
				return BR_ERROR;
			trimStack(fun->stack);
			g_sludge->_gfxMan->_lightMapMode %= LIGHTMAPMODE_NUM;
			// fall through

		case 1:
			if (fun->stack->thisVar.varType == SVT_FILE) {
				int v;
				fun->stack->thisVar.getValueType(v, SVT_FILE);
				trimStack(fun->stack);
				if (!g_sludge->_gfxMan->loadLightMap(v))
					return BR_ERROR;
				fun->reg.setVariable(SVT_INT, 1);
			} else {
				trimStack(fun->stack);
				g_sludge->_gfxMan->killLightMap();
				fun->reg.setVariable(SVT_INT, 0);
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
	int speechMode;
	if (!fun->stack->thisVar.getValueType(speechMode, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (speechMode < 0 || speechMode > 2) {
		fatal("Valid parameters are be SPEECHANDTEXT, SPEECHONLY or TEXTONLY");
		return BR_ERROR;
	}
	g_sludge->_speechMan->setSpeechMode(speechMode);
	return BR_CONTINUE;
}

builtIn(somethingSpeaking) {
	UNUSEDALL
	int i = g_sludge->_speechMan->isThereAnySpeechGoingOn();
	if (i == -1) {
		fun->reg.setVariable(SVT_INT, 0);
	} else {
		fun->reg.setVariable(SVT_OBJTYPE, i);
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
	if (g_sludge->_regionMan->getOverRegion())
		// Return value
		fun->reg.setVariable(SVT_OBJTYPE, g_sludge->_regionMan->getOverRegion()->thisType->objectNum);
	else
		// Return value
		fun->reg.setVariable(SVT_INT, 0);
	return BR_CONTINUE;
}

builtIn(rename) {
	UNUSEDALL
	Common::String newText = fun->stack->thisVar.getTextFromAnyVar();
	int objT;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(objT, SVT_OBJTYPE))
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
	if (!fun->stack->thisVar.getValueType(objectNumber, SVT_OBJTYPE))
		return BR_ERROR;
	trimStack(fun->stack);

	OnScreenPerson *pers = g_sludge->_peopleMan->findPerson(objectNumber);
	if (pers) {
		fun->reg.setVariable(SVT_INT, pers->x);
	} else {
		ScreenRegion *la = g_sludge->_regionMan->getRegionForObject(objectNumber);
		if (la) {
			fun->reg.setVariable(SVT_INT, la->sX);
		} else {
			fun->reg.setVariable(SVT_INT, 0);
		}
	}
	return BR_CONTINUE;
}

builtIn(getObjectY) {
	UNUSEDALL
	int objectNumber;
	if (!fun->stack->thisVar.getValueType(objectNumber, SVT_OBJTYPE))
		return BR_ERROR;
	trimStack(fun->stack);

	OnScreenPerson *pers = g_sludge->_peopleMan->findPerson(objectNumber);
	if (pers) {
		fun->reg.setVariable(SVT_INT, pers->y);
	} else {
		ScreenRegion *la = g_sludge->_regionMan->getRegionForObject(objectNumber);
		if (la) {
			fun->reg.setVariable(SVT_INT, la->sY);
		} else {
			fun->reg.setVariable(SVT_INT, 0);
		}
	}
	return BR_CONTINUE;
}

builtIn(addScreenRegion) {
	UNUSEDALL
	int sX, sY, x1, y1, x2, y2, di, objectNumber;
	if (!fun->stack->thisVar.getValueType(di, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(sY, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(sX, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(y2, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(x2, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(y1, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(x1, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(objectNumber, SVT_OBJTYPE))
		return BR_ERROR;
	trimStack(fun->stack);
	if (g_sludge->_regionMan->addScreenRegion(x1, y1, x2, y2, sX, sY, di, objectNumber))
		return BR_CONTINUE;
	return BR_ERROR;

}

builtIn(removeScreenRegion) {
	UNUSEDALL
	int objectNumber;
	if (!fun->stack->thisVar.getValueType(objectNumber, SVT_OBJTYPE))
		return BR_ERROR;
	trimStack(fun->stack);
	g_sludge->_regionMan->removeScreenRegion(objectNumber);
	return BR_CONTINUE;
}

builtIn(showBoxes) {
	UNUSEDALL
	g_sludge->_regionMan->showBoxes();
	return BR_CONTINUE;
}

builtIn(removeAllScreenRegions) {
	UNUSEDALL
	g_sludge->_regionMan->kill();
	return BR_CONTINUE;
}

builtIn(addCharacter) {
	UNUSEDALL
	Persona *p;
	int x, y, objectNumber;

	p = fun->stack->thisVar.getCostumeFromVar();
	if (p == NULL)
		return BR_ERROR;

	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(y, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(x, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(objectNumber, SVT_OBJTYPE))
		return BR_ERROR;
	trimStack(fun->stack);
	if (g_sludge->_peopleMan->addPerson(x, y, objectNumber, p))
		return BR_CONTINUE;
	return BR_ERROR;
}

builtIn(hideCharacter) {
	UNUSEDALL
	int objectNumber;
	if (!fun->stack->thisVar.getValueType(objectNumber, SVT_OBJTYPE))
		return BR_ERROR;
	trimStack(fun->stack);
	g_sludge->_peopleMan->setShown(false, objectNumber);
	return BR_CONTINUE;
}

builtIn(showCharacter) {
	UNUSEDALL
	int objectNumber;
	if (!fun->stack->thisVar.getValueType(objectNumber, SVT_OBJTYPE))
		return BR_ERROR;
	trimStack(fun->stack);
	g_sludge->_peopleMan->setShown(true, objectNumber);
	return BR_CONTINUE;
}

builtIn(removeAllCharacters) {
	UNUSEDALL
	killSpeechTimers();
	g_sludge->_peopleMan->killMostPeople();
	return BR_CONTINUE;
}

builtIn(setCharacterDrawMode) {
	UNUSEDALL
	int obj, di;
	if (!fun->stack->thisVar.getValueType(di, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(obj, SVT_OBJTYPE))
		return BR_ERROR;
	trimStack(fun->stack);
	g_sludge->_peopleMan->setDrawMode(di, obj);
	return BR_CONTINUE;
}
builtIn(setCharacterTransparency) {
	UNUSEDALL
	int obj, x;
	if (!fun->stack->thisVar.getValueType(x, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(obj, SVT_OBJTYPE))
		return BR_ERROR;
	trimStack(fun->stack);
	g_sludge->_peopleMan->setPersonTransparency(obj, x);
	return BR_CONTINUE;
}
builtIn(setCharacterColourise) {
	UNUSEDALL
	int obj, r, g, b, mix;
	if (!fun->stack->thisVar.getValueType(mix, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(b, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(g, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(r, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(obj, SVT_OBJTYPE))
		return BR_ERROR;
	trimStack(fun->stack);
	g_sludge->_peopleMan->setPersonColourise(obj, r, g, b, mix);
	return BR_CONTINUE;
}

builtIn(setScale) {
	UNUSEDALL
	int val1, val2;
	if (!fun->stack->thisVar.getValueType(val2, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(val1, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	g_sludge->_peopleMan->setScale((int16)val1, (int16)val2);
	return BR_CONTINUE;
}

builtIn(stopCharacter) {
	UNUSEDALL
	int obj;
	if (!fun->stack->thisVar.getValueType(obj, SVT_OBJTYPE))
		return BR_ERROR;
	trimStack(fun->stack);

	// Return value
	fun->reg.setVariable(SVT_INT, g_sludge->_peopleMan->stopPerson(obj));
	return BR_CONTINUE;
}

builtIn(pasteCharacter) {
	UNUSEDALL
	int obj;
	if (!fun->stack->thisVar.getValueType(obj, SVT_OBJTYPE))
		return BR_ERROR;
	trimStack(fun->stack);

	OnScreenPerson *thisPerson = g_sludge->_peopleMan->findPerson(obj);
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
		fun->reg.setVariable(SVT_INT, 1);
	} else {
		fun->reg.setVariable(SVT_INT, 0);
	}
	return BR_CONTINUE;
}

builtIn(animate) {
	UNUSEDALL
	int obj;
	PersonaAnimation *pp = fun->stack->thisVar.getAnimationFromVar();
	if (pp == NULL)
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(obj, SVT_OBJTYPE))
		return BR_ERROR;
	trimStack(fun->stack);
	g_sludge->_peopleMan->animatePerson(obj, pp);
	fun->reg.setVariable(SVT_INT, pp->getTotalTime());
	return BR_CONTINUE;
}

builtIn(setCostume) {
	UNUSEDALL
	int obj;
	Persona *pp = fun->stack->thisVar.getCostumeFromVar();
	if (pp == NULL)
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(obj, SVT_OBJTYPE))
		return BR_ERROR;
	trimStack(fun->stack);
	g_sludge->_peopleMan->animatePerson(obj, pp);
	return BR_CONTINUE;
}

builtIn(floatCharacter) {
	UNUSEDALL
	int obj, di;
	if (!fun->stack->thisVar.getValueType(di, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(obj, SVT_OBJTYPE))
		return BR_ERROR;
	trimStack(fun->stack);
	fun->reg.setVariable(SVT_INT, g_sludge->_peopleMan->floatCharacter(di, obj));
	return BR_CONTINUE;
}

builtIn(setCharacterWalkSpeed) {
	UNUSEDALL
	int obj, di;
	if (!fun->stack->thisVar.getValueType(di, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(obj, SVT_OBJTYPE))
		return BR_ERROR;
	trimStack(fun->stack);
	fun->reg.setVariable(SVT_INT, g_sludge->_peopleMan->setCharacterWalkSpeed(di, obj));
	return BR_CONTINUE;
}

builtIn(turnCharacter) {
	UNUSEDALL
	int obj, di;
	if (!fun->stack->thisVar.getValueType(di, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(obj, SVT_OBJTYPE))
		return BR_ERROR;
	trimStack(fun->stack);
	fun->reg.setVariable(SVT_INT, g_sludge->_peopleMan->turnPersonToFace(obj, di));
	return BR_CONTINUE;
}

builtIn(setCharacterExtra) {
	UNUSEDALL
	int obj, di;
	if (!fun->stack->thisVar.getValueType(di, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(obj, SVT_OBJTYPE))
		return BR_ERROR;
	trimStack(fun->stack);
	fun->reg.setVariable(SVT_INT, g_sludge->_peopleMan->setPersonExtra(obj, di));
	return BR_CONTINUE;
}

builtIn(removeCharacter) {
	UNUSEDALL
	int objectNumber;
	if (!fun->stack->thisVar.getValueType(objectNumber, SVT_OBJTYPE))
		return BR_ERROR;
	trimStack(fun->stack);
	g_sludge->_peopleMan->removeOneCharacter(objectNumber);
	return BR_CONTINUE;
}

static BuiltReturn moveChr(int numParams, LoadedFunction *fun, bool force, bool immediate) {
	switch (numParams) {
		case 3: {
			int x, y, objectNumber;

			if (!fun->stack->thisVar.getValueType(y, SVT_INT))
				return BR_ERROR;
			trimStack(fun->stack);
			if (!fun->stack->thisVar.getValueType(x, SVT_INT))
				return BR_ERROR;
			trimStack(fun->stack);
			if (!fun->stack->thisVar.getValueType(objectNumber, SVT_OBJTYPE))
				return BR_ERROR;
			trimStack(fun->stack);

			if (force) {
				if (g_sludge->_peopleMan->forceWalkingPerson(x, y, objectNumber, fun, -1))
					return BR_PAUSE;
			} else if (immediate) {
				g_sludge->_peopleMan->jumpPerson(x, y, objectNumber);
			} else {
				if (g_sludge->_peopleMan->makeWalkingPerson(x, y, objectNumber, fun, -1))
					return BR_PAUSE;
			}
			return BR_CONTINUE;
		}

		case 2: {
			int toObj, objectNumber;
			ScreenRegion*reggie;

			if (!fun->stack->thisVar.getValueType(toObj, SVT_OBJTYPE))
				return BR_ERROR;
			trimStack(fun->stack);
			if (!fun->stack->thisVar.getValueType(objectNumber, SVT_OBJTYPE))
				return BR_ERROR;
			trimStack(fun->stack);
			reggie = g_sludge->_regionMan->getRegionForObject(toObj);
			if (reggie == NULL)
				return BR_CONTINUE;

			if (force) {
				if (g_sludge->_peopleMan->forceWalkingPerson(reggie->sX, reggie->sY, objectNumber, fun, reggie->di))
					return BR_PAUSE;
			} else if (immediate) {
				g_sludge->_peopleMan->jumpPerson(reggie->sX, reggie->sY, objectNumber);
			} else {
				if (g_sludge->_peopleMan->makeWalkingPerson(reggie->sX, reggie->sY, objectNumber, fun, reggie->di))
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
	Common::String newText = fun->stack->thisVar.getTextFromAnyVar();
	trimStack(fun->stack);
	setStatusBar(newText);
	return BR_CONTINUE;
}

builtIn(lightStatus) {
	UNUSEDALL
	int val;
	if (!fun->stack->thisVar.getValueType(val, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	setLitStatus(val);
	return BR_CONTINUE;
}

builtIn(positionStatus) {
	UNUSEDALL
	int x, y;
	if (!fun->stack->thisVar.getValueType(y, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(x, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	positionStatus(x, y);
	return BR_CONTINUE;
}

builtIn(alignStatus) {
	UNUSEDALL
	int val;
	if (!fun->stack->thisVar.getValueType(val, SVT_INT))
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
			if (!fun->stack->thisVar.getValueType(functionNum, SVT_FUNC))
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
	Common::String theText = fun->stack->thisVar.getTextFromAnyVar();
	trimStack(fun->stack);

	// Return value
	fun->reg.setVariable(SVT_INT, g_sludge->_txtMan->stringWidth(theText));
	return BR_CONTINUE;
}

builtIn(hardScroll) {
	UNUSEDALL
	int v;
	if (!fun->stack->thisVar.getValueType(v, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	g_sludge->_gfxMan->hardScroll(v);
	return BR_CONTINUE;
}

builtIn(isScreenRegion) {
	UNUSEDALL
	int objectNumber;
	if (!fun->stack->thisVar.getValueType(objectNumber, SVT_OBJTYPE))
		return BR_ERROR;
	trimStack(fun->stack);
	fun->reg.setVariable(SVT_INT, g_sludge->_regionMan->getRegionForObject(objectNumber) != NULL);
	return BR_CONTINUE;
}

builtIn(setSpeechSpeed) {
	UNUSEDALL
	int number;
	if (!fun->stack->thisVar.getValueType(number, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	g_sludge->_speechMan->setSpeechSpeed(number * 0.01);
	fun->reg.setVariable(SVT_INT, 1);
	return BR_CONTINUE;
}

builtIn(setFontSpacing) {
	UNUSEDALL
	int fontSpaceI;
	if (!fun->stack->thisVar.getValueType(fontSpaceI, SVT_INT))
		return BR_ERROR;
	g_sludge->_txtMan->setFontSpace(fontSpaceI);
	trimStack(fun->stack);
	fun->reg.setVariable(SVT_INT, 1);
	return BR_CONTINUE;
}

builtIn(transitionLevel) {
	UNUSEDALL
	int brightnessLevel;
	if (!fun->stack->thisVar.getValueType(brightnessLevel, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);

	g_sludge->_gfxMan->setBrightnessLevel(brightnessLevel);

	fun->reg.setVariable(SVT_INT, 1);
	return BR_CONTINUE;
}

builtIn(captureAllKeys) {
	UNUSEDALL
	// This built-in function doesn't have any effect any more, we capture all keys by default
	bool captureAllKeysDeprecated = fun->stack->thisVar.getBoolean();
	trimStack(fun->stack);
	fun->reg.setVariable(SVT_INT, captureAllKeysDeprecated);
	return BR_CONTINUE;
}

builtIn(spinCharacter) {
	UNUSEDALL
	int number, objectNumber;
	if (!fun->stack->thisVar.getValueType(number, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(objectNumber, SVT_OBJTYPE))
		return BR_ERROR;
	trimStack(fun->stack);

	OnScreenPerson *thisPerson = g_sludge->_peopleMan->findPerson(objectNumber);
	if (thisPerson) {
		thisPerson->wantAngle = number;
		thisPerson->spinning = true;
		thisPerson->continueAfterWalking = fun;
		fun->reg.setVariable(SVT_INT, 1);
		return BR_PAUSE;
	} else {
		fun->reg.setVariable(SVT_INT, 0);
		return BR_CONTINUE;
	}
}

builtIn(getCharacterDirection) {
	UNUSEDALL
	int objectNumber;
	if (!fun->stack->thisVar.getValueType(objectNumber, SVT_OBJTYPE))
		return BR_ERROR;
	trimStack(fun->stack);
	OnScreenPerson *thisPerson = g_sludge->_peopleMan->findPerson(objectNumber);
	if (thisPerson) {
		fun->reg.setVariable(SVT_INT, thisPerson->direction);
	} else {
		fun->reg.setVariable(SVT_INT, 0);
	}
	return BR_CONTINUE;
}

builtIn(isCharacter) {
	UNUSEDALL
	int objectNumber;
	if (!fun->stack->thisVar.getValueType(objectNumber, SVT_OBJTYPE))
		return BR_ERROR;
	trimStack(fun->stack);
	OnScreenPerson *thisPerson = g_sludge->_peopleMan->findPerson(objectNumber);
	fun->reg.setVariable(SVT_INT, thisPerson != NULL);
	return BR_CONTINUE;
}

builtIn(normalCharacter) {
	UNUSEDALL
	int objectNumber;
	if (!fun->stack->thisVar.getValueType(objectNumber, SVT_OBJTYPE))
		return BR_ERROR;
	trimStack(fun->stack);
	OnScreenPerson *thisPerson = g_sludge->_peopleMan->findPerson(objectNumber);
	if (thisPerson) {
		thisPerson->myAnim = thisPerson->myPersona->animation[thisPerson->direction];
		fun->reg.setVariable(SVT_INT, 1);
	} else {
		fun->reg.setVariable(SVT_INT, 0);
	}
	return BR_CONTINUE;
}

builtIn(isMoving) {
	UNUSEDALL
	int objectNumber;
	if (!fun->stack->thisVar.getValueType(objectNumber, SVT_OBJTYPE))
		return BR_ERROR;
	trimStack(fun->stack);
	OnScreenPerson *thisPerson = g_sludge->_peopleMan->findPerson(objectNumber);
	if (thisPerson) {
		fun->reg.setVariable(SVT_INT, thisPerson->walking);
	} else {
		fun->reg.setVariable(SVT_INT, 0);
	}
	return BR_CONTINUE;
}

builtIn(fetchEvent) {
	UNUSEDALL
	int obj1, obj2;
	if (!fun->stack->thisVar.getValueType(obj2, SVT_OBJTYPE))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(obj1, SVT_OBJTYPE))
		return BR_ERROR;
	trimStack(fun->stack);

	int fNum = g_sludge->_objMan->getCombinationFunction(obj1, obj2);

	// Return value
	if (fNum) {
		fun->reg.setVariable(SVT_FUNC, fNum);
	} else {
		fun->reg.setVariable(SVT_INT, 0);
	}
	return BR_CONTINUE;
}

builtIn(deleteFile) {
	UNUSEDALL

	Common::String namNormal = fun->stack->thisVar.getTextFromAnyVar();
	trimStack(fun->stack);
	Common::String nam = encodeFilename(namNormal);
	namNormal.clear();
	if (failSecurityCheck(nam))
		return BR_ERROR;
	fun->reg.setVariable(SVT_INT, remove(nam.c_str()));

	return BR_CONTINUE;
}

builtIn(renameFile) {
	UNUSEDALL
	Common::String temp;

	temp.clear();
	temp = fun->stack->thisVar.getTextFromAnyVar();
	Common::String newnam = encodeFilename(temp);
	trimStack(fun->stack);
	if (failSecurityCheck(newnam))
		return BR_ERROR;
	temp.clear();

	temp = fun->stack->thisVar.getTextFromAnyVar();
	Common::String nam = encodeFilename(temp);
	trimStack(fun->stack);
	if (failSecurityCheck(nam))
		return BR_ERROR;

	fun->reg.setVariable(SVT_INT, rename(nam.c_str(), newnam.c_str()));

	return BR_CONTINUE;
}

builtIn(cacheSound) {
	UNUSEDALL
	int fileNumber;
	if (!fun->stack->thisVar.getValueType(fileNumber, SVT_FILE))
		return BR_ERROR;
	trimStack(fun->stack);
	if (g_sludge->_soundMan->cacheSound(fileNumber) == -1)
		return BR_ERROR;
	return BR_CONTINUE;
}

builtIn(burnString) {
	UNUSEDALL
	Common::String newText = fun->stack->thisVar.getTextFromAnyVar();
	trimStack(fun->stack);
	int y, x;
	if (!fun->stack->thisVar.getValueType(y, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(x, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (x == IN_THE_CENTRE)
		x = g_sludge->_gfxMan->getCenterX(g_sludge->_txtMan->stringWidth(newText));
	g_sludge->_txtMan->burnStringToBackdrop(newText, x, y);
	return BR_CONTINUE;
}

builtIn(setCharacterSpinSpeed) {
	UNUSEDALL
	int speed, who;
	if (!fun->stack->thisVar.getValueType(speed, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(who, SVT_OBJTYPE))
		return BR_ERROR;
	trimStack(fun->stack);

	OnScreenPerson *thisPerson = g_sludge->_peopleMan->findPerson(who);

	if (thisPerson) {
		thisPerson->spinSpeed = speed;
		fun->reg.setVariable(SVT_INT, 1);
	} else {
		fun->reg.setVariable(SVT_INT, 0);
	}
	return BR_CONTINUE;
}

builtIn(setCharacterAngleOffset) {
	UNUSEDALL
	int angle, who;
	if (!fun->stack->thisVar.getValueType(angle, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(who, SVT_OBJTYPE))
		return BR_ERROR;
	trimStack(fun->stack);

	OnScreenPerson *thisPerson = g_sludge->_peopleMan->findPerson(who);

	if (thisPerson) {
		thisPerson->angleOffset = angle;
		fun->reg.setVariable(SVT_INT, 1);
	} else {
		fun->reg.setVariable(SVT_INT, 0);
	}
	return BR_CONTINUE;
}

builtIn(transitionMode) {
	UNUSEDALL
	int n;
	if (!fun->stack->thisVar.getValueType(n, SVT_INT))
		return BR_ERROR;
	g_sludge->_gfxMan->setFadeMode(n);
	trimStack(fun->stack);
	fun->reg.setVariable(SVT_INT, 1);
	return BR_CONTINUE;
}

// Removed function - does nothing
builtIn(_rem_updateDisplay) {
	UNUSEDALL
	trimStack(fun->stack);
	fun->reg.setVariable(SVT_INT, true);
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
	Common::String fileNameB = fun->stack->thisVar.getTextFromAnyVar();

	Common::String fileName = encodeFilename(fileNameB);

	if (failSecurityCheck(fileName))
		return BR_ERROR;
	trimStack(fun->stack);

	if (fun->stack->thisVar.varType != SVT_STACK) {
		fatal("First parameter isn't a stack");
		return BR_ERROR;
	}
	if (!CustomSaveHelper::stackToFile(fileName, fun->stack->thisVar))
		return BR_ERROR;
	trimStack(fun->stack);
	return BR_CONTINUE;
}

builtIn(loadCustomData) {
	UNUSEDALL

	Common::String newTextA = fun->stack->thisVar.getTextFromAnyVar();

	Common::String newText = encodeFilename(newTextA);

	if (failSecurityCheck(newText))
		return BR_ERROR;
	trimStack(fun->stack);

	fun->reg.unlinkVar();
	fun->reg.varType = SVT_STACK;
	fun->reg.varData.theStack = new StackHandler;
	if (!checkNew(fun->reg.varData.theStack))
		return BR_ERROR;
	fun->reg.varData.theStack->first = NULL;
	fun->reg.varData.theStack->last = NULL;
	fun->reg.varData.theStack->timesUsed = 1;
	if (!CustomSaveHelper::fileToStack(newText, fun->reg.varData.theStack))
		return BR_ERROR;
	return BR_CONTINUE;
}

builtIn(setCustomEncoding) {
	UNUSEDALL
	int n;
	if (!fun->stack->thisVar.getValueType(n, SVT_INT))
		return BR_ERROR;
	CustomSaveHelper::_saveEncoding = n;
	trimStack(fun->stack);
	fun->reg.setVariable(SVT_INT, 1);
	return BR_CONTINUE;
}

builtIn(freeSound) {
	UNUSEDALL
	int v;
	if (!fun->stack->thisVar.getValueType(v, SVT_FILE))
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
		if (!fun->stack->thisVar.getValueType(wrapY, SVT_INT))
			return BR_ERROR;
		trimStack(fun->stack);
		if (!fun->stack->thisVar.getValueType(wrapX, SVT_INT))
			return BR_ERROR;
		trimStack(fun->stack);
		if (!fun->stack->thisVar.getValueType(v, SVT_FILE))
			return BR_ERROR;
		trimStack(fun->stack);

		if (!g_sludge->_gfxMan->loadParallax(v, wrapX, wrapY))
			return BR_ERROR;
		fun->reg.setVariable(SVT_INT, 1);
	}
	return BR_CONTINUE;
}

builtIn(parallaxClear) {
	UNUSEDALL
	g_sludge->_gfxMan->killParallax();
	fun->reg.setVariable(SVT_INT, 1);
	return BR_CONTINUE;
}

builtIn(getPixelColour) {
	UNUSEDALL
	int x, y;
	if (!fun->stack->thisVar.getValueType(y, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(x, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);

	fun->reg.unlinkVar();
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
			bool success = fun->reg.makeFastArrayFromStack(fun->stack->thisVar.varData.theStack);
			trimStack(fun->stack);
			return success ? BR_CONTINUE : BR_ERROR;
		}
			break;

		case SVT_INT: {
			int i = fun->stack->thisVar.varData.intValue;
			trimStack(fun->stack);
			return fun->reg.makeFastArraySize(i) ? BR_CONTINUE : BR_ERROR;
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
	if (!fun->stack->thisVar.getValueType(objectNumber, SVT_OBJTYPE))
		return BR_ERROR;
	trimStack(fun->stack);

	OnScreenPerson *pers = g_sludge->_peopleMan->findPerson(objectNumber);
	if (pers) {
		fun->reg.setVariable(SVT_INT, pers->scale * 100);
	} else {
		fun->reg.setVariable(SVT_INT, 0);
	}
	return BR_CONTINUE;
}

builtIn(getLanguageID) {
	UNUSEDALL
	fun->reg.setVariable(SVT_INT, g_sludge->getLanguageID());
	return BR_CONTINUE;
}

// Removed function
builtIn(_rem_launchWith) {
	UNUSEDALL

	trimStack(fun->stack);

	// To support some windows only games
	Common::String filename = fun->stack->thisVar.getTextFromAnyVar();
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
	fun->reg.setVariable(SVT_INT, false);
	return BR_CONTINUE;
}

builtIn(getFramesPerSecond) {
	UNUSEDALL
	fun->reg.setVariable(SVT_INT, g_sludge->_timer.getLastFps());
	return BR_CONTINUE;
}

builtIn(showThumbnail) {
	UNUSEDALL
	int x, y;
	if (!fun->stack->thisVar.getValueType(y, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(x, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);

	// Encode the name!Encode the name!
	Common::String aaaaa = fun->stack->thisVar.getTextFromAnyVar();
	trimStack(fun->stack);
	Common::String file = encodeFilename(aaaaa);
	g_sludge->_gfxMan->showThumbnail(file, x, y);
	return BR_CONTINUE;
}

builtIn(setThumbnailSize) {
	UNUSEDALL
	int thumbHeight, thumbWidth;
	if (!fun->stack->thisVar.getValueType(thumbHeight, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(thumbWidth, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!g_sludge->_gfxMan->setThumbnailSize(thumbWidth, thumbHeight)) {
		Common::String buff = Common::String::format("%i x %i", thumbWidth, thumbWidth);
		fatal("Invalid thumbnail size", buff);
		return BR_ERROR;
	}
	return BR_CONTINUE;
}

builtIn(hasFlag) {
	UNUSEDALL
	int objNum, flagIndex;
	if (!fun->stack->thisVar.getValueType(flagIndex, SVT_INT))
		return BR_ERROR;
	trimStack(fun->stack);
	if (!fun->stack->thisVar.getValueType(objNum, SVT_OBJTYPE))
		return BR_ERROR;
	trimStack(fun->stack);
	ObjectType *objT = g_sludge->_objMan->findObjectType(objNum);
	if (!objT)
		return BR_ERROR;
	fun->reg.setVariable(SVT_INT, objT->flags & (1 << flagIndex));
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
	allowAnyFilename = fun->stack->thisVar.getBoolean();
	trimStack(fun->stack);
	fun->reg.setVariable(SVT_INT, lastValue);
	return BR_CONTINUE;
}

// Deprecated - does nothing.
builtIn(_rem_registryGetString) {
	UNUSEDALL
	trimStack(fun->stack);
	trimStack(fun->stack);
	fun->reg.setVariable(SVT_INT, 0);

	return BR_CONTINUE;
}

builtIn(quitWithFatalError) {
	UNUSEDALL
	Common::String mess = fun->stack->thisVar.getTextFromAnyVar();
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
	fun->reg.setVariable(SVT_INT, done ? 1 : 0);
	return BR_CONTINUE;
}

builtIn(doBackgroundEffect) {
	UNUSEDALL
	bool done = blurScreen();
	fun->reg.setVariable(SVT_INT, done ? 1 : 0);
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
		if (builtInFunctionArray[whichFunc].paramNum != -1) {
			if (builtInFunctionArray[whichFunc].paramNum != numParams) {
				Common::String buff = Common::String::format("Built in function must have %i parameter%s", builtInFunctionArray[whichFunc].paramNum, (builtInFunctionArray[whichFunc].paramNum == 1) ? "" : "s");
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
