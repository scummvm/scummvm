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

#ifndef SWORD1_SOUND_H
#define SWORD1_SOUND_H

#include "sword1/object.h"
#include "sword1/sworddefs.h"

#include "common/file.h"
#include "common/mutex.h"
#include "common/random.h"
#include "common/timer.h"
#include "common/util.h"

#include "audio/mixer.h"
#include "audio/audiostream.h"

namespace Audio {
class Mixer;
}

namespace Sword1 {

#define TOTAL_FX_PER_ROOM   7       // total loop & random fx per room (see fx_list.c)
#define MAX_ROOMS_PER_FX    7       // max no. of rooms in the fx's room,vol list
#define MAX_FXQ_LENGTH      32      // max length of sound queue - ie. max number of fx that can be stored up/playing together

#define FX_SPOT   1
#define FX_LOOP   2
#define FX_RANDOM 3

struct QueueElement {
	uint32 id, delay;
	Audio::SoundHandle handle;

	void reset() {
		id = 0, delay = 0;
		handle = Audio::SoundHandle();
	}
};

struct RoomVol {
	int32 roomNo, leftVol, rightVol;
};

struct SampleId {
	byte cluster;
	byte idStd;
	byte idWinDemo;
};

struct FxDef {
	SampleId sampleId;
	uint32 type, delay;
	RoomVol roomVolList[MAX_ROOMS_PER_FX];
};

class ResMan;
class SwordEngine;

#define MUSIC_UNDERSCORE     50
#define NEG_MOUTH_THRESHOLD -750
#define POS_MOUTH_THRESHOLD  750
#define MAX_FX               4
#define MAX_MUSIC            2
#define S_STATUS_FINISHED    1
#define S_STATUS_RUNNING     0
#define MUSIC_BUFFER_SIZE    0x2000
#define TOTAL_TUNES          270
#define DEFAULT_SAMPLE_RATE  11025

enum CowMode {
	CowWave = 0,
	CowFLAC,
	CowVorbis,
	CowMP3,
	CowDemo,
	CowPSX
};

enum MusCompMode {
	MusWav = 0,
	MusAif,
	MusFLAC,
	MusVorbis,
	MusMP3,
	MusPSX
};

class Sound {
	friend class SwordConsole;
	friend class Control;
public:
	Sound(Audio::Mixer *mixer, SwordEngine *vm, ResMan *pResMan);
	~Sound();

	void newScreen(uint32 screen);
	void closeCowSystem();

	void engine();

	void checkSpeechFileEndianness();
	double endiannessHeuristicValue(int16* data, uint32 dataSize, uint32 &maxSamples);

	void installFadeTimer();
	void uninstallFadeTimer();

	void playSample(int32 fxNo);
	void stopSample(int32 fxNo);
	void setFXVolume(byte targetVolume, int handleIdx);
	void clearAllFx();
	int addToQueue(uint32 fxNo);
	void removeFromQueue(uint32 fxNo);

	void startSpeech(uint16 roomNo, uint16 localNo);
	bool amISpeaking();
	int32 checkSpeechStatus();
	void playSpeech();
	void stopSpeech();

	void streamMusicFile(int32 tuneId, int32 looped);
	void updateMusicStreaming();
	void setCrossFadeIncrement();

	void fadeMusicDown(int32 rate);
	void fadeFxDown(int32 rate);
	void fadeFxUp(int32 rate);

	void pauseSpeech();
	void unpauseSpeech();
	void pauseMusic();
	void unpauseMusic();
	void pauseFx();
	void unpauseFx();

	void getVolumes();
	void setVolumes();

	Common::Mutex _soundMutex;
	Audio::Mixer *_mixer;

	// Handles for external volume changes (control panel)
	uint32 _volFX[2]     = { 0, 0 };
	uint32 _volSpeech[2] = { 0, 0 };
	uint32 _volMusic[2]  = { 0, 0 };

