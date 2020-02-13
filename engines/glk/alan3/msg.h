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

#ifndef GLK_ALAN3_MSG
#define GLK_ALAN3_MSG

#include "glk/alan3/acode.h"
#include "glk/jumps.h"
#include "glk/alan3/types.h"
#include "glk/alan3/params.h"

namespace Glk {
namespace Alan3 {

/* TYPES */
typedef struct MessageEntry {   /* MESSAGE TABLE */
	Aaddr stms;           /* Address to statements*/
} MessageEntry;


/* DATA */
extern MessageEntry *msgs;  /* Message table pointer */


/* FUNCTIONS */
extern void setErrorHandler(void (*handler)(MsgKind));
extern void abortPlayerCommand(CONTEXT);
extern void error(CONTEXT, MsgKind msg);
extern bool confirm(CONTEXT, MsgKind msgno);
extern void printMessage(MsgKind msg);
extern void printMessageWithParameters(MsgKind msg, Parameter *messageParameters);
extern void printMessageWithInstanceParameter(MsgKind message, int i);
extern void printMessageUsing2InstanceParameters(MsgKind message, int instance1, int instance2);

} // End of namespace Alan3
} // End of namespace Glk

#endif
