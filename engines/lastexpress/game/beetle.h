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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef LASTEXPRESS_BEETLE_H
#define LASTEXPRESS_BEETLE_H

#include "lastexpress/lastexpress.h"

#include "lastexpress/helpers.h"

#include "common/array.h"
#include "common/rect.h"
#include "common/system.h"

namespace LastExpress {

class LastExpressEngine;
struct Seq;
struct Sprite;

class CBeetle {
public:
	CBeetle(LastExpressEngine *engine);
	~CBeetle();

	void tick();
	void checkMouse();
	void setDirection(int direction);
	bool onTable();
	bool click();

private:
	LastExpressEngine *_engine = nullptr;

	Seq *_sequences[30];
	Seq *_currentSequence = nullptr;
	int _currentFrame = 0;
	int _index = 0;
	int _coordOffset = 0;
	Common::Point _coords;
	int _directions[16];
	int _currentDirectionIndex = 0;
	Sprite *_frame = nullptr;
	bool _loaded = false;
	int _mouseCooldown = 0;
	int _fleeSpeed = 0;
	int _spawnCounter = 0;
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_BEETLE_H
