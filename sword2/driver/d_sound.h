/* Copyright (C) 1994-2003 Revolution Software Ltd
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

typedef struct {
	int32 _id;
	bool _paused;
	int8 _volume;
	uint16 _rate;
	uint32 _flags;
	uint16 *_buf;
	int32 _bufSize;
	PlayingSoundHandle _handle;
} FxHandle;

class MusicHandle : public AudioInputStream {
public:
	uint32 _id;
	bool _firstTime;
	bool _streaming;
	bool _paused;
	bool _looping;
	int32 _fading;
	int32 _fileStart;
	int32 _filePos;
	int32 _fileEnd;
	uint16 _lastSample;

	bool isStereo()	const	{ return false; }
	int getRate() const	{ return 22050; }

	virtual int readBuffer(int16 *buffer, const int numSamples) {
		int samples;
		for (samples = 0; samples < numSamples && !endOfData(); samples++) {
			*buffer++ = read();
		}
		return samples;
	}

	int16 read();
	bool endOfData() const;
	// This stream never 'ends'
	bool endOfStream() const { return false; }

	MusicHandle() : _firstTime(false),
			_streaming(false), _paused(false), _looping(false),
			_fading(0), _fileStart(0), _filePos(0), _fileEnd(0),
			_lastSample(0) {}
};

class Sound {
private:
	Sword2Engine *_vm;

	OSystem::MutexRef _mutex;
	RateConverter *_converter;

	int32 _panTable[33];

	FxHandle _fx[MAXFX];
	MusicHandle _music[MAXMUS + 1];

	bool _soundOn;
	bool _speechStatus;
	bool _speechPaused;
	bool _fxPaused;
	bool _musicMuted;
	bool _speechMuted;
	bool _fxMuted;
	uint8 _musicVol;
	uint8 _speechVol;
	uint8 _fxVol;

	PlayingSoundHandle _soundHandleSpeech;

	int32 getFxIndex(int32 id);
	int32 dipMusic();

	void updateCompSampleStreaming(int16 *data, uint len);

public:
	Sound(Sword2Engine *vm);
	~Sound();
	void fxServer(int16 *data, uint len);
	int32 playCompSpeech(const char *filename, uint32 speechid, uint8 vol, int8 pan);
	uint32 preFetchCompSpeech(const char *filename, uint32 speechid, uint16 **buf);
	int32 amISpeaking();
	int32 stopSpeech(void);
	int32 getSpeechStatus(void);
	void pauseSpeech(void);
	void unpauseSpeech(void);
	int32 openFx(int32 id, uint8 *data);
	int32 playFx(int32 id, uint8 *data, uint8 vol, int8 pan, uint8 type);
	int32 closeFx(int32 id);
	void clearAllFx(void);
	void pauseFx(void);
	void pauseFxForSequence(void);
	void unpauseFx(void);
	void pauseMusic(void);
	void unpauseMusic(void);
	int32 streamCompMusic(const char *filename, uint32 musicId, bool looping);
	void saveMusicState();
	void restoreMusicState();
	void playLeadOut(uint8 *leadOut);
	int32 musicTimeRemaining();
	void buildPanTable(bool reverse);
	uint8 getFxVolume(void);
	uint8 getSpeechVolume(void);
	uint8 getMusicVolume(void);
	bool isMusicMute(void);
	bool isFxMute(void);
	bool isSpeechMute(void);
	void stopMusic(void);
	void setFxVolume(uint8 vol);
	void setSpeechVolume(uint8 vol);
	void setMusicVolume(uint8 vol);
	void muteMusic(bool mute);
	void muteFx(bool mute);
	void muteSpeech(bool mute);
	int32 isFxOpen(int32 id);
	int32 setFxIdVolumePan(int32 id, uint8 vol, int8 pan);
	int32 setFxIdVolume(int32 id, uint8 vol);
};

} // End of namespace Sword2

#endif
