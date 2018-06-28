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

	Variable *fastArrayGetByIndex(uint theIndex);
};

struct StackHandler {
	struct VariableStack *first;
	struct VariableStack *last;
	int timesUsed;

	int getStackSize() const;
	bool getSavedGamesStack(const Common::String &ext);
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

	Variable() {
		varType = SVT_NULL;
		varData.intValue = 0;
	}

	void unlinkVar();
	void setVariable(VariableType vT, int value);

	// Copy from another variable
	bool copyFrom(const Variable &from);
	bool copyMain(const Variable &from); // without variable unlink

	// Load & save
	bool save(Common::WriteStream *stream);
	bool load(Common::SeekableReadStream *stream);

	// Text variable
	void makeTextVar(const Common::String &txt);
	bool loadStringToVar(int value);

	// Animation variable
	void makeAnimationVariable(PersonaAnimation *i);
	struct PersonaAnimation *getAnimationFromVar();

	// Custome variable
	void makeCostumeVariable(Persona *i);
	struct Persona *getCostumeFromVar();

	// Fast array variable
	bool makeFastArrayFromStack(const StackHandler *stacky);
	bool makeFastArraySize(int size);

	// Stack variable
	bool copyStack(const Variable &from);

	// Add variables
	void addVariablesInSecond(const Variable &other);
	void compareVariablesInSecond(const Variable &other);
	int compareVars(const Variable &other) const;

	// General getters
	Common::String getTextFromAnyVar() const;
	bool getBoolean() const;
	bool getValueType(int &toHere, VariableType vT) const;
};

struct VariableStack {
	Variable thisVar;
	VariableStack *next;

	// Variable getter & setter
	bool stackSetByIndex(uint, const Variable &);
	Variable *stackGetByIndex(uint);

	// Find last
	VariableStack *stackFindLast();
};

// Stacky stuff

bool addVarToStack(const Variable &va, VariableStack *&thisStack);
bool addVarToStackQuick(Variable &va, VariableStack *&thisStack);
void trimStack(VariableStack *&stack);
int deleteVarFromStack(const Variable &va, VariableStack *&thisStack, bool allOfEm = false);

// load & save
void saveStack(VariableStack *vs, Common::WriteStream *stream);
VariableStack *loadStack(Common::SeekableReadStream *stream, VariableStack **last);
bool saveStackRef(StackHandler *vs, Common::WriteStream *stream);
StackHandler *loadStackRef(Common::SeekableReadStream *stream);
void clearStackLib();

} // End of namespace Sludge

#endif

