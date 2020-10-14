/** @file filereader.cpp
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

#include "filereader.h"
#include "fcaseopen.h"
#include <ctype.h>

/** Feed buffer from file
	@param fr FileReader pointer */
void frfeed(FileReader* fr) {
	fread(fr->buffer, BUFFER_SIZE, 1, fr->fd);
	fr->bufferPos = 0;
}

/** Read file
	@param fr FileReader pointer
	@param destPtr content destination pointer
	@param size size of read characters */
void frread(FileReader* fr, void* destPtr, uint32 size) {
	if (BUFFER_SIZE - fr->bufferPos >= size) {
		memcpy(destPtr, &fr->buffer[fr->bufferPos], size);
		fr->bufferPos += size;
	} else {
		// feed what we can
		int8* tempPtr = (int8*)destPtr;
		memcpy(tempPtr, &fr->buffer[fr->bufferPos], BUFFER_SIZE - fr->bufferPos);
		tempPtr += BUFFER_SIZE - fr->bufferPos;
		size -= BUFFER_SIZE - fr->bufferPos;

		// feed the rest
		do {
			fr->currSector++;
			frfeed(fr);
			if (size >= BUFFER_SIZE) {
				memcpy(tempPtr, fr->buffer, BUFFER_SIZE);
				tempPtr += BUFFER_SIZE;
				size -= BUFFER_SIZE;
			} else {
				memcpy(tempPtr, fr->buffer, size);
				fr->bufferPos += size;
				size = 0;
			}
		} while (size > 0);
	}
}

/** Seek file
	@param fr FileReader pointer
	@param seekPosition position to seek */
void frseek(FileReader* fr, uint32 seekPosition) {
	uint32 sectorToSeek;

	sectorToSeek = seekPosition / 2048;

	fseek(fr->fd, sectorToSeek * 2048, SEEK_SET);

	fr->currSector = sectorToSeek;
	frfeed(fr);
	fr->bufferPos = (seekPosition - (sectorToSeek * 2048));
}

/** Open file
	@param fr FileReader pointer
	@param filename file path
	@return true if file open and false if error occurred */
int32 fropen2(FileReader* fr, char* filename, const char* mode) {
	fr->fd = fcaseopen(filename, mode);

	if (fr->fd) {
		fr->currSector = 0;
		frfeed(fr);
		return 1;
	}

	return 0;
}

/** Write file
	@param fr FileReader pointer
	@param destPtr content destination pointer
	@param size size of read characters */
void frwrite(FileReader* fr, void* destPtr, uint32 size, uint32 count) {
	fwrite(destPtr, size, count, fr->fd);
}

/** Close file
	@param fr FileReader pointer */
void frclose(FileReader* fr) {
	fclose(fr->fd);
}
