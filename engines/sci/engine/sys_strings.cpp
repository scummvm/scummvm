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

#include "sci/tools.h"
#include "sci/include/sys_strings.h"
#include "sci/sci_memory.h"

namespace Sci {

void sys_string_acquire(sys_strings_t *strings, int index, const char *name, int max_len) {
	sys_string_t *str = strings->strings + index;

	if (index < 0 || index >= SYS_STRINGS_MAX) {
		fprintf(stderr, "[SYSSTR] Error: Attempt to acquire string #%d\n",
		        index);
		BREAKPOINT();
	}

	if (str->name
	        && (strcmp(name, str->name)
	            || (str->max_size != max_len))) {
		fprintf(stderr, "[SYSSTR] Error: Attempt to re-acquire existing string #%d;"
		        "was '%s', tried to claim as '%s'\n",
		        index, str->name, name);
		BREAKPOINT();
	}

	str->name = strdup(name);
	str->max_size = max_len;
	str->value = (char*)sci_malloc(max_len + 1);
	str->value[0] = 0; // Set to empty string
}

int sys_string_set(sys_strings_t *strings, int index, const char *value) {
	sys_string_t *str = strings->strings + index;

	if (index < 0 || index >= SYS_STRINGS_MAX || !str->name) {
		fprintf(stderr, "[SYSSTR] Error: Attempt to write to invalid/unused string #%d\n",
		        index);
		BREAKPOINT();
		return 1;
	}

	strncpy(str->value, value, str->max_size);
	str->value[str->max_size] = 0; // Make sure to terminate

	return 0;
}

void sys_string_free(sys_strings_t *strings, int index) {
	sys_string_t *str = strings->strings + index;

	free(str->name);
	str->name = NULL;

	free(str->value);
	str->value = NULL;

	str->max_size = 0;
}

void sys_string_free_all(sys_strings_t *strings) {
	int i;

	for (i = 0;i < SYS_STRINGS_MAX;i++) {
		if (strings->strings[i].name)
			sys_string_free(strings, i);
	}

}

void sys_strings_restore(sys_strings_t *new_strings, sys_strings_t *old_strings) {
	int i;

	// First, pad memory
	for (i = 0; i < SYS_STRINGS_MAX; i++) {
		sys_string_t *s = new_strings->strings + i;
		char *data = s->value;
		if (data) {
			s->value = (char *)sci_malloc(s->max_size + 1);
			strcpy(s->value, data);
			free(data);
		}
	}

	sys_string_set(new_strings, SYS_STRING_SAVEDIR, old_strings->strings[SYS_STRING_SAVEDIR].value);
}

} // End of namespace Sci
