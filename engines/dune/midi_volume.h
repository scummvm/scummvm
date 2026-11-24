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

#ifndef DUNE_MIDI_VOLUME_H
#define DUNE_MIDI_VOLUME_H

#include "common/scummsys.h"

namespace Dune {

/** DNMID 01EE/022B/0293/0333/04D7: retained master and CC7 fade. */
class MidiVolume {
public:
	void setBase(byte volume) {
		_base = _target = volume;
		_mask = 0xffff;
	}

	void play() {
		_current = _target = _base;
		_playing = true;
		_fading = false;
	}

	void stop() { _playing = _fading = false; }

	void request(uint16 duration, byte volume) {
		_target = volume; // DNMID ignores the pan byte.
		_mask = duration < 0x60 ? 0xffff : duration < 0xc0 ? 0xaaaa :
				duration < 0x180 ? 0x8888 : duration < 0x300 ? 0x8080 : 0x8000;
		if (_playing)
			_fading = true;
	}

	// Return true only when 04D7 writes the ten channel volume messages.
	bool tick() {
		if (!_playing)
			return false;
		const bool carry = (_mask & 0x8000) != 0;
		_mask = (uint16)((_mask << 1) | (_mask >> 15));
		if (!carry)
			return false;
		if (_current == _target) {
			_mask = 1;
			_fading = false;
			return false;
		}
		const int delta = (int)_target - _current;
		_current += delta < -3 ? -3 : delta > 3 ? 3 : delta;
		if (!_current)
			stop(); // 0523 stops playback when a fade changes the level to zero.
		return true;
	}

	byte channel(byte volume) const { return (uint16)volume * _current >> 8; }
	byte current() const { return _current; }
	byte target() const { return _target; }
	byte base() const { return _base; }
	uint16 mask() const { return _mask; }
	bool fading() const { return _fading; }
	bool playing() const { return _playing; }

private:
	byte _current = 230;
	byte _target = 230;
	byte _base = 230;
	uint16 _mask = 0;
	bool _fading = false;
	bool _playing = false;
};

} // namespace Dune

#endif
