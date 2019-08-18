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

#include "teenagent/console.h"
#include "teenagent/resources.h"
#include "teenagent/teenagent.h"

namespace TeenAgent {

Console::Console(TeenAgentEngine *engine) : _engine(engine) {
	registerCmd("enable_object",		WRAP_METHOD(Console, enableObject));
	registerCmd("disable_object",		WRAP_METHOD(Console, enableObject));
	registerCmd("set_ons",			WRAP_METHOD(Console, setOns));
	registerCmd("set_music",			WRAP_METHOD(Console, setMusic));
	registerCmd("animation",			WRAP_METHOD(Console, playAnimation));
	registerCmd("actor_animation",	WRAP_METHOD(Console, playActorAnimation));
	registerCmd("call",				WRAP_METHOD(Console, call));
	registerCmd("playSound",			WRAP_METHOD(Console, playSound));
	registerCmd("playVoice",			WRAP_METHOD(Console, playVoice));
}

bool Console::enableObject(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("usage: %s object_id [scene_id]\n", argv[0]);
		return true;
	}

	int id = atoi(argv[1]);
	if (id < 0) {
		debugPrintf("object id %d is invalid\n", id);
		return true;
	}

	int scene_id = 0;
	if (argc > 2) {
		scene_id = atoi(argv[2]);
		if (scene_id < 0) {
			debugPrintf("scene id %d is invalid\n", scene_id);
			return true;
		}
	}

	if (strcmp(argv[0], "disable_object") == 0)
		_engine->disableObject(id, scene_id);
	else
		_engine->enableObject(id, scene_id);

	return true;
}

bool Console::setOns(int argc, const char **argv) {
	if (argc < 3) {
		debugPrintf("usage: %s index(0-3) value [scene_id]\n", argv[0]);
		return true;
	}

	int index = atoi(argv[1]);
	if (index < 0 || index > 3) {
		debugPrintf("index %d is invalid\n", index);
		return true;
	}

	int value = 0;
	value = atoi(argv[2]);
	if (value < 0) {
		debugPrintf("invalid value\n");
		return true;
	}

	int scene_id = 0;
	if (argc > 3) {
		scene_id = atoi(argv[3]);
		if (scene_id < 0) {
			debugPrintf("scene id %d is invalid\n", scene_id);
			return true;
		}
	}

	_engine->setOns(index, value, scene_id);
	return true;
}

bool Console::setMusic(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("usage: %s index(1-11)\n", argv[0]);
		return true;
	}

	int index = atoi(argv[1]);
	if (index <= 0 || index > 11) {
		debugPrintf("invalid value\n");
		return true;
	}

	_engine->setMusic(index);
	return true;
}

bool Console::playAnimation(int argc, const char **argv) {
	if (argc < 3) {
		debugPrintf("usage: %s id slot(0-3)\n", argv[0]);
		return true;
	}

	int id = atoi(argv[1]);
	int slot = atoi(argv[2]);
	if (id < 0 || slot < 0 || slot > 3) {
		debugPrintf("invalid slot or animation id\n");
		return true;
	}

	_engine->playAnimation(id, slot);
	return true;
}

bool Console::playActorAnimation(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("usage: %s id\n", argv[0]);
		return true;
	}

	int id = atoi(argv[1]);
	if (id < 0) {
		debugPrintf("invalid animation id\n");
		return true;
	}

	_engine->playActorAnimation(id);
	return true;
}

bool Console::call(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("usage: %s 0xHEXADDR\n", argv[0]);
		return true;
	}

	uint addr;
	if (sscanf(argv[1], "0x%x", &addr) != 1) {
		debugPrintf("invalid address\n");
		return true;
	}

	if (!_engine->processCallback(addr))
		debugPrintf("calling callback %04x failed\n", addr);

	return true;
}

bool Console::playSound(int argc, const char **argv) {
	uint32 fileCount = _engine->res->sam_sam.fileCount();
	if (argc < 2) {
		debugPrintf("usage: %s index(1-%d)\n", argv[0], fileCount);
		return true;
	}

	uint index = atoi(argv[1]);
	if (index <= 0 || index > fileCount) {
		debugPrintf("invalid value\n");
		return true;
	}

	_engine->playSoundNow(&_engine->res->sam_sam, index);
	return true;
}

bool Console::playVoice(int argc, const char **argv) {
	uint32 fileCount = _engine->res->voices.fileCount();
	if (argc < 2) {
		debugPrintf("usage: %s index(1-%d)\n", argv[0], fileCount);
		return true;
	}

	uint index = atoi(argv[1]);
	if (index <= 0 || index > fileCount) {
		debugPrintf("invalid value\n");
		return true;
	}

	_engine->playSoundNow(&_engine->res->voices, index);
	return true;
}

}
