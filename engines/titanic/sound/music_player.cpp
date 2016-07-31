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

#include "titanic/sound/music_player.h"

namespace Titanic {

void CMusicPlayer::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_fieldBC, indent);
	file->writeQuotedLine(_string1, indent);
	file->writeNumberLine(_fieldCC, indent);
	file->writeNumberLine(_fieldD0, indent);

	CGameObject::save(file, indent);
}

void CMusicPlayer::load(SimpleFile *file) {
	file->readNumber();
	_fieldBC = file->readNumber();
	_string1 = file->readString();
	_fieldCC = file->readNumber();
	_fieldD0 = file->readNumber();

	CGameObject::load(file);
}

bool CMusicPlayer::EnterRoomMsg(CEnterRoomMsg *msg) {
	warning("TODO: CMusicPlayer::handleEvent");
	return true;
}

} // End of namespace Titanic
