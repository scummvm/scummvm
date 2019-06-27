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

#ifndef GLK_ALAN3_PARAMS
#define GLK_ALAN3_PARAMS

/* Various utility functions for handling parameters */
#include "glk/alan3/types.h"
#include "glk/alan3/acode.h"

namespace Glk {
namespace Alan3 {

/* TYPES */
struct Parameter {        /* PARAMETER */
	Aid instance;                  /* Instance code for the parameter (0=multiple) */
	bool isLiteral;
	bool isPronoun;
	bool isThem;
	bool useWords;                 /* Indicate to use words instead of instance code when saying */
	int firstWord;                 /* Index to first word used by player */
	int lastWord;                  /* d:o to last */
	struct Parameter *candidates; /* Array of instances possibly matching this parameter depending on player input */
};

typedef Parameter *ParameterArray;


/* DATA */
extern Parameter *globalParameters;


/* FUNCTIONS */
/* Single Parameter: */
extern Parameter *newParameter(int instanceId);
extern void clearParameter(Parameter *parameter);
extern void copyParameter(Parameter *theCopy, Parameter *theOriginal);

/* ParameterArray: */
extern ParameterArray newParameterArray(void);
extern ParameterArray ensureParameterArrayAllocated(ParameterArray currentArray);
extern void freeParameterArray(Parameter *array);

extern bool parameterArrayIsEmpty(ParameterArray parameters);
extern void addParameterToParameterArray(ParameterArray theArray, Parameter *theParameter);
extern void addParameterForInstance(ParameterArray parameters, int instance);
extern void addParameterForInteger(ParameterArray parameters, int value);
extern void addParameterForString(ParameterArray parameters, char *value);
extern Parameter *findEndOfParameterArray(ParameterArray parameters);
extern void compressParameterArray(ParameterArray a);
extern int lengthOfParameterArray(ParameterArray a);
extern bool equalParameterArrays(ParameterArray parameters1, ParameterArray parameters2);
extern bool inParameterArray(ParameterArray l, Aword e);
extern void copyParameterArray(ParameterArray to, ParameterArray from);
extern void clearParameterArray(ParameterArray list);
extern void subtractParameterArrays(ParameterArray a, ParameterArray b);
extern void mergeParameterArrays(ParameterArray a, ParameterArray b);
extern void intersectParameterArrays(ParameterArray a, ParameterArray b);
extern void copyReferencesToParameterArray(Aint *references, ParameterArray parameters);
extern void printParameterArray(ParameterArray parameters);

extern int findMultiplePosition(ParameterArray parameters);

/* Global Parameters: */
extern void setGlobalParameters(ParameterArray parameters);
extern ParameterArray getGlobalParameters(void);
extern ParameterArray getGlobalParameter(int parameterIndex);

} // End of namespace Alan3
} // End of namespace Glk

#endif
