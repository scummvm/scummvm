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

#include "agds/console.h"
#include "agds/agds.h"
#include "agds/animation.h"
#include "agds/object.h"
#include "agds/patch.h"
#include "agds/process.h"
#include "agds/screen.h"

namespace AGDS {

Console::Console(AGDSEngine *engine) : _engine(engine) {
	registerCmd("activate", WRAP_METHOD(Console, activate));
	registerCmd("info", WRAP_METHOD(Console, info));
	registerCmd("load", WRAP_METHOD(Console, load));
	registerCmd("run", WRAP_METHOD(Console, run));
	registerCmd("stop", WRAP_METHOD(Console, stop));
	registerCmd("set", WRAP_METHOD(Console, setGlobal));
	registerCmd("invadd", WRAP_METHOD(Console, inventoryAdd));
	registerCmd("patch", WRAP_METHOD(Console, patch));
}

bool Console::load(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("usage: %s object_id\n", argv[0]);
		return true;
	}
	ObjectPtr object = _engine->loadObject(argv[1]);
	if (!object) {
		debugPrintf("no object %s\n", argv[1]);
		return true;
	}
	_engine->runObject(object);
	detach();
	return false;
}

bool Console::run(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("usage: %s object_id\n", argv[0]);
		return true;
	}
	ObjectPtr object = _engine->getCurrentScreenObject(argv[1]);
	if (!object) {
		debugPrintf("no object %s\n", argv[1]);
		return true;
	}
	_engine->getCurrentScreen()->remove(object);
	_engine->runObject(object);
	detach();
	return false;
}

bool Console::stop(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("usage: %s object_id\n", argv[0]);
		return true;
	}
	ObjectPtr object = _engine->getCurrentScreenObject(argv[1]);
	if (!object) {
		debugPrintf("no object %s\n", argv[1]);
		return true;
	}
	_engine->getCurrentScreen()->remove(object);
	auto process = _engine->findProcess(argv[1]);
	if (!process) {
		debugPrintf("no process %s\n", argv[1]);
		return true;
	}
	process->done();
	detach();
	return false;
}

bool Console::activate(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("usage: %s object_id\n", argv[0]);
		return true;
	}
	_engine->reactivate(argv[1], "console");
	detach();
	return false;
}

bool Console::info(int argc, const char **argv) {
	auto screen = _engine->getCurrentScreen();
	if (screen) {
		debugPrintf("screen %s:\n", screen->getName().c_str());
		for (auto &object : screen->children()) {
			auto pos = object->getPosition();
			debugPrintf("object %s [alive: %d] at %d,%d\n", object->getName().c_str(), object->alive(), pos.x, pos.y);
		}
		for (auto &desc : screen->animations()) {
			auto &animation = desc.animation;
			auto pos = animation->position();
			debugPrintf("animation %s (process: %s, %s) at %d,%d,%d, frame: %d\n",
						animation->phaseVar().c_str(), animation->process().c_str(), animation->paused() ? "paused" : "running",
						pos.x, pos.y, animation->z(), animation->phase());
		}
	}
	debugPrintf("processes:\n");
	auto &processes = _engine->processes();
	for (auto &process : processes) {
		if (process)
			debugPrintf("%s\n", process->getName().c_str());
	}
	return true;
}

bool Console::setGlobal(int argc, const char **argv) {
	if (argc < 3) {
		debugPrintf("usage: %s var value\n", argv[0]);
		return true;
	}
	int value;
	if (sscanf(argv[2], "%d", &value) != 1) {
		debugPrintf("invalid value\n");
		return true;
	}
	_engine->setGlobal(argv[1], value);
	return true;
}

bool Console::inventoryAdd(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("usage: %s inv.object\n", argv[0]);
		return true;
	}
	int idx = _engine->inventory().add(argv[1]);
	debugPrintf("add = %d\n", idx);

	detach();
	return true;
}

bool Console::patch(int argc, const char **argv) {
	if (argc != 2 && argc != 4) {
		debugPrintf("usage: %s screen [object flag]\n", argv[0]);
		return true;
	}
	if (argc == 2) {
		auto patch = _engine->getPatch(argv[1]);
		if (!patch) {
			debugPrintf("no patch for %s found.", argv[1]);
			return true;
		}
		debugPrintf("screen saved: %d\n", patch->screenSaved);
		debugPrintf("screen region: %s\n", patch->screenRegionName.c_str());
		debugPrintf("previous screen: %s\n", patch->prevScreenName.c_str());
		debugPrintf("screen loading type: %d\n", static_cast<int>(patch->loadingType));
		debugPrintf("character pos: %d,%d, direction: %d, present: %d\n", patch->characterPosition.x, patch->characterPosition.y, patch->characterDirection, patch->characterPresent);
		debugPrintf("mouse cursor: %s\n", patch->defaultMouseCursor.c_str());
		for (auto &object : patch->objects) {
			debugPrintf(" - object %s: present: %d\n", object.name.c_str(), object.flag);
		}
	} else if (argc == 4) {
		auto patch = _engine->getPatch(argv[1]);
		if (!patch)
			debugPrintf("no patch found, creating...\n");
		patch = _engine->createPatch(argv[1]);
		patch->setFlag(argv[2], atoi(argv[3]));
	}

	detach();
	return true;
}

} // namespace AGDS
