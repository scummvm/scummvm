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
 * $URL$
 * $Id$
 *
 */

#ifndef M4_MADS_PLAYER_H
#define M4_MADS_PLAYER_H

#include "common/scummsys.h"

namespace M4 {

#define PLAYER_SEQ_INDEX -2

class MadsPlayer {
private:
	int getScale(int yp);
	int getSpriteSlot();
public:
	char _spritesPrefix[16];
	int _spriteSetCount;
	int _spriteSetIndexes[8];
	Common::Point _playerPos;
	Common::Point _destPos;
	int16 _direction, _direction2;
	bool _visible, _priorVisible;
	bool _visible3;
	bool _forceRefresh;
	int16 _currentScale;
	int16 _yScale;
	int16 _currentDepth;
	int16 _spriteListIdx, _spriteListIdx2;
	bool _spritesChanged;
	int16 _frameOffset, _frameNum;
	bool _moving;
public:
	MadsPlayer();

	bool loadSprites(const char *prefix);
	void update();
	void idle();
};

} // End of namespace M4

#endif
