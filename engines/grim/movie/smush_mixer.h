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

#ifndef GRIM_MOVIE_SMUSH_MIXER_H
#define GRIM_MOVIE_SMUSH_MIXER_H

#include "common/scummsys.h"
#include "common/mutex.h"
#include "audio/audiostream.h"
#include "audio/mixer.h"

namespace Grim {

#define SMUSH_RING_BLOCK_SIZE 6144

class SmushTimer;
class SmushSound;

class SmushMixer {

private:
	int32 *_mixBuf;
	int _mixBufSize;
	int _outChannelCount;
	byte _mixerTable[17 * 17];

	void mixMono8(const uint8 *srcBuf, int32 inFrameCount, uint32 feedSize, int32 mixBufStartIndex, int leftAmp, int rightAmp);
	void mixStereo8(const uint8 *srcBuf, int32 inFrameCount, uint32 feedSize, int32 mixBufStartIndex, int volume);
	void mixMono16(const int16 *srcBuf, int32 inFrameCount, uint32 feedSize, int32 mixBufStartIndex, int leftAmp, int rightAmp);
	void mixStereo16(const int16 *srcBuf, int32 inFrameCount, uint32 feedSize, int32 mixBufStartIndex, int volume);
	void mixMono16Unsigned(const uint16 *srcBuf, int32 inFrameCount, uint32 feedSize, int32 mixBufStartIndex, int leftAmp, int rightAmp);
	void mixStereo16Unsigned(const uint16 *srcBuf, int32 inFrameCount, uint32 feedSize, int32 mixBufStartIndex, int volume);
	void mixMono16Swap(const uint16 *srcBuf, int32 inFrameCount, uint32 feedSize, int32 mixBufStartIndex, int leftAmp, int rightAmp);
	void mixStereo16Swap(const uint16 *srcBuf, int32 inFrameCount, uint32 feedSize, int32 mixBufStartIndex, int volume);

public:
	SmushMixer();
	~SmushMixer();

	int souMixerInitialize(int outChannelCount, int mixBufSize);
	int souMixerTerminate();
	int souMixerStart();
	void souMixerMix(const uint8 *srcBuf, int32 inFrameCount, int wordSize, int channelCount, uint32 feedSize, int32 mixBufStartIndex, int volume, int pan, int signedFlag, int swapEndiannessFlag);
	int souMixerEnd(int16 *destBuf, int frameCount);

};

class SmushDriver;

class SmushDriverStream : public Audio::AudioStream {

private:
	SmushDriver *_driver;

public:
	SmushDriverStream(SmushDriver *driver);

	int readBuffer(int16 *buffer, const int numSamples) override;
	bool isStereo() const override { return true; }
	int getRate() const override { return 22050; }
	bool endOfData() const override { return false; }

};

class SmushDriver {

private:
	SmushMixer _mixer;
	SmushTimer *_timer;
	SmushSound *_sound;
	Audio::Mixer *_system;
	Audio::SoundHandle _soundHandle;
	Common::Mutex _ringMutex;
	byte *_ring;
	int _ringBlockCount;
	int _ringPlayPosition;
	int _lastBlock;
	int _driverRate;
	int _driverVolume;
	int _fillTimer;
	int _volumeTable[128];
	bool _volumeTableBuilt;

	int sndDrivOpenRing();
	void sndDrivCloseRing();
	int sndDrivGetPlayBlock();
	int sndDrivMixerInitialize(int arg, int size);
	void sndDrivMixerTerminate();
	int sndDrivMixerEnd(int16 *destBuf, int frameCount);

public:
	SmushDriver(SmushTimer *timer);
	~SmushDriver();

	int sndDrivInitialize(int rate, int channelCount, SmushSound *sound);
	void sndDrivTerminate();
	int sndDrivSetVol(int volume);
	int sndDrivGetVol();
	int sndDrivGetRate();
	int sndDrivDummy(int rate);
	void sndDrivFillThread();
	void sndDrivMix(const uint8 *srcBuf, int32 mixBufStartIndex, uint32 feedSize, int32 srcByteCount, int reserved, int volume, int pan, int format);
	static void sndDrivFillThreadCallback(int timer, void *context);

	int ringRead(int16 *buffer, int numSamples);

};

struct SmushDemoAudio {
	byte *data;
	int produced;
	int consumed;
	int size;
	bool opened;
};

class SmushDemoDriver;

class SmushDemoDriverStream : public Audio::AudioStream {

private:
	SmushDemoDriver *_driver;

public:
	SmushDemoDriverStream(SmushDemoDriver *driver);

	int readBuffer(int16 *buffer, const int numSamples) override;
	bool isStereo() const override { return true; }
	int getRate() const override { return 22050; }
	bool endOfData() const override { return false; }

};

class SmushDemoDriver {

private:
	Audio::Mixer *_system;
	Audio::SoundHandle _soundHandle;
	Common::Mutex _mutex;
	byte *_buffer;
	int _bufferSize;
	int _playPosition;
	bool _playing;
	bool _created;
	int _rate;
	int _blockAlign;
	int _written;
	int _wrapCount;
	int _lastPosition;
	int _lastTime;
	uint32 _lastMeasure;
	int _lastReturned;
	int _waiting;
	int _paused;
	int _threshold;
	double _msPerByte;

public:
	SmushDemoDriver();
	~SmushDemoDriver();

	void sndDrivCreate();
	void sndDrivRelease();
	int sndDrivOpen(int rate, int channels, int bits, SmushDemoAudio *audio);
	void sndDrivClose(SmushDemoAudio *audio);
	int sndDrivGetTime(SmushDemoAudio *audio);
	int sndDrivPause(SmushDemoAudio *audio);
	int sndDrivResume(SmushDemoAudio *audio);
	void sndDrivFill(SmushDemoAudio *audio);

	int ringRead(int16 *buffer, int numSamples);

};

} // End of namespace Grim
#endif
