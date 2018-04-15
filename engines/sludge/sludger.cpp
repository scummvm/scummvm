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
#include "sludge/graphics.h"
#include "sludge/imgloader.h"
#include "sludge/loadsave.h"
#include "sludge/language.h"
#include "sludge/moreio.h"
#include "sludge/newfatal.h"
#include "sludge/objtypes.h"
#include "sludge/people.h"
#include "sludge/region.h"
#include "sludge/statusba.h"
#include "sludge/sprites.h"
#include "sludge/sprbanks.h"
#include "sludge/sound.h"
#include "sludge/sludge.h"
#include "sludge/sludger.h"
#include "sludge/speech.h"
#include "sludge/transition.h"
#include "sludge/variable.h"
#include "sludge/version.h"
#include "sludge/zbuffer.h"

namespace Sludge {

extern int dialogValue;

int numBIFNames = 0;
Common::String *allBIFNames;
int numUserFunc = 0;
Common::String *allUserFunc = NULL;
int numResourceNames = 0;
Common::String *allResourceNames = NULL;
int selectedLanguage = 0;

int gameVersion;
FILETIME fileTime;

byte brightnessLevel = 255;

extern LoadedFunction *saverFunc;

LoadedFunction *allRunningFunctions = NULL;
VariableStack *noStack = NULL;
Variable *globalVars;

int numGlobals = 0;

extern Variable *launchResult;
extern int lastFramesPerSecond, thumbWidth, thumbHeight;

extern bool allowAnyFilename;
extern byte fadeMode;
extern uint16 saveEncoding;

const char *sludgeText[] = { "?????", "RETURN", "BRANCH", "BR_ZERO",
		"SET_GLOBAL", "SET_LOCAL", "LOAD_GLOBAL", "LOAD_LOCAL", "PLUS", "MINUS",
		"MULT", "DIVIDE", "AND", "OR", "EQUALS", "NOT_EQ", "MODULUS",
		"LOAD_VALUE", "LOAD_BUILT", "LOAD_FUNC", "CALLIT", "LOAD_STRING",
		"LOAD_FILE", "LOAD_OBJTYPE", "NOT", "LOAD_NULL", "STACK_PUSH",
		"LESSTHAN", "MORETHAN", "NEGATIVE", "U", "LESS_EQUAL", "MORE_EQUAL",
		"INC_LOCAL", "DEC_LOCAL", "INC_GLOBAL", "DEC_GLOBAL", "INDEXSET",
		"INDEXGET", "INC_INDEX", "DEC_INDEX", "QUICK_PUSH" };

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
	g_sludge->_languageMan->init();
	g_sludge->_gfxMan->init();
	g_sludge->_resMan->init();
	initPeople();
	initFloor();
	g_sludge->_objMan->init();
	g_sludge->_speechMan->init();
	initStatusBar();
	resetRandW();
	g_sludge->_evtMan->init();
	g_sludge->_txtMan->init();
	g_sludge->_cursorMan->init();

	g_sludge->_soundMan->init();
	if (!ConfMan.hasKey("mute") || !ConfMan.getBool("mute")) {
		g_sludge->_soundMan->initSoundStuff();
	}

	// global variables
	numGlobals = 0;
	launchResult = nullptr;

	lastFramesPerSecond = -1;
	thumbWidth = thumbHeight = 0;
	allowAnyFilename = true;
	noStack = nullptr;
	numBIFNames = numUserFunc = 0;
	allUserFunc = allBIFNames = nullptr;
	brightnessLevel = 255;
	fadeMode = 2;
	saveEncoding = false;
}

void killSludge() {
	killAllFunctions();
	killAllPeople();
	g_sludge->_regionMan->kill();
	setFloorNull();
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

	int a = 0;
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
			numResourceNames = fp->readUint16BE();
			debugC(2, kSludgeDebugDataLoad, "numResourceNames %i",
					numResourceNames);
			allResourceNames = new Common::String[numResourceNames];
			if (!checkNew(allResourceNames))
				return false;

			for (int fn = 0; fn < numResourceNames; fn++) {
				allResourceNames[fn].clear();
				allResourceNames[fn] = readString(fp);
				debugC(2, kSludgeDebugDataLoad, "Resource %i: %s", fn, allResourceNames[fn].c_str());
			}
		}
	}

	int winWidth = fp->readUint16BE();
	debugC(2, kSludgeDebugDataLoad, "winWidth : %i", winWidth);
	int winHeight = fp->readUint16BE();
	debugC(2, kSludgeDebugDataLoad, "winHeight : %i", winHeight);
	g_sludge->_gfxMan->setWindowSize(winWidth, winHeight);

	int specialSettings = fp->readByte();
	debugC(2, kSludgeDebugDataLoad, "specialSettings : %i", specialSettings);
	g_sludge->_timer.setDesiredfps(1000 / fp->readByte());

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
	for (a = 0; a < numGlobals; a++)
		initVarNew(globalVars[a]);

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
	drawPeople();// Then add any moving characters...
	g_sludge->_gfxMan->displaySpriteLayers();
}

