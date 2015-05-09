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

#include "sherlock/sherlock.h"
#include "sherlock/sound.h"
#include "common/config-manager.h"

namespace Sherlock {

Sound::Sound(SherlockEngine *vm): _vm(vm) {
	_digitized = false;
	_music = false;
	_voices = 0;
	_playingEpilogue = false;
	_diskSoundPlaying = false;
	_soundPlaying = false;
	_soundIsOn = &_soundPlaying;

	_soundOn = true;
	_musicOn = true;
	_speechOn = true;
}

/**
 * Saves sound-related settings
 */
void Sound::syncSoundSettings() {
	_digitized = !ConfMan.getBool("mute");
	_music = !ConfMan.getBool("mute") && !ConfMan.getBool("music_mute");
	_voices = !ConfMan.getBool("mute") && !ConfMan.getBool("speech_mute") ? 1 : 0;
}

void Sound::loadSound(const Common::String &name, int priority) {
	// TODO
	warning("TODO: Sound::loadSound");
}

bool Sound::playSound(const Common::String &name, WaitType waitType) {
	// TODO
	warning("TODO: Sound::playSound");
	return true;
}

void Sound::cacheSound(const Common::String &name, int index) {
	// TODO
	warning("TODO: Sound::cacheSound");
}

void Sound::playLoadedSound(int bufNum, int waitMode) {
	// TODO
	warning("TODO: Sound::playLoadedSound");
}

void Sound::playCachedSound(int index) {
	// TODO
	warning("TODO: Sound::playCachedSound");
}

void Sound::freeLoadedSounds() {
	// TODO
	warning("TODO: Sound::clearLoadedSound");
}

void Sound::clearCache() {
	// TODO
	warning("TODO: Sound::clearCache");
}

void Sound::stopSound() {
	// TODO
	warning("TODO: Sound::stopSound");
}

void Sound::playMusic(const Common::String &name) {
	// TODO
	warning("TODO: Sound::playMusic");
}

void Sound::stopMusic() {
	// TODO
	warning("TODO: Sound::stopMusic");
}

int Sound::loadSong(int songNumber) {
	// TODO
	warning("TODO: Sound::loadSong");
	return 0;
}

void Sound::startSong() {
	// TODO
	warning("TODO: Sound::startSong");
}

void Sound::freeSong() {
	// TODO
	warning("TODO: Sound::freeSong");
}

void Sound::stopSndFuncPtr(int v1, int v2) {
	// TODO
	warning("TODO: Sound::stopSndFuncPtr");
}

void Sound::waitTimerRoland(uint time) {
	// TODO
	warning("TODO: Sound::waitTimerRoland");
}

void Sound::freeDigiSound() {
	delete[] _digiBuf;
	_digiBuf = nullptr;
	_diskSoundPlaying = false;
	_soundPlaying = false;
}

} // End of namespace Sherlock
