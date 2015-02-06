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

#include "audio_player.h"

#include "bladerunner/archive.h"
#include "bladerunner/aud_decoder.h"

#include "bladerunner/bladerunner.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"

#include "common/array.h"
#include "common/debug.h"
#include "common/mutex.h"
#include "common/stream.h"

namespace Common {
	class MemoryReadStream;
}

namespace BladeRunner {

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
		_accessCounter(0)
	{}
	~AudioCache();

	bool  canAllocate(uint32 size);
	bool  dropOldest();
	byte *findByHash(int32 hash);
	void  storeByHash(int32 hash, Common::SeekableReadStream *stream);

	void  incRef(int32 hash);
	void  decRef(int32 hash);
};

AudioCache::~AudioCache() {
	for (uint i = 0; i != _cacheItems.size(); ++i) {
		free(_cacheItems[i].data);
	}
}

bool AudioCache::canAllocate(uint32 size) {
	Common::StackLock lock(_mutex);

	return _maxSize - _totalSize >= size;
}

bool AudioCache::dropOldest() {
	Common::StackLock lock(_mutex);

	if (_cacheItems.size() == 0)
		return false;

	uint oldest = 0;
	for (uint i = 1; i != _cacheItems.size(); ++i) {
		if (_cacheItems[i].refs == 0 && _cacheItems[i].lastAccess < _cacheItems[oldest].lastAccess)
			oldest = i;
	}

	free(_cacheItems[oldest].data);
	_totalSize -= _cacheItems[oldest].size;
	_cacheItems.remove_at(oldest);
	return true;
}

byte *AudioCache::findByHash(int32 hash) {
	Common::StackLock lock(_mutex);

	for (uint i = 0; i != _cacheItems.size(); ++i) {
		if (_cacheItems[i].hash == hash) {
			_cacheItems[i].lastAccess = _accessCounter++;
			return _cacheItems[i].data;
		}
	}

	return NULL;
}

void AudioCache::storeByHash(int32 hash, Common::SeekableReadStream *stream) {
	Common::StackLock lock(_mutex);

	uint32 size = stream->size();
	byte *data = (byte*)malloc(size);
	stream->read(data, size);

	cacheItem item = {
		hash,
		0,
		_accessCounter++,
		data,
		size
	};

	_cacheItems.push_back(item);
	_totalSize += size;
}

void AudioCache::incRef(int32 hash) {
	Common::StackLock lock(_mutex);

	for (uint i = 0; i != _cacheItems.size(); ++i) {
		if (_cacheItems[i].hash == hash) {
			_cacheItems[i].refs++;
			return;
		}
	}
	assert(0 && "AudioCache::incRef: hash not found");
}

void AudioCache::decRef(int32 hash) {
	Common::StackLock lock(_mutex);

	for (uint i = 0; i != _cacheItems.size(); ++i) {
		if (_cacheItems[i].hash == hash) {
			assert(_cacheItems[i].refs > 0);
			_cacheItems[i].refs--;
			return;
		}
	}
	assert(0 && "AudioCache::decRef: hash not found");
}

class AudStream : public Audio::RewindableAudioStream {
	byte       *_data;
	byte       *_p;
	byte       *_end;
	AudioCache *_cache;
	int32       _hash;
	byte        _compressionType;
	uint16      _deafBlockRemain;

	ADPCMWestwoodDecoder _decoder;

public:
	AudStream(AudioCache *cache, int32 hash)
		: _cache(cache), _hash(hash)
	{
		_data = _cache->findByHash(_hash);
		_end = _data + READ_LE_UINT32(_data + 2) + 12;
		_cache->incRef(_hash);

		assert(_end - _data >= 12);

		_compressionType = *(_data + 11);

		_deafBlockRemain = 0;
		_p = _data + 12;
	}
	~AudStream() {
		_cache->decRef(_hash);
	}

	int readBuffer(int16 *buffer, const int numSamples);
	bool isStereo() const { return false; }
	int getRate() const { return READ_LE_UINT16(_data); };
	bool endOfData() const { return _p == _end; }
	bool rewind();
};

