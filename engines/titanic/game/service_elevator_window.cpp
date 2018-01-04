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

#include "titanic/game/service_elevator_window.h"
#include "titanic/core/room_item.h"
#include "titanic/npcs/doorbot.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CServiceElevatorWindow, CBackground)
	ON_MESSAGE(ServiceElevatorFloorChangeMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(EnterViewMsg)
END_MESSAGE_MAP()

static const int FACTORS[4] = { 0, 20, 100, 0 };

CServiceElevatorWindow::CServiceElevatorWindow() : CBackground(),
	_destFloor(0), _notifyFlag(false), _isIndicator(false), _intoSpace(false) {
}

void CServiceElevatorWindow::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_destFloor, indent);
	file->writeNumberLine(_notifyFlag, indent);
	file->writeNumberLine(_isIndicator, indent);
	file->writeNumberLine(_intoSpace, indent);

	CBackground::save(file, indent);
}

void CServiceElevatorWindow::load(SimpleFile *file) {
	file->readNumber();
	_destFloor = file->readNumber();
	_notifyFlag = file->readNumber();
	_isIndicator = file->readNumber();
	_intoSpace = file->readNumber();

	CBackground::load(file);
}

bool CServiceElevatorWindow::ServiceElevatorFloorChangeMsg(CServiceElevatorFloorChangeMsg *msg) {
	if (getView() == findView()) {
		CDoorbot *doorbot = dynamic_cast<CDoorbot *>(findRoom()->findByName("Doorbot"));
		int fps = (_isIndicator && doorbot) ? 65 : 15;
		CMovieClip *clip = _movieClips.findByName("Going Up");

		if (!clip)
			return true;

		int count = clip->_endFrame - clip->_startFrame;
		int startFrame = clip->_startFrame + count * FACTORS[msg->_startFloor] / 100;
		int endFrame = clip->_startFrame + count * FACTORS[msg->_endFloor] / 100;

		double rate = (startFrame < endFrame ? 1.0 : -1.0) * count / fps;
		setMovieFrameRate(rate);

		if (_notifyFlag) {
			// Service elevator indicator
			playMovie(startFrame, endFrame, MOVIE_NOTIFY_OBJECT);
		} else {
			// Background outside elevator
			playMovie(startFrame, endFrame, 0);

			if (_intoSpace)
				playClip("Into Space");
		}
	}

	_destFloor = msg->_endFloor;
	return true;
}

bool CServiceElevatorWindow::MovieEndMsg(CMovieEndMsg *msg) {
	// Called when indicator reaches desired destination floor
	CServiceElevatorMsg elevMsg(5);
	elevMsg.execute(findRoom()->findByName("Service Elevator Entity"));
	return true;
}

bool CServiceElevatorWindow::EnterViewMsg(CEnterViewMsg *msg) {
	if (_intoSpace) {
		playClip("Fade Up");
		playMovie(1, 2, 0);
	} else {
		CMovieClip *clip = _movieClips.findByName("Going Up");

		if (clip) {
			int frameNum = clip->_startFrame + (clip->_endFrame - clip->_startFrame)
				* FACTORS[_destFloor] / 100;
			loadFrame(frameNum);
		} else {
			loadFrame(0);
		}
	}

	return true;
}

} // End of namespace Titanic
