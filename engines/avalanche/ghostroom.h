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
* This code is based on the original source code of Lord Avalot d'Argent version 1.3.
* Copyright (c) 1994-1995 Mike: Mark and Thomas Thurman.
*/

#ifndef AVALANCHE_GHOSTROOM_H
#define AVALANCHE_GHOSTROOM_H

#include "common/scummsys.h"
#include "graphics/surface.h"

namespace Avalanche {
class AvalancheEngine;

struct ChunkBlock {
	Flavour _flavour;
	int16 _x, _y;
	int16 _width, _height;
	int32 _size;
};

class GhostRoom {
public:
	GhostRoom(AvalancheEngine *vm);
	~GhostRoom();

	void run();
	ChunkBlock readChunkBlock(Common::File &file);

private:
	AvalancheEngine *_vm;

	static const int8 kAdjustment[5];
	static const byte kWaveOrder[5];
	static const byte kGlerkFade[26];
	static const byte kGreldetFade[18];

	Common::Point dummyCoord;
	byte ****_ghost;// [5][2][66][26]
	Graphics::Surface _eyes[2];
	Graphics::Surface _exclamation;
	Graphics::Surface _bat[3];
	byte ****_glerk; // [6][4][35][9]
	Graphics::Surface _aargh[6];
	Common::Point _aarghWhere[6];
	Graphics::Surface _greenEyes[5];
	Graphics::Surface _greldet[6][2];

	int16 _batX, _batY;
	uint16 _batCount;
	byte _glerkStage;
	int8 _aarghCount;
	int16 _greldetX, _greldetY;
	byte _greldetCount;
	bool _redGreldet;
	bool _wasLoaded;

	void loadPictures();
	void wait(uint16 howLong);
	void doBat();
	void bigGreenEyes(byte how);
};

} // End of namespace Avalanche

#endif // AVALANCHE_GHOSTROOM_H
