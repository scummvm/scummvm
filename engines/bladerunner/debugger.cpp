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

#include "bladerunner/debugger.h"

#include "bladerunner/actor.h"
#include "bladerunner/bladerunner.h"
#include "bladerunner/boundingbox.h"
#include "bladerunner/combat.h"
#include "bladerunner/font.h"
#include "bladerunner/game_constants.h"
#include "bladerunner/game_flags.h"
#include "bladerunner/game_info.h"
#include "bladerunner/light.h"
#include "bladerunner/lights.h"
#include "bladerunner/regions.h"
#include "bladerunner/savefile.h"
#include "bladerunner/scene.h"
#include "bladerunner/scene_objects.h"
#include "bladerunner/settings.h"
#include "bladerunner/set.h"
#include "bladerunner/text_resource.h"
#include "bladerunner/vector.h"
#include "bladerunner/view.h"
#include "bladerunner/vqa_decoder.h"
#include "bladerunner/vqa_player.h"
#include "bladerunner/waypoints.h"
#include "bladerunner/zbuffer.h"

#include "common/debug.h"
#include "common/str.h"

#include "graphics/surface.h"

namespace BladeRunner {

Debugger::Debugger(BladeRunnerEngine *vm) : GUI::Debugger() {
	_vm = vm;

	_viewSceneObjects = false;
	_viewActorsOnly = false;
	_viewObstacles = false;
	_viewUI = false;
	_viewZBuffer = false;

	registerCmd("anim", WRAP_METHOD(Debugger, cmdAnimation));
	registerCmd("chapter", WRAP_METHOD(Debugger, cmdChapter));
	registerCmd("draw", WRAP_METHOD(Debugger, cmdDraw));
	registerCmd("flag", WRAP_METHOD(Debugger, cmdFlag));
	registerCmd("goal", WRAP_METHOD(Debugger, cmdGoal));
	registerCmd("loop", WRAP_METHOD(Debugger, cmdLoop));
	registerCmd("pos", WRAP_METHOD(Debugger, cmdPosition));
	registerCmd("say", WRAP_METHOD(Debugger, cmdSay));
	registerCmd("scene", WRAP_METHOD(Debugger, cmdScene));
	registerCmd("var", WRAP_METHOD(Debugger, cmdVariable));
	registerCmd("load", WRAP_METHOD(Debugger, cmdLoad));
	registerCmd("save", WRAP_METHOD(Debugger, cmdSave));
}

Debugger::~Debugger() {
}

bool Debugger::cmdAnimation(int argc, const char **argv) {
	if (argc != 2 && argc != 3) {
		debugPrintf("Get or set animation mode of the actor.\n");
		debugPrintf("Usage: %s <actorId> [<animationMode>]\n", argv[0]);
		return true;
	}

	int actorId = atoi(argv[1]);

	Actor *actor = nullptr;
	if (actorId >= 0 && actorId < (int)_vm->_gameInfo->getActorCount()) {
		actor = _vm->_actors[actorId];
	}

	if (actor == nullptr) {
		debugPrintf("Unknown actor %i\n", actorId);
		return true;
	}

	if (argc == 3) {
		int animationMode = atoi(argv[2]);
		debugPrintf("actorAnimationMode(%i) = %i\n", actorId, animationMode);
		actor->changeAnimationMode(animationMode);
		return false;
	}

	debugPrintf("actorAnimationMode(%i) = %i\n", actorId, actor->getAnimationMode());
	return true;
}

bool Debugger::cmdDraw(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Enables debug rendering of scene objects, obstacles, ui elements, zbuffer or disables debug rendering.\n");
		debugPrintf("Usage: %s (obj | actors | obstacles | ui | zbuf | reset)\n", argv[0]);
		return true;
	}

	Common::String arg = argv[1];
	if (arg == "obj") {
		_viewSceneObjects = !_viewSceneObjects;
		debugPrintf("Drawing scene objects = %i\n", _viewSceneObjects);
	} else if (arg == "actors") {
		_viewSceneObjects = !_viewSceneObjects;
		_viewActorsOnly = _viewSceneObjects;
		debugPrintf("Drawing scene actors = %i\n", _viewSceneObjects);
	} else if (arg == "obstacles") {
		_viewObstacles = !_viewObstacles;
		debugPrintf("Drawing obstacles = %i\n", _viewObstacles);
	} else if (arg == "ui") {
		_viewUI = !_viewUI;
		debugPrintf("Drawing UI elements = %i\n", _viewUI);
	} else if (arg == "zbuf") {
		_viewZBuffer = !_viewZBuffer;
		debugPrintf("Drawing Z buffer = %i\n", _viewZBuffer);
	} else if (arg == "reset") {
		_viewSceneObjects = false;
		_viewUI = false;
		_viewZBuffer = false;
		debugPrintf("Drawing scene objects = %i\n", _viewSceneObjects);
		debugPrintf("Drawing UI elements = %i\n", _viewUI);
		debugPrintf("Drawing Z buffer = %i\n", _viewZBuffer);
	}

