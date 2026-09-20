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

#ifndef SCOOBY_PASSWORD_DISPLAY_STATE_H
#define SCOOBY_PASSWORD_DISPLAY_STATE_H

#include "common/scummsys.h"

#include "common/array.h"
#include "common/str.h"

#include "password_display_builder.h"
#include "scooby/common/span.h"
#include "scooby/runtime/runtime_state.h"

// Owns the two grouped password rows retained across title-menu phases.

namespace Scooby {

class PasswordDisplayState {
public:
	// Gets the two 30-character rows projected from original shared text workspace
	// g_abSharedTextWorkspace at 0xFF08BC-0xFF08F9. No image-backed value is consumed;
	// Refresh replaces both rows before their first read.
	const Common::Array<Common::String> &Rows() const { return _rows; }

	// Rebuilds both rows from the current room and the confirmed 29-byte password state.
	// state: runtime state supplying the room and complete progress table.
	void refresh(const RuntimeState &state) {
		_rows = Build(state.RoomId, MakeSpan(state.ProgressStateBytes));
	}

	// Replaces one symbol while preserving the authored space after every five positions.
	// rowIndex: zero-based password row selected by original register D5.
	// symbolIndex: zero-based symbol position selected by original register D4.
	// symbol: alphabet character selected from the four-by-eight grid.
	void insertSymbol(int16 rowIndex, int16 symbolIndex, char symbol) {
		Common::String &row = _rows[static_cast<std::size_t>(rowIndex)];
		row[static_cast<std::size_t>(symbolIndex + symbolIndex / kSymbolsPerGroup)] = symbol;
	}

private:
	static const int kSymbolsPerGroup = 5;

	Common::Array<Common::String> _rows = Common::Array<Common::String>(kRowCount);
};
} // namespace Scooby

#endif // SCOOBY_PASSWORD_DISPLAY_STATE_H
