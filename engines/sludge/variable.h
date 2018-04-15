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
#ifndef SLUDGE_VARIABLE_H
#define SLUDGE_VARIABLE_H

namespace Sludge {

struct Persona;
struct PersonaAnimation;
struct Variable;
struct VariableStack;

enum VariableType {
	SVT_NULL,
	SVT_INT,
	SVT_FUNC,
	SVT_STRING,
	SVT_BUILT,
	SVT_FILE,
	SVT_STACK,
	SVT_OBJTYPE,
	SVT_ANIM,
	SVT_COSTUME,
	SVT_FASTARRAY,
	SVT_NUM_TYPES
};

struct FastArrayHandler {
	struct Variable *fastVariables;
	int size;
	int timesUsed;
};

struct StackHandler {
	struct VariableStack *first;
	struct VariableStack *last;
	int timesUsed;
};

union VariableData {
	signed int intValue;
	const char *theString;
	StackHandler *theStack;
	PersonaAnimation *animHandler;
	Persona *costumeHandler;
	FastArrayHandler *fastArray;
};

struct Variable {
	VariableType varType;
	VariableData varData;
};

struct VariableStack {
	Variable thisVar;
	VariableStack *next;
};

// Initialisation

#define initVarNew(thisVar)     thisVar.varType = SVT_NULL

// Setting variables

void setVariable(Variable &thisVar, VariableType vT, int value);
bool copyVariable(const Variable &from, Variable &to);
bool loadStringToVar(Variable &thisVar, int value);
void newAnimationVariable(Variable &thisVar, struct PersonaAnimation  *i);
void newCostumeVariable(Variable &thisVar, struct Persona *i);
void makeTextVar(Variable &thisVar, const Common::String &txt);
void addVariablesInSecond(Variable &var1, Variable &var2);
void compareVariablesInSecond(const Variable &var1, Variable &var2);
char *createCString(const Common::String &s);

// Misc.

void unlinkVar(Variable &thisVar);
Common::String getNumberedString(int value);
Common::String getTextFromAnyVar(const Variable &from);
struct Persona *getCostumeFromVar(Variable &thisVar);
struct PersonaAnimation  *getAnimationFromVar(Variable &thisVar);
bool getBoolean(const Variable &from);
bool getValueType(int &toHere, VariableType vT, const Variable &v);

// Stacky stuff

bool addVarToStack(const Variable &va, VariableStack *&thisStack);
bool addVarToStackQuick(Variable &va, VariableStack *&thisStack);
void trimStack(VariableStack *&stack);
int deleteVarFromStack(const Variable &va, VariableStack *&thisStack,
		bool allOfEm = false);
VariableStack *stackFindLast(VariableStack *hunt);
bool copyStack(const Variable &from, Variable &to);
int stackSize(const StackHandler *me);
bool stackSetByIndex(VariableStack *, uint, const Variable &);
Variable *stackGetByIndex(VariableStack *, uint);
bool getSavedGamesStack(StackHandler *sH, const Common::String &ext);

bool makeFastArrayFromStack(Variable &to, const StackHandler *stacky);
bool makeFastArraySize(Variable &to, int size);
Variable *fastArrayGetByIndex(FastArrayHandler *vS, uint theIndex);

} // End of namespace Sludge

#endif

