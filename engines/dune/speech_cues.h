/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef DUNE_SPEECH_CUES_H
#define DUNE_SPEECH_CUES_H

#include "common/array.h"
#include "common/endian.h"

namespace Dune {

class SpeechCues {
public:
	void clear() {
		_mouths.clear();
	}

	/** A87E's FREQ sample establishes a shared 16.16 PIT period. */
	bool calibrate(const byte *freq, uint32 size) {
		_period = 0;
		// FREQ has a private 26-byte label, not a Creative VOC signature.
		if (!freq || size < 33 || freq[26] != 1 || freq[31] != 0)
			return false;
		const uint32 length = freq[27] | (freq[28] << 8) | (freq[29] << 16);
		if (length <= 2 || length > size - 30)
			return false;
		const uint64 durationUs = (uint64)(length - 2) * (256 - freq[30]);
		// A87E measures the complete sample in whole 200-Hz ticks, then
		// multiplies that count by 0800h: divide by 32 in 16.16 units.
		const uint64 ticks = (durationUs + 2500) / 5000;
		if (!ticks || ticks > 65535)
			return false;
		_period = (uint32)ticks << 11;
		return true;
	}

	bool calibrated() const { return _period != 0; }

	void load(const byte *voc, uint32 size) {
		clear();
		if (!voc || size < 26 || memcmp(voc, "Creative Voice File\x1a", 20))
			return;
		const uint32 at = READ_LE_UINT16(voc + 20);
		if (at < 26 || at > size - 4 || voc[at] != 5)
			return;
		const uint32 length = voc[at + 1] | (voc[at + 2] << 8) | (voc[at + 3] << 16);
		if (length < 3 || length > size - at - 4)
			return;
		// A83F skips the text block's two-byte line id. A814 then consumes
		// mouth ids until FF, not ASCII text or the following sound block.
		const byte *cues = voc + at + 6;
		uint count = 0;
		// A814 stops at FF. Named-speaker tables (including the guard in
		// PO02C/PO02D) have more entries than the four mouths per Fremen.
		while (count < length - 2 && cues[count] != 0xff)
			count++;
		if (!count || count == length - 2 || cues[count] != 0xff)
			return;
		_mouths.assign(cues, cues + count);
	}

	int mouthAt(uint32 elapsedMs) const {
		if (_mouths.empty() || !_period)
			return -1;
		// A75C starts the cue anchor at half a PIT tick. A7C2 compares
		// the whole deadline before fetching each cue, including catch-up.
		// Thus cue N is due at floor(N * period + 0.5), not N * 882/rate.
		// The strict upper bound keeps an exactly-next-tick cue for that tick.
		const uint64 frame = (((uint64)(elapsedMs / 5) + 1) * 65536 - 32769) / _period;
		return _mouths[frame < _mouths.size() ? (uint)frame : _mouths.size() - 1];
	}

private:
	Common::Array<byte> _mouths;
	uint32 _period = 0;
};

} // namespace Dune

#endif
