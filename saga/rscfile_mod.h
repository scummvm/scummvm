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
/*

 Description:   
 
    RSC Resource file management module public header file

 Notes: 
*/

#ifndef SAGA_RSCFILE_MOD_H__
#define SAGA_RSCFILE_MOD_H__

#include "resnames.h"

namespace Saga {

R_RSCFILE_CONTEXT *RSC_CreateContext(void);

int RSC_DestroyContext(R_RSCFILE_CONTEXT *);

int RSC_OpenContext(R_RSCFILE_CONTEXT *, const char *);

int RSC_CloseContext(R_RSCFILE_CONTEXT *);

uint32 RSC_GetResourceCount(R_RSCFILE_CONTEXT *);

int RSC_GetResourceSize(R_RSCFILE_CONTEXT *, uint32, uint32 *);

int RSC_GetResourceOffset(R_RSCFILE_CONTEXT *, uint32, uint32 *);

int RSC_LoadResource(R_RSCFILE_CONTEXT *, uint32, byte **, size_t *);

int RSC_FreeResource(byte *);

} // End of namespace Saga

#endif				/* SAGA_RSCFILE_MOD_H__ */
