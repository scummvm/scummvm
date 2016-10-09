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
	_fieldE0(0), _fieldE4(0), _fieldE8(0), _fieldEC(0) {
}

void CServiceElevatorWindow::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_fieldE0, indent);
	file->writeNumberLine(_fieldE4, indent);
	file->writeNumberLine(_fieldE8, indent);
	file->writeNumberLine(_fieldEC, indent);

	CBackground::save(file, indent);
}

void CServiceElevatorWindow::load(SimpleFile *file) {
	file->readNumber();
	_fieldE0 = file->readNumber();
	_fieldE4 = file->readNumber();
	_fieldE8 = file->readNumber();
	_fieldEC = file->readNumber();

	CBackground::load(file);
}

bool CServiceElevatorWindow::ServiceElevatorFloorChangeMsg(CServiceElevatorFloorChangeMsg *msg) {
	if (getView() == findView()) {
		CDoorbot *doorbot = dynamic_cast<CDoorbot *>(findRoom()->findByName("Doorbot"));
		int val = (_fieldE8 && doorbot) ? 65 : 15;
		CMovieClip *clip = _movieClips.findByName("Going Up");

		if (!clip)
			return true;

		int count = _endFrame - _startFrame;
		setMovieFrameRate(1.0 * count / val);

		int startFrame = clip->_startFrame + count * FACTORS[msg->_value1] / 100;
		int endFrame = clip->_startFrame + count * FACTORS[msg->_value2] / 100;

		if (_fieldE4) {
			playMovie(startFrame, endFrame, MOVIE_NOTIFY_OBJECT);
		} else {
			playMovie(startFrame, endFrame, 0);
			if (_fieldEC)
				playClip("Into Space");
		}
	}

	_fieldE0 = msg->_value2;
	return true;
}

bool CServiceElevatorWindow::MovieEndMsg(CMovieEndMsg *msg) {
	CServiceElevatorMsg elevMsg(5);
	elevMsg.execute(findRoom()->findByName("Service Elevator Entity"));
	return true;
}

bool CServiceElevatorWindow::EnterViewMsg(CEnterViewMsg *msg) {
	if (_fieldEC) {
		playClip("Fade Up");
		playMovie(1, 2, 0);
	} else {
		CMovieClip *clip = _movieClips.findByName("Going Up");

		if (clip) {
			int frameNum = clip->_startFrame + (clip->_endFrame - clip->_startFrame)
				* FACTORS[_fieldE0] / 100;
			loadFrame(frameNum);
		} else {
			loadFrame(0);
		}
	}

	return true;
}

} // End of namespace Titanic
