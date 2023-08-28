/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/***************************************************************************/
/*                                                                         */
/*  ftutil.c                                                               */
/*                                                                         */
/*    FreeType utility file for memory and list management (body).         */
/*                                                                         */
/*  Copyright 2002 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

#include <ft2build.h>
#include "engines/ags/lib/freetype-2.1.3/fterrors.h"
#include "engines/ags/lib/freetype-2.1.3/ftmemory.h"

#include "common/debug.h"


namespace AGS3 {
namespace FreeType213 {


/**** MEMORY  MANAGEMENT ****/

FT_Error FT_Alloc(FT_Memory memory, FT_Long size, void **P) {
	assert(P != 0);

	if (size > 0) {
		*P = memory->alloc(memory, size);
		if (!*P) {
			warning("FT_Alloc: Out of memory? (%ld requested)", size);
			return FT_Err_Out_Of_Memory;
		}
		FT_MEM_ZERO(*P, size);
	} else
		*P = NULL;

	return FT_Err_Ok;
}

FT_Error FT_Realloc(FT_Memory memory, FT_Long current, FT_Long size, void **P) {
	void *Q;

	assert(P != 0);

	/* if the original pointer is NULL, call FT_Alloc() */
	if (!*P)
		return FT_Alloc(memory, size, P);

	/* if the new block if zero-sized, clear the current one */
	if (size <= 0) {
		FT_Free(memory, P);
		return FT_Err_Ok;
	}

	Q = memory->realloc(memory, current, size, *P);
	if (!Q)
		goto Fail;

	if (size > current)
		FT_MEM_ZERO((char *)Q + current, size - current);

	*P = Q;
	return FT_Err_Ok;

Fail:
	warning("FT_Realloc: Failed (current %ld, requested %ld)", current, size);
	return FT_Err_Out_Of_Memory;
}

void FT_Free(FT_Memory memory, void **P) {
	if (P && *P) {
		memory->free(memory, *P);
		*P = 0;
	}
}


} // End of namespace FreeType213
} // End of namespace AGS3
