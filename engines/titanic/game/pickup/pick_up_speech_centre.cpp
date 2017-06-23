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

#include "titanic/game/pickup/pick_up_speech_centre.h"
#include "titanic/core/project_item.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CPickUpSpeechCentre, CPickUp)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(StatusChangeMsg)
	ON_MESSAGE(MouseDragStartMsg)
END_MESSAGE_MAP()

void CPickUpSpeechCentre::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CPickUp::save(file, indent);
}

void CPickUpSpeechCentre::load(SimpleFile *file) {
	file->readNumber();
	CPickUp::load(file);
}

bool CPickUpSpeechCentre::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	return true;
}

bool CPickUpSpeechCentre::StatusChangeMsg(CStatusChangeMsg *msg) {
	_enabled = msg->_newStatus == 1;
	return true;
}

bool CPickUpSpeechCentre::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	if (checkStartDragging(msg)) {
		if (_enabled) {
			CVisibleMsg visibleMsg;
			visibleMsg.execute("SpeechCentre");
			CPassOnDragStartMsg passMsg(msg->_mousePos, 1);
			passMsg.execute("SpeechCentre");

			msg->_dragItem = getRoot()->findByName("SpeechCentre");

			CActMsg actMsg("PlayerGetsSpeechCentre");
			actMsg.execute("SeasonalAdjust");
		} else {
			petDisplayMessage(STUCK_TO_BRANCH);
		}
	}

	return true;
}

} // End of namespace Titanic
