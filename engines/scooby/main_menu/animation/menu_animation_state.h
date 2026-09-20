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

#ifndef SCOOBY_MENU_ANIMATION_STATE_H
#define SCOOBY_MENU_ANIMATION_STATE_H

#include "common/scummsys.h"

// Retains the coupled sprite, reveal-mask, and row-wave phases written by Ghidra
// AdvanceMenuAnimation at 0x0000879E.
//
// The triangle advances by two per retrace between zero and 0x40, producing a sampled
// non-cumulative sprite offset of zero through eight pixels. Once initialization bit 4 is set,
// the pre-step phase is copied into the row mask. A countdown initialized to three advances the
// 39-word row-wave phase by two bytes every fourth retrace and wraps at 0x4E.

namespace Scooby {

class MenuAnimationState {
public:
	// Gets the animated row-mask offset copied to original word 0xFF0626 once the reveal is complete.
	int16 animatedRowMaskOffset() const { return _animatedRowMaskOffset; }

	// Gets the word index selected by Ghidra g_wSharedMenuRowShiftOrSpriteScratch000C at 0xFF000C.
	int rowShiftStartIndex() const { return _rowShiftByteOffset / static_cast<int>(sizeof(uint16)); }

	// Gets the phase sampled by Ghidra ApplyMenuSpriteVerticalOffset at 0x00008F40.
	int verticalOffset() const { return _spritePhase >> 3; }

	// Restores the zero phases and three-retrace countdown established by parent range
	// 0x000081CE-0x0000822B.
	void reset() {
		_animatedRowMaskOffset = 0;
		_descending = false;
		_rowShiftByteOffset = 0;
		_rowShiftCountdown = kRowShiftCadence;
		_spritePhase = 0;
	}

	// Advances every menu-animation output by one retrace.
	// copySpritePhaseToRowMask: whether original initialization flag bit 4 copies the current
	// sprite phase into the reveal row mask.
	//
	// Ghidra: AdvanceMenuAnimation (0x0000879E).
	void advance(bool copySpritePhaseToRowMask) {
		// Ghidra 0x0000879E-0x000087B1: once enabled, retain the pre-step phase in low byte 0xFF0627.
		if (copySpritePhaseToRowMask) {
			_animatedRowMaskOffset = static_cast<int16>(_spritePhase);
		}

		// Ghidra 0x000087B2-0x000087E3: advance the inclusive 0..0x40 triangle wave by two.
		_spritePhase += _descending ? -kPhaseStep : kPhaseStep;
		if (_spritePhase == kMaximumPhase) {
			_descending = true;
		} else if (_spritePhase == 0) {
			_descending = false;
		}

		// Ghidra 0x000087E4-0x000087ED: update the row wave only after the countdown becomes negative.
		_rowShiftCountdown--;
		if (_rowShiftCountdown >= 0) {
			return;
		}

		// Ghidra 0x000087EE-0x00008815: reset cadence and replace the unconsumed 64-word RAM queue with its
		// visible intent; both compositors sample the same ROM wave directly from the retained phase.
		_rowShiftCountdown = kRowShiftCadence;

		// Ghidra 0x00008816-0x00008826: advance one word and wrap the complete 0x4E-byte table.
		_rowShiftByteOffset += static_cast<int>(sizeof(uint16));
		if (_rowShiftByteOffset >= kRowShiftByteLength) {
			_rowShiftByteOffset = 0;
		}
	}

private:
	static const int kMaximumPhase = 0x40;
	static const int kPhaseStep = 2;
	static const int kRowShiftByteLength = 0x4E;
	static const int kRowShiftCadence = 3;

	int16 _animatedRowMaskOffset{};
	bool _descending{};
	int _rowShiftByteOffset{};
	int _rowShiftCountdown{};
	int _spritePhase{};
};
} // namespace Scooby

#endif // SCOOBY_MENU_ANIMATION_STATE_H
