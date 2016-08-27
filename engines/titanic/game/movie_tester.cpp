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

#include "titanic/game/movie_tester.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CMovieTester, CGameObject)
	ON_MESSAGE(MouseButtonDownMsg)
END_MESSAGE_MAP()

void CMovieTester::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_movieNumFrames, indent);
	file->writeNumberLine(_movieFrameNum, indent);
	CGameObject::save(file, indent);
}

void CMovieTester::load(SimpleFile *file) {
	file->readNumber();
	_movieNumFrames = file->readNumber();
	_movieFrameNum = file->readNumber();
	CGameObject::load(file);
}

bool CMovieTester::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (msg->_buttons == MB_RIGHT) {
		if (--_movieFrameNum < 0) {
			_movieFrameNum = _movieNumFrames - 1;
		}
	} else {
		if (++_movieFrameNum >= _movieNumFrames)
			_movieFrameNum = 0;
	}

	loadFrame(_movieFrameNum);
	return true;
}

} // End of namespace Titanic
