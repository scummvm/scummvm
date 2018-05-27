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

#include "common/file.h"

#include "sludge/allfiles.h"
#include "sludge/variable.h"
#include "sludge/csludge.h"
#include "sludge/language.h"

namespace Sludge {

typedef struct _FILETIME {
	uint32 dwLowDateTime;
	uint32 dwHighDateTime;
} FILETIME;

struct Variable;
struct VariableStack;

struct LineOfCode {
	sludgeCommand theCommand;
	int32 param;
};

struct LoadedFunction {
	int originalNumber;
	LineOfCode *compiledLines;
	int numLocals, timeLeft, numArgs;
	Variable *localVars;
	VariableStack *stack;
	Variable reg;
	uint runThisLine;
	LoadedFunction *calledBy;
	LoadedFunction *next;
	bool returnSomething, isSpeech, unfreezable, cancelMe;
	byte freezerLevel;
};

bool initSludge(const Common::String &);
bool runSludge();

void initSludge();
void killSludge();

void displayBase();
void sludgeDisplay();
int startNewFunctionNum(uint, uint, LoadedFunction *, VariableStack*&, bool = true);
void restartFunction(LoadedFunction *fun);
bool loadFunctionCode(LoadedFunction *newFunc);
void killAllFunctions();

void finishFunction(LoadedFunction *fun);
void abortFunction(LoadedFunction *fun);
Common::File *openAndVerify(const Common::String &filename, char extra1, char extra2, const char *er, int &fileVersion);

void freezeSubs();
void unfreezeSubs();
void completeTimers();
void killSpeechTimers();
int cancelAFunction(int funcNum, LoadedFunction *myself, bool &killedMyself);

} // End of namespace Sludge

#endif
