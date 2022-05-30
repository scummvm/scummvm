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
#include "common/rect.h"
#include "mm/mm1/maps/map00.h"

namespace MM {
namespace MM1 {
namespace Maps {

enum TownId {
	SORPIGAL = 1, PORTSMITH = 2, ALGARY = 3,
	DUSK = 4, ERLIQUIN = 5
};

enum DirMask {
	DIRMASK_NONE = 0,
	DIRMASK_N = 0xC0, DIRMASK_E = 0x30,
	DIRMASK_S = 0xC, DIRMASK_W = 3
};

/** PLACEHOLDER MAPS **/
#define PLACEHOLDER(INDEX, NAME, ID) \
	class Map##INDEX : public Map { \
	public: \
		Map##INDEX(Maps *owner) : Map(owner, NAME, ID) {} \
	}

PLACEHOLDER(01, "portsmit", 0xC03);
PLACEHOLDER(02, "algary", 0x203);
PLACEHOLDER(03, "dusk", 0x802);
PLACEHOLDER(04, "erliquin", 0x0B1A);
PLACEHOLDER(05, "cave1", 0x0A11);
PLACEHOLDER(06, "cave2", 0x1);
PLACEHOLDER(07, "cave3", 0xC01);
PLACEHOLDER(08, "cave4", 0x202);
PLACEHOLDER(09, "cave5", 0x5);
PLACEHOLDER(10, "cave6", 0x51B);
PLACEHOLDER(11, "cave7", 0x212);
PLACEHOLDER(12, "cave8", 0x601);
PLACEHOLDER(13, "cave9", 0xA00);
PLACEHOLDER(14, "areaa1", 0xF01);
PLACEHOLDER(15, "areaa2", 0x502);
PLACEHOLDER(16, "areaa3", 0xB02);
PLACEHOLDER(17, "areaa4", 0x103);
PLACEHOLDER(18, "areab1", 0xA00);
PLACEHOLDER(19, "areab2", 0x703);
PLACEHOLDER(20, "areab3", 0x101);
PLACEHOLDER(21, "areab4", 0xD03);
PLACEHOLDER(22, "areac1", 0x304);
PLACEHOLDER(23, "areac2", 0xA11);
PLACEHOLDER(24, "areac3", 0x904);
PLACEHOLDER(25, "areac4", 0xF04);
PLACEHOLDER(26, "aread1", 0x505);
PLACEHOLDER(27, "aread2", 0xB05);
PLACEHOLDER(28, "aread3", 0x106);
PLACEHOLDER(29, "aread4", 0x801);
PLACEHOLDER(30, "areae1", 0x112);
PLACEHOLDER(31, "areae2", 0x706);
PLACEHOLDER(32, "areae3", 0xB1A);
PLACEHOLDER(33, "areae4", 0x11B);
PLACEHOLDER(34, "doom", 0x706);
PLACEHOLDER(35, "blackrn", 0xF08);
PLACEHOLDER(36, "blackrs", 0x508);
PLACEHOLDER(37, "qvl1", 0xF03);
PLACEHOLDER(38, "qvl2", 0x703);
PLACEHOLDER(39, "rwl1", 0xF02);
PLACEHOLDER(40, "rwl2", 0x702);
PLACEHOLDER(41, "enf1", 0xF04);
PLACEHOLDER(42, "enf2", 0x704);
PLACEHOLDER(43, "whitew", 0xA11);
PLACEHOLDER(44, "dragad", 0x107);
PLACEHOLDER(45, "udrag1", 0xF05);
PLACEHOLDER(46, "udrag2", 0xA00);
PLACEHOLDER(47, "udrag3", 0x705);
PLACEHOLDER(48, "demon", 0x412);
PLACEHOLDER(49, "alamar", 0xB07);
PLACEHOLDER(50, "pp1", 0xF01);
PLACEHOLDER(51, "pp2", 0x701);
PLACEHOLDER(52, "pp3", 0xE00);
PLACEHOLDER(53, "pp4", 0x201);
PLACEHOLDER(54, "astral", 0xB1A);
#undef PLACEHOLDER

/**
 * Container for all the game maps
 */
class Maps {
	friend class Map;
private:
	Common::Array<Map *> _maps;
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

	/**
	 * Gets the index of a map given the id values
	 */
	uint getIndex(uint16 id, byte section);

	/**
	 * Loads tile graphics needed for rendering the 3d view
	 */
	void loadTiles();
	void loadTile();

	/**
	 * Town setup for SORPIGAL & ERLIQUIN
	 */
	void town15setup();

	/**
	 * Town setup for PORTSMITH and ALGARY
	 */
	void town23setup();

	/**
	 * Town setup for DUSK
	 */
	void town4setup();

public:
	uint _mapId = (uint)-1;
	Common::Point _mapPos;
	DirMask _mapDirectionMask = DIRMASK_NONE;
	Map *_currentMap = nullptr;
	byte _data1[32];
	int _colorOffset = 0;
	int _val1 = 0, _val2 = 0, _val3 = 0, _val4 = 0;
	int _val5 = 0, _val6 = 0, _val7 = 0;
	int _loadId = 0;
	int _loadArea = 0;
	int _loadSection = 0;
	byte _loadFlag = 0;
public:
	Maps();

	/**
	 * Load a map
	 */
	void load(uint mapId);

	/**
	 * Selects a map
	 */
	void select(uint16 id, byte section);

	/**
	 * Selects a map, and switches to in-game display
	 */
	void display(uint16 id, byte section = 1);

	/**
	 * Loads a town
	 */
	void loadTown(TownId townId);

	/**
	 * Get a given map
	 */
	Map *getMap(uint mapId) const { return _maps[mapId]; }
};

} // namespace Maps
} // namespace MM1
} // namespace MM

#endif
