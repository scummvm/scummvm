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
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "yslib.h"

namespace Saga {

int ys_get_filesize(FILE * file_p, unsigned long *len_p, int *p_errno)
/*--------------------------------------------------------------------------*\
 * Returns the 'size' of the specified file. The file must be opened in 
 *  binary mode. Note that not all operating systems support determing the 
 *  exact end of a binary file stream, so this function is limited in 
 *  portability.
\*--------------------------------------------------------------------------*/
{
	long f_pos;
	long f_len;

	f_pos = ftell(file_p);
	if (f_pos == -1) {
		*p_errno = errno;
		return YS_E_FAILURE;
	}

	fseek(file_p, 0, SEEK_END);

	f_len = ftell(file_p);
	if (f_pos == -1) {
		*p_errno = errno;
		return YS_E_FAILURE;
	}

	fseek(file_p, f_pos, SEEK_SET);

	*len_p = (unsigned long)f_len;

	return YS_E_SUCCESS;;
}

} // End of namespace Saga
