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

#include "titanic/game/transport/lift_indicator.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CLiftindicator, CLift)
	ON_MESSAGE(EnterRoomMsg)
END_MESSAGE_MAP()

CLiftindicator::CLiftindicator() : CLift(),
		_fieldFC(0), _field108(0), _field10C(0) {
}

void CLiftindicator::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_fieldFC, indent);
	file->writePoint(_pos2, indent);
	file->writeNumberLine(_field108, indent);
	file->writeNumberLine(_field10C, indent);

	CLift::save(file, indent);
}

void CLiftindicator::load(SimpleFile *file) {
	file->readNumber();
	_fieldFC = file->readNumber();
	_pos2 = file->readPoint();
	_field108 = file->readNumber();
	_field10C = file->readNumber();

	CLift::load(file);
}

} // End of namespace Titanic
