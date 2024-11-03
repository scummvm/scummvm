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

#ifndef DGDS_SOUND_H
#define DGDS_SOUND_H

#include "common/scummsys.h"
#include "audio/audiostream.h"
#include "audio/mixer.h"

namespace Dgds {

class ResourceManager;
class Decompressor;
class DgdsMidiPlayer;
class SciMusic;
class MusicEntry;
class AudioPlayer;

struct Channel {
	Audio::AudioStream *stream;
	Audio::SoundHandle handle;
	byte volume;
};

struct SoundData {
	SoundData() : _size(0), _data(nullptr), _flags(0) {}
	uint32 _size;
	const byte *_data;
	uint16 _flags;
};

class Sound {
public:
	Sound(Audio::Mixer *mixer, ResourceManager *resource, Decompressor *decompressor);
	~Sound();

	void playAmigaSfx(const Common::String &filename, byte channel, byte volume);
	bool loadMusic(const Common::String &filename);
	void loadSFX(const Common::String &filename);

	void playMusic(int num);
	void stopMusic();
	void unloadMusic();

	void playSFX(int num);

	void playMusicOrSFX(int num);
	void stopMusicOrSFX(int num);

	void stopSfxForChannel(byte channel);
	void stopSfxByNum(int num);
	void stopAllSfx();

	bool playPCM(const byte *data, uint32 size);

private:
	void loadSNGSoundData(const Common::String &filename, Common::Array<SoundData> &dataArray);
	bool loadSXSoundData(const Common::String &filename, Common::Array<SoundData> &dataArray, Common::HashMap<uint16, uint16> &idMap);

	void playPCSound(int num, const Common::Array<SoundData> &dataArray, Audio::Mixer::SoundType soundType);

	void processInitSound(uint32 obj, const SoundData &data, Audio::Mixer::SoundType soundType);
	void processDisposeSound(uint32 obj);
	void processStopSound(uint32 obj, bool sampleFinishedPlaying);
	void processPlaySound(uint32 obj, bool playBed, bool restoring, const SoundData &data);
	void initSoundResource(MusicEntry *newSound, const SoundData &data, Audio::Mixer::SoundType soundType);

	int mapSfxNum(int num) const;
	int mapMusicNum(int num) const;

	struct Channel _channels[2];

	Common::Array<SoundData> _musicData;
	Common::HashMap<uint16, uint16> _musicIdMap;

	Common::Array<SoundData> _sfxData;
	Common::HashMap<uint16, uint16> _sfxIdMap;

	Common::String _currentMusic;

	SciMusic *_music;

	Audio::Mixer *_mixer;
	ResourceManager *_resource;
	Decompressor *_decompressor;
};

enum {
	DIGITAL_PCM   = 1 << 0,
	TRACK_ADLIB   = 1 << 1,
	TRACK_GM      = 1 << 2,
	TRACK_MT32    = 1 << 3,
	TRACK_CMS     = 1 << 4,
	TRACK_PCSPK   = 1 << 5,
	TRACK_TANDY   = 1 << 6,
};

} // End of namespace Dgds

#endif // DGDS_SOUND_H
