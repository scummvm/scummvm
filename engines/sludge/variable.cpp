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

#include "sludge/debug.h"
#include "sludge/allfiles.h"
#include "sludge/variable.h"
#include "sludge/moreio.h"
#include "sludge/newfatal.h"
#include "sludge/stringy.h"
#include "sludge/objtypes.h"
#include "sludge/people.h"
#include "sludge/fileset.h"
#include "sludge/sludge.h"

namespace Sludge {

const char *typeName[] = { "undefined", "number", "user function", "string",
		"built-in function", "file", "stack", "object type", "animation",
		"costume" };

extern char *outputDir;

void unlinkVar(variable &thisVar) {
	switch (thisVar.varType) {
		case SVT_STRING:
			delete[] thisVar.varData.theString;
			thisVar.varData.theString = NULL;
			break;

		case SVT_STACK:
			thisVar.varData.theStack->timesUsed--;
			if (thisVar.varData.theStack->timesUsed <= 0) {
				while (thisVar.varData.theStack->first)
					trimStack(thisVar.varData.theStack->first);
				delete thisVar.varData.theStack;
				thisVar.varData.theStack = NULL;
			}
			break;

		case SVT_FASTARRAY:
			thisVar.varData.fastArray->timesUsed--;
			if (thisVar.varData.theStack->timesUsed <= 0) {
				delete thisVar.varData.fastArray->fastVariables;
				delete[] thisVar.varData.fastArray;
				thisVar.varData.fastArray = NULL;
			}
			break;

		case SVT_ANIM:
			deleteAnim(thisVar.varData.animHandler);
			break;

		default:
			break;
	}
}

void setVariable(variable &thisVar, variableType vT, int value) {
	unlinkVar(thisVar);
	thisVar.varType = vT;
	thisVar.varData.intValue = value;
}

void newAnimationVariable(variable &thisVar, personaAnimation *i) {
	unlinkVar(thisVar);
	thisVar.varType = SVT_ANIM;
	thisVar.varData.animHandler = i;
}

personaAnimation *getAnimationFromVar(variable &thisVar) {
	if (thisVar.varType == SVT_ANIM)
		return copyAnim(thisVar.varData.animHandler);

	if (thisVar.varType == SVT_INT && thisVar.varData.intValue == 0)
		return makeNullAnim();

	fatal("Expecting an animation variable; found variable of type", typeName[thisVar.varType]);
	return NULL;
}

void newCostumeVariable(variable &thisVar, persona *i) {
	unlinkVar(thisVar);
	thisVar.varType = SVT_COSTUME;
	thisVar.varData.costumeHandler = i;
}

persona *getCostumeFromVar(variable &thisVar) {
	persona *p = NULL;

	switch (thisVar.varType) {
		case SVT_ANIM:
			p = new persona;
			if (!checkNew(p))
				return NULL;
			p->numDirections = 1;
			p->animation = new personaAnimation *[3];
			if (!checkNew(p->animation))
				return NULL;

			for (int iii = 0; iii < 3; iii++)
				p->animation[iii] = copyAnim(thisVar.varData.animHandler);

			break;

		case SVT_COSTUME:
			return thisVar.varData.costumeHandler;
			break;

		default:
			fatal("Expecting an animation variable; found variable of type", typeName[thisVar.varType]);
	}

	return p;
}

int stackSize(const stackHandler *me) {
	int r = 0;
	variableStack *a = me->first;
	while (a) {
		r++;
		a = a->next;
	}
	return r;
}

bool getSavedGamesStack(stackHandler *sH, char *ext) {
	char *pattern = joinStrings("*", ext);
	if (!pattern)
		return false;

	variable newName;
	newName.varType = SVT_NULL;
#if 0
	DIR *dir = opendir(".");
	if (!dir)
		return false;

	struct dirent *d = readdir(dir);
	while (d != NULL) {
		if (!strcmp(d->d_name + strlen(d->d_name) - strlen(ext), ext)) {
			d->d_name[strlen(d->d_name) - strlen(ext)] = 0;
			char *decoded = decodeFilename(d->d_name);
			makeTextVar(newName, decoded);
			delete[] decoded;
			if (!addVarToStack(newName, sH->first))
				return false;
			if (sH->last == NULL)
				sH->last = sH->first;
		}

		d = readdir(dir);
	}

	closedir(dir);
#endif
	delete[] pattern;
	pattern = NULL;
	return true;
}

bool copyStack(const variable &from, variable &to) {
	to.varType = SVT_STACK;
	to.varData.theStack = new stackHandler;
	if (!checkNew(to.varData.theStack))
		return false;
	to.varData.theStack->first = NULL;
	to.varData.theStack->last = NULL;
	to.varData.theStack->timesUsed = 1;
	variableStack *a = from.varData.theStack->first;

#if DEBUG_STACKINESS
	{
		char *str = getTextFromAnyVar(from);
		stackDebug((stackfp, "in copyStack, copying %s\n", str));
		delete[] str;
	}
#endif

	while (a) {
		addVarToStack(a->thisVar, to.varData.theStack->first);
		if (to.varData.theStack->last == NULL) {
#if DEBUG_STACKINESS
			stackDebug((stackfp, "LAST"));
#endif
			to.varData.theStack->last = to.varData.theStack->first;
		}

#if DEBUG_STACKINESS
		{
			char *str = getTextFromAnyVar(a->thisVar);
			stackDebug((stackfp, "\ta->thisVar = %s (%p)\n", str, to.varData.theStack->first));
			delete[] str;
		}
#endif

		a = a->next;
	}

#if DEBUG_STACKINESS
	{
		char *str = getTextFromAnyVar(to);
		stackDebug((stackfp, "finished copy, got %s\n", str));
		delete[] str;
		stackDebug((stackfp, "first = %p\n", to.varData.theStack->first));
		stackDebug((stackfp, "last = %p\n", to.varData.theStack->last));
	}
#endif
	return true;
}

/*void newStackVar (variable & thisVar) {
 unlinkVar (thisVar);
 thisVar.varType = VT_STACK;
 thisVar.varData.theStack = NULL;
 } */

void addVariablesInSecond(variable &var1, variable &var2) {
	if (var1.varType == SVT_INT && var2.varType == SVT_INT) {
		var2.varData.intValue += var1.varData.intValue;
	} else {
		char *string1 = getTextFromAnyVar(var1);
		char *string2 = getTextFromAnyVar(var2);

		unlinkVar(var2);
		var2.varData.theString = joinStrings(string1, string2);
		var2.varType = SVT_STRING;
		delete[] string1;
		delete[] string2;
	}
}

int compareVars(const variable &var1, const variable &var2) {
	int re = 0;
	if (var1.varType == var2.varType) {
		switch (var1.varType) {
			case SVT_NULL:
				re = 1;
				break;

			case SVT_COSTUME:
				re = (var1.varData.costumeHandler == var2.varData.costumeHandler);
				break;

			case SVT_ANIM:
				re = (var1.varData.animHandler == var2.varData.animHandler);
				break;

			case SVT_STRING:

				re = (strcmp(var1.varData.theString, var2.varData.theString) == 0);
				break;

			case SVT_STACK:
				re = (var1.varData.theStack == var2.varData.theStack);
				break;

			default:
				re = (var1.varData.intValue == var2.varData.intValue);
		}
	}
	return re;
}

void compareVariablesInSecond(const variable &var1, variable &var2) {
	setVariable(var2, SVT_INT, compareVars(var1, var2));
}

void makeTextVar(variable &thisVar, const char *txt) {
	unlinkVar(thisVar);
	thisVar.varType = SVT_STRING;
	thisVar.varData.theString = copyString(txt);
}

bool loadStringToVar(variable &thisVar, int value) {

	makeTextVar(thisVar, getNumberedString(value));
	return (bool)(thisVar.varData.theString != NULL);
}

char *getTextFromAnyVar(const variable &from) {
	switch (from.varType) {
		case SVT_STRING:
			return copyString(from.varData.theString);

		case SVT_FASTARRAY: {
			char *builder = copyString("FAST:");
			char *builder2;
			char *grabText;

			for (int i = 0; i < from.varData.fastArray->size; i++) {
				builder2 = joinStrings(builder, " ");
				if (!builder2)
					return NULL;
				delete builder;
				grabText = getTextFromAnyVar(from.varData.fastArray->fastVariables[i]);
				builder = joinStrings(builder2, grabText);
				if (!builder)
					return NULL;
				delete grabText;
				grabText = NULL;
				delete builder2;
				builder2 = NULL;
			}
			return builder;
		}

		case SVT_STACK: {
			char *builder = copyString("ARRAY:");
			char *builder2;
			char *grabText;

			variableStack *stacky = from.varData.theStack->first;

			while (stacky) {
				builder2 = joinStrings(builder, " ");
				if (!builder2)
					return NULL;
				delete[] builder;
				grabText = getTextFromAnyVar(stacky->thisVar);
				builder = joinStrings(builder2, grabText);
				if (!builder)
					return NULL;
				delete[] grabText;
				grabText = NULL;
				delete[] builder2;
				builder2 = NULL;
				stacky = stacky->next;
			}
			return builder;
		}

		case SVT_INT: {
			char *buff = new char[10];
			if (!checkNew(buff))
				return NULL;
			sprintf(buff, "%i", from.varData.intValue);
			return buff;
		}

		case SVT_FILE: {
//			char * buff = new char[15];
//			if (! checkNew (buff)) return NULL;
//			sprintf (buff, "FILE %i", from.varData.intValue);
			return joinStrings("", resourceNameFromNum(from.varData.intValue));
		}

			/*      case SVT_ANIM:
			 {
			 char * buff = new char[20];
			 if (! checkNew (buff)) return NULL;
			 sprintf (buff, "%p", from.varData.animHandler);
			 return buff;
			 }*/

		case SVT_OBJTYPE: {
			objectType *thisType = findObjectType(from.varData.intValue);
			if (thisType)
				return copyString(thisType->screenName);
		}

		default:
			break;
	}

	return copyString(typeName[from.varType]);
}

bool getBoolean(const variable &from) {
	switch (from.varType) {
		case SVT_NULL:
			return false;

		case SVT_INT:
			return (bool)(from.varData.intValue != 0);

		case SVT_STACK:
			return (bool)(from.varData.theStack->first != NULL);

		case SVT_STRING:
			return (bool)(from.varData.theString[0] != 0);

		case SVT_FASTARRAY:
			return (bool)(from.varData.fastArray->size != 0);

		default:
			break;
	}
	return true;
}

bool copyMain(const variable &from, variable &to) {
	to.varType = from.varType;
	switch (to.varType) {
		case SVT_INT:
		case SVT_FUNC:
		case SVT_BUILT:
		case SVT_FILE:
		case SVT_OBJTYPE:
			to.varData.intValue = from.varData.intValue;
			return true;

		case SVT_FASTARRAY:
			to.varData.fastArray = from.varData.fastArray;
			to.varData.fastArray->timesUsed++;
			return true;

		case SVT_STRING:
			to.varData.theString = copyString(from.varData.theString);
			return to.varData.theString ? true : false;

		case SVT_STACK:
			to.varData.theStack = from.varData.theStack;
			to.varData.theStack->timesUsed++;
			return true;

		case SVT_COSTUME:
			to.varData.costumeHandler = from.varData.costumeHandler;
			return true;

		case SVT_ANIM:
			to.varData.animHandler = copyAnim(from.varData.animHandler);
			return true;

		case SVT_NULL:
			return true;

		default:
			break;
	}
	fatal("Unknown value type");
	return false;
}

bool copyVariable(const variable &from, variable &to) {
	unlinkVar(to);
	return copyMain(from, to);
}

variable *fastArrayGetByIndex(fastArrayHandler *vS, unsigned int theIndex) {
	if (theIndex < 0 || theIndex >= vS->size)
		return NULL;
	return &vS->fastVariables[theIndex];
}

bool makeFastArraySize(variable &to, int size) {
	if (size < 0)
		return fatal("Can't create a fast array with a negative number of elements!");
	unlinkVar(to);
	to.varType = SVT_FASTARRAY;
	to.varData.fastArray = new fastArrayHandler;
	if (!checkNew(to.varData.fastArray))
		return false;
	to.varData.fastArray->fastVariables = new variable[size];
	if (!checkNew(to.varData.fastArray->fastVariables))
		return false;
	for (int i = 0; i < size; i++) {
		initVarNew(to.varData.fastArray->fastVariables[i]);
	}
	to.varData.fastArray->size = size;
	to.varData.fastArray->timesUsed = 1;
	return true;
}

bool makeFastArrayFromStack(variable &to, const stackHandler *stacky) {
	int size = stackSize(stacky);
	if (!makeFastArraySize(to, size))
		return false;

	// Now let's fill up the new array

	variableStack *allV = stacky->first;
	size = 0;
	while (allV) {
		copyMain(allV->thisVar, to.varData.fastArray->fastVariables[size]);
		size++;
		allV = allV->next;
	}
	return true;
}

/*
 bool moveVariable (variable & from, variable & to) {
 unlinkVar (to);
 memcpy (& to, & from, sizeof (variable));
 from.varType = SVT_NULL;
 }
 */

bool addVarToStack(const variable &va, variableStack *&thisStack) {
	variableStack *newStack = new variableStack;
	if (!checkNew(newStack))
		return false;

	if (!copyMain(va, newStack->thisVar))
		return false;
	newStack->next = thisStack;
	thisStack = newStack;
	//debug(kSludgeDebugStackMachine, "Variable %s was added to stack", getTextFromAnyVar(va));
	return true;
}

bool addVarToStackQuick(variable &va, variableStack *&thisStack) {
	variableStack *newStack = new variableStack;
	if (!checkNew(newStack))
		return false;

//	if (! copyMain (va, newStack -> thisVar)) return false;

	memcpy(&(newStack->thisVar), &va, sizeof(variable));
	va.varType = SVT_NULL;

	newStack->next = thisStack;
	thisStack = newStack;
	//debug(kSludgeDebugStackMachine, "Variable %s was added to stack quick", getTextFromAnyVar(va));
	return true;
}

bool stackSetByIndex(variableStack *vS, unsigned int theIndex, const variable &va) {
	while (theIndex--) {
		vS = vS->next;
		if (!vS)
			return fatal("Index past end of stack.");
	}
	return copyVariable(va, vS->thisVar);
}

variable *stackGetByIndex(variableStack *vS, unsigned int theIndex) {
	while (theIndex--) {
		vS = vS->next;
		if (!vS) {
			return NULL;
		}
	}
	return &(vS->thisVar);
}

int deleteVarFromStack(const variable &va, variableStack *&thisStack, bool allOfEm) {
	variableStack * * huntVar = &thisStack;
	variableStack *killMe;
	int reply = 0;

	while (*huntVar) {
		if (compareVars((*huntVar)->thisVar, va)) {
			killMe = *huntVar;
			*huntVar = killMe->next;
			unlinkVar(killMe->thisVar);
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
variableStack *stackFindLast(variableStack *hunt) {
	if (hunt == NULL)
		return NULL;

	while (hunt->next)
		hunt = hunt->next;

	return hunt;
}

bool getValueType(int &toHere, variableType vT, const variable &v) {
	//if (! v) return false;
	if (v.varType != vT) {
		char *e1 = joinStrings("Can only perform specified operation on a value which is of type ", typeName[vT]);
		char *e2 = joinStrings("... value supplied was of type ", typeName[v.varType]);
		fatal(e1, e2);

		return false;
	}
	toHere = v.varData.intValue;
	return true;
}

void trimStack(variableStack *&stack) {
	variableStack *killMe = stack;
	stack = stack->next;

	//debug(kSludgeDebugStackMachine, "Variable %s was removed from stack", getTextFromAnyVar(killMe->thisVar));

	// When calling this, we've ALWAYS checked that stack != NULL
	unlinkVar(killMe->thisVar);
	delete killMe;
}

} // End of namespace Sludge
