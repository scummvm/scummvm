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

#include "glk/alan3/alt_info.h"
#include "glk/alan3/types.h"
#include "glk/alan3/checkentry.h"
#include "glk/alan3/debug.h"
#include "glk/alan3/inter.h"
#include "glk/alan3/glkio.h"
#include "glk/alan3/lists.h"
#include "glk/alan3/instance.h"
#include "glk/alan3/options.h"
#include "glk/alan3/memory.h"
#include "glk/alan3/current.h"
#include "glk/alan3/class.h"
#include "glk/alan3/params.h"
#include "glk/alan3/literal.h"
#include "glk/alan3/syntax.h"

namespace Glk {
namespace Alan3 {

/* Types */
typedef AltInfo *AltInfoFinder(int verb, Parameter parameters[]);


/*======================================================================*/
void primeAltInfo(AltInfo *altInfo, int level, int parameter, int instance, int cls) {
	altInfo->level = level;
	altInfo->parameter = parameter;
	altInfo->instance = instance;
	altInfo->_class = cls;
	altInfo->done = FALSE;
	altInfo->end = FALSE;
}


/*----------------------------------------------------------------------*/
static void traceInstanceAndItsClass(CONTEXT, Aid instance, Aid cls) {
	CALL1(traceSay, instance)
	printf("[%d]", instance);
	if (cls != NO_CLASS)
		printf(", inherited from %s[%d]", idOfClass(cls), cls);
}


/*----------------------------------------------------------------------*/
static void traceAltInfo(CONTEXT, AltInfo *alt) {
	switch (alt->level) {
	case GLOBAL_LEVEL:
		printf("GLOBAL");
		break;
	case LOCATION_LEVEL:
		printf("in (location) ");
		CALL2(traceInstanceAndItsClass, current.location, alt->_class)
		break;
	case PARAMETER_LEVEL: {
		char *parameterName = parameterNameInSyntax(current.verb, alt->parameter);
		if (parameterName != NULL)
			printf("in parameter %s(#%d)=", parameterName, alt->parameter);
		else
			printf("in parameter #%d=", alt->parameter);
		CALL2(traceInstanceAndItsClass, globalParameters[alt->parameter - 1].instance, alt->_class)
		break;
	}
	default:
		break;
	}
}


/*----------------------------------------------------------------------*/
static void traceVerbCheck(CONTEXT, AltInfo *alt, bool execute) {
	if (traceSectionOption && execute) {
		printf("\n<VERB %d, ", current.verb);
		CALL1(traceAltInfo, alt)
		printf(", CHECK:>\n");
	}
}


/*======================================================================*/
bool checkFailed(CONTEXT, AltInfo *altInfo, bool execute) {
	if (altInfo->alt != NULL && altInfo->alt->checks != 0) {
		R0CALL2(traceVerbCheck, altInfo, execute)

		// TODO Why does this not generate a regression error with !
		// Need a new regression case?
		R0FUNC2(checksFailed, fail, altInfo->alt->checks, execute)
		return fail;
	}
	return FALSE;
}


/*----------------------------------------------------------------------*/
static void traceVerbExecution(CONTEXT, AltInfo *alt) {
	if (traceSectionOption) {
		printf("\n<VERB %d, ", current.verb);
		CALL1(traceAltInfo, alt)
		printf(", DOES");

		switch (alt->alt->qual) {
		case Q_BEFORE:
			printf(" (BEFORE)");
			break;
		case Q_ONLY:
			printf(" (ONLY)");
			break;
		case Q_AFTER:
			printf(" (AFTER)");
			break;
		case Q_DEFAULT:
		default:
			break;
		}
		printf(":>\n");
	}
}


/*======================================================================*/
bool executedOk(CONTEXT, AltInfo *altInfo) {
	fail = FALSE;
	if (!altInfo->done && altInfo->alt->action != 0) {
		R0CALL1(traceVerbExecution, altInfo)
		current.instance = altInfo->instance;
		R0CALL1(interpret, altInfo->alt->action)
	}
	altInfo->done = TRUE;
	return !fail;
}


/*======================================================================*/
bool canBeExecuted(AltInfo *altInfo) {
	return altInfo->alt != NULL && altInfo->alt->action != 0;
}


/*======================================================================*/
AltInfo *duplicateAltInfoArray(AltInfo original[]) {
	int size;
	AltInfo *duplicate;

	for (size = 0; original[size].end != TRUE; size++)
		;
	size++;
	duplicate = (AltInfo *)allocate(size * sizeof(AltInfo));
	memcpy(duplicate, original, size * sizeof(AltInfo));
	return duplicate;
}


/*======================================================================*/
int lastAltInfoIndex(AltInfo altInfo[]) {
	int altIndex;

	/* Loop to last alternative */
	for (altIndex = -1; !altInfo[altIndex + 1].end; altIndex++)
		;
	return altIndex;
}


/*----------------------------------------------------------------------*/
static AltInfo *nextFreeAltInfo(AltInfoArray altInfos) {
	return &altInfos[lastAltInfoIndex(altInfos) + 1];
}


/*----------------------------------------------------------------------*/
static void addAlternative(AltInfoArray altInfos, int verb, int level, Aint parameterNumber, Aint theClass, Aid theInstance, AltEntryFinder finder) {
	AltInfo *altInfoP = nextFreeAltInfo(altInfos);

	altInfoP->alt = (*finder)(verb, parameterNumber, theInstance, theClass);
	if (altInfoP->alt != NULL) {
		primeAltInfo(altInfoP, level, parameterNumber, theInstance, theClass);
		altInfoP[1].end = TRUE;
	}
}


/*----------------------------------------------------------------------*/
static void addGlobalAlternatives(AltInfoArray altInfos, int verb, AltEntryFinder finder) {
	addAlternative(altInfos, verb, GLOBAL_LEVEL, NO_PARAMETER, NO_CLASS, NO_INSTANCE, finder);
}


/*----------------------------------------------------------------------*/
static void addAlternativesFromParents(AltInfoArray altInfos, int verb, int level, Aint parameterNumber, Aint theClass, Aid theInstance, AltEntryFinder finder) {
	if (classes[theClass].parent != 0)
		addAlternativesFromParents(altInfos, verb, level,
		                           parameterNumber,
		                           classes[theClass].parent,
		                           theInstance,
		                           finder);

	addAlternative(altInfos, verb, level, parameterNumber, theClass, theInstance, finder);
}


/*----------------------------------------------------------------------*/
static void addAlternativesFromLocation(AltInfoArray altInfos, int verb, Aid location, AltEntryFinder finder) {
	if (admin[location].location != 0)
		addAlternativesFromLocation(altInfos, verb, admin[location].location, finder);

	addAlternativesFromParents(altInfos, verb,
	                           LOCATION_LEVEL,
	                           NO_PARAMETER,
	                           instances[location].parent,
	                           location,
	                           finder);

	addAlternative(altInfos, verb, LOCATION_LEVEL, NO_PARAMETER, NO_CLASS, location, finder);
}


/*----------------------------------------------------------------------*/
static void addAlternativesFromParameter(AltInfoArray altInfos, int verb, Parameter parameters[], int parameterNumber, AltEntryFinder finder) {
	Aid parent;
	Aid theInstance = parameters[parameterNumber - 1].instance;

	if (isLiteral(theInstance))
		parent = literals[literalFromInstance(theInstance)]._class;
	else
		parent = instances[theInstance].parent;
	addAlternativesFromParents(altInfos, verb, PARAMETER_LEVEL, parameterNumber, parent, theInstance, finder);

	if (!isLiteral(theInstance))
		addAlternative(altInfos, verb, PARAMETER_LEVEL, parameterNumber, NO_CLASS, theInstance, finder);
}


/*======================================================================*/
bool anyCheckFailed(CONTEXT, AltInfoArray altInfo, bool execute) {
	int altIndex;
	bool flag;

	if (altInfo != NULL)
		for (altIndex = 0; !altInfo[altIndex].end; altIndex++) {
			current.instance = altInfo[altIndex].instance;

			R0FUNC2(checkFailed, flag, &altInfo[altIndex], execute)
			return flag;
		}
	return FALSE;
}


/*======================================================================*/
bool anythingToExecute(AltInfo altInfo[]) {
	int altIndex;

	/* Check for anything to execute... */
	if (altInfo != NULL)
		for (altIndex = 0; !altInfo[altIndex].end; altIndex++)
			if (canBeExecuted(&altInfo[altIndex]))
				return TRUE;
	return FALSE;
}


/*----------------------------------------------------------------------*/
static VerbEntry *findVerbEntry(int verbCode, VerbEntry *entries) {
	VerbEntry *verbEntry;
	for (verbEntry = entries; !isEndOfArray(verbEntry); verbEntry++) {
		if (verbEntry->code < 0) {
			/* Verb codes are negative for Meta verbs, if so they are also 1 off to avoid EOD */
			if (abs(verbEntry->code) - 1 == verbCode)
				return verbEntry;
		} else {
			if (verbEntry->code == verbCode)
				return verbEntry;
		}
	}
	return NULL;
}


/*----------------------------------------------------------------------*/
static AltEntry *findAlternative(Aaddr verbTableAddress, int verbCode, int parameterNumber) {
	AltEntry *alt;
	VerbEntry *verbEntry;

	if (verbTableAddress == 0) return NULL;

	verbEntry = findVerbEntry(verbCode, (VerbEntry *) pointerTo(verbTableAddress));
	if (verbEntry != NULL)
		for (alt = (AltEntry *) pointerTo(verbEntry->alts); !isEndOfArray(alt); alt++) {
			if (alt->param == parameterNumber || alt->param == 0) {
				if (verbEntry->code < 0) current.meta = TRUE;
				return alt;
			}
		}
	return NULL;
}


/*----------------------------------------------------------------------*/
static AltEntry *alternativeFinder(int verb, int parameterNumber, int theInstance, int theClass) {
	if ((Aword)theClass != NO_CLASS)
		return findAlternative(classes[theClass].verbs, verb, parameterNumber);
	else if ((Aword)theInstance != NO_INSTANCE)
		return findAlternative(instances[theInstance].verbs, verb, parameterNumber);
	else
		return findAlternative(header->verbTableAddress, verb, parameterNumber);
}


/*======================================================================*/
AltInfo *findAllAlternatives(int verb, Parameter parameters[]) {
	int parameterNumber;
	AltInfo altInfos[1000];
	altInfos[0].end = TRUE;

	addGlobalAlternatives(altInfos, verb, &alternativeFinder);

	addAlternativesFromLocation(altInfos, verb, current.location, &alternativeFinder);

	for (parameterNumber = 1; !isEndOfArray(&parameters[parameterNumber - 1]); parameterNumber++) {
		addAlternativesFromParameter(altInfos, verb, parameters, parameterNumber, &alternativeFinder);
	}
	return duplicateAltInfoArray(altInfos);
}


/*----------------------------------------------------------------------*/
static bool possibleWithFinder(CONTEXT, int verb, Parameter parameters[], AltInfoFinder *finder) {
	bool anything;
	AltInfo *allAlternatives;
	bool flag;

	allAlternatives = finder(verb, parameters);

	// TODO Need to do this since anyCheckFailed() call execute() which assumes the global parameters
	setGlobalParameters(parameters);
	R0FUNC2(anyCheckFailed, flag, allAlternatives, DONT_EXECUTE_CHECK_BODY_ON_FAIL)
	if (flag)
		anything = FALSE;
	else
		anything = anythingToExecute(allAlternatives);

	if (allAlternatives != NULL)
		deallocate(allAlternatives);

	return (anything);
}



/*======================================================================*/
bool possible(CONTEXT, int verb, Parameter inParameters[], ParameterPosition parameterPositions[]) {
	// This is a wrapper for possibleWithFinder() which is used in unit tests
	// possible() should be used "for real".

	return possibleWithFinder(context, verb, inParameters, findAllAlternatives);
}

} // End of namespace Alan3
} // End of namespace Glk
