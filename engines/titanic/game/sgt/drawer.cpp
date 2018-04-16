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

#include "titanic/game/sgt/drawer.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CDrawer, CSGTStateRoom)
	ON_MESSAGE(TurnOn)
	ON_MESSAGE(TurnOff)
	ON_MESSAGE(MovieEndMsg)
END_MESSAGE_MAP()

CDrawer::CDrawer() : CSGTStateRoom(), _fieldF4(0) {
}

void CDrawer::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_fieldF4, indent);
	CSGTStateRoom::save(file, indent);
}

void CDrawer::load(SimpleFile *file) {
	file->readNumber();
	_fieldF4 = file->readNumber();
	CSGTStateRoom::load(file);
}

bool CDrawer::TurnOn(CTurnOn *msg) {
	if (_statics->_drawer == "Closed" && _statics->_chestOfDrawers == "Open") {
		_statics->_drawer = "Open";
		_isClosed = false;
		_startFrame = 50;
		_endFrame = 75;
		setVisible(true);
		_statics->_drawer = "Open";
		playMovie(_startFrame, _endFrame, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
		playSound(TRANSLATE("b#10.wav", "b#95.wav"));
	}

	return true;
}

bool CDrawer::TurnOff(CTurnOff *msg) {
	if (_statics->_drawer == "Open") {
		_statics->_drawer = "Closed";
		_startFrame = 75;
		_endFrame = 100;
		_isClosed = true;
		playMovie(_startFrame, _endFrame, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
		playSound(TRANSLATE("b#10.wav", "b#95.wav"));
	}

	return true;
}

bool CDrawer::MovieEndMsg(CMovieEndMsg *msg) {
	if (_statics->_drawer == "Closed")
		setVisible(false);

	return true;
}

} // End of namespace Titanic
