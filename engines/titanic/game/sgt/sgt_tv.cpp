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

#include "titanic/game/sgt/sgt_tv.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CSGTTV, CSGTStateRoom)
	ON_MESSAGE(TurnOff)
	ON_MESSAGE(TurnOn)
	ON_MESSAGE(MovieEndMsg)
END_MESSAGE_MAP()

void CSGTTV::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CSGTStateRoom::save(file, indent);
}

void CSGTTV::load(SimpleFile *file) {
	file->readNumber();
	CSGTStateRoom::load(file);
}

bool CSGTTV::TurnOff(CTurnOff *msg) {
	if (CSGTStateRoom::_statics->_tv == "Open") {
		CSGTStateRoom::_statics->_tv = "Closed";
		_isClosed = true;
		_startFrame = 6;
		_endFrame = 12;
		playMovie(6, 12, MOVIE_NOTIFY_OBJECT | MOVIE_GAMESTATE);
	}

	return true;
}

bool CSGTTV::TurnOn(CTurnOn *msg) {
	if (CSGTStateRoom::_statics->_tv == "Closed" &&
			CSGTStateRoom::_statics->_bedfoot != "Closed") {
		CSGTStateRoom::_statics->_tv = "Open";
		setVisible(true);
		_isClosed = false;
		_startFrame = 1;
		_endFrame = 6;
		playMovie(1, 6, MOVIE_GAMESTATE);
	}

	return true;
}

bool CSGTTV::MovieEndMsg(CMovieEndMsg *msg) {
	setVisible(false);
	return true;
}

} // End of namespace Titanic
