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

#include "common/util.h"
#include "hollywood/runaway/animation.h"

namespace Hollywood {
namespace {

// The original 0x40fef0 generates one mouth shape per text character. Work
// on decoded letters so accented French, German and Spanish vowels also
// select the appropriate shape, independently of the source code page.
uint32 speechLetter(uint32 c) {
	if ((c >= 'A' && c <= 'Z') || (c >= 0xc0 && c <= 0xde && c != 0xd7))
		c += 0x20;
	switch (c) {
	case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: return 'a';
	case 0xe8: case 0xe9: case 0xea: case 0xeb: return 'e';
	case 0xec: case 0xed: case 0xee: case 0xef: return 'i';
	case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: return 'o';
	case 0xf9: case 0xfa: case 0xfb: case 0xfc: return 'u';
	case 0xe7: return 'c';
	case 0xf1: return 'n';
	default: return c;
	}
}

byte speechVowel(uint32 c) {
	switch (c) {
	case 'a': return 1;
	case 'e': return 2;
	case 'i': return 3;
	case 'o': return 4;
	case 'u': return 5;
	default: return 0;
	}
}

byte speechMouth(uint32 c, uint32 next, byte previous) {
	const byte vowel = speechVowel(c), nextVowel = speechVowel(next);
	if (vowel)
		return vowel;
	// Native 0x40fd30 anticipates the next vowel for most consonants, closes
	// for B/P/V, and retains the previous shape at spaces and some endings.
	switch (c) {
	case 'b': case 'p': case 'v': return 0;
	case 'f': return 5;
	case 'l': return next == 'l' ? 3 : nextVowel ? nextVowel : previous;
	case 'm': return nextVowel ? 0 : previous;
	case 's': return nextVowel ? nextVowel : previous;
	case ' ': return previous;
	default:
		if ((c >= 'a' && c <= 'z') || c == 0xdf)
			return next == ' ' ? previous : nextVowel;
		return 0;
	}
}

} // End of anonymous namespace

void RunawayLipSync::start(const Common::U32String &text, uint32 now, uint32 duration, Common::RandomSource &random) {
	_started = now;
	_duration = duration ? duration : text.size() * 80;
	_frames.resize(text.size());
	if (_frames.empty())
		return;
	// The native timer advances in 10 ms ticks, rounding up at a fractional
	// tick of 0.3. Without a voice sample it uses 80 ms per character.
	_frameDuration = duration ? MAX<uint>(1, (duration / _frames.size() + 7) / 10) * 10 : 80;
	for (uint i = 0; i < _frames.size(); ++i) {
		const uint32 c = speechLetter(text[i]);
		const uint32 next = i + 1 < text.size() ? speechLetter(text[i + 1]) : 0;
		_frames[i] = speechMouth(c, next, i ? _frames[i - 1] : 0);
	}
	// Native 0x412680 occasionally selects the closed-eye set (6..11) for
	// four consecutive characters, then returns to the open-eye set (0..5).
	bool blink = false;
	for (uint i = 0; i < _frames.size(); i += 4) {
		blink = !blink && random.getRandomNumber(4) == 0;
		if (blink) {
			for (uint j = i; j < MIN<uint>(i + 4, _frames.size()); ++j)
				_frames[j] += 6;
		}
	}
}

byte RunawayLipSync::frame(uint32 now) const {
	const uint32 elapsed = now - _started;
	const uint index = elapsed / _frameDuration;
	return elapsed < _duration && index < _frames.size() ? _frames[index] : 0;
}

void RunawayIdleHead::reset(uint32 now) {
	_turnTick = _blinkTick = now;
	_step = _target = 6;
	_frame = 0;
	_blink = _active = false;
}

void RunawayIdleHead::pause(uint32 elapsed) {
	_turnTick += elapsed;
	_blinkTick += elapsed;
}

void RunawayIdleHead::update(uint32 now, bool enabled, uint blinkInterval, Common::RandomSource &random) {
	if (!enabled || !_active) {
		reset(now);
		_active = enabled;
		return;
	}
	// Native 0x42a610 moves between three poses on a 60 ms clock. The
	// center is index 6 in the transition table at 0x464660. Blinks use
	// 0x42ab50's separate timer, retaining the last speech character period.
	static const byte frames[] = {24, 23, 22, 21, 20, 19, 0, 12, 13, 14, 15, 16, 17};
	blinkInterval = MAX<uint>(10, blinkInterval);
	while (now - _turnTick >= 60 || now - _blinkTick >= blinkInterval) {
		if (now - _turnTick >= 60 && (int32)((_turnTick + 60) - (_blinkTick + blinkInterval)) <= 0) {
			_turnTick += 60;
			if (_step != _target) {
				_step += _target > _step ? 1 : -1;
				_frame = frames[_step];
			} else if (!_blink) {
				const uint choice = random.getRandomNumber(499);
				if (_step == 6) {
					if (choice < 4)
						_target = choice < 2 ? 0 : 12;
				} else if (choice < 20) {
					_target = choice < 2 ? 12 - _step : 6;
				}
			}
		} else {
			_blinkTick += blinkInterval;
			if (_blink) {
				_blink = false;
				_frame = frames[_step];
			} else if (_step == _target && random.getRandomNumber(14) == 0) {
				_blink = true;
				_frame = frames[_step] + (_step == 6 ? 6 : 1);
			}
		}
	}
}

} // End of namespace Hollywood
