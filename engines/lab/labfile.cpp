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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#include "lab/labfun.h"
#include "lab/mouse.h"
#include "common/file.h"

namespace Lab {

#define MAXREADSIZE 30720L
/* NOTE: set to 0 for non-CDTV machines.          */



static byte *buffer = NULL, *realbufferstart = NULL, *startoffilestorage = NULL;

byte **startoffile = &startoffilestorage;
static uint32 buffersize, realbuffersize;

int32 ReadSoFar;

/*****************************************************************************/
/* Returns the size of a file.                                               */
/*****************************************************************************/
uint32 sizeOfFile(const char *name) {
	Common::File file;

	file.open(translateFileName(name));
	if (!file.isOpen()) {
		warning("Cannot open file %s", translateFileName(name));

		return 0;
	}
	uint32 size = file.size();
	file.close();

	return size;
}




/*-------------------- Routines that buffer a whole file --------------------*/



/*----- divides up and manages the buffer -----*/



#define MAXMARKERS        15



struct FileMarker {
	char name[32];
	void *Start, *End;
};



static FileMarker FileMarkers[MAXMARKERS];
static uint16 CurMarker  = 0;
static void *MemPlace   = NULL;





/*****************************************************************************/
/* Frees a File's resources.                                                 */
/*****************************************************************************/
static void freeFile(uint16 RMarker) {
	FileMarkers[RMarker].name[0] = 0;
	FileMarkers[RMarker].Start  = NULL;
	FileMarkers[RMarker].End    = NULL;
}





/*****************************************************************************/
/* Gets a chunk of memory from the buffer.                                   */
/*****************************************************************************/
static void *getCurMemLabFile(uint32 size) {
	void *ptr = 0;

	if ((((char *) MemPlace) + size - 1) >=
	        (((char *) buffer) + buffersize))
		MemPlace = buffer;

	ptr = MemPlace;
	MemPlace = (char *)MemPlace + size;

	for (int counter = 0; counter < MAXMARKERS; counter++) {
		if (FileMarkers[counter].name[0]) {
			if (((FileMarkers[counter].Start >= ptr) &&
			        (FileMarkers[counter].Start < MemPlace))  ||
			        ((FileMarkers[counter].End >= ptr) &&
			         (FileMarkers[counter].End < MemPlace))    ||
			        ((ptr >= FileMarkers[counter].Start) &&
			         (ptr <= FileMarkers[counter].End)))

				freeFile(counter);
		}
	}

	return ptr;
}




/*****************************************************************************/
/* Checks if a file is already buffered.                                     */
/*****************************************************************************/
byte **isBuffered(const char *fileName) {
	if (fileName == NULL)
		return NULL;

	for (int i = 0; i < MAXMARKERS; i++) {
		if (strcmp(FileMarkers[i].name, fileName) == 0) {
			*startoffile = (byte *)FileMarkers[i].Start;
			return startoffile;
		}
	}

	return NULL;
}




/*****************************************************************************/
/* Grabs a chunk of memory from the room buffer, and manages it for a        */
/* particular room. If it returns true, then the file is already in memory. */
/*****************************************************************************/
bool allocFile(void **Ptr, uint32 Size, const char *fileName) {
	uint16 RMarker;
	byte **temp;

	if (1 & Size)  /* Memory is required to be even aligned */
		Size++;

	temp = isBuffered(fileName);

	if (temp) {
		*Ptr = *temp;
		return true;
	}

	RMarker = CurMarker;
	CurMarker++;

	if (CurMarker >= MAXMARKERS)
		CurMarker = 0;

	freeFile(RMarker);
	strcpy(FileMarkers[RMarker].name, fileName);

	*Ptr = getCurMemLabFile(Size);
	FileMarkers[RMarker].Start = *Ptr;
	FileMarkers[RMarker].End   = (void *)(((char *)(*Ptr)) + Size - 1);

	return false;
}





/*----- Main routines -----*/


/*****************************************************************************/
/* Reads a file into memory.                                                 */
/*****************************************************************************/
byte **openFile(const char *name, uint32 &size) {
	byte *buf;

	Common::File file;

	file.open(translateFileName(name));
	if (!file.isOpen()) {
		warning("Cannot open file %s", translateFileName(name));

		return NULL;
	}

	size = file.size();

	buf = (byte *)malloc(size);

	if (!buf) {
		error("Unable to allocate %d bytes file file %s", size, name);
	}

	*startoffile = buf;

	file.read(buf, size);

	return startoffile;
}



/*****************************************************************************/
/* Reads a block of memory.                                                  */
/*****************************************************************************/
void readBlock(void *Buffer, uint32 Size, byte **File) {
	memcpy(Buffer, *File, (size_t) Size);
	(*File) += Size;
}



/*****************************************************************************/
/* Reads on character.                                                       */
/*****************************************************************************/
char readChar(char **File) {
	char c = **File;

	(*File)++;
	return c;
}




/*****************************************************************************/
/* Skips a chunk of memory.                                                  */
/*****************************************************************************/
void skip(byte **File, uint32 skip) {
	(*File) += skip;
}



/*****************************************************************************/
/* Resets the internal buffers to empty.                                     */
/*****************************************************************************/
void resetBuffer() {
	uint16 RMarker;

	CurMarker = 0;
	RMarker   = 0;
	MemPlace  = buffer;

	while (RMarker < MAXMARKERS) {
		freeFile(RMarker);
		RMarker++;
	}
}



/*****************************************************************************/
/* Initializes the buffer.                                                   */
/*****************************************************************************/
bool initBuffer(uint32 BufSize, bool IsGraphicsMem) {
	buffer = (byte *)calloc(BufSize, 1);

	buffersize = BufSize;
	realbuffersize = buffersize;
	realbufferstart = buffer;

	resetBuffer();

	return (buffer != NULL);
}




/*****************************************************************************/
/* Frees the buffer.                                                         */
/*****************************************************************************/
void freeBuffer() {
	freeAllStolenMem();

	if (buffer)
		free(buffer);
}




/*------------------------------------------------------------------------*/
/* The following routines allow stealing of memory from the buffer (which */
/* later may or may not be given back).                                   */
/*------------------------------------------------------------------------*/




/*****************************************************************************/
/* Clears all the buffers.                                                   */
/*****************************************************************************/
static void flushBuffers() {
	for (int i = 0; i < MAXMARKERS; i++)
		freeFile(i);
}



/*****************************************************************************/
/* Steal some memory from the buffer                                         */
/*****************************************************************************/
void *stealBufMem(int32 Size) {
	void *Mem;

	flushBuffers();
	Mem = buffer;

	buffer += Size;
	buffersize -= Size;
	MemPlace = buffer;

	return Mem;
}


Common::File *openPartial(const char *name) {
	Common::File *f;

	f = new Common::File();
	f->open(translateFileName(name));

	if (!f->isOpen()) {
		delete f;
		return 0;
	}

	return f;
}


/*****************************************************************************/
/* Frees all the memory stolen from the buffer.                              */
/*****************************************************************************/
void freeAllStolenMem() {
	flushBuffers();

	buffer = realbufferstart;
	buffersize = realbuffersize;
	MemPlace = buffer;
}

} // End of namespace Lab
