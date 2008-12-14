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
 * $URL: https://www.switchlink.se/svn/picture/screen.cpp $
 * $Id: screen.cpp 32 2008-09-16 11:18:09Z johndoe $
 *
 */

#include "common/events.h"
#include "common/keyboard.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/config-manager.h"

#include "base/plugins.h"
#include "base/version.h"

#include "graphics/cursorman.h"

#include "sound/mixer.h"

#include "picture/picture.h"
#include "picture/palette.h"
#include "picture/render.h"
#include "picture/resource.h"
#include "picture/screen.h"
#include "picture/script.h"
#include "picture/segmap.h"
#include "picture/sound.h"

namespace Picture {

Sound::Sound(PictureEngine *vm) : _vm(vm) {
	for (int i = 0; i < 4; i++) {
		channels[i].type = 0;
		channels[i].resIndex = -1;
	}
}

Sound::~Sound() {
}

void Sound::playSpeech(int16 resIndex) {

	// TODO

	debug(0, "playSpeech(%d)", resIndex);

	internalPlaySound(resIndex, -3, 50 /*TODO*/, 64);

}

void Sound::playSound(int16 resIndex, int16 type, int16 volume) {

	// TODO: Use the right volumes

	debug(0, "playSound(%d, %d, %d)", resIndex, type, volume);
	
	if (volume == -1 || type == -2) {
		if (type == -1) {
			internalPlaySound(resIndex, type, 50 /*TODO*/, 64);
		} else {
			internalPlaySound(resIndex, type, 100 /*TODO*/, 64);
		}
	} else {
		internalPlaySound(resIndex, type, 100 /*TODO*/, 64);
	}

}

void Sound::playSoundAtPos(int16 resIndex, int16 x, int16 y) {

	// TODO: Everything

	debug(0, "playSoundAtPos(%d, %d, %d)", resIndex, x, y);

	internalPlaySound(resIndex, 1, 50, 64);

}

void Sound::internalPlaySound(int16 resIndex, int16 type, int16 volume, int16 panning) {

	// TODO
	
	if (resIndex == -1) {
		// Stop all sounds
		_vm->_mixer->stopAll();
		_vm->_screen->keepTalkTextItemsAlive();
		for (int i = 0; i < 4; i++) {
			channels[i].type = 0;
			channels[i].resIndex = -1;
		}
	} else if (type == -2) {
		// Stop sounds with specified resIndex
		for (int i = 0; i < 4; i++) {
			if (channels[i].resIndex == resIndex) {
				_vm->_mixer->stopHandle(channels[i].handle);
				channels[i].type = 0;
				channels[i].resIndex = -1;
			}
		}
	} else {

		if (type == -3) {
			// Stop sounds with type == -3 and play new sound
			stopSpeech();
		}
	
		// Play new sound in empty channel
		int freeChannel = -1;
		for (int i = 0; i < 4; i++) {
			if (channels[i].type == 0) {
				freeChannel = i;
				break;
			}
		}
		
		// If all channels are in use no new sound will be played
		if (freeChannel >= 0) {

			byte *soundData = _vm->_res->load(resIndex);
			uint32 soundSize = _vm->_res->getCurItemSize();

			byte flags = Audio::Mixer::FLAG_UNSIGNED;
			// Sounds with type == -1 loop
			if (type == -1)
				flags |= Audio::Mixer::FLAG_LOOP;
			Audio::AudioStream *stream = Audio::makeLinearInputStream(soundData, soundSize, 22050, flags, 0, 0);

			channels[freeChannel].type = type;
			channels[freeChannel].resIndex = resIndex;

			_vm->_mixer->playInputStream(Audio::Mixer::kPlainSoundType/*TODO*/, &channels[freeChannel].handle,
				stream, -1, volume, panning);
			
		}

	}
	
}

void Sound::updateSpeech() {
	for (int i = 0; i < 4; i++) {
		if (channels[i].type == -3 && _vm->_mixer->isSoundHandleActive(channels[i].handle)) {
			_vm->_screen->keepTalkTextItemsAlive();
			break;
		}
	}
}

void Sound::stopSpeech() {
	for (int i = 0; i < 4; i++) {
		if (channels[i].type == -3) {
			_vm->_mixer->stopHandle(channels[i].handle);
			_vm->_screen->keepTalkTextItemsAlive();
			channels[i].type = 0;
			channels[i].resIndex = -1;
		}
	}
}

} // End of namespace Picture
