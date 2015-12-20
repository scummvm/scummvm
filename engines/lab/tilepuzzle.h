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

class TilePuzzle {
private:
	LabEngine *_vm;
	Image *_tiles[16];
	Image *_numberImages[10];
	uint16 _curTile[4][4];
	byte _combination[6];

public:
	TilePuzzle(LabEngine *vm);
	virtual ~TilePuzzle();

	void mouseTile(Common::Point pos);
	void showTile(const Common::String filename, bool showSolution);
	void mouseCombination(Common::Point pos);
	void showCombination(const char *filename);
	void save(Common::OutSaveFile *file);
	void load(Common::InSaveFile *file);

private:
	void doTile(bool showsolution);
	void doTileScroll(uint16 col, uint16 row, uint16 scrolltype);
	void changeCombination(uint16 number);
	void scrollRaster(int16 dx, int16 dy, uint16 x1, uint16 y1, uint16 x2, uint16 y2, byte *buffer);
	void doCombination();
	void changeTile(uint16 col, uint16 row);
};

} // End of namespace Lab

#endif // LAB_TILEPUZZLE_H
