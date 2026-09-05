
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

#include "ultima/ultima1/logic/city_castle_logic.h"
#include "ultima/ultima1/core/strings.h"
#include "ultima/ultima1/metaengine.h"
#include "ultima/ultima1/ultima1.h"

namespace Ultima {
namespace Ultima1 {
namespace Logic {

constexpr int LOCATION_STYLE_COUNT = 10;

// Per map-style sets of widgets to populate a city/castle interior with,
// indexed by Map::_mapStyle. The first two styles are castles (each has a
// king and princess); the remaining eight are cities. Unused slots within
// a style are padded with a _type of -1
static const Data::LocationEntity CITY_WIDGETS[LOCATION_STYLE_COUNT][Data::LOCATION_ENTITY_COUNT] = {
	// Style 0 - castle
	{
		{ 20, { 33, 4 }, 2000 }, { 22, { 35, 12 }, 1 }, { 17, { 1, 6 }, 500 },
		{ 17, { 17, 6 }, 500 }, { 17, { 17, 11 }, 500 }, { 17, { 25, 7 }, 500 },
		{ 17, { 25, 12 }, 500 }, { 17, { 36, 9 }, 500 }, { 19, { 35, 6 }, 1 },
		{ -1, { 0, 0 }, 0 }, { -1, { 0, 0 }, 0 }, { -1, { 0, 0 }, 0 },
		{ -1, { 0, 0 }, 0 }, { -1, { 0, 0 }, 0 }, { -1, { 0, 0 }, 0 }
	},
	// Style 1 - castle
	{
		{ 20, { 25, 4 }, 2000 }, { 22, { 31, 2 }, 1 }, { 17, { 28, 12 }, 500 },
		{ 17, { 34, 12 }, 500 }, { 17, { 0, 8 }, 500 }, { 17, { 11, 3 }, 500 },
		{ 17, { 22, 1 }, 500 }, { 17, { 28, 1 }, 500 }, { 17, { 33, 5 }, 500 },
		{ 19, { 22, 14 }, 1 }, { -1, { 0, 0 }, 0 }, { -1, { 0, 0 }, 0 },
		{ -1, { 0, 0 }, 0 }, { -1, { 0, 0 }, 0 }, { -1, { 0, 0 }, 0 }
	},
	// Style 2 - city
	{
		{ 19, { 15, 5 }, 1 }, { 50, { 34, 6 }, 1 }, { 17, { 2, 9 }, 500 },
		{ 17, { 16, 16 }, 500 }, { 17, { 18, 2 }, 500 }, { 17, { 20, 8 }, 500 },
		{ 17, { 21, 16 }, 500 }, { 17, { 36, 8 }, 500 }, { 21, { 6, 3 }, 1 },
		{ 21, { 6, 14 }, 1 }, { 21, { 13, 14 }, 1 }, { 21, { 25, 3 }, 1 },
		{ 21, { 25, 12 }, 1 }, { 21, { 32, 3 }, 1 }, { -1, { 0, 0 }, 0 }
	},
	// Style 3 - city
	{
		{ 19, { 15, 9 }, 1 }, { 50, { 27, 3 }, 1 }, { 17, { 1, 10 }, 500 },
		{ 17, { 9, 8 }, 500 }, { 17, { 16, 1 }, 500 }, { 17, { 21, 16 }, 500 },
		{ 17, { 36, 7 }, 500 }, { 21, { 4, 16 }, 1 }, { 21, { 5, 3 }, 1 },
		{ 21, { 12, 3 }, 1 }, { 21, { 30, 1 }, 1 }, { 21, { 34, 12 }, 1 },
		{ -1, { 0, 0 }, 0 }, { -1, { 0, 0 }, 0 }, { -1, { 0, 0 }, 0 }
	},
	// Style 4 - city
	{
		{ 19, { 17, 3 }, 1 }, { 50, { 21, 5 }, 1 }, { 17, { 10, 10 }, 500 },
		{ 17, { 15, 15 }, 500 }, { 17, { 22, 15 }, 500 }, { 17, { 23, 9 }, 500 },
		{ 21, { 3, 12 }, 1 }, { 21, { 6, 3 }, 1 }, { 21, { 19, 1 }, 1 },
		{ 21, { 34, 1 }, 1 }, { -1, { 0, 0 }, 0 }, { -1, { 0, 0 }, 0 },
		{ -1, { 0, 0 }, 0 }, { -1, { 0, 0 }, 0 }, { -1, { 0, 0 }, 0 }
	},
	// Style 5 - city
	{
		{ 19, { 19, 12 }, 1 }, { 50, { 24, 3 }, 1 }, { 17, { 1, 7 }, 500 },
		{ 17, { 1, 11 }, 500 }, { 17, { 8, 16 }, 500 }, { 17, { 14, 7 }, 500 },
		{ 17, { 29, 16 }, 500 }, { 17, { 36, 11 }, 500 }, { 21, { 4, 1 }, 1 },
		{ 21, { 12, 1 }, 1 }, { 21, { 17, 1 }, 1 }, { 21, { 22, 1 }, 1 },
		{ 21, { 28, 1 }, 1 }, { 21, { 34, 1 }, 1 }, { -1, { 0, 0 }, 0 }
	},
	// Style 6 - city
	{
		{ 17, { 17, 2 }, 500 }, { 17, { 17, 15 }, 500 }, { 17, { 20, 2 }, 500 },
		{ 17, { 20, 15 }, 500 }, { 21, { 12, 8 }, 1 }, { 21, { 25, 8 }, 1 },
		{ -1, { 0, 0 }, 0 }, { -1, { 0, 0 }, 0 }, { -1, { 0, 0 }, 0 },
		{ -1, { 0, 0 }, 0 }, { -1, { 0, 0 }, 0 }, { -1, { 0, 0 }, 0 },
		{ -1, { 0, 0 }, 0 }, { -1, { 0, 0 }, 0 }, { -1, { 0, 0 }, 0 }
	},
	// Style 7 - city
	{
		{ 50, { 20, 4 }, 1 }, { 17, { 1, 1 }, 500 }, { 17, { 1, 16 }, 500 },
		{ 17, { 8, 9 }, 500 }, { 17, { 16, 15 }, 500 }, { 17, { 18, 10 }, 500 },
		{ 17, { 21, 15 }, 500 }, { 17, { 29, 9 }, 500 }, { 17, { 36, 1 }, 500 },
		{ 17, { 36, 16 }, 500 }, { 21, { 8, 4 }, 1 }, { 21, { 9, 13 }, 1 },
		{ 21, { 18, 2 }, 1 }, { 21, { 28, 13 }, 1 }, { 21, { 29, 4 }, 1 }
	},
	// Style 8 - city
	{
		{ 19, { 31, 14 }, 1 }, { 50, { 27, 6 }, 1 }, { 17, { 4, 11 }, 500 },
		{ 17, { 17, 14 }, 500 }, { 17, { 20, 14 }, 500 }, { 21, { 7, 4 }, 1 },
		{ 21, { 18, 4 }, 1 }, { 21, { 29, 3 }, 1 }, { -1, { 0, 0 }, 0 },
		{ -1, { 0, 0 }, 0 }, { -1, { 0, 0 }, 0 }, { -1, { 0, 0 }, 0 },
		{ -1, { 0, 0 }, 0 }, { -1, { 0, 0 }, 0 }, { -1, { 0, 0 }, 0 }
	},
	// Style 9 - city
	{
		{ 19, { 19, 10 }, 5 }, { 17, { 6, 12 }, 500 }, { 17, { 6, 15 }, 500 },
		{ 17, { 7, 7 }, 500 }, { 17, { 9, 5 }, 500 }, { 17, { 28, 17 }, 500 },
		{ 17, { 37, 8 }, 500 }, { 21, { 1, 13 }, 1 }, { 21, { 4, 1 }, 1 },
		{ 21, { 16, 1 }, 1 }, { 21, { 24, 1 }, 1 }, { 21, { 33, 16 }, 1 },
		{ 21, { 34, 1 }, 1 }, { -1, { 0, 0 }, 0 }, { -1, { 0, 0 }, 0 }
	}
};

static const int8 DELTA_X[5] = { 0, -1, 1, 0, 0 };
static const int8 DELTA_Y[5] = { 0, 0, 0, -1, 1 };


void CityCastleLogic::entering() {
	loadEntities();
}

bool CityCastleLogic::drop() {
	g_engine->addView("Drop");
	return false;
}

bool CityCastleLogic::get() {
	writeString("Get");

	if (_G(map)._mapType == Data::MAPTYPE_CASTLE) {
		const auto &pos = _G(savegame)._locationPosition;

		switch (_G(map).getTileAt(pos.x, pos.y)) {
		case Data::CTILE_STEAL_WEAPON:
			if (checkCastlePermission())
				findWeapon(false);
			break;
		case Data::CTILE_STEAL_ARMOR:
			if (checkCastlePermission())
				findArmor(false);
			break;
		case Data::CTILE_STEAL_FOOD:
			if (checkCastlePermission())
				findFood(false);
			break;
		default:
			writeString(" - nothing here!\n");
			break;
		}
	} else {
		if (_G(map)._mapType == Data::MAPTYPE_CITY)
			writeString(" what");
		writeString("?\n");
		playFX(1);
	}

	return true;
}

bool CityCastleLogic::steal() {
	writeString("Steal");

	const auto &pos = _G(savegame)._locationPosition;
	switch (_G(map).getTileAt(pos.x, pos.y)) {
	case Data::CTILE_STEAL_FOOD:
		findFood(true);
		break;
	case Data::CTILE_STEAL_WEAPON:
		findWeapon(true);
		break;
	case Data::CTILE_STEAL_ARMOR:
		findArmor(true);
		break;
	default:
		writeString(" - nothing here!\n");
		playFX(1);
		break;
	}

	return true;
}

bool CityCastleLogic::checkCastlePermission() {
	if (_G(savegame)._castleItemAllowance <= 0) {
		writeString("\n");
		writeString("Thou hast not the king's\n");
		writeString("permission!\n");
		playFX(1);
		return false;
	}

	--_G(savegame)._castleItemAllowance;
	return true;
}

bool CityCastleLogic::checkCaughtStealing() {
	auto &sg = _G(savegame);
	int roll = getRandomNumber(1, 255);
	bool caught = (sg._guardsHostile != 0) || (roll < 38);

	if (!caught && sg._class == Data::CLASS_WIZARD)
		return false;

	if (!caught) {
		if (roll > 77)
			return false;
		caught = true;
	}

	writeString("\n");
	writeString("Oh no!  Thou wert caught!\n");
	sg._guardsHostile = 1;
	return true;
}

void CityCastleLogic::findWeapon(bool checkCaught) {
	if (checkCaught && checkCaughtStealing())
		return;

	writeString("\n");
	writeString("Thou dost find a");

	int idx = getRandomNumber(1, 15);
	const char *name = Data::WEAPON_NAMES[idx];

	if (isVowel(*name))
		writeString("n");
	writeString(Common::String(name).size() > 8 ? "\n" : " ");
	writeString("%s\n", name);

	auto &sg = _G(savegame);
	if (sg._weapons[idx] < 255)
		++sg._weapons[idx];
}

void CityCastleLogic::findFood(bool checkCaught) {
	if (checkCaught && checkCaughtStealing())
		return;

	writeString("\n");
	writeString("Thou dost find ");

	int amount = getRandomNumber(2, 31);
	writeString("%d bags of\n", amount);
	writeString("food!\n");

	_G(savegame)._food += amount;
	redrawStats();
}

void CityCastleLogic::findArmor(bool checkCaught) {
	if (checkCaught && checkCaughtStealing())
		return;

	writeString("\n");
	writeString("Thou dost find ");

	int idx = getRandomNumber(1, 5);
	if (idx <= 4)
		writeString("a ");
	writeString("%s\n", Data::ARMOR_NAMES[idx]);

	auto &sg = _G(savegame);
	if (sg._armor[idx] < 255)
		++sg._armor[idx];
}

void CityCastleLogic::loadEntities() {
	auto &map = _G(map);
	auto &savegame = _G(savegame);
	const Data::LocationEntity *widgets = CITY_WIDGETS[map._mapStyle];

	for (int idx = 0; idx < Data::LOCATION_ENTITY_COUNT; ++idx)
		savegame._locationEntities[idx] = widgets[idx];
}

/*-------------------------------------------------------------------*/

CityLogic::CityLogic() {
	_G(map)._tavernCtr = 0;
	_G(map)._mapStyle = (_G(map)._currentMap - 1) % 8 + 2;
	_G(map)._mapType = Data::MAPTYPE_CITY;
}

void CityLogic::entering() {
	_G(savegame)._locationPosition = Common::Point(19, 17);
	CityCastleLogic::entering();
}

bool CityLogic::move(Data::Direction dir) {
	const int deltaX = DELTA_X[dir];
	const int deltaY = DELTA_Y[dir];
	const int x = _G(savegame)._locationPosition.x + deltaX;
	const int y = _G(savegame)._locationPosition.y + deltaY;

	if (x < 0 || y < 0 || x >= Data::CITY_WIDTH || y >= Data::CITY_HEIGHT) {
		// Left the bounds of the map, returning to the overworld
		_G(map).load(Data::MAP_OVERWORLD);
	} else {
		int tile = checkAt(x, y);

		if (tile == -1 || tile == Data::CTILE_BLANK) {
			// Write direction traveled
			writeString(Common::String::format("%s\n", Data::DIRECTION_NAMES[dir]));

			_G(savegame)._locationPosition = Common::Point(x, y);
			playFX(1);
			redrawMap();
			
		} else {
			// Can't move in that direction
			writeString("Blocked!\n");
			playFX(1);
		}
	}

	return true;
}

bool CityLogic::transact() {
	writeString("Transact");

	if (_G(savegame)._guardsHostile) {
		writeString("\n");
		writeString("None will talk to thee!\n");
		return true;
	}

	const auto &pos = _G(savegame)._locationPosition;
	int tile = _G(map).getTileAt(pos.x, pos.y);
	if (tile < Data::CTILE_ARMORY || tile > Data::CTILE_TRANSPORT) {
		writeString("\n");
		writeString("Thou art not by a counter!\n");
		playFX(1);
		return true;
	}

	g_engine->addView("Merchant");
	return false;
}

int CityLogic::checkAt(int x, int y) const {
	int tile = _G(map).getTileAt(x, y);
	if (tile == Data::CTILE_BLANK)
		return tile;

	if (tile == Data::CTILE_GROUND || tile >= Data::CTILE_GUARD_MB)
		return -1;

	return 0;
}


/*-------------------------------------------------------------------*/

CastleLogic::CastleLogic() {
	_G(map)._castleNum = _G(map)._currentMap - 33;
	_G(map)._mapStyle = (_G(map)._currentMap - 1) % 2;
	_G(map)._castleNum2 = _G(map)._castleNum + 1;
	_G(map)._mapType = Data::MAPTYPE_CASTLE;

	_G(savegame)._guardsHostile = 0;
	_G(savegame)._hasCastleKey = 0;
	_G(savegame)._freeingPrincess = 0;
	_G(savegame)._castleItemAllowance = 0;

	// The two castle map styles are vertical mirrors of each other, so
	// the row these two cells sit on differs between them
	int row = (_G(map)._mapStyle == 0) ? 14 : 4;
	_G(map)[row][35] = 11;
	_G(map)[row][31] = 11;

	_G(savegame)._castleKeyVal = getRandomNumber(1) == 1 ? 61 : 60;
}

bool CastleLogic::transact() {
	writeString("Transact with king\n");

	const auto &pos = _G(savegame)._locationPosition;
	if (_G(map).getTileAt(pos.x, pos.y) != Data::CTILE_KING) {
		writeString("He is not here!\n");
		return true;
	}

	if (_G(savegame)._guardsHostile) {
		writeString("He rejects thine offer!\n");
		return true;
	}

	g_engine->addView("King");
	return false;
}

void CastleLogic::entering() {
	_G(savegame)._locationPosition = Common::Point(0, 9);
	CityCastleLogic::entering();
}

} // namespace Logic
} // namespace Ultima1
} // namespace Ultima
