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

#include "titanic/game/gondolier/gondolier_chest.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CGondolierChest, CGondolierBase)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(MouseDragStartMsg)
END_MESSAGE_MAP()

void CGondolierChest::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CGondolierBase::save(file, indent);
}

void CGondolierChest::load(SimpleFile *file) {
	file->readNumber();
	CGondolierBase::load(file);
}

bool CGondolierChest::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (!_chestOpen)
		playMovie(0, 14, MOVIE_NOTIFY_OBJECT);
	else if (msg->_mousePos.y < 330)
		return false;
	else if (!_leftSliderHooked && !_rightSliderHooked) {
		playMovie(14, 29, 0);
		_chestOpen = false;
	}

	return true;
}

bool CGondolierChest::MovieEndMsg(CMovieEndMsg *msg) {
	if (msg->_endFrame == 14)
		_chestOpen = true;
	return true;
}

bool CGondolierChest::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	return false;
}

} // End of namespace Titanic
