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

#include "titanic/game/sgt/washstand.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CWashstand, CSGTStateRoom)
	ON_MESSAGE(TurnOn)
	ON_MESSAGE(TurnOff)
	ON_MESSAGE(MovieEndMsg)
END_MESSAGE_MAP()

void CWashstand::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CSGTStateRoom::save(file, indent);
}

void CWashstand::load(SimpleFile *file) {
	file->readNumber();
	CSGTStateRoom::load(file);
}

bool CWashstand::TurnOn(CTurnOn *msg) {
	if (_statics->_washstand == "Closed" && _statics->_bedfoot != "NotOnWashstand") {
		setVisible(true);
		_statics->_washstand = "Open";
		_isClosed = false;
		_startFrame = 0;
		_endFrame = 14;
		playMovie(0, 14, MOVIE_WAIT_FOR_FINISH);
		playSound(TRANSLATE("b#14.wav", "b#99.wav"));
	}

	return true;
}

bool CWashstand::TurnOff(CTurnOff *msg) {
	if (_statics->_washstand == "Open" && _statics->_basin == "Closed"
			&& _statics->_toilet == "Closed" && _statics->_bedfoot != "Open") {
		_statics->_washstand = "Closed";
		_isClosed = true;
		_startFrame = 14;
		_endFrame = 28;
		playMovie(14, 28, MOVIE_WAIT_FOR_FINISH | MOVIE_NOTIFY_OBJECT);
		playSound(TRANSLATE("b#14.wav", "b#99.wav"));
	}

	return true;
}

bool CWashstand::MovieEndMsg(CMovieEndMsg *msg) {
	return true;
}

} // End of namespace Titanic
