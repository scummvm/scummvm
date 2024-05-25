
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

#include "common/system.h"
#include "bagel/boflib/timer.h"
#include "bagel/bagel.h"
#include "log.h"

namespace Bagel {

bool CBofTimer::_bModified = false;
CBofTimer *CBofTimer::_pTimerList = nullptr;

WindowTimer::WindowTimer(uint32 interval, uint32 id, BofCallback callback) :
		_interval(interval), _id(id), _callback(callback) {
	_lastExpiryTime = g_system->getMillis();
}


CBofTimer::CBofTimer() {
	_lLastTime = 0;
	_nID = 0;
	_nInterval = 0;
	_pCallBack = nullptr;
	_lUserInfo = 0;
	_bActive = false;

	// Another item for the list
	if (_pTimerList == nullptr) {
		_pTimerList = this;

	} else {
		_pTimerList->addToTail(this);
	}

	// Creating a new timer object modifies the timer list
	_bModified = true;
}


CBofTimer::CBofTimer(uint32 nID, uint32 nInterval, void *lUserInfo, BofCallback pCallBack) {
	_lLastTime = 0;
	_nID = nID;
	_nInterval = nInterval;
	_pCallBack = pCallBack;
	_lUserInfo = lUserInfo;
	_bActive = false;

	// Another item for the list
	if (_pTimerList == nullptr) {
		_pTimerList = this;

	} else {
		_pTimerList->addToTail(this);
	}

	// Creating a new timer object modifies the timer list
	_bModified = true;
}


CBofTimer::~CBofTimer() {
	assert(isValidObject(this));

	if (_pTimerList == this) {
		_pTimerList = (CBofTimer *)_pTimerList->getNext();
	}

	// Removing a timer object modifies the timer list
	_bModified = true;
}


void CBofTimer::handleTimers() {
	CBofTimer *pTimer = _pTimerList;
	while (pTimer != nullptr) {
		if (pTimer->isActive()) {
			uint32 lCurrentTime = g_system->getMillis();

			if ((uint32)(lCurrentTime - pTimer->_lLastTime) >= pTimer->_nInterval) {
				// Remember for next time
				pTimer->_lLastTime = lCurrentTime;

				if (pTimer->_pCallBack != nullptr) {
					// Execute call back
					(*pTimer->_pCallBack)(pTimer->_nID, pTimer->_lUserInfo);

					// If callback modifies the timer list, then we must start over
					if (_bModified) {
						pTimer = _pTimerList;
						continue;
					}

				} else {
					// Otherwise, something is wrong
					logWarning(buildString("Timer without a callback: %d", pTimer->_nID));
				}
			}
		}

		pTimer = (CBofTimer *)pTimer->getNext();
	}
}

} // end of namespace Bagel
