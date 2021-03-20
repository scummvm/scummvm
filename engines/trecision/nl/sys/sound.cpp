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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common/scummsys.h"
#include "trecision/nl/sysdef.h"
#include "trecision/nl/define.h"
#include "trecision/trecision.h"

#include "audio/mixer.h"
#include "common/memstream.h"
#include "common/system.h"
#include "audio/decoders/wave.h"
#include "audio/audiostream.h"

#define SOUND_OFF		0
#define SOUND_ON		1

#define SFADIN		1
#define SFADOUT		2

#define	SAMPLEVOICES	6
#define	NUMSAMPLES		MAXSAMPLE

namespace Trecision {

struct SNLSample {
	Audio::SeekableAudioStream *stream;
	Audio::Mixer::SoundType type;
} NLSample[NUMSAMPLES];
SNLSample SpeechSample;

extern SSound GSample[];

int SoundDevice = -1;
int BackSound = 1;

Audio::SoundHandle    smp[SAMPLEVOICES];	// Sample handles for each mixer channel
uint32        nltime;			// timer variable
int32 	   MinSampleBuffer;

int16 playing[SAMPLEVOICES];			// sample currently playing
int16  smpvol[SAMPLEVOICES];

uint8 StepChannel = 1;
uint8 BackChannel = 0;
uint8 SpeechChannel = 5;
uint8 SoundFadStatus = 0;

int16 SoundFadInVal;
int16 SoundFadOutVal;

extern uint8 *SpeechBuf[2];
extern bool  SpeechTrackEnabled;
void CloseSys(const char *str);
void ReadExtraObj41D();
void StopTalk();

extern bool SoundSystemActive;
extern const char *_sysSentence[];
int SpeechFileLen(const char *name);
int SpeechFileRead(const char *name, unsigned char *buf);
uint32 ReadTime();

#define VOLUME(a)	( (a*255)/127 )
#define TIME(a)		( (a*3)/50 )
#define FADMULT		100
/* -----------------05/08/97 16.36-------------------
					soundtimefunct
 --------------------------------------------------*/
void soundtimefunct() {
	if (!SoundSystemActive)
		return;
	uint32 ctime = g_system->getMillis() / 8;		    // entra una volta ogni otto

	if (ctime > nltime)
		nltime = ctime;
	else
		return ;

	if (SoundFadStatus) {	// solo se sono in un fad
		if (SoundFadStatus & SFADOUT) {
			if (!g_system->getMixer()->isSoundHandleActive(smp[BackChannel])) {
				SoundFadStatus &= (~SFADOUT);
			} else {
				SoundFadOutVal -= FADMULT;

				if (SoundFadOutVal > 0)
					g_system->getMixer()->setChannelVolume(smp[BackChannel], VOLUME(SoundFadOutVal / FADMULT));
				else {
					SoundFadOutVal = 0;
					g_system->getMixer()->setChannelVolume(smp[BackChannel], VOLUME(SoundFadOutVal));

					SoundFadStatus &= (~SFADOUT);
				}
			}
		}
		if (SoundFadStatus & SFADIN) {
			SoundFadInVal += FADMULT;

			if (SoundFadInVal < (GSample[playing[StepChannel]]._volume * FADMULT))
				g_system->getMixer()->setChannelVolume(smp[StepChannel], VOLUME(SoundFadInVal / FADMULT));
			else {
				SoundFadInVal = GSample[playing[StepChannel]]._volume * FADMULT;
				g_system->getMixer()->setChannelVolume(smp[StepChannel], VOLUME(SoundFadInVal / FADMULT));

				//SoundFadStatus &= ( ~SFADIN );
			}

			for (int a = 2; a < SAMPLEVOICES; a++) {
				if (playing[a] != 0) {
					smpvol[a] += FADMULT;

					if (smpvol[a] > GSample[playing[a]]._volume * FADMULT)
						smpvol[a] = GSample[playing[a]]._volume * FADMULT;

					g_system->getMixer()->setChannelVolume(smp[a], VOLUME(smpvol[a] / FADMULT));
				}
			}
		}
	}
}

/* -----------------05/08/97 16.36-------------------
					StartSoundSystem
 --------------------------------------------------*/
void StartSoundSystem() {
	if (g_system->getMixer()->isReady())
		SoundSystemActive = true;
}

/* -----------------05/08/97 16.35-------------------
					StopSoundSystem
 --------------------------------------------------*/
void StopSoundSystem() {
	if (!SoundSystemActive)
		return;

	g_system->getMixer()->stopAll();
}

/* -----------------14/08/97 12.06-------------------
					LoadAudioWav
 --------------------------------------------------*/
short LoadAudioWav(int num, uint8 *wav, int len) {
	if (!SoundSystemActive)
		return 0;

	Audio::SeekableAudioStream *stream = Audio::makeWAVStream(new Common::MemoryReadStream(wav, len), DisposeAfterUse::YES);

	if (num != 0xFFFF) {
		NLSample[num].stream = stream;
		if (GSample[num]._flag & SOUNDFLAG_SBACK)
			NLSample[num].type = Audio::Mixer::kMusicSoundType;
		else
			NLSample[num].type = Audio::Mixer::kSFXSoundType;
	} else {
		SpeechSample.stream = stream;
		SpeechSample.type = Audio::Mixer::kSpeechSoundType;
	}
	return 1;
}

/* -----------------14/08/97 14.08-------------------
					NLPlaySound
 --------------------------------------------------*/
void NLPlaySound(int num) {

	if (!SoundSystemActive)
		return;

	int channel = 2;
	if (g_system->getMixer()->isSoundHandleActive(smp[channel])) {
		g_system->getMixer()->stopHandle(smp[channel]);
		playing[channel] = 0;
	}

	int volume = VOLUME(GSample[num]._volume);

	if (GSample[num]._flag & SOUNDFLAG_SON) {
		volume = 0;
		smpvol[channel] = 0;
	}

	Audio::AudioStream *stream = NLSample[num].stream;
	if (stream != nullptr && GSample[num]._flag & SOUNDFLAG_SLOOP)
		stream = Audio::makeLoopingAudioStream(NLSample[num].stream, 0);

	g_system->getMixer()->playStream(NLSample[num].type, &smp[channel], stream, -1, volume, 0, DisposeAfterUse::NO);

	playing[channel] = num;
}

/* -----------------14/08/97 16.30-------------------
					NLStopSound
 --------------------------------------------------*/
void NLStopSound(int num) {
	if (!SoundSystemActive)
		return;

	for (int a = 2; a < SpeechChannel; a++) {
		if (playing[a] == num) {
			g_system->getMixer()->stopHandle(smp[a]);
			playing[a] = 0;
		}
	}
}

/* -----------------14/08/97 16.30-------------------
					SoundFadOut
 --------------------------------------------------*/
void SoundFadOut() {
	if (!SoundSystemActive)
		return;

	for (int a = 0; a < SAMPLEVOICES; a++) {	// spegne tutti i canali eccetto il background
		if (a != BackChannel) {
			g_system->getMixer()->stopHandle(smp[a]);
			playing[a] = 0;
		}
	}

	SoundFadOutVal = g_system->getMixer()->getChannelVolume(smp[BackChannel]) * FADMULT;
	SoundFadStatus = SFADOUT;
}

/* -----------------14/08/97 16.30-------------------
					SoundFadIn
 --------------------------------------------------*/
void SoundFadIn(int num) {
	if (!SoundSystemActive)
		return;

	Audio::AudioStream *stream = NLSample[num].stream;
	if (stream != nullptr && GSample[num]._flag & SOUNDFLAG_SLOOP)
		stream = Audio::makeLoopingAudioStream(NLSample[num].stream, 0);

	g_system->getMixer()->playStream(NLSample[num].type, &smp[StepChannel], stream, -1, 0, 0, DisposeAfterUse::NO);

	playing[StepChannel] = num;

	SoundFadInVal = 0;
	SoundFadStatus |= SFADIN;
}

/* -----------------14/08/97 16.31-------------------
					WaitSoundFadEnd
 --------------------------------------------------*/
void WaitSoundFadEnd() {
	if (!SoundSystemActive) {
		if (g_vm->_curRoom == r41D)
			ReadExtraObj41D();
		return;
	}

	while ((SoundFadInVal != (GSample[playing[StepChannel]]._volume * FADMULT)) && (playing[StepChannel] != 0) && (SoundFadOutVal != 0))
		g_vm->CheckSystem();
	SoundFadStatus = 0;

	g_system->getMixer()->stopHandle(smp[BackChannel]);

	g_system->getMixer()->setChannelVolume(smp[StepChannel], VOLUME(GSample[playing[StepChannel]]._volume));
	playing[BackChannel] = 0;

	for (uint8 a = 2; a < SpeechChannel; a++)
		if (playing[a] != 0)
			g_system->getMixer()->setChannelVolume(smp[a], VOLUME(GSample[playing[a]]._volume));

	SWAP(StepChannel, BackChannel);

	if (g_vm->_curRoom == r41D)
		ReadExtraObj41D();
}

/* -----------------14/08/97 16.31-------------------
					SounPasso
 --------------------------------------------------*/
void SoundPasso(int midx, int midz, int act, int frame, unsigned short *list) {
	extern unsigned char _defActionLen[];
	int b;

	if (!SoundSystemActive)
		return;

	int StepRight = 0;
	int StepLeft = 0;

	switch (act) {
	case hWALK:
		if (frame == 3)
			StepLeft = 1;
		else if (frame == 8)
			StepRight = 1;
		break;

	case hWALKIN:
		if (frame == 3)
			StepLeft = 1;
		else if (frame == 9)
			StepRight = 1;
		break;

	case hWALKOUT:
		if (frame == 5)
			StepLeft = 1;
		else if (frame == 10)
			StepRight = 1;
		break;

	case hSTOP0:
	case hSTOP1:
	case hSTOP2:
	case hSTOP3:
	case hSTOP9:
		if (frame >= (_defActionLen[act] - 1))
			StepLeft = 1;
	case hSTOP4:
	case hSTOP5:
	case hSTOP6:
	case hSTOP7:
	case hSTOP8:
		if (frame >= (_defActionLen[act] - 1))
			StepRight = 1;
		break;
	}

	if (!(StepRight) && !(StepLeft))
		return;

	for (int a = 0; a < MAXSOUNDSINROOM; a++) {
		b = list[a];

		if ((StepRight) && (GSample[b]._flag & SOUNDFLAG_SPDX))
			break;
		if ((StepLeft) && (GSample[b]._flag & SOUNDFLAG_SPSX))
			break;
		if (b == 0)
			return;
	}

	if (midz < 0)
		midz = -midz;

	midz = ((int)(GSample[b]._volume) * 1000) / midz;

	if (midz > 255)
		midz = 255;

	int St = StepChannel;

	g_system->getMixer()->stopHandle(smp[St]);
	NLSample[b].stream->rewind();

	int panpos = ((midx - 320) * 127 / 320) / 2;

	g_system->getMixer()->playStream(NLSample[b].type, &smp[St], NLSample[b].stream, -1, VOLUME(midz), panpos, DisposeAfterUse::NO);
}

void ContinueTalk() {
	if (!SpeechTrackEnabled)
		return;

	if (!g_system->getMixer()->isSoundHandleActive(smp[SpeechChannel]))
		StopTalk();
}

int32 Talk(const char *name) {
	StopTalk();

	int Len = SpeechFileLen(name);
	if (Len > SPEECHSIZE || !SoundSystemActive) {
		SpeechTrackEnabled = false;
		return (Len * 60L) / 11025;
	}
	if (SpeechFileRead(name, SpeechBuf[0]) == 0) {
		SpeechTrackEnabled = false;
		return (Len * 60L) / 11025;
	}

	SpeechTrackEnabled = true;
	if (LoadAudioWav(0xFFFF, SpeechBuf[0], Len))
		Len *= 2;

	if (!SoundSystemActive)
		SpeechTrackEnabled = false;
	else {
		extern uint32 CharacterSpeakTime;
		g_system->getMixer()->playStream(SpeechSample.type, &smp[SpeechChannel], SpeechSample.stream);
		CharacterSpeakTime = ReadTime();
	}

	return TIME(SpeechSample.stream->getLength().msecs());
}

void StopTalk() {
	if (SoundSystemActive)
		g_system->getMixer()->stopHandle(smp[SpeechChannel]);

	SpeechTrackEnabled = false;
}

} // End of namespace Trecision
