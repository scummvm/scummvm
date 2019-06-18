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

#ifndef GLK_ALAN2_GLK_START
#define GLK_ALAN2_GLK_START

/* This header defines an interface that must be used by program linked
    with the various Unix Glk libraries -- at least, the three I wrote.
    (I encourage anyone writing a Unix Glk library to use this interface,
    but it's not part of the Glk spec.)
    
    Because Glk is *almost* perfectly portable, this interface *almost*
    doesn't have to exist. In practice, it's small.
*/

namespace Glk {
namespace Alan2 {

#define glkunix_arg_End (0)
#define glkunix_arg_ValueFollows (1)
#define glkunix_arg_NoValue (2)
#define glkunix_arg_ValueCanFollow (3)
#define glkunix_arg_NumberValue (4)

typedef struct glkunix_argumentlist_struct {
    char *name;
    int argtype;
    char *desc;
} glkunix_argumentlist_t;

typedef struct glkunix_startup_struct {
    int argc;
    char **argv;
} glkunix_startup_t;

/* The list of command-line arguments; this should be defined in your code. */
extern glkunix_argumentlist_t glkunix_arguments[];

/* The external function; this should be defined in your code. */
extern bool glkunix_startup_code(glkunix_startup_t *data);

/* Some helpful utility functions which the library makes available
   to your code. Obviously, this is nonportable; so you should
   only call it from glkunix_startup_code().
*/
extern strid_t glkunix_stream_open_pathname(char *pathname, uint textmode, uint rock);

} // End of namespace Alan2
} // End of namespace Glk

#endif
