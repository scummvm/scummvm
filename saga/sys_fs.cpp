/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
#include "reinherit.h"

namespace Saga {

int
SYSFS_GetFQFN(const char *f_dir, const char *f_name, char *buf, size_t buf_len)
{
	size_t f_dir_len;
	size_t f_name_len;
	char sep_buf[2] = { R_DIRECTORY_SEP };

	size_t i;

	if ((f_dir == NULL) || (f_name == NULL) || (buf == NULL)) {

		return R_FAILURE;
	}

	f_dir_len = strlen(f_dir);
	f_name_len = strlen(f_name);

	buf[0] = 0;

	strncat(buf, f_dir, buf_len);

#if R_DIRECTORY_SEP != '/'
	/* Convert frontslashes to backslashes */
	for (i = 0; i < f_dir_len; i++) {
		if (buf[i] == '/') {
			buf[i] = R_DIRECTORY_SEP;
		}
	}
#endif

	for (i = f_dir_len - 1; i > 0; i--) {

		/* Remove any trailing whitespace */
		if (isspace(buf[i])) {
			buf[i] = 0;
		} else {
			break;
		}
	}

	f_dir_len = strlen(buf);

	if (buf[f_dir_len - 1] != R_DIRECTORY_SEP) {

		/* Append a proper directory separator if req. */
		strncat(buf, sep_buf, buf_len);
	}

	strncat(buf, f_name, buf_len);

	return R_SUCCESS;
}

} // End of namespace Saga


