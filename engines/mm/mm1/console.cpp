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
#include "common/util.h"
#include "mm/shared/utils/strings.h"
#include "mm/mm1/console.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/events.h"
#include "mm/mm1/game/spells_party.h"
#include "mm/mm1/messages.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {

namespace {

const char *const SOUND_TESTS[] = {
	"wall",
	"message",
	"triple",
	"encounter",
	"combat",
	"search",
	"cast",
	"death"
};

const SoundId SOUND_TEST_SOUND_IDS[] = {
	SOUND_1, SOUND_2, SOUND_3
};

const SoundId SOUND_TEST_SOUND2_IDS[] = {
	SOUND_TITLE, SOUND_1, SOUND_2, SOUND_3, SOUND_4, SOUND_5, SOUND_8, SOUND_9
};

class SoundTestRunner;
SoundTestRunner *g_soundTestRunner = nullptr;

void faceDirection(Maps::DirMask dir) {
	Maps::Maps &maps = g_globals->_maps;

	for (int i = 0; i < 4 && maps._forwardMask != dir; ++i)
		maps.turnLeft();
}

bool stageWallBump() {
	Maps::Maps &maps = g_globals->_maps;
	maps.loadTown(Maps::SORPIGAL);
	g_globals->_intangible = false;

	static const Maps::DirMask DIRS[] = {
		Maps::DIRMASK_N, Maps::DIRMASK_E, Maps::DIRMASK_S, Maps::DIRMASK_W
	};

	for (int y = 0; y < MAP_H; ++y) {
		for (int x = 0; x < MAP_W; ++x) {
			const uint offset = y * MAP_W + x;
			if (maps._currentMap->_states[offset] & Maps::CELL_SPECIAL)
				continue;

			for (uint i = 0; i < ARRAYSIZE(DIRS); ++i) {
				const Maps::DirMask dir = DIRS[i];
				if (!(maps._currentMap->_walls[offset] & dir) ||
						!(maps._currentMap->_states[offset] & 0x55 & dir))
					continue;

				maps._mapPos = Common::Point(x, y);
				faceDirection(dir);
				maps._mapOffset = offset;
				maps._currentWalls = maps._currentMap->_walls[offset];
				maps._currentState = maps._currentMap->_states[offset];
				g_events->send("View", ActionMessage(KEYBIND_FORWARDS));
				return true;
			}
		}
	}

	return false;
}

Character *findSpellTester() {
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		if (c._class == CLERIC || c._class == SORCERER || c._class == ARCHER)
			return &c;
	}

	return g_globals->_party.empty() ? nullptr : &g_globals->_party[0];
}

void stageSpellDone() {
	Character *c = findSpellTester();
	if (!c)
		return;

	g_globals->_currCharacter = c;
	c->_class = SORCERER;
	c->_spellLevel = 7;
	c->_sp = 100;
	c->_gems = 100;

	int spellIndex = Console::getSpellIndex(c, 1, 5);
	g_events->send("CastSpell", GameMessage("SPELL", spellIndex));
}

void stageSearchTreasure() {
	g_globals->_treasure.clear();
	g_globals->_treasure._container = CLOTH_SACK;
	g_globals->_treasure._trapType = 2;
	g_globals->_treasure.setGold(6);
	g_globals->_treasure.setGems(0);
	g_events->send("Search", GameMessage("SHOW"));
}

void stageEncounter() {
	if (g_globals->_party.empty()) {
		warning("MM1 sound test: encounter requires an active party");
		return;
	}

	Game::Encounter &enc = g_globals->_encounters;
	const bool encountersOn = g_globals->_encountersOn;
	g_globals->_encountersOn = true;

	enc.clearMonsters();
	enc.addMonster(1, 1);
	enc._manual = true;
	enc._levelIndex = 80;
	enc._encounterType = Game::FORCE_SURPRISED;
	enc.execute();

	g_globals->_encountersOn = encountersOn;
}

void stageTripleBeepSpecial() {
	g_events->send("GameMessages", InfoMessage(
		0, 1, STRING["maps.map13.snake_pit"],
		0, 2, STRING["maps.map13.levitation2"]
	));
	Sound::sound(SOUND_3);
}

void stageCombatKill() {
	Character *c = g_globals->_party.empty() ? nullptr : &g_globals->_party[0];
	if (!c)
		return;

	c->_class = KNIGHT;
	c->_condition = FINE;
	c->_level = 50;
	c->_hp = c->_hpCurrent = c->_hpMax = 200;
	c->_might = 50;
	c->_accuracy = 50;
	c->_speed = 50;
	c->_physicalAttr = 50;
	g_globals->_currCharacter = c;

	stageEncounter();
	g_events->send("Combat", GameMessage("COMBAT"));
}

