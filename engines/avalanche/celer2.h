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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

/* CELER	The unit for updating the screen pics. */

#ifndef CELER2_H
#define CELER2_H

#include "common/scummsys.h"
#include "common/file.h"
#include "common/str.h"

#include "graphics/surface.h"



namespace Avalanche {
class AvalancheEngine;

class Celer {
public:
	enum PictureType {kEga, kBgi, kNaturalImage};

	struct SpriteType {
		PictureType _type;
		int16 _x, _y;
		int16 _xl, _yl;
		int32 _size;
		bool _natural;

		bool _memorise; /* Hold it in memory? */
	};

	struct MemoType {
		int16 _x, _y;
		int16 _xl, _yl;
		PictureType _type;
		uint16 _size;
	};

	int32 _offsets[40];
	byte _chunkNum;
	MemoType _memos[40];
	::Graphics::Surface _memory[40]; // .free() is called in ~Celer()
	bytefield _r;

	

	Celer(AvalancheEngine *vm);

	~Celer();

	void refreshBackgroundSprites();

	void loadBackgroundSprites(byte number);

	void forgetBackgroundSprites();

	// Setting the destination to negative coordinates means the picture should be drawn to it's original position.
	// If you give it positive values, the picture will be plotted to the desired coordinates on the screen.
	// By that we get rid of show_one_at(), which would be almost identical and cause a lot of code duplication.
	void drawBackgroundSprite(int16 destX, int16 destY, byte which);

private:
	AvalancheEngine *_vm;

	Common::String _filename;

	static const int16 kOnDisk; /* Value of memos[fv].x when it's not in memory. */

	void drawSprite(int16 x, int16 y, int16 xl, int16 yl, PictureType type, const ::Graphics::Surface &picture);
};

} // End of namespace Avalanche.

#endif // CELER2_H
