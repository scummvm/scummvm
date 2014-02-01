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

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
* GNU General Public License for more details.

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

namespace Avalanche {
class AvalancheEngine;

class GhostRoom {
public:
	GhostRoom(AvalancheEngine *vm);

	void run();

private:
	AvalancheEngine *_vm;

	static const int8 kAdjustment[5];
	static const byte kPlaneToUse[4];
	static const byte kWaveOrder[5];
	static const byte kGlerkFade[26];
	static const byte kGreldetFade[18];

	enum FlavourType { ch_EGA, ch_BGI, ch_Natural, ch_Two, ch_One };

	struct ChunkBlockType {
		FlavourType _flavour;
		int8 _x, _y;
		int8 _xl, _yl;
		int32 _size;
	};

	typedef byte GlerkType[6][4][35][9];
	
	Common::File _f;
	ChunkBlockType _cb;
	byte _ghost[5][2][66][26];
	void *_memLevel;
	byte _y, _yy, _bit, _xofs;
	void *_eyes[2];
	void *_exclamation;
	void *_aargh[6];
	void *_bat[3];
	GlerkType *_glerk;
	void *_greenEyes[5];
	void *_greldet[6][2];
	Common::Point _aarghWhere[6];
	int16 _gd, _gm;
	bool _gb;
	byte _glerkStage;
	int16 _batX, _batY;
	uint16 _batCount;
	int8 _aarghCount;
	int16 _greldetX, _greldetY;
	byte _greldetCount;
	bool _redGreldet;

	void plainGrab();
	void getMe(void *p);
	void getMeAargh(byte which);
	void wait(uint16 howLong);
	void doBat();
	void bigGreenEyes(byte how);
};

} // End of namespace Avalanche

#endif // AVALANCHE_GHOSTROOM_H