void sludgeDisplay() {
	displayBase();
	g_sludge->_speechMan->display();
	drawStatusBar();
	g_sludge->_cursorMan->displayCursor();
	g_sludge->_gfxMan->display();
	if (brightnessLevel < 255) fixBrightness();// This is for transitionLevel special effects
}

void pauseFunction(LoadedFunction *fun) {
	LoadedFunction **huntAndDestroy = &allRunningFunctions;
	while (*huntAndDestroy) {
		if (fun == *huntAndDestroy) {
			(*huntAndDestroy) = (*huntAndDestroy)->next;
			fun->next = NULL;
		} else {
			huntAndDestroy = &(*huntAndDestroy)->next;
		}
	}
}

void restartFunction(LoadedFunction *fun) {
	fun->next = allRunningFunctions;
	allRunningFunctions = fun;
}

void killSpeechTimers() {
	LoadedFunction *thisFunction = allRunningFunctions;

	while (thisFunction) {
		if (thisFunction->freezerLevel == 0 && thisFunction->isSpeech
				&& thisFunction->timeLeft) {
			thisFunction->timeLeft = 0;
			thisFunction->isSpeech = false;
		}
		thisFunction = thisFunction->next;
	}

	g_sludge->_speechMan->kill();
}

void completeTimers() {
	LoadedFunction *thisFunction = allRunningFunctions;

	while (thisFunction) {
		if (thisFunction->freezerLevel == 0)
			thisFunction->timeLeft = 0;
		thisFunction = thisFunction->next;
	}
}

void finishFunction(LoadedFunction *fun) {
	int a;

	pauseFunction(fun);
	if (fun->stack)
		fatal(ERROR_NON_EMPTY_STACK);
	delete[] fun->compiledLines;
	for (a = 0; a < fun->numLocals; a++)
		unlinkVar(fun->localVars[a]);
	delete[] fun->localVars;
	unlinkVar(fun->reg);
	delete fun;
	fun = NULL;
}

void abortFunction(LoadedFunction *fun) {
	int a;

	pauseFunction(fun);
	while (fun->stack)
		trimStack(fun->stack);
	delete []fun->compiledLines;
	for (a = 0; a < fun->numLocals; a++)
		unlinkVar(fun->localVars[a]);
	delete []fun->localVars;
	unlinkVar(fun->reg);
	if (fun->calledBy)
		abortFunction(fun->calledBy);
	delete fun;
	fun = NULL;
}

int cancelAFunction(int funcNum, LoadedFunction *myself, bool &killedMyself) {
	int n = 0;
	killedMyself = false;

	LoadedFunction *fun = allRunningFunctions;
	while (fun) {
		if (fun->originalNumber == funcNum) {
			fun->cancelMe = true;
			n++;
			if (fun == myself)
				killedMyself = true;
		}
		fun = fun->next;
	}
	return n;
}

void freezeSubs() {
	LoadedFunction *thisFunction = allRunningFunctions;

	while (thisFunction) {
		if (thisFunction->unfreezable) {
			//msgBox ("SLUDGE debugging bollocks!", "Trying to freeze an unfreezable function!");
		} else {
			thisFunction->freezerLevel++;
		}
		thisFunction = thisFunction->next;
	}
}

void unfreezeSubs() {
	LoadedFunction *thisFunction = allRunningFunctions;

	while (thisFunction) {
		if (thisFunction->freezerLevel)
			thisFunction->freezerLevel--;
		thisFunction = thisFunction->next;
	}
}

