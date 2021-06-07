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

#ifndef SAGA2_SAVEFILE_H
#define SAGA2_SAVEFILE_H

namespace Saga2 {

/* ===================================================================== *
   Constants
 * ===================================================================== */

const int16     saveNameSize = 40,
                fileNameSize = 260;

#ifdef FTA
const ChunkID   gameID = MKTAG('F', 'T', 'A', '2');
#else
const ChunkID   gameID = MKTAG('D', 'I', 'N', 'O');
#endif

/* ===================================================================== *
   SaveFileHeader class
 * ===================================================================== */

//	This structure represents the first 128 bytes written to a save game
//	file.  It stores the game ID and the long name of the saved game state.

struct SaveFileHeader {
	ChunkID     gameID;                     //  ID of game (FTA2 of DINO).
	char        saveName[saveNameSize];   //  The long name of the saved
	//  game state.
	int8        reserved[84];             //  Room for expansion.
};  //  128 bytes

/* ===================================================================== *
   SaveFileChunkInfo class
 * ===================================================================== */

//	This is a save file chunk header.  The save file will consist of as many
//	chunks as is needed.  The first chunk begins immediately after the
//	save file header.  Each chunk header has a four byte ID followed by
//	a chunk size.

struct SaveFileChunkInfo {
	ChunkID     id;                         //  4-byte chunk ID
	int32       size;                       //  Number of bytes in chunk
};  //  8 bytes

/* ===================================================================== *
   SaveFileConstructor class
 * ===================================================================== */

//	This class is used to define an object which can be passed to various
//	parts of the program in order to create a save game file.

class SaveFileConstructor {
	FILE    *fileHandle;                    //  A standard C file handle

	int32   chunkSize,                      //  Current chunk size
	        posInChunk;                     //  Current file position
	//  relative to the begining
	//  of the chunk data

public:
	//  Constructor
	SaveFileConstructor(int16 saveNo, char *saveName);

	//  Destructor
	~SaveFileConstructor(void);

	//  Create a new chunk without writing the chunk data.  NOTE: the
	//  size of a chunk must be known to call this function.
	bool newChunk(ChunkID id, int32 size);

	//  Write data to a chunk created with the newChunk() function.
	int32 write(void *buf, int32 size);

	//  Create a new chunk and write the chunk data.
	bool writeChunk(ChunkID id, void *buf, int32 size);
};

/* ===================================================================== *
   SaveFileReader class
 * ===================================================================== */

//	This class is used to define an object which can be passed to various
//	parts of the program in order to read data from a save game file.

class SaveFileReader {
	FILE            *fileHandle;            //  A standard C file handle

	int32           chunkSize,              //  Current chunk size
	                posInChunk;             //  Current file position
	//  relative to the begining
	//  of the chunk data.
	SaveFileHeader  header;                 //  The save file header.

public:
	//  Constructor
	SaveFileReader(int16 saveNo);

	//  Destructor
	~SaveFileReader(void);

	//  Return a pointer to the long name of the save game state
	char *saveName(void) {
		return header.saveName;
	}

	//  Return the number of bytes in the current chunk
	int32 getChunkSize(void) {
		return chunkSize;
	}

	//  Set the first chunk to be the current chunk
	bool firstChunk(ChunkID &chunk, int32 &size);

	//  Make the next chunk the current chunk
	bool nextChunk(ChunkID &chunk, int32 &size);

	//  Read data from the current chunk
	int32 read(void *buf, int32 size);

	//  Return the number of bytes left to be read from the
	//  current chunk
	int32 bytesLeftInChunk(void) {
		return chunkSize - posInChunk;
	}
};

} // end of namespace Saga2

#endif
