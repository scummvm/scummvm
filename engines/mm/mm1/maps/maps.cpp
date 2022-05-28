/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "mm/mm1/maps/maps.h"

namespace MM {
namespace MM1 {
namespace Maps {

Maps::Maps() :
	_map00(this), _map01(this), _map02(this), _map03(this),
	_map04(this), _map05(this), _map06(this), _map07(this),
	_map08(this), _map09(this), _map10(this), _map11(this),
	_map12(this), _map13(this), _map14(this), _map15(this),
	_map16(this), _map17(this), _map18(this), _map19(this),
	_map20(this), _map21(this), _map22(this), _map23(this),
	_map24(this), _map25(this), _map26(this), _map27(this),
	_map28(this), _map29(this), _map30(this), _map31(this),
	_map32(this), _map33(this), _map34(this), _map35(this),
	_map36(this), _map37(this), _map38(this), _map39(this),
	_map40(this), _map41(this), _map42(this), _map43(this),
	_map44(this), _map45(this), _map46(this), _map47(this),
	_map48(this), _map49(this), _map50(this), _map51(this),
	_map52(this), _map53(this), _map54(this) {
}

void Maps::load(uint mapId) {
	_mapId = mapId;
	_currentMap = _maps[mapId];
	_currentMap->load();
}

} // namespace Maps
} // namespace MM1
} // namespace MM
