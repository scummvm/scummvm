/** @file filereader.h
	@brief
	This file contains file read routines

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

#ifndef FILEREADER_H
#define FILEREADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sys.h"

/** Number of sector in the buffer */
#define SECTORS_IN_BUFFER (3)
/** Buffer size */
#define BUFFER_SIZE (2048*SECTORS_IN_BUFFER)

/** File reader structure */
typedef struct FileReader {
	/** File descriptor */
	FILE* fd;
	/** Content buffer */
	uint8 buffer[BUFFER_SIZE];
	/** Current position in the buffer */
	uint32 bufferPos;
	/** Current sector in the buffer */
	uint32 currSector;
} FileReader;

/** Feed buffer from file
	@param fr FileReader pointer */
void frfeed(FileReader* fr);

/** Read file
	@param fr FileReader pointer
	@param destPtr content destination pointer
	@param size size of read characters */
void frread(FileReader* fr, void* destPtr, uint32 size);

/** Seek file
	@param fr FileReader pointer
	@param seekPosition position to seek */
void frseek(FileReader* fr, uint32 seekPosition);

/** Open file
	@param fr FileReader pointer
	@param filename file path
	@return true if file open and false if error occurred */
int32 fropen2(FileReader* fr, char* filename, const char* mode);

/** Write file
	@param fr FileReader pointer
	@param destPtr content destination pointer
	@param size size of read characters */
void frwrite(FileReader* fr, void* destPtr, uint32 size, uint32 count);

/** Close file
	@param fr FileReader pointer */
void frclose(FileReader* fr);

#endif
