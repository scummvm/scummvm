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

//=============================================================================
//
//	Filename	:	d_sound.h
//	Created		:	5th December 1996
//	By			:	P.R.Porter
//
//	Summary		:	This include file defines links to all data which is
//					defined in the d_sound.c module, but can be accessed by
//					other parts of the driver96 library.
//
//
//=============================================================================


#ifndef D_SOUND_H
#define D_SOUND_H

#include "sound/audiostream.h"
#include "sound/mixer.h"
#include "sound/rate.h"
#include "common/file.h"

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
	char _fileName[256];
	bool _streaming;
	bool _paused;
	bool _looping;
	int32 _fading;
	int32 _filePos;
	int32 _fileEnd;
	uint16 _lastSample;

	bool isStereo()	const	{ return false; }
	int getRate() const	{ return 22050; }

	int16 read();
	bool eos() const;

	MusicHandle() : MusicStream(), _streaming(false), _paused(false),
			_looping(false), _fading(0), _filePos(0), _fileEnd(0),
			_lastSample(0) {
		_fileName[0] = 0;
	}
};

class Sword2Sound {
	public:
		Sword2Sound(SoundMixer *mixer);
		~Sword2Sound();
		void FxServer(int16 *data, uint len);
		int32 PlaySpeech(uint8 *data, uint8 vol, int8 pan);
		int32 PlayCompSpeech(const char *filename, uint32 speechid, uint8 vol, int8 pan);
		uint32 PreFetchCompSpeech(const char *filename, uint32 speechid, uint16 **buf);
		int32 AmISpeaking();
		int32 StopSpeechSword2(void);
		int32 GetSpeechStatus(void);
		int32 PauseSpeech(void);
		int32 UnpauseSpeech(void);
		int32 OpenFx(int32 id, uint8 *data);
		int32 PlayFx(int32 id, uint8 *data, uint8 vol, int8 pan, uint8 type);
		int32 CloseFx(int32 id);
		int32 ClearAllFx(void);
		int32 PauseFx(void);
		int32 PauseFxForSequence(void);
		int32 UnpauseFx(void);
		int32 PauseMusic(void);
		int32 UnpauseMusic(void);
		int32 StreamCompMusic(const char *filename, uint32 musicId, bool looping);
		int32 MusicTimeRemaining();
		int32 ReverseStereo(void);
		uint8 GetFxVolume(void);
		uint8 GetSpeechVolume(void);
		uint8 GetMusicVolume(void);
		uint8 IsMusicMute(void);
		uint8 IsFxMute(void);
		uint8 IsSpeechMute(void);
		void StopMusic(void);
		void SetFxVolume(uint8 vol);
		void SetSpeechVolume(uint8 vol);
		void SetMusicVolume(uint8 vol);
		void MuteMusic(uint8 mute);
		void MuteFx(uint8 mute);
		void MuteSpeech(uint8 mute);
		int32 IsFxOpen(int32 id);
		int32 SetFxVolumePan(int32 id, uint8 vol, int8 pan);
		int32 SetFxIdVolume(int32 id, uint8 vol);
		void UpdateCompSampleStreaming(int16 *data, uint len);
		SoundMixer *_mixer;
	private:
		int32 StreamCompMusicFromLock(const char *filename, uint32 musicId, bool looping);
		int32 GetFxIndex(int32 id);
		int32 DipMusic();

		OSystem::MutexRef _mutex;
		RateConverter *_converter;

		FxHandle fx[MAXFX];
		MusicHandle music[MAXMUS];

		// We used to have two music volumes - one for each channel -
		// but they were always set to the same value.

		uint8 musicVol;

		uint8 soundOn;
		uint8 speechStatus;
		uint8 fxPaused;
		uint8 speechPaused;
		uint8 speechVol;
		uint8 fxVol;
		uint8 speechMuted;
		uint8 fxMuted;
		uint8 compressedMusic;

		PlayingSoundHandle soundHandleSpeech;
		uint8 musicMuted;
};


#endif