int AudStream::readBuffer(int16 *buffer, const int numSamples) {
	int samplesRead = 0;

	assert(numSamples % 2 == 0);

	if (_compressionType == 99) {
		while (samplesRead < numSamples) {
			if (_deafBlockRemain == 0) {
				if (_end - _p == 0)
					break;

				assert(_end - _p >= 6);

				uint16 blockSize     = READ_LE_UINT16(_p);
				uint16 blockOutSize  = READ_LE_UINT16(_p + 2);
				uint32 sig           = READ_LE_UINT32(_p + 4);
				_p += 8;

				assert(sig == 0xdeaf);
				assert(_end - _p >= blockSize);
				assert(blockOutSize = 4 * blockSize);

				_deafBlockRemain = blockSize;
			}

			assert(_end - _p >= _deafBlockRemain);

			int bytesConsumed = MIN<int>(_deafBlockRemain, (numSamples - samplesRead) / 2);

			_decoder.decode(_p, bytesConsumed, buffer + samplesRead);
			_p += bytesConsumed;
			_deafBlockRemain -= bytesConsumed;

			samplesRead += 2 * bytesConsumed;
		}
	} else {
		assert(0 && "readBuffer: Unimplemented");
	}

	return samplesRead;
}

bool AudStream::rewind() {
	_p = _data + 12;
	_decoder.setParameters(0, 0);
	return true;
}

AudioPlayer::AudioPlayer(BladeRunnerEngine *vm)
	: _vm(vm)
{
	_cache = new AudioCache();

	for (int i = 0; i != 6; ++i) {
		_tracks[i].hash = 0;
		_tracks[i].priority = 0;
	}
}

AudioPlayer::~AudioPlayer() {
	delete _cache;
}

bool AudioPlayer::isTrackActive(Track *track)
{
	if (!track->isMaybeActive)
		return false;

	return track->isMaybeActive = _vm->_mixer->isSoundHandleActive(track->soundHandle);
}

void AudioPlayer::stopAll()
{
	for (int i = 0; i != TRACKS; ++i) {
		_vm->_mixer->stopHandle(_tracks[i].soundHandle);
	}
}

void AudioPlayer::fadeAndStopTrack(Track *track, int time)
{
	(void)time;

	_vm->_mixer->stopHandle(track->soundHandle);
}

int AudioPlayer::playAud(const Common::String &name, int volume, int panFrom, int panTo, int priority, byte flags) {
	/* Find first available track or, alternatively, the lowest priority playing track */
	int    trackId;
	Track *track = NULL;
	int    lowestPriority = 100;
	Track *lowestPriorityTrack = NULL;

	for (int i = 0; i != 6; ++i) {
		track = &_tracks[i];
		if (!isTrackActive(track)) {
			trackId = 1;
			break;
		}

		if (track->priority < lowestPriority) {
			lowestPriority = track->priority;
			lowestPriorityTrack = track;
		}
	}

	/* If there's no available track, stop the lowest priority track if it's lower than
	 * the new priority
	 */
	if (track == NULL && lowestPriority < priority) {
		fadeAndStopTrack(lowestPriorityTrack, 1);
		track = lowestPriorityTrack;
	}

	/* If there's still no available track, give up */
	if (track == NULL)
		return -1;

	/* Load audio resource and store in cache. Playback will happen directly from there. */
	int32 hash = mix_id(name);
	if (!_cache->findByHash(hash)) {
		Common::SeekableReadStream *r = _vm->getResourceStream(name);
		if (!r)
			return -1;

		int32 size = r->size();
		while (!_cache->canAllocate(size)) {
			if (!_cache->dropOldest()) {
				delete r;
				return -1;
			}
		}
		_cache->storeByHash(hash, r);
		delete r;
	}

	AudStream *audStream = new AudStream(_cache, hash);

	Audio::AudioStream *audioStream = audStream;
	if (flags & LOOP) {
		audioStream = new Audio::LoopingAudioStream(audStream, 0, DisposeAfterUse::YES);
	}

	Audio::SoundHandle soundHandle;

	debug("PlayStream: %s", name.c_str());

	int balance = panFrom;

	_vm->_mixer->playStream(
		Audio::Mixer::kPlainSoundType,
		&soundHandle,
		audioStream,
		-1,
		volume * 255 / 100,
		balance);

	track->isMaybeActive = true;
	track->soundHandle   = soundHandle;
	track->priority      = priority;
	track->hash          = hash;
	track->volume        = volume;

	return trackId;
}

} // End of namespace BladeRunner
