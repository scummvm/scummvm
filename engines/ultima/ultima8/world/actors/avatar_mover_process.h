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

#ifndef WORLD_ACTORS_AVATARMOVERPROCESS_H
#define WORLD_ACTORS_AVATARMOVERPROCESS_H

#include "ultima/ultima8/kernel/process.h"
#include "ultima/ultima8/kernel/mouse.h"
#include "ultima/ultima8/world/actors/animation.h"

namespace Ultima {
namespace Ultima8 {

class AvatarMoverProcess : public Process {
public:
	AvatarMoverProcess();
	~AvatarMoverProcess() override;

	// p_dynamic_cast stuff
	ENABLE_RUNTIME_CLASSTYPE()

	void run() override;

	void onMouseDown(int button, int32 mx, int32 my);
	void onMouseUp(int button);

	void resetIdleTime() {
		_idleTime = 0;
	}

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;

	bool hasMovementFlags(uint32 flags) const {
		return (_movementFlags & flags) != 0;
	}
	void setMovementFlag(uint32 mask) {
		_movementFlags |= mask;
	}
	void clearMovementFlag(uint32 mask) {
		_movementFlags &= ~mask;
	}

	void tryAttack();

	enum MovementFlags {
		MOVE_MOUSE_DIRECTION = 0x001,
		MOVE_RUN = 0x002,
		MOVE_STEP = 0x0004,
		MOVE_JUMP = 0x0008,

		// Tank controls 
		MOVE_TURN_LEFT = 0x0010,
		MOVE_TURN_RIGHT = 0x0020,
		MOVE_FORWARD = 0x0040,
		MOVE_BACK = 0x0080,

		// Directional controls
		MOVE_LEFT = 0x0100,
		MOVE_RIGHT = 0x0200,
		MOVE_UP = 0x0400,
		MOVE_DOWN = 0x0800,

		MOVE_ANY_DIRECTION = MOVE_MOUSE_DIRECTION | MOVE_FORWARD | MOVE_BACK | MOVE_LEFT | MOVE_RIGHT | MOVE_UP | MOVE_DOWN
	};

private:
	void handleHangingMode();
	void handleCombatMode();
	void handleNormalMode();

	void step(Animation::Sequence action, Direction direction, bool adjusted = false);
	void jump(Animation::Sequence action, Direction direction);
	void turnToDirection(Direction direction);
	bool checkTurn(Direction direction, bool moving);
	bool canAttack();

	uint32 _lastFrame;

	// attack speed limiting
	uint32 _lastAttack;

	// shake head when idle
	uint32 _idleTime;
	Animation::Sequence _lastHeadShakeAnim;
	
	MButton _mouseButton[2];

	uint32 _movementFlags;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
