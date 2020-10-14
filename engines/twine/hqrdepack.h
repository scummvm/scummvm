/** @file hqrdepack.h
	@brief
	This file contains High Quality Resource (HQR) decompress routines.

	TwinEngine: a Little Big Adventure engine

	Copyright (C) 2013 The TwinEngine team
	Copyright (C) 2008-2013 Prequengine team
	Copyright (C) 2002-2007 The TwinEngine team

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef HQRDEPACK_H
#define HQRDEPACK_H

#include "sys.h"

/** Get a HQR entry pointer
	@param ptr pointer to save the entry
	@param filename HQR file name
	@param index entry index to extract
	@return entry real size */
int32 hqrGetEntry(uint8 * ptr, int8 *filename, int32 index);

/** Get a HQR entry pointer
	@param filename HQR file name
	@param index entry index to extract
	@return entry real size */
int32 hqrEntrySize(int8 *filename, int32 index);

/** Get a HQR total number of entries
	@param filename HQR file name
	@return total number of entries */
int32 hqrNumEntries(int8 *filename);

/** Get a HQR entry pointer with memory allocation
	@param ptr pointer to save the entry
	@param filename HQR file name
	@param index entry index to extract
	@return entry real size */
int32 hqrGetallocEntry(uint8 ** ptr, int8 *filename, int32 index);

int32 hqrGetVoxEntry(uint8 * ptr, int8 *filename, int32 index, int32 hiddenIndex);
int32 hqrGetallocVoxEntry(uint8 ** ptr, int8 *filename, int32 index, int32 hiddenIndex);

#endif
