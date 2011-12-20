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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef SCUMM_PLAYER_APPLEII_H
#define SCUMM_PLAYER_APPLEII_H

#include "common/mutex.h"
#include "common/scummsys.h"
#include "common/memstream.h"
#include "scumm/music.h"
#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/softsynth/sid.h"

namespace Scumm {

class ScummEngine;

class DynamicMemoryStream {
public:
	DynamicMemoryStream() : _data(0) {
		clear();
	}

	~DynamicMemoryStream() {
		free(_data);
	}

	void clear() {
		free(_data);
		_data = 0;
		_capacity = 0;
		_size = 0;
		_ptr = 0;
		_pos = 0;
		_readPos = 0;
	}

	void ensureCapacity(uint32 new_len) {
		if (new_len <= _capacity)
			return;

		byte *old_data = _data;

		_capacity *= 2;
		if (_capacity < new_len + 2048)
			_capacity = new_len + 2048;
		_data = (byte *)malloc(_capacity);
		_ptr = _data + _pos;

		if (old_data) {
			// Copy old data
			memcpy(_data, old_data, _size);
			free(old_data);
		}

		_size = new_len;
	}

	uint32 availableSize() const {
		if (_readPos >= _size)
			return 0;
		return _size - _readPos;
	}

	virtual uint32 write(const void *dataPtr, uint32 dataSize) {
		ensureCapacity(_pos + dataSize);
		memcpy(_ptr, dataPtr, dataSize);
		_ptr += dataSize;
		_pos += dataSize;
		if (_pos > _size)
			_size = _pos;
		return dataSize;
	}

	uint32 read(byte *dataPtr, uint32 dataSize) const {
		uint32 avail = availableSize();
		if (avail == 0)
			return 0;
		if (dataSize > avail)
			dataSize = avail;
		memcpy(dataPtr, _data + _readPos, dataSize);
		_readPos += dataSize;
		return dataSize;
	}

private:
	mutable uint32 _readPos;
	uint32 _capacity;
	uint32 _size;
	byte *_ptr;
	byte *_data;
	uint32 _pos;
};

// CPU_CLOCK according to AppleWin
static const double APPLEII_CPU_CLOCK = 1020484.5; // ~ 1.02 MHz

class SampleConverter {
private:
	void addSampleToBuffer(int sample) {
		int16 value = sample * _volume / _maxVolume;
		_buffer.write(&value, sizeof(value));
	}

public:
	SampleConverter() : 
		_cyclesPerSampleFP(0),
		_missingCyclesFP(0),
		_sampleCyclesSumFP(0),
		_volume(_maxVolume)
	{}

	~SampleConverter() {}

	void reset() {
		_missingCyclesFP = 0;
		_sampleCyclesSumFP = 0;
		_buffer.clear();	
	}

	uint32 availableSize() const {
		return _buffer.availableSize();
	}

	void setMusicVolume(int vol) {
		assert(vol >= 0 && vol <= _maxVolume);
		_volume = vol;
	}

	void setSampleRate(int rate) {
		/* ~46 CPU cycles per sample @ 22.05kHz */
		_cyclesPerSampleFP = APPLEII_CPU_CLOCK * (1 << PREC_SHIFT) / rate;
		reset();
	}

	void addCycles(byte level, const int cycles) {
		/* convert to fixed precision floats */
		int cyclesFP = cycles << PREC_SHIFT;

		// step 1: if cycles are left from the last call, process them first
		if (_missingCyclesFP > 0) {
			int n = (_missingCyclesFP < cyclesFP) ? _missingCyclesFP : cyclesFP;
			if (level)
				_sampleCyclesSumFP += n;
			cyclesFP -= n;
			_missingCyclesFP -= n;
			if (_missingCyclesFP == 0) {
				addSampleToBuffer(2*32767 * _sampleCyclesSumFP / _cyclesPerSampleFP - 32767);
			} else {
				return;
			}
		}

		_sampleCyclesSumFP = 0;

		// step 2: process blocks of cycles fitting into a whole sample
		while (cyclesFP >= _cyclesPerSampleFP) {
			addSampleToBuffer(level ? 32767 : -32767);
			cyclesFP -= _cyclesPerSampleFP;
		}

		// step 3: remember cycles left for next call
		if (cyclesFP > 0) {
			_missingCyclesFP = _cyclesPerSampleFP - cyclesFP;
			if (level)
				_sampleCyclesSumFP = cyclesFP;
		}
	}

	uint32 readSamples(void *buffer, int numSamples) {
		return _buffer.read((byte*)buffer, numSamples * 2) / 2;
	}

#if 0
	void logToFile(Common::String fileName) {
		FILE *f;
		byte buffer[512];
		int n;

		f = fopen(fileName.c_str(), "wb");
		while ((n = _buffer.read(buffer, 512)) != 0) {
			fwrite(buffer, 1, n, f);
		}
		fclose(f);
	}
#endif

private:
	static const int PREC_SHIFT = 7;

private:
	int _cyclesPerSampleFP;   /* (fixed precision) */
	int _missingCyclesFP;     /* (fixed precision) */
	int _sampleCyclesSumFP;   /* (fixed precision) */
	int _volume; /* 0 - 256 */
	static const int _maxVolume = 256;
	DynamicMemoryStream _buffer;
};

class AppleII_SoundFunction;

class Player_AppleII : public Audio::AudioStream, public MusicEngine {
public:
	Player_AppleII(ScummEngine *scumm, Audio::Mixer *mixer);
	virtual ~Player_AppleII();

	virtual void setMusicVolume(int vol) { _sampleConverter.setMusicVolume(vol); }
	void setSampleRate(int rate) {
		_sampleRate = rate;
		_sampleConverter.setSampleRate(rate); 
	}
	void startMusic(int songResIndex);
	virtual void startSound(int sound);
	virtual void stopSound(int sound);
	virtual void stopAllSounds();
	virtual int  getSoundStatus(int sound) const;
	virtual int  getMusicTimer();

	// AudioStream API
	int readBuffer(int16 *buffer, const int numSamples);
	bool isStereo() const { return false; }
	bool endOfData() const { return false; }
	int getRate() const { return _sampleRate; }

public:
	void speakerToggle();
	void generateSamples(int cycles);
	void wait(int interval, int count);

private:
	struct sound_state {
		// sound number
		int soundNr;
		// type of sound
		int type;
		// number of loops left
		int loop;
		// global sound param list
		const byte *params;
		// speaker toggle state (0 / 1)
		byte speakerState;
		// sound function
		AppleII_SoundFunction *soundFunc;
	} _state;

	ScummEngine *_vm;
	Audio::Mixer *_mixer;
	Audio::SoundHandle _soundHandle;
	int _sampleRate;
	Common::Mutex _mutex;

private:
	SampleConverter _sampleConverter;

private:
	void resetState();
	bool updateSound();
};

class AppleII_SoundFunction {
public:
	AppleII_SoundFunction() {}
	virtual ~AppleII_SoundFunction() {}
	virtual void init(Player_AppleII *player, const byte *params) = 0;
	/* returns true if finished */
	virtual bool update() = 0;
protected:
	Player_AppleII *_player;
};

} // End of namespace Scumm

#endif
