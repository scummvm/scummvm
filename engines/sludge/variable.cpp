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

#include "common/debug.h"
#include "common/savefile.h"

#include "sludge/allfiles.h"
#include "sludge/fileset.h"
#include "sludge/moreio.h"
#include "sludge/newfatal.h"
#include "sludge/objtypes.h"
#include "sludge/people.h"
#include "sludge/sludge.h"
#include "sludge/variable.h"

namespace Sludge {

const char *typeName[] = { "undefined", "number", "user function", "string",
		"built-in function", "file", "stack", "object type", "animation",
		"costume", "fast array" };

void Variable::unlinkVar() {
	switch (varType) {
		case SVT_STRING:
			delete []varData.theString;
			varData.theString = NULL;
			break;

		case SVT_STACK:
			varData.theStack->timesUsed--;
			if (varData.theStack->timesUsed <= 0) {
				while (varData.theStack->first)
					trimStack(varData.theStack->first);
				delete varData.theStack;
				varData.theStack = NULL;
			}
			break;

		case SVT_FASTARRAY:
			varData.fastArray->timesUsed--;
			if (varData.theStack->timesUsed <= 0) {
				delete varData.fastArray->fastVariables;
				delete[] varData.fastArray;
				varData.fastArray = NULL;
			}
			break;

		case SVT_ANIM:
			if (varData.animHandler) {
				delete varData.animHandler;
				varData.animHandler = nullptr;
			}
			break;

		default:
			break;
	}
}

void Variable::setVariable(VariableType vT, int value) {
	unlinkVar();
	varType = vT;
	varData.intValue = value;
}

void Variable::makeAnimationVariable(PersonaAnimation  *i) {
	unlinkVar();
	varType = SVT_ANIM;
	varData.animHandler = i;
}

PersonaAnimation *Variable::getAnimationFromVar() {
	if (varType == SVT_ANIM)
		return new PersonaAnimation(varData.animHandler);

	if (varType == SVT_INT && varData.intValue == 0)
		return new PersonaAnimation();

	fatal("Expecting an animation variable; found Variable of type", typeName[varType]);
	return NULL;
}

void Variable::makeCostumeVariable(Persona *i) {
	unlinkVar();
	varType = SVT_COSTUME;
	varData.costumeHandler = i;
}

Persona *Variable::getCostumeFromVar() {
	Persona *p = NULL;

	switch (varType) {
		case SVT_ANIM:
			p = new Persona;
			if (!checkNew(p))
				return NULL;
			p->numDirections = 1;
			p->animation = new PersonaAnimation  *[3];
			if (!checkNew(p->animation))
				return NULL;

			for (int iii = 0; iii < 3; iii++)
				p->animation[iii] = new PersonaAnimation(varData.animHandler);

			break;

		case SVT_COSTUME:
			return varData.costumeHandler;
			break;

		default:
			fatal("Expecting an animation variable; found Variable of type", typeName[varType]);
	}

	return p;
}

int StackHandler::getStackSize() const {
	int r = 0;
	VariableStack *a = first;
	while (a) {
		r++;
		a = a->next;
	}
	return r;
}

bool StackHandler::getSavedGamesStack(const Common::String &ext) {
	// Make pattern
	uint len = ext.size();
	Common::String pattern = "*";
	pattern += ext;

	// Get all saved files
	Common::StringArray sa = g_system->getSavefileManager()->listSavefiles(pattern);

	// Save file names to stacks
	Variable newName;
	newName.varType = SVT_NULL;
	Common::StringArray::iterator it;
	for (it = sa.begin(); it != sa.end(); ++it) {
		(*it).erase((*it).size() - len, len);
		newName.makeTextVar((*it));
		if (!addVarToStack(newName, first))
			return false;
		if (last == NULL)
			last = first;
	}

	return true;
}

bool Variable::copyStack(const Variable &from) {
	varType = SVT_STACK;
	varData.theStack = new StackHandler;
	if (!checkNew(varData.theStack))
		return false;
	varData.theStack->first = NULL;
	varData.theStack->last = NULL;
	varData.theStack->timesUsed = 1;
	VariableStack *a = from.varData.theStack->first;

	while (a) {
		addVarToStack(a->thisVar, varData.theStack->first);
		if (varData.theStack->last == NULL) {
			varData.theStack->last = varData.theStack->first;
		}
		a = a->next;
	}

	return true;
}

void Variable::addVariablesInSecond(const Variable &other) {
	if (other.varType == SVT_INT && varType == SVT_INT) {
		varData.intValue += other.varData.intValue;
	} else {
		Common::String string1 = other.getTextFromAnyVar();
		Common::String string2 = getTextFromAnyVar();

		unlinkVar();
		varData.theString = createCString(string1 + string2);
		varType = SVT_STRING;
	}
}

int Variable::compareVars(const Variable &other) const {
	int re = 0;
	if (other.varType == varType) {
		switch (other.varType) {
			case SVT_NULL:
				re = 1;
				break;

			case SVT_COSTUME:
				re = (other.varData.costumeHandler == varData.costumeHandler);
				break;

			case SVT_ANIM:
				re = (other.varData.animHandler == varData.animHandler);
				break;

			case SVT_STRING:
				re = (strcmp(other.varData.theString, varData.theString) == 0);
				break;

			case SVT_STACK:
				re = (other.varData.theStack == varData.theStack);
				break;

			default:
				re = (other.varData.intValue == varData.intValue);
		}
	}
	return re;
}

void Variable::compareVariablesInSecond(const Variable &other) {
	setVariable(SVT_INT, compareVars(other));
}

void Variable::makeTextVar(const Common::String &txt) {
	unlinkVar();
	varType = SVT_STRING;
	varData.theString = createCString(txt);
}

bool Variable::loadStringToVar(int value) {
	makeTextVar(g_sludge->_resMan->getNumberedString(value));
	return (bool)(varData.theString != NULL);
}

Common::String Variable::getTextFromAnyVar() const {
	switch (varType) {
		case SVT_STRING:
			return varData.theString;

		case SVT_FASTARRAY: {
			Common::String builder = "FAST:";
			Common::String builder2 = "";
			Common::String grabText = "";

			for (int i = 0; i < varData.fastArray->size; i++) {
				builder2 = builder + " ";
				grabText = varData.fastArray->fastVariables[i].getTextFromAnyVar();
				builder.clear();
				builder = builder2 + grabText;
			}
			return builder;
		}

		case SVT_STACK: {
			Common::String builder = "ARRAY:";
			Common::String builder2 = "";
			Common::String grabText = "";

			VariableStack *stacky = varData.theStack->first;

			while (stacky) {
				builder2 = builder + " ";
				grabText = stacky->thisVar.getTextFromAnyVar();
				builder.clear();
				builder = builder2 + grabText;
				stacky = stacky->next;
			}
			return builder;
		}

		case SVT_INT: {
			Common::String buff = Common::String::format("%i", varData.intValue);
			return buff;
		}

		case SVT_FILE: {
			return g_sludge->_resMan->resourceNameFromNum(varData.intValue);
		}

		case SVT_OBJTYPE: {
			ObjectType *thisType = g_sludge->_objMan->findObjectType(varData.intValue);
			if (thisType)
				return thisType->screenName;
			break;
		}

		default:
			break;
	}

	return typeName[varType];
}

bool Variable::getBoolean() const {
	switch (varType) {
		case SVT_NULL:
			return false;

		case SVT_INT:
			return (bool)(varData.intValue != 0);

		case SVT_STACK:
			return (bool)(varData.theStack->first != NULL);

		case SVT_STRING:
			return (bool)(varData.theString[0] != 0);

		case SVT_FASTARRAY:
			return (bool)(varData.fastArray->size != 0);

		default:
			break;
	}
	return true;
}

bool Variable::copyMain(const Variable &from) {
	varType = from.varType;
	switch (varType) {
		case SVT_INT:
		case SVT_FUNC:
		case SVT_BUILT:
		case SVT_FILE:
		case SVT_OBJTYPE:
			varData.intValue = from.varData.intValue;
			return true;

		case SVT_FASTARRAY:
			varData.fastArray = from.varData.fastArray;
			varData.fastArray->timesUsed++;
			return true;

		case SVT_STRING:
			varData.theString = createCString(from.varData.theString);
			return varData.theString ? true : false;

		case SVT_STACK:
			varData.theStack = from.varData.theStack;
			varData.theStack->timesUsed++;
			return true;

		case SVT_COSTUME:
			varData.costumeHandler = from.varData.costumeHandler;
			return true;

		case SVT_ANIM:
			varData.animHandler = new PersonaAnimation(from.varData.animHandler);
			return true;

		case SVT_NULL:
			return true;

		default:
			break;
	}
	fatal("Unknown value type");
	return false;
}

bool Variable::copyFrom(const Variable &from) {
	unlinkVar();
	return copyMain(from);
}

Variable *FastArrayHandler::fastArrayGetByIndex(uint theIndex) {
	if ((int)theIndex >= size)
		return NULL;
	return &fastVariables[theIndex];
}

bool Variable::makeFastArraySize(int size) {
	if (size < 0)
		return fatal("Can't create a fast array with a negative number of elements!");
	unlinkVar();
	varType = SVT_FASTARRAY;
	varData.fastArray = new FastArrayHandler;
	if (!checkNew(varData.fastArray))
		return false;
	varData.fastArray->fastVariables = new Variable[size];
	if (!checkNew(varData.fastArray->fastVariables))
		return false;
	varData.fastArray->size = size;
	varData.fastArray->timesUsed = 1;
	return true;
}

bool Variable::makeFastArrayFromStack(const StackHandler *stacky) {
	int size = stacky->getStackSize();
	if (!makeFastArraySize(size))
		return false;

	// Now let's fill up the new array

	VariableStack *allV = stacky->first;
	size = 0;
	while (allV) {
		varData.fastArray->fastVariables[size].copyMain(allV->thisVar);
		size++;
		allV = allV->next;
	}
	return true;
}

bool addVarToStack(const Variable &va, VariableStack *&thisStack) {
	VariableStack *newStack = new VariableStack;
	if (!checkNew(newStack))
		return false;

	if (!newStack->thisVar.copyMain(va))
		return false;
	newStack->next = thisStack;
	thisStack = newStack;
	//debugC(2, kSludgeDebugStackMachine, "Variable %s was added to stack", getTextFromAnyVar(va));
	return true;
}

bool addVarToStackQuick(Variable &va, VariableStack *&thisStack) {
	VariableStack *newStack = new VariableStack;
	if (!checkNew(newStack))
		return false;

//	if (! copyMain (va, newStack -> thisVar)) return false;

	memcpy(&(newStack->thisVar), &va, sizeof(Variable));
	va.varType = SVT_NULL;

	newStack->next = thisStack;
	thisStack = newStack;
	//debugC(2, kSludgeDebugStackMachine, "Variable %s was added to stack quick", getTextFromAnyVar(va));
	return true;
}

bool VariableStack::stackSetByIndex(uint theIndex, const Variable &va) {
	VariableStack *vS = this;
	while (theIndex--) {
		vS = vS->next;
		if (!vS)
			return fatal("Index past end of stack.");
	}
	return vS->thisVar.copyFrom(va);
}

Variable *VariableStack::stackGetByIndex(uint theIndex) {
	VariableStack *vS = this;
	while (theIndex--) {
		vS = vS->next;
		if (!vS) {
			return NULL;
		}
	}
	return &(vS->thisVar);
}

int deleteVarFromStack(const Variable &va, VariableStack *&thisStack, bool allOfEm) {
	VariableStack **huntVar = &thisStack;
	VariableStack *killMe;
	int reply = 0;

	while (*huntVar) {
		if (va.compareVars((*huntVar)->thisVar)) {
			killMe = *huntVar;
			*huntVar = killMe->next;
			killMe->thisVar.unlinkVar();
			delete killMe;
			if (!allOfEm)
				return 1;
			reply++;
		} else {
			huntVar = &((*huntVar)->next);
		}
	}

	return reply;
}

// Would be a LOT better just to keep this up to date in the above function... ah well
VariableStack *VariableStack::stackFindLast() {
	VariableStack *hunt = this;
	while (hunt->next)
		hunt = hunt->next;

	return hunt;
}

bool Variable::getValueType(int &toHere, VariableType vT) const {
	if (varType != vT) {
		Common::String e1 = "Can only perform specified operation on a value which is of type ";
		e1 += typeName[vT];
		Common::String e2 = "... value supplied was of type ";
		e2 += typeName[varType];
		fatal(e1, e2);

		return false;
	}
	toHere = varData.intValue;
	return true;
}

void trimStack(VariableStack *&stack) {
	VariableStack *killMe = stack;
	stack = stack->next;

	//debugC(2, kSludgeDebugStackMachine, "Variable %s was removed from stack", getTextFromAnyVar(killMe->thisVar));

	// When calling this, we've ALWAYS checked that stack != NULL
	killMe->thisVar.unlinkVar();
	delete killMe;
}

//----------------------------------------------------------------------
// Globals (so we know what's saved already and what's a reference
//----------------------------------------------------------------------

struct stackLibrary {
	StackHandler *stack;
	stackLibrary *next;
};

int stackLibTotal = 0;
stackLibrary *stackLib = NULL;

//----------------------------------------------------------------------
// For saving and loading stacks...
//----------------------------------------------------------------------
void saveStack(VariableStack *vs, Common::WriteStream *stream) {
	int elements = 0;
	int a;

	VariableStack *search = vs;
	while (search) {
		elements++;
		search = search->next;
	}

	stream->writeUint16BE(elements);
	search = vs;
	for (a = 0; a < elements; a++) {
		search->thisVar.save(stream);
		search = search->next;
	}
}

VariableStack *loadStack(Common::SeekableReadStream *stream, VariableStack **last) {
	int elements = stream->readUint16BE();
	int a;
	VariableStack *first = NULL;
	VariableStack **changeMe = &first;

	for (a = 0; a < elements; a++) {
		VariableStack *nS = new VariableStack;
		if (!checkNew(nS))
			return NULL;
		nS->thisVar.load(stream);
		if (last && a == elements - 1) {
			*last = nS;
		}
		nS->next = NULL;
		(*changeMe) = nS;
		changeMe = &(nS->next);
	}

	return first;
}

bool saveStackRef(StackHandler *vs, Common::WriteStream *stream) {
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

StackHandler *getStackFromLibrary(int n) {
	n = stackLibTotal - n;
	while (n) {
		stackLib = stackLib->next;
		n--;
	}
	return stackLib->stack;
}

StackHandler *loadStackRef(Common::SeekableReadStream *stream) {
	StackHandler *nsh;

	if (stream->readByte()) {    // It's one we've loaded already...
		nsh = getStackFromLibrary(stream->readUint16BE());
		nsh->timesUsed++;
	} else {
		// Load the new stack

		nsh = new StackHandler;
		if (!checkNew(nsh))
			return NULL;
		nsh->last = NULL;
		nsh->first = loadStack(stream, &nsh->last);
		nsh->timesUsed = 1;

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
bool Variable::save(Common::WriteStream *stream) {
	stream->writeByte(varType);
	switch (varType) {
		case SVT_INT:
		case SVT_FUNC:
		case SVT_BUILT:
		case SVT_FILE:
		case SVT_OBJTYPE:
			stream->writeUint32LE(varData.intValue);
			return true;

		case SVT_STRING:
			writeString(varData.theString, stream);
			return true;

		case SVT_STACK:
			return saveStackRef(varData.theStack, stream);

		case SVT_COSTUME:
			varData.costumeHandler->save(stream);
			return false;

		case SVT_ANIM:
			varData.animHandler->save(stream);
			return false;

		case SVT_NULL:
			return false;

		default:
			fatal("Can't save variables of this type:", (varType < SVT_NUM_TYPES - 1) ? typeName[varType] : "bad ID");
	}
	return true;
}

bool Variable::load(Common::SeekableReadStream *stream) {
	varType = (VariableType)stream->readByte();
	switch (varType) {
		case SVT_INT:
		case SVT_FUNC:
		case SVT_BUILT:
		case SVT_FILE:
		case SVT_OBJTYPE:
			varData.intValue = stream->readUint32LE();
			return true;

		case SVT_STRING:
			varData.theString = createCString(readString(stream));
			return true;

		case SVT_STACK:
			varData.theStack = loadStackRef(stream);
			return true;

		case SVT_COSTUME:
			varData.costumeHandler = new Persona;
			if (!checkNew(varData.costumeHandler))
				return false;
			varData.costumeHandler->load(stream);
			return true;

		case SVT_ANIM:
			varData.animHandler = new PersonaAnimation;
			if (!checkNew(varData.animHandler))
				return false;
			varData.animHandler->load(stream);
			return true;

		default:
			break;
	}
	return true;
}

} // End of namespace Sludge
