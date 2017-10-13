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

#include "titanic/sound/restricted_auto_music_player.h"
#include "titanic/core/room_item.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CRestrictedAutoMusicPlayer, CAutoMusicPlayer)
	ON_MESSAGE(EnterRoomMsg)
	ON_MESSAGE(LeaveRoomMsg)
END_MESSAGE_MAP()

void CRestrictedAutoMusicPlayer::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_oldNodeName, indent);
	file->writeQuotedLine(_newNodeName, indent);
	file->writeQuotedLine(_newRoomName, indent);
	file->writeQuotedLine(_oldRoomName, indent);

	CAutoMusicPlayer::save(file, indent);
}

void CRestrictedAutoMusicPlayer::load(SimpleFile *file) {
	file->readNumber();
	_oldNodeName = file->readString();
	_newNodeName = file->readString();
	_newRoomName = file->readString();
	_oldRoomName = file->readString();

	CAutoMusicPlayer::load(file);
}

bool CRestrictedAutoMusicPlayer::EnterRoomMsg(CEnterRoomMsg *msg) {
	if (!msg->_oldRoom)
		return true;
	if (petCheckNode(_oldNodeName))
		return false;

	CString roomName = msg->_oldRoom->getName();
	if (!_oldRoomName.compareToIgnoreCase(roomName)) {
		_isEnabled = true;
		return false;
	} else {
		return CAutoMusicPlayer::EnterRoomMsg(msg);
	}
}

bool CRestrictedAutoMusicPlayer::LeaveRoomMsg(CLeaveRoomMsg *msg) {
	CString roomName = msg->_newRoom->getName();

	if (petCheckNode(_newNodeName) || !_newRoomName.compareToIgnoreCase(roomName)) {
		_isEnabled = false;
		return true;
	} else {
		return CAutoMusicPlayer::LeaveRoomMsg(msg);
	}
}

} // End of namespace Titanic