	return true;
}

bool Debugger::cmdChapter(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Changes chapter of the game without changing scene.\n");
		debugPrintf("Usage: %s <chapter>\n", argv[0]);
		return true;
	}

	int chapter = atoi(argv[1]);
	if (chapter >= 1 && chapter <= 5) {
		_vm->_settings->setChapter(chapter);
	} else {
		debugPrintf("Chapter must be between 1 and 5\n");
	}

	return true;
}

bool Debugger::cmdFlag(int argc, const char **argv) {
	if (argc != 2 && argc != 3) {
		debugPrintf("Get or set game flag (boolean value).\n");
		debugPrintf("Usage: %s <id> [<value>]\n", argv[0]);
		return true;
	}

	int flag = atoi(argv[1]);
	int flagCount = _vm->_gameInfo->getFlagCount();
	if (flag > 0 && flag < flagCount) {
		if (argc == 3) {
			int value = atoi(argv[2]);
			if (value == 0) {
				_vm->_gameFlags->reset(flag);
			} else {
				_vm->_gameFlags->set(flag);
			}
		}
		debugPrintf("flag(%i) = %i\n", flag, _vm->_gameFlags->query(flag));
	} else {
		debugPrintf("Flag id must be between 0 and %i\n", flagCount - 1);
	}

	return true;
}

bool Debugger::cmdGoal(int argc, const char **argv) {
	if (argc != 2 && argc != 3) {
		debugPrintf("Get or set goal of the actor.\n");
		debugPrintf("Usage: %s <actorId> [<goal>]\n", argv[0]);
		return true;
	}

	int actorId = atoi(argv[1]);

	Actor *actor = nullptr;
	if (actorId >= 0 && actorId < (int)_vm->_gameInfo->getActorCount()) {
		actor = _vm->_actors[actorId];
	}

	if (actor == nullptr) {
		debugPrintf("Unknown actor %i\n", actorId);
		return true;
	}

	if (argc == 3) {
		int goal = atoi(argv[2]);
		debugPrintf("actorGoal(%i) = %i\n", actorId, goal);
		actor->setGoal(goal);
		return false;
	}

	debugPrintf("actorGoal(%i) = %i\n", actorId, actor->getGoal());
	return true;
}

bool Debugger::cmdLoop(int argc, const char **argv) {
	if (argc != 1 && argc != 2) {
		debugPrintf("Show scene loops or play scene loop.\n");
		debugPrintf("Usage: %s [<loopId>]\n", argv[0]);
		return true;
	}

	VQADecoder::LoopInfo &loopInfo = _vm->_scene->_vqaPlayer->_decoder._loopInfo;
	if (argc == 1) {
		debugPrintf("id start  end name\n");
		for (int i = 0; i < loopInfo.loopCount; ++i) {
			debugPrintf("%2d  %4d %4d %s\n", i, loopInfo.loops[i].begin, loopInfo.loops[i].end, loopInfo.loops[i].name.c_str());
		}
		return true;
	}

	int loopId = atoi(argv[1]);
	if (loopId >= 0 && loopId < loopInfo.loopCount) {
		_vm->_scene->loopStartSpecial(kSceneLoopModeOnce, loopId, false);
		return false;
	} else {
		debugPrintf("Unknown loop %i\n", loopId);
		return true;
	}
}

