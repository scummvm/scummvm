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

/**
 * Base class for mover processes that decide which animation to
 * do next based on last anim and keyboard / mouse / etc.
 */
class AvatarMoverProcess : public Process {
public:
	AvatarMoverProcess();
	~AvatarMoverProcess() override;

	void run() override;

	void resetIdleTime() {
		_idleTime = 0;
	}

	bool loadData(Common::ReadStream *rs, uint32 version);
	virtual void saveData(Common::WriteStream *ws) override;

	bool hasMovementFlags(uint32 flags) const {
		return (_movementFlags & flags) != 0;
	}
	void setMovementFlag(uint32 mask) {
		_movementFlags |= mask;
	}
	void clearMovementFlag(uint32 mask) {
		_movementFlags &= ~mask;
	}

	void onMouseDown(int button, int32 mx, int32 my);
	void onMouseUp(int button);

	virtual void tryAttack() = 0;

	enum MovementFlags {
		MOVE_MOUSE_DIRECTION = 0x001,
		MOVE_RUN = 0x002,
		MOVE_STEP = 0x0004, // also side-steps in crusader
		MOVE_JUMP = 0x0008, // used for roll in crusader (when combined with left/right), and crouch (when combined with back)

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

protected:
	virtual void handleHangingMode() = 0;
	virtual void handleCombatMode() = 0;
	virtual void handleNormalMode() = 0;

	virtual bool canAttack() = 0;

	void turnToDirection(Direction direction);
	bool checkTurn(Direction direction, bool moving);

	// Walk and then stop in the given direction
	void slowFromRun(Direction direction);

	// Stow weapon and stand
	void putAwayWeapon(Direction direction);

	// If the last animation was falling or die but we're not dead, stand up!
	// return true if we are waiting to get up
	bool standUpIfNeeded(Direction direction);

	// Get directions based on what movement flags are set, eg y=+1 for up, x=-1 for left.
	void getMovementFlagAxes(int &x, int &y);

	// Adjust the direction based on the current turn flags
	Direction getTurnDirForTurnFlags(Direction direction, DirectionMode dirmode);

	// attack speed limiting
	uint32 _lastAttack;

	// shake head when idle
	uint32 _idleTime;

	MButton _mouseButton[2];

	uint32 _movementFlags;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
