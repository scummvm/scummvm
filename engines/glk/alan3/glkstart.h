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

#ifndef GLK_ALAN3_GLKSTART
#define GLK_ALAN3_GLKSTART

/* glkstart.h: Unix-specific header file for GlkTerm, CheapGlk, and XGlk
        (Unix implementations of the Glk API).
    Designed by Andrew Plotkin <erkyrath@netcom.com>
    http://www.eblong.com/zarf/glk/index.html
*/

/* This header defines an interface that must be used by program linked
    with the various Unix Glk libraries -- at least, the three I wrote.
    (I encourage anyone writing a Unix Glk library to use this interface,
    but it's not part of the Glk spec.)
    
    Because Glk is *almost* perfectly portable, this interface *almost*
    doesn't have to exist. In practice, it's small.
*/

namespace Glk {
namespace Alan3 {

/* We define our own TRUE and FALSE and NULL, because ANSI
    is a strange world. */
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef NULL
#define NULL 0
#endif

#define glkunix_arg_End (0)
#define glkunix_arg_ValueFollows (1)
#define glkunix_arg_NoValue (2)
#define glkunix_arg_ValueCanFollow (3)
#define glkunix_arg_NumberValue (4)

struct glkunix_argumentlist_t {
    const char *name;
    int argtype;
	const char *desc;
};

struct glkunix_startup_t {
    int argc;
    char **argv;
};

/* The list of command-line arguments; this should be defined in your code. */
extern const glkunix_argumentlist_t glkunix_arguments[];

/* The external function; this should be defined in your code. */
extern int glkunix_startup_code(glkunix_startup_t *data);

} // End of namespace Alan3
} // End of namespace Glk

#endif
