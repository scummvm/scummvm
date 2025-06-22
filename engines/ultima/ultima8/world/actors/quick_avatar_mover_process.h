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

#ifndef WORLD_ACTORS_QUICKAVATARMOVERPROCESS_H
#define WORLD_ACTORS_QUICKAVATARMOVERPROCESS_H

#include "ultima/ultima8/metaengine.h"
#include "ultima/ultima8/kernel/process.h"

namespace Ultima {
namespace Ultima8 {

class QuickAvatarMoverProcess : public Process {
public:
	QuickAvatarMoverProcess();
	~QuickAvatarMoverProcess() override;

	ENABLE_RUNTIME_CLASSTYPE()

	static QuickAvatarMoverProcess *get_instance();

	void run() override;
	void terminate() override;

	static bool isEnabled() {
		return _enabled;
	}
	static void setEnabled(bool value) {
		_enabled = value;
	}
	static bool isClipping() {
		return _clipping;
	}
	static void setClipping(bool value) {
		_clipping = value;
	}
	static void toggleClipping() {
		_clipping = !_clipping;
	}

	bool hasMovementFlags(uint32 flags) const {
		return (_movementFlags & flags) != 0;
	}
	void setMovementFlag(uint32 mask) {
		_movementFlags |= mask;
	}
	virtual void clearMovementFlag(uint32 mask) {
		_movementFlags &= ~mask;
	}
	void resetMovementFlags() {
		_movementFlags = 0;
	}

	// Return true if handled, false if not.
	bool onActionDown(KeybindingAction action);
	bool onActionUp(KeybindingAction action);

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;

	enum MovementFlags {
		MOVE_ASCEND = 0x0001,
		MOVE_DESCEND = 0x0002,
		MOVE_SLOW = 0x0004,
		MOVE_FAST = 0x0008,

		// Tank controls
		MOVE_TURN_LEFT  = 0x0010,
		MOVE_TURN_RIGHT = 0x0020,
		MOVE_FORWARD    = 0x0040,
		MOVE_BACK       = 0x0080,

		// Directional controls
		MOVE_LEFT  = 0x0100,
		MOVE_RIGHT = 0x0200,
		MOVE_UP    = 0x0400,
		MOVE_DOWN  = 0x0800,

		MOVE_ANY_DIRECTION = MOVE_LEFT | MOVE_RIGHT | MOVE_UP | MOVE_DOWN | MOVE_ASCEND | MOVE_DESCEND
	};

protected:
	uint32 _movementFlags;
	static ProcId _amp;
	static bool _enabled;
	static bool _clipping;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
