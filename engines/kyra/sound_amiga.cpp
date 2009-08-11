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
 * $URL$
 * $Id$
 *
 */

#include "common/system.h"
#include "common/mutex.h"
#include "kyra/resource.h"
#include "kyra/sound_intern.h"

#include "sound/mixer.h"
#include "sound/mods/maxtrax.h"
#include "sound/audiostream.h"

namespace {

FORCEINLINE uint8 sfxTableGetNote(const byte* address) {
	return (uint8)address[0];
}
FORCEINLINE uint8 sfxTableGetPatch(const byte* address) {
	return (uint8)address[1];
}
FORCEINLINE uint16 sfxTableGetDuration(const byte* address) {
	return READ_BE_UINT16(&address[4]);
}
FORCEINLINE int8 sfxTableGetVolume(const byte* address) {
	return (int8)address[6];
}
FORCEINLINE int8 sfxTableGetPan(const byte* address) {
	return (int8)address[7];
}

} // end of namespace

namespace Kyra {

SoundAmiga::SoundAmiga(KyraEngine_v1 *vm, Audio::Mixer *mixer)
	: Sound(vm, mixer),
	  _driver(0),
	  _musicHandle(),
	  _fileLoaded(kFileNone),
	  _tableSfxIntro(),
	  _tableSfxGame() {
}

SoundAmiga::~SoundAmiga() {
	_mixer->stopHandle(_musicHandle);
	delete _driver;
}

extern const byte LoKAmigaSfxIntro[];
extern const byte LoKAmigaSfxGame[];

bool SoundAmiga::init() {
	_driver = new Audio::MaxTrax(_mixer->getOutputRate(), true);
	_tableSfxIntro = LoKAmigaSfxIntro;
	_tableSfxGame = LoKAmigaSfxGame;

	return _driver != 0 && _tableSfxIntro && _tableSfxGame;
}

void SoundAmiga::loadSoundFile(uint file) {
	static const char *const tableFilenames[3][2] = {
		{ "introscr.mx",  "introinst.mx" },
		{ "kyramusic.mx", 0 },
		{ "finalescr.mx", "introinst.mx" }
	};
	assert(file < ARRAYSIZE(tableFilenames));
	if (_fileLoaded == (FileType)file)
		return;
	const char* scoreName = tableFilenames[file][0];
	const char* sampleName = tableFilenames[file][1];
	bool loaded = false;

	Common::SeekableReadStream *scoreIn = _vm->resource()->createReadStream(scoreName);
	if (sampleName) {
		Common::SeekableReadStream *sampleIn = _vm->resource()->createReadStream(sampleName);
		if (scoreIn && sampleIn) {
			_fileLoaded = kFileNone;
			loaded = _driver->load(*scoreIn, true, false);
			loaded = loaded && _driver->load(*sampleIn, false, true);
		} else
			warning("SoundAmiga: missing atleast one of those music files: %s, %s", scoreName, sampleName);
		delete sampleIn;
	} else {
		if (scoreIn) {
			_fileLoaded = kFileNone;
			loaded = _driver->load(*scoreIn);
		} else
			warning("SoundAmiga: missing music file: %s", scoreName);
	}
	delete scoreIn;

	if (loaded)
		_fileLoaded = (FileType)file;
}

void SoundAmiga::playTrack(uint8 track) {
	static const byte tempoIntro[6] = { 0x46, 0x55, 0x3C, 0x41, 0x78, 0x50 };
	static const byte tempoIngame[23] = {
		0x64, 0x64, 0x64, 0x64, 0x64, 0x73, 0x4B, 0x64,
		0x64, 0x64, 0x55, 0x9C, 0x6E, 0x91, 0x78, 0x84,
		0x32, 0x64, 0x64, 0x6E, 0x3C, 0xD8, 0xAF
	};
	static const byte loopIngame[23] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01,
		0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00
	};

	int score = -1;
	bool loop = false;
	byte volume = 0x40;
	byte tempo = 0;


	switch (_fileLoaded) {
	case kFileFinal:
		// score 0 gets started immediately after loading the music-files with different tempo.
		// we need to define a track-value for the fake call of this function
		if (track == 10) {
			score = 0;
			loop = true;
			tempo = 0x78;
			break;
		}
		// if this is not the hardcoded start of the song then
		// Fallthrough
	case kFileIntro:
		if (track >= 2 && track < ARRAYSIZE(tempoIntro) + 2) {
			score = track - 2;
			tempo = tempoIntro[score];
		}
		break;

	case kFileGame:
		if (track >= 11 && track < ARRAYSIZE(tempoIngame) + 11) {
			score = track - 11;
			loop = loopIngame[score] != 0;
			tempo = tempoIngame[score];
		}
		break;

	default:
		return;
	}

	if (score >= 0) {
		if (_driver->playSong(score, loop)) {
			_driver->setVolume(volume);
			_driver->setTempo(tempo << 4);
			if (!_mixer->isSoundHandleActive(_musicHandle))
				_mixer->playInputStream(Audio::Mixer::kPlainSoundType, &_musicHandle, _driver, -1, Audio::Mixer::kMaxChannelVolume, 0, false);
		}
	} else if (track == 0)
		_driver->stopMusic();
	else if (track == 1)
		beginFadeOut();
}

void SoundAmiga::haltTrack() {
	_driver->stopMusic();
}

void SoundAmiga::beginFadeOut() {
	for (int i = 0x3F; i >= 0; --i) {
		_driver->setVolume((byte)i);
		_vm->delay(_vm->tickLength());
	}

	_driver->stopMusic();
	_vm->delay(_vm->tickLength());
	_driver->setVolume(0x40);
}

void SoundAmiga::playSoundEffect(uint8 track) {
	debug("play sfx %d", track);
	const byte* tableEntry = 0;
	bool pan = false;

	switch (_fileLoaded) {
	case kFileFinal:
	case kFileIntro:
		assert(track < 40);

		tableEntry = &_tableSfxIntro[track * 8];
		pan = (sfxTableGetPan(tableEntry) != 0);
		break;

	case kFileGame:
		// 0x61 <= track && track <= 0x63 && variable(0x1BFE2) which might indicate song playing in game and finale
		if (0x61 <= track && track <= 0x63)
			playTrack(track - 0x4F);

		assert(track < 120);
		// variable(0x1BFE2) && tableEffectsGame[track].note, which gets set for ingame and unset for finale
		// (and some function reverses its state)
		if (sfxTableGetNote(&_tableSfxGame[track * 8])) { 
			tableEntry = &_tableSfxGame[track * 8];
			pan = (sfxTableGetPan(tableEntry) != 0) && (sfxTableGetPan(tableEntry) != 2);
		}
		break;
	default:
		;
	}

	if (tableEntry) {
		const bool success = _driver->playNote(sfxTableGetNote(tableEntry), sfxTableGetPatch(tableEntry), sfxTableGetDuration(tableEntry), sfxTableGetVolume(tableEntry), pan);
		if (success && !_mixer->isSoundHandleActive(_musicHandle))
			_mixer->playInputStream(Audio::Mixer::kPlainSoundType, &_musicHandle, _driver, -1, Audio::Mixer::kMaxChannelVolume, 0, false);
	}
}

} // end of namespace Kyra