	// Volume fading variables
	int32 _fxCount = 0;
	int32 _fxFadingFlag = 0;
	int32 _fxFadingRate = 0;
	int32 _fxFadeVolume[2]    = { 0, 0 };
	int32 _musicFadeVolume[2] = { 0, 0 };

	// Sound FX information
	bool _fxSampleBusy[MAX_FX] = { false, false, false, false };

	// Speech data
	byte *_speechSample = nullptr;

private:
	struct WaveHeader {
		uint32 riffTag;
		uint32 riffSize;
		uint32 waveTag;
		uint32 fmtTag;
		uint32 fmtSize;

		// Format subchunk
		uint16 wFormatTag;
		uint16 wChannels;
		uint32 dwSamplesPerSec;
		uint32 dwAvgBytesPerSec;
		uint16 wBlockAlign;
		uint16 wBitsPerSample;
		uint32 dwDataTag;
		uint32 dwDataSize;
	};

	void initCowSystem();

	uint32 getSampleId(int32 fxNo);
	void playFX(int32 fxID, int32 type, uint8 *wavData, uint32 vol[2]);
	void stopFX(int32 fxID);
	int32 checkSampleStatus(int32 id);

	bool expandSpeech(byte *src, byte *dst, uint32 dstSize,
		bool *endiannessCheck = nullptr, uint32 *sizeForEndiannessCheck = nullptr);
	int32 getSpeechSize(byte *compData, uint32 compSize);

	bool prepareMusicStreaming(const Common::Path &filename, int newHandleId, int32 tuneId,
							   uint32 volume, int8 pan, MusCompMode assignedMode);
	void serveSample(Common::File *file, int32 i);
	void reduceMusicVolume();
	void restoreMusicVolume();

	int8 scalePan(int pan); // From 0,127 to -127,127

	Common::File _cowFile;
	uint32       *_cowHeader;
	uint32       _cowHeaderSize;
	uint8        _currentCowFile;
	CowMode      _cowMode;
	Common::RandomSource _rnd;

	QueueElement _fxQueue[MAX_FXQ_LENGTH];
	uint8        _endOfQueue;
	SwordEngine *_vm;
	ResMan *_resMan;

	bool _bigEndianSpeech;
	static const char _musicList[270];
	static const uint16 _roomsFixedFx[TOTAL_ROOMS][TOTAL_FX_PER_ROOM];
	static const FxDef _fxList[312];
	static const char _tuneList[TOTAL_TUNES][8]; // In staticres.cpp

	// Volume fading variables
	bool _crossFadeIncrement = false;

	// Speech variables
	int32 _speechLipsyncCounter = 0;
	int32 _speechSize = 0;
	bool _speechSampleBusy = false;

	// Sound handles
	Audio::SoundHandle _hSampleFX[MAX_FX];
	Audio::SoundHandle _hSampleSpeech;
	Audio::SoundHandle _hSampleMusic[MAX_MUSIC];

	// Music stream information
	bool _musicStreamPlaying[MAX_MUSIC] = { false, false };
	bool _streamLoopingFlag[MAX_MUSIC]  = { false, false };
	int32 _musicStreamFading[MAX_MUSIC] = { 0, 0 };
	MusCompMode _musicStreamFormat[MAX_MUSIC] = { MusWav, MusWav };
	Audio::QueuingAudioStream *_musicOutputStream[MAX_MUSIC] = { nullptr, nullptr };
	Audio::RewindableAudioStream *_compressedMusicStream[MAX_MUSIC] = { nullptr, nullptr };
	Common::File _musicFile[MAX_MUSIC];

	// Sound FX information
	int32 _fxSampleId[MAX_FX] = { 0, 0, 0, 0 };

	// Pause variables
	bool _speechPaused = false;
	bool _fxPaused[MAX_FX] = { false, false, false, false };
	bool _musicPaused[MAX_MUSIC] = { false, false };
};

} // End of namespace Sword1

#endif // SOUND_H