bool continueFunction(LoadedFunction *fun) {
	bool keepLooping = true;
	bool advanceNow;
	uint param;
	sludgeCommand com;

	if (fun->cancelMe) {
		abortFunction(fun);
		return true;
	}

	while (keepLooping) {
		advanceNow = true;
		debugC(1, kSludgeDebugStackMachine, "Executing command line %i : ", fun->runThisLine);
		param = fun->compiledLines[fun->runThisLine].param;
		com = fun->compiledLines[fun->runThisLine].theCommand;

		if (numBIFNames) {
			setFatalInfo((fun->originalNumber < numUserFunc) ? allUserFunc[fun->originalNumber] : "Unknown user function", (com < numSludgeCommands) ? sludgeText[com] : ERROR_UNKNOWN_MCODE);
		}

		switch (com) {
		case SLU_RETURN:
			if (fun->calledBy) {
				LoadedFunction *returnTo = fun->calledBy;
				if (fun->returnSomething)
					copyVariable(fun->reg, returnTo->reg);
				finishFunction(fun);
				fun = returnTo;
				restartFunction(fun);
			} else {
				finishFunction(fun);
				advanceNow = false;   // So we don't do anything else with "fun"
				keepLooping = false;    // So we drop out of the loop
			}
			break;

		case SLU_CALLIT:
			switch (fun->reg.varType) {
			case SVT_FUNC:
				pauseFunction(fun);
				if (numBIFNames)
					setFatalInfo(
							(fun->originalNumber < numUserFunc) ?
									allUserFunc[fun->originalNumber] :
									"Unknown user function",
							(fun->reg.varData.intValue < numUserFunc) ?
									allUserFunc[fun->reg.varData.intValue] :
									"Unknown user function");

				if (!startNewFunctionNum(fun->reg.varData.intValue, param, fun,
						fun->stack))
					return false;
				fun = allRunningFunctions;
				advanceNow = false;   // So we don't do anything else with "fun"
				break;

			case SVT_BUILT: {
				debugC(1, kSludgeDebugStackMachine, "Built-in init value: %i",
						fun->reg.varData.intValue);
				BuiltReturn br = callBuiltIn(fun->reg.varData.intValue, param,
						fun);

				switch (br) {
				case BR_ERROR:
					return fatal(
							"Unknown error. This shouldn't happen. Please notify the SLUDGE developers.");

				case BR_PAUSE:
					pauseFunction(fun);
					// fall through

				case BR_KEEP_AND_PAUSE:
					keepLooping = false;
					break;

				case BR_ALREADY_GONE:
					keepLooping = false;
					advanceNow = false;
					break;

				case BR_CALLAFUNC: {
					int i = fun->reg.varData.intValue;
					setVariable(fun->reg, SVT_INT, 1);
					pauseFunction(fun);
					if (numBIFNames)
						setFatalInfo(
								(fun->originalNumber < numUserFunc) ?
										allUserFunc[fun->originalNumber] :
										"Unknown user function",
								(i < numUserFunc) ?
										allUserFunc[i] :
										"Unknown user function");
					if (!startNewFunctionNum(i, 0, fun, noStack, false))
						return false;
					fun = allRunningFunctions;
					advanceNow = false; // So we don't do anything else with "fun"
				}
					break;

				default:
					break;
				}
			}
				break;

			default:
				return fatal(ERROR_CALL_NONFUNCTION);
			}
			break;

			// These all grab things and shove 'em into the register

		case SLU_LOAD_NULL:
			setVariable(fun->reg, SVT_NULL, 0);
			break;

		case SLU_LOAD_FILE:
			setVariable(fun->reg, SVT_FILE, param);
			break;

		case SLU_LOAD_VALUE:
			setVariable(fun->reg, SVT_INT, param);
			break;

		case SLU_LOAD_LOCAL:
			if (!copyVariable(fun->localVars[param], fun->reg))
				return false;
			break;

		case SLU_AND:
			setVariable(fun->reg, SVT_INT,
					getBoolean(fun->reg) && getBoolean(fun->stack->thisVar));
			trimStack(fun->stack);
			break;

		case SLU_OR:
			setVariable(fun->reg, SVT_INT,
					getBoolean(fun->reg) || getBoolean(fun->stack->thisVar));
			trimStack(fun->stack);
			break;

		case SLU_LOAD_FUNC:
			setVariable(fun->reg, SVT_FUNC, param);
			break;

		case SLU_LOAD_BUILT:
			setVariable(fun->reg, SVT_BUILT, param);
			break;

		case SLU_LOAD_OBJTYPE:
			setVariable(fun->reg, SVT_OBJTYPE, param);
			break;

		case SLU_UNREG:
			if (dialogValue != 1)
				fatal(ERROR_HACKER);
			break;

		case SLU_LOAD_STRING:
			if (!loadStringToVar(fun->reg, param)) {
				return false;
			}
			break;

		case SLU_INDEXGET:
		case SLU_INCREMENT_INDEX:
		case SLU_DECREMENT_INDEX:
			switch (fun->stack->thisVar.varType) {
			case SVT_NULL:
				if (com == SLU_INDEXGET) {
					setVariable(fun->reg, SVT_NULL, 0);
					trimStack(fun->stack);
				} else {
					return fatal(ERROR_INCDEC_UNKNOWN);
				}
				break;

			case SVT_FASTARRAY:
			case SVT_STACK:
				if (fun->stack->thisVar.varData.theStack->first == NULL) {
					return fatal(ERROR_INDEX_EMPTY);
				} else {
					int ii;
					if (!getValueType(ii, SVT_INT, fun->reg))
						return false;
					Variable *grab =
							(fun->stack->thisVar.varType == SVT_FASTARRAY) ?
									fastArrayGetByIndex(
											fun->stack->thisVar.varData.fastArray,
											ii) :
									stackGetByIndex(
											fun->stack->thisVar.varData.theStack->first,
											ii);

					trimStack(fun->stack);

					if (!grab) {
						setVariable(fun->reg, SVT_NULL, 0);
					} else {
						int kk;
						switch (com) {
						case SLU_INCREMENT_INDEX:
							if (!getValueType(kk, SVT_INT, *grab))
								return false;
							setVariable(fun->reg, SVT_INT, kk);
							grab->varData.intValue = kk + 1;
							break;

						case SLU_DECREMENT_INDEX:
							if (!getValueType(kk, SVT_INT, *grab))
								return false;
							setVariable(fun->reg, SVT_INT, kk);
							grab->varData.intValue = kk - 1;
							break;

						default:
							if (!copyVariable(*grab, fun->reg))
								return false;
						}
					}
				}
				break;

			default:
				return fatal(ERROR_INDEX_NONSTACK);
			}
			break;

		case SLU_INDEXSET:
			switch (fun->stack->thisVar.varType) {
			case SVT_STACK:
				if (fun->stack->thisVar.varData.theStack->first == NULL) {
					return fatal(ERROR_INDEX_EMPTY);
				} else {
					int ii;
					if (!getValueType(ii, SVT_INT, fun->reg))
						return false;
					if (!stackSetByIndex(
							fun->stack->thisVar.varData.theStack->first, ii,
							fun->stack->next->thisVar)) {
						return false;
					}
					trimStack(fun->stack);
					trimStack(fun->stack);
				}
				break;

			case SVT_FASTARRAY: {
				int ii;
				if (!getValueType(ii, SVT_INT, fun->reg))
					return false;
				Variable *v = fastArrayGetByIndex(
						fun->stack->thisVar.varData.fastArray, ii);
				if (v == NULL)
					return fatal("Not within bounds of fast array.");
				if (!copyVariable(fun->stack->next->thisVar, *v))
					return false;
				trimStack(fun->stack);
				trimStack(fun->stack);
			}
				break;

			default:
				return fatal(ERROR_INDEX_NONSTACK);
			}
			break;

			// What can we do with the register? Well, we can copy it into a local
			// variable, a global or onto the stack...

		case SLU_INCREMENT_LOCAL: {
			int ii;
			if (!getValueType(ii, SVT_INT, fun->localVars[param]))
				return false;
			setVariable(fun->reg, SVT_INT, ii);
			setVariable(fun->localVars[param], SVT_INT, ii + 1);
		}
			break;

		case SLU_INCREMENT_GLOBAL: {
			int ii;
			if (!getValueType(ii, SVT_INT, globalVars[param]))
				return false;
			setVariable(fun->reg, SVT_INT, ii);
			setVariable(globalVars[param], SVT_INT, ii + 1);
		}
			break;

		case SLU_DECREMENT_LOCAL: {
			int ii;
			if (!getValueType(ii, SVT_INT, fun->localVars[param]))
				return false;
			setVariable(fun->reg, SVT_INT, ii);
			setVariable(fun->localVars[param], SVT_INT, ii - 1);
		}
			break;

		case SLU_DECREMENT_GLOBAL: {
			int ii;
			if (!getValueType(ii, SVT_INT, globalVars[param]))
				return false;
			setVariable(fun->reg, SVT_INT, ii);
			setVariable(globalVars[param], SVT_INT, ii - 1);
		}
			break;

		case SLU_SET_LOCAL:
			if (!copyVariable(fun->reg, fun->localVars[param]))
				return false;
			break;

		case SLU_SET_GLOBAL:
//			newDebug ("  Copying TO global variable", param);
//			newDebug ("  Global type at the moment", globalVars[param].varType);
			if (!copyVariable(fun->reg, globalVars[param]))
				return false;
//			newDebug ("  New type", globalVars[param].varType);
			break;

		case SLU_LOAD_GLOBAL:
//			newDebug ("  Copying FROM global variable", param);
//			newDebug ("  Global type at the moment", globalVars[param].varType);
			if (!copyVariable(globalVars[param], fun->reg))
				return false;
			break;

		case SLU_STACK_PUSH:
			if (!addVarToStack(fun->reg, fun->stack))
				return false;
			break;

		case SLU_QUICK_PUSH:
			if (!addVarToStackQuick(fun->reg, fun->stack))
				return false;
			break;

		case SLU_NOT:
			setVariable(fun->reg, SVT_INT, !getBoolean(fun->reg));
			break;

		case SLU_BR_ZERO:
			if (!getBoolean(fun->reg)) {
				advanceNow = false;
				fun->runThisLine = param;
			}
			break;

		case SLU_BRANCH:
			advanceNow = false;
			fun->runThisLine = param;
			break;

		case SLU_NEGATIVE: {
			int i;
			if (!getValueType(i, SVT_INT, fun->reg))
				return false;
			setVariable(fun->reg, SVT_INT, -i);
		}
			break;

			// All these things rely on there being somet' on the stack

		case SLU_MULT:
		case SLU_PLUS:
		case SLU_MINUS:
		case SLU_MODULUS:
		case SLU_DIVIDE:
		case SLU_EQUALS:
		case SLU_NOT_EQ:
		case SLU_LESSTHAN:
		case SLU_MORETHAN:
		case SLU_LESS_EQUAL:
		case SLU_MORE_EQUAL:
			if (fun->stack) {
				int firstValue, secondValue;

				switch (com) {
				case SLU_PLUS:
					addVariablesInSecond(fun->stack->thisVar, fun->reg);
					trimStack(fun->stack);
					break;

				case SLU_EQUALS:
					compareVariablesInSecond(fun->stack->thisVar, fun->reg);
					trimStack(fun->stack);
					break;

				case SLU_NOT_EQ:
					compareVariablesInSecond(fun->stack->thisVar, fun->reg);
					trimStack(fun->stack);
					fun->reg.varData.intValue = !fun->reg.varData.intValue;
					break;

				default:
					if (!getValueType(firstValue, SVT_INT, fun->stack->thisVar))
						return false;
					if (!getValueType(secondValue, SVT_INT, fun->reg))
						return false;
					trimStack(fun->stack);

					switch (com) {
					case SLU_MULT:
						setVariable(fun->reg, SVT_INT,
								firstValue * secondValue);
						break;

					case SLU_MINUS:
						setVariable(fun->reg, SVT_INT,
								firstValue - secondValue);
						break;

					case SLU_MODULUS:
						setVariable(fun->reg, SVT_INT,
								firstValue % secondValue);
						break;

					case SLU_DIVIDE:
						setVariable(fun->reg, SVT_INT,
								firstValue / secondValue);
						break;

					case SLU_LESSTHAN:
						setVariable(fun->reg, SVT_INT,
								firstValue < secondValue);
						break;

					case SLU_MORETHAN:
						setVariable(fun->reg, SVT_INT,
								firstValue > secondValue);
						break;

					case SLU_LESS_EQUAL:
						setVariable(fun->reg, SVT_INT,
								firstValue <= secondValue);
						break;

					case SLU_MORE_EQUAL:
						setVariable(fun->reg, SVT_INT,
								firstValue >= secondValue);
						break;

					default:
						break;
					}
				}
			} else {
				return fatal(ERROR_NOSTACK);
			}
			break;

		default:
			return fatal(ERROR_UNKNOWN_CODE);
		}

		if (advanceNow)
			fun->runThisLine++;

	}
	return true;
}

