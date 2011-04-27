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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/config-manager.h"
#include "common/endian.h"
#include "tsage/core.h"
#include "tsage/globals.h"
#include "tsage/debugger.h"
#include "tsage/graphics.h"

namespace tSage {

void SoundManager::postInit() {
	_saver->addSaveNotifier(&SoundManager::saveNotifier);
	_saver->addLoadNotifier(&SoundManager::loadNotifier);
	_saver->addListener(this);
}

void SoundManager::saveNotifier(bool postFlag) {
	_globals->_soundManager.saveNotifierProc(postFlag);
}

void SoundManager::saveNotifierProc(bool postFlag) {
	warning("TODO: SoundManager::saveNotifierProc");
}

void SoundManager::loadNotifier(bool postFlag) {
	_globals->_soundManager.loadNotifierProc(postFlag);
}

void SoundManager::loadNotifierProc(bool postFlag) {
	warning("TODO: SoundManager::loadNotifierProc");
}

void SoundManager::listenerSynchronise(Serialiser &s) {
	s.validate("SoundManager");
	warning("TODO: SoundManager listenerSynchronise");
}

/*--------------------------------------------------------------------------*/

void Sound::play(int soundNum, int volume) {

}

void Sound::stop() {

}

void Sound::prime(int soundNum) {

}

void Sound::prime(int soundNum, int v2) {

}

void Sound::_unPrime() {

}

void Sound::unPrime() {

}

void Sound::go() {

}

void Sound::halt(void) {

}

int Sound::getSoundNum() const {
	return 0;
}

bool Sound::isPlaying() const {
	return false;
}

bool Sound::isPrimed() const {
	return false;
}

bool Sound::isPaused() const {
	return false;
}

bool Sound::isMuted() const {
	return false;
}

void Sound::pause() {

}

void Sound::mute() {

}

void Sound::fadeIn() {

}

void Sound::fadeOut(EventHandler *evtHandler) {

}

void Sound::fade(int v1, int v2, int v3, int v4) {

}

void Sound::setTimeIndex(uint32 timeIndex) {

}

uint32 Sound::getTimeIndex() const {
	return 0;
}

bool Sound::getCueValue() const {
	return false;
}

void Sound::setCueValue(bool flag) {

}

void Sound::setVol(int volume) {

}

int Sound::getVol() const {
	return 0;
}

void Sound::setPri(int v) {

}

void Sound::setLoop(bool flag) {

}

int Sound::getPri() const {
	return 0;
}

bool Sound::getLoop() {
	return false;
}

void Sound::holdAt(int v) {

}

void Sound::release() {

}


/*--------------------------------------------------------------------------*/

ASound::ASound(): EventHandler() {
	_action = NULL;
	_cueFlag = false;
}

void ASound::synchronise(Serialiser &s) {
	EventHandler::synchronise(s);
	SYNC_POINTER(_action);
	s.syncAsByte(_cueFlag);
}

void ASound::dispatch() {
	EventHandler::dispatch();

	if (!_sound.getCueValue()) {
		_cueFlag = false;
		_sound.setCueValue(true);

		if (_action)
			_action->signal();
	}

	if (!_cueFlag) {
		if (!_sound.isPrimed()) {
			_cueFlag = true;
			if (_action) {
				_action->signal();
				_action = NULL;
			}
		}
	}
}

void ASound::play(int soundNum, Action *action, int volume) {
	_action = action;
	_cueFlag = false;
	
	setVol(volume);
	_sound.play(soundNum);
}

void ASound::stop() {
	_sound.stop();
	_action = NULL;
}

void ASound::prime(int soundNum, Action *action) {
	_action = action;
	_cueFlag = false;
	_sound.prime(soundNum);
}

void ASound::unPrime() {
	_sound.unPrime();
	_action = NULL;
}

void ASound::fade(int v1, int v2, int v3, int v4, Action *action) {
	if (action)
		_action = action;

	_sound.fade(v1, v2, v3, v4);
}

} // End of namespace tSage
