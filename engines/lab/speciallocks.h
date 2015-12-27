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

#ifndef LAB_TILEPUZZLE_H
#define LAB_TILEPUZZLE_H

#include "common/savefile.h"

namespace Lab {

class LabEngine;

class SpecialLocks {
private:
	LabEngine *_vm;
	Image *_tiles[16];
	Image *_numberImages[10];
	uint16 _curTile[4][4];
	byte _combination[6];

public:
	SpecialLocks(LabEngine *vm);
	~SpecialLocks();

	void showTileLock(const Common::String filename, bool showSolution);

	/**
	 * Processes mouse clicks and changes tile positions.
	 */
	void tileClick(Common::Point pos);

	void showCombinationLock(const Common::String filename);

	/**
	 * Processes mouse clicks and changes the door combination.
	 */
	void combinationClick(Common::Point pos);

	void save(Common::OutSaveFile *file);
	void load(Common::InSaveFile *file);

private:
	/**
	 * Changes the combination number of one of the slots
	 */
	void changeCombination(uint16 number);

	/**
	 * Changes the tile positions in the tile puzzle
	 */
	void changeTile(uint16 col, uint16 row);

	/**
	 * Draws the images of the combination lock to the display bitmap.
	 */
	void doTile(bool showsolution);

	/**
	 * Does the scrolling for the tiles on the tile puzzle.
	 */
	void doTileScroll(uint16 col, uint16 row, uint16 scrolltype);
	void scrollRaster(int16 dx, int16 dy, uint16 x1, uint16 y1, uint16 x2, uint16 y2, byte *buffer);
};

} // End of namespace Lab

#endif // LAB_TILEPUZZLE_H
