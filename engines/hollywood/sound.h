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

#ifndef HOLLYWOOD_SOUND_H
#define HOLLYWOOD_SOUND_H

#include "audio/mixer.h"
#include "common/rect.h"

namespace Hollywood {

// Owns a mixer channel. Optional positions describe the horizontal (x, z)
// sound plane: Point::y is distance in front of the listener, not screen y.
class SoundChannel {
public:
	enum PositionMode {
		kSpatial,
		kStereoPan
	};

	SoundChannel() = default;
	~SoundChannel();
	SoundChannel(const SoundChannel &) = delete;
	SoundChannel &operator=(const SoundChannel &) = delete;

	void play(Audio::Mixer *mixer, Audio::Mixer::SoundType type, Audio::AudioStream *stream,
		byte volume, const Common::Point *position = nullptr, int listenerX = 0, float rolloff = 1.0f,
		PositionMode mode = kSpatial);
	void stop();
	bool isPlaying() const;
	void setVolume(byte volume);
	void setListenerPosition(int x);

private:
	void mixerLevels(byte &volume, int8 &balance) const;
	void updateVolume();

	Audio::Mixer *_mixer = nullptr;
	Audio::SoundHandle _handle;
	byte _volume = 0;
	bool _positional = false;
	Common::Point _position;
	int _listenerX = 0;
	float _rolloff = 1.0f;
	PositionMode _positionMode = kSpatial;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SOUND_H
