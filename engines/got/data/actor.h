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

#include "common/stream.h"
#include "graphics/managed_surface.h"

#ifndef GOT_DATA_ACTOR_H
#define GOT_DATA_ACTOR_H

namespace Got {

#define DIRECTION_COUNT 4
#define FRAME_COUNT 4

struct Actor {
	// First part loaded from disk
	byte _moveType = 0;           // Movement pattern (0=none)
	byte _width = 0;              // Physical width
	byte _height = 0;             // Physical height
	byte _directions = 0;         // 1,2 or 4 (1=uni-directional)
	byte _framesPerDirection = 0; // # frames per direction
	byte _frameSpeed = 0;         // # cycles between frame changes
	byte _frameSequence[4] = {};  // Sequence
	byte _speed = 0;              // Move every Nth cycle
	byte _sizeX = 0;              // Non-physical padding on X coordinate
	byte _sizeY = 0;              // Non-physical padding on Y coordinate
	byte _hitStrength = 0;        // Hit strength
	byte _health = 0;             //
	byte _numMoves = 0;           // # of moves every <_speed> cycles
	byte _shotType = 0;           // Actor # of shot
	byte _shotPattern = 0;        // Func number to decide to shoot
	byte _numShotsAllowed = 0;    // # shots allowed on screen
	byte _solid = 0;              // 1=solid (not ghost,etc)
	bool _flying = false;         //
	byte _dropRating = 0;         // rnd(100) < _dropRating = jewel
	byte _type = 0;               // Actor (0=thor,1=hammer,2=enemy,3=shot)
	char _name[9] = {};           // Actors name
	byte _funcNum = 0;            // Special function when thor touches
	byte _funcPass = 0;           // Value to pass to func
	uint16 _magicHurts = 0;       // Bitwise magic hurts flags

	// The rest is dynamic   //size=216
	// Direction/frame surfaces
	Graphics::ManagedSurface pic[DIRECTION_COUNT][FRAME_COUNT];

	byte _frameCount = 0;           // Count to switch frames
	byte _dir = 0;                  // Direction of travel
	byte _lastDir = 0;              // Last direction of travel
	int _x = 0;                     // Actual X coordinate
	int _y = 0;                     // Actual Y coordinate
	int _center = 0;                // Center of object
	int _lastX[2] = {};             // Last X coordinate on each page
	int _lastY[2] = {};             // Last Y coordinate on each page
	byte _active = 0; //boolean     // 1=active, 0=not active
	byte _nextFrame = 0;            // Next frame to be shown
	byte _moveCountdown = 0;        // Count down to movement
	byte _vulnerableCountdown = 0;  // Count down to vulnerability
	byte _shotCountdown = 0;        // Count down to another shot
	byte _currNumShots = 0;         // # of shots currently on screen
	byte _creator = 0;              // Which actor # created this actor
	byte _unpauseCountdown = 0;     // Must be 0 to move
	byte _actorNum = 0;
	byte _moveCount = 0;
	byte _dead = 0;
	byte _toggle = 0;
	byte _centerX = 0;
	byte _centerY = 0;
	byte _show = 0;                 // Display or not (for blinking)
	byte _temp1 = 0;
	byte _temp2 = 0;
	byte _counter = 0;
	byte _moveCounter = 0;
	byte _edgeCounter = 0;
	byte _temp3 = 0;
	byte _temp4 = 0;
	byte _temp5 = 0;
	byte _hitThor = 0; // boolean
	int _rand = 0;
	byte _initDir = 0;
	byte _passValue = 0;
	byte _shotActor = 0;
	byte _magicHit = 0;
	byte _temp6 = 0;
	int _i1 = 0, _i2 = 0, _i3 = 0, _i4 = 0, _i5 = 0, _i6 = 0;
	byte _initHealth = 0;
	byte _talkCounter = 0;
	byte _eType = 0; // unused

	void loadFixed(Common::SeekableReadStream *src);
	void loadFixed(const byte *src);

	/**
     * Copies the fixed portion and pics from a source actor.
     */
	void copyFixedAndPics(const Actor &src);

	Actor &operator=(const Actor &src);

	int getPos() const {
		return ((_x + 7) / 16) + (((_y + 8) / 16) * 20);
	}
};

} // namespace Got

#endif
