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

#define SOUND_FILE_BASE 60203
#define SFX_BASE 0xA60

SkySound::SkySound(SoundMixer *mixer, SkyDisk *pDisk) {
	_skyDisk = pDisk;
	_soundData = NULL;
	_mixer = mixer;
	_voiceHandle = 0;
	_effectHandle = 0;
	_bgSoundHandle = 0;
	_ingameSound = 0;
}

SkySound::~SkySound(void) {

	if (_ingameSound) _mixer->stop(_ingameSound - 1);
	if (_soundData) free(_soundData);
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

void SkySound::loadSection(uint8 pSection) {

	if (_ingameSound) _mixer->stop(_ingameSound - 1);
	_ingameSound = 0;
	if (_soundData) free(_soundData);
	_soundData = _skyDisk->loadFile(pSection * 4 + SOUND_FILE_BASE, NULL);
	_sampleRates = _soundData + 0xA4E;
	_sfxInfo = _soundData + 0xA60;
}

void SkySound::playSound(uint16 sound, uint16 volume) {

	if (!_soundData) {
		warning("SkySound::playSound(%04X, %04X) called with a section having been loaded.\n", sound, volume);
		return;
	}
	
	if (sound > 2) {
		if (sound & 0x80) warning("SkySound::playSound(%04X, %04X) not implemented.\n", sound, volume);
		else warning("SkySound::playSound(%04X, %04X) ignored.\n", sound, volume);
		return ;
	}

	volume = ((volume & 0x7F) + 1) << 1;
	sound &= 0xFF;
	
	// note: all those tables are big endian. Don't ask me why. *sigh*
	uint16 sampleRate = (_sampleRates[sound << 2] << 8) | _sampleRates[(sound << 2) | 1];
	uint16 dataOfs = ((_sfxInfo[sound << 3] << 8) | _sfxInfo[(sound << 3) | 1]) << 4;
	dataOfs += SFX_BASE;
	uint16 dataSize = (_sfxInfo[(sound << 3) | 2] << 8) | _sfxInfo[(sound << 3) | 3];
	uint16 dataLoop = (_sfxInfo[(sound << 3) | 6] << 8) | _sfxInfo[(sound << 3) | 7];

	byte flags = SoundMixer::FLAG_UNSIGNED;
	if (dataSize == dataLoop)
		flags |= SoundMixer::FLAG_LOOP;
	
	if (_ingameSound) _mixer->stop(_ingameSound - 1);
	_mixer->setVolume(volume);
	_mixer->playRaw(&_ingameSound, _soundData + dataOfs, dataSize, sampleRate, flags);
}