bool Debugger::cmdPosition(int argc, const char **argv) {
	if (argc != 2 && argc != 3 && argc != 7) {
		debugPrintf("Get or set position of the actor.\n");
		debugPrintf("Usage: %s <actorId> [(<setId> <x> <y> <z> <facing>)|<otherActorId>]\n", argv[0]);
		return true;
	}

	int actorId = atoi(argv[1]);

	Actor *actor = nullptr;
	if (actorId >= 0 && actorId < (int)_vm->_gameInfo->getActorCount()) {
		actor = _vm->_actors[actorId];
	}

	if (actor == nullptr) {
		debugPrintf("Unknown actor %i\n", actorId);
		return true;
	}

	if (argc == 2) {
		debugPrintf("actorSet(%i) = %i\n", actorId, actor->getSetId());
		debugPrintf("actorX(%i) = %f\n", actorId, actor->getX());
		debugPrintf("actorY(%i) = %f\n", actorId, actor->getY());
		debugPrintf("actorZ(%i) = %f\n", actorId, actor->getZ());
		debugPrintf("actorFacing(%i) = %i\n", actorId, actor->getFacing());
	}

	if (argc == 3) {
		int otherActorId = atoi(argv[2]);
		Actor *otherActor = nullptr;
		if (otherActorId >= 0 && otherActorId < (int)_vm->_gameInfo->getActorCount()) {
			otherActor = _vm->_actors[otherActorId];
		}

		if (otherActor == nullptr) {
			debugPrintf("Unknown actor %i\n", otherActorId);
			return true;
		}

		Vector3 position = otherActor->getXYZ();
		actor->setSetId(otherActor->getSetId());
		actor->setAtXYZ(position, otherActor->getFacing());
	}

	if (argc == 7) {
		int setId = atoi(argv[2]);
		Vector3 position(atof(argv[3]), atof(argv[4]), atof(argv[5]));
		int facing = atoi(argv[6]);

		actor->setSetId(setId);
		actor->setAtXYZ(position, facing);
	}
	return true;
}

bool Debugger::cmdSay(int argc, const char **argv) {
	if (argc != 3) {
		debugPrintf("Actor will say specified line.\n");
		debugPrintf("Usage: %s <actorId> <sentenceId>\n", argv[0]);
		return true;
	}

	int actorId = atoi(argv[1]);
	int sentenceId = atoi(argv[2]);

	Actor *actor = nullptr;
	if (actorId >= 0 && actorId < (int)_vm->_gameInfo->getActorCount()) {
		actor = _vm->_actors[actorId];
	}

	if (actor == nullptr) {
		debugPrintf("Unknown actor %i\n", actorId);
		return true;
	}

	actor->speechPlay(sentenceId, true);
	return false;
}

