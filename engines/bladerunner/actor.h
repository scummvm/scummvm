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

#ifndef BLADERUNNER_ACTOR_H
#define BLADERUNNER_ACTOR_H

#include "bladerunner/bladerunner.h"

#include "bladerunner/vector.h"

namespace BladeRunner {

class BladeRunnerEngine;
class BoundingBox;

class Actor {
	BladeRunnerEngine *_vm;

private:
	BoundingBox   *_bbox;
	// MovementTrack *_movementTrack;

	int  _honesty;
	int  _intelligence;
	int  _stability;
	int  _combatAggressiveness;
	int  _goalNumber;
	int *_friendlinessToOther;

	int _currentHP;
	int _maxHP;

	// Clues _clues;

	int     _id;
	int     _set;
	Vector3 _position;
	int     _facing; // [0, 1024)
	int     _walkboxId;

	// Flags
	bool _isTargetable;
	bool _isInvisible;
	bool _isImmuneToObstacles;

	// Animation
	int _width;
	int _height;
	int _animationMode;
	int _fps;
	int _frame_ms;
	int _animationId;
	int _animationFrame;

	// WalkInfo _walkInfo;

	int _timersRemain[7];
	int _timersBegan[7];

	float _scale;

public:
	Actor(BladeRunnerEngine *_vm, int actorId);
	~Actor();

	void setup(int actorId);

	void set_at_xyz(Vector3 pos, int facing);

	void draw();
};

} // End of namespace BladeRunner

#endif
