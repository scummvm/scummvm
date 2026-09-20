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

#ifndef SCOOBY_AMBIENT_MOVEMENT_CONTROLLER_H
#define SCOOBY_AMBIENT_MOVEMENT_CONTROLLER_H

#include "scooby/assets/rom.h"
#include "scooby/randomness/deterministic_random.h"
#include "scooby/runtime/runtime_state.h"
#include "scripted_room_movement_executor.h"

// Owns the ambient actor-movement decision recovered inside Ghidra RunAdventure at 0x00000BF4.

namespace Scooby {

class AmbientMovementController {
public:
	// Binds ambient movement selection to its ROM table and shared actor state.
	// rom: verified cartridge address space containing movement entries.
	// random: shared original random-value owner.
	// state: shared actor, cursor, and room state.
	// movement: shared owner of companion path selection and movement startup.
	AmbientMovementController(const ScoobyDooRom &rom, DeterministicRandom &random,
							  RuntimeState &state, ScriptedRoomMovementExecutor &movement)
		: _rom(rom), _random(random), _state(state), _movement(movement) {
	}

	// Starts one random ambient move when every original movement gate permits it.
	// Returns false when host closure interrupts movement startup.
	bool tryStartRandomMove();

private:
	const ScoobyDooRom &_rom;
	DeterministicRandom &_random;
	RuntimeState &_state;
	ScriptedRoomMovementExecutor &_movement;
};
} // namespace Scooby

#endif // SCOOBY_AMBIENT_MOVEMENT_CONTROLLER_H
