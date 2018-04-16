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

#include "titanic/game/sgt/armchair.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CArmchair, CSGTStateRoom)
	ON_MESSAGE(TurnOn)
	ON_MESSAGE(TurnOff)
	ON_MESSAGE(MovieEndMsg)
END_MESSAGE_MAP()

void CArmchair::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CSGTStateRoom::save(file, indent);
}

void CArmchair::load(SimpleFile *file) {
	file->readNumber();
	CSGTStateRoom::load(file);
}

bool CArmchair::TurnOn(CTurnOn *msg) {
	if (_statics->_armchair == "Closed" && _statics->_toilet == "Closed") {
		CVisibleMsg visibleMsg(false);
		visibleMsg.execute("Deskchair");

		if (_statics->_deskchair == "Open") {
			CActMsg actMsg("Squash");
			actMsg.execute("Deskchair");
			_startFrame = 22;
			_endFrame = 31;
		} else {
			_startFrame = 0;
			_endFrame = 10;
		}

		playMovie(_startFrame, _endFrame, MOVIE_WAIT_FOR_FINISH);
		playSound(TRANSLATE("b#0.wav", "b#85.wav"));
		_statics->_armchair = "Open";
		_isClosed = false;
	}

	return true;
}

bool CArmchair::TurnOff(CTurnOff *msg) {
	if (_statics->_armchair == "Open") {
		_statics->_armchair = "Closed";
		_startFrame = 11;
		_endFrame = 21;
		_isClosed = true;
		playMovie(11, 21, MOVIE_WAIT_FOR_FINISH | MOVIE_NOTIFY_OBJECT);
		playSound(TRANSLATE("b#0.wav", "b#85.wav"));
	}

	return true;
}

bool CArmchair::MovieEndMsg(CMovieEndMsg *msg) {
	if (_statics->_armchair == "Closed")
		loadFrame(0);

	return true;
}

} // End of namespace Titanic
