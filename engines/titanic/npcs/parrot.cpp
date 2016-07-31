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

#include "titanic/npcs/parrot.h"

namespace Titanic {

int CParrot::_v1;
int CParrot::_v2;
int CParrot::_v3;
int CParrot::_v4;
int CParrot::_v5;

CParrot::CParrot() : CTrueTalkNPC() {
	_field108 = 0;
	_string2 = "CarryParrot";
	_field118 = 1;
	_field11C = 25;
	_field120 = 0;
	_field124 = 73;
	_field128 = 58;
	_field12C = 0;
	_field130 = 0;
	_field134 = 0;
	_field138 = 851;
	_field13C = 851;
	_field140 = 265;
	_field144 = 274;
	_field148 = 726;
	_field14C = 730;
	_field150 = 510;
	_field154 = 570;
	_field158 = 569;
	_field15C = 689;
	_field160 = 690;
	_field164 = 725;
	_field168 = 375;
	_field16C = 508;
	_field170 = 363;
	_field174 = 375;
	_field178 = 303;
	_field17C = 313;
	_field180 = 279;
	_field184 = 302;
	_field188 = 260;
	_field18C = 264;
	_field190 = 315;
	_field194 = 327;
	_field198 = 330;
	_field19C = 360;
	_field1A0 = 175;
	_field1A4 = 259;
	_field1A8 = 175;
	_field1AC = 175;
	_field1B0 = 162;
	_field1B4 = 175;
	_field1B8 = 150;
	_field1BC = 162;
	_field1C0 = 135;
	_field1C4 = 150;
	_field1C8 = 95;
	_field1CC = 135;
	_field1D0 = 76;
	_field1D4 = 95;
	_field1D8 = 55;
	_field1DC = 76;
	_field1E0 = 30;
	_field1E4 = 55;
	_field1E8 = 0;
	_field1EC = 30;

	_assetName = "z454.dlg";
	_assetNumber = 0x13880;
}

void CParrot::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_assetNumber, indent);
	
	file->writeQuotedLine(_assetName, indent);
	file->writeNumberLine(_field108, indent);
	file->writeNumberLine(_v1, indent);
	file->writeNumberLine(_v2, indent);
	file->writeNumberLine(_v3, indent);
	
	file->writeQuotedLine(_string2, indent);
	file->writeNumberLine(_field118, indent);
	file->writeNumberLine(_field11C, indent);
	file->writeNumberLine(_field120, indent);
	file->writeNumberLine(_field124, indent);
	file->writeNumberLine(_field128, indent);
	file->writeNumberLine(_field12C, indent);
	file->writeNumberLine(_field130, indent);
	file->writeNumberLine(_v4, indent);
	file->writeNumberLine(_v5, indent);

	CTrueTalkNPC::save(file, indent);
}

void CParrot::load(SimpleFile *file) {
	file->readNumber();
	_assetNumber = file->readNumber();

	_assetName = file->readString();
	_field108 = file->readNumber();
	_v1 = file->readNumber();
	_v2 = file->readNumber();
	_v3 = file->readNumber();

	_string2 = file->readString();
	_field118 = file->readNumber();
	_field11C = file->readNumber();
	_field120 = file->readNumber();
	_field124 = file->readNumber();
	_field128 = file->readNumber();
	_field12C = file->readNumber();
	_field130 = file->readNumber();
	_v4 = file->readNumber();
	_v5 = file->readNumber();

	CTrueTalkNPC::load(file);
}

} // End of namespace Titanic
