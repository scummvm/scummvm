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

#include "titanic/game/hammer_dispensor.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CHammerDispensor, CBackground)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(LeaveViewMsg)
	ON_MESSAGE(MovieEndMsg)
END_MESSAGE_MAP()

CHammerDispensor::CHammerDispensor() : CBackground(),
	_fieldE0(false), _fieldE4(true), _state(0) {
}

void CHammerDispensor::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_fieldE0, indent);
	file->writeNumberLine(_fieldE4, indent);
	file->writeNumberLine(_state, indent);

	CBackground::save(file, indent);
}

void CHammerDispensor::load(SimpleFile *file) {
	file->readNumber();
	_fieldE0 = file->readNumber();
	_fieldE4 = file->readNumber();
	_state = file->readNumber();

	CBackground::load(file);
}

bool CHammerDispensor::ActMsg(CActMsg *msg) {
	if (msg->_action == "DispenseHammer" && !_fieldE0) {
		_state = 1;
		playMovie(15, 31, MOVIE_NOTIFY_OBJECT);
		_fieldE0 = true;
	}

	if (msg->_action == "HammerTaken" && _fieldE0)
		loadFrame(32);

	return true;
}

bool CHammerDispensor::EnterViewMsg(CEnterViewMsg *msg) {
	if (_fieldE4) {
		playMovie(7, 14, 0);
		_fieldE4 = false;
	}

	return true;
}

bool CHammerDispensor::LeaveViewMsg(CLeaveViewMsg *msg) {
	_fieldE4 = true;
	_fieldE0 = 0;
	_state = 2;

	if (_fieldE0)
		playMovie(32, 50, MOVIE_NOTIFY_OBJECT | MOVIE_GAMESTATE);
	else
		playMovie(0, 7, MOVIE_NOTIFY_OBJECT | MOVIE_GAMESTATE);
	return true;
}

bool CHammerDispensor::MovieEndMsg(CMovieEndMsg *msg) {
	if (_state == 1) {
		CStatusChangeMsg statusMsg;
		statusMsg._newStatus = 1;
		statusMsg.execute("HammerClip");
	} else if (_state == 2) {
		CStatusChangeMsg statusMsg;
		statusMsg._newStatus = 2;
		statusMsg.execute("HammerClip");
	}

	return true;
}

} // End of namespace Titanic
