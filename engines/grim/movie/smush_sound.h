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

#ifndef GRIM_MOVIE_SMUSH_SOUND_H
#define GRIM_MOVIE_SMUSH_SOUND_H

#include "common/scummsys.h"

namespace Grim {

#define SOU_MAX_CHANNELS 16
#define SOU_FREE_ID -255

class SmushTimer;
class SmushDriver;

struct SmushSoundChannel {
	byte *dataBuffer;
	byte *stagingBuffer;
	byte *scriptPointer;
	byte *dataArea;
	int capacity;
	uint8 volume;
	int8 pan;
	uint8 state;
	uint8 format;
	int soundClass;
	int soundId;
	int age;
	int dataAreaSize;
	int writePosition;
	int consumedPosition;
	int totalSize;
	int tag1;
	int chunkCounter;
	int tag2;
};

struct SmushSoundPlayback {
	byte *script;
	byte *dataStart;
	int dataSize;
	int readPosition;
	int remaining;
	int rate;
	uint8 format;
	int previousState;
	int previousRate;
	int previousVolume;
	int tailBytes;
	uint8 previousFormat;
	int gain;
	int repeatAccumulator;
	int loopRemaining;
};

class SmushSound {

private:
	SmushTimer *_timer;
	SmushDriver *_driver;
	SmushSoundChannel _channels[SOU_MAX_CHANNELS];
	SmushSoundPlayback _playback[SOU_MAX_CHANNELS];
	int _channelCount;
	int _masterVolume;
	int _sampleRate;
	int _duckFloor;
	int _duckRate;
	int _duckLevel;
	int _resetFlag;
	int _bytesPerFrame[6];
	uint8 _groupVolumes[256];
	int _pauseCount;
	int _engineOk;
	int _syncMark;
	double _syncAccumulator;
	int _fillCounter;
	int _musicParam;
	int _voiceParam;
	int _normalParam;
	int _panInvert;
	int _driverRate;
	int _reserveRam;
	int _bufferSizes[SOU_MAX_CHANNELS + 1];

	int engineInitialize(int rate, int reserveRam, const int *bufferSizes);
	int engineTerminate();
	int engineAllocChannel(int size, int reserveRam);
	uint8 getGroupVolume(uint8 index);
	void setGroupVolume(uint8 index, uint8 volume);
	int souEngineMarkSyncTime();
	int souEngineExternalSync(double ticks);
	int souEngineStartCreative(const byte *data, int size, int soundId, int volume, int pan, int soundClass);

public:
	SmushSound(SmushTimer *timer, SmushDriver *driver);
	~SmushSound();

	int moduleStartup();
	int moduleShutdown();

	int souEngineStart(const byte *data, int soundId, int volume, int pan, int soundClass);
	void souEngineStop(int soundId);
	void souEnginePause();
	int souEngineResume();
	void souEngineStopAll();
	void souEngineSetVol(int soundId, int volume);
	int souEngineGetVol(int soundId);
	void souEngineSetPan(int soundId, int pan);
	int souEngineGetPan(int soundId);
	int souEngineSetHVol(int volume);
	int souEngineGetHVol();
	int souEngineSetRate(int rate);
	int souEngineGetRate();
	int souEngineQuery(int soundId);
	int souEngineQueryTime(int channelIndex);
	int souEngineSetParam(int soundId, int param, int value1, int value2);
	int souEngineGetParam(int soundId, int param, int *value1, int *value2);
	int souEngineStartStream(const byte *data, int size, int soundId, uint32 volume, int pan, int soundClass, int tag1, int chunkCounter, int tag2, int copyFlag);
	void souEngineFill(int frameCount, SmushDriver *driver);

};

} // End of namespace Grim
#endif
