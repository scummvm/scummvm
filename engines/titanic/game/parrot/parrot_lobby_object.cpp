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

#include "titanic/game/parrot/parrot_lobby_object.h"

namespace Titanic {

EMPTY_MESSAGE_MAP(CParrotLobbyObject, CGameObject);

bool CParrotLobbyObject::_haveParrot;
bool CParrotLobbyObject::_havePerch;
bool CParrotLobbyObject::_haveStick;
int CParrotLobbyObject::_flags;

void CParrotLobbyObject::init() {
	_haveParrot = true;
	_havePerch = true;
	_haveStick = true;
	_flags = 7;
}

void CParrotLobbyObject::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_haveParrot, indent);
	file->writeNumberLine(_havePerch, indent);
	file->writeNumberLine(_haveStick, indent);
	file->writeNumberLine(_flags, indent);

	CGameObject::save(file, indent);
}

void CParrotLobbyObject::load(SimpleFile *file) {
	file->readNumber();
	_haveParrot = file->readNumber();
	_havePerch = file->readNumber();
	_haveStick = file->readNumber();
	_flags = file->readNumber();

	CGameObject::load(file);
}

} // End of namespace Titanic