bool runSludge() {

	LoadedFunction *thisFunction = allRunningFunctions;
	LoadedFunction *nextFunction;

	while (thisFunction) {
		nextFunction = thisFunction->next;

		if (!thisFunction->freezerLevel) {
			if (thisFunction->timeLeft) {
				if (thisFunction->timeLeft < 0) {
					if (!g_sludge->_soundMan->stillPlayingSound(
							g_sludge->_speechMan->getLastSpeechSound())) {
						thisFunction->timeLeft = 0;
					}
				} else if (!--(thisFunction->timeLeft)) {
				}
			} else {
				if (thisFunction->isSpeech) {
					thisFunction->isSpeech = false;
					g_sludge->_speechMan->kill();
				}
				if (!continueFunction(thisFunction))
					return false;
			}
		}

		thisFunction = nextFunction;
	}

	if (!g_sludge->loadNow.empty()) {
		if (g_sludge->loadNow[0] == ':') {
			saveGame(g_sludge->loadNow.c_str() + 1);
			setVariable(saverFunc->reg, SVT_INT, 1);
		} else {
			if (!loadGame(g_sludge->loadNow))
				return false;
		}
		g_sludge->loadNow.clear();
	}

	return true;
}

void killAllFunctions() {
	while (allRunningFunctions)
		finishFunction(allRunningFunctions);
}

