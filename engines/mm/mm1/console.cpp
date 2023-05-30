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
#include "common/savefile.h"
#include "common/system.h"
#include "mm/shared/utils/strings.h"
#include "mm/mm1/console.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/events.h"
#include "mm/mm1/game/spells_party.h"

namespace MM {
namespace MM1 {

Console::Console() : GUI::Debugger() {
	registerCmd("dump_map", WRAP_METHOD(Console, cmdDumpMap));
	registerCmd("dump_monsters", WRAP_METHOD(Console, cmdDumpMonsters));
	registerCmd("dump_items", WRAP_METHOD(Console, cmdDumpItems));
	registerCmd("map_string", WRAP_METHOD(Console, cmdMapString));
	registerCmd("map", WRAP_METHOD(Console, cmdMap));
	registerCmd("pos", WRAP_METHOD(Console, cmdPos));
	registerCmd("intangible", WRAP_METHOD(Console, cmdIntangible));
	registerCmd("cast", WRAP_METHOD(Console, cmdCast));
	registerCmd("spells", WRAP_METHOD(Console, cmdSpellsAll));
	registerCmd("encounter", WRAP_METHOD(Console, cmdEncounter));
	registerCmd("encounters", WRAP_METHOD(Console, cmdEncounters));
	registerCmd("specials", WRAP_METHOD(Console, cmdSpecials));
	registerCmd("special", WRAP_METHOD(Console, cmdSpecial));
	registerCmd("view", WRAP_METHOD(Console, cmdView));
}

bool Console::cmdDumpMap(int argc, const char **argv) {
	Common::File f;
	Common::OutSaveFile *dest;

	if (argc != 2) {
		debugPrintf("%s <roomNum>\n", argv[0]);
	} else {
		int mapId = strToInt(argv[1]);
		Maps::Map *map = g_globals->_maps.getMap(mapId);

		// Dump the map data
		if (f.open("mazedata.dta")) {
			dest = g_system->getSavefileManager()->openForSaving(
				Common::String::format("map%.2d-maze.bin", mapId), false);
			assert(dest);
			byte buffer[512];

			f.seek(mapId * 512);
			f.read(buffer, 512);
			dest->write(buffer, 512);

			dest->finalize();
			delete dest;
			f.close();
		}

		if (f.open(Common::String::format("%s.ovr", map->getName().c_str()))) {
			int magicId = f.readUint16LE();
			int codePtr = f.readUint16LE();
			int codeSize = f.readUint16LE();
			int dataPtr = f.readUint16LE();
			int dataSize = f.readUint16LE();
			int extrasSize = f.readUint16LE();
			int startPtr = f.readUint16LE();

			assert(magicId == 0xf2);
			assert(startPtr >= codePtr &&
				startPtr < (codePtr + codeSize));

			dest = g_system->getSavefileManager()->openForSaving(
				Common::String::format("map%.2d-code.bin", mapId), false);
			byte *code = new byte[codeSize];
			f.read(code, codeSize);

			for (int i = 0; i < (codePtr % 16); ++i)
				dest->writeByte(0);
			dest->write(code, codeSize);
			dest->flush();
			delete dest;
			delete[] code;

			dest = g_system->getSavefileManager()->openForSaving(
				Common::String::format("map%.2d-data.bin", mapId), false);
			byte *data = new byte[dataSize];
			f.read(data, dataSize);
			dest->write(data, dataSize);
			dest->flush();
			delete dest;
			delete[] data;

			f.close();

			debugPrintf("data: ptr=%xh, size=%xh\n", dataPtr, dataSize);
			debugPrintf("code: ptr=%xh, size=%xh start=%xh\n",
				codePtr, codeSize, startPtr);
			debugPrintf("Extras size=%xh\n", extrasSize);
		}

		debugPrintf("Done.\n");
	}

	return true;
}

bool Console::cmdDumpMonsters(int argc, const char **argv) {
	Common::File f;
	Common::DumpFile df;
	if (f.open("mm.exe")) {
		if (df.open("monsters.txt")) {
			f.seek(0x1b312);

			for (int i = 0; i < 195; ++i) {
				Common::String line = "\"";
				for (int j = 0; j < 15; ++j)
					line += f.readByte();
				line += '"';

				for (int j = 0; j < 16; ++j) {
					line += ", ";
					int val = (j == 7) ? f.readUint16LE() : f.readByte();
					line += Common::String::format("%d", val);
				}

				df.writeString(line);
				df.writeByte('\n');
			}

			df.close();
			f.close();
			debugPrintf("Done\n");
			return true;
		}
	}

	debugPrintf("Could not create\n");
	return true;
}

bool Console::cmdDumpItems(int argc, const char **argv) {
	Common::File f;
	Common::DumpFile df;
	Common::String line;

	if (f.open("mm.exe")) {
		if (df.open("items.txt")) {
			f.seek(0x19b2a);

			for (int i = 0; i < 255; ++i) {
				if (i == 85) {
					// Add the blank unused item line
					line = "\"              \", 0, 0, 0, 0, 0, 0, 0, 0, 0";
					df.writeString(line);
					df.writeByte('\n');
				}

				line = "\"";
				for (int j = 0; j < 14; ++j)
					line += f.readByte();
				line += '"';

				for (int j = 0; j < 9; ++j) {
					line += ", ";
					line += Common::String::format("%d",
						(j == 6) ? f.readUint16BE() : f.readByte());
				}

				df.writeString(line);
				df.writeByte('\n');
			}

			df.close();
			f.close();
			debugPrintf("Done\n");
			return true;
		}
	}

	debugPrintf("Could not create\n");
	return true;
}

bool Console::cmdMapString(int argc, const char **argv) {
	Common::File f;

	if (argc != 3) {
		debugPrintf("%s <map Id> <offset>\n", argv[0]);
	} else {
		int mapId = strToInt(argv[1]);
		Maps::Map *map = g_globals->_maps.getMap(mapId);
		int offset = strToInt(Common::String::format(
			"%sh", argv[2]).c_str());

		if (!f.open(Common::String::format("%s.ovr", map->getName().c_str())))
			error("Failed to open map");

		f.readUint16LE();
		f.readUint16LE();
		int codeSize = f.readUint16LE();
		int dataPtr = f.readUint16LE();
		f.readUint16LE();
		f.readUint16LE();
		f.readUint16LE();

		f.skip(codeSize);
		f.skip(offset - dataPtr);

		// Read the string
		Common::String s;
		char c;
		
		while ((c = f.readByte()) != '\0') {
			if (c == '\r') {
				s += "\\n";
				debugPrintf("%s\n", s.c_str());
				s = "";
			} else {
				s += c;

				if (s.size() == 40) {
					debugPrintf("%s\n", s.c_str());
					s = "";
				}
			}
		}
		debugPrintf("%s\n", s.c_str());

		f.close();
	}

	return true;
}

bool Console::cmdMap(int argc, const char **argv) {
	Maps::Maps &maps = g_globals->_maps;

	if (argc < 2) {
		debugPrintf("map <mapId section> | <mapNum [ xp, yp ]>\n");
		return true;

	} else if (argc == 3) {
		int mapId = strToInt(argv[1]);
		int section = strToInt(argv[2]);
		maps.changeMap(mapId, section);

	} else {
		int mapNum = strToInt(argv[1]);
		Maps::Map &map = *maps.getMap(mapNum);
		int x = g_maps->_mapPos.x, y = g_maps->_mapPos.y;

		if (argc == 4) {
			x = strToInt(argv[2]);
			y = strToInt(argv[3]);
		}

		maps._mapPos.x = x;
		maps._mapPos.y = y;
		maps.changeMap(map.getId(), map.getDefaultSection());
	}

	return false;
}

bool Console::cmdPos(int argc, const char **argv) {
	if (argc < 3) {
		debugPrintf("pos xp, yp\n");
		return true;
	} else {
		Maps::Maps &maps = g_globals->_maps;
		maps._mapPos.x = strToInt(argv[1]);
		maps._mapPos.y = strToInt(argv[2]);
		g_events->send("Game", GameMessage("UPDATE"));

		return false;
	}
}

bool Console::cmdIntangible(int argc, const char **argv) {
	g_globals->_intangible = (argc < 2) || strcmp(argv[1], "off");
	debugPrintf("Intangibility is %s\n", g_globals->_intangible ? "on" : "off");
	return true;
}

bool Console::cmdCast(int argc, const char **argv) {
	if (argc != 3) {
		debugPrintf("%s <level> <number>\n", argv[0]);
		return true;
	} else {
		Character *c = g_globals->_currCharacter;
		if (!c || (c->_class != CLERIC && c->_class != SORCERER
				&& c->_class != ARCHER)) {
			uint i;
			for (i = 0; i < g_globals->_party.size(); ++i) {
				if (g_globals->_party[i]._class == SORCERER) {
					c = &g_globals->_party[i];
					break;
				}
			}
			if (i == g_globals->_party.size()) {
				debugPrintf("Could not find sorcerer in party\n");
				return true;
			}
		}

		int spellIndex = getSpellIndex(c, strToInt(argv[1]), strToInt(argv[2]));
		setSpell(spellIndex, 0, 0);
		Game::SpellsParty::cast(_spellIndex, c);
		return false;
	}
}

bool Console::cmdSpellsAll(int argc, const char **argv) {
	g_globals->_allSpells = (argc == 2) && !strcmp(argv[1], "on");
	debugPrintf("All spells is %s\n",
		g_globals->_allSpells ? "enabled" : "disabled");
	return true;
}

bool Console::cmdEncounter(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("encounter <monster-num> [<level>, <# monsters>, <encounter type>]\n");
		return true;

	} else {
		Game::Encounter &enc = g_globals->_encounters;
		int monsterNum = strToInt(argv[1]);
		if (monsterNum < 1 || monsterNum > MONSTERS_COUNT) {
			debugPrintf("monster-num must be between 1 and %d\n", MONSTERS_COUNT);
			return true;
		}
		int level = (argc == 3) ? strToInt(argv[2]) : 1;
		if (level < 1 || level >= 15)
			level = 1;

		int count = (argc > 3) ? strToInt(argv[3]) : 1;
		int encType = (argc > 4) ? strToInt(argv[2]) :
			Game::NORMAL_ENCOUNTER;
		if (encType != -1 || encType == 0 || encType == 1)
			enc._encounterType = (Game::EncounterType)encType;

		enc.clearMonsters();
		for (int i = 0; i < count; ++i)
			enc.addMonster(monsterNum, level);

		enc._manual = true;
		enc._levelIndex = 80;

		bool monstersOn = g_globals->_encountersOn;
		g_globals->_encountersOn = true;
		enc.execute();
		g_globals->_encountersOn = monstersOn;

		return false;
	}
}

bool Console::cmdEncounters(int argc, const char **argv) {
	if (argc == 1) {
		g_globals->_encountersOn = !g_globals->_encountersOn;
	} else {
		g_globals->_encountersOn = !scumm_stricmp(argv[1], "on");
	}

	debugPrintf("Encounters are %s\n",
		g_globals->_encountersOn ? "on" : "off");
	return true;
}

bool Console::cmdSpecials(int argc, const char **argv) {
	int count = g_maps->_currentMap->dataByte(Maps::MAP_SPECIAL_COUNT);

	// List specials that have code attached
	for (int i = 0; i < count; ++i) {
		int mapOffset = g_maps->_currentMap->dataByte(51 + i);
		int x = mapOffset % MAP_W;
		int y = mapOffset / MAP_W;
		Common::String line = Common::String::format(
			"Special #%.2d - %d, %d (", i, x, y);

		int dirMask = g_maps->_currentMap->dataByte(51 + i);
		if (dirMask & Maps::DIRMASK_N)
			line += "N,";
		if (dirMask & Maps::DIRMASK_S)
			line += "S,";
		if (dirMask & Maps::DIRMASK_E)
			line += "E,";
		if (dirMask & Maps::DIRMASK_W)
			line += "W,";

		line.deleteLastChar();
		line += ')';
		debugPrintf("%s\n", line.c_str());
	}

	// Iterate through the map to find special cells that are codeless
	int mapOffset = 0, i;
	for (int mapPos = 0; mapPos < (MAP_W * MAP_H); ++mapPos) {
		bool isSpecial = (g_maps->_currentMap->_states[mapPos]
			& Maps::CELL_SPECIAL) != 0;
		if (!isSpecial)
			continue;

		int x = mapPos % MAP_W;
		int y = mapPos / MAP_W;

		for (i = 0; i < count; ++i) {
			mapOffset = g_maps->_currentMap->dataByte(51 + i);
			if (mapOffset == mapPos)
				break;
		}

		// Add row for special if there's no code handling
		if (i == count) {
			Common::String line = Common::String::format(
				"Special #-- - %d, %d", x, y);
			debugPrintf("%s\n", line.c_str());
		}
	}

	return true;
}

bool Console::cmdSpecial(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("special <num> to execute special in the current map\n");
		return true;
	}

	Maps::Maps &maps = *g_maps;
	Maps::Map &map = *g_maps->_currentMap;
	uint count = g_maps->_currentMap->dataByte(Maps::MAP_SPECIAL_COUNT);
	uint specialNum = strToInt(argv[1]);

	if (specialNum > count) {
		debugPrintf("Invalid special number\n");
		return true;
	}

	// Set new position
	maps._mapOffset = map[51 + specialNum];
	maps._mapPos.x = maps._mapOffset % 16;
	maps._mapPos.y = maps._mapOffset / 16;

	// Rotate to find a direction that will trigger the special
	for (int i = 0; i < 4; ++i) {
		if (maps._forwardMask & map[51 + count + specialNum])
			break;
		maps.turnLeft();
	}

	// Execute the specials handler for the map
	map.special();
	return false;
}

bool Console::cmdView(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("view <view name>\n");
		return true;
	} else {
		g_events->addView(argv[1]);
		return false;
	}
}

} // namespace MM1
} // namespace MM
