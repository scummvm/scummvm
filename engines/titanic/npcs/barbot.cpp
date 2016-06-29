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

#include "titanic/npcs/barbot.h"

namespace Titanic {

int CBarbot::_v0;

CBarbot::CBarbot() : CTrueTalkNPC() {
	_field108 = 0;
	_field10C = 0;
	_field110 = 0;
	_field114 = 0;
	_field118 = 0;
	_field11C = 0;
	_field120 = 0;
	_field124 = 0;
	_field128 = 0;
	_field12C = 0;
	_field130 = 0;
	_field134 = 0;
	_field138 = 0;
	_field13C = -1;
	_field140 = 30;
	_field144 = -1;
	_field148 = -1;
	_field14C = 0;
	_field150 = 0;
	_field154 = 0;
	_field158 = -1;
	_field15C = 0;
	_field160 = 0;
	_field164 = 558;
	_field168 = 585;
	_field16C = 659;
	_field170 = 692;
	_field174 = 802;
	_field178 = 816;
	_field17C = 1941;
	_field180 = 1977;
	_field184 = 1901;
	_field188 = 1941;
	_field18C = 810;
	_field190 = 816;
	_field194 = 857;
	_field198 = 865;
	_field19C = 842;
	_field1A0 = 857;
	_field1A4 = 821;
	_field1A8 = 842;
	_field1AC = 682;
	_field1B0 = 692;
	_field1B4 = 1977;
	_field1B8 = 2018;
	_field1BC = 2140;
	_field1C0 = 2170;
	_field1C4 = 2101;
	_field1C8 = 2139;
	_field1CC = 2018;
	_field1D0 = 2099;
	_field1D4 = 1902;
	_field1D8 = 2015;
	_field1E0 = 1811;
	_field1E4 = 1901;
	_field1E8 = 1810;
	_field1EC = 1703;
	_field1F0 = 1750;
	_field1F4 = 1681;
	_field1F8 = 1702;
	_field1FC = 1642;

	_field200 = 1702;
	_field204 = 1571;
	_field208 = 1641;
	_field20C = 1499;
	_field210 = 1570;
	_field214 = 1403;
	_field218 = 1463;
	_field21C = 1464;
	_field220 = 1499;
	_field224 = 1288;
	_field228 = 1295;
	_field22C = 1266;
	_field230 = 1287;
	_field234 = 1245;
	_field238 = 1265;
	_field23C = 1208;
	_field240 = 1244;
	_field244 = 1171;
	_field248 = 1207;
	_field24C = 1120;
	_field250 = 1170;
	_field254 = 1092;
	_field258 = 1120;
	_field25C = 1092;
	_field260 = 1092;
	_field264 = 1044;
	_field268 = 1091;
	_field26C = 1011;
	_field270 = 1043;
	_field274 = 1001;
	_field278 = 1010;
	_field27C = 985;
	_field280 = 1001;
	_field284 = 927;
	_field288 = 984;
	_field28C = 912;
	_field290 = 926;
	_field294 = 898;
	_field298 = 906;
	_field29C = 802;
	_field2A0 = 896;
	_field2A4 = 865;
	_field2A8 = 896;
	_field2AC = 842;
	_field2B0 = 865;
	_field2B4 = 816;
	_field2B8 = 842;
	_field2BC = 802;
	_field2C0 = 842;
	_field2C4 = 740;
	_field2C8 = 740;
	_field2CC = 740;
	_field2D0 = 692;
	_field2D4 = 610;
	_field2D8 = 558;
	_field2E0 = 610;
	_field2E4 = 500;
	_field2E8 = 558;
	_field2EC = 467;
	_field2F0 = 500;
	_field2F4 = 421;
	_field2F8 = 466;
	_field2FC = 349;
	_field300 = 306;
	_field304 = 306;
	_field308 = 348;
	_field30C = 305;
	_field310 = 306;
	_field314 = 281;
	_field318 = 305;
	_field31C = 202;
	_field320 = 281;
	_field324 = 182;
	_field328 = 202;
	_field32C = 165;
	_field330 = 182;
	_field334 = 96;
	_field338 = 165;
	_field33C = 0;
	_field340 = 95;
}

void CBarbot::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_field108, indent);
	file->writeNumberLine(_field10C, indent);
	file->writeNumberLine(_field110, indent);
	file->writeNumberLine(_field114, indent);
	file->writeNumberLine(_field118, indent);
	file->writeNumberLine(_field11C, indent);
	file->writeNumberLine(_field120, indent);
	file->writeNumberLine(_field124, indent);
	file->writeNumberLine(_field128, indent);

	file->writeNumberLine(_v0, indent);
	file->writeNumberLine(_field12C, indent);
	file->writeNumberLine(_field130, indent);
	file->writeNumberLine(_field134, indent);
	file->writeNumberLine(_field138, indent);
	file->writeNumberLine(_field13C, indent);
	file->writeNumberLine(_field140, indent);
	file->writeNumberLine(_field144, indent);
	file->writeNumberLine(_field148, indent);
	file->writeNumberLine(_field14C, indent);
	file->writeNumberLine(_field150, indent);
	file->writeNumberLine(_field154, indent);
	file->writeNumberLine(_field158, indent);
	file->writeNumberLine(_field15C, indent);
	file->writeNumberLine(_field160, indent);

	CTrueTalkNPC::save(file, indent);
}

void CBarbot::load(SimpleFile *file) {
	file->readNumber();
	_field108 = file->readNumber();
	_field10C = file->readNumber();
	_field110 = file->readNumber();
	_field114 = file->readNumber();
	_field118 = file->readNumber();
	_field11C = file->readNumber();
	_field120 = file->readNumber();
	_field124 = file->readNumber();
	_field128 = file->readNumber();

	_v0 = file->readNumber();
	_field12C = file->readNumber();
	_field130 = file->readNumber();
	_field134 = file->readNumber();
	_field138 = file->readNumber();
	_field13C = file->readNumber();
	_field140 = file->readNumber();
	_field144 = file->readNumber();
	_field148 = file->readNumber();
	_field14C = file->readNumber();
	_field150 = file->readNumber();
	_field154 = file->readNumber();
	_field158 = file->readNumber();
	_field15C = file->readNumber();
	_field160 = file->readNumber();
	
	CTrueTalkNPC::load(file);
}

bool CBarbot::EnterRoomMsg(CEnterRoomMsg *msg) {
	warning("TODO: Barbot::CEnterRoomMsg");
	return true;
}

} // End of namespace Titanic
