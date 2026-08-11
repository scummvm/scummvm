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
#ifndef TOT_SOUND_H
#define TOT_SOUND_H

#include "audio/decoders/raw.h"
#include "audio/softsynth/pcspk.h"
#include "audio/mixer.h"
#include "common/file.h"

#include "tot/midi.h"

namespace Tot {
const int kSfxId = 1;

class SoundManager {
public:
	SoundManager(Audio::Mixer *mixer);
	~SoundManager();

	void init();
	void syncSoundSettings();
	void loadVoc(Common::String vocFile, int32 startPos, int16 vocSize);
	void autoPlayVoc(Common::String vocFile, int32 startPos, int16 vocSize);
	void playVoc(Common::String vocFile, int32 startPos, uint vocSize);
	void playVoc();
	void stopVoc();
	bool isVocPlaying();
	void playMidi(Common::String fileName, bool loop);
	void playMidi(byte *data, int size, bool loop);
	void toggleMusic();
	void beep(int32 frequency, int32 ms);
	void waitForSoundEnd();
	// Set game logic volume levels
	void setSfxVolume(byte leftVol, byte rightVol);
	void setMidiVolume(byte leftVol, byte rightVol);
	void fadeOutMusic();
	void fadeInMusic();
	void setMasterVolume(byte leftVol, byte rightVol);
	void setSfxBalance(bool left, bool right);
	// Game logic volume settings
	uint _rightSfxVol, _leftSfxVol;
 	uint _musicVolRight, _musicVolLeft;

private:
	// Apply game logic volume levels
	void setSfxVolume(byte volume);
	void setMusicVolume(byte volume);

	Audio::SoundHandle _soundHandle;
	Audio::Mixer *_mixer;
	MidiPlayer *_midiPlayer;
	Audio::PCSpeaker *_speaker;

	Common::SeekableReadStream *_lastSrcStream = nullptr;
	Audio::SeekableAudioStream *_audioStream = nullptr;
};

} // End of namespace Tot
#endif
