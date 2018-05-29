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
#ifndef SLUDGE_FUNCTION_H
#define SLUDGE_FUNCTION_H

#include "sludge/allfiles.h"
#include "sludge/csludge.h"
#include "sludge/variable.h"

namespace Sludge {

struct Variable;
struct VariableStack;

struct LineOfCode {
	SludgeCommand theCommand;
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

bool runAllFunctions();
int startNewFunctionNum(uint, uint, LoadedFunction *, VariableStack*&, bool = true);
void restartFunction(LoadedFunction *fun);
bool loadFunctionCode(LoadedFunction *newFunc);
void killAllFunctions();

void finishFunction(LoadedFunction *fun);
void abortFunction(LoadedFunction *fun);

void freezeSubs();
void unfreezeSubs();
void completeTimers();
void killSpeechTimers();
int cancelAFunction(int funcNum, LoadedFunction *myself, bool &killedMyself);

LoadedFunction *loadFunction(Common::SeekableReadStream *stream);
void saveFunction(LoadedFunction *fun, Common::WriteStream *stream);

} // End of namespace Sludge

#endif
