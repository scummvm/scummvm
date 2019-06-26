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

#include "glk/alan3/act.h"
#include "glk/alan3/alt_info.h"
#include "glk/alan3/output.h"
#include "glk/alan3/msg.h"
#include "glk/alan3/exe.h"
#include "glk/alan3/lists.h"
#include "common/textconsole.h"

namespace Glk {
namespace Alan3 {

/*----------------------------------------------------------------------*/
static void executeCommand(int verb, Parameter parameters[])
{
    static AltInfo *altInfos = NULL; /* Need to survive lots of different exits...*/
    int altIndex;

    /* Did we leave anything behind last time... */
    if (altInfos != NULL)
        free(altInfos);

    altInfos = findAllAlternatives(verb, parameters);

    if (anyCheckFailed(altInfos, EXECUTE_CHECK_BODY_ON_FAIL))
        return;

    /* Check for anything to execute... */
    if (!anythingToExecute(altInfos))
        error(M_CANT0);

    /* Now perform actions! First try any BEFORE or ONLY from inside out */
    for (altIndex = lastAltInfoIndex(altInfos); altIndex >= 0; altIndex--) {
        if (altInfos[altIndex].alt != 0) // TODO Can this ever be NULL? Why?
            if (altInfos[altIndex].alt->qual == (Aword)Q_BEFORE
                || altInfos[altIndex].alt->qual == (Aword)Q_ONLY) {
                if (!executedOk(&altInfos[altIndex]))
                    abortPlayerCommand();
                if (altInfos[altIndex].alt->qual == (Aword)Q_ONLY)
                    return;
            }
    }
        
    /* Then execute any not declared as AFTER, i.e. the default */
    for (altIndex = 0; !altInfos[altIndex].end; altIndex++) {
        if (altInfos[altIndex].alt != 0)
            if (altInfos[altIndex].alt->qual != (Aword)Q_AFTER)
                if (!executedOk(&altInfos[altIndex]))
                    abortPlayerCommand();
    }
        
    /* Finally, the ones declared as AFTER */
    for (altIndex = lastAltInfoIndex(altInfos); altIndex >= 0; altIndex--) {
        if (altInfos[altIndex].alt != 0)
            if (!executedOk(&altInfos[altIndex]))
                abortPlayerCommand();
    }
}


/*======================================================================
 
  action()
 
  Execute the command. Handles acting on multiple items
  such as ALL, THEM or lists of objects.
 
*/
void action(int verb, Parameter parameters[], Parameter multipleMatches[])
{
    int multiplePosition;
#ifdef TODO
	char marker[10];
#endif

    multiplePosition = findMultiplePosition(parameters);
    if (multiplePosition != -1) {
#ifdef TODO
		jmp_buf savedReturnLabel;
        memcpy(savedReturnLabel, returnLabel, sizeof(returnLabel));
        sprintf(marker, "($%d)", multiplePosition+1); /* Prepare a printout with $1/2/3 */
        for (int i = 0; !isEndOfArray(&multipleMatches[i]); i++) {
            copyParameter(&parameters[multiplePosition], &multipleMatches[i]);
            setGlobalParameters(parameters); /* Need to do this here since the marker use them */
            output(marker);
            // TODO: if execution for one parameter aborts we should return here, not to top level
            if (setjmp(returnLabel) == NO_JUMP_RETURN)
                executeCommand(verb, parameters);
            if (multipleMatches[i+1].instance != EOF)
                para();
        }
        memcpy(returnLabel, savedReturnLabel, sizeof(returnLabel));
        parameters[multiplePosition].instance = 0;
#else
		::error("TODO: action");
#endif
    } else {
        setGlobalParameters(parameters);
        executeCommand(verb, parameters);
    }

}

} // End of namespace Alan3
} // End of namespace Glk
