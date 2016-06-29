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

#include "titanic/game/gondolier/gondolier_base.h"

namespace Titanic {

int CGondolierBase::_v1;
int CGondolierBase::_v2;
int CGondolierBase::_v3;
int CGondolierBase::_v4;
int CGondolierBase::_v5;
int CGondolierBase::_v6;
int CGondolierBase::_v7;
int CGondolierBase::_v8;
int CGondolierBase::_v9;
int CGondolierBase::_v10;

void CGondolierBase::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_v1, indent);
	file->writeNumberLine(_v2, indent);
	file->writeNumberLine(_v3, indent);
	file->writeNumberLine(_v4, indent);
	file->writeNumberLine(_v5, indent);
	file->writeNumberLine(_v6, indent);
	file->writeNumberLine(_v7, indent);
	file->writeNumberLine(_v8, indent);
	file->writeNumberLine(_v9, indent);
	file->writeNumberLine(_v10, indent);

	CGameObject::save(file, indent);
}

void CGondolierBase::load(SimpleFile *file) {
	file->readNumber();
	_v1 = file->readNumber();
	_v2 = file->readNumber();
	_v3 = file->readNumber();
	_v4 = file->readNumber();
	_v5 = file->readNumber();
	_v6 = file->readNumber();
	_v7 = file->readNumber();
	_v8 = file->readNumber();
	_v9 = file->readNumber();
	_v10 = file->readNumber();

	CGameObject::load(file);
}

} // End of namespace Titanic
