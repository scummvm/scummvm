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

/* interface to command line option service routines
 */

#ifndef GLK_TADS_TADS2_COMMAND_LINE
#define GLK_TADS_TADS2_COMMAND_LINE

#include "glk/tads/tads2/error_handling.h"
#include "glk/tads/os_frob_tads.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

/*
 *   Get argument to an option.  Option can be rammed up against option
 *   letter(s) with no space, or can be separated by a space.  argp is a
 *   pointer to the pointer to the current position in the argv[] array;
 *   ip is a pointer to the index in the argv[] array.  Both *argpp and
 *   *ip are incremented if the next word must be read.  argc is the total
 *   number of arguments.  ofs gives the number of characters (NOT
 *   including the '-') in this option flag; most options will have ofs==1
 *   since they are of the form '-x'.  usagefn is a function to call if
 *   the parsing fails; it is not expected to return, but should signal an
 *   error instead.  
 */
char *cmdarg(errcxdef *ec, char ***argpp, int *ip, int argc,
             int ofs, void (*usagefn)(errcxdef*));


/*
 *   Read a toggle argument.  prv is the previous value (prior to this
 *   switch) of the parameter (TRUE or FALSE).  argp is a pointer to the
 *   current argument word.  ofs is the length of this option flag, NOT
 *   including the '-'; most options have ofs==1 since they are of the
 *   form '-x'.  If the option is followed by '+', the value returned is
 *   TRUE; if it's followed by '-', the value is FALSE; if followed by
 *   nothing, the option is the logical inverse of the previous value.  If
 *   it's followed by any other character, we call the usage callback,
 *   which is not expected to return, but should signal an error. 
 */
int cmdtog(struct errcxdef *ec, int prv, char *argp, int ofs,
           void (*usagefn)(errcxdef*));

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk

#endif
