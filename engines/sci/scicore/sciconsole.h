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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/* Header file for the SCI console.
** Please note that the console does not use the priority field; the console
** should therefore be drawn after everything else has been drawn (with the
** possible exception of the mouse pointer).
*/

#ifndef SCI_SCICORE_SCICONSOLE_H
#define SCI_SCICORE_SCICONSOLE_H

#include "common/scummsys.h"

#include "sci/tools.h"
#include "sci/engine/vm_types.h"

#define SCI_CONSOLE

namespace Sci {

struct gfx_pixmap_t;

union cmd_param_t {
	int32 val;
	const char *str;
	reg_t reg;
};


typedef int (*ConCommand)(EngineState *s, const Common::Array<cmd_param_t> &cmdParams);

/*** FUNCTION DEFINITIONS ***/

void con_set_pixmap_callback(void(*callback)(gfx_pixmap_t *));
/* Sets the console pixmap callback
** Parameters: (void -> gfx_pixmap_t *) callback: The closure to invoke after
**                                      a pixmap has been provided to be
**                                      published in the on-screen console
** This sets a single callback function to be used after sciprintf()
** is used.
*/

void con_init();
/* Initializes the command parser
** Parameters: (void)
** Returns   : (void)
** This function will initialize hook up a few commands to the parser.
** It must be called before cmdParse() is used.
*/


void con_parse(EngineState *s, const char *command);
/* Parses a command and summons appropriate facilities to handle it
** Parameters: (EngineState *) s: The EngineState to use
**             command: The command to execute
** Returns   : (void)
*/


int con_hook_command(ConCommand command, const char *name, const char *param, const char *description);
/* Adds a command to the parser's command list
** Parameters: command: The command to add
**             name: The command's name
**             param: A description of the parameters it takes
**             description: A short description of what it does
** Returns   : 0 if successful, 1 if appending failed because
**             of an incorrect *param string, 'command'==0, or
**             'name' already being in use.
** A valid param string is either empty (no parameters allowed)
** or contains one of the following tokens:
**   ! Special token: EngineState* must be set for this function to be called
**   i (an int)
**   s (a 'string' (char *))
**   h (a byte, described in hexadecimal digits)
**   a (a heap address, register or object name)
**   r (any register value)
**   x* (an arbitrary (possibly 0) number of 'x' tokens)
** The '*' token may only be used as the last token of the list.
** Another way to specify optional parameters is by means of the
** '-opt:t' notation, which allows an optional parameter of type 't'
** to be specified as 'opt:<value>' when calling. See also the
** con_hasopt() and con_getopt() calls.
**
** Please note that the 'h' type does accept hexadecimal numbers greater
** than 0xff and less than 0x00, but clips them to this range.
**
** Example: "isi*" would define the function to take an int, a
** string, and an arbitrary number of ints as parameters (in that sequence).
**
** When the function is called, it can retrieve its parameters from cmd_params;
** the actual number of parameters is stored in cmd_paramlength.
** It is allowed to modify the char*s from a cmd_params[] element, as long
** as no element beyond strlen(cmd_params[x].str)+1 is accessed.
*/

int con_can_handle_pixmaps();
/* Determines whether the console supports pixmap inserts
** Returns   : (int) non-zero iff pixmap inserts are supported
*/

int con_insert_pixmap(gfx_pixmap_t *pixmap);
/* Inserts a pixmap into the console history buffer
** Parameters: (gfx_pixmap_t *) pixmap: The pixmap to insert
** Returns   : (int) 0 on success, non-zero if no receiver for
**                   the pixmap could not be found
** The pixmap must be unique; it is freed by the console on demand.
** Use gfx_clone_pixmap() if neccessary.
** If the pixmap could not be inserted, the called must destroy it
*/

int con_hook_page(const char *topic, const char *body);
/* Hooks a general information page to the manual page system
** Parameters: (const char *) topic: The topic name
**             (const char *) body: The text body to assign to the topic
** Returns   : (int) 0 on success
*/

int con_hook_int(int *pointer, const char *name, const char *description);
/* Adds an int to the list of modifyable ints.
** Parameters: pointer: Pointer to the int to add to the list
**             name: Name for this value
**             description: A short description for the value
** Returns   : 0 on success, 1 if either value has already been added
**             or if name is already being used for a different value.
** The internal list of int references is used by some of the basic commands.
*/


int sci_hexdump(byte *data, int length, int offsetplus);

} // End of namespace Sci

#endif // SCI_SCICORE_SCICONSOLE_H
