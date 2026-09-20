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

#ifndef SCOOBY_SOUND_TEST_CATALOG_H
#define SCOOBY_SOUND_TEST_CATALOG_H

#include "common/scummsys.h"

#include "common/array.h"
#include "common/str.h"

#include "scooby/assets/rom.h"

// Pairs the authored sound-test labels with their managed audio-command mapping.

namespace Scooby {

class SoundTestCatalog {
public:
	// Decodes the 87 authored NUL-terminated labels at 0x001F7082-0x001F7491 once.
	// rom: verified cartridge image containing the sound-test text asset.
	explicit SoundTestCatalog(const ScoobyDooRom &rom);

	// Returns the authored label for one normalized sound-test selection.
	// selection: selection in the recovered inclusive range 0-86.
	const Common::String &getName(int selection) const { return _names[static_cast<std::size_t>(selection)]; }

	// Returns the original audio command paired with one normalized selection.
	// selection: selection in the recovered inclusive range 0-86.
	uint8 getAudioCommand(int selection) const {
		return kAudioCommands[static_cast<std::size_t>(selection)];
	}

private:
	static const int kNameCount = 87;
	static const int kNameTableOffset = 0x1F7082;

	// Original executable lookup bytes at 0x001F7492-0x001F74E8, one command for each display label.
	static const Common::Array<uint8> kAudioCommands;

	Common::Array<Common::String> _names;
};
} // namespace Scooby

#endif // SCOOBY_SOUND_TEST_CATALOG_H
