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

#include "backends/cloud/storage.h"
#include "common/system.h"
#include "common/timer.h"

namespace Cloud {

void cloudThread(void *thread) {
	Storage *cloudThread = (Storage *)thread;
	cloudThread->handler();
}

Storage::Storage() : _timerStarted(false) {}

void Storage::addRequest(Request *request) {
	_requests.push_back(request);
	if (!_timerStarted) startTimer();
}

void Storage::handler() {
	//TODO: lock mutex here (in case another handler() would be called before this one ends)
	warning("handler's here");
	for (Common::Array<Request *>::iterator i = _requests.begin(); i != _requests.end();) {
		if ((*i)->handle()) _requests.erase(i);
		else ++i;
	}
	if (_requests.empty()) stopTimer();
	//TODO: unlock mutex here
}

void Storage::startTimer(int interval) {
	Common::TimerManager *manager = g_system->getTimerManager();
	if (manager->installTimerProc(cloudThread, interval, this, "Cloud Thread")) {
		_timerStarted = true;
	} else {
		warning("Failed to create cloud thread");		
	}
}

void Storage::stopTimer() {
	Common::TimerManager *manager = g_system->getTimerManager();
	manager->removeTimerProc(cloudThread);
	_timerStarted = false;
}

} //end of namespace Cloud
