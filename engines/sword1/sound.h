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

#define WAVE_VOL_TAB_LENGTH 480
#define WAVE_VOL_THRESHOLD 190000 //120000
#define MUSIC_UNDERSCORE 50
#define NEG_MOUTH_THRESHOLD (-750)
#define POS_MOUTH_THRESHOLD 750
#define MAX_FX 4
#define MAX_MUSIC 2
#define S_STATUS_FINISHED 1
#define S_STATUS_RUNNING  0

enum CowMode {
	CowWave = 0,
	CowFLAC,
	CowVorbis,
	CowMP3,
	CowDemo,
	CowPSX
};

class Sound {
	friend class SwordConsole;
	friend class Control;
public:
	Sound(Audio::Mixer *mixer, SwordEngine *vm, ResMan *pResMan);
	~Sound();
	void setSpeechVol(uint8 volL, uint8 volR) { _speechVolL = volL; _speechVolR = volR; }
	void giveSpeechVol(uint8 *volL, uint8 *volR) { *volL = _speechVolL; *volR = _speechVolR; }
	void newScreen(uint32 screen);
	void clearAllFx();
	void closeCowSystem();

	bool startSpeech(uint16 roomNo, uint16 localNo);
	bool speechFinished();
	void stopSpeech();
	bool amISpeaking(byte *buf);

	void fnStopFx(int32 fxNo);
	int addToQueue(uint32 fxNo);
	void removeFromQueue(uint32 fxNo);

	void engine();

	void checkSpeechFileEndianness();
	double endiannessHeuristicValue(int16* data, uint32 dataSize, uint32 &maxSamples);

	Common::Mutex _soundMutex;
	Audio::Mixer *_mixer;

private:
	uint8 _speechVolL, _speechVolR;
	void initCowSystem();

	uint32 getSampleId(int32 fxNo);
	int16 *uncompressSpeech(uint32 index, uint32 cSize, uint32 *size, bool* ok = 0);
	void calcWaveVolume(int16 *data, uint32 length);
	bool _waveVolume[WAVE_VOL_TAB_LENGTH];
	uint16 _waveVolPos;
	Common::File _cowFile;
	uint32       *_cowHeader;
	uint32       _cowHeaderSize;
	uint8        _currentCowFile;
	CowMode      _cowMode;
	Audio::SoundHandle _speechHandle, _fxHandle;
	Common::RandomSource _rnd;

	QueueElement _fxQueue[MAX_FXQ_LENGTH];
	uint8        _endOfQueue;
	SwordEngine *_vm;
	ResMan *_resMan;

	bool _bigEndianSpeech;
	static const char _musicList[270];
	static const uint16 _roomsFixedFx[TOTAL_ROOMS][TOTAL_FX_PER_ROOM];
	static const FxDef _fxList[312];

	// New stuff
public:
	void installFadeTimer();
	void uninstallFadeTimer();
	void PlaySample(int32 fxNo);

	int32 StreamSample(char filename[], int32 looped);
	void UpdateSampleStreaming();

	int32 PlayFX(int32 fxID, int32 type, uint8 *wavData, uint32 vol[2]);
	int32 StopFX(int32 fxID);
	int32 CheckSampleStatus(int32 id);
	int32 CheckSpeechStatus();
	int32 PlaySpeech(void *wavData, int32 size);
	int32 StopSpeech();
	void FadeVolumeDown(int32 rate);
	void FadeVolumeUp(int32 rate);
	void FadeMusicDown(int32 rate);
	void FadeMusicUp(int32 rate);
	void FadeFxDown(int32 rate);
	void FadeFxUp(int32 rate);
	int32 GetSpeechSize(void *compData);
	void SetCrossFadeIncrement();
	void PauseSpeech();
	void UnpauseSpeech();
	void PauseMusic();
	void UnpauseMusic();
	void PauseFx();
	void UnpauseFx();

	int8 scalePan(int pan); // From 0,127 to -127,127

	void ReduceMusicVolume();
	void RestoreMusicVolume();

	// Handles for external volume changes (control panel)
	uint32 volFX[2] = { 0, 0 };
	uint32 volSpeech[2] = { 0, 0 };
	uint32 volMusic[2] = { 0, 0 };

	// Volume variables
	int32 volumeFadingFlag = 0;
	int32 volumeFadingRate = 0;
	int32 musicFadingFlag = 0;
	int32 musicFadingRate = 0;
	int32 fxFadingFlag = 0;
	int32 fxFadingRate = 0;
	int32 masterVolume[2] = { 0, 0 };
	int32 fadeVolume[2] = { 0, 0 };
	int32 musicFadeVolume[2] = { 0, 0 };
	int32 fxFadeVolume[2] = { 0, 0 };
	int32 volumeCount = 0;
	int32 musicCount = 0;
	int32 fxCount = 0;
	int32 speechCount = 0;
	int32 speechSize = 0;

	// Sample handles and information.
	Audio::SoundHandle hSampleFX[MAX_FX];
	Audio::SoundHandle hSampleSpeech;
	Audio::SoundHandle hSampleMusic[MAX_MUSIC];
	int32 streamSamplePlaying[MAX_MUSIC] = { 0, 0 };
	int32 streamSampleFading[MAX_MUSIC] = { 0, 0 };
	int32 streamLoopingFlag[MAX_MUSIC] = { 0, 0 };
	Audio::SoundHandle hStreamSample[MAX_MUSIC];
	int32 streamFile[MAX_MUSIC];
	byte *streamBuffer[MAX_MUSIC][2];
	bool fxSampleBusy[MAX_FX] = { false, false, false, false };
	int32 fxSampleID[MAX_FX] = { 0, 0, 0, 0 };
	bool speechSampleBusy = false;
	uint32 musicSamples = 0;
	bool crossFadeIncrement = false;
	bool speechSamplePaused = false;
	bool fxPaused[MAX_FX] = { false, false, false, false };
	bool musicPaused[MAX_MUSIC] = { false, false };
};

} // End of namespace Sword1

#endif //BSSOUND_H
