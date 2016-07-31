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

#include "titanic/npcs/succubus.h"

namespace Titanic {

int CSuccUBus::_v0;
int CSuccUBus::_v1;
int CSuccUBus::_v2;
int CSuccUBus::_v3;
int CSuccUBus::_v4;

CSuccUBus::CSuccUBus() : CTrueTalkNPC() {
	_field108 = -1;
	_field10C = -1;
	_field110 = -1;
	_field114 = -1;
	_field118 = 0x44;
	_field11C = 0xA8;
	_field120 = 0xA8;
	_field124 = 0xF8;
	_field128 = 0;
	_field12C = 0x0E;
	_field130 = 0x0E;
	_field134 = 27;
	_field138 = 40;
	_field13C = 0x44;
	_field140 = 1;
	_field144 = 0;
	_field148 = 0;
	_field14C = 0;
	_field150 = 0xE0;
	_field154 = 0;
	_field158 = 0;
	_field15C = 0;
	_string2 = "NULL";
	_field16C = 28;
	_field170 = 40;
	_field174 = 82;
	_field178 = 284;
	_field17C = 148;
	_field180 = 339;
	_field184 = 15;
	_field188 = 0;
	_field18C = 0;
	_field190 = 0;
	_field194 = 240;
	_field198 = 340;
	_field19C = 0;
	_field1A0 = -1;
	_field1A4 = 0;
	_field1A8 = 0;
	_field1AC = 0;
	_field1B0 = 0;
	_field1B4 = 303;
	_field1B8 = 312;
	_field1BC = 313;
	_field1C0 = 325;
	_field1C4 = 326;
	_field1C8 = 347;
	_field1CC = 348;
	_field1D0 = 375;
	_field1D4 = 1;
	_field1D8 = 0;
}

void CSuccUBus::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);

	file->writeNumberLine(_v0, indent);
	file->writeNumberLine(_field108, indent);
	file->writeNumberLine(_field10C, indent);
	file->writeNumberLine(_field110, indent);
	file->writeNumberLine(_field114, indent);
	file->writeNumberLine(_field118, indent);
	file->writeNumberLine(_field11C, indent);
	file->writeNumberLine(_field120, indent);
	file->writeNumberLine(_field124, indent);
	file->writeNumberLine(_field128, indent);
	file->writeNumberLine(_field12C, indent);
	file->writeNumberLine(_field130, indent);
	file->writeNumberLine(_field134, indent);
	file->writeNumberLine(_field138, indent);
	file->writeNumberLine(_field13C, indent);
	file->writeNumberLine(_field140, indent);

	file->writeNumberLine(_v2, indent);
	file->writeNumberLine(_field148, indent);
	file->writeNumberLine(_field14C, indent);
	file->writeNumberLine(_field150, indent);
	file->writeNumberLine(_field154, indent);
	file->writeNumberLine(_field158, indent);
	file->writeNumberLine(_field15C, indent);

	file->writeQuotedLine(_string2, indent);
	file->writeNumberLine(_field16C, indent);
	file->writeNumberLine(_field170, indent);
	file->writeNumberLine(_field174, indent);
	file->writeNumberLine(_field178, indent);
	file->writeNumberLine(_field17C, indent);
	file->writeNumberLine(_field180, indent);
	file->writeNumberLine(_field184, indent);
	file->writeNumberLine(_field188, indent);
	file->writeNumberLine(_field18C, indent);
	file->writeNumberLine(_field190, indent);
	file->writeNumberLine(_field194, indent);
	file->writeNumberLine(_field198, indent);
	file->writeNumberLine(_field19C, indent);
	file->writeNumberLine(_field1A0, indent);
	file->writeNumberLine(_field1A4, indent);
	file->writeNumberLine(_field1A8, indent);
	file->writeNumberLine(_field1AC, indent);
	file->writeNumberLine(_field1B0, indent);
	file->writeNumberLine(_field1B4, indent);
	file->writeNumberLine(_field1B8, indent);
	file->writeNumberLine(_field1BC, indent);
	file->writeNumberLine(_field1C0, indent);
	file->writeNumberLine(_field1C4, indent);
	file->writeNumberLine(_field1C8, indent);
	file->writeNumberLine(_field1CC, indent);
	file->writeNumberLine(_field1D0, indent);
	file->writeNumberLine(_field1D4, indent);

	file->writeNumberLine(_v3, indent);
	file->writeNumberLine(_field1D8, indent);
	file->writeNumberLine(_field104, indent);

	CTrueTalkNPC::save(file, indent);
}

void CSuccUBus::load(SimpleFile *file) {
	file->readNumber();

	_v0 = file->readNumber();
	_field108 = file->readNumber();
	_field10C = file->readNumber();
	_field110 = file->readNumber();
	_field114 = file->readNumber();
	_field118 = file->readNumber();
	_field11C = file->readNumber();
	_field120 = file->readNumber();
	_field124 = file->readNumber();
	_field128 = file->readNumber();
	_field12C = file->readNumber();
	_field130 = file->readNumber();
	_field134 = file->readNumber();
	_field138 = file->readNumber();
	_field13C = file->readNumber();
	_field140 = file->readNumber();
	
	_v2 = file->readNumber();
	_field148 = file->readNumber();
	_field14C = file->readNumber();
	_field150 = file->readNumber();
	_field154 = file->readNumber();
	_field158 = file->readNumber();
	_field15C = file->readNumber();

	_string2 = file->readString();
	_field16C = file->readNumber();
	_field170 = file->readNumber();
	_field174 = file->readNumber();
	_field178 = file->readNumber();
	_field17C = file->readNumber();
	_field180 = file->readNumber();
	_field184 = file->readNumber();
	_field188 = file->readNumber();
	_field18C = file->readNumber();
	_field190 = file->readNumber();
	_field194 = file->readNumber();
	_field198 = file->readNumber();
	_field19C = file->readNumber();
	_field1A0 = file->readNumber();
	_field1A4 = file->readNumber();
	_field1A8 = file->readNumber();
	_field1AC = file->readNumber();
	_field1B0 = file->readNumber();
	_field1B4 = file->readNumber();
	_field1B8 = file->readNumber();
	_field1BC = file->readNumber();
	_field1C0 = file->readNumber();
	_field1C4 = file->readNumber();
	_field1C8 = file->readNumber();
	_field1CC = file->readNumber();
	_field1D0 = file->readNumber();
	_field1D4 = file->readNumber();

	_v3 = file->readNumber();
	_field1D8 = file->readNumber();
	_field104 = file->readNumber();

	CTrueTalkNPC::load(file);
}

} // End of namespace Titanic
