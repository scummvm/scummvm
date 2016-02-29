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

#include "titanic/sound/auto_sound_player.h"

namespace Titanic {

CAutoSoundPlayer::CAutoSoundPlayer() : CGameObject(),
	_fieldC8(0), _fieldCC(70), _fieldD0(0), _fieldD4(0), _fieldD8(-1),
	_fieldDC(0), _fieldE0(-1), _fieldE4(0), _fieldE8(0) {
}

void CAutoSoundPlayer::save(SimpleFile *file, int indent) const {
	file->writeNumberLine(1, indent);
	CGameObject::save(file, indent);
}

void CAutoSoundPlayer::load(SimpleFile *file) {
	file->readNumber();
	CGameObject::load(file);
}

} // End of namespace Titanic
