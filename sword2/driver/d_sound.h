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
//	Version	Date		By		Description
//	------- ---------	---		-----------------------------------------------
//	1.0		05-Dec-96	PRP		Interface to the DirectSound driver functions
//
//	Summary		:	This include file defines links to all data which is
//					defined in the d_sound.c module, but can be accessed by
//					other parts of the driver96 library.
//
//
//=============================================================================


#ifndef D_SOUND_H
#define D_SOUND_H

#include "sound/mixer.h"
#include "common/file.h"

void sword2_sound_handler (void *engine);

class Sword2Sound {
	public:
		Sword2Sound(SoundMixer *mixer);
		void FxServer(void);
		int32 InitialiseSound(uint16 freq, uint16 channels, uint16 bitDepth);
		int32 PlaySpeech(uint8 *data, uint8 vol, int8 pan);
		int32 PlayCompSpeech(const char *filename, uint32 speechid, uint8 vol, int8 pan);
		int32 PreFetchCompSpeech(const char *filename, uint32 speechid, uint8 *waveMem);
		int32 GetCompSpeechSize(const char *filename, uint32 speechid);
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
		int32 StreamMusic(uint8 *filename, int32 looping);
		int32 StreamCompMusic(const char *filename, uint32 musicId, int32 looping);
		int32 MusicTimeRemaining();
		int32 ReverseStereo(void);
		uint8 GetFxVolume(void);
		uint8 GetSpeechVolume(void);
		uint8 GetMusicVolume(void);
		uint8 IsMusicMute(void);
		uint8 IsFxMute(void);
		uint8 IsSpeechMute(void);
		void  StopMusic(void);
		void  SetFxVolume(uint8 vol);
		void  SetSpeechVolume(uint8 vol);
		void  SetMusicVolume(uint8 vol);
		void  MuteMusic(uint8 mute);
		void  MuteFx(uint8 mute);
		void  MuteSpeech(uint8 mute);
		int32 IsFxOpen(int32 id);
		int32 SetFxVolumePan(int32 id, uint8 vol, int8 pan);
		int32 SetFxIdVolume(int32 id, uint8 vol);
		void UpdateCompSampleStreaming(void);
		SoundMixer *_mixer;
	private:
		int32 GetFxIndex(int32 id);
		void StartMusicFadeDown(int i);
		int32 DipMusic();

		int32 fxId[MAXFX];
		uint8 fxCached[MAXFX];
		uint8 fxiPaused[MAXFX];
		uint8 fxLooped[MAXFX];
		uint8 fxVolume[MAXFX];
		uint32 flagsFx[MAXFX];
		uint16 *bufferFx[MAXFX];
		int32 bufferSizeFx[MAXFX];

		uint8 soundOn;
		uint8 speechStatus;
		uint8 fxPaused;
		uint8 speechPaused;
		uint8 speechVol;
		uint8 fxVol;
		uint8 speechMuted;
		uint8 fxMuted;
		uint8 compressedMusic;

		int16 musStreaming[MAXMUS];
		int16 musicPaused[MAXMUS];
		int16 musCounter[MAXMUS];
		int16 musFading[MAXMUS];
		int16 musLooping[MAXMUS];

		PlayingSoundHandle soundHandleFx[MAXFX];
		PlayingSoundHandle soundHandleMusic[MAXMUS];
		PlayingSoundHandle soundHandleSpeech;
		File				fpMus;
		int bufferSizeMusic;
		int musicIndexChannel[MAXMUS];
		int musicChannels[MAXMUS];
		int32 streamCursor[MAXMUS];
		char musFilename[MAXMUS][256];
		int32 musFilePos[MAXMUS];
		int32 musEnd[MAXMUS];
		int16 musLastSample[MAXMUS];
		uint32 musId[MAXMUS];
		uint32 volMusic[2];
		uint8 musicMuted;
};


#endif
