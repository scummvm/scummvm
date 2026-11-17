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

#include "tesoro/console.h"
#include "tesoro/core/animbank.h"
#include "tesoro/core/cursor.h"
#include "tesoro/core/jim.h"
#include "tesoro/core/level.h"
#include "tesoro/core/namekey.h"
#include "tesoro/core/walkfloor.h"
#include "tesoro/scripts.h"
#include "tesoro/world.h"

namespace Tesoro {

Console::Console() : GUI::Debugger() {
	registerCmd("testCmd", WRAP_METHOD(Console, testCmd));
	registerCmd("fade", WRAP_METHOD(Console, fade));
	registerCmd("level", WRAP_METHOD(Console, level));
	registerCmd("hover", WRAP_METHOD(Console, hover));
	registerCmd("hitter", WRAP_METHOD(Console, hitter));
	registerCmd("touching", WRAP_METHOD(Console, touching));
	registerCmd("outlines", WRAP_METHOD(Console, outlines));
	registerCmd("jim", WRAP_METHOD(Console, jim));
	registerCmd("scale", WRAP_METHOD(Console, scale));
	registerCmd("route", WRAP_METHOD(Console, route));
	registerCmd("start", WRAP_METHOD(Console, start));
	registerCmd("voice", WRAP_METHOD(Console, voice));
	registerCmd("jimsay", WRAP_METHOD(Console, jimsay));
	registerCmd("scripts", WRAP_METHOD(Console, scripts));

}

Console::~Console() {
}

bool Console::testCmd(int argc, const char **argv) {
	debugPrintf("test\n");
	return false;
}



bool Console::fade(int argc, const char **argv) {
	if (argc != 3) {
		debugPrintf("Usage: %s STEPS DELAY_MS\n", argv[0]);
		return true;
	}
	g_engine->fadeOut(atoi(argv[1]), atoi(argv[2]));
	return false;
}

bool Console::level(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: %s INDEX   (0 to 46: the game's list of levels)\n", argv[0]);
		return true;
	}
	if (!g_engine->level(atoi(argv[1]))) {
		debugPrintf("Cannot load level %s\n", argv[1]);
		return true;
	}
	return false;
}

bool Console::hover(int argc, const char **argv) {
	if (argc < 2) {
		g_engine->setHoverText(nullptr);
		return false;
	}
	g_engine->setHoverText(argv[1]);
	return false;
}

bool Console::voice(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: %s ID\n", argv[0]);
		return true;
	}

	World &world = g_engine->world();
	const uint32 ref = world.speech().find(argv[1]);
	if (!ref) {
		debugPrintf("No line named %s\n", argv[1]);
		return true;
	}

	const SpeechTable::Line &line = world.speech().lineOf(ref);
	if (!line.wavOffset) {
		debugPrintf("No voice\n");
		return true;
	}

	const uint32 id = world.playVoice(line.wavOffset);
	debugPrintf("Sound id: %u\n", id);
	return true;
}

bool Console::jimsay(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: %s ID [narration]\n", argv[0]);
		return true;
	}

	World &world = g_engine->world();
	if (!world.jimLevel()) {
		debugPrintf("Jim is not in a level (use start first)\n");
		return true;
	}

	const uint32 ref = world.speech().find(argv[1]);
	if (!ref) {
		debugPrintf("No line named %s\n", argv[1]);
		return true;
	}

	const SpeechTable::Line &line = world.speech().lineOf(ref);
	if (argc > 2 && scumm_stricmp(argv[2], "narration") == 0)
		world.jim()->sayNarrationLine(line);
	else
		world.jim()->sayAnswer(line);
	return true;
}

bool Console::hitter(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: %s OBJECT   (makes an object of the level something the pointer can touch)\n", argv[0]);
		return true;
	}
	if (!g_engine->makeHitter(argv[1]))
		debugPrintf("No object %s in the level\n", argv[1]);
	return true;
}

bool Console::outlines(int argc, const char **argv) {
	debugPrintf("Outlines %s\n", g_engine->toggleOutlines() ? "on: red = pointer can touch, yellow = collidable, grey = other, cyan = outline" : "off");
	return true;
}

bool Console::jim(int argc, const char **argv) {
	if (argc != 3) {
		debugPrintf("Usage: %s X Y   (puts Jim in the level at world coordinates)\n", argv[0]);
		return true;
	}
	if (!g_engine->putJim(atoi(argv[1]), atoi(argv[2])))
		debugPrintf("Jim cannot be put there\n");
	return true;
}

bool Console::scale(int argc, const char **argv) {
	if (argc != 3) {
		debugPrintf("Usage: %s X Y   (the size of things at world coordinates; 256 is life size)\n", argv[0]);
		return true;
	}
	Level *level = g_engine->shownLevel();
	WalkFloor *floor = level ? level->floor() : nullptr;
	if (!floor)
		debugPrintf("No floor in this level\n");
	else
		debugPrintf("%d\n", (int)floor->scaleAt(atoi(argv[1]), atoi(argv[2])));
	return true;
}

bool Console::route(int argc, const char **argv) {
	if (argc != 3) {
		debugPrintf("Usage: %s X Y   (sends Jim to world coordinates round the waypoints)\n", argv[0]);
		return true;
	}
	World &world = g_engine->world();
	if (!world.jimLevel()) {
		debugPrintf("Jim is not in a level (use jim X Y)\n");
		return true;
	}
	debugPrintf(world.findPath(atoi(argv[1]), atoi(argv[2]), nullptr) ? "true\n" : "false\n");
	return true;
}

bool Console::start(int argc, const char **argv) {
	g_engine->startGame();
	return true;
}

bool Console::touching(int argc, const char **argv) {
	const Cursor *cursor = g_engine->cursor();
	if (!cursor) {
		debugPrintf("No pointer\n");
		return true;
	}

	// The pointer in the interface level and its companion in the level the player is in.
	const ObjectList *lists[2] = {&cursor->touched(), &cursor->companion()->touched()};
	uint count = 0;
	for (uint l = 0; l < 2; ++l) {
		for (uint i = 0; i < lists[l]->size(); ++i) {
			const Object *object = (*lists[l])[i];
			debugPrintf("kind %u, name key %016llx\n", object->kindId(), (unsigned long long)object->nameKey());
			++count;
		}
	}
	if (count == 0)
		debugPrintf("Nothing under the pointer\n");
	return true;
}

bool Console::scripts(int argc, const char **argv) {
	Scripts *scripts = g_engine->world().scripts();
	if (!scripts) {
		debugPrintf("The scripts are not loaded\n");
		return true;
	}

	const ScriptLibrary &library = scripts->library();
	if (argc < 2) {
		uint32 labels = 0;
		for (uint32 i = 0; i < library.blockCount(); ++i)
			labels += library.block(i).labelCount();
		debugPrintf("Blocks: %u\nLabels: %u\n", library.blockCount(), labels);
		return true;
	}

	const ScriptBlock *block = library.find(nameKey64(argv[1]));
	if (!block) {
		debugPrintf("No block named %s\n", argv[1]);
		return true;
	}

	debugPrintf("Type: %u\nCharacters: %u\nLabels: %u\nCommands: %u\n", block->type(), block->characterCount(), block->labelCount(), block->entryCount());
	for (uint32 i = 0; i < block->entryCount(); ++i)
		debugPrintf("%u ", block->entry(i).op());
	debugPrintf("\n");
	return true;
}

} // End of namespace Tesoro
