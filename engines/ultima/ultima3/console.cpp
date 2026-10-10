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

#include "common/file.h"
#include "common/fs.h"
#include "ultima/ultima3/console.h"
#include "ultima/ultima3/ultima3.h"
#include "ultima/ultima3/data/data.h"
#include "ultima/ultima3/logic/location_logic.h"

namespace Ultima {
namespace Ultima3 {

constexpr int MAX_VALUE = 9999;
constexpr int MAX_ITEMS = 99;
constexpr byte DEFAULT_ENEMY = 0x18;
constexpr byte MARKS_AND_CARDS = 0xFF;
constexpr const char *WORLD_FILE = "SOSARIA.ULT";
constexpr const char *ROSTER_FILE = "ROSTER.ULT";
constexpr const char *PARTY_FILE = "PARTY.ULT";

Console::Console() : GUI::Debugger() {
	registerCmd("map", WRAP_METHOD(Console, cmdMap));
	registerCmd("tiles", WRAP_METHOD(Console, cmdTiles));
	registerCmd("teleport", WRAP_METHOD(Console, cmdTeleport));
	registerCmd("locations", WRAP_METHOD(Console, cmdLocations));
	registerCmd("intangible", WRAP_METHOD(Console, cmdIntangible));
	registerCmd("enemy", WRAP_METHOD(Console, cmdEnemy));
	registerCmd("hp", WRAP_METHOD(Console, cmdHP));
	registerCmd("food", WRAP_METHOD(Console, cmdFood));
	registerCmd("gold", WRAP_METHOD(Console, cmdGold));
	registerCmd("inventory", WRAP_METHOD(Console, cmdInventory));
	registerCmd("load", WRAP_METHOD(Console, cmdLoad));
	registerCmd("save", WRAP_METHOD(Console, cmdSave));
	registerCmd("exodus", WRAP_METHOD(Console, cmdExodus));
}

Console::~Console() {
}

bool Console::getTargets(int argc, const char **argv, int index, int &first, int &last) {
	const Data::Savegame &sg = _G(savegame);

	if (sg._partySize == 0) {
		debugPrintf("There is no party\n");
		return false;
	}

	if (argc > index) {
		int position = atoi(argv[index]);
		if (position < 1 || position > sg._partySize) {
			debugPrintf("Party position must be between 1 and %d\n", sg._partySize);
			return false;
		}

		first = last = position - 1;
	} else {
		first = 0;
		last = sg._partySize - 1;
	}

	return true;
}

void Console::ensureGame() {
	Data::Savegame &sg = _G(savegame);

	if (sg._partySize == 0)
		sg.setupDummyParty();

	if (!sg._mapLoaded) {
		_G(map).load(WORLD_FILE);
		sg._mapLoaded = true;
	}
}

void Console::restartGame() {
	_G(resumeGame) = true;
	g_engine->replaceView(g_engine->gameViewName(), true);
}

bool Console::cmdMap(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("map <name>  - SOSARIA for the world, or the name of a town, castle or dungeon:\n");

		for (int i = 0; i < LocationLogic::entranceCount(); ++i) {
			Common::String name(LocationLogic::entranceFilename(i));
			name.erase(name.size() - 4);
			debugPrintf("  %s\n", name.c_str());
		}

		return true;
	}

	int index = -1;
	Common::String wanted(argv[1]);
	wanted.toUppercase();

	if (wanted != "SOSARIA") {
		for (int i = 0; i < LocationLogic::entranceCount() && index < 0; ++i) {
			Common::String name(LocationLogic::entranceFilename(i));
			name.erase(name.size() - 4);
			if (name == wanted)
				index = i;
		}

		if (index < 0) {
			debugPrintf("No such map: %s\n", argv[1]);
			return true;
		}
	}

	ensureGame();
	LocationLogic logic;

	if (_G(savegame)._location != Data::LOCATION_SOSARIA)
		logic.exitToWorld();

	if (index >= 0 && !logic.enterLocation(index)) {
		debugPrintf("Could not enter %s, which has no data file\n", argv[1]);
		return true;
	}

