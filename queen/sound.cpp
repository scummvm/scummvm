/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "queen/sound.h"

#include "queen/input.h"
#include "queen/music.h"
#include "queen/queen.h"
#include "queen/resource.h"

#define	SB_HEADER_SIZE	110
#define	STOP_MUSIC	-1

namespace Queen {

Sound::Sound(SoundMixer *mixer, QueenEngine *vm) : 
	_mixer(mixer), _vm(vm), _sfxToggle(true), _speechToggle(true), _musicToggle(true), _lastOverride(0), _currentSong(0) {
}

Sound::~Sound() {
}

Sound *Sound::giveSound(SoundMixer *mixer, QueenEngine *vm, uint8 compression) {
	switch(compression) {
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
		default:
				warning("Unknown compression type");
				return new SilentSound(mixer, vm);
	}
}

void Sound::waitSfxFinished() {
	while(_sfxHandle.isActive())
		_vm->input()->delay(10);
}

void Sound::playSfx(uint16 sfx) {
	if (sfx != 0) {
		char name[13];
		strcpy(name, _sfxName[sfx - 1]);
		strcat(name, ".SB");
		sfxPlay(name);
	}
}

void Sound::playSfx(const char *base) {
	char name[13];
	strcpy(name, base);
	// alter filename to add zeros and append ".SB"
	for (int i = 0; i < 8; i++) {
		if (name[i] == ' ')
			name[i] = '0';
	}
	strcat(name, ".SB");
	sfxPlay(name);
}

void Sound::playSong(int16 songNum) {
	if (songNum == STOP_MUSIC) {
		_vm->music()->stopSong();
		return;
	}
	
	int16 newTune = _song[songNum - 1].tuneList[0];

	if (_tune[newTune - 1].sfx[0]) {
		if (sfxOn())
			playSfx(_tune[newTune - 1].sfx[0]);
		return;
	}

	if (!musicOn())
		return;

	_lastOverride = songNum;
	
	switch (_tune[newTune - 1].mode) {
		//Random loop
		case  0:
			warning("Music: Random loop not yet supported (doing sequential loop instead)");
		//Sequential loop
		case  1:
			_vm->music()->loop(true);
			break;
		//Play once
		case  2:
		default:
			_vm->music()->loop(false);
			break;
	}

	int16 song = _tune[newTune - 1].tuneNum[0] - 1;

	// Work around bug in Roland music, note that these numbers are 'one-off' from
	// the original code.
	if (/*isRoland && */ song == 88 || song == 89) {
		warning("Working around Roland music bug");
		song = 62;
	}
	
	_vm->music()->playSong(song);
}


void SBSound::playSound(byte *sound, uint32 size) {
	byte flags = SoundMixer::FLAG_UNSIGNED | SoundMixer::FLAG_AUTOFREE;
	_mixer->playRaw(&_sfxHandle, sound, size, 11025, flags);
}

void SBSound::sfxPlay(const char *name) {
	waitSfxFinished();	
	if (_vm->resource()->exists(name)) 
		playSound(_vm->resource()->loadFileMalloc(name, SB_HEADER_SIZE), _vm->resource()->fileSize(name) - SB_HEADER_SIZE);
}

#ifdef USE_MAD
void MP3Sound::sfxPlay(const char *name) {
	waitSfxFinished();
	if (_vm->resource()->exists(name)) 
		_mixer->playMP3(&_sfxHandle, _vm->resource()->giveCompressedSound(name), _vm->resource()->fileSize(name));
}
#endif

#ifdef USE_VORBIS
void OGGSound::sfxPlay(const char *name) {
	waitSfxFinished();	
	if (_vm->resource()->exists(name))
		_mixer->playVorbis(&_sfxHandle, _vm->resource()->giveCompressedSound(name), _vm->resource()->fileSize(name));
}
#endif

} //End of namespace Queen
