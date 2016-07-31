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

#include "titanic/npcs/maitre_d.h"

namespace Titanic {

int CMaitreD::_v1;

CMaitreD::CMaitreD() : CTrueTalkNPC(),
	_string2("z#40.wav"), _string3("z#40.wav"), _field108(0), _field118(1),
	_field11C(0), _field12C(0), _field130(1), _field134(0), _field138(0) {
}

void CMaitreD::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_field108, indent);
	file->writeQuotedLine(_string2, indent);
	file->writeNumberLine(_field118, indent);
	file->writeNumberLine(_field11C, indent);
	file->writeQuotedLine(_string3, indent);
	file->writeNumberLine(_field12C, indent);
	file->writeNumberLine(_field130, indent);

	file->writeNumberLine(_v1, indent);
	file->writeNumberLine(_field134, indent);
	file->writeNumberLine(_field138, indent);

	CTrueTalkNPC::save(file, indent);
}

void CMaitreD::load(SimpleFile *file) {
	file->readNumber();
	_field108 = file->readNumber();
	_string2 = file->readString();
	_field118 = file->readNumber();
	_field11C = file->readNumber();
	_string3 = file->readString();
	_field12C = file->readNumber();
	_field130 = file->readNumber();

	_v1 = file->readNumber();
	_field134 = file->readNumber();
	_field138 = file->readNumber();

	CTrueTalkNPC::load(file);
}

} // End of namespace Titanic
