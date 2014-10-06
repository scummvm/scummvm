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
#include "lab/storage.h"
#include "common/file.h"

namespace Lab {

#define MAXREADSIZE 30720L
#define DMABUGSIZE  0     /* The number of bytes the DMA driver might screw */
/* NOTE: set to 0 for non-CDTV machines.          */



static byte *buffer = NULL, *realbufferstart = NULL, *startoffilestorage = NULL;

byte **startoffile = &startoffilestorage;
static uint32 buffersize, realbuffersize;

uint32 ReadSoFar;
bool ReadIsError, ReadIsDone;


#if defined(WIN32)
#define stat _stat
#endif

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



typedef struct {
	char name[32];
	void *Start, *End;
} FileMarker;



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
static void *getCurMem(uint32 size) {
	void *ptr = 0;

	warning ("STUB: getCurMem(%d)", size);

#if 0
	if ((((char *) MemPlace) + size - 1) >=
	        (((char *) buffer) + buffersize))
		MemPlace = buffer;

	ptr = MemPlace;
	((char *)MemPlace) += size;

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
#endif
	return ptr;
}




/*****************************************************************************/
/* Checks if a file is already buffered.                                     */
/*****************************************************************************/
byte **isBuffered(const char *fileName) {
	uint16 RMarker;

	if (fileName == NULL)
		return NULL;

	RMarker = 0;

	while (RMarker < MAXMARKERS) {
		if (strcmp(FileMarkers[RMarker].name, fileName) == 0) {
			*startoffile = (byte *) FileMarkers[RMarker].Start;
			return startoffile;
		} else
			RMarker++;
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

	Size += DMABUGSIZE;

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

	*Ptr = getCurMem(Size);
	FileMarkers[RMarker].Start = *Ptr;
	FileMarkers[RMarker].End   = (void *)(((char *)(*Ptr)) + Size - 1);

	return false;
}





/*----- Main routines -----*/


/*****************************************************************************/
/* Reads a file into memory.                                                 */
/*****************************************************************************/
byte **openFile(const char *name) {
	warning("STUB: openFile");

	return NULL;
#if 0
	char *tempbuffer, *Buffer;
	int32 Size, Left;
	int FPtr, ReadSize, myread;

	ReadSoFar   = 0L;
	ReadIsError = false;
	ReadIsDone  = false;

	if ((buffer == NULL) || (name == NULL)) {
		ReadIsError = true;
		ReadIsDone  = true;
		return NULL;
	}

	Size = sizeOfFile(name);

	if (!Size || (Size > ((int32) buffersize))) {
		ReadIsError = true;
		ReadIsDone  = true;
		return NULL;
	}

	if (allocFile((void **) &Buffer, Size, name)) { /* Get place in Buffer     */
		*startoffile = Buffer;                        /* If the file is buffered */

		ReadSoFar   = Size;
		ReadIsError = false;
		ReadIsDone  = true;

		return startoffile;
	}

#if defined(WIN32)
#if defined(DEMODATA)
	{
		FILE *fh = fopen("c:\\depot\\labyrinth\\demodata.log", "a+w");
		fprintf(fh, "%s\n", name);
		fclose(fh);
	}
#endif

	FPtr = open(translateFileName(name), O_RDONLY | O_BINARY);
#else
	FPtr = open(translateFileName(name), O_RDONLY);
#endif

	if (FPtr != -1) {
		Left = Size;
		tempbuffer = Buffer;
		*startoffile = Buffer;

		while (Left) {
			fileCheckMusic(Left);

			if (Left > MAXREADSIZE)
				ReadSize = MAXREADSIZE;
			else
				ReadSize = (int) Left;

			if (!(myread = read(FPtr, tempbuffer, ReadSize + DMABUGSIZE))) {
				ReadIsError = false;
				ReadIsDone  = true;

				close(FPtr);
				return NULL;
			}

			/* Not necessary for IBM version
			      if ((ReadSize == MAXREADSIZE) && DMABUGSIZE)
			        Seek(FPtr, -DMABUGSIZE, (int32) OFFSET_CURRENT);
			 */

			ReadSoFar  += ReadSize;
			tempbuffer += ReadSize;
			Left       -= ReadSize;
		}

		ReadIsDone = true;
		close(FPtr);
		return startoffile;
	} else {
		ReadIsError = false;
		ReadIsDone  = true;

		return NULL;
	}
#endif
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
void resetBuffer(void) {
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
	if (!allocate((void **) &buffer, BufSize))
		buffer = NULL;

	buffersize = BufSize;
	realbuffersize = buffersize;
	realbufferstart = buffer;

	resetBuffer();

	return (buffer != NULL);
}




/*****************************************************************************/
/* Frees the buffer.                                                         */
/*****************************************************************************/
void freeBuffer(void) {
	freeAllStolenMem();

	if (buffer)
		deallocate(buffer, buffersize);
}




/*------------------------------------------------------------------------*/
/* The following routines allow stealing of memory from the buffer (which */
/* later may or may not be given back).                                   */
/*------------------------------------------------------------------------*/




/*****************************************************************************/
/* Clears all the buffers.                                                   */
/*****************************************************************************/
static void flushBuffers(void) {
	uint16 RMarker;

	RMarker = 0;

	while (RMarker < MAXMARKERS) {
		freeFile(RMarker);
		RMarker++;
	}
}



/*****************************************************************************/
/* Steal some memory from the buffer                                         */
/*****************************************************************************/
void *stealBufMem(int32 Size) {
	void *Mem;

	Size += DMABUGSIZE;

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
void freeAllStolenMem(void) {
	flushBuffers();

	buffer = realbufferstart;
	buffersize = realbuffersize;
	MemPlace = buffer;
}




/*--------------- Routines that read partial chunks of a file ---------------*/



/*****************************************************************************/
/* Reads Size amount of bytes into buffer.                                   */
/*****************************************************************************/
uint32 readPartial(int32 File, void *buf, uint32 Size) {
	uint32 haveread = 0;

	warning("STUB: readPartial");

#if 0
	uint32 nsize;

	while (Size) {
		if (Size > MAXREADSIZE)
			nsize = MAXREADSIZE;
		else
			nsize = Size;

		haveread += (uint32)(read((int) File, buf, (int) nsize));
		Size -= nsize;
		((char *) buf) += nsize;
		updateMouse();
	}
#endif
	return haveread;
}



/*****************************************************************************/
/* Reads Size amount of bytes into buffer.  Use this one if the data must    */
/* absolutely be correct (compressed data for example).  Otherwise, because  */
/* of the DMA bug, last two bytes may be screwed.                            */
/*****************************************************************************/
uint32 newReadPartial(int32 File, void *buf, uint32 Size) {
	return readPartial((int) File, buf, Size);
}



/*****************************************************************************/
/* Reads Size amount of bytes into buffer.  Use this one if the data must    */
/* absolutely be correct (compressed data for example).  Otherwise, because  */
/* of the DMA bug, last two bytes may be screwed.  This one will work if the */
/* data is not padded the extra two bytes.                                   */
/*****************************************************************************/
uint32 bufferedReadPartial(int32 File, void *buf, uint32 Size) {
	return readPartial(File, buf, Size);
}



/*****************************************************************************/
/* Sets the current position in the file relative to the beginning of the    */
/* file.                                                                     */
/*****************************************************************************/
void setPos(int32 File, uint32 Place) {
	warning("STUB: setPos");
	//lseek((int) File, (int32) Place, SEEK_SET);
}



/*****************************************************************************/
/* Skips a certain number of bytes either forward or backwards.              */
/*****************************************************************************/
void skipPartial(int32 File, int32 Skip) {
	warning("STUB: skipPartial");

	//lseek((int) File, Skip, SEEK_CUR);
}

} // End of namespace Lab
