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

#ifndef HOLLYWOOD_RUNAWAY_AUDIO_H
#define HOLLYWOOD_RUNAWAY_AUDIO_H

#include "audio/mixer.h"

namespace Hollywood {

class RunawayAudio {
public:
	explicit RunawayAudio(Audio::Mixer *mixer) : _mixer(mixer) {}
	~RunawayAudio();
	void playIntroMusic();
	void playMusic(uint16 cue);
	uint32 playSpeech(uint16 sample, bool global = false);
	void playEffect(uint16 cue, byte volume = Audio::Mixer::kMaxChannelVolume);
	void stopMusic();
	void stopSpeech();
	void stopEffect();
	bool musicPlaying() const;
	bool speechPlaying() const;

private:
	uint32 play(const char *archive, uint16 cue, uint tableCount, bool pairedTables,
		byte flags, Audio::Mixer::SoundType type, Audio::SoundHandle &handle, byte volume, bool loop);
	Audio::Mixer *_mixer;
	Audio::SoundHandle _music, _speech, _effect;
};

} // End of namespace Hollywood

#endif
