/* Copyright (C) 1994-2004 Revolution Software Ltd
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef D_SOUND_H
#define D_SOUND_H

#include "sound/audiostream.h"
#include "sound/mixer.h"

class RateConverter;

namespace Sword2 {

// Max number of sound fx
#define MAXFX 16
#define MAXMUS 2

extern void sword2_sound_handler(void *refCon);

struct WavInfo {
	uint8 channels;
	uint16 rate;
	uint32 samples;
	uint8 *data;
};

struct FxHandle {
	int32 _id;
	bool _paused;
	int8 _volume;
	uint16 _rate;
	uint32 _flags;
	uint16 *_buf;
	int32 _bufSize;
	PlayingSoundHandle _handle;
};

class MusicHandle : public AudioStream {
public:
	bool _firstTime;
	bool _streaming;
	bool _paused;
	bool _looping;
	int32 _fading;
	int32 _fileStart;
	int32 _filePos;
	int32 _fileEnd;
	uint16 _lastSample;

	bool isStereo(void) const	{ return false; }
	int getRate(void) const		{ return 22050; }

	void fadeDown(void);
	void fadeUp(void);
	int32 play(const char *filename, uint32 musicId, bool looping);
	void stop(void);
	int readBuffer(int16 *buffer, const int numSamples);
	bool endOfData(void) const;
	// This stream never 'ends'
	bool endOfStream(void) const { return false; }

	MusicHandle() : _firstTime(false), _streaming(false), _paused(false),
			_looping(false), _fading(0), _fileStart(0),
			_filePos(0), _fileEnd(0), _lastSample(0) {}
};

class Sound {
private:
	Sword2Engine *_vm;

	OSystem::MutexRef _mutex;

	int32 _panTable[33];
	bool _soundOn;

	static int32 _musicVolTable[17];
	MusicHandle _music[MAXMUS + 1];
	char *savedMusicFilename;
	RateConverter *_converter;
	bool _musicMuted;
	uint8 _musicVol;

	void updateCompSampleStreaming(int16 *data, uint len);
	int32 dipMusic(void);

	PlayingSoundHandle _soundHandleSpeech;
	bool _speechStatus;
	bool _speechPaused;
	bool _speechMuted;
	uint8 _speechVol;

	FxHandle _fx[MAXFX];
	bool _fxPaused;
	bool _fxMuted;
	uint8 _fxVol;

	int32 getFxIndex(int32 id);
	void stopFxHandle(int i);

public:
	Sound(Sword2Engine *vm);
	~Sound();

	void fxServer(int16 *data, uint len);
	void buildPanTable(bool reverse);

	bool getWavInfo(uint8 *data, WavInfo *wavInfo);

	void muteMusic(bool mute);
	bool isMusicMute(void);
	void setMusicVolume(uint8 vol);
	uint8 getMusicVolume(void);
	void pauseMusic(void);
	void unpauseMusic(void);
	void stopMusic(void);
	void saveMusicState(void);
	void restoreMusicState(void);
	void waitForLeadOut(void);
	int32 streamCompMusic(const char *filename, uint32 musicId, bool looping);
	int32 musicTimeRemaining(void);

	void muteSpeech(bool mute);
	bool isSpeechMute(void);
	void setSpeechVolume(uint8 vol);
	uint8 getSpeechVolume(void);
	void pauseSpeech(void);
	void unpauseSpeech(void);
	int32 stopSpeech(void);
	int32 getSpeechStatus(void);
	int32 amISpeaking(void);
	uint32 preFetchCompSpeech(const char *filename, uint32 speechid, uint16 **buf);
	int32 playCompSpeech(const char *filename, uint32 speechid, uint8 vol, int8 pan);

	void muteFx(bool mute);
	bool isFxMute(void);
	uint8 getFxVolume(void);
	void setFxVolume(uint8 vol);
	int32 setFxIdVolumePan(int32 id, uint8 vol, int8 pan);
	int32 setFxIdVolume(int32 id, uint8 vol);
	void pauseFx(void);
	void pauseFxForSequence(void);
	void unpauseFx(void);
	bool isFxPlaying(int32 id);
	int32 openFx(int32 id, uint8 *data);
	int32 closeFx(int32 id);
	int32 playFx(int32 id, uint8 *data, uint8 vol, int8 pan, uint8 type);
	void clearAllFx(void);
};

} // End of namespace Sword2

#endif
