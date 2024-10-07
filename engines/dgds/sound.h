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

class Sound {
public:
	Sound(Audio::Mixer *mixer, ResourceManager *resource, Decompressor *decompressor);
	~Sound();

	void playAmigaSfx(const Common::String &filename, byte channel, byte volume);
	void loadMusic(const Common::String &filename);
	void loadMacMusic(const Common::String &filename);
	void loadSFX(const Common::String &filename);

	void playMusic(uint num);
	void stopMusic();
	void unloadMusic();

	void playSFX(uint num);

	void stopSfxForChannel(byte channel);
	void stopSfxByNum(uint num);
	void stopAllSfx();

	bool playPCM(const byte *data, uint32 size);

private:
	void loadPCSound(const Common::String &filename, Common::Array<uint32> &sizeArray, Common::Array<byte *> &dataArray);
	void playPCSound(uint num, const Common::Array<uint32> &sizeArray, const Common::Array<byte *> &dataArray, Audio::Mixer::SoundType soundType);

	void processInitSound(uint num, const byte *data, int dataSz, Audio::Mixer::SoundType soundType);
	void processDisposeSound(uint32 obj);
	void processStopSound(uint32 obj, bool sampleFinishedPlaying);
	void processPlaySound(uint32 obj, bool playBed, bool restoring, const byte *data, int dataSz);
	void initSoundResource(MusicEntry *newSound, const byte *data, int dataSz, Audio::Mixer::SoundType soundType);

	struct Channel _channels[2];

	Common::Array<uint32> _musicSizes;
	Common::Array<byte *> _musicData;
	Common::HashMap<uint16, uint16> _musicIdMap;

	Common::Array<uint32> _sfxSizes;
	Common::Array<byte *> _sfxData;

	SciMusic *_music;

	Audio::Mixer *_mixer;
	ResourceManager *_resource;
	Decompressor *_decompressor;
};

enum {
	DIGITAL_PCM   = 1 << 0,
	TRACK_ADLIB   = 1 << 1,
	TRACK_GM      = 1 << 2,
	TRACK_MT32    = 1 << 3
};

} // End of namespace Dgds

#endif // DGDS_SOUND_H
