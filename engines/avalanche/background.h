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
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

/* Original name: CELER		The unit for updating the screen pics. */

#ifndef AVALANCHE_BACKGROUND_H
#define AVALANCHE_BACKGROUND_H

#include "common/str.h"

namespace Avalanche {
class AvalancheEngine;

enum PictureType {kEga, kBgi, kNaturalImage};

struct SpriteType {
	PictureType _type;
	int16 _x, _y;
	int16 _width, _height;
	int32 _size;
	Graphics::Surface _picture;
};

class Background {
public:
	Background(AvalancheEngine *vm);
	~Background();

	void update();
	void loadSprites(byte number);
	void release();

	// Setting the destination to negative coordinates means the picture should be drawn to it's original position.
	// If you give it positive values, the picture will be plotted to the desired coordinates on the screen.
	// By that we get rid of show_one_at(), which would be almost identical and cause a lot of code duplication.
	void draw(int16 destX, int16 destY, byte sprId);
	void resetVariables();
	void synchronize(Common::Serializer &sz);

private:
	AvalancheEngine *_vm;

	byte _nextBell; // For the ringing.
	int32 _offsets[40];
	byte _spriteNum;
	SpriteType _sprites[40];
	Common::String _filename;
	static const int16 kOnDisk; // Value of _sprites[fv]._x when it's not in memory.

	void drawSprite(int16 x, int16 y, SpriteType &sprite);
};

} // End of namespace Avalanche.

#endif // AVALANCHE_BACKGROUND_H
