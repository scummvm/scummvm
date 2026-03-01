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

	g_animPrevInactivityTimer = _xp->setInactivityTimer(0);

	// Try 80KB ring buffer, fall back to 40KB
	bufSize = 0x14000;
	g_animRingBuffer = (byte *)_xp->tryAllocMem(bufSize);
	if (!g_animRingBuffer) {
		bufSize = 0xA000;
		g_animRingBuffer = (byte *)_xp->allocMem(bufSize);
	}

	if (!g_animRingBuffer)
		return false;

	if (!playRTF(rtf, animIndex, g_animRingBuffer, bufSize))
		return false;

	return true;
}

void BoltEngine::cleanUpAnim() {
	if (g_animRingBuffer) {
		_xp->freeMem(g_animRingBuffer);
		g_animRingBuffer = nullptr;
	}

	if (g_animFileHandle) {
		_xp->closeFile(g_animFileHandle);
		g_animFileHandle = nullptr;
	}

	_xp->setInactivityTimer(g_animPrevInactivityTimer);
}

} // End of namespace Bolt
