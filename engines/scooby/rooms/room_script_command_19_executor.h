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

#ifndef SCOOBY_ROOM_SCRIPT_COMMAND_19_EXECUTOR_H
#define SCOOBY_ROOM_SCRIPT_COMMAND_19_EXECUTOR_H

#include "common/scummsys.h"

#include "common/func.h"

#include "scooby/assets/rom.h"
#include "scooby/runtime/runtime_state.h"

// Owns command 0x19's actor-frame publication and visibility transition.

namespace Scooby {

class RoomScriptCommand19Executor {
public:
	// Binds actor selection to animation publication and the installed room callback.
	// rom: verified cartridge containing command fields.
	// state: shared room-object, actor-publication, and visibility state.
	// updateActorAnimationFrames: recovered actor-animation update invoked until publication completes.
	// waitForRoomVerticalBlank: callback-aware clocked frame publishing staged actor graphics.
	RoomScriptCommand19Executor(const ScoobyDooRom &rom, RuntimeState &state,
								Common::Functor0<void> &updateActorAnimationFrames,
								Common::Functor0<bool> &waitForRoomVerticalBlank)
		: _rom(rom), _state(state), _updateActorAnimationFrames(updateActorAnimationFrames),
		  _waitForRoomVerticalBlank(waitForRoomVerticalBlank) {
	}

	// Publishes a selected actor's current frame before restoring its visibility.
	// mode: zero scans the four-byte record; nonzero performs the publication transition.
	//
	// Ghidra: executeRoomScriptCommand19 (0x00002B0E). Every identity other than one or two retains the
	// wrapped object-table branch, and byte-bit selection remains modulo eight.
	void executeRoomScriptCommand19(int mode);

private:
	static const int kRoomObjectRecordSize = 0x1A;

	const ScoobyDooRom &_rom;
	RuntimeState &_state;
	Common::Functor0<void> &_updateActorAnimationFrames;
	Common::Functor0<bool> &_waitForRoomVerticalBlank;
};
} // namespace Scooby

#endif // SCOOBY_ROOM_SCRIPT_COMMAND_19_EXECUTOR_H
