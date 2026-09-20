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

#ifndef SCOOBY_DIALOGUE_TEXT_SOURCE_H
#define SCOOBY_DIALOGUE_TEXT_SOURCE_H

#include "common/scummsys.h"

#include "common/str.h"

#include "scooby/assets/rom.h"
#include "scooby/common/optional.h"

// Preserves whether text came directly from immutable cartridge content or from a managed composition,
// replacing the original A0 alias between ROM and the shared Work RAM text buffer.

namespace Scooby {

class DialogueTextSource {
public:
	// Creates a source backed by a NUL-terminated cartridge string.
	// romOffset: cartridge offset of the first text byte.
	explicit DialogueTextSource(int romOffset) : _romOffset(romOffset), _text() {
	}

	// Creates a source backed by text composed for the current interaction or dialogue lifetime.
	// text: complete managed text without an original RAM terminator.
	explicit DialogueTextSource(const Common::String &text) : _romOffset(), _text(text) {
	}

	// Gets the cartridge offset when the original source pointer addressed immutable ROM.
	const Optional<int> &romOffset() const { return _romOffset; }

	// Gets the managed composition when the original source pointer addressed shared Work RAM.
	const Optional<Common::String> &text() const { return _text; }

	// Reads one original text byte while supplying the managed composition's implicit NUL terminator.
	// rom: verified cartridge containing immutable text sources.
	// index: signed byte displacement from the original A0 source.
	// Returns the selected cartridge or composed-text byte.
	uint8 readByte(const ScoobyDooRom &rom, int index) const {
		if (_romOffset.hasValue()) {
			return rom.readByte(_romOffset.value() + index);
		}

		const Common::String &text = _text.value();
		return index == static_cast<int>(text.size())
				   ? static_cast<uint8>(0)
				   : static_cast<uint8>(text[static_cast<std::size_t>(index)]);
	}

private:
	Optional<int> _romOffset;
	Optional<Common::String> _text;
};
} // namespace Scooby

#endif // SCOOBY_DIALOGUE_TEXT_SOURCE_H
