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

#include "titanic/game/idle_summoner.h"

namespace Titanic {

CIdleSummoner::CIdleSummoner() : CGameObject(), _fieldBC(0x57E40),
	_fieldC0(0xEA60), _fieldC4(0x57E40), _fieldC8(0xEA60),
	_fieldCC(0xEA60), _fieldD0(0xEA60), _fieldD4(0xEA60),
	_fieldD8(0xEA60), _fieldDC(0xEA60) {
}

void CIdleSummoner::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_fieldBC, indent);
	file->writeNumberLine(_fieldC0, indent);
	file->writeNumberLine(_fieldC4, indent);
	file->writeNumberLine(_fieldC8, indent);
	file->writeNumberLine(_fieldCC, indent);
	file->writeNumberLine(_fieldD0, indent);
	file->writeNumberLine(_fieldD4, indent);
	file->writeNumberLine(_fieldD8, indent);
	file->writeNumberLine(_fieldDC, indent);

	CGameObject::save(file, indent);
}

void CIdleSummoner::load(SimpleFile *file) {
	file->readNumber();
	_fieldBC = file->readNumber();
	_fieldC0 = file->readNumber();
	_fieldC4 = file->readNumber();
	_fieldC8 = file->readNumber();
	_fieldCC = file->readNumber();
	_fieldD0 = file->readNumber();
	_fieldD4 = file->readNumber();
	_fieldD8 = file->readNumber();
	_fieldDC = file->readNumber();

	CGameObject::load(file);
}

} // End of namespace Titanic
