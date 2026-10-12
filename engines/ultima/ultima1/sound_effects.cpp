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

#include "ultima/ultima1/sound_effects.h"

namespace Ultima {
namespace Ultima1 {

// The clock of the processor the timing loops were written for
constexpr uint32 CPU_HZ = 4772727;

// Cost in clock cycles of writing to the speaker's port, and of flipping the
// bit that is written
constexpr uint32 OUT_CYCLES = 14;
constexpr uint32 FLIP_CYCLES = 4;

constexpr int AMPLITUDE = 0x30;

namespace {

/**
 * Counts the clock cycles spent and notes when the speaker changed level
 */
class Speaker {
private:
	uint32 _cycles = 0;
	bool _bit = false;
	bool _level = false;

public:
	Common::Array<uint32> _edges;

	uint32 cycles() const {
		return _cycles;
	}

	void run(uint32 cycles) {
		_cycles += cycles;
	}

	// Writes the current bit to the speaker
	void out() {
		run(OUT_CYCLES);
		if (_bit != _level) {
			_level = _bit;
			_edges.push_back(_cycles);
		}
	}

	// Flips the bit the next write will use
	void flip() {
		run(FLIP_CYCLES);
		_bit = !_bit;
	}

	// A loop of decrementing a byte register until it reaches zero
	void countByte(int count) {
		if (count == 0)
			count = 256;
		_cycles += 19 * count - 12;
	}

	// The same, with a word register
	void countWord(int count) {
		if (count == 0)
			count = 65536;
		_cycles += 18 * count - 10;
	}

	// The cost of a conditional jump that is taken or isn't
	void jump(bool taken) {
		run(taken ? 16 : 4);
	}

	// The same for the loop instruction
	void loop(bool taken) {
		run(taken ? 17 : 5);
	}

	// Lets go of the speaker at the end
	void release() {
		_bit = false;
		out();
	}
};

// Sixteen long pulses
void bump(Speaker &s) {
	s.run(4);
	for (int left = 0x10; left > 0; --left) {
		s.out();
		s.flip();
		s.run(4);
		s.countWord(0x3E8);
		s.loop(left > 1);
	}
}

// Forty eight shorter pulses
void ack(Speaker &s) {
	s.run(4);
	for (int left = 0x30; left > 0; --left) {
		s.out();
		s.flip();
		s.run(4);
		s.run(4);
		s.countWord(0x163);
		s.run(2);
		s.jump(false);
		s.loop(left > 1);
	}
}

// Pulses that lengthen steadily, starting from a given length. The length is
// a byte, so it wraps back to short again if the effect is long enough
void chirp(Speaker &s, byte repeats, byte length) {
	s.run(4 + 4);
	for (int left = repeats; left > 0; --left) {
		length = (byte)(length + 5);
		s.run(4 + 2);
		s.countByte(length);
		s.out();
		s.flip();
		s.run(3);
		s.jump(left > 1);
	}
}

// Pairs of pulses, one lengthening and one shortening, which sweep up and down
void siren(Speaker &s, byte high, byte repeats) {
	byte low = 1;
	s.run(2 + 2);

	do {
		s.run(2);
		for (int left = repeats; left > 0; --left) {
			s.run(2);
			s.countByte(high);
			s.out();
			s.flip();
			s.run(2);
			s.countByte(low);
			s.out();
			s.flip();
			s.run(3);
			s.jump(left > 1);
		}

		--high;
		++low;
		s.run(3 + 3 + 4);
		s.jump(low != 0x1B);
	} while (low != 0x1B);
}

// Pulses that steadily shorten
void fall(Speaker &s) {
	s.run(4 + 3);
	for (int length = 0xA0; length > 0; --length) {
		s.run(2);
		s.countWord(length);
		s.out();
		s.flip();
		s.run(3);
		s.jump(length > 1);
	}
}

// Pulses that steadily lengthen, getting faster than a chirp
void zap(Speaker &s) {
	s.run(4);
	for (int low = 0xFB; low > 0; --low) {
		s.out();
		s.flip();
		s.run(2);
		s.countByte(256 - low);
		s.run(3);
		s.jump(low > 1);
	}
}

// Turns when the speaker changed level into samples
Common::Array<byte> toSamples(Speaker &s) {
	s.release();

	Common::Array<byte> samples;
	uint32 total = (uint32)(((uint64)s.cycles() * SoundEffects::SAMPLE_RATE) / CPU_HZ) + 1;
	samples.resize(total);

	uint32 pos = 0;
	bool high = false;
	for (uint i = 0; i <= s._edges.size(); ++i) {
		uint32 end = (i < s._edges.size()) ? (uint32)(((uint64)s._edges[i] * SoundEffects::SAMPLE_RATE) / CPU_HZ) : total;
		byte level = high ? 0x80 + AMPLITUDE : 0x80 - AMPLITUDE;

		for (; pos < end && pos < total; ++pos)
			samples[pos] = level;
		high = !high;
	}

	return samples;
}

} // End of anonymous namespace

Common::Array<byte> SoundEffects::render(int effect) {
	Speaker s;

	switch (effect) {
	case 0:
		bump(s);
		break;
	case 1:
		ack(s);
		break;
	case 2:
		chirp(s, 200, 0x7E);
		break;
	case 3:
		chirp(s, 255, 0x7E);
		break;
	case 4:
		chirp(s, 8, 0x7E);
		break;
	case 5:
		siren(s, 0xC0, 0x3C);
		break;
	case 6:
		fall(s);
		break;
	case 7:
		siren(s, 5, 0x0F);
		break;
	case 8:
		zap(s);
		break;
	case 9:
		chirp(s, 4, 0);
		break;
	default:
		return Common::Array<byte>();
	}

	return toSamples(s);
}

} // namespace Ultima1
} // namespace Ultima
