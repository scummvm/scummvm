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

#include "common/scummsys.h"

#include "trecision/sound.h"

#include "trecision/trecision.h"
#include "trecision/nl/extern.h"
#include "trecision/nl/define.h"

#include "audio/decoders/wave.h"
#include "common/memstream.h"
#include "common/system.h"
#include "audio/mixer.h"
#include "audio/audiostream.h"

namespace Trecision {

SoundManager::SoundManager(TrecisionEngine *vm) : _vm(vm) {
	for (int i = 0; i < NUMSAMPLES; ++i)
		sfxStream[i] = nullptr;

	nltime = 0;

	for (int i = 0; i < SAMPLEVOICES; ++i) {
		playing[i] = 0;
		smpvol[i] = 0;
	}

	StepChannel = kSoundChannelStep;
	BackChannel = kSoundChannelBack;
	SoundFadStatus = SFADNONE;

	SoundFadInVal = 0;
	SoundFadOutVal = 0;
}

SoundManager::~SoundManager() {
}

/* -----------------05/08/97 16.36-------------------
					soundtimefunct
 --------------------------------------------------*/
void SoundManager::soundtimefunct() {
	uint32 ctime = g_system->getMillis() / 8;		    // entra una volta ogni otto

	if (ctime > nltime)
		nltime = ctime;
	else
		return;

	if (SoundFadStatus) {	// solo se sono in un fad
		if (SoundFadStatus & SFADOUT) {
			if (!g_system->getMixer()->isSoundHandleActive(soundHandle[BackChannel])) {
				SoundFadStatus &= (~SFADOUT);
			}
			else {
				SoundFadOutVal -= FADMULT;

				if (SoundFadOutVal > 0)
					g_system->getMixer()->setChannelVolume(soundHandle[BackChannel], VOLUME(SoundFadOutVal / FADMULT));
				else {
					SoundFadOutVal = 0;
					g_system->getMixer()->setChannelVolume(soundHandle[BackChannel], VOLUME(SoundFadOutVal));

					SoundFadStatus &= (~SFADOUT);
				}
			}
		}
		if (SoundFadStatus & SFADIN) {
			SoundFadInVal += FADMULT;

			if (SoundFadInVal > GSample[playing[StepChannel]]._volume * FADMULT)
				SoundFadInVal = GSample[playing[StepChannel]]._volume * FADMULT;

			g_system->getMixer()->setChannelVolume(soundHandle[StepChannel], VOLUME(SoundFadInVal / FADMULT));

			for (int a = 2; a < SAMPLEVOICES; a++) {
				if (playing[a] != 0) {
					smpvol[a] += FADMULT;

					if (smpvol[a] > GSample[playing[a]]._volume * FADMULT)
						smpvol[a] = GSample[playing[a]]._volume * FADMULT;

					g_system->getMixer()->setChannelVolume(soundHandle[a], VOLUME(smpvol[a] / FADMULT));
				}
			}
		}
	}
}

void SoundManager::StopSoundSystem() {
	g_system->getMixer()->stopAll();
}

void SoundManager::LoadAudioWav(int num, Common::String fileName) {
	assert(num != 0xFFFF);
	Common::SeekableReadStream *stream = _vm->_dataFile.createReadStreamForMember(fileName);
	byte *buf = new byte[stream->size()];
	int size = stream->size();
	stream->read(buf, size);
	delete stream;
	sfxStream[num] = Audio::makeWAVStream(new Common::MemoryReadStream(buf, size), DisposeAfterUse::YES);
}

void SoundManager::NLPlaySound(int num) {
	int channel = 2;
	if (g_system->getMixer()->isSoundHandleActive(soundHandle[channel])) {
		g_system->getMixer()->stopHandle(soundHandle[channel]);
		playing[channel] = 0;
	}

	int volume = VOLUME(GSample[num]._volume);

	if (GSample[num]._flag & kSoundFlagSoundOn) {
		volume = 0;
		smpvol[channel] = 0;
	}

	Audio::AudioStream *stream = sfxStream[num];
	Audio::Mixer::SoundType type = GSample[num]._flag & kSoundFlagBgMusic ? Audio::Mixer::kMusicSoundType : Audio::Mixer::kSFXSoundType;
	if (stream != nullptr && GSample[num]._flag & kSoundFlagSoundLoop)
		stream = Audio::makeLoopingAudioStream(sfxStream[num], 0);

	g_system->getMixer()->playStream(type, &soundHandle[channel], stream, -1, volume, 0, DisposeAfterUse::NO);

	playing[channel] = num;
}

/* -----------------14/08/97 16.30-------------------
					NLStopSound
 --------------------------------------------------*/
void SoundManager::NLStopSound(int num) {
	for (int a = 2; a < kSoundChannelSpeech; a++) {
		if (playing[a] == num) {
			g_system->getMixer()->stopHandle(soundHandle[a]);
			playing[a] = 0;
		}
	}
}

void SoundManager::SoundStopAll() {
	for (int a = 0; a < SAMPLEVOICES; a++) {
		g_system->getMixer()->stopHandle(soundHandle[a]);
		playing[a] = 0;
	}

	SoundFadOutVal = SFADNONE;
	SoundFadStatus = 0;
}

/* -----------------14/08/97 16.30-------------------
					SoundFadOut
 --------------------------------------------------*/
void SoundManager::SoundFadOut() {
	for (int a = 0; a < SAMPLEVOICES; a++) {	// Turns off all channels except background
		if (a != BackChannel) {
			g_system->getMixer()->stopHandle(soundHandle[a]);
			playing[a] = 0;
		}
	}

	SoundFadOutVal = g_system->getMixer()->getChannelVolume(soundHandle[BackChannel]) * FADMULT;
	SoundFadStatus = SFADOUT;
}

/* -----------------14/08/97 16.30-------------------
					SoundFadIn
 --------------------------------------------------*/
void SoundManager::SoundFadIn(int num) {
	Audio::AudioStream *stream = sfxStream[num];
	Audio::Mixer::SoundType type = GSample[num]._flag & kSoundFlagBgMusic ? Audio::Mixer::kMusicSoundType : Audio::Mixer::kSFXSoundType;
	if (stream != nullptr && GSample[num]._flag & kSoundFlagSoundLoop)
		stream = Audio::makeLoopingAudioStream(sfxStream[num], 0);

	g_system->getMixer()->playStream(type, &soundHandle[StepChannel], stream, -1, 0, 0, DisposeAfterUse::NO);

	playing[StepChannel] = num;

	SoundFadInVal = 0;
	SoundFadStatus |= SFADIN;
}

/* -----------------14/08/97 16.31-------------------
					WaitSoundFadEnd
 --------------------------------------------------*/
void SoundManager::WaitSoundFadEnd() {
	while ((SoundFadInVal != (GSample[playing[StepChannel]]._volume * FADMULT)) && (playing[StepChannel] != 0) && (SoundFadOutVal != 0))
		_vm->checkSystem();
	SoundFadStatus = SFADNONE;

	g_system->getMixer()->stopHandle(soundHandle[BackChannel]);

	g_system->getMixer()->setChannelVolume(soundHandle[StepChannel], VOLUME(GSample[playing[StepChannel]]._volume));
	playing[BackChannel] = 0;

	for (uint8 a = 2; a < kSoundChannelSpeech; a++) {
		if (playing[a] != 0)
			g_system->getMixer()->setChannelVolume(soundHandle[a], VOLUME(GSample[playing[a]]._volume));
	}

	SWAP(StepChannel, BackChannel);

	if (_vm->_curRoom == kRoom41D)
		ReadExtraObj41D();
}

/* -----------------14/08/97 16.31-------------------
					SounPasso
 --------------------------------------------------*/
void SoundManager::SoundPasso(int midx, int midz, int act, int frame, uint16 *list) {
	extern unsigned char _defActionLen[];

	bool stepRight = false;
	bool stepLeft = false;

	switch (act) {
	case hWALK:
		if (frame == 3)
			stepLeft = true;
		else if (frame == 8)
			stepRight = true;
		break;

	case hWALKIN:
		if (frame == 3)
			stepLeft = true;
		else if (frame == 9)
			stepRight = true;
		break;

	case hWALKOUT:
		if (frame == 5)
			stepLeft = true;
		else if (frame == 10)
			stepRight = true;
		break;

	case hSTOP0:
	case hSTOP1:
	case hSTOP2:
	case hSTOP3:
	case hSTOP9:
		if (frame >= _defActionLen[act] - 1)
			stepLeft = true;
	case hSTOP4:
	case hSTOP5:
	case hSTOP6:
	case hSTOP7:
	case hSTOP8:
		if (frame >= _defActionLen[act] - 1)
			stepRight = true;
		break;
	default:
		break;
	}

	if (!stepRight && !stepLeft)
		return;

	int b;
	for (int a = 0; a < MAXSOUNDSINROOM; a++) {
		b = list[a];

		if (stepRight && (GSample[b]._flag & kSoundFlagStepRight))
			break;
		if (stepLeft && (GSample[b]._flag & kSoundFlagStepLeft))
			break;
		if (b == 0)
			return;
	}

	midz = ((int)(GSample[b]._volume) * 1000) / ABS(midz);

	if (midz > 255)
		midz = 255;

	g_system->getMixer()->stopHandle(soundHandle[StepChannel]);
	sfxStream[b]->rewind();

	int panpos = ((midx - 320) * 127 / 320) / 2;
	Audio::Mixer::SoundType type = GSample[b]._flag & kSoundFlagBgMusic ? Audio::Mixer::kMusicSoundType : Audio::Mixer::kSFXSoundType;

	g_system->getMixer()->playStream(type, &soundHandle[StepChannel], sfxStream[b], -1, VOLUME(midz), panpos, DisposeAfterUse::NO);
}

int32 SoundManager::Talk(const char *name) {
	if (!_vm->_speechFile.isOpen())
		return 0;

	StopTalk();

	Common::SeekableReadStream *stream = _vm->_speechFile.createReadStreamForMember(name);
	if (!stream)
		return 0;

	Audio::SeekableAudioStream *speechStream = Audio::makeWAVStream(stream, DisposeAfterUse::YES);

	g_system->getMixer()->playStream(Audio::Mixer::kSpeechSoundType, &soundHandle[kSoundChannelSpeech], speechStream);
	_characterSpeakTime = _vm->ReadTime();

	return TIME(speechStream->getLength().msecs());
}

void SoundManager::StopTalk() {
	g_system->getMixer()->stopHandle(soundHandle[kSoundChannelSpeech]);
}

/*-----------------12/12/95 11.39-------------------
					ReadSounds
--------------------------------------------------*/
void SoundManager::ReadSounds() {
	for (uint16 a = 0; a < MAXSOUNDSINROOM; a++) {
		uint16 b = _vm->_room[_vm->_curRoom]._sounds[a];

		if (b == 0)
			break;

		if (!scumm_stricmp(GSample[b]._name, "RUOTE2C.WAV"))
			break;

		LoadAudioWav(b, GSample[b]._name);

		if (GSample[b]._flag & kSoundFlagBgMusic)
			SoundFadIn(b);
		else if (GSample[b]._flag & kSoundFlagSoundOn)
			NLPlaySound(b);
	}
}

void SoundManager::syncGameStream(Common::Serializer &ser) {
	for (int a = 0; a < MAXSAMPLE; a++) {
		ser.syncAsByte(GSample[a]._volume);
		ser.syncAsByte(GSample[a]._flag);
	}
}

void SoundManager::loadSamples(Common::File *file) {
	for (int i = 0; i < MAXSAMPLE; ++i) {
		file->read(&GSample[i]._name, ARRAYSIZE(GSample[i]._name));
		GSample[i]._volume = file->readByte();
		GSample[i]._flag = file->readByte();
		GSample[i]._panning = file->readSByte();
	}
}

} // End of namespace Trecision
