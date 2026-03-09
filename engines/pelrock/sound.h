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

#ifndef PELROCK_SOUND_H
#define PELROCK_SOUND_H

#include "audio/mixer.h"
#include "common/file.h"
#include "common/scummsys.h"
#include "common/str.h"

namespace Pelrock {

struct SonidoFile {
	Common::String filename;
	uint32 offset;
	uint32 size;
	byte *data;
};

extern const char *SOUND_FILENAMES[];

enum SoundFormat {
	SOUND_FORMAT_RAWPCM,
	SOUND_FORMAT_MILES,
	SOUND_FORMAT_MILES2,
	SOUND_FORMAT_RIFF,
	SOUND_FORMAT_INVALID
};

struct SoundData {
	SoundFormat format;
	int sampleRate;
	byte *data;
	uint32 size;
};

const int kMaxChannels = 15;
const int kAmbientSoundSlotBase = 4; // Room sound indices 4-7 are ambient sounds

class SoundManager {
public:
	SoundManager(Audio::Mixer *mixer);
	~SoundManager();
	void playSound(byte index, int channel = -1, int loopCount = 1);
	void playSound(const char *filename, int channel, int loopCount = 1);
	void playSound(byte *soundData, uint32 size, int channel);
	void stopAllSounds();
	void stopSound(int channel);
	void setVolume(int volume);
	bool isPlaying() const;
	bool isPlaying(int channel) const;
	void loadSoundIndex();

	bool isMusicPlaying();
	void playMusicTrack(int trackNumber, bool loop = true);
	void stopMusic();
	void pauseMusic();

	/**
	 * Check if ambient sound should play this frame.
	 * @param frameCount Current game frame counter
	 * @return Ambient slot offset (0-3) to play, or -1 if no sound this frame
	 *         Add kAmbientSoundSlotBase (4) to get room sound index
	 */
	int tickAmbientSound(uint32 frameCount);

	bool isPaused() const { return _isPaused; }
	byte getCurrentMusicTrack() const { return _currentMusicTrack; }

private:
	void playSound(SonidoFile sound, int channel = -1, int loopCount = 1);
	SoundFormat detectFormat(byte *data, uint32 size);
	int getSampleRate(byte *data, SoundFormat format);
	int findFreeChannel();

private:
	Audio::Mixer *_mixer;
	int _currentVolume;
	Audio::SoundHandle _musicHandle;
	Audio::SoundHandle _sfxHandles[kMaxChannels];
	Common::HashMap<Common::String, SonidoFile> _soundMap;
	bool _isPaused = false;
	byte _currentMusicTrack = 0;


	uint32 _cdTrackStart = 0;
	uint32 _cdTrackDuration;
	uint32 _cdPlayStartTime; // time at the moment of calling play()

};

} // End of namespace Pelrock

#endif // PELROCK_SOUND_H
