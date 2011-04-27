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

void ASound::play(int soundNum, Action *action, int volume) {
	
}

void ASound::stop() {

}

void ASound::prime(int v, Action *action) {

}

void ASound::unPrime() {

}

void ASound::go() {

}

void ASound::hault(void) {
}

int ASound::getSoundNum() const { return 0; }
bool ASound::isPlaying() const { return false; }
bool ASound::isPaused() const { return false; }
bool ASound::isMuted() const { return false; }

void ASound::pause() {

}

void ASound::mute() {

}

void ASound::fadeIn() {

}

void ASound::fadeOut(EventHandler *evtHandler) {

}

void ASound::fade(int v1, int v2, int v3, int v4, EventHandler *evtHandler) {

}

void ASound::setTimeIndex(uint32 timeIndex) {

}

uint32 ASound::getTimeIndex() const { 
	return 0; 
}

void ASound::setPri(int v) {

}

void ASound::setLoop(bool flag) {

}

int ASound::getPri() const { 
	return 0; 
}

bool ASound::getLoop() { 
	return false; 
}

void ASound::setVolume(int volume) {

}

int ASound::getVol() const {
	return 0;
}

void ASound::holdAt(int v) {

}

void ASound::release() {

}


} // End of namespace tSage
