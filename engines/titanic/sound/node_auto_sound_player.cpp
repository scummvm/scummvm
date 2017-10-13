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

#include "titanic/sound/node_auto_sound_player.h"
#include "titanic/sound/auto_music_player.h"
#include "titanic/core/room_item.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CNodeAutoSoundPlayer, CAutoSoundPlayer)
	ON_MESSAGE(EnterNodeMsg)
	ON_MESSAGE(LeaveNodeMsg)
END_MESSAGE_MAP()

void CNodeAutoSoundPlayer::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_enabled, indent);
	CAutoSoundPlayer::save(file, indent);
}

void CNodeAutoSoundPlayer::load(SimpleFile *file) {
	file->readNumber();
	_enabled = file->readNumber();
	CAutoSoundPlayer::load(file);
}

bool CNodeAutoSoundPlayer::EnterNodeMsg(CEnterNodeMsg *msg) {
	CNodeItem *node = findNode();
	CRoomItem *room = findRoom();

	if (node == msg->_newNode) {
		CTurnOn onMsg;
		onMsg.execute(this);

		if (_enabled) {
			CChangeMusicMsg changeMsg;
			changeMsg._action = MUSIC_STOP;
			changeMsg.execute(room, CAutoMusicPlayer::_type,
				MSGFLAG_CLASS_DEF | MSGFLAG_BREAK_IF_HANDLED | MSGFLAG_SCAN);
		}
	}

	return true;
}

bool CNodeAutoSoundPlayer::LeaveNodeMsg(CLeaveNodeMsg *msg) {
	CNodeItem *node = findNode();
	CRoomItem *room = findRoom();

	if (node == msg->_oldNode) {
		CTurnOff offMsg;
		offMsg.execute(this);

		if (_enabled) {
			CChangeMusicMsg changeMsg;
			changeMsg._action = MUSIC_START;
			changeMsg.execute(room, CAutoMusicPlayer::_type,
				MSGFLAG_CLASS_DEF | MSGFLAG_BREAK_IF_HANDLED | MSGFLAG_SCAN);
		}
	}

	return true;
}

} // End of namespace Titanic