const struct SceneList {
	int chapter;
	const char *name;
	int set;
	int scene;
} sceneList[] = {
	{ 1, "CT01", 4, 13 },    { 1, "CT02", 27, 14 },  { 1, "CT03", 5, 15 },    { 1, "CT04", 5, 16 },
	{ 1, "CT05", 28, 17 },   { 1, "CT06", 29, 18 },  { 1, "CT07", 30, 19 },   { 1, "CT12", 4, 24 },
	{ 1, "MA01", 49, 48 },   { 1, "MA02", 10, 49 },  { 1, "MA04", 10, 50 },   { 1, "MA04", 50, 50 },
	{ 1, "MA05", 51, 51 },   { 1, "MA06", 52, 52 },  { 1, "MA07", 53, 53 },   { 1, "PS01", 61, 65 },
	{ 1, "PS02", 62, 66 },   { 1, "PS03", 63, 67 },  { 1, "PS04", 64, 68 },   { 1, "PS05", 15, 59 },
	{ 1, "PS06", 65, 70 },   { 1, "PS07", 66, 71 },  { 1, "PS09", 67, 72 },   { 1, "PS10", 14, 73 },
	{ 1, "PS11", 14, 74 },   { 1, "PS12", 14, 75 },  { 1, "PS13", 14, 76 },   { 1, "PS14", 68, 77 },
	{ 1, "PS15", 101, 119 }, { 1, "RC01", 69, 78 },  { 1, "RC02", 16, 79 },   { 1, "RC51", 16, 107 },

	{ 2, "AR01", 0, 0 },     { 2, "AR02", 0, 1 },    { 2, "BB01", 20, 2 },    { 2, "BB02", 1, 3 },
	{ 2, "BB03", 21, 4 },    { 2, "BB04", 1, 5 },    { 2, "BB05", 22, 6 },    { 2, "BB06", 1, 7 },
	{ 2, "BB06", 2, 7 },     { 2, "BB07", 2, 8 },    { 2, "BB07", 3, 8 },     { 2, "BB08", 23, 9 },
	{ 2, "BB10", 25, 11 },   { 2, "BB11", 26, 12 },  { 2, "BB12", 102, 120 }, { 2, "BB51", 1, 104 },
	{ 2, "CT01", 4, 13 },    { 2, "CT02", 27, 14 },  { 2, "CT03", 5, 15 },    { 2, "CT04", 5, 16 },
	{ 2, "CT05", 28, 17 },   { 2, "CT06", 29, 18 },  { 2, "CT08", 6, 20 },    { 2, "CT09", 31, 21 },
	{ 2, "CT10", 32, 22 },   { 2, "CT11", 33, 23 },  { 2, "CT12", 4, 24 },    { 2, "CT51", 6, 105 },
	{ 2, "DR01", 7, 25 },    { 2, "DR02", 7, 26 },   { 2, "DR03", 34, 27 },   { 2, "DR04", 7, 28 },
	{ 2, "DR05", 35, 29 },   { 2, "DR06", 36, 30 },  { 2, "HC01", 8, 31 },    { 2, "HC02", 8, 32 },
	{ 2, "HC03", 8, 33 },    { 2, "HC04", 8, 106 },  { 2, "HF01", 37, 34 },   { 2, "HF02", 38, 35 },
	{ 2, "HF03", 39, 36 },   { 2, "HF04", 40, 37 },  { 2, "HF05", 41, 38 },   { 2, "HF06", 42, 39 },
	{ 2, "MA01", 49, 48 },   { 2, "MA02", 10, 49 },  { 2, "MA04", 10, 50 },   { 2, "MA04", 50, 50 },
	{ 2, "MA05", 51, 51 },   { 2, "MA06", 52, 52 },  { 2, "MA07", 53, 53 },   { 2, "NR01", 54, 54 },
	{ 2, "NR02", 11, 55 },   { 2, "NR03", 55, 56 },  { 2, "NR04", 12, 57 },   { 2, "NR05", 13, 58 },
	{ 2, "NR06", 56, 59 },   { 2, "NR07", 57, 60 },  { 2, "NR08", 13, 61 },   { 2, "NR09", 58, 62 },
	{ 2, "NR10", 59, 63 },   { 2, "NR11", 60, 64 },  { 2, "PS01", 61, 65 },   { 2, "PS02", 62, 66 },
	{ 2, "PS03", 63, 67 },   { 2, "PS04", 64, 68 },  { 2, "PS05", 15, 59 },   { 2, "PS06", 65, 70 },
	{ 2, "PS07", 66, 71 },   { 2, "PS09", 67, 72 },  { 2, "PS10", 14, 73 },   { 2, "PS11", 14, 74 },
	{ 2, "PS12", 14, 75 },   { 2, "PS13", 14, 76 },  { 2, "PS14", 68, 77 },   { 2, "PS15", 101, 119 },
	{ 2, "RC01", 69, 78 },   { 2, "RC03", 70, 80 },  { 2, "RC04", 71, 81 },   { 2, "TB02", 17, 82 },
	{ 2, "TB05", 72, 84 },   { 2, "TB06", 73, 85 },  { 2, "TB07", 18, 108 },  { 2, "UG01", 74, 86 },
	{ 2, "UG02", 75, 87 },   { 2, "UG03", 76, 88 },  { 2, "UG04", 77, 89 },   { 2, "UG06", 79, 91 },
	{ 2, "UG10", 83, 95 },

	{ 4, "AR01", 0, 0 },     { 4, "AR02", 0, 1 },    { 4, "BB01", 20, 2 },    { 4, "BB02", 1, 3 },
	{ 4, "BB03", 21, 4 },    { 4, "BB04", 1, 5 },    { 4, "BB51", 1, 104 },   { 4, "CT01", 4, 13 },
	{ 4, "CT02", 27, 14 },   { 4, "CT03", 5, 15 },   { 4, "CT04", 5, 16 },    { 4, "CT05", 28, 17 },
	{ 4, "CT06", 29, 18 },   { 4, "CT08", 6, 20 },   { 4, "CT09", 31, 21 },   { 4, "CT10", 32, 22 },
	{ 4, "CT11", 33, 23 },   { 4, "CT12", 4, 24 },   { 4, "CT51", 6, 105 },   { 4, "DR01", 7, 25 },
	{ 4, "DR02", 7, 26 },    { 4, "DR03", 34, 27 },  { 4, "DR04", 7, 28 },    { 4, "DR05", 35, 29 },
	{ 4, "DR06", 36, 30 },   { 4, "HC01", 8, 31 },   { 4, "HC02", 8, 32 },    { 4, "HC03", 8, 33 },
	{ 4, "HC04", 8, 106 },   { 4, "HF01", 37, 34 },  { 4, "HF02", 38, 35 },   { 4, "HF03", 39, 36 },
	{ 4, "HF04", 40, 37 },   { 4, "HF05", 41, 38 },  { 4, "HF06", 42, 39 },   { 4, "HF07", 43, 40 },
	{ 4, "KP01", 44, 41 },   { 4, "KP02", 45, 42 },  { 4, "KP03", 46, 43 },   { 4, "KP04", 47, 44 },
	{ 4, "KP05", 9, 45 },    { 4, "KP06", 9, 46 },   { 4, "KP07", 48, 47 },   { 4, "MA02", 10, 49 },
	{ 4, "MA04", 10, 50 },   { 4, "MA04", 50, 50 },  { 4, "MA05", 51, 51 },   { 4, "MA06", 52, 52 },
	{ 4, "MA07", 53, 53 },   { 4, "NR01", 54, 54 },  { 4, "NR02", 11, 55 },   { 4, "NR03", 55, 56 },
	{ 4, "NR04", 12, 57 },   { 4, "NR05", 13, 58 },  { 4, "NR06", 56, 59 },   { 4, "NR07", 57, 60 },
	{ 4, "NR08", 13, 61 },   { 4, "NR09", 58, 62 },  { 4, "NR10", 59, 63 },   { 4, "NR11", 60, 64 },
	{ 4, "PS09", 67, 72 },   { 4, "PS14", 68, 77 },  { 4, "RC01", 69, 78 },   { 4, "RC02", 16, 89 },
	{ 4, "RC03", 70, 80 },   { 4, "RC04", 71, 81 },  { 4, "RC51", 16, 107 },  { 4, "TB02", 17, 82 },
	{ 4, "TB03", 17, 83 },   { 4, "TB07", 18, 108 }, { 4, "UG01", 74, 86 },   { 4, "UG02", 75, 87 },
	{ 4, "UG03", 76, 88 },   { 4, "UG04", 77, 89 },  { 4, "UG05", 78, 90 },   { 4, "UG06", 79, 91 },
	{ 4, "UG07", 80, 92 },   { 4, "UG08", 81, 93 },  { 4, "UG09", 82, 94 },   { 4, "UG10", 83, 95 },
	{ 4, "UG12", 84, 96 },   { 4, "UG13", 85, 97 },  { 4, "UG14", 86, 98 },   { 4, "UG15", 87, 99 },
	{ 4, "UG16", 16, 100 },  { 4, "UG17", 88, 101 }, { 4, "UG18", 89, 102 },  { 4, "UG19", 90, 103 },

	{ 0, NULL, 0, 0 }
};

