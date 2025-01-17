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

#include "got/data/actor.h"
#include "common/algorithm.h"
#include "common/memstream.h"

namespace Got {

void Actor::loadFixed(Common::SeekableReadStream *src) {
	_moveType = src->readByte();
	_width = src->readByte();
	_height = src->readByte();
	_directions = src->readByte();
	_framesPerDirection = src->readByte();
	_frameSpeed = src->readByte();
	src->read(_frameSequence, 4);
	_speed = src->readByte();
	_sizeX = src->readByte();
	_sizeY = src->readByte();
	_hitStrength = src->readByte();
	_health = src->readByte();
	_numMoves = src->readByte();
	_shotType = src->readByte();
	_shotPattern = src->readByte();
	_numShotsAllowed = src->readByte();
	_solid = src->readByte();
	_flying = src->readByte() == 1;
	_dropRating = src->readByte();
	_type = src->readByte();
	src->read(_name, 9);
	_funcNum = src->readByte();
	_funcPass = src->readByte();
	_magicHurts = src->readSint16LE();
	src->skip(4);
}

void Actor::loadFixed(const byte *src) {
	Common::MemoryReadStream stream(src, 40);
	loadFixed(&stream);
}

void Actor::copyFixedAndPics(const Actor &src) {
	_moveType = src._moveType;
	_width = src._width;
	_height = src._height;
	_directions = src._directions;
	_framesPerDirection = src._framesPerDirection;
	_frameSpeed = src._frameSpeed;
	Common::copy(src._frameSequence, src._frameSequence + 4, _frameSequence);
	_speed = src._speed;
	_sizeX = src._sizeX;
	_sizeY = src._sizeY;
	_hitStrength = src._hitStrength;
	_health = src._health;
	_numMoves = src._numMoves;
	_shotType = src._shotType;
	_shotPattern = src._shotPattern;
	_numShotsAllowed = src._numShotsAllowed;
	_solid = src._solid;
	_flying = src._flying;
	_dropRating = src._dropRating;
	_type = src._type;
	Common::copy(src._name, src._name + 9, _name);
	_funcNum = src._funcNum;
	_funcPass = src._funcPass;
	_magicHurts = src._magicHurts;

	// Copy all the surfaces for all the directions over
	for (int d = 0; d < DIRECTION_COUNT; ++d) {
		for (int f = 0; f < FRAME_COUNT; ++f)
			pic[d][f].copyFrom(src.pic[d][f]);
	}
}

Actor &Actor::operator=(const Actor &src) {
	// First copy the fixed portion and the pics
	copyFixedAndPics(src);

	// Copy temporary fields
	_frameCount = src._frameCount;
	_dir = src._dir;
	_lastDir = src._lastDir;
	_x = src._x;
	_y = src._y;
	_center = src._center;
	Common::copy(src._lastX, src._lastX + 2, _lastX);
	Common::copy(src._lastY, src._lastY + 2, _lastY);
	_active = src._active;
	_nextFrame = src._nextFrame;
	_moveCountdown = src._moveCountdown;
	_vulnerableCountdown = src._vulnerableCountdown;
	_shotCountdown = src._shotCountdown;
	_currNumShots = src._currNumShots;
	_creator = src._creator;
	_unpauseCountdown = src._unpauseCountdown;
	_actorNum = src._actorNum;
	_moveCount = src._moveCount;
	_dead = src._dead;
	_toggle = src._toggle;
	_centerX = src._centerX;
	_centerY = src._centerY;
	_show = src._show;
	_temp1 = src._temp1;
	_temp2 = src._temp2;
	_counter = src._counter;
	_moveCounter = src._moveCounter;
	_edgeCounter = src._edgeCounter;
	_temp3 = src._temp3;
	_temp4 = src._temp4;
	_temp5 = src._temp5;
	_hitThor = src._hitThor;
	_rand = src._rand;
	_initDir = src._initDir;
	_passValue = src._passValue;
	_shotActor = src._shotActor;
	_magicHit = src._magicHit;
	_temp6 = src._temp6;
	_i1 = src._i1;
	_i2 = src._i2;
	_i3 = src._i3;
	_i4 = src._i4;
	_i5 = src._i5;
	_i6 = src._i6;
	_initHealth = src._initHealth;
	_talkCounter = src._talkCounter;
	_eType = src._eType;

	return *this;
}

} // namespace Got
