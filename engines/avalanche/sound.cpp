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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "avalanche/avalanche.h"
#include "avalanche/sound.h"

#include "audio/softsynth/pcspk.h"
#include "common/config-manager.h"

namespace Avalanche {

SoundHandler::SoundHandler(AvalancheEngine *vm) : _vm(vm) {
	_soundFl = true;
	_speakerStream = new Audio::PCSpeaker(_vm->_mixer->getOutputRate());
	_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, &_speakerHandle,
						_speakerStream, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::YES, true);
}

SoundHandler::~SoundHandler() {
	_vm->_mixer->stopHandle(_speakerHandle);
}

/**
 * Stop any sound that might be playing
 */
void SoundHandler::stopSound() {
	_vm->_mixer->stopAll();
}

/**
 * Turn digitized sound on and off
 */
void SoundHandler::toggleSound() {
	_soundFl = !_soundFl;
	_vm->_graphics->drawSoundLight(_soundFl);
}

void SoundHandler::syncVolume() {
	int soundVolume;

	if (ConfMan.getBool("sfx_mute") || ConfMan.getBool("mute"))
		soundVolume = -1;
	else
		soundVolume = MIN(255, ConfMan.getInt("sfx_volume"));

	_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, soundVolume);
}

void SoundHandler::playNote(int freq, int length) {
	// Does the user not want any sound?
	if (!_soundFl || !_vm->_mixer->isReady())
		return;

	// Start a note playing (we will stop it when the timer expires).
	_speakerStream->play(Audio::PCSpeaker::kWaveFormSquare, freq, length);
}

void SoundHandler::click() {
	_vm->_mixer->stopAll();

	playNote(7177, 1);
}

void SoundHandler::blip() {
	_vm->_mixer->stopAll();

	playNote(177, 77);
}

} // End of namespace Avalanche
