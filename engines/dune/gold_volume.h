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

#ifndef DUNE_GOLD_VOLUME_H
#define DUNE_GOLD_VOLUME_H

#include "common/scummsys.h"

namespace Dune {

/** DNADG 056E/05BE/06F6/0ECC: packed FM volumes and the rotating fade mask. */
class GoldVolume {
public:
	static byte packed(byte volume, byte pan) {
		const uint v = volume >> 3;
		const uint first = pan < 120 ? pan : 120;
		const byte other = (byte)(240 - pan);
		const uint second = other < 120 ? other : 120;
		// Both divisions precede multiplication in056E. Combining them loses
		// the original rounding at noncentral pan positions.
		const uint low = ((first * 256 / 240) * v) >> 8;
		const uint high = ((second * 256 / 240) * v) >> 8;
		return (high << 4) | low;
	}

	// 05AB keeps the ordinary level separately from temporary fades.
	void setBase(byte volume, byte pan) {
		_base = packed(volume, pan);
		_target = _base;
		_mask = 0xffff;
	}

	// 066C/0675 restore that saved level on every song load. 0683 clears
	// the fading flag; it does not replace the retained timer mask.
	void play() {
		_current = _target = _base;
		_fading = false;
	}

	void stop() { _fading = false; }

	void request(uint16 duration, byte volume, byte pan, bool playing = true) {
		_target = packed(volume, pan);
		_mask = duration < 0x60 ? 0xffff : duration < 0xc0 ? 0xaaaa :
				duration < 0x180 ? 0x8888 : duration < 0x300 ? 0x8080 : 0x8000;
		_fading = playing; // 05F4 sets40h only while the driver is playing.
	}

	void tick() {
		const bool carry = (_mask & 0x8000) != 0;
		_mask = (uint16)((_mask << 1) | (_mask >> 15));
		if (!carry)
			return;
		if (_current == _target) {
			_mask = 1;
			_fading = false;
			return;
		}
		int low = _current & 15, high = _current >> 4;
		const int targetLow = _target & 15, targetHigh = _target >> 4;
		low += low < targetLow ? 1 : low > targetLow ? -1 : 0;
		high += high < targetHigh ? 1 : high > targetHigh ? -1 : 0;
		_current = (high << 4) | low;
		if (_current == 0)
			_fading = false; // 0F15 stops a fade that reaches zero.
	}

	byte current() const { return _current; }
	byte target() const { return _target; }
	byte base() const { return _base; }
	bool fading() const { return _fading; }
	uint16 mask() const { return _mask; }
	uint left() const { return ((_current >> 4) * 2) + 1; }
	uint right() const { return ((_current & 15) * 2) + 1; }

private:
	byte _current = 0xff;
	byte _target = 0xff;
	byte _base = 0xff;
	uint16 _mask = 1;
	bool _fading = false;
};

} // namespace Dune

#endif