	restartGame();
	return false;
}

bool Console::cmdExodus(int argc, const char **argv) {
	constexpr byte ALL_CARDS = 0x0F;
	constexpr int START_X = 30, START_Y = 12;

	ensureGame();
	LocationLogic logic;

	if (_G(savegame)._location != Data::LOCATION_SOSARIA)
		logic.exitToWorld();

	int index = -1;
	for (int i = 0; i < LocationLogic::entranceCount() && index < 0; ++i) {
		if (!strcmp(LocationLogic::entranceFilename(i), "EXODUS.ULT"))
			index = i;
	}

	if (index < 0 || !logic.enterLocation(index)) {
		debugPrintf("Could not enter the castle of Exodus, which has no data file\n");
		return true;
	}

	// In front of the machine's first slot, holding all four cards
	Data::Savegame &sg = _G(savegame);
	sg._posX = START_X;
	sg._posY = START_Y;
	sg.partyMember(0)._marksAndCards |= ALL_CARDS;

	restartGame();
	return false;
}

bool Console::cmdTiles(int argc, const char **argv) {
	Graphics::ManagedSurface surface(Gfx::SHAPE_SIZE, Gfx::SHAPE_SIZE, Graphics::PixelFormat::createFormatCLUT8());

	_G(shapes).load();
	debugPrintf("Loaded %d tiles\n", Gfx::SHAPE_COUNT);

	for (int t = 0; t < Gfx::SHAPE_COUNT; ++t) {
		_G(shapes).drawTile(surface, 0, 0, t);

		uint32 sum = 0;
		const byte *pixels = (const byte *)surface.getPixels();
		for (int i = 0; i < Gfx::SHAPE_SIZE * Gfx::SHAPE_SIZE; ++i)
			sum += pixels[i];

		debugPrintf("Tile %2d: pixel sum=%u\n", t, sum);
	}

	// The town tile has a distinctive shape, useful as a visual sanity check
	// that the graphics have been decoded correctly
	const int sanityTile = 6;
	debugPrintf("Tile %d (Towne):\n", sanityTile);
	_G(shapes).drawTile(surface, 0, 0, sanityTile);
	const byte *pixels = (const byte *)surface.getPixels();

	for (int y = 0; y < Gfx::SHAPE_SIZE; ++y) {
		Common::String line;
		for (int x = 0; x < Gfx::SHAPE_SIZE; ++x)
			line += (char)('0' + pixels[y * Gfx::SHAPE_SIZE + x]);
		debugPrintf("%s\n", line.c_str());
	}

	return true;
}

bool Console::cmdTeleport(int argc, const char **argv) {
	Data::Savegame &sg = _G(savegame);
	const bool inDungeon = sg._location == Data::LOCATION_DUNGEON;

	if (argc == 3 || (argc == 4 && inDungeon)) {
		const int mask = inDungeon ? Data::DUNGEON_SIZE - 1 : Data::MAP_SIZE - 1;
		sg._posX = atoi(argv[1]) & mask;
		sg._posY = atoi(argv[2]) & mask;

		if (argc == 4)
			sg._dungeonLevel = CLIP(atoi(argv[3]), 0, Data::DUNGEON_LEVELS - 1);

		g_engine->focusedView()->redraw();
	} else if (argc != 1) {
		debugPrintf("teleport [<x> <y>%s]\n", inDungeon ? " [<level>]" : "");
		return true;
	}

	if (inDungeon)
		debugPrintf("Position: (%d,%d) on level %d\n", sg._posX, sg._posY, sg._dungeonLevel);
	else
		debugPrintf("Position: (%d,%d)\n", sg._posX, sg._posY);
	return true;
}

bool Console::cmdLocations(int argc, const char **argv) {
	if (argc > 2) {
		debugPrintf("locations [<index>]\n");
		return true;
	}

	if (argc == 2) {
		Data::Savegame &sg = _G(savegame);
		int index = atoi(argv[1]);

		if (index < 0 || index >= LocationLogic::entranceCount()) {
			debugPrintf("Invalid location index\n");
			return true;
		}
		if (sg._location != Data::LOCATION_SOSARIA) {
			debugPrintf("Only supported on the world map\n");
			return true;
		}

		Common::Point pos = LocationLogic::entrancePosition(index);
		sg._posX = pos.x;
		sg._posY = pos.y;
		g_engine->focusedView()->redraw();
		debugPrintf("Teleported to %d: (%d,%d)\n", index, pos.x, pos.y);
		return true;
	}

	for (int i = 0; i < LocationLogic::entranceCount(); ++i) {
		Common::String name(LocationLogic::entranceFilename(i));
		name.erase(name.size() - 4);
		Common::Point pos = LocationLogic::entrancePosition(i);

		debugPrintf("%2d: %-10s (%d,%d)\n", i, name.c_str(), pos.x, pos.y);
	}

	return true;
}

bool Console::cmdIntangible(int argc, const char **argv) {
	_G(intangible) = !_G(intangible);
	debugPrintf("Intangible mode %s\n", _G(intangible) ? "ON" : "OFF");
	return true;
}

bool Console::cmdEnemy(int argc, const char **argv) {
	Data::Savegame &sg = _G(savegame);
	if (sg._location != Data::LOCATION_SOSARIA && sg._location != Data::LOCATION_TOWN &&
			sg._location != Data::LOCATION_CASTLE) {
		debugPrintf("Only supported on the world map and in towns and castles\n");
		return true;
	}
	if (argc > 2) {
		debugPrintf("enemy [<tile>]  - tile number of the creature, defaulting to an orc\n");
		return true;
	}

	byte tile = (argc == 2) ? atoi(argv[1]) & 0x3F : DEFAULT_ENEMY;
	Data::Creatures &c = _G(map)._creatures;

	int slot = -1;
	for (int i = 0; i < Data::CREATURE_COUNT && slot < 0; ++i) {
		if (!c._tile[i])
			slot = i;
	}
	if (slot < 0) {
		debugPrintf("No free creature slots\n");
		return true;
	}

	// Things of the sea need water, and the rest need open ground
	const bool sea = tile >= 0x0B && tile < 0x10;
	int spawnX = -1, spawnY = -1;

	for (int radius = 1; radius <= 10 && spawnX < 0; ++radius) {
		for (int dy = -radius; dy <= radius && spawnX < 0; ++dy) {
			for (int dx = -radius; dx <= radius && spawnX < 0; ++dx) {
				if (MAX(ABS(dx), ABS(dy)) != radius)
					continue;

				int x = (sg._posX + dx) & (Data::MAP_SIZE - 1);
				int y = (sg._posY + dy) & (Data::MAP_SIZE - 1);
				byte cell = _G(map).cell(x, y);
				bool fits = sea ? cell == 0 : (cell == 4 || cell == 8 || cell == 0x0C || cell == 0x20);
				if (!fits)
					continue;

				bool occupied = false;
				for (int i = 0; i < Data::CREATURE_COUNT; ++i)
					occupied |= c._tile[i] && c._x[i] == x && c._y[i] == y;

				if (!occupied) {
					spawnX = x;
					spawnY = y;
				}
			}
		}
	}

	if (spawnX < 0) {
		debugPrintf("Couldn't find a free place nearby\n");
		return true;
	}

	c._tile[slot] = tile * 4;
	c._floor[slot] = _G(map).cell(spawnX, spawnY);
	c._x[slot] = spawnX;
	c._y[slot] = spawnY;
	c._flags[slot] = 0xC0;
	_G(map).setCell(spawnX, spawnY, tile * 4);

	g_engine->focusedView()->redraw();
	debugPrintf("Spawned %s in slot %d at (%d,%d)\n", Data::NAMES[tile], slot, spawnX, spawnY);
	return true;
}

bool Console::cmdHP(int argc, const char **argv) {
	int first, last;
	if (argc > 2 || !getTargets(argc, argv, 1, first, last)) {
		debugPrintf("hp [<position>]\n");
		return true;
	}

	// Anyone who has fallen is raised too
	for (int i = first; i <= last; ++i) {
		Data::RosterEntry &e = _G(savegame).partyMember(i);
		e._hitPoints = e._maxHitPoints = Data::toBcdWord(MAX_VALUE);
		e._status = Data::STATUS_GOOD;
	}

	g_engine->focusedView()->redraw();
	debugPrintf("HP set to %d\n", MAX_VALUE);
	return true;
}

bool Console::cmdFood(int argc, const char **argv) {
	int first, last;
	if (argc > 2 || !getTargets(argc, argv, 1, first, last)) {
		debugPrintf("food [<position>]\n");
		return true;
	}

	for (int i = first; i <= last; ++i) {
		Data::RosterEntry &e = _G(savegame).partyMember(i);
		e._food = Data::toBcdWord(MAX_VALUE);
		e._foodSubCounter = 0;
	}

	g_engine->focusedView()->redraw();
	debugPrintf("Food set to %d\n", MAX_VALUE);
	return true;
}

bool Console::cmdGold(int argc, const char **argv) {
	int first, last;
	if (argc > 2 || !getTargets(argc, argv, 1, first, last)) {
		debugPrintf("gold [<position>]\n");
		return true;
	}

	for (int i = first; i <= last; ++i)
		_G(savegame).partyMember(i)._gold = Data::toBcdWord(MAX_VALUE);

	g_engine->focusedView()->redraw();
	debugPrintf("Gold set to %d\n", MAX_VALUE);
	return true;
}

bool Console::cmdInventory(int argc, const char **argv) {
	int first, last;
	if (argc > 2 || !getTargets(argc, argv, 1, first, last)) {
		debugPrintf("inventory [<position>]\n");
		return true;
	}

	Data::Savegame &sg = _G(savegame);
	sg._allWeapons = sg._allArmour = sg._plusTwoWeapons = sg._plusTwoArmour = true;

	for (int i = first; i <= last; ++i) {
		Data::RosterEntry &e = sg.partyMember(i);

		for (int w = 0; w < Data::WEAPON_COUNT - 1; ++w)
			e._weaponOwned[w] = Data::toBcd(MAX_ITEMS);
		for (int a = 0; a < Data::ARMOUR_COUNT - 1; ++a)
			e._armourOwned[a] = Data::toBcd(MAX_ITEMS);

		e._torches = e._keys = e._powder = e._gems = e._magicPoints = Data::toBcd(MAX_ITEMS);
		e._marksAndCards = MARKS_AND_CARDS;
		e._hitPoints = e._maxHitPoints = e._food = e._gold = Data::toBcdWord(MAX_VALUE);
		e._foodSubCounter = 0;
		e._status = Data::STATUS_GOOD;

		// Ready the best that the character's class is allowed to, which
		// leaves out the exotic weapon and armour that anyone can use
		e._weaponIndex = 0;
		for (int w = Data::WEAPON_COUNT - 2; w > 0; --w) {
			if ('A' + w < Data::weaponLimit(e._class)) {
				e._weaponIndex = w;
				break;
			}
		}

		e._armourIndex = 0;
		for (int a = Data::ARMOUR_COUNT - 2; a > 0; --a) {
			if ('A' + a < Data::armourLimit(e._class)) {
				e._armourIndex = a;
				break;
			}
		}

		debugPrintf("%s: readied %s and %s armour\n", e._name, Data::WEAPON_NAMES[e._weaponIndex],
			Data::ARMOUR_NAMES[e._armourIndex]);
	}

	g_engine->focusedView()->redraw();
	debugPrintf("Full inventory given\n");
	return true;
}

// Where the moons are kept in the world file, after the whirlpool
constexpr int MOON_PHASES = 4;
constexpr int MOON_COUNTDOWNS = 6;

// Opens a file of a saved game, from a folder if given or else from the game's own
static Common::SeekableReadStream *openSaved(const Common::String &folder, const char *name) {
	if (!folder.empty())
		return Common::FSNode(Common::Path(folder)).getChild(name).createReadStream();

	Common::File *f = new Common::File();
	if (!f->open(name)) {
		delete f;
		return nullptr;
	}

	return f;
}

static Common::WriteStream *createSaved(const Common::String &folder, const char *name) {
	if (!folder.empty())
		return Common::FSNode(Common::Path(folder)).getChild(name).createWriteStream();

	Common::DumpFile *f = new Common::DumpFile();
	if (!f->open(Common::Path(name), true)) {
		delete f;
		return nullptr;
	}

	return f;
}

bool Console::cmdLoad(int argc, const char **argv) {
	if (argc > 2) {
		debugPrintf("load [<folder>]  - imports the original DOS save files ROSTER.ULT, PARTY.ULT "
			"and SOSARIA.ULT from a folder, defaulting to the game's own\n");
		return true;
	}

	Common::String folder = (argc == 2) ? argv[1] : "";
	Common::ScopedPtr<Common::SeekableReadStream> roster(openSaved(folder, ROSTER_FILE));
	Common::ScopedPtr<Common::SeekableReadStream> party(openSaved(folder, PARTY_FILE));
	if (!roster || !party) {
		debugPrintf("Could not open %s and %s\n", ROSTER_FILE, PARTY_FILE);
		return true;
	}

	Data::Savegame &sg = _G(savegame);
	if (!sg.importOriginal(*roster, *party)) {
		debugPrintf("%s and %s are too short to be save files\n", ROSTER_FILE, PARTY_FILE);
		return true;
	}

	// The world they were saved in comes with the party, including where the moons are
	Common::ScopedPtr<Common::SeekableReadStream> world(openSaved(folder, WORLD_FILE));
	if (world) {
		if (!_G(map).load(*world)) {
			debugPrintf("%s is too short to be a map\n", WORLD_FILE);
			return true;
		}

		for (int i = 0; i < 2; ++i) {
			sg._moonPhase[i] = _G(map).extra(MOON_PHASES + i);
			sg._moonCountdown[i] = _G(map).extra(MOON_COUNTDOWNS + i);
		}

		sg._mapLoaded = true;
	} else {
		ensureGame();
	}

	debugPrintf("Imported a party of %d at (%d,%d)\n", sg._partySize, sg._posX, sg._posY);
	if (sg._partySize == 0)
		return true;

	restartGame();
	return false;
}

bool Console::cmdSave(int argc, const char **argv) {
	if (argc > 2) {
		debugPrintf("save [<folder>]  - exports the original DOS save files ROSTER.ULT, PARTY.ULT "
			"and SOSARIA.ULT to a folder, defaulting to the dumps folder\n");
		return true;
	}

	Data::Savegame &sg = _G(savegame);
	if (sg._partySize == 0) {
		debugPrintf("There is no party to save\n");
		return true;
	}

	Common::String folder = (argc == 2) ? argv[1] : "";
	Common::ScopedPtr<Common::WriteStream> roster(createSaved(folder, ROSTER_FILE));
	Common::ScopedPtr<Common::WriteStream> party(createSaved(folder, PARTY_FILE));
	Common::ScopedPtr<Common::WriteStream> world(createSaved(folder, WORLD_FILE));
	if (!roster || !party || !world) {
		debugPrintf("Could not create the save files\n");
		return true;
	}

	sg.exportOriginal(*roster, *party);

	// The world is the one the party came from if they're somewhere else
	Data::Map map = (sg._location == Data::LOCATION_SOSARIA) ? _G(map) : _G(worldMap);
	for (int i = 0; i < 2; ++i) {
		map.extra(MOON_PHASES + i) = sg._moonPhase[i];
		map.extra(MOON_COUNTDOWNS + i) = sg._moonCountdown[i];
	}
	map.save(*world);

	debugPrintf("Exported %s, %s and %s\n", ROSTER_FILE, PARTY_FILE, WORLD_FILE);
	return true;
}

} // namespace Ultima3
} // namespace Ultima
