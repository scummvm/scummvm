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
#include "sky/sound.h"
#include "sky/struc.h"

SkySound::SkySound(SoundMixer *mixer) {
	_mixer = mixer;
	_voiceHandle = 0;
	_effectHandle = 0;
	_bgSoundHandle = 0;
}

int SkySound::playVoice(byte *sound, uint32 size) {

	return playSound(sound, size, &_voiceHandle);
}


int SkySound::playBgSound(byte *sound, uint32 size) {

	size -= 512; //Hack to get rid of the annoying pop at the end of some bg sounds 
	return playSound(sound, size, &_bgSoundHandle);
}

int SkySound::playSound(byte *sound, uint32 size, PlayingSoundHandle *handle) {

	byte flags = 0;
	flags |= SoundMixer::FLAG_UNSIGNED|SoundMixer::FLAG_AUTOFREE;
	size -= sizeof(struct dataFileHeader);
	byte *buffer = (byte *)malloc(size); 
	memcpy(buffer, sound+sizeof(struct dataFileHeader), size);	
	
	return _mixer->playRaw(handle, buffer, size, 11025, flags);
}
