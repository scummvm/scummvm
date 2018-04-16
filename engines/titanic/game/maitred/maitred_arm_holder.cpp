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

#include "titanic/game/maitred/maitred_arm_holder.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CMaitreDArmHolder, CDropTarget)
	ON_MESSAGE(MaitreDArmHolder)
	ON_MESSAGE(ActMsg)
END_MESSAGE_MAP()

void CMaitreDArmHolder::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CDropTarget::save(file, indent);
}

void CMaitreDArmHolder::load(SimpleFile *file) {
	file->readNumber();
	CDropTarget::load(file);
}

bool CMaitreDArmHolder::MaitreDArmHolder(CMaitreDArmHolder *msg) {
	_dropEnabled = false;
	return true;
}

bool CMaitreDArmHolder::ActMsg(CActMsg *msg) {
	if (msg->_action == "LoseArm") {
		_bounds = Rect();
		setVisible(false);
	}

	return true;
}

} // End of namespace Titanic
