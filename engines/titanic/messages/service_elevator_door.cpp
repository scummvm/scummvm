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

#include "titanic/messages/service_elevator_door.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CServiceElevatorDoor, CDoorAutoSoundEvent)
	ON_MESSAGE(PreEnterNodeMsg)
END_MESSAGE_MAP()

CServiceElevatorDoor::CServiceElevatorDoor() : CDoorAutoSoundEvent() {
	_string1 = "z#31.wav";
	_string2 = "z#32.wav";
}

void CServiceElevatorDoor::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_string2, indent);
	file->writeQuotedLine(_string1, indent);

	CDoorAutoSoundEvent::save(file, indent);
}

void CServiceElevatorDoor::load(SimpleFile *file) {
	file->readNumber();
	_string2 = file->readString();
	_string1 = file->readString();

	CDoorAutoSoundEvent::load(file);
}

bool CServiceElevatorDoor::PreEnterNodeMsg(CPreEnterNodeMsg *msg) {
	if (!findRoom()->isEquals("BilgeRoomWith"))
		CDoorAutoSoundEvent::PreEnterNodeMsg(msg);
	return true;
}

} // End of namespace Titanic
