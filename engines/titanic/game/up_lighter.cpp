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

#include "titanic/game/up_lighter.h"

namespace Titanic {

CUpLighter::CUpLighter() : CDropTarget(), _field118(0),
	_field11C(0), _field120(0), _field124(0) {
}

void CUpLighter::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_field118, indent);
	file->writeNumberLine(_field11C, indent);
	file->writeNumberLine(_field120, indent);
	file->writeNumberLine(_field124, indent);

	CDropTarget::save(file, indent);
}

void CUpLighter::load(SimpleFile *file) {
	file->readNumber();
	_field118 = file->readNumber();
	_field11C = file->readNumber();
	_field120 = file->readNumber();
	_field124 = file->readNumber();

	CDropTarget::load(file);
}

bool CUpLighter::EnterRoomMsg(CEnterRoomMsg *msg) {
	warning("CUpLighter::handleEvent");
	return true;
}

} // End of namespace Titanic
