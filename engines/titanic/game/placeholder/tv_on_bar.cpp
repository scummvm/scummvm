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

#include "titanic/game/placeholder/tv_on_bar.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CTVOnBar, CPlaceHolder)
	ON_MESSAGE(VisibleMsg)
END_MESSAGE_MAP()

void CTVOnBar::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writePoint(_tvPos, indent);
	CPlaceHolder::save(file, indent);
}

void CTVOnBar::load(SimpleFile *file) {
	file->readNumber();
	_tvPos = file->readPoint();
	CPlaceHolder::load(file);
}

bool CTVOnBar::VisibleMsg(CVisibleMsg *msg) {
	setVisible(msg->_visible);
	if (msg->_visible)
		setPosition(_tvPos);
	else
		setPosition(Point(0, 0));

	return true;
}

} // End of namespace Titanic
