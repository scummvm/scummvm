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

#include "bladerunner/actor.h"

#include "bladerunner/bladerunner.h"

#include "bladerunner/boundingbox.h"
#include "bladerunner/gameinfo.h"
#include "bladerunner/slice_renderer.h"

namespace BladeRunner {

Actor::Actor(BladeRunnerEngine *vm, int actorId) {
	_vm = vm;
	_id = actorId;

	// TODO: Construct Walkinfo

	_bbox = new BoundingBox();

	// TODO: Construct _clues ((actorId && actor != 99) ? 2 : 4)

	// TODO: Construct _movementTrack

	_friendlinessToOther = new int[_vm->_gameInfo->getActorCount()];
}

Actor::~Actor() {
	delete[] _friendlinessToOther;
	delete   _bbox;
	// delete   _clues;
	// delete   _movementTrack;
}

void Actor::setup(int actorId) {
	_id  = actorId;
	_set = -1;

	_position  = Vector3(0.0, 0.0, 0.0);
	_facing    = 512;
	_walkboxId = -1;

	_walkboxId = -1;
	_animationId = 0;
	_animationFrame = 0;
	_fps = 15;
	_frame_ms = 1000 / _fps;

	_isTargetable        = false;
	_isInvisible         = false;
	_isImmuneToObstacles = false;

	_width = 0;
	_height = 0;

	for (int i = 0; i != 7; ++i) {
		_timersRemain[i] = 0;
		_timersBegan[i]  = _vm->getTotalPlayTime();
	}

	_scale = 1.0;

	_honesty              = 50;
	_intelligence         = 50;
	_combatAggressiveness = 50;
	_stability            = 50;

	_currentHP            = 50;
	_maxHP                = 50;
	_goalNumber           = -1;

	_timersRemain[4] = 60000;
	_animationMode = -1;
	// TODO: screen rect = { -1, -1, -1, -1 };

	int actorCount = (int)_vm->_gameInfo->getActorCount();
	for (int i = 0; i != actorCount; ++i)
		_friendlinessToOther[i] = 50;

	// TODO: Setup clues

	// TODO: Flush movement track
}

void Actor::set_at_xyz(Vector3 pos, int facing) {
	_position = pos;
	_facing = facing;
}

void Actor::draw() {
	Vector3 draw_position(_position.x, -_position.z, _position.y + 2.0);
	float   draw_facing = _facing * M_PI / 512.0;
	// float   draw_scale  = _scale;

	// TODO: Handle SHORTY mode

	_vm->_sliceRenderer->setupFrame(19, 1, draw_position, M_PI - draw_facing);
	_vm->_sliceRenderer->drawFrame(_vm->_surface2, _vm->_zBuffer2);
}

} // End of namespace BladeRunner
