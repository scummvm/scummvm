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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/engines/m4/midi.cpp $
 * $Id: midi.cpp 47705 2010-01-30 09:21:07Z dreammaster $
 *
 */

#include "common/stream.h"

#include "prisoner/prisoner.h"
#include "prisoner/resource.h"

namespace Prisoner {

void PrisonerEngine::playSound(int16 soundIndex) {
	if (soundIndex >= 0) {
		SoundSlot &soundSlot = _sounds[soundIndex];
		stopSound(soundIndex);
		// If no other volume was set use the default one
		if (!soundSlot.volumeFlag)
			setSoundVolume(soundIndex, 100);
		soundSlot.shouldResume = false;
		SoundResource *soundResource = _res->get<SoundResource>(soundSlot.resourceCacheSlot);
		Audio::AudioStream *audioStream = soundResource->createAudioStream();
		_mixer->playStream(Audio::Mixer::kSFXSoundType, &soundSlot.handle,
			audioStream, -1, soundSlot.volume, 0, DisposeAfterUse::YES);
	}
}

// TODO: Merge with playSound
void PrisonerEngine::playLoopingSound(int16 soundIndex, int16 loops) {
	if (soundIndex >= 0) {
		SoundSlot &soundSlot = _sounds[soundIndex];
		stopSound(soundIndex);
		// If no other volume was set use the default one
		if (!soundSlot.volumeFlag)
			setSoundVolume(soundIndex, 100);
		soundSlot.shouldResume = loops == 0;
		SoundResource *soundResource = _res->get<SoundResource>(soundSlot.resourceCacheSlot);
		Audio::AudioStream *audioStream = soundResource->createLoopingAudioStream(loops);
		_mixer->playStream(Audio::Mixer::kSFXSoundType, &soundSlot.handle,
			audioStream, -1, soundSlot.volume, 0, DisposeAfterUse::YES);
	}
}

void PrisonerEngine::stopSound(int16 soundIndex) {
	if (soundIndex >= 0) {
		SoundSlot &soundSlot = _sounds[soundIndex];
	    if (_mixer->isSoundHandleActive(soundSlot.handle))
			_mixer->stopHandle(soundSlot.handle);
		soundSlot.shouldResume = false;
	}
}

bool PrisonerEngine::isSoundPlaying(int16 soundIndex) {
	if (soundIndex >= 0) {
		SoundSlot &soundSlot = _sounds[soundIndex];
		return _mixer->isSoundHandleActive(soundSlot.handle);
	} else
		return false;
}

void PrisonerEngine::setSoundVolume(int16 soundIndex, uint volume) {
	if (soundIndex >= 0) {
		SoundSlot &soundSlot = _sounds[soundIndex];
		soundSlot.volume = volume;
		soundSlot.volumeFlag = true;
		// Update the volume if the sound is playing
		if (_mixer->isSoundHandleActive(soundSlot.handle)) {
			_mixer->setChannelVolume(soundSlot.handle, Audio::Mixer::kMaxChannelVolume / 100 * volume);
		}
	}
}

int16 PrisonerEngine::loadSound(Common::String &pakName, int16 pakSlot, bool moduleWide) {
	int16 soundIndex = _sounds.getFreeSlot();
	SoundSlot &soundSlot = _sounds[soundIndex];
	soundSlot.resourceCacheSlot = _res->load<SoundResource>(pakName, pakSlot, 12);
	soundSlot.pakName = pakName;
	soundSlot.pakSlot = pakSlot;
	soundSlot.moduleWide = moduleWide;
	soundSlot.shouldResume = false;
	soundSlot.volumeFlag = false;
	return soundIndex;
}

void PrisonerEngine::unloadSound(int16 soundIndex) {
	if (soundIndex >= 0) {
		SoundSlot &soundSlot = _sounds[soundIndex];
		stopSound(soundIndex);
		_res->unload(soundSlot.resourceCacheSlot);
		soundSlot.resourceCacheSlot = -1;
		clearActorFrameSoundsBySoundIndex(soundIndex);
	}
}

void PrisonerEngine::unloadSounds(bool all) {
	for (int16 soundIndex = 0; soundIndex < kMaxSounds; soundIndex++) {
		SoundSlot &soundSlot = _sounds[soundIndex];
		if (soundSlot.resourceCacheSlot != -1 && (all || !soundSlot.moduleWide)) {
			stopSound(soundIndex);
			_res->unload(soundSlot.resourceCacheSlot);
			soundSlot.resourceCacheSlot = -1;
		}
	}
}

} // End of namespace Prisoner
