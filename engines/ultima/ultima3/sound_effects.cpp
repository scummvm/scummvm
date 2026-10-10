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

#include "ultima/ultima3/sound_effects.h"
#include "graphics/views/events.h"

namespace Ultima {
namespace Ultima3 {

// The clock of the processor the timing loops were written for
constexpr uint32 CPU_HZ = 4772727;

// Cost in clock cycles of the code that flips the speaker, and of
// the pseudo random number routine
constexpr uint32 TOGGLE_CYCLES = 4 + 14;
constexpr uint32 RANDOM_CYCLES = 1100;

constexpr int AMPLITUDE = 0x30;

namespace {

/**
 * Counts the clock cycles spent and notes when the speaker was flipped
 */
class Speaker {
public:
	uint32 _cycles = 0;
	Common::Array<uint32> _edges;

	void run(uint32 cycles) {
		_cycles += cycles;
	}

	void toggle() {
		_cycles += TOGGLE_CYCLES;
		_edges.push_back(_cycles);
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
		_cycles += taken ? 16 : 4;
	}

	// A random number up to a limit, which the original then reduces to fit
	int getRandomNumber(int limit) {
		_cycles += RANDOM_CYCLES;
		return Graphics::Views::g_events->getRandomNumber(255) % limit;
	}
};

// A sweep up and down in the width of the pulses
void sweep(Speaker &s, byte start, byte repeats) {
	int high = start, low = 1;

	auto pulses = [&]() {
		s.run(2);
		for (int left = repeats ? repeats : 256; left > 0; --left) {
			s.run(2);
			s.countByte(high);
			s.toggle();
			s.run(2);
			s.countByte(low);
			s.toggle();
			s.run(3);
			s.jump(left > 1);
		}
	};

	do {
		pulses();
		high = (high - 1) & 0xFF;
		++low;
		s.run(3 + 3 + 4);
		s.jump(low != 0x1B);
	} while (low != 0x1B);

	do {
		pulses();
		--low;
		high = (high + 1) & 0xFF;
		s.run(3 + 3 + 4);
		s.jump(low != 0);
	} while (low != 0);
}

// Bursts of noise, each a pulse of random length
void noise(Speaker &s, int count, int minimum) {
	for (int left = count; left > 0; --left) {
		int length = s.getRandomNumber(255) | minimum;
		s.run(3);
		s.countByte(length);
		s.toggle();
		s.run(3);
		s.jump(left > 1);
	}
}

void rumble(Speaker &s, byte base) {
	for (int left = 0x80; left > 0; --left) {
		s.run(2);
		int length = (s.getRandomNumber(15) + base) & 0xFF;
		s.run(3);

		do {
			s.run(4);
			s.run(17 * 2 + 5);
			s.run(3);
			length = (length - 1) & 0xFF;
			s.jump(length != 0);
		} while (length != 0);

		s.toggle();
		s.run(3);
		s.jump(left > 1);
	}
}

// A fall in pitch
void descend(Speaker &s) {
	for (int low = 0xFB; low > 0; --low) {
		s.toggle();
		s.run(2);
		s.run(19 * (256 - low) - 12);
		s.run(3);
		s.jump(low > 1);
	}
}

// A rise in pitch
void ascend(Speaker &s) {
	for (int count = 0xA0; count > 0; --count) {
		s.run(2);
		s.countWord(count);
		s.toggle();
		s.run(3);
		s.jump(count > 1);
	}
}

// Noise getting steadily lower
void crackle(Speaker &s) {
	for (int low = 0xE0; low != 0x40; --low) {
		int length = s.getRandomNumber(255) | low;
		s.run(3);
		s.countByte(length);
		s.toggle();
		s.run(3 + 3);
		s.jump(low - 1 != 0x40);
	}
}

// Short bursts of tone at random pitches
void warble(Speaker &s, byte seed) {
	int bursts = ((seed & 0x0F) << 1) + 8;

	for (; bursts > 0; --bursts) {
		int length = s.getRandomNumber(255);
		s.run(2);

		for (int left = 0x28; left > 0; --left) {
			s.run(2);
			s.countByte(length);
			s.toggle();
			s.run(3);
			s.jump(left > 1);
		}

		s.run(3);
		s.jump(bursts > 1);
	}
}

// A long fall in pitch
void swirl(Speaker &s) {
	for (int low = 0x40; low < 0xC0; ++low) {
		for (int left = 0x1E; left > 0; --left) {
			s.run(2);
			s.countByte(low);
			s.toggle();
			s.run(3);
			s.jump(left > 1);
		}

		s.run(3 + 4);
		s.jump(low + 1 < 0xC0);
	}
}

} // End of anonymous namespace

Common::Array<byte> SoundEffects::render(byte effect, byte arg1, byte arg2) {
	Speaker s;

	switch (effect) {
	case 0xFD:
		sweep(s, arg1, arg2);
		break;
	case 0xFC:
		rumble(s, arg1);
		break;
	case 0xFB:
		descend(s);
		break;
	case 0xFA:
		ascend(s);
		break;
	case 0xF9:
		sweep(s, 0xE0, 4);
		break;
	case 0xF8:
		crackle(s);
		break;
	case 0xF7:
		noise(s, 0xFF, 0);
		break;
	case 0xF6:
		noise(s, 8, 0);
		break;
	case 0xF5:
		warble(s, arg1);
		break;
	case 0xF4:
		swirl(s);
		break;
	default:
		return Common::Array<byte>();
	}

	// The speaker is let go of at the end
	if (s._edges.size() & 1)
		s._edges.push_back(s._cycles);

	Common::Array<byte> samples;
	uint32 total = (uint32)(((uint64)s._cycles * SAMPLE_RATE) / CPU_HZ) + 1;
	samples.resize(total);

	uint32 pos = 0;
	bool high = false;
	for (uint i = 0; i <= s._edges.size(); ++i) {
		uint32 end = (i < s._edges.size()) ? (uint32)(((uint64)s._edges[i] * SAMPLE_RATE) / CPU_HZ) : total;
		byte level = high ? 0x80 + AMPLITUDE : 0x80 - AMPLITUDE;

		for (; pos < end && pos < total; ++pos)
			samples[pos] = level;
		high = !high;
	}

	return samples;
}

} // namespace Ultima3
} // namespace Ultima
