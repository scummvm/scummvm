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

#include "agds/console.h"
#include "agds/agds.h"
#include "agds/animation.h"
#include "agds/object.h"
#include "agds/process.h"
#include "agds/screen.h"

namespace AGDS {

Console::Console(AGDSEngine *engine) : _engine(engine) {
	registerCmd("activate",		WRAP_METHOD(Console, activate));
	registerCmd("info",			WRAP_METHOD(Console, info));
	registerCmd("run",			WRAP_METHOD(Console, run));
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
	_engine->runObject(object);
	detach();
	return false;
}

bool Console::activate(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("usage: %s object_id\n", argv[0]);
		return true;
	}
	_engine->reactivate(argv[1]);
	detach();
	return false;
}

bool Console::info(int argc, const char **argv) {
	auto screen = _engine->getCurrentScreen();
	if (screen) {
		debugPrintf("screen %s:\n", screen->getName().c_str());
		for(auto & object : screen->children()) {
			auto pos = object->getPosition();
			debugPrintf("object %s [inscene: %d] at %d,%d\n", object->getName().c_str(), object->inScene(), pos.x, pos.y);
		}
		for(auto & animation : screen->animations()) {
			auto pos = animation->position();
			debugPrintf("animation %s (process: %s, %s) at %d,%d,%d, frame: %d\n",
				animation->phaseVar().c_str(), animation->process().c_str(), animation->paused()? "paused": "running",
				pos.x, pos.y, animation->z(), animation->frameIndex());
		}
	}
	debugPrintf("processes:\n");
	auto & processes = _engine->processes();
	for(auto & process : processes) {
		debugPrintf("%s\n", process->getName().c_str());
	}
	return true;
}

}
