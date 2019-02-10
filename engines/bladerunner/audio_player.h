/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef BLADERUNNER_AUDIO_PLAYER_H
#define BLADERUNNER_AUDIO_PLAYER_H

#include "common/array.h"
#include "common/mutex.h"
#include "common/str.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"

namespace BladeRunner {

class BladeRunnerEngine;
class AudioCache;

/*
 * This is a poor imitation of Bladerunner's resource cache
 */
class AudioCache {
	struct cacheItem {
		int32   hash;
		int     refs;
		uint    lastAccess;
		byte   *data;
		uint32  size;
	};

	Common::Mutex            _mutex;
	Common::Array<cacheItem> _cacheItems;

	uint32 _totalSize;
	uint32 _maxSize;
	uint32 _accessCounter;

public:
	AudioCache() :
		_totalSize(0),
		_maxSize(2457600),
		_accessCounter(0) {
	}
	~AudioCache();

	bool  canAllocate(uint32 size) const;
	bool  dropOldest();
	byte *findByHash(int32 hash);
	void  storeByHash(int32 hash, Common::SeekableReadStream *stream);

	void  incRef(int32 hash);
	void  decRef(int32 hash);
};


enum AudioPlayerFlags {
	kAudioPlayerLoop = 1,
	kAudioPlayerOverrideVolume = 2
};

class AudioPlayer {
	static const int kTracks = 6;

	struct Track {
		bool                isActive;
		int                 channel;
		int                 priority;
		int32               hash;
		int                 volume;
		int                 pan;
		Audio::AudioStream *stream;
	};

	BladeRunnerEngine *_vm;

	Common::Mutex _mutex;
	AudioCache   *_cache;
	Track         _tracks[kTracks];
	int           _sfxVolume;

public:
	AudioPlayer(BladeRunnerEngine *vm);
	~AudioPlayer();

	int playAud(const Common::String &name, int volume, int panStart, int panEnd, int priority, byte flags = 0, Audio::Mixer::SoundType type = Audio::Mixer::kSFXSoundType);
	bool isActive(int track) const;
	void stop(int track, bool immediately);
	void stopAll();
	void adjustVolume(int track, int volume, int delay, bool overrideVolume);
	void adjustPan(int track, int pan, int delay);

	void setVolume(int volume);
	int getVolume() const;
	void playSample();

private:
	void remove(int channel);
	static void mixerChannelEnded(int channel, void *data);
};

} // End of namespace BladeRunner

#endif