bool runSoundTestCase(const Common::String &testName) {
	if (!scumm_stricmp(testName.c_str(), "wall")) {
		return stageWallBump();

	} else if (!scumm_stricmp(testName.c_str(), "message")) {
		g_events->send("GameMessages", SoundMessage(STRING["dialogs.search.nothing"]));
		return true;

	} else if (!scumm_stricmp(testName.c_str(), "triple")) {
		stageTripleBeepSpecial();
		return true;

	} else if (!scumm_stricmp(testName.c_str(), "encounter")) {
		stageEncounter();
		return true;

	} else if (!scumm_stricmp(testName.c_str(), "combat")) {
		stageCombatKill();
		return true;

	} else if (!scumm_stricmp(testName.c_str(), "search")) {
		stageSearchTreasure();
		return true;

	} else if (!scumm_stricmp(testName.c_str(), "death") ||
			!scumm_stricmp(testName.c_str(), "dead")) {
		g_events->addView("Dead");
		return true;

	} else if (!scumm_stricmp(testName.c_str(), "cast")) {
		stageSpellDone();
		return true;
	}

	return false;
}

bool parseSoundId(const char *str, SoundId &soundId) {
	char *end = nullptr;
	const long value = strtol(str, &end, 10);

	if (!str[0] || *end)
		return false;

	switch (value) {
	case SOUND_TITLE:
	case SOUND_1:
	case SOUND_2:
	case SOUND_3:
	case SOUND_4:
	case SOUND_5:
	case SOUND_8:
	case SOUND_9:
		soundId = (SoundId)value;
		return true;
	default:
		return false;
	}
}

bool isSoundIdSupported(const SoundId *ids, uint count, SoundId soundId) {
	for (uint i = 0; i < count; ++i) {
		if (ids[i] == soundId)
			return true;
	}

	return false;
}

bool runDirectSoundTest(const Common::String &routine, SoundId soundId) {
	if (!scumm_stricmp(routine.c_str(), "sound")) {
		if (!isSoundIdSupported(SOUND_TEST_SOUND_IDS, ARRAYSIZE(SOUND_TEST_SOUND_IDS), soundId))
			return false;

		Sound::sound(soundId);
		return true;
	}

	if (!scumm_stricmp(routine.c_str(), "sound2")) {
		if (!isSoundIdSupported(SOUND_TEST_SOUND2_IDS, ARRAYSIZE(SOUND_TEST_SOUND2_IDS), soundId))
			return false;

		Sound::sound2(soundId);
		return true;
	}

	return false;
}

bool runDirectSoundTestAlias(const Common::String &testName) {
	const char *name = testName.c_str();
	SoundId soundId;

	if (!strncmp(name, "sound2-", 7) && parseSoundId(name + 7, soundId))
		return runDirectSoundTest("sound2", soundId);

	if (!strncmp(name, "sound-", 6) && parseSoundId(name + 6, soundId))
		return runDirectSoundTest("sound", soundId);

	return false;
}

void soundTestPromptAccepted();

class SoundTestRunner : public UIElement {
private:
	enum State {
		ST_INACTIVE,
		ST_PROMPT,
		ST_RUN,
		ST_WAIT
	};

	State _state = ST_INACTIVE;
	uint _testIndex = 0;
	uint32 _waitUntil = 0;
	bool _promptOpen = false;
	bool _promptAccepted = false;

	void nextTest() {
		++_testIndex;
		_promptOpen = false;
		_promptAccepted = false;

		if (_testIndex >= ARRAYSIZE(SOUND_TESTS)) {
			_state = ST_INACTIVE;
			close();
		} else {
			_state = ST_PROMPT;
		}
	}

	uint32 waitMillisForTest(const Common::String &testName) const {
		if (!scumm_stricmp(testName.c_str(), "encounter"))
			return 5000;
		if (!scumm_stricmp(testName.c_str(), "combat"))
			return 8000;
		if (!scumm_stricmp(testName.c_str(), "cast"))
			return 3000;
		if (!scumm_stricmp(testName.c_str(), "search"))
			return 3000;
		return 2000;
	}

public:
	SoundTestRunner(UIElement *owner) : UIElement("SoundTestRunner", owner) {}

	void start() {
		_state = ST_PROMPT;
		_testIndex = 0;
		_promptOpen = false;
		_promptAccepted = false;
		_waitUntil = 0;
	}

	void promptAccepted() {
		_promptAccepted = true;
	}

