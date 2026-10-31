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

#include "common/scummsys.h"

#include "engines/grim/movie/smush_anim.h"
#include "engines/grim/movie/smush_sound.h"
#include "engines/grim/movie/smush_timer.h"

namespace Grim {

SmushAnimPlay::SmushAnimPlay(SmushTimer *timer, SmushSound *sound) {
	_timer = timer;
	_sound = sound;
	_frameTimer = 0;
	_syncTimer = 0;
	_flags = 0;
	_finished = 0;
	_framesDue = 0;
	_lateTicks = 0;
	_nominalDelay = 0;
	_currentDelay = 0;
	_skipFlag = 0;
	_state = 0;
	_started = 0;
	_syncPaused = 0;
}

SmushAnimPlay::~SmushAnimPlay() {
	anmStop();
}

int SmushAnimPlay::anmStart(int fps, int flags) {
	_flags = flags;
	_lateTicks = 0;
	_framesDue = 0;
	_skipFlag = 0;
	_started = 0;
	_state = 0;
	_nominalDelay = 1193180 / fps;
	_currentDelay = 1193180 / fps;

	_frameTimer = _timer->sysAlloc(frameTimer, 1, SYS_TIMER_ONE_SHOT, this);
	if (!_frameTimer)
		return -1;

	_syncTimer = _timer->sysAlloc(syncTimer, _nominalDelay, SYS_TIMER_EXT_SYNC, this);
	if (!_syncTimer) {
		_timer->sysFree(_frameTimer);
		_frameTimer = 0;
		return -1;
	}

	_finished = 0;
	return 0;
}

void SmushAnimPlay::anmStop() {
	if (!_frameTimer && !_syncTimer)
		return;

	_timer->sysFree(_frameTimer);
	_timer->sysFree(_syncTimer);
	_frameTimer = 0;
	_syncTimer = 0;

	if (_syncPaused)
		_sound->souEngineResume();
}

int SmushAnimPlay::anmRender(SmushAnimClient *client) {
	if (_finished)
		return -1;

	return anmRenderBody(client);
}

int SmushAnimPlay::anmRenderBody(SmushAnimClient *client) {
	if (_state == 5) {
		int result = client->animatorRender(_flags);
		_state = result;
		if (result < 0)
			_finished = 1;

		return result;
	}

	if (!_framesDue)
		return 6;

	if (!_started)
		client->resetFrameCounter();

	int drift = (int)(int64)(((double)_sound->souEngineQueryTime(0) - (double)client->getFrameCounter() * 66.66666666667) * 0.0149999999999925);
	_lateTicks--;
	_framesDue = 0;

	if (drift > 0 || _state == 2 || _state == 3 || _state == 4) {
		if (_currentDelay > _nominalDelay)
			_currentDelay = _nominalDelay;

		int step = (drift == 1) ? _nominalDelay / 16 : _nominalDelay / 8;
		_currentDelay -= step;
		if (_currentDelay < 1)
			_currentDelay = 1;

		int minimum = 3 * _nominalDelay / 4;
		if (_currentDelay < minimum)
			_currentDelay = minimum;
	} else if (drift < 0) {
		if (_currentDelay < _nominalDelay)
			_currentDelay = _nominalDelay;

		int step = (drift == -1) ? _nominalDelay / 16 : _nominalDelay / 8;
		_currentDelay += step;

		int maximum = 4 * _nominalDelay / 3;
		if (_currentDelay > maximum)
			_currentDelay = maximum;
	}

	_timer->sysReset(_frameTimer, _currentDelay);

	if (drift <= 2) {
		if (drift <= 0)
			_skipFlag = 0;
	} else {
		_skipFlag = 1;
	}

	int flags = _flags;
	if (_skipFlag && !(flags & 2))
		flags |= 0x8000;

	_state = client->animatorRender(flags);
	if (_state < 0)
		_finished = 1;
	if (_state == 1)
		_started = 1;

	return _state;
}

void SmushAnimPlay::anmSetRate(int fps) {
	_nominalDelay = 1193180 / fps;
	_currentDelay = 1193180 / fps;
	_timer->sysReset(_frameTimer, 1193180 / fps);
	_timer->sysReset(_syncTimer, _nominalDelay);
}

void SmushAnimPlay::anmSyncUp() {
	_lateTicks = 0;
	_timer->sysReset(_frameTimer, _currentDelay);
}

bool SmushAnimPlay::anmRenderQuery() {
	return _state == 5 || _framesDue;
}

int SmushAnimPlay::anmGetTime() {
	return 1000 * _currentDelay / 1193180;
}

void SmushAnimPlay::frameTimer(int timer, void *context) {
	((SmushAnimPlay *)context)->_framesDue++;
}

void SmushAnimPlay::syncTimer(int timer, void *context) {
	SmushAnimPlay *anim = (SmushAnimPlay *)context;

	if (!anim->_started)
		return;

	if (anim->_flags & 1) {
		if (!anim->_syncPaused && anim->_lateTicks >= 7) {
			anim->_syncPaused = 1;
			anim->_sound->souEnginePause();
		} else if (anim->_syncPaused) {
			if (anim->_lateTicks < 0) {
				anim->_syncPaused = 0;
				anim->_sound->souEngineResume();
			}
		}
	} else if (anim->_syncPaused) {
		if (anim->_lateTicks < 0) {
			anim->_syncPaused = 0;
			anim->_sound->souEngineResume();
		}
	}

	if (anim->_lateTicks < 20 && !anim->_syncPaused)
		anim->_lateTicks++;
}

} // End of namespace Grim
