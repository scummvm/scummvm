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
#include "sound/rate.h"
#include "common/file.h"

namespace Sword2 {

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

class MusicHandle : public MusicStream {
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

	int16 read();
	bool eos() const;

	MusicHandle() : MusicStream(), _firstTime(false),
			_streaming(false), _paused(false), _looping(false),
			_fading(0), _fileStart(0), _filePos(0), _fileEnd(0),
			_lastSample(0) {}
};

class Sound {
private:
	SoundMixer *_mixer;

	OSystem::MutexRef _mutex;
	RateConverter *_converter;

	FxHandle _fx[MAXFX];
	MusicHandle _music[MAXMUS + 1];

	uint8 _musicVol;

	uint8 _soundOn;
	uint8 _speechStatus;
	uint8 _fxPaused;
	uint8 _speechPaused;
	uint8 _speechVol;
	uint8 _fxVol;
	uint8 _speechMuted;
	uint8 _fxMuted;

	PlayingSoundHandle _soundHandleSpeech;
	uint8 _musicMuted;

	int32 getFxIndex(int32 id);
	int32 dipMusic();

	void updateCompSampleStreaming(int16 *data, uint len);

public:
	Sound(SoundMixer *mixer);
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
	void reverseStereo(void);
	uint8 getFxVolume(void);
	uint8 getSpeechVolume(void);
	uint8 getMusicVolume(void);
	uint8 isMusicMute(void);
	uint8 isFxMute(void);
	uint8 isSpeechMute(void);
	void stopMusic(void);
	void setFxVolume(uint8 vol);
	void setSpeechVolume(uint8 vol);
	void setMusicVolume(uint8 vol);
	void muteMusic(uint8 mute);
	void muteFx(uint8 mute);
	void muteSpeech(uint8 mute);
	int32 isFxOpen(int32 id);
	int32 setFxIdVolumePan(int32 id, uint8 vol, int8 pan);
	int32 setFxIdVolume(int32 id, uint8 vol);
};

} // End of namespace Sword2

#endif
