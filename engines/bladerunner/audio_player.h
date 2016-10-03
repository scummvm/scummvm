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

#ifndef BLADERUNNER_AUDIO_H
#define BLADERUNNER_AUDIO_H

#include "audio/mixer.h"
#include "common/array.h"
#include "common/mutex.h"
#include "common/str.h"
#include "common/types.h"

namespace BladeRunner {

class BladeRunnerEngine;
class AudioCache;

#define TRACKS 6

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

	bool  canAllocate(uint32 size);
	bool  dropOldest();
	byte *findByHash(int32 hash);
	void  storeByHash(int32 hash, Common::SeekableReadStream *stream);

	void  incRef(int32 hash);
	void  decRef(int32 hash);
};

class AudioPlayer {
	BladeRunnerEngine *_vm;
	AudioCache *_cache;

	struct Track {
		bool               isMaybeActive;
		Audio::SoundHandle soundHandle;
		int                priority;
		int32              hash;
		int                volume;

		Track() : isMaybeActive(false) {}
	};

	Track _tracks[TRACKS];

	bool isTrackActive(Track *track);
	void fadeAndStopTrack(Track *track, int time);

public:
	AudioPlayer(BladeRunnerEngine *vm);
	~AudioPlayer();

	enum {
		LOOP = 1,
		OVERRIDE_VOLUME = 2
	};

	int playAud(const Common::String &name, int volume, int panFrom, int panTo, int priority, byte flags = 0);

	void stopAll();
};

} // End of namespace BladeRunner

#endif
