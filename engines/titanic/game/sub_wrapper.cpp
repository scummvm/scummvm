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

#include "titanic/game/sub_wrapper.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CSUBWrapper, CGameObject)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(SignalObject)
END_MESSAGE_MAP()

void CSUBWrapper::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_flag, indent);
	CGameObject::save(file, indent);
}

void CSUBWrapper::load(SimpleFile *file) {
	file->readNumber();
	_flag = file->readNumber();
	CGameObject::load(file);
}

bool CSUBWrapper::MovieEndMsg(CMovieEndMsg *msg) {
	if (_flag) {
		stopMovie();
		setVisible(false);
		_flag = false;
	}

	return true;
}

bool CSUBWrapper::SignalObject(CSignalObject *msg) {
	switch (msg->_numValue) {
	case 1:
		if (!_flag) {
			loadFrame(0);
			setVisible(true);
			playMovie(MOVIE_NOTIFY_OBJECT);
			_flag = true;
		}
		break;

	case 2:
		if (!_flag) {
			setVisible(true);
			_flag = true;
		}
		break;

	default:
		break;
	}

	return true;
}

} // End of namespace Titanic
