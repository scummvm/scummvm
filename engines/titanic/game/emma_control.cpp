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

#include "titanic/game/emma_control.h"
#include "titanic/core/room_item.h"
#include "titanic/sound/auto_music_player.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CEmmaControl, CBackground)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(StatusChangeMsg)
END_MESSAGE_MAP()

void CEmmaControl::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_flag, indent);
	file->writeQuotedLine(_hiddenSoundName, indent);
	file->writeQuotedLine(_visibleSoundName, indent);

	CBackground::save(file, indent);
}

void CEmmaControl::load(SimpleFile *file) {
	file->readNumber();
	_flag = file->readNumber();
	_hiddenSoundName = file->readString();
	_visibleSoundName = file->readString();

	CBackground::load(file);
}

bool CEmmaControl::EnterViewMsg(CEnterViewMsg *msg) {
	setVisible(_flag);
	return true;
}

bool CEmmaControl::StatusChangeMsg(CStatusChangeMsg *msg) {
	_flag = !_flag;
	setVisible(_flag);
	CChangeMusicMsg changeMsg(_flag ? _visibleSoundName : _hiddenSoundName, MUSIC_NONE);
	changeMsg.execute(findRoom(), CAutoMusicPlayer::_type,
		MSGFLAG_SCAN | MSGFLAG_BREAK_IF_HANDLED | MSGFLAG_CLASS_DEF);
	return true;
}

} // End of namespace Titanic
