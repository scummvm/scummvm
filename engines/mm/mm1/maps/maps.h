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

#ifndef MM1_MAPS_MAPS_H
#define MM1_MAPS_MAPS_H

#include "common/array.h"
#include "mm/mm1/maps/map00.h"

namespace MM {
namespace MM1 {
namespace Maps {

/** PLACEHOLDER MAPS **/
#define PLACEHOLDER(ID, NAME) \
	class Map##ID : public Map { \
	public: \
		Map##ID(Maps *owner) : Map(owner, NAME) {} \
	}

PLACEHOLDER(01, "portsmit");
PLACEHOLDER(02, "algary");
PLACEHOLDER(03, "dusk");
PLACEHOLDER(04, "erliquin");
PLACEHOLDER(05, "cave1");
PLACEHOLDER(06, "cave2");
PLACEHOLDER(07, "cave3");
PLACEHOLDER(08, "cave4");
PLACEHOLDER(09, "cave5");
PLACEHOLDER(10, "cave6");
PLACEHOLDER(11, "cave7");
PLACEHOLDER(12, "cave8");
PLACEHOLDER(13, "cave9");
PLACEHOLDER(14, "areaa1");
PLACEHOLDER(15, "areaa2");
PLACEHOLDER(16, "areaa3");
PLACEHOLDER(17, "areaa4");
PLACEHOLDER(18, "areab1");
PLACEHOLDER(19, "areab2");
PLACEHOLDER(20, "areab3");
PLACEHOLDER(21, "areab4");
PLACEHOLDER(22, "areac1");
PLACEHOLDER(23, "areac2");
PLACEHOLDER(24, "areac3");
PLACEHOLDER(25, "areac4");
PLACEHOLDER(26, "aread1");
PLACEHOLDER(27, "aread2");
PLACEHOLDER(28, "aread3");
PLACEHOLDER(29, "aread4");
PLACEHOLDER(30, "areae1");
PLACEHOLDER(31, "areae2");
PLACEHOLDER(32, "areae3");
PLACEHOLDER(33, "areae4");
PLACEHOLDER(34, "doom");
PLACEHOLDER(35, "blackrn");
PLACEHOLDER(36, "blackrs");
PLACEHOLDER(37, "qvl1");
PLACEHOLDER(38, "qvl2");
PLACEHOLDER(39, "rwl1");
PLACEHOLDER(40, "rwl2");
PLACEHOLDER(41, "enf1");
PLACEHOLDER(42, "enf2");
PLACEHOLDER(43, "whitew");
PLACEHOLDER(44, "dragad");
PLACEHOLDER(45, "udrag1");
PLACEHOLDER(46, "udrag2");
PLACEHOLDER(47, "udrag3");
PLACEHOLDER(48, "demon");
PLACEHOLDER(49, "alamar");
PLACEHOLDER(50, "pp1");
PLACEHOLDER(51, "pp2");
PLACEHOLDER(52, "pp3");
PLACEHOLDER(53, "pp4");
PLACEHOLDER(54, "astral");
#undef PLACEHOLDER

/**
 * Container for all the game maps
 */
class Maps {
	friend class Map;
private:
	Common::Array<Map *> _maps;
	uint _mapId = (uint)-1;
	Map *_currentMap = nullptr;
private:
	Map00 _map00;
	Map01 _map01;
	Map02 _map02;
	Map03 _map03;
	Map04 _map04;
	Map05 _map05;
	Map06 _map06;
	Map07 _map07;
	Map08 _map08;
	Map09 _map09;
	Map10 _map10;
	Map11 _map11;
	Map12 _map12;
	Map13 _map13;
	Map14 _map14;
	Map15 _map15;
	Map16 _map16;
	Map17 _map17;
	Map18 _map18;
	Map19 _map19;
	Map20 _map20;
	Map21 _map21;
	Map22 _map22;
	Map23 _map23;
	Map24 _map24;
	Map25 _map25;
	Map26 _map26;
	Map27 _map27;
	Map28 _map28;
	Map29 _map29;
	Map30 _map30;
	Map31 _map31;
	Map32 _map32;
	Map33 _map33;
	Map34 _map34;
	Map35 _map35;
	Map36 _map36;
	Map37 _map37;
	Map38 _map38;
	Map39 _map39;
	Map40 _map40;
	Map41 _map41;
	Map42 _map42;
	Map43 _map43;
	Map44 _map44;
	Map45 _map45;
	Map46 _map46;
	Map47 _map47;
	Map48 _map48;
	Map49 _map49;
	Map50 _map50;
	Map51 _map51;
	Map52 _map52;
	Map53 _map53;
	Map54 _map54;
private:
	/**
	 * Adds map class
	 */
	uint addMap(Map *map) {
		_maps.push_back(map);
		return _maps.size() - 1;
	}
public:
	Maps();

	/**
	 * Load a map
	 */
	void load(uint mapId);

	/**
	 * Get a given map
	 */
	Map *getMap(uint mapId) const { return _maps[mapId]; }
};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
