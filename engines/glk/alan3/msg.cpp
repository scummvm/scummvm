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

#include "glk/alan3/msg.h"
#include "glk/alan3/memory.h"
#include "glk/alan3/inter.h"
#include "glk/alan3/exe.h"
#include "glk/alan3/lists.h"

namespace Glk {
namespace Alan3 {

/* PUBLIC DATA */
MessageEntry *msgs;         /* Message table pointer */

/*======================================================================*/
void printMessage(MsgKind msg) {    /* IN - message number */
	Context ctx;
	interpret(ctx, msgs[msg].stms);
}


static void (*errorHandler)(MsgKind msg) = NULL;

/*======================================================================*/
void setErrorHandler(void (*handler)(MsgKind msg)) { /* IN - The error message number */
	// N.B. The error handler must not return because the standard handler does not...
	errorHandler = handler;
}


/*======================================================================*/
void error(CONTEXT, MsgKind msgno) { /* IN - The error message number */
	if (errorHandler != NULL)
		errorHandler(msgno);
	else {
		/* Print an error message and longjmp to main loop. */
		if (msgno != NO_MSG)
			printMessage(msgno);
		LONG_JUMP_LABEL("returnError");
	}
}


/*======================================================================*/
void abortPlayerCommand(CONTEXT) {
	error(context, NO_MSG);
}


/*======================================================================*/
void printMessageWithInstanceParameter(MsgKind message, int instanceId) {
	ParameterArray parameters = newParameterArray();
	addParameterForInstance(parameters, instanceId);
	printMessageWithParameters(message, parameters);
	freeParameterArray(parameters);
}


/*======================================================================*/
void printMessageUsing2InstanceParameters(MsgKind message, int instance1, int instance2) {
	ParameterArray parameters = newParameterArray();
	addParameterForInstance(parameters, instance1);
	addParameterForInstance(parameters, instance2);
	printMessageWithParameters(message, parameters);
	freeParameterArray(parameters);
}


/*======================================================================*/
void printMessageWithParameters(MsgKind msg, Parameter *messageParameters) {
	Parameter *savedParameters = newParameterArray();
	Context ctx;

	copyParameterArray(savedParameters, globalParameters);
	copyParameterArray(globalParameters, messageParameters);

	interpret(ctx, msgs[msg].stms);

	copyParameterArray(globalParameters, savedParameters);
	freeParameterArray(savedParameters);
}

} // End of namespace Alan3
} // End of namespace Glk
