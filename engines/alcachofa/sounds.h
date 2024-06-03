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

#ifndef SOUNDS_H
#define SOUNDS_H

#include "common.h"
#include "audio/mixer.h"

namespace Alcachofa {

class Character;

using SoundID = uint32;
static constexpr SoundID kInvalidSoundID = 0;
class Sounds {
public:
	Sounds();
	~Sounds();

	void update();
	SoundID playVoice(const Common::String &fileName, byte volume = Audio::Mixer::kMaxChannelVolume);
	void stopVoice();
	void fadeOut(SoundID id, uint32 duration);
	bool isAlive(SoundID id);
	void setVolume(SoundID id, byte volume);
	void setAppropriateVolume(SoundID id,
		MainCharacterKind processCharacter,
		Character *speakingCharacter);

private:
	struct Playback {
		Playback(uint32 id, Audio::SoundHandle handle, Audio::Mixer::SoundType type);

		SoundID _id;
		Audio::SoundHandle _handle;
		Audio::Mixer::SoundType _type;
		uint32 _fadeStart = 0,
			_fadeDuration = 0;
	};
	Playback *getPlaybackById(SoundID id);

	Common::Array<Playback> _playbacks;
	Audio::Mixer *_mixer;
	SoundID _nextID = 1;
};

}

#endif // SOUNDS_H
