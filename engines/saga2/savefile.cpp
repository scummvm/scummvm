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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#define FORBIDDEN_SYMBOL_ALLOW_ALL // FIXME: Remove

#include "saga2/saga2.h"
#include "saga2/savefile.h"

namespace Saga2 {

/* ===================================================================== *
   Functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Construct a file name string for a save file based on the save
//	number

void getSaveFileName(int16 saveNo, char *fileName) {
	sprintf(fileName, "%3.3d.SAV", saveNo);
}

/* ===================================================================== *
   SaveFileConstructor member functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Constructor

SaveFileConstructor::SaveFileConstructor(int16 saveNo, char *saveName) {
	char    fileName[fileNameSize];

	//  Construct the file name string
	getSaveFileName(saveNo, fileName);

	//  Open the file.  Throw an exception if the file cannot be opened
	if ((fileHandle = fopen(fileName, "wb")) == NULL)
		error("Cannot create save game file \"%s\"", fileName);

	SaveFileHeader  header;

	//  Initialize the file header
	memset(&header, '\0', sizeof(header));

	header.gameID = gameID;
	strncpy(header.saveName, saveName, sizeof(header.saveName) - 1);

	//  Write the header
	if (fwrite(&header, sizeof(header), 1, fileHandle) != 1)
		error("Error writing save game header: \"%s\"", fileName);

	chunkSize = posInChunk = 0;
}

//----------------------------------------------------------------------
//	Destructor

SaveFileConstructor::~SaveFileConstructor(void) {
	//  Simply close the file
	if (fileHandle != NULL) fclose(fileHandle);
}

//----------------------------------------------------------------------
//	Create a new chunk without writing the chunk data.  NOTE: the size
//	of a chunk must be known to call this function.

bool SaveFileConstructor::newChunk(ChunkID id, int32 size) {
	//  Determine if file position is at end of previous chunk
	if (posInChunk < chunkSize) return false;

	assert(posInChunk == chunkSize);

	SaveFileChunkInfo   chunkHeader;

	//  Initialize the chunk header
	chunkHeader.id = id;
	chunkHeader.size = size;

	//  Write the chunk header
	if (fwrite(&chunkHeader, sizeof(chunkHeader), 1, fileHandle) != 1)
		error("Error writing save game chunk header");

	//  Set the chunk variables
	chunkSize = size;
	posInChunk = 0;

	return true;
}

//----------------------------------------------------------------------
//	Write data to a chunk created with the newChunk() function.

int32 SaveFileConstructor::write(void *buf, int32 size) {
	//  If asking to write more data than the chunk size, adjust
	//  the number of bytes to write
	if (size > chunkSize - posInChunk) size = chunkSize - posInChunk;

	//  Write data
	if (size > 0 && fwrite(buf, size, 1, fileHandle) != 1)
		error("Error writing save game data");

	//  Adjust the position in chunk
	posInChunk += size;

	//  Return the number of bytes written
	return size;
}

//----------------------------------------------------------------------
//	Create a new chunk and write the chunk data.

bool SaveFileConstructor::writeChunk(ChunkID id, void *buf, int32 size) {
	//  Determine if file position is at end of previous chunk
	if (posInChunk < chunkSize) return false;

	assert(posInChunk == chunkSize);

	SaveFileChunkInfo   chunkHeader;

	//  Initialize the chunk header
	chunkHeader.id = id;
	chunkHeader.size = size;

	//  Write the chunk header
	if (fwrite(&chunkHeader, sizeof(chunkHeader), 1, fileHandle) != 1)
		error("Error writing save game chunk header");

	//  Write the chunk data
	if (size > 0 && fwrite(buf, size, 1, fileHandle) != 1)
		error("Error writing save game data");

	//  Initialize the chunk varibles to indicate the file position is
	//  at the end of the chunk
	chunkSize = posInChunk = size;

	//  Return success
	return true;
}

/* ===================================================================== *
   SaveFileReader member functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Constructor

SaveFileReader::SaveFileReader(int16 saveNo) {
	char    fileName[fileNameSize];

	//  Construct the file name based on the save number
	getSaveFileName(saveNo, fileName);

	//  Open the file or throw an exception
	if ((fileHandle = fopen(fileName, "rb")) == NULL)
		error("Cannot open save game file \"%s\"", fileName);

	//  Read the save file header
	if (fread(&header, sizeof(header), 1, fileHandle) != 1)
		error("Error reading save game header: \"%s\"", fileName);

	//  Initialize the chunk variables
	chunkSize = posInChunk = 0;
}

//----------------------------------------------------------------------
//	Destructor

SaveFileReader::~SaveFileReader(void) {
	//  Close the file
	if (fileHandle != NULL) fclose(fileHandle);
}

//----------------------------------------------------------------------
//	Set the first chunk to be the current chunk

bool SaveFileReader::firstChunk(ChunkID &chunk, int32 &size) {
	SaveFileChunkInfo   chunkHeader;

	//  Seek the first chunk header
	if (fseek(fileHandle, sizeof(SaveFileHeader), SEEK_SET) != 0)
		error("Error seeking first save game chunk");

	//  Read the chunk header
	if (fread(&chunkHeader, sizeof(chunkHeader), 1, fileHandle) != 1)
		return false;

	//  Initialize the chunk variables
	chunkSize = chunkHeader.size;
	posInChunk = 0;

	//  Return the chunk ID and chunk size
	chunk = chunkHeader.id;
	size = chunkHeader.size;

	//  Return success
	return true;
}

//----------------------------------------------------------------------
//	Make the next chunk the current chunk

bool SaveFileReader::nextChunk(ChunkID &chunk, int32 &size) {
	assert(posInChunk <= chunkSize);

	//  If not already at the beginning of the next chunk header, seek
	//  the next chunk
	if (posInChunk < chunkSize) {
		if (fseek(fileHandle, chunkSize - posInChunk, SEEK_CUR) != 0)
			error("Error seeking next save game chunk");
	}

	SaveFileChunkInfo   chunkHeader;

	//  Read the chunk header
	if (fread(&chunkHeader, sizeof(chunkHeader), 1, fileHandle) != 1)
		return false;

	//  Initialize the chunk variables
	chunkSize = chunkHeader.size;
	posInChunk = 0;

	//  Return the chunk ID and chunk size
	chunk = chunkHeader.id;
	size = chunkHeader.size;

	//  Return success
	return true;
}

//----------------------------------------------------------------------
//	Read data from the current chunk

int32 SaveFileReader::read(void *buf, int32 size) {
	//  If asking for more data than is left in the chunk adjust the
	//  number of bytes to read
	if (size > chunkSize - posInChunk) size = chunkSize - posInChunk;

	//  Read the chunk data
	if (size > 0 && fread(buf, size, 1, fileHandle) != 1)
		error("Error reading save game data");

	//  Adjust the position in chunk
	posInChunk += size;

	//  Return the number of bytes read
	return size;
}

} // end if namespace Saga2