bool loadFunctionCode(LoadedFunction *newFunc) {
	uint numLines, numLinesRead;

	if (!g_sludge->_resMan->openSubSlice(newFunc->originalNumber))
		return false;

	debugC(3, kSludgeDebugDataLoad, "Load function code");

	Common::SeekableReadStream *readStream = g_sludge->_resMan->getData();
	newFunc->unfreezable = readStream->readByte();
	numLines = readStream->readUint16BE();
	debugC(3, kSludgeDebugDataLoad, "numLines: %i", numLines);
	newFunc->numArgs = readStream->readUint16BE();
	debugC(3, kSludgeDebugDataLoad, "numArgs: %i", newFunc->numArgs);
	newFunc->numLocals = readStream->readUint16BE();
	debugC(3, kSludgeDebugDataLoad, "numLocals: %i", newFunc->numLocals);
	newFunc->compiledLines = new LineOfCode[numLines];
	if (!checkNew(newFunc->compiledLines))
		return false;

	for (numLinesRead = 0; numLinesRead < numLines; numLinesRead++) {
		newFunc->compiledLines[numLinesRead].theCommand = (sludgeCommand)readStream->readByte();
		newFunc->compiledLines[numLinesRead].param = readStream->readUint16BE();
		debugC(3, kSludgeDebugDataLoad, "command line %i: %i", numLinesRead,
				newFunc->compiledLines[numLinesRead].theCommand);
	}
	g_sludge->_resMan->finishAccess();

	// Now we need to reserve memory for the local variables
	newFunc->localVars = new Variable[newFunc->numLocals];
	if (!checkNew(newFunc->localVars))
		return false;
	for (int a = 0; a < newFunc->numLocals; a++) {
		initVarNew(newFunc->localVars[a]);
	}

	return true;
}