bool Debugger::cmdScene(int argc, const char **argv) {
	if (argc != 0 && argc > 3) {
		debugPrintf("Changes set and scene.\n");
		debugPrintf("Usage: %s [<setId> <sceneId>] | [[<chapterId>] <sceneName>]\n", argv[0]);
		return true;
	}

	// scene <setId> <sceneId>
	if (argc == 3 && Common::isDigit(*argv[1]) && Common::isDigit(*argv[2])) {
		int setId = atoi(argv[1]);
		int sceneId = atoi(argv[2]);
		// Sanity check
		uint i;
		for (i = 0; sceneList[i].chapter != 0; i++) {
			if (sceneList[i].chapter == _vm->_settings->getChapter() && sceneList[i].set == setId
					&& sceneList[i].scene == sceneId)
				break;
		}

		if (sceneList[i].chapter == 0) {	// end of list
			debugPrintf("Scene does not exist in this chapter.\n");
			return true;
		}

		_vm->_settings->setNewSetAndScene(setId, sceneId);
		return false;
	} else if (argc > 1) {
		int chapterId = 0;
		Common::String sceneName;

		// <chapterId> <sceneName>
		if (argc == 3) {
			chapterId = atoi(argv[1]);

			if (chapterId < 1 || chapterId > 5) {
				debugPrintf("Chapter must be between 1 and 5\n");
				return true;
			}

			sceneName = argv[2];
		} else if (argc == 2) { // <sceneName>
			chapterId = _vm->_settings->getChapter();
			sceneName = argv[1];
		}

		int chapterIdNormalized = chapterId;

		if (chapterId == 3 || chapterId == 5) {
			chapterIdNormalized = chapterId - 1;
		}

		uint i;
		for (i = 0; sceneList[i].chapter != 0; i++) {
			if (sceneList[i].chapter == chapterId && sceneName.equalsIgnoreCase(sceneList[i].name))
				break;
		}

		if (sceneList[i].chapter == 0) {
			debugPrintf("Invalid scene name or chapter.\n");
			return true;
		} else {
			if (chapterId != _vm->_settings->getChapter())
				_vm->_settings->setChapter(chapterId);
		}

		_vm->_settings->setNewSetAndScene(sceneList[i].set, sceneList[i].scene);
		return false;
	}

	uint i;
	for (i = 0; sceneList[i].chapter != 0; i++) {
		if (sceneList[i].chapter == _vm->_settings->getChapter() && sceneList[i].set == _vm->_scene->getSetId()
				&& sceneList[i].scene == _vm->_scene->getSceneId())
			break;
	}

	debugPrintf("chapter = %i\nset = %i\nscene = %i '%s'\n", _vm->_settings->getChapter(), _vm->_scene->getSetId(),
				_vm->_scene->getSceneId(), sceneList[i].name);
	return true;
}

