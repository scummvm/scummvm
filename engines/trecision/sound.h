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

#ifndef TRECISION_SOUND_H
#define TRECISION_SOUND_H

#include "common/file.h"
#include "trecision/trecision.h"

namespace Trecision {

#define SOUND_OFF 0
#define SOUND_ON 1

#define SFADNONE 0
#define SFADIN 1
#define SFADOUT 2

#define SAMPLEVOICES 6
#define NUMSAMPLES MAXSAMPLE

#define VOLUME(a)	( (a * 255) / 127 )
#define TIME(a)		( (a * 3) / 50 )
#define FADMULT		100

enum SoundChannel {
	kSoundChannelBack = 0,
	kSoundChannelStep = 1,
	kSoundChannelSpeech = 5
};

struct SSound {
	char  _name[14];
	uint8 _volume;
	uint8 _flag;
	int8  _panning;
};

class SoundManager {
public:
	SoundManager(TrecisionEngine *vm);
	~SoundManager();

private:
	TrecisionEngine *_vm;
	SSound GSample[MAXSAMPLE];

public:
	Audio::SeekableAudioStream *sfxStream[NUMSAMPLES];
	Audio::SoundHandle    soundHandle[SAMPLEVOICES];	// Sample handles for each mixer channel

	uint32 nltime;			// timer variable
	int32  MinSampleBuffer;

	int16 playing[SAMPLEVOICES];			// sample currently playing
	int16 smpvol[SAMPLEVOICES];

	uint8 StepChannel;
	uint8 BackChannel;
	uint8 SoundFadStatus;

	int16 SoundFadInVal;
	int16 SoundFadOutVal;


	void soundtimefunct();
	void StopSoundSystem();
	void LoadAudioWav(int num, Common::String fileName);
	void NLPlaySound(int num);
	void NLStopSound(int num);
	void SoundStopAll();
	void SoundFadOut();
	void SoundFadIn(int num);
	void WaitSoundFadEnd();
	void SoundPasso(int midx, int midz, int act, int frame, uint16 *list);
	int32 Talk(const char *name);
	void StopTalk();
	void ReadSounds();

	void syncGameStream(Common::Serializer &ser);
	void loadSamples(Common::File *file);
};


} // End of namespace Trecision

#endif
