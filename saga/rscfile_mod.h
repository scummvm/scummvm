/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005 The ScummVM project
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

// RSC Resource file management module public header file

#ifndef SAGA_RSCFILE_MOD_H__
#define SAGA_RSCFILE_MOD_H__

#include "saga/resnames.h"

namespace Saga {

RSCFILE_CONTEXT *RSC_CreateContext();
int RSC_DestroyContext(RSCFILE_CONTEXT *);
int RSC_OpenContext(RSCFILE_CONTEXT *, const char *);
int RSC_CloseContext(RSCFILE_CONTEXT *);
uint32 RSC_GetResourceCount(RSCFILE_CONTEXT *);
int RSC_GetResourceSize(RSCFILE_CONTEXT *, uint32, uint32 *);
int RSC_GetResourceOffset(RSCFILE_CONTEXT *, uint32, uint32 *);
int RSC_LoadResource(RSCFILE_CONTEXT *, uint32, byte **, size_t *);
int RSC_FreeResource(byte *);
const char *RSC_FileName(RSCFILE_CONTEXT *rsc);
int RSC_ConvertID(int id);

} // End of namespace Saga

#endif