	bool tick() override {
		if (_state == ST_INACTIVE)
			return false;

		const Common::String testName = SOUND_TESTS[_testIndex];

		switch (_state) {
		case ST_PROMPT:
			if (!_promptOpen) {
				_promptOpen = true;
				g_events->send("GameMessages", InfoMessage(
					0, 3,
					Common::String::format("proceed with %s test", testName.c_str()),
					soundTestPromptAccepted));
				return true;
			}

			if (_promptAccepted)
				_state = ST_RUN;
			return true;

		case ST_RUN:
			runSoundTestCase(testName);
			_waitUntil = g_system->getMillis() + waitMillisForTest(testName);
			_state = ST_WAIT;
			return true;

		case ST_WAIT:
			if (g_system->getMillis() >= _waitUntil)
				nextTest();
			return true;

		default:
			break;
		}

		return false;
	}
};

void soundTestPromptAccepted() {
	if (g_soundTestRunner)
		g_soundTestRunner->promptAccepted();
}

} // namespace

Console::Console() : GUI::Debugger() {
	registerCmd("dump_map", WRAP_METHOD(Console, cmdDumpMap));
	registerCmd("dump_monsters", WRAP_METHOD(Console, cmdDumpMonsters));
	registerCmd("dump_items", WRAP_METHOD(Console, cmdDumpItems));
	registerCmd("dump_roster", WRAP_METHOD(Console, cmdDumpRoster));
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
	registerCmd("sound", WRAP_METHOD(Console, cmdSoundTest));
}

Console::~Console() {
	if (g_soundTestRunner == _soundTestRunner)
		g_soundTestRunner = nullptr;
	delete _soundTestRunner;
}

void Console::postEnter() {
	GUI::Debugger::postEnter();

	if (_pendingSoundTest) {
		_pendingSoundTest = false;
		if (!_soundTestRunner) {
			UIElement *game = g_events->findView("Game");
			assert(game);
			_soundTestRunner = new SoundTestRunner(game);
			g_soundTestRunner = static_cast<SoundTestRunner *>(_soundTestRunner);
		}
		static_cast<SoundTestRunner *>(_soundTestRunner)->start();
	}
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

		if (f.open(Common::Path(Common::String::format("%s.ovr", map->getName().c_str())))) {
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

bool Console::cmdDumpRoster(int argc, const char **argv) {
	g_globals->_roster.saveOriginal();
	debugPrintf("Dumped roster\n");
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

		if (!f.open(Common::Path(Common::String::format("%s.ovr", map->getName().c_str()))))
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

bool Console::cmdSoundTest(int argc, const char **argv) {
	Common::String testName;
	SoundId soundId;

	if (argc > 1 && !scumm_stricmp(argv[1], "list")) {
		debugPrintf("sound [test|list|wall|message|triple|encounter|combat|search|cast|death]\n");
		debugPrintf("sound sound <1|2|3>\n");
		debugPrintf("sound sound2 <0|1|2|3|4|5|8|9>\n");
		debugPrintf("  sound <name> stages one gameplay event for classic sound testing.\n");
		debugPrintf("  sound test runs the staged checks interactively.\n");
		debugPrintf("  sound sound <id> calls the original sound(id) routine directly.\n");
		debugPrintf("  sound sound2 <id> calls the original sound2(id) routine directly.\n");
		for (uint i = 0; i < ARRAYSIZE(SOUND_TESTS); ++i)
			debugPrintf("  %u: %s\n", i + 1, SOUND_TESTS[i]);
		for (uint i = 0; i < ARRAYSIZE(SOUND_TEST_SOUND_IDS); ++i)
			debugPrintf("  sound-%d\n", SOUND_TEST_SOUND_IDS[i]);
		for (uint i = 0; i < ARRAYSIZE(SOUND_TEST_SOUND2_IDS); ++i)
			debugPrintf("  sound2-%d\n", SOUND_TEST_SOUND2_IDS[i]);
		return true;
	}

	if (argc == 1 || !scumm_stricmp(argv[1], "test")) {
		_pendingSoundTest = true;
		return false;
	}

	if (argc == 3 && parseSoundId(argv[2], soundId)) {
		if (runDirectSoundTest(argv[1], soundId))
			return false;

		debugPrintf("Sound routine '%s' does not support id %d\n", argv[1], soundId);
		debugPrintf("Use 'sound list' to show known tests.\n");
		return true;
	}

	testName = argv[1];
	if (runDirectSoundTestAlias(testName) || runSoundTestCase(testName))
		return false;

	debugPrintf("Unknown sound test '%s'\n", argv[1]);
	debugPrintf("Use 'sound list' to show known tests.\n");
	return true;
}

} // namespace MM1
} // namespace MM
