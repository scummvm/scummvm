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

#include "bolt/bolt.h"

namespace Bolt {

bool BoltEngine::startAnimation(RTFResource *rtf, int16 animIndex) {
	if (!initAnim(rtf, animIndex)) {
		cleanUpAnim();
		debug("BoltEngine::startAnimation(): Failed to start animation %d", animIndex);
		return false;
	}

	return true;
}

void BoltEngine::stopAnimation() {
	killRTF(nullptr);
	cleanUpAnim();
}

bool BoltEngine::maintainAudioPlay(int16 mode) {
	RTFPacket *frameData;

	if (!maintainRTF(mode, &frameData)) {
		cleanUpAnim();
		return false;
	}

	// Check for "TRGR" trigger marker in frame data
	if (frameData && frameData->tag == MKTAG('T', 'R', 'G', 'R')) {
		_xp->postEvent(etTrigger, 0);
	}

	return true;
}

bool BoltEngine::initAnim(RTFResource *rtf, int16 animIndex) {
	int32 bufSize;

	_animPrevInactivityTimer = _xp->setInactivityTimer(0);

	// Try 80KB ring buffer, fall back to 40KB
	bufSize = 0x14000;
	_animRingBuffer = (byte *)_xp->tryAllocMem(bufSize);
	if (!_animRingBuffer) {
		bufSize = 0xA000;
		_animRingBuffer = (byte *)_xp->allocMem(bufSize);
	}

	if (!_animRingBuffer)
		return false;

	if (!playRTF(rtf, animIndex, _animRingBuffer, bufSize))
		return false;

	return true;
}

void BoltEngine::cleanUpAnim() {
	if (_animRingBuffer) {
		_xp->freeMem(_animRingBuffer);
		_animRingBuffer = nullptr;
	}

	if (_animFileHandle) {
		_xp->closeFile(_animFileHandle);
		_animFileHandle = nullptr;
	}

	_xp->setInactivityTimer(_animPrevInactivityTimer);
}

} // End of namespace Bolt
