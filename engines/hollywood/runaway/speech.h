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

#ifndef HOLLYWOOD_RUNAWAY_SPEECH_H
#define HOLLYWOOD_RUNAWAY_SPEECH_H

#include "hollywood/runaway/animation.h"
#include "hollywood/runaway/audio.h"

namespace Hollywood {

// Playback of one line; scenes retain their cue ordering and speaker changes.
class RunawaySpeech {
public:
	enum LipSync { kNoLipSync, kTextLipSync, kRecordedLipSync, kRandomLipSync };
	RunawaySpeech(RunawayAudio &audio, Common::RandomSource &random, LipSync mode = kRecordedLipSync) :
		_audio(audio), _random(random), _mode(mode) {}
	void start(const Common::U32String &text, uint16 sample, bool global, uint32 now, byte color = 11, uint minimumDuration = 1000);
	void stop();
	// Scene preparation must not stop audio belonging to the active scene.
	void reset();
	bool finished(uint32 now) const { return (int32)(now - _deadline) >= 0; }
	const Common::U32String &subtitle() const { return _subtitle; }
	byte subtitleColor() const { return _color; }
	byte frame(uint32 now) const { return _lipSync.frame(now); }
	uint frameDuration() const { return _lipSync.frameDuration(); }

private:
	RunawayAudio &_audio;
	Common::RandomSource &_random;
	const LipSync _mode;
	RunawayLipSync _lipSync;
	Common::U32String _subtitle;
	byte _color = 11; // Brian's white speech in the original font palette.
	uint32 _deadline = 0;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_SPEECH_H