bool Debugger::cmdVariable(int argc, const char **argv) {
	if (argc != 2 && argc != 3) {
		debugPrintf("Get or set game variable (integer).\n");
		debugPrintf("Usage: %s <id> [<value>]\n", argv[0]);
		return true;
	}

	int variable = atoi(argv[1]);
	int variableCount = _vm->_gameInfo->getGlobalVarCount();
	if (variable >= 0 && variable < variableCount) {
		if (argc == 3) {
			_vm->_gameVars[variable] = atoi(argv[2]);
		}
		debugPrintf("variable(%i) = %i\n", variable, _vm->_gameVars[variable]);
	} else {
		debugPrintf("Variable id must be between 0 and %i\n", variableCount - 1);
	}
	return true;
}

bool Debugger::cmdLoad(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Loads a save game from original format.\n");
		debugPrintf("Usage: %s <file path>\n", argv[0]);
		return true;
	}

	Common::FSNode fs(argv[1]);

	if (!fs.isReadable()) {
		debugPrintf("Warning: File %s does not exist or is not readable\n", argv[1]);
		return true;
	}

	Common::SeekableReadStream *saveFile = fs.createReadStream();

	_vm->loadGame(*saveFile);

	delete saveFile;

	return false;
}

bool Debugger::cmdSave(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Saves game to original format.\n");
		debugPrintf("Usage: %s <file path>\n", argv[0]);
		return true;
	}

	Common::FSNode fs(argv[1]);

	if (fs.exists() && !fs.isWritable()) {
		debugPrintf("Warning: File %s is not writable\n", argv[1]);
		return true;
	}

	Common::WriteStream *saveFile = fs.createWriteStream();

	Graphics::Surface thumbnail = _vm->generateThumbnail();

	_vm->saveGame(*saveFile, thumbnail);

	saveFile->finalize();

	thumbnail.free();

	delete saveFile;

	return true;
}

void Debugger::drawBBox(Vector3 start, Vector3 end, View *view, Graphics::Surface *surface, int color) {
	Vector3 bfl = view->calculateScreenPosition(Vector3(start.x, start.y, start.z));
	Vector3 bfr = view->calculateScreenPosition(Vector3(start.x, end.y, start.z));
	Vector3 bbr = view->calculateScreenPosition(Vector3(end.x, end.y, start.z));
	Vector3 bbl = view->calculateScreenPosition(Vector3(end.x, start.y, start.z));

	Vector3 tfl = view->calculateScreenPosition(Vector3(start.x, start.y, end.z));
	Vector3 tfr = view->calculateScreenPosition(Vector3(start.x, end.y, end.z));
	Vector3 tbr = view->calculateScreenPosition(Vector3(end.x, end.y, end.z));
	Vector3 tbl = view->calculateScreenPosition(Vector3(end.x, start.y, end.z));

	surface->drawLine(bfl.x, bfl.y, bfr.x, bfr.y, color);
	surface->drawLine(bfr.x, bfr.y, bbr.x, bbr.y, color);
	surface->drawLine(bbr.x, bbr.y, bbl.x, bbl.y, color);
	surface->drawLine(bbl.x, bbl.y, bfl.x, bfl.y, color);

	surface->drawLine(tfl.x, tfl.y, tfr.x, tfr.y, color);
	surface->drawLine(tfr.x, tfr.y, tbr.x, tbr.y, color);
	surface->drawLine(tbr.x, tbr.y, tbl.x, tbl.y, color);
	surface->drawLine(tbl.x, tbl.y, tfl.x, tfl.y, color);

	surface->drawLine(bfl.x, bfl.y, tfl.x, tfl.y, color);
	surface->drawLine(bfr.x, bfr.y, tfr.x, tfr.y, color);
	surface->drawLine(bbr.x, bbr.y, tbr.x, tbr.y, color);
	surface->drawLine(bbl.x, bbl.y, tbl.x, tbl.y, color);
}

