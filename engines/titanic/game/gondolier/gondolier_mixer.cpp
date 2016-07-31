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

#include "titanic/game/gondolier/gondolier_mixer.h"
#include "titanic/core/room_item.h"

namespace Titanic {

CGondolierMixer::CGondolierMixer() : CGondolierBase(),
	_string1("c#0.wav"), _string2("c#1.wav"),
	_fieldBC(-1), _fieldC0(-1), _fieldC4(0), _fieldC8(0),
	_fieldE4(0) {
}

void CGondolierMixer::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_fieldBC, indent);
	file->writeNumberLine(_fieldC0, indent);
	file->writeNumberLine(_fieldC4, indent);
	file->writeNumberLine(_fieldC8, indent);
	file->writeQuotedLine(_string1, indent);
	file->writeQuotedLine(_string2, indent);
	file->writeNumberLine(_fieldE4, indent);

	CGondolierBase::save(file, indent);
}

void CGondolierMixer::load(SimpleFile *file) {
	file->readNumber();
	_fieldBC = file->readNumber();
	_fieldC0 = file->readNumber();
	_fieldC4 = file->readNumber();
	_fieldC8 = file->readNumber();
	_string1 = file->readString();
	_string2 = file->readString();
	_fieldE4 = file->readNumber();

	CGondolierBase::load(file);
}

bool CGondolierMixer::EnterRoomMsg(CEnterRoomMsg *msg) {
	CRoomItem *parentRoom = dynamic_cast<CRoomItem *>(getParent());
	if (parentRoom == msg->_newRoom)
		msg->execute(parentRoom);
	
	return true;
}

} // End of namespace Titanic
