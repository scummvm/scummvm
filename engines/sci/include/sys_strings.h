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

#ifndef SCI_SYS_STRINGS_H
#define SCI_SYS_STRINGS_H

namespace Sci {

enum {
	SYS_STRINGS_MAX = 4,

	SYS_STRING_SAVEDIR = 0,
	SYS_STRING_PARSER_BASE = 1,

	MAX_PARSER_BASE = 64
};

struct sys_string_t {
	char *name;
	int max_size;
	char *value;
};

struct sys_strings_t {
	sys_string_t strings[SYS_STRINGS_MAX];
};

void sys_string_acquire(sys_strings_t *strings, int index, const char *name, int max_len);
/* Reserves a new system string
** Parameters: (sys_strings_t *) strings: The string table to reserve in
**             (int) index: Index number to reserve
**             (const char *) name: Name the entry should be tagged with
**             (int) max_len: Maximum string length in bytes
*/

int sys_string_set(sys_strings_t *strings, int index, const char *value);
/* Sets the value of a system string
** Parameters: (sys_strings_t *) strings: The string table to use
**             (int) index: Index of the string to write to
**             (const char *) value: The value to copy
** Returns   : 0 on success, 1 on error
** Length clipping is performed.
*/

void sys_strings_restore(sys_strings_t *new_strings, sys_strings_t *old_strings);
/* Cleanup system strings after a gamestate restore
** Parameters: (sys_strings_t *) The freshly loaded system strings to clean up
**             (sys_strings_t *) The old system strings to clean up
*/

void sys_string_free_all(sys_strings_t *strings);
/* Deallocates all allocated system strings
** Parameters: (sys_strings_t *) strings: The string table to deallocate
*/

} // End of namespace Sci

#endif // SCI_SYS_STRINGS_H