void Debugger::drawSceneObjects() {
	//draw scene objects
	int count = _vm->_sceneObjects->_count;
	if (count > 0) {
		for (int i = 0; i < count; i++) {
			SceneObjects::SceneObject *sceneObject = &_vm->_sceneObjects->_sceneObjects[_vm->_sceneObjects->_sceneObjectsSortedByDistance[i]];

			const BoundingBox &bbox = sceneObject->boundingBox;
			Vector3 a, b;
			bbox.getXYZ(&a.x, &a.y, &a.z, &b.x, &b.y, &b.z);
			Vector3 pos = _vm->_view->calculateScreenPosition(0.5 * (a + b));
			int color;

			if (_viewActorsOnly && sceneObject->type != kSceneObjectTypeActor)
				continue;

			switch (sceneObject->type) {
			case kSceneObjectTypeUnknown:
				break;
			case kSceneObjectTypeActor:
				color = 0x7C00; // 11111 00000 00000;
				drawBBox(a, b, _vm->_view, &_vm->_surfaceFront, color);
				_vm->_surfaceFront.frameRect(sceneObject->screenRectangle, color);
				_vm->_mainFont->drawColor(_vm->_textActorNames->getText(sceneObject->id - kSceneObjectOffsetActors), _vm->_surfaceFront, pos.x, pos.y, color);
				break;
			case kSceneObjectTypeItem:
				color = 0x03E0; // 00000 11111 00000
				char itemText[40];
				drawBBox(a, b, _vm->_view, &_vm->_surfaceFront, color);
				sprintf(itemText, "item %i", sceneObject->id - kSceneObjectOffsetItems);
				_vm->_surfaceFront.frameRect(sceneObject->screenRectangle, color);
				_vm->_mainFont->drawColor(itemText, _vm->_surfaceFront, pos.x, pos.y, color);
				break;
			case kSceneObjectTypeObject:
				color = 0x3DEF; //01111 01111 01111;
				//if (sceneObject->_isObstacle)
				//	color += 0b100000000000000;
				if (sceneObject->isClickable) {
					color = 0x03E0; // 00000 11111 00000;
				}
				drawBBox(a, b, _vm->_view, &_vm->_surfaceFront, color);
				_vm->_surfaceFront.frameRect(sceneObject->screenRectangle, color);
				_vm->_mainFont->drawColor(_vm->_scene->objectGetName(sceneObject->id - kSceneObjectOffsetObjects), _vm->_surfaceFront, pos.x, pos.y, color);
				break;
			}
		}
	}

	if (_viewActorsOnly)
		return;

	//draw regions
	for (int i = 0; i < 10; i++) {
		Regions::Region *region = &_vm->_scene->_regions->_regions[i];
		if (!region->present) continue;
		_vm->_surfaceFront.frameRect(region->rectangle, 0x001F); // 00000 00000 11111
	}

	for (int i = 0; i < 10; i++) {
		Regions::Region *region = &_vm->_scene->_exits->_regions[i];
		if (!region->present) continue;
		_vm->_surfaceFront.frameRect(region->rectangle, 0x7FFF); // 11111 11111 11111
	}

	//draw walkboxes
	for (int i = 0; i < _vm->_scene->_set->_walkboxCount; i++) {
		Set::Walkbox *walkbox = &_vm->_scene->_set->_walkboxes[i];

		for (int j = 0; j < walkbox->vertexCount; j++) {
			Vector3 start = _vm->_view->calculateScreenPosition(walkbox->vertices[j]);
			Vector3 end = _vm->_view->calculateScreenPosition(walkbox->vertices[(j + 1) % walkbox->vertexCount]);
			_vm->_surfaceFront.drawLine(start.x, start.y, end.x, end.y, 0x7FE0); // 11111 11111 00000
			Vector3 pos = _vm->_view->calculateScreenPosition(0.5 * (start + end));
			_vm->_mainFont->drawColor(walkbox->name, _vm->_surfaceFront, pos.x, pos.y, 0x7FE0); // 11111 11111 00000
		}
	}

	// draw lights
	for (int i = 0; i < (int)_vm->_lights->_lights.size(); i++) {
		Light *light = _vm->_lights->_lights[i];
		Matrix4x3 m = light->_matrix;
		m = invertMatrix(m);
		//todo do this properly
		Vector3 posOrigin = m * Vector3(0.0f, 0.0f, 0.0f);
		float t = posOrigin.y;
		posOrigin.y = posOrigin.z;
		posOrigin.z = -t;

		Vector3 posTarget = m * Vector3(0.0f, 0.0f, -100.0f);
		t = posTarget.y;
		posTarget.y = posTarget.z;
		posTarget.z = -t;

		Vector3 size = Vector3(5.0f, 5.0f, 5.0f);
		int colorR = (light->_color.r * 31.0f);
		int colorG = (light->_color.g * 31.0f);
		int colorB = (light->_color.b * 31.0f);
		int color = (colorR << 10) + (colorG << 5) + colorB;

		drawBBox(posOrigin - size, posOrigin + size, _vm->_view, &_vm->_surfaceFront, color);

		Vector3 posOriginT = _vm->_view->calculateScreenPosition(posOrigin);
		Vector3 posTargetT = _vm->_view->calculateScreenPosition(posTarget);
		_vm->_surfaceFront.drawLine(posOriginT.x, posOriginT.y, posTargetT.x, posTargetT.y, color);
		_vm->_mainFont->drawColor(light->_name, _vm->_surfaceFront, posOriginT.x, posOriginT.y, color);
	}

	//draw waypoints
	for (int i = 0; i < _vm->_waypoints->_count; i++) {
		Waypoints::Waypoint *waypoint = &_vm->_waypoints->_waypoints[i];
		if(waypoint->setId != _vm->_scene->getSetId()) {
			continue;
		}
		Vector3 pos = waypoint->position;
		Vector3 size = Vector3(3.0f, 3.0f, 3.0f);
		int color = 0x7FFF; // 11111 11111 11111
		drawBBox(pos - size, pos + size, _vm->_view, &_vm->_surfaceFront, color);
		Vector3 spos = _vm->_view->calculateScreenPosition(pos);
		char waypointText[40];
		sprintf(waypointText, "waypoint %i", i);
		_vm->_mainFont->drawColor(waypointText, _vm->_surfaceFront, spos.x, spos.y, color);
	}

	//draw combat cover waypoints
	for (int i = 0; i < (int)_vm->_combat->_coverWaypoints.size(); i++) {
		Combat::CoverWaypoint *cover = &_vm->_combat->_coverWaypoints[i];
		if (cover->setId != _vm->_scene->getSetId()) {
			continue;
		}
		Vector3 pos = cover->position;
		Vector3 size = Vector3(3.0f, 3.0f, 3.0f);
		int color = 0x7C1F; // 11111 00000 11111
		drawBBox(pos - size, pos + size, _vm->_view, &_vm->_surfaceFront, color);
		Vector3 spos = _vm->_view->calculateScreenPosition(pos);
		char coverText[40];
		sprintf(coverText, "cover %i", i);
		_vm->_mainFont->drawColor(coverText, _vm->_surfaceFront, spos.x, spos.y, color);
	}

	//draw combat flee waypoints
	for (int i = 0; i < (int)_vm->_combat->_fleeWaypoints.size(); i++) {
		Combat::FleeWaypoint *flee = &_vm->_combat->_fleeWaypoints[i];
		if (flee->setId != _vm->_scene->getSetId()) {
			continue;
		}
		Vector3 pos = flee->position;
		Vector3 size = Vector3(3.0f, 3.0f, 3.0f);
		int color = 0x03FF; // 00000 11111 11111
		drawBBox(pos - size, pos + size, _vm->_view, &_vm->_surfaceFront, color);
		Vector3 spos = _vm->_view->calculateScreenPosition(pos);
		char fleeText[40];
		sprintf(fleeText, "flee %i", i);
		_vm->_mainFont->drawColor(fleeText, _vm->_surfaceFront, spos.x, spos.y, color);
	}

#if 0
	//draw aesc
	for (uint i = 0; i < _screenEffects->_entries.size(); i++) {
		ScreenEffects::Entry &entry = _screenEffects->_entries[i];
		int j = 0;
		for (int y = 0; y < entry.height; y++) {
			for (int x = 0; x < entry.width; x++) {
				Common::Rect r((entry.x + x) * 2, (entry.y + y) * 2, (entry.x + x) * 2 + 2, (entry.y + y) * 2 + 2);

				int ec = entry.data[j++];
				Color256 color = entry.palette[ec];
				int bladeToScummVmConstant = 256 / 16;

				Graphics::PixelFormat _pixelFormat = createRGB555();
				int color555 = _pixelFormat.RGBToColor(
					CLIP(color.r * bladeToScummVmConstant, 0, 255),
					CLIP(color.g * bladeToScummVmConstant, 0, 255),
					CLIP(color.b * bladeToScummVmConstant, 0, 255));
				_surfaceFront.fillRect(r, color555);
			}
		}
	}
#endif
}

} // End of namespace BladeRunner
