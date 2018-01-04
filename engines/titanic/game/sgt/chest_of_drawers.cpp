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

#include "titanic/game/sgt/chest_of_drawers.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CChestOfDrawers, CSGTStateRoom)
	ON_MESSAGE(TurnOn)
	ON_MESSAGE(TurnOff)
	ON_MESSAGE(MovieEndMsg)
END_MESSAGE_MAP()

void CChestOfDrawers::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CSGTStateRoom::save(file, indent);
}

void CChestOfDrawers::load(SimpleFile *file) {
	file->readNumber();
	CSGTStateRoom::load(file);
}

bool CChestOfDrawers::TurnOn(CTurnOn *msg) {
	if (_statics->_chestOfDrawers == "Closed" && _statics->_desk == "Open") {
		_isClosed = false;
		_statics->_chestOfDrawers = "Open";
		_startFrame = 1;
		_endFrame = 14;
		playMovie(1, 14, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
		playSound(TRANSLATE("b#11.wav", "b#96.wav"));
	}

	return true;
}

bool CChestOfDrawers::TurnOff(CTurnOff *msg) {
	if (_statics->_chestOfDrawers == "Open" && _statics->_drawer == "Closed") {
		CVisibleMsg visibleMsg(false);
		visibleMsg.execute("Drawer");
		_statics->_chestOfDrawers = "Closed";
		_isClosed = true;

		_startFrame = 14;
		_endFrame = 27;
		playMovie(14, 27, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
		playSound(TRANSLATE("b#11.wav", "b#96.wav"));
	}

	return true;
}

bool CChestOfDrawers::MovieEndMsg(CMovieEndMsg *msg) {
	if (_statics->_chestOfDrawers == "Open") {
		CVisibleMsg visibleMsg;
		visibleMsg.execute("Drawer");
	}

	return true;
}

} // End of namespace Titanic
