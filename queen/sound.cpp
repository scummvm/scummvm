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

#ifdef USE_VORBIS
// These are wrapper functions to allow using a File object to
// provide data to the OggVorbis_File object.

struct file_info {
	File *file;
	int start, curr_pos;
	size_t len;
};

static size_t read_wrap(void *ptr, size_t size, size_t nmemb, void *datasource) {
	file_info *f = (file_info *) datasource;
	int result;

	nmemb *= size;
	if (f->curr_pos > (int) f->len)
		nmemb = 0;
	else if (nmemb > f->len - f->curr_pos)
		nmemb = f->len - f->curr_pos;
	result = f->file->read(ptr, nmemb);
	if (result == -1) {
		f->curr_pos = f->file->pos() - f->start;
		return (size_t) -1;
	} else {
		f->curr_pos += result;
		return result / size;
	}
}

static int seek_wrap(void *datasource, ogg_int64_t offset, int whence) {
	file_info *f = (file_info *) datasource;

	if (whence == SEEK_SET)
		offset += f->start;
	else if (whence == SEEK_END) {
		offset += f->start + f->len;
		whence = SEEK_SET;
	}

	f->file->seek(offset, whence);
	f->curr_pos = f->file->pos() - f->start;
	return f->curr_pos;
}

static int close_wrap(void *datasource) {
	file_info *f = (file_info *) datasource;

	f->file->close();
	delete f;
	return 0;
}

static long tell_wrap(void *datasource) {
	file_info *f = (file_info *) datasource;

	return f->curr_pos;
}

static ov_callbacks g_File_wrap = {
	read_wrap, seek_wrap, close_wrap, tell_wrap
};
#endif

Sound::Sound(SoundMixer *mixer, QueenEngine *vm) : 
	_mixer(mixer), _vm(vm), _sfxToggle(true), _speechToggle(true), _musicToggle(true), _lastOverride(0), _currentSong(0), _sfxHandle(0) {
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
	while(_sfxHandle != 0)
		_vm->input()->delay(10);
}

void Sound::playSong(int16 songNum) {
	if (songNum == STOP_MUSIC) {
		_vm->music()->stopSong();
		return;
	}
	
	int16 newTune = _song[songNum - 1].tuneList[0];

	if (_tune[newTune - 1].sfx[0]) {
		if (sfxOn())
			sfxPlay(_sfxName[_tune[newTune - 1].sfx[0] - 1]);
		return;
	}

	if (!musicOn())
		return;

	//TODO: Record onto song stack for saving/loading
	
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

	_vm->music()->playSong(_tune[newTune - 1].tuneNum[0] - 1);
}


int SBSound::playSound(byte *sound, uint32 size) {
	byte flags = 0 | SoundMixer::FLAG_UNSIGNED | SoundMixer::FLAG_AUTOFREE;
	return _mixer->playRaw(&_sfxHandle, sound, size, 11025, flags);
}

void SBSound::sfxPlay(const char *base) {
	char name[13];
	strcpy(name, base);
	//alter filename to add zeros and append ".SB"
	for (int i = 0; i < 8; i++) {
		if (name[i] == ' ')
			name[i] = '0';
	}
	strcat(name, ".SB");

	waitSfxFinished();
	
	if (_vm->resource()->exists(name)) 
		playSound(_vm->resource()->loadFileMalloc(name, SB_HEADER_SIZE), _vm->resource()->fileSize(name) - SB_HEADER_SIZE);
}

#ifdef USE_MAD
void MP3Sound::sfxPlay(const char *base) {
	char name[13];
	strcpy(name, base);
	//alter filename to add zeros and append ".SB"
	for (int i = 0; i < 8; i++) {
		if (name[i] == ' ')
			name[i] = '0';
	}
	strcat(name, ".SB");
	
	waitSfxFinished();

	if (_vm->resource()->exists(name)) 
		_mixer->playMP3(&_sfxHandle, _vm->resource()->giveCompressedSound(name), _vm->resource()->fileSize(name));
}
#endif

#ifdef USE_VORBIS
void OGGSound::sfxPlay(const char *base) {
	char name[13];
	strcpy(name, base);
	//alter filename to add zeros and append ".SB"
	for (int i = 0; i < 8; i++) {
		if (name[i] == ' ')
			name[i] = '0';
	}
	strcat(name, ".SB");

	waitSfxFinished();
	
	if (_vm->resource()->exists(name)) {
		OggVorbis_File *oggFile = new OggVorbis_File;
		file_info *f = new file_info;

		f->file = _vm->resource()->giveCompressedSound(name);
		f->start = _vm->resource()->fileOffset(name);
		f->len = _vm->resource()->fileSize(name);
		f->curr_pos = 0;

		if (ov_open_callbacks((void *)f, oggFile, NULL, 0, g_File_wrap) < 0) {
			delete oggFile;
			delete f;
		} else
			_mixer->playVorbis(&_sfxHandle, oggFile, 0, false);
	}
}
#endif

} //End of namespace Queen
