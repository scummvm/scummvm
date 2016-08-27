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

#include "titanic/carry/maitred_left_arm.h"
#include "titanic/npcs/true_talk_npc.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CMaitreDLeftArm, CArm)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(MouseDragStartMsg)
END_MESSAGE_MAP()

void CMaitreDLeftArm::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_flag, indent);
	CArm::save(file, indent);
}

void CMaitreDLeftArm::load(SimpleFile *file) {
	file->readNumber();
	_flag = file->readNumber();
	CArm::load(file);
}

bool CMaitreDLeftArm::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (!_flag) {
		CTrueTalkNPC *maitreD = dynamic_cast<CTrueTalkNPC *>(findRoomObject("MaitreD"));
		startTalking(maitreD, 126);
		startTalking(maitreD, 127);
	}

	return true;
}

bool CMaitreDLeftArm::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	if (checkPoint(msg->_mousePos) && !_flag) {
		CVisibleMsg visibleMsg;
		visibleMsg.execute("MD left arm background image");
		_flag = true;

		CArmPickedUpFromTableMsg takenMsg;
		takenMsg.execute("Restaurant Table Pan Handler", nullptr, MSGFLAG_SCAN);
	}

	return CArm::MouseDragStartMsg(msg);
}

} // End of namespace Titanic
