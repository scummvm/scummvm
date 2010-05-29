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
 * $URL$
 * $Id$
 *
 */

#include "backends/platform/sdl/mutex.h"

SdlSubSys_Mutex::SdlSubSys_Mutex()
	:
	_inited(false) {

}

SdlSubSys_Mutex::~SdlSubSys_Mutex() {
	if (_inited) {
		mutexDone();
	}
}

void SdlSubSys_Mutex::mutexInit(OSystem *mainSys) {
	if (_inited) {
		return;
	}
	_mainSys = mainSys;

	_graphicsMutex = createMutex();

	_inited = true;
}

void SdlSubSys_Mutex::mutexDone() {
	deleteMutex(_graphicsMutex);

	_inited = false;
}

bool SdlSubSys_Mutex::hasFeature(Feature f) {
	return false;
}

void SdlSubSys_Mutex::setFeatureState(Feature f, bool enable) {
	
}

bool SdlSubSys_Mutex::getFeatureState(Feature f) {
	return false;
}

OSystem::MutexRef SdlSubSys_Mutex::createMutex() {
	return (MutexRef) SDL_CreateMutex();
}

void SdlSubSys_Mutex::lockMutex(MutexRef mutex) {
	SDL_mutexP((SDL_mutex *) mutex);
}

void SdlSubSys_Mutex::unlockMutex(MutexRef mutex) {
	SDL_mutexV((SDL_mutex *) mutex);
}

void SdlSubSys_Mutex::deleteMutex(MutexRef mutex) {
	SDL_DestroyMutex((SDL_mutex *) mutex);
}
