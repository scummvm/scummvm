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

	static bool isQuarterSpeed() {
		return _quarter;
	}
	static bool isClipping() {
		return _clipping;
	}
	static void setQuarterSpeed(bool q) {
		_quarter = q;
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

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;

	enum MovementFlags {
		MOVE_LEFT = 0x01,
		MOVE_RIGHT = 0x02,
		MOVE_UP = 0x04,
		MOVE_DOWN = 0x08,
		MOVE_ASCEND = 0x10,
		MOVE_DESCEND = 0x20,

		MOVE_ANY_DIRECTION = MOVE_LEFT | MOVE_RIGHT | MOVE_UP | MOVE_DOWN | MOVE_ASCEND | MOVE_DESCEND
	};

protected:
	uint32 _movementFlags;
	static ProcId _amp;
	static bool _clipping;
	static bool _quarter;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
