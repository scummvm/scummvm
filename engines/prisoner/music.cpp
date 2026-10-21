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

#include "audio/audiostream.h"
#include "audio/mixer.h"

#include "prisoner/prisoner.h"
#include "prisoner/resource.h"
#include "prisoner/midi.h"

namespace Prisoner {

void PrisonerEngine::playMusic(int16 musicIndex, bool loop, int16 loopCount) {
	if (musicIndex >= 0) {
		MusicSlot &musicSlot = _musics[musicIndex];
		stopMusic(musicIndex);
		// If no other volume was set use the default one
		if (!musicSlot.volumeFlag)
			setMusicVolume(musicIndex, 100); // TODO
		musicSlot.isPlaying = true;
		musicSlot.shouldResume = false;
		musicSlot.loop = loop;
		musicSlot.loopCount = loopCount;
		if (musicSlot.isDigital) {
			// The original restarts a looping track at its end and decrements the
			// loop count each time; a count <= 0 loops forever, otherwise the track
			// plays loopCount + 1 times.
			uint loops = 1;
			if (loop)
				loops = loopCount <= 0 ? 0 : loopCount + 1;
			SoundResource *soundResource = _res->get<SoundResource>(musicSlot.resourceCacheSlot);
			Audio::AudioStream *audioStream = soundResource->createLoopingAudioStream(loops);
			_mixer->playStream(Audio::Mixer::kMusicSoundType, &musicSlot.handle,
				audioStream, -1, toMixerVolume(musicSlot.volume), 0, DisposeAfterUse::YES);
		} else {
			MidiResource *midiResource = _res->get<MidiResource>(musicSlot.resourceCacheSlot);
			_midi->playMusic(midiResource, musicSlot.volume, loop);
		}
	}
}

void PrisonerEngine::stopMusic(int16 musicIndex) {
	if (musicIndex >= 0) {
		MusicSlot &musicSlot = _musics[musicIndex];
		if (musicSlot.isDigital) {
			if (_mixer->isSoundHandleActive(musicSlot.handle))
				_mixer->stopHandle(musicSlot.handle);
		} else {
			_midi->stopMusic();
		}
		musicSlot.isPlaying = false;
		musicSlot.shouldResume = false;
	}
}

void PrisonerEngine::stopMusics() {
	for (int16 musicIndex = 0; musicIndex < kMaxMusics; musicIndex++) {
		if (!_musics[musicIndex].isEmpty() && _musics[musicIndex].isPlaying) {
			stopMusic(musicIndex);
		}
	}
}

bool PrisonerEngine::isMusicPlaying(int16 musicIndex) {
	if (musicIndex >= 0) {
		MusicSlot &musicSlot = _musics[musicIndex];
		if (musicSlot.isDigital)
			return _mixer->isSoundHandleActive(musicSlot.handle);
		return musicSlot.isPlaying;
	} else
		return false;
}

void PrisonerEngine::setMusicVolume(int16 musicIndex, uint volume) {
	if (musicIndex >= 0) {
		MusicSlot &musicSlot = _musics[musicIndex];
		musicSlot.volume = volume;
		musicSlot.volumeFlag = true;
		// Update the volume if the sound is playing
		if (musicSlot.isPlaying) {
			if (musicSlot.isDigital) {
				if (_mixer->isSoundHandleActive(musicSlot.handle))
					_mixer->setChannelVolume(musicSlot.handle, toMixerVolume(volume));
			} else {
				_midi->setVolume(volume);
			}
		}
	}
}

int16 PrisonerEngine::loadMusic(Common::String &pakName, int16 pakSlot, bool moduleWide) {
	// The original only has one music slot and always replaces what's in it
	const int16 musicIndex = 0;
	if (!_musics[musicIndex].isEmpty())
		unloadMusic(musicIndex);
	MusicSlot &musicSlot = _musics[musicIndex];
	musicSlot.isDigital = _digitalMusic;
	if (musicSlot.isDigital) {
		// Digital music lives in the "W" paks ("MUS02" -> "WUS02"), same slot
		Common::String digiPakName = pakName;
		digiPakName.setChar('W', 0);
		musicSlot.resourceCacheSlot = _res->load<SoundResource>(digiPakName, pakSlot, 12);
	} else {
		musicSlot.resourceCacheSlot = _res->load<MidiResource>(pakName, pakSlot, 13);
	}
	musicSlot.moduleWide = moduleWide;
	musicSlot.isPlaying = false;
	musicSlot.shouldResume = false;
	musicSlot.volumeFlag = false;
	musicSlot.loop = false;
	musicSlot.loopCount = 0;
	return musicIndex;
}

void PrisonerEngine::unloadMusic(int16 musicIndex) {
	if (musicIndex >= 0) {
		MusicSlot &musicSlot = _musics[musicIndex];
		stopMusic(musicIndex);
		_res->unload(musicSlot.resourceCacheSlot);
		musicSlot.resourceCacheSlot = -1;
	}
}

void PrisonerEngine::unloadMusics(bool all) {
	for (int16 musicIndex = 0; musicIndex < kMaxMusics; musicIndex++) {
		MusicSlot &musicSlot = _musics[musicIndex];
		if (musicSlot.resourceCacheSlot != -1 && (all || !musicSlot.moduleWide)) {
			stopMusic(musicIndex);
			_res->unload(musicSlot.resourceCacheSlot);
			musicSlot.resourceCacheSlot = -1;
		}
	}
}

// Stops the music and remembers whether it was playing (original: stopMusics)
void PrisonerEngine::suspendMusics() {
	for (int16 musicIndex = 0; musicIndex < kMaxMusics; musicIndex++) {
		MusicSlot &musicSlot = _musics[musicIndex];
		if (!musicSlot.isEmpty()) {
			bool wasPlaying = isMusicPlaying(musicIndex);
			stopMusic(musicIndex);
			musicSlot.shouldResume = wasPlaying;
		}
	}
}

// Restarts the music stopped by suspendMusics with its loop setting (original: resumeMusics)
void PrisonerEngine::resumeMusics() {
	for (int16 musicIndex = 0; musicIndex < kMaxMusics; musicIndex++) {
		MusicSlot &musicSlot = _musics[musicIndex];
		if (!musicSlot.isEmpty() && musicSlot.shouldResume) {
			setMusicVolume(musicIndex, musicSlot.volume);
			playMusic(musicIndex, musicSlot.loop, musicSlot.loopCount);
		}
	}
}

} // End of namespace Prisoner
