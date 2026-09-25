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

#ifndef SCOOBY_APPLICATION_LOOP_H
#define SCOOBY_APPLICATION_LOOP_H

#include "common/error.h"
#include "common/serializer.h"
#include "scooby/assets/rom.h"
#include "scooby/graphics/indexed_frame.h"
#include "scooby/graphics/tile_scene.h"
#include "scooby/randomness/deterministic_random.h"
#include "scooby/raylib_host.h"
#include "scooby/runtime/runtime_state.h"

// Owns recovered top-level sequencing independently from the raylib platform boundary.

namespace Scooby {

class ApplicationLoop {
public:
	// Binds verified original data, the decoded tile scene, and the process-owned presentation host.
	// rom: verified cartridge image.
	// scene: high-level visual scene populated from decoded ROM assets.
	// frame: shared logical presentation frame.
	// host: process-global raylib owner.
	ApplicationLoop(const ScoobyDooRom &rom, TileScene &scene, IndexedFrame &frame,
					RaylibHost &host)
		: _rom(rom), _scene(scene), _frame(frame), _host(host) {
	}

	// Executes the ported startup path and then enters Ghidra RunAdventure at 0x00000BF4.
	void run();

	Common::Error syncGame(Common::Serializer &s);

private:
	// Initializes the shared state lifecycle before any startup presentation can run.
	// state: runtime state retained through the following adventure session.
	// random: deterministic random owner retained through the following adventure session.
	//
	// Ghidra: initializeRuntimeState (0x0000A248). Its complete body is 0x0000A248-0x0000A279, followed
	// by fallthrough into ResetRuntimeState at 0x0000A27A. The V-counter-scaled warm-up result is
	// discarded, while its seed transition is independent of the sampled counter.
	void initializeRuntimeState(RuntimeState &state, DeterministicRandom &random);

	const ScoobyDooRom &_rom;
	TileScene &_scene;
	IndexedFrame &_frame;
	RaylibHost &_host;
	RuntimeState _state;
	SaveGameData _loadGameData;
};
} // namespace Scooby

#endif // SCOOBY_APPLICATION_LOOP_H
