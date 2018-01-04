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
#include "titanic/translation.h"

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
	if (CSGTStateRoom::_statics->_toilet == "Closed"
			&& CSGTStateRoom::_statics->_washstand == "Open"
			&& CSGTStateRoom::_statics->_armchair == "Closed") {
		setVisible(true);
		CSGTStateRoom::_statics->_toilet = "Open";

		_isClosed = false;
		_startFrame = 0;
		_endFrame = 11;
		playMovie(0, 11, MOVIE_WAIT_FOR_FINISH);
		playSound(TRANSLATE("b#1.wav", "b#86.wav"));
	}

	return true;
}

bool CToilet::TurnOff(CTurnOff *msg) {
	if (CSGTStateRoom::_statics->_toilet == "Open") {
		CSGTStateRoom::_statics->_toilet = "Closed";

		_isClosed = true;
		_startFrame = 11;
		_endFrame = 18;
		playMovie(11, 18, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
		playSound(TRANSLATE("b#1.wav", "b#86.wav"));
	}

	return true;
}

bool CToilet::MovieEndMsg(CMovieEndMsg *msg) {
	if (CSGTStateRoom::_statics->_toilet == "Closed")
		setVisible(false);

	return true;
}

} // End of namespace Titanic
