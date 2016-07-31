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

#include "titanic/sound/auto_music_player_base.h"

namespace Titanic {

CAutoMusicPlayerBase::CAutoMusicPlayerBase() : CGameObject(),
	_fieldC8(1), _fieldCC(0), _fieldD0(-1), _fieldD4(1) {
}
void CAutoMusicPlayerBase::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_string1, indent);
	file->writeNumberLine(_fieldC8, indent);
	file->writeNumberLine(_fieldCC, indent);
	file->writeNumberLine(_fieldD0, indent);
	file->writeNumberLine(_fieldD4, indent);

	CGameObject::save(file, indent);
}

void CAutoMusicPlayerBase::load(SimpleFile *file) {
	file->readNumber();
	_string1 = file->readString();
	_fieldC8 = file->readNumber();
	_fieldCC = file->readNumber();
	_fieldD0 = file->readNumber();
	_fieldD4 = file->readNumber();

	CGameObject::load(file);
}

} // End of namespace Titanic