int startNewFunctionNum(uint funcNum, uint numParamsExpected,
		LoadedFunction *calledBy, VariableStack *&vStack, bool returnSommet) {
	LoadedFunction *newFunc = new LoadedFunction;
	checkNew(newFunc);
	newFunc->originalNumber = funcNum;

	loadFunctionCode(newFunc);

	if (newFunc->numArgs != (int) numParamsExpected)
		return fatal("Wrong number of parameters!");
	if (newFunc->numArgs > newFunc->numLocals)
		return fatal("More arguments than local Variable space!");

	// Now, lets copy the parameters from the calling function's stack...

	while (numParamsExpected) {
		numParamsExpected--;
		if (vStack == NULL)
			return fatal(
					"Corrupted file!The stack's empty and there were still parameters expected");
		copyVariable(vStack->thisVar, newFunc->localVars[numParamsExpected]);
		trimStack(vStack);
	}

	newFunc->cancelMe = false;
	newFunc->timeLeft = 0;
	newFunc->returnSomething = returnSommet;
	newFunc->calledBy = calledBy;
	newFunc->stack = NULL;
	newFunc->freezerLevel = 0;
	newFunc->runThisLine = 0;
	newFunc->isSpeech = 0;
	initVarNew(newFunc->reg);

	restartFunction(newFunc);
	return 1;
}

int lastFramesPerSecond = -1;
int thisFramesPerSecond = -1;

} // End of namespace Sludge
