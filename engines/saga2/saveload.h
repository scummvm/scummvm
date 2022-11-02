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
 * aint32 with this program; if not, write to the Free Software
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_LOADSAVE_H
#define SAGA2_LOADSAVE_H

#include "common/memstream.h"

namespace Saga2 {

#define CHUNK_BEGIN Common::MemoryWriteStreamDynamic *out = new Common::MemoryWriteStreamDynamic(DisposeAfterUse::YES)

#define CHUNK_END outS->writeUint32LE(out->pos()); \
	outS->write(out->getData(), out->pos()); \
	delete out

/* ===================================================================== *
   SaveFileHeader class
 * ===================================================================== */

//	This structure represents the first 128 bytes written to a save game
//	file.  It stores the game ID and the long name of the saved game state.

struct SaveFileHeader {
	enum {
		kSaveNameSize = 40,
		kHeaderSize = 128
	};

	ChunkID gameID;                     //  ID of game (FTA2 or DINO)
	Common::String saveName;            //  The long name of the saved

	void read(Common::InSaveFile *in);
	void write(Common::OutSaveFile *out);
};

//  Load initial game state
void initGameState();

//  Save the current game state
void saveGame(Common::OutSaveFile *out, Common::String saveName);

//  Load a previously saved game state
void loadSavedGameState(int16 saveNo);

// Perform a cleanup and load process with fade-in
void loadGame(int16 saveNo);

//  Cleanup the game state
void cleanupGameState();

void checkRestartGame(const char *exeName);
void loadRestartGame();

} // end of namespace Saga2

#endif
