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

#include "bagel/spacebar/baglib/exam.h"
#include "bagel/spacebar/dialogs/opt_window.h"
#include "bagel/spacebar/baglib/bagel.h"
#include "bagel/spacebar/baglib/master_win.h"

namespace Bagel {
namespace SpaceBar {

bool CBagExam::initExam() {
	CBofRect paintRect(155, 55, 155 + 330 - 1, 55 + 250 - 1);
	paintBitmap(CBagel::getBagApp()->getMasterWnd()->getCurrentGameWindow(), buildSysDir("SSBORDER.BMP"), &paintRect);

	Common::Event e;
	while (g_system->getEventManager()->pollEvent(e)) {
		// Flush events, though not sure why we need it
	}

	getParent()->disable();
	getParent()->flushAllMessages();

	markBegEnd();
	setRotationRects();
	_bLoop = true;
	_bEscCanStop = true;

	pause();

	CBofCursor::show();
	doModal();
	CBofCursor::hide();

	return true;
}

void CBagExam::onReSize(CBofSize *size) {

	CBofMovie::onReSize(size);
	setRotationRects();

}

bool CBagExam::setRotationRects() {
	// Get the  windows rect
	const CBofRect clientRect = getClientRect();

	// Left quarter of the video window
	_leftRect.left = clientRect.left;
	_leftRect.top = clientRect.top;
	_leftRect.right = clientRect.left + ((clientRect.right - clientRect.left) / 4);
	_leftRect.bottom = clientRect.bottom;

	// Right quarter of the video window
	_rightRect.left = clientRect.right - ((clientRect.bottom - clientRect.left) / 4);
	_rightRect.top = clientRect.top;
	_rightRect.right = clientRect.right;
	_rightRect.bottom = clientRect.bottom;

	return true;
}

bool CBagExam::markBegEnd() {
	_end = _pSmk->getFrameCount() - 1;
	_start = 0;

	return true;
}

void  CBagExam::onButtonUp(uint32 /*n flags, unused */, CBofPoint */* point, unused */) {
	// Clean up and exit
	_bLoop = false;

	stop();
	onMovieDone();
}


bool CBagExam::rotateLeft() {
	if (_eMovStatus != FORWARD)
		return play();

	return true;
}

bool CBagExam::rotateRight() {
	if (_eMovStatus != REVERSE)
		return reverse();

	return true;
}

bool CBagExam::rotateStop() {
	if (_eMovStatus == FORWARD || _eMovStatus == REVERSE)
		// The movie is currently playing
		return pause();

	return true;

}

void CBagExam::onMouseMove(uint32 /* flags, unused */, CBofPoint *point, void */* extraInfo, unused */) {
	assert(isValidObject(this));
	assert(point != nullptr);

	// No more cursor in Examine movies
	if (point->x <= _leftRect.right) {
		// Left rect, play reverse
		rotateLeft();
	} else if (point->x >= _rightRect.left) {
		// Right rect, play forward
		rotateRight();
	} else if (_eMovStatus == FORWARD || _eMovStatus == REVERSE) {
		rotateStop();
	}
}

} // namespace SpaceBar
} // namespace Bagel
