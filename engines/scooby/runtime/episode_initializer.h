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

#ifndef SCOOBY_EPISODE_INITIALIZER_H
#define SCOOBY_EPISODE_INITIALIZER_H

#include "runtime_state.h"
#include "scooby/assets/rom.h"

// Loads the authored initial progress and room for one of the two selectable episodes.

namespace Scooby {

class EpisodeInitializer {
public:
	// Binds episode initialization to the verified descriptor table and recovered runtime state.
	// rom: verified cartridge address space containing both episode descriptors.
	// state: runtime state that receives initial progress and room selection.
	EpisodeInitializer(const ScoobyDooRom &rom, RuntimeState &state) : _rom(rom), _state(state) {
	}

	// Clears prior progress, copies the selected authored defaults, and publishes its initial room.
	// episodeIndex: zero-based episode index supplied by original word 0xFF06AA.
	//
	// Ghidra: InitializeEpisodeProgress (0x00007F22). Descriptor pointers 0x000FCA0C and 0x000FCA40
	// select exact progress ranges of 18 bytes at 0x000FCA74 and 14 bytes at 0x0015059A; all 256
	// managed progress bytes are cleared first. Descriptor field +0x30 supplies initial room 16 or
	// 12 respectively.
	void initialize(int episodeIndex);

private:
	static const int kDescriptorTableOffset = 0x31AF2;
	static const int kInitialRoomPointerOffset = 0x30;

	const ScoobyDooRom &_rom;
	RuntimeState &_state;
};
} // namespace Scooby

#endif // SCOOBY_EPISODE_INITIALIZER_H
