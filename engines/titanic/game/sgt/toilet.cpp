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

#include "titanic/game/sgt/toilet.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CToilet, CSGTStateRoom)
	ON_MESSAGE(TurnOn)
	ON_MESSAGE(TurnOff)
	ON_MESSAGE(MovieEndMsg)
END_MESSAGE_MAP()

void CToilet::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CSGTStateRoom::save(file, indent);
}

void CToilet::load(SimpleFile *file) {
	file->readNumber();
	CSGTStateRoom::load(file);
}

bool CToilet::TurnOn(CTurnOn *msg) {
	if (CSGTStateRoom::_statics->_v12 == "Closed"
			&& CSGTStateRoom::_statics->_v10 == "Open"
			&& CSGTStateRoom::_statics->_v8 == "Closed") {
		setVisible(true);
		CSGTStateRoom::_statics->_v12 = "Open";

		_isClosed = false;
		_startFrame = 0;
		_endFrame = 11;
		playMovie(0, 11, MOVIE_GAMESTATE);
		playSound("b#1.wav");
	}

	return true;
}

bool CToilet::TurnOff(CTurnOff *msg) {
	if (CSGTStateRoom::_statics->_v12 == "Open") {
		CSGTStateRoom::_statics->_v12 = "Closed";

		_isClosed = true;
		_startFrame = 11;
		_endFrame = 18;
		playMovie(11, 18, MOVIE_GAMESTATE);
		playSound("b#1.wav");
	}

	return true;
}

bool CToilet::MovieEndMsg(CMovieEndMsg *msg) {
	if (CSGTStateRoom::_statics->_v12 == "Closed")
		setVisible(false);

	return true;
}

} // End of namespace Titanic
