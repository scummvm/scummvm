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
#ifndef SLUDGER_H
#define SLUDGER_H

#include <stdint.h>

#include "allfiles.h"
#include "variable.h"
#include "csludge.h"
#include "language.h"

#include "common/file.h"

namespace Sludge {

#ifndef _WIN32
typedef struct _FILETIME {
	uint32_t dwLowDateTime;
	uint32_t dwHighDateTime;
} FILETIME;
#endif

struct variable;
struct variableStack;

struct eventHandlers {
	int leftMouseFunction;
	int leftMouseUpFunction;
	int rightMouseFunction;
	int rightMouseUpFunction;
	int moveMouseFunction;
	int focusFunction;
	int spaceFunction;
};

struct lineOfCode {
	sludgeCommand theCommand;
	int32_t param;
};

struct loadedFunction {
	int originalNumber;
	lineOfCode *compiledLines;
	int numLocals, timeLeft, numArgs;
	variable *localVars;
	variableStack *stack;
	variable reg;
	unsigned int runThisLine;
	loadedFunction *calledBy;
	loadedFunction *next;
	bool returnSomething, isSpeech, unfreezable, cancelMe;
	unsigned char freezerLevel;
};

struct inputType {
	bool leftClick, rightClick, justMoved, leftRelease, rightRelease;
	int mouseX, mouseY, keyPressed;
};

extern unsigned char *gameIcon;
extern int iconW, iconH;

bool initSludge(char *);
void sludgeDisplay();
int startNewFunctionNum(unsigned int, unsigned int, loadedFunction *,
		variableStack*&, bool = true);
bool handleInput();
void restartFunction(loadedFunction *fun);
bool loadFunctionCode(loadedFunction *newFunc);
void loadHandlers(Common::SeekableReadStream *stream);
void saveHandlers(Common::WriteStream *stream);

void finishFunction(loadedFunction *fun);
void abortFunction(loadedFunction *fun);
Common::File *openAndVerify(char *filename, char extra1, char extra2,
		const char *er, int &fileVersion);

void freezeSubs();
void unfreezeSubs();
void completeTimers();
void killSpeechTimers();
int cancelAFunction(int funcNum, loadedFunction *myself, bool &killedMyself);

} // End of namespace Sludge

#endif
