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
#include "common/file.h"
#include "common/util.h"
#include "queen/input.h"
#include "queen/resource.h"
#include "queen/sound.h"

#define	SB_HEADER_SIZE	110

namespace Queen {

Sound::Sound(SoundMixer *mixer, Input *input, Resource  *resource) : 
  _mixer(mixer), _input(input), _resource(resource), _sfxHandle(0) {
}

Sound::~Sound() {
}

Sound *Sound::giveSound(SoundMixer *mixer, Input *input, Resource *resource, uint8 compression) {
	switch(compression) {
		case COMPRESSION_NONE:
				return new SBSound(mixer, input, resource);
				break;
		case COMPRESSION_MP3:
				#ifndef USE_MAD
					warning("Using MP3 compressed datafile, but MP3 support not compiled in");
					return new SilentSound(mixer, input, resource);
				#else
					return new MP3Sound(mixer, input, resource);

				#endif
				break;
		default:
				warning("Unknown compression type");
				return new SilentSound(mixer, input, resource);
	}
}

bool Sound::isPlaying() {
	return _sfxHandle != 0;
}

int SBSound::playSound(byte *sound, uint32 size) {
	byte flags = 0 | SoundMixer::FLAG_UNSIGNED | SoundMixer::FLAG_AUTODELETE;
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

	while(isPlaying())
	  _input->delay(10);
	
	if (_resource->exists(name)) 
		playSound(_resource->loadFile(name, SB_HEADER_SIZE), _resource->fileSize(name) - SB_HEADER_SIZE);
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
	
	while(isPlaying())
		_input->delay(10);

	if (_resource->exists(name)) 
		_mixer->playMP3(&_sfxHandle, _resource->giveMP3(name), _resource->fileSize(name));
}
#endif

} //End of namespace Queen
