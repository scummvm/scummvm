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

#ifndef GLK_ALAN3_ALT_INFO
#define GLK_ALAN3_ALT_INFO

/* An info node about the Verb Alternatives found and possibly executed */

#include "glk/alan3/types.h"
#include "glk/alan3/acode.h"
#include "glk/jumps.h"
#include "glk/alan3/params.h"
#include "glk/alan3/parameter_position.h"

namespace Glk {
namespace Alan3 {

/* Constants */

#define GLOBAL_LEVEL (0)
#define LOCATION_LEVEL (1)
#define PARAMETER_LEVEL (2)

#define NO_PARAMETER ((Aword)-1)
#define NO_INSTANCE ((Aword)-1)
#define NO_CLASS ((Aword)-1)

/* tryCheck() flags */
#define EXECUTE_CHECK_BODY_ON_FAIL TRUE
#define DONT_EXECUTE_CHECK_BODY_ON_FAIL FALSE


/* Types */

struct AltInfo {
	bool end;       /* Indicator of end in AltInfoArray, first empty has TRUE here */
	AltEntry *alt;
	bool done;
	Aint level;     /* 0 - Global, 1 - location, 2 - parameter */
	Aid _class;     /* In which class, only used for tracing */
	Aid instance;   /* In which instance the Alternative was found,
                       used to set current.instance and tracing */
	Aid parameter;     /* In which parameter, only used for tracing */
};

typedef AltEntry *(*AltEntryFinder)(int verb, int parameterNumber, int theInstance, int theClass);

typedef AltInfo AltInfoArray[];



/* Data */


/* Functions */
extern void primeAltInfo(AltInfo *altInfo, int level, int parameter, int instance, int cls);
extern bool executedOk(CONTEXT, AltInfo *altInfo);
extern bool checkFailed(CONTEXT, AltInfo *altInfo, bool execute);
extern bool canBeExecuted(AltInfo *altInfo);
extern AltInfo *duplicateAltInfoArray(AltInfoArray altInfos);
extern int lastAltInfoIndex(AltInfoArray altInfos);
extern bool anyCheckFailed(CONTEXT, AltInfoArray altInfos, bool execute);
extern bool anythingToExecute(AltInfoArray altInfos);
extern bool possible(CONTEXT, int verb, Parameter parameters[], ParameterPosition parameterPositions[]);
extern AltInfo *findAllAlternatives(int verb, Parameter parameters[]);

} // End of namespace Alan3
} // End of namespace Glk

#endif
