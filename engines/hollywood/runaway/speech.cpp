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
#include "hollywood/runaway/speech.h"

namespace Hollywood {

void RunawaySpeech::start(const Common::U32String &text, uint16 sample, bool global, uint32 now, byte color, uint minimumDuration) {
	_subtitle = text;
	_color = color;
	const uint32 duration = sample ? _audio.playSpeech(sample, global) : 0;
	_deadline = now + (duration ? MAX<uint>(minimumDuration, duration + 150) : MAX<uint>(3000, _subtitle.size() * 55));
	if (_mode != kNoLipSync)
		_lipSync.start(_subtitle, now, duration, _random, _mode == kRecordedLipSync ? sample : 0, _mode == kRandomLipSync);
}

void RunawaySpeech::stop() {
	_audio.stopSpeech();
	reset();
}

void RunawaySpeech::reset() {
	_subtitle.clear();
	_lipSync.stop();
}

} // End of namespace Hollywood
