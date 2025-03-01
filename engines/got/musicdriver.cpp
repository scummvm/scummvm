/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "got/musicdriver.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/system.h"

namespace Got {

MusicDriver_Got::MusicDriver_Got(uint8 timerFrequency) : _isOpen(false), _timer_param(nullptr), _timer_proc(nullptr) {
	setTimerFrequency(timerFrequency);
}

bool MusicDriver_Got::isOpen() const {
	return _isOpen;
}

void MusicDriver_Got::setTimerFrequency(uint8 timerFrequency) {
	assert(timerFrequency > 0);

	_timerFrequency = timerFrequency;
}

void MusicDriver_Got::onTimer() {
	if (_timer_proc && _timer_param)
		_timer_proc(_timer_param);
}

void MusicDriver_Got::setTimerCallback(void *timer_param, Common::TimerManager::TimerProc timer_proc) {
	_timer_param = timer_param;
	_timer_proc = timer_proc;
}

int MusicDriver_Got_NULL::open() {
	// There is no output device, so register a timer to trigger the callbacks.
	g_system->getTimerManager()->installTimerProc(timerCallback, 1000000 / _timerFrequency, this, "MusicDriver_Got_NULL");

	_isOpen = true;

	return 0;
}

void MusicDriver_Got_NULL::close() {
	if (!_isOpen)
		return;

	g_system->getTimerManager()->removeTimerProc(timerCallback);

	_isOpen = false;
}

void MusicDriver_Got_NULL::setTimerFrequency(uint8 timerFrequency) {
	if (timerFrequency == _timerFrequency)
		return;

	MusicDriver_Got::setTimerFrequency(timerFrequency);

	// Update the timer frequency.
	g_system->getTimerManager()->removeTimerProc(timerCallback);
	g_system->getTimerManager()->installTimerProc(timerCallback, 1000000 / _timerFrequency, this, "MusicDriver_Got_NULL");
}

void MusicDriver_Got_NULL::timerCallback(void *data) {
	MusicDriver_Got_NULL *driver = (MusicDriver_Got_NULL *)data;
	driver->onTimer();
}

} // namespace Got
