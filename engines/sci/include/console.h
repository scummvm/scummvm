/***************************************************************************
 console.h Copyright (C) 1999,2000,01 Christoph Reichenbach


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

    Christoph Reichenbach (CJR) [creichen@rbg.informatik.tu-darmstadt.de]

***************************************************************************/
/* Header file for the SCI console.
** Please note that the console does not use the priority field; the console
** should therefore be drawn after everything else has been drawn (with the
** possible exception of the mouse pointer).
*/

#ifndef _SCI_CONSOLE_H_
#define _SCI_CONSOLE_H_

#include "sci/include/sci_memory.h"
#include "sci/include/resource.h"
#include "sci/include/vm_types.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#ifdef _DOS
#include <sci_dos.h>
#endif

#define SCI_CONSOLE
#include "sci/include/gfx_operations.h"


extern DLLEXTERN int con_passthrough;
/* Echo all sciprintf() stuff to the text console */
extern FILE *con_file;
/* Echo all sciprintf() output to a text file. Note: clients of freesci.dll
** should use open_console_file and close_console_file, rather than refer
** directly to the con_file variable.
*/

struct _state; /* state_t later on */

typedef union {
	long val;
	char *str;
	reg_t reg;
} cmd_param_t;

extern unsigned int cmd_paramlength;
/* The number of parameters passed to a function called from the parser */

extern cmd_param_t *cmd_params;
/* The parameters passed to a function called by the parser */

extern struct _state *con_gamestate;
/* The game state as used by some of the console commands */


/*** FUNCTION DEFINITIONS ***/

void
con_set_string_callback(void(*callback)(char *));
/* Sets the console string callback
** Parameters: (void -> char *) callback: The closure to invoke after
**                              a string for sciprintf() has been generated
** This sets a single callback function to be used after sciprintf()
** is used.
*/

void
con_set_pixmap_callback(void(*callback)(gfx_pixmap_t *));
/* Sets the console pixmap callback
** Parameters: (void -> gfx_pixmap_t *) callback: The closure to invoke after
**                                      a pixmap has been provided to be
**                                      published in the on-screen console
** This sets a single callback function to be used after sciprintf()
** is used.
*/

void
con_init(void);
/* Initializes the command parser
** Parameters: (void)
** Returns   : (void)
** This function will initialize hook up a few commands to the parser.
** It must be called before cmdParse() is used.
*/


void
con_parse(struct _state *s, const char *command);
/* Parses a command and summons appropriate facilities to handle it
** Parameters: (state_t *) s: The state_t to use
**             command: The command to execute
** Returns   : (void)
*/


int
con_hook_command(int command(struct _state *s), const char *name, const char *param, const char *description);
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
**   ! Special token: state_t* must be set for this function to be called
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

cmd_param_t
con_getopt(char *opt);
/* Retreives the specified optional parameter
** -- for use within console functions only --
** Parameters: (char *) opt: The optional parameter to retreive
** Returns   : (cmd_param_t) The corresponding parameter
** Should only be used if con_hasopt() reports its presence.
*/

int
con_hasopt(char *opt);
/* Checks whether an optional parameter was specified
** -- for use within console functions only --
** Parameters: (char *) opt: The optional parameter to check for
** Returns   : (int) non-zero iff the parameter was specified
*/

int
con_can_handle_pixmaps(void);
/* Determines whether the console supports pixmap inserts
** Returns   : (int) non-zero iff pixmap inserts are supported
*/

int
con_insert_pixmap(gfx_pixmap_t *pixmap);
/* Inserts a pixmap into the console history buffer
** Parameters: (gfx_pixmap_t *) pixmap: The pixmap to insert
** Returns   : (int) 0 on success, non-zero if no receiver for
**                   the pixmap could not be found
** The pixmap must be unique; it is freed by the console on demand.
** Use gfx_clone_pixmap() if neccessary.
** If the pixmap could not be inserted, the called must destroy it
*/

int
con_hook_page(const char *topic, const char *body);
/* Hooks a general information page to the manual page system
** Parameters: (const char *) topic: The topic name
**             (const char *) body: The text body to assign to the topic
** Returns   : (int) 0 on success
*/

int
con_hook_int(int *pointer, const char *name, const char *description);
/* Adds an int to the list of modifyable ints.
** Parameters: pointer: Pointer to the int to add to the list
**             name: Name for this value
**             description: A short description for the value
** Returns   : 0 on success, 1 if either value has already been added
**             or if name is already being used for a different value.
** The internal list of int references is used by some of the basic commands.
*/


void
con_gfx_init(void);
/* Initializes the gfx console
*/

void
con_gfx_show(gfx_state_t *state);
/* Enters on-screen console mode
** Parameters: (gfx_state_t *state): The graphics state to use for interaction
** Returns   : (void)
*/

char *
con_gfx_read(gfx_state_t *state);
/* Reads a single line from the on-screen console, if it is open
** Parameters: (gfx_state_t *state): The graphics state to use for interaction
** Returns   : (char *) The input, in a static buffer
*/

void
con_gfx_hide(gfx_state_t *stae);
/* Closes the on-screen console
** Parameters: (gfx_state_t *state): The graphics state to use for interaction
** Returns   : (void)
*/


int
sci_hexdump(byte *data, int length, int offsetplus);

void
open_console_file(char *filename);
/* Opens the file to which the console output is echoed. If a file was opened
** before, closes it.
** Parameters: filename - name of the file
** Returns   : (void)
*/

void
close_console_file(void);
/* Closes the console output file.
** Parameters: (void)
** Returns   : (void)
*/

#endif /* _SCI_CONSOLE_H_ */ 
