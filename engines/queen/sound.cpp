/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2006 The ScummVM project
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

#include "common/stdafx.h"
#include "common/endian.h"

#include "queen/sound.h"

#include "queen/input.h"
#include "queen/music.h"
#include "queen/queen.h"
#include "queen/resource.h"

#include "sound/flac.h"
#include "sound/mp3.h"
#include "sound/vorbis.h"

#define	SB_HEADER_SIZE	110
#define	STOP_MUSIC	-1

namespace Queen {

Sound::Sound(Audio::Mixer *mixer, QueenEngine *vm) :
	_mixer(mixer), _vm(vm), _sfxToggle(true), _speechToggle(true), _musicToggle(true), _lastOverride(0) {
}

Sound::~Sound() {
}

Sound *Sound::giveSound(Audio::Mixer *mixer, QueenEngine *vm, uint8 compression) {
	if (!mixer->isReady())
		return new SilentSound(mixer, vm);

	switch (compression) {
	case COMPRESSION_NONE:
		return new SBSound(mixer, vm);
		break;
	case COMPRESSION_MP3:
#ifndef USE_MAD
		warning("Using MP3 compressed datafile, but MP3 support not compiled in");
		return new SilentSound(mixer, vm);
#else
		return new MP3Sound(mixer, vm);
#endif
		break;
	case COMPRESSION_OGG:
#ifndef USE_VORBIS
		warning("Using OGG compressed datafile, but OGG support not compiled in");
		return new SilentSound(mixer, vm);
#else
		return new OGGSound(mixer, vm);
#endif
		break;
	case COMPRESSION_FLAC:
#ifndef USE_FLAC
		warning("Using FLAC compressed datafile, but FLAC support not compiled in");
		return new SilentSound(mixer, vm);
#else
		return new FLACSound(mixer, vm);
#endif
		break;
	default:
		warning("Unknown compression type");
		return new SilentSound(mixer, vm);
	}
}

void Sound::waitFinished(bool isSpeech) {
	if (isSpeech)
		while (_mixer->isSoundHandleActive(_speechHandle))
			_vm->input()->delay(10);
	else
		while (_mixer->isSoundHandleActive(_sfxHandle))
			_vm->input()->delay(10);
}

void Sound::playSfx(uint16 sfx, bool isSpeech) {
	if (isSpeech && !speechOn()) return;
	else if (!sfxOn()) return;

	if (sfx != 0) {
		char name[13];
#ifndef PALMOS_68K
		strcpy(name, _sfxName[sfx - 1]);
#else
		strncpy(name, _sfxName + 10 * (sfx - 1), 10);	// saved as 8char + /0/0
#endif
		strcat(name, ".SB");
		waitFinished(isSpeech);
		if (sfxPlay(name, isSpeech ? &_speechHandle : &_sfxHandle)) {
			_speechSfxExists = isSpeech;
		} else {
			_speechSfxExists = false;
		}
	}
}

void Sound::playSfx(const char *base, bool isSpeech) {
	if (isSpeech && !speechOn()) return;
	else if (!sfxOn()) return;

	char name[13];
	strcpy(name, base);
	// alter filename to add zeros and append ".SB"
	for (int i = 0; i < 8; i++) {
		if (name[i] == ' ')
			name[i] = '0';
	}
	strcat(name, ".SB");
	waitFinished(isSpeech);
	if (sfxPlay(name, isSpeech ? &_speechHandle : &_sfxHandle)) {
		_speechSfxExists = isSpeech;
	} else {
		_speechSfxExists = false;
	}
}

void Sound::playSong(int16 songNum) {
	if (songNum <= 0) {
		_vm->music()->stopSong();
		return;
	}

	int16 newTune;
	if (_vm->resource()->isDemo()) {
		if (songNum == 17) {
			_vm->music()->stopSong();
			return;
		}
		newTune = _songDemo[songNum - 1].tuneList[0] - 1;
	} else {
		newTune = _song[songNum - 1].tuneList[0] - 1;
	}

	if (_tune[newTune].sfx[0]) {
		if (sfxOn())
			playSfx(_tune[newTune].sfx[0], false);
		return;
	}

	if (!musicOn())
		return;

	int override = (_vm->resource()->isDemo()) ? _songDemo[songNum - 1].override : _song[songNum - 1].override;
	switch (override) {
	// Override all songs
	case  1:
		break;
	// Alter song settings (such as volume) and exit
	case  2:
		_vm->music()->toggleVChange();
	default:
		return;
		break;
	}

	_lastOverride = songNum;

	_vm->music()->queueTuneList(newTune);
	_vm->music()->playMusic();
}

void Sound::saveState(byte *&ptr) {
	WRITE_BE_UINT16(ptr, _lastOverride); ptr += 2;
}

void Sound::loadState(uint32 ver, byte *&ptr) {
	_lastOverride = (int16)READ_BE_INT16(ptr); ptr += 2;
}

bool SilentSound::sfxPlay(const char *name, Audio::SoundHandle *soundHandle) {
	return false;
}

bool SBSound::sfxPlay(const char *name, Audio::SoundHandle *soundHandle) {
	if (_vm->resource()->fileExists(name)) {
		uint32 size;
		uint8 *sound = _vm->resource()->loadFile(name, SB_HEADER_SIZE, &size, true);
		byte flags = Audio::Mixer::FLAG_UNSIGNED | Audio::Mixer::FLAG_AUTOFREE;
		_mixer->playRaw(soundHandle, sound, size, 11025, flags);
		return true;
	}
	return false;
}

#ifdef USE_MAD
bool MP3Sound::sfxPlay(const char *name, Audio::SoundHandle *soundHandle) {
	uint32 size;
	Common::File *f = _vm->resource()->giveCompressedSound(name, &size);
	if (f) {
		_mixer->playInputStream(Audio::Mixer::kSFXSoundType, soundHandle, Audio::makeMP3Stream(f, size));
		return true;
	}
	return false;
}
#endif

#ifdef USE_VORBIS
bool OGGSound::sfxPlay(const char *name, Audio::SoundHandle *soundHandle) {
	uint32 size;
	Common::File *f = _vm->resource()->giveCompressedSound(name, &size);
	if (f) {
		_mixer->playInputStream(Audio::Mixer::kSFXSoundType, soundHandle, Audio::makeVorbisStream(f, size));
		return true;
	}
	return false;
}
#endif

#ifdef USE_FLAC
bool FLACSound::sfxPlay(const char *name, Audio::SoundHandle *soundHandle) {
	uint32 size;
	Common::File *f = _vm->resource()->giveCompressedSound(name, &size);
	if (f) {
		_mixer->playInputStream(Audio::Mixer::kSFXSoundType, soundHandle, Audio::makeFlacStream(f, size));
		return true;
	}
	return false;
}
#endif

} //End of namespace Queen
