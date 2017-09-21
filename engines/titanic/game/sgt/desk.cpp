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

#include "titanic/game/sgt/desk.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CDesk, CSGTStateRoom)
	ON_MESSAGE(TurnOn)
	ON_MESSAGE(TurnOff)
	ON_MESSAGE(MovieEndMsg)
END_MESSAGE_MAP()

void CDesk::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CSGTStateRoom::save(file, indent);
}

void CDesk::load(SimpleFile *file) {
	file->readNumber();
	CSGTStateRoom::load(file);
}

bool CDesk::TurnOn(CTurnOn *msg) {
	if (_statics->_desk == "Closed" && _statics->_bedhead != "RestingG"
			&& _statics->_bedhead != "OpenWrong") {
		_statics->_desk = "Open";
		_isClosed = false;
		_startFrame = 1;
		_endFrame = 26;
		playMovie(1, 26, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
		playSound(TRANSLATE("b#12.wav", "b#97.wav"));
	}

	return true;
}

bool CDesk::TurnOff(CTurnOff *msg) {
	if (_statics->_desk == "Open" && _statics->_chestOfDrawers == "Closed"
			&& _statics->_bedhead != "Open") {
		CVisibleMsg visibleMsg(false);
		visibleMsg.execute("ChestOfDrawers");

		_statics->_desk = "Closed";
		_isClosed = true;
		_startFrame = 26;
		_endFrame = 51;
		playMovie(26, 51, MOVIE_WAIT_FOR_FINISH);
		playSound(TRANSLATE("b#9.wav", "b#94.wav"));
	}

	return true;
}

bool CDesk::MovieEndMsg(CMovieEndMsg *msg) {
	if (_statics->_desk == "Open") {
		CVisibleMsg visibleMsg(true);
		visibleMsg.execute("ChestOfDrawers");
	}

	return true;
}

} // End of namespace Titanic
