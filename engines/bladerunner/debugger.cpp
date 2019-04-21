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
#include "bladerunner/fog.h"
#include "bladerunner/game_constants.h"
#include "bladerunner/game_flags.h"
#include "bladerunner/game_info.h"
#include "bladerunner/light.h"
#include "bladerunner/lights.h"
#include "bladerunner/regions.h"
#include "bladerunner/savefile.h"
#include "bladerunner/scene.h"
#include "bladerunner/scene_objects.h"
#include "bladerunner/screen_effects.h"
#include "bladerunner/settings.h"
#include "bladerunner/set.h"
#include "bladerunner/set_effects.h"
#include "bladerunner/text_resource.h"
#include "bladerunner/time.h"
#include "bladerunner/vector.h"
#include "bladerunner/view.h"
#include "bladerunner/vqa_decoder.h"
#include "bladerunner/vqa_player.h"
#include "bladerunner/waypoints.h"
#include "bladerunner/zbuffer.h"
#include "bladerunner/chapters.h"
#include "bladerunner/ui/kia.h"
#include "bladerunner/ui/esper.h"
#include "bladerunner/ui/spinner.h"
#include "bladerunner/ui/elevator.h"
#include "bladerunner/ui/vk.h"
#include "bladerunner/ui/scores.h"
#include "bladerunner/overlays.h"
#include "bladerunner/subtitles.h"


#include "common/debug.h"
#include "common/str.h"

#include "graphics/surface.h"

namespace BladeRunner {

Debugger::Debugger(BladeRunnerEngine *vm) : GUI::Debugger() {
	_vm = vm;

	_isDebuggerOverlay = false;

	_viewActorsOnly = false;
	_viewLights = false;
	_viewFogs = false;
	_viewSceneObjects = false;
	_viewScreenEffects = false;
	_viewObstacles = false;
	_viewRegions = false;
	_viewUI = false;
	_viewWaypoints = false;
	_viewWalkboxes = false;
	_viewZBuffer = false;

	registerCmd("anim", WRAP_METHOD(Debugger, cmdAnimation));
	registerCmd("draw", WRAP_METHOD(Debugger, cmdDraw));
	registerCmd("list", WRAP_METHOD(Debugger, cmdList));
	registerCmd("flag", WRAP_METHOD(Debugger, cmdFlag));
	registerCmd("goal", WRAP_METHOD(Debugger, cmdGoal));
	registerCmd("loop", WRAP_METHOD(Debugger, cmdLoop));
	registerCmd("pos", WRAP_METHOD(Debugger, cmdPosition));
	registerCmd("say", WRAP_METHOD(Debugger, cmdSay));
	registerCmd("scene", WRAP_METHOD(Debugger, cmdScene));
	registerCmd("var", WRAP_METHOD(Debugger, cmdVariable));
	registerCmd("clue", WRAP_METHOD(Debugger, cmdClue));
	registerCmd("timer", WRAP_METHOD(Debugger, cmdTimer));
	registerCmd("friend", WRAP_METHOD(Debugger, cmdFriend));
	registerCmd("load", WRAP_METHOD(Debugger, cmdLoad));
	registerCmd("save", WRAP_METHOD(Debugger, cmdSave));
	registerCmd("overlay", WRAP_METHOD(Debugger, cmdOverlay));
	registerCmd("subtitle", WRAP_METHOD(Debugger, cmdSubtitle));
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
		actor->changeAnimationMode(animationMode, true);
		return false;
	}

	debugPrintf("actorAnimationMode(%i) = %i\n", actorId, actor->getAnimationMode());
	return true;
}

bool Debugger::cmdDraw(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Enables debug rendering of actors, screen effect, fogs, lights, scene objects, obstacles, regions, ui elements, walk boxes, waypoints, zbuffer or disables debug rendering.\n");
		debugPrintf("Usage: %s (act | eff | fog | lit | obj | obstacles | reg | ui | walk | way | zbuf | reset)\n", argv[0]);
		return true;
	}

	Common::String arg = argv[1];
	if (arg == "act") {
		_viewSceneObjects = !_viewSceneObjects;
		_viewActorsOnly = _viewSceneObjects;
		debugPrintf("Drawing actors = %i\n", _viewSceneObjects);
	} else if (arg == "eff") {
		_viewScreenEffects = !_viewScreenEffects;
		debugPrintf("Drawing screen effects = %i\n", _viewScreenEffects);
	} else if (arg == "fog") {
		_viewFogs = !_viewFogs;
		debugPrintf("Drawing fogs = %i\n", _viewFogs);
	} else if (arg == "lit") {
		_viewLights = !_viewLights;
		debugPrintf("Drawing lights = %i\n", _viewLights);
	} else if (arg == "reg") {
		_viewRegions = !_viewRegions;
		debugPrintf("Drawing regions = %i\n", _viewRegions);
	}else if (arg == "obj") {
		_viewSceneObjects = !_viewSceneObjects;
		debugPrintf("Drawing scene objects = %i\n", _viewSceneObjects);
	} else if (arg == "obstacles") {
		_viewObstacles = !_viewObstacles;
		debugPrintf("Drawing obstacles = %i\n", _viewObstacles);
	} else if (arg == "ui") {
		_viewUI = !_viewUI;
		debugPrintf("Drawing UI elements = %i\n", _viewUI);
	} else if (arg == "way") {
		_viewWaypoints = !_viewWaypoints;
		debugPrintf("Drawing waypoints = %i\n", _viewWaypoints);
	} else if (arg == "walk") {
		_viewWalkboxes = !_viewWalkboxes;
		debugPrintf("Drawing walk boxes = %i\n", _viewWalkboxes);
	} else if (arg == "zbuf") {
		_viewZBuffer = !_viewZBuffer;
		debugPrintf("Drawing Z buffer = %i\n", _viewZBuffer);
	} else if (arg == "reset") {
		_viewActorsOnly = false;
		_viewScreenEffects = false;
		_viewFogs = false;
		_viewLights = false;
		_viewObstacles = false;
		_viewRegions = false;
		_viewSceneObjects = false;
		_viewUI = false;
		_viewWaypoints = false;
		_viewWalkboxes = false;
		_viewZBuffer = false;

		debugPrintf("Drawing screen effects = %i\n", _viewScreenEffects);
		debugPrintf("Drawing fogs = %i\n", _viewFogs);
		debugPrintf("Drawing lights = %i\n", _viewLights);
		debugPrintf("Drawing obstacles = %i\n", _viewObstacles);
		debugPrintf("Drawing regions = %i\n", _viewRegions);
		debugPrintf("Drawing scene objects = %i\n", _viewSceneObjects);
		debugPrintf("Drawing UI elements = %i\n", _viewUI);
		debugPrintf("Drawing waypoints = %i\n", _viewWaypoints);
		debugPrintf("Drawing walkboxes = %i\n", _viewWalkboxes);
		debugPrintf("Drawing Z buffer = %i\n", _viewZBuffer);
	}

	_isDebuggerOverlay = _viewSceneObjects | _viewScreenEffects | _viewRegions | _viewLights | _viewFogs | _viewWaypoints | _viewWalkboxes;
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
		debugPrintf("Usage: %s <actorId> [(<setId> <x> <y> <z> <facing>) | <otherActorId>]\n", argv[0]);
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
	if ((actorId >= 0 && actorId < (int)_vm->_gameInfo->getActorCount()) || (actorId == kActorVoiceOver)) {
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
	{ 1, "PS02", 62, 66 },   { 1, "PS03", 63, 67 },  { 1, "PS04", 64, 68 },   { 1, "PS05", 15, 69 },
	{ 1, "PS06", 65, 70 },   { 1, "PS07", 66, 71 },  { 1, "PS09", 67, 72 },   { 1, "PS10", 14, 73 },
	{ 1, "PS11", 14, 74 },   { 1, "PS12", 14, 75 },  { 1, "PS13", 14, 76 },   { 1, "PS14", 68, 77 },
	{ 1, "PS15", 101, 119 }, { 1, "RC01", 69, 78 },  { 1, "RC02", 16, 79 },   { 1, "RC51", 16, 107 },

	{ 2, "AR01", 0, 0 },     { 2, "AR02", 0, 1 },    { 2, "BB01", 20, 2 },    { 2, "BB02", 1, 3 },
	{ 2, "BB03", 21, 4 },    { 2, "BB04", 1, 5 },    { 2, "BB05", 22, 6 },    { 2, "BB06", 1, 7 },
	{ 2, "BB06", 2, 7 },     { 2, "BB07", 2, 8 },    { 2, "BB07", 3, 8 },     { 2, "BB08", 23, 9 },
	{ 2, "BB09", 24, 10 },   { 2, "BB10", 25, 11 },  { 2, "BB11", 26, 12 },   { 2, "BB12", 102, 120 },
	{ 2, "BB51", 1, 104 },   { 2, "CT01", 4, 13 },   { 2, "CT02", 27, 14 },   { 2, "CT03", 5, 15 },
	{ 2, "CT04", 5, 16 },    { 2, "CT05", 28, 17 },  { 2, "CT06", 29, 18 },   { 2, "CT08", 6, 20 },
	{ 2, "CT09", 31, 21 },   { 2, "CT10", 32, 22 },  { 2, "CT11", 33, 23 },   { 2, "CT12", 4, 24 },
	{ 2, "CT51", 6, 105 },   { 2, "DR01", 7, 25 },   { 2, "DR02", 7, 26 },    { 2, "DR03", 34, 27 },
	{ 2, "DR04", 7, 28 },    { 2, "DR05", 35, 29 },  { 2, "DR06", 36, 30 },   { 2, "HC01", 8, 31 },
	{ 2, "HC02", 8, 32 },    { 2, "HC03", 8, 33 },   { 2, "HC04", 8, 106 },   { 2, "HF01", 37, 34 },
	{ 2, "HF02", 38, 35 },   { 2, "HF03", 39, 36 },  { 2, "HF04", 40, 37 },   { 2, "HF05", 41, 38 },
	{ 2, "HF06", 42, 39 },   { 2, "MA01", 49, 48 },  { 2, "MA02", 10, 49 },   { 2, "MA04", 10, 50 },
	{ 2, "MA04", 50, 50 },   { 2, "MA05", 51, 51 },  { 2, "MA06", 52, 52 },   { 2, "MA07", 53, 53 },
	{ 2, "NR01", 54, 54 },   { 2, "NR02", 11, 55 },  { 2, "NR03", 55, 56 },   { 2, "NR04", 12, 57 },
	{ 2, "NR05", 13, 58 },   { 2, "NR06", 56, 59 },  { 2, "NR07", 57, 60 },   { 2, "NR08", 13, 61 },
	{ 2, "NR09", 58, 62 },   { 2, "NR10", 59, 63 },  { 2, "NR11", 60, 64 },   { 2, "PS01", 61, 65 },
	{ 2, "PS02", 62, 66 },   { 2, "PS03", 63, 67 },  { 2, "PS04", 64, 68 },   { 2, "PS05", 15, 69 },
	{ 2, "PS06", 65, 70 },   { 2, "PS07", 66, 71 },  { 2, "PS09", 67, 72 },   { 2, "PS10", 14, 73 },
	{ 2, "PS11", 14, 74 },   { 2, "PS12", 14, 75 },  { 2, "PS13", 14, 76 },   { 2, "PS14", 68, 77 },
	{ 2, "PS15", 101, 119 }, { 2, "RC01", 69, 78 },  { 2, "RC03", 70, 80 },   { 2, "RC04", 71, 81 },
	{ 2, "TB02", 17, 82 },   { 2, "TB05", 72, 84 },  { 2, "TB06", 73, 85 },   { 2, "TB07", 18, 108 },
	{ 2, "UG01", 74, 86 },   { 2, "UG02", 75, 87 },  { 2, "UG03", 76, 88 },   { 2, "UG04", 77, 89 },
	{ 2, "UG06", 79, 91 },   { 2, "UG10", 83, 95 },

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
	if (argc != 0 && argc > 4) {
		debugPrintf("Changes set and scene.\n");
		debugPrintf("Usage: %s [(<chapterId> <setId> <sceneId>) | (<chapterId> <sceneName>) | <sceneName>]\n", argv[0]);
		return true;
	}

	// scene <chapterId> <setId> <sceneId>
	if (argc == 4 && Common::isDigit(*argv[1]) && Common::isDigit(*argv[2]) && Common::isDigit(*argv[3])) {
		int chapterId = atoi(argv[1]);
		int setId = atoi(argv[2]);
		int sceneId = atoi(argv[3]);

		if (chapterId < 1 || chapterId > 5) {
			debugPrintf("chapterID must be between 1 and 5\n");
			return true;
		}

		int chapterIdNormalized = chapterId;

		if (chapterId == 3 || chapterId == 5) {
			chapterIdNormalized = chapterId - 1;
		}

		// Sanity check
		uint i;
		for (i = 0; sceneList[i].chapter != 0; i++) {
			if (sceneList[i].chapter == chapterIdNormalized &&
			    sceneList[i].set == setId &&
			    sceneList[i].scene == sceneId
			) {
				break;
			}
		}

		if (sceneList[i].chapter == 0) { // end of list
			debugPrintf("chapterId, setId and sceneId combination is not valid.\n");
			return true;
		}

		if (chapterId != _vm->_settings->getChapter()) {
			_vm->_settings->setChapter(chapterId);
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
				debugPrintf("chapterId must be between 1 and 5\n");
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
			if (sceneList[i].chapter == chapterIdNormalized && sceneName.equalsIgnoreCase(sceneList[i].name))
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

	int chapterId = _vm->_settings->getChapter();
	int chapterIdNormalized = chapterId;

	if (chapterId == 3 || chapterId == 5) {
		chapterIdNormalized = chapterId - 1;
	}

	uint i;
	for (i = 0; sceneList[i].chapter != 0; i++) {
		if (sceneList[i].chapter == chapterIdNormalized && sceneList[i].set == _vm->_scene->getSetId()
				&& sceneList[i].scene == _vm->_scene->getSceneId())
			break;
	}

	debugPrintf("chapterID = %i\nsetId = %i\nsceneId = %i\nsceneName = '%s'\n", _vm->_settings->getChapter(), _vm->_scene->getSetId(),
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

bool Debugger::cmdClue(int argc, const char **argv) {
	if (argc != 3 && argc != 4) {
		debugPrintf("Get or changes clue for an actor.\n");
		debugPrintf("Usage: %s <actorId> <clueId> [<value>]\n", argv[0]);
		return true;
	}

	int actorId = atoi(argv[1]);

	Actor *actor = nullptr;
	if ((actorId >= 0 && actorId < (int)_vm->_gameInfo->getActorCount()) || (actorId == kActorVoiceOver)) {
		actor = _vm->_actors[actorId];
	}

	if (actor == nullptr) {
		debugPrintf("Unknown actor %i\n", actorId);
		return true;
	}

	int clueId = atoi(argv[2]);

	// TODO: check clueId

	if (argc == 4) {
		int value = atoi(argv[3]);
		if (value != 0) {
			actor->acquireClue(clueId, true, -1);
		} else {
			actor->loseClue(clueId);
		}
	}
	debugPrintf("actorClue(%i, %i) = %i\n", actorId, clueId, actor->hasClue(clueId));

	return true;
}

bool Debugger::cmdTimer(int argc, const char **argv) {
	if (argc != 2 && argc != 4) {
		debugPrintf("Get or changes timers for an actor.\n");
		debugPrintf("Usage: %s <actorId> [<timer> <value>]\n", argv[0]);
		return true;
	}

	int actorId = atoi(argv[1]);

	Actor *actor = nullptr;
	if ((actorId >= 0 && actorId < (int)_vm->_gameInfo->getActorCount()) || (actorId == kActorVoiceOver)) {
		actor = _vm->_actors[actorId];
	}

	if (actor == nullptr) {
		debugPrintf("Unknown actor %i\n", actorId);
		return true;
	}

	if (argc == 4) {
		int timer = atoi(argv[2]);
		int value = atoi(argv[3]);

		if (timer < 0 || timer > 6) {
			debugPrintf("Timer must be [0..6]");
			return true;
		}

		if (value == 0) {
			actor->timerReset(timer);
		} else {
			actor->timerStart(timer, value);
		}
	}

	for (int i = 0; i < 7; ++i) {
		debugPrintf("actorTimer(%i, %i) = %i ms\n", actorId, i, actor->timerLeft(i));
	}

	return true;
}

bool Debugger::cmdFriend(int argc, const char **argv) {
	if (argc != 3 && argc != 4) {
		debugPrintf("Get or changes friendliness for an actor towards another actor.\n");
		debugPrintf("Usage: %s <actorId> <otherActorId> [<value>]\n", argv[0]);
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

	int otherActorId = atoi(argv[2]);

	if (otherActorId < 0 && otherActorId >= (int)_vm->_gameInfo->getActorCount()) {
		debugPrintf("Unknown actor %i\n", otherActorId);
	}

	if (argc == 4) {
		int value = atoi(argv[3]);

		if (value < 0 || value > 100) {
			debugPrintf("Value must be [0..100]");
			return true;
		}

		actor->setFriendlinessToOther(otherActorId, value);
	}

	debugPrintf("actorFriendliness(%i, %i) = %i\n", actorId, otherActorId, actor->getFriendlinessToOther(otherActorId));

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

	_vm->_time->pause();
	_vm->saveGame(*saveFile, thumbnail);
	_vm->_time->resume();

	saveFile->finalize();

	thumbnail.free();

	delete saveFile;

	return false;
}

const struct OverlayAndScenesVQAsList {
	int resourceId;
	const char *name;
	bool isOverlayVQA; // else it is a scene VQA
} overlaysList[] = {
	{ 1, "MA04OVR2", true }, { 1, "PS10", false },    { 1, "MA01", false },    { 1, "RC01", false },    { 1, "PS01", false },
	{ 1, "CT01", false },    { 1, "PS11", false },    { 1, "RC51", false },    { 1, "MA02", false },    { 1, "RC02", false },
	{ 1, "PS02", false },    { 1, "CT02", false },    { 1, "PS12", false },    { 1, "CT12", false },    { 1, "PS03", false },
	{ 1, "CT03", false },    { 1, "PS13", false },    { 1, "MA04", false },    { 1, "PS04", false },    { 1, "CT04", false },
	{ 1, "PS14", false },    { 1, "CT01SPNR", true }, { 1, "MA05", false },    { 1, "PS05", false },    { 1, "CT05", false },
	{ 1, "PS15", false },    { 1, "MA06", false },    { 1, "PS06", false },    { 1, "CT06", false },    { 1, "MA02OVER", true },
	{ 1, "CT02OVER", true }, { 1, "MA07", false },    { 1, "PS07", false },    { 1, "CT07", false },    { 1, "PS09", false },
	{ 1, "MA04OVER", true }, { 1, "PS05OVER", true }, { 1, "CT05OVER", true },

	{ 2, "BB10OVR1", true }, { 2, "BB10OVR2", true }, { 2, "BB10OVR3", true }, { 2, "BB10OVR4", true }, { 2, "BB10OVR5", true },
	{ 2, "BB10_2", false },  { 2, "UG10_2", false },  { 2, "NR10_2", false },  { 2, "PS10_2", false },  { 2, "CT10_2", false },
	{ 2, "MA01_2", false },  { 2, "BB01_2", false },  { 2, "HC01_2", false },  { 2, "RC01_2", false },  { 2, "HF01_2", false },
	{ 2, "UG01_2", false },  { 2, "AR01_2", false },  { 2, "DR01_2", false },  { 2, "NR01_2", false },  { 2, "PS01_2", false },
	{ 2, "CT01_2", false },  { 2, "BB11_2", false },  { 2, "NR11_2", false },  { 2, "PS11_2", false },  { 2, "CT11_2", false },
	{ 2, "BB51_2", false },  { 2, "CT51_2", false },  { 2, "MA02_2", false },  { 2, "BB02_2", false },  { 2, "TB02_2", false },
	{ 2, "HC02_2", false },  { 2, "HF02_2", false },  { 2, "UG02_2", false },  { 2, "AR02_2", false },  { 2, "DR02_2", false },
	{ 2, "NR02_2", false },  { 2, "PS02_2", false },  { 2, "CT02_2", false },  { 2, "BB12_2", false },  { 2, "PS12_2", false },
	{ 2, "CT12_2", false },  { 2, "MA04OVR2", true }, { 2, "BB03_2", false },  { 2, "HC03_2", false },  { 2, "RC03_2", false },
	{ 2, "HF03_2", false },  { 2, "UG03_2", false },  { 2, "DR03_2", false },  { 2, "NR03_2", false },  { 2, "PS03_2", false },
	{ 2, "CT03_2", false },  { 2, "PS13_2", false },  { 2, "MA04_2", false },  { 2, "BB04_2", false },  { 2, "HC04_2", false },
	{ 2, "RC04_2", false },  { 2, "HF04_2", false },  { 2, "UG04_2", false },  { 2, "DR04_2", false },  { 2, "NR04_2", false },
	{ 2, "PS04_2", false },  { 2, "CT04_2", false },  { 2, "PS14_2", false },  { 2, "DR06OVR2", true }, { 2, "MA05_2", false },
	{ 2, "BB05_2", false },  { 2, "TB05_2", false },  { 2, "HF05_2", false },  { 2, "DR05_2", false },  { 2, "NR05_2", false },
	{ 2, "PS05_2", false },  { 2, "CT05_2", false },  { 2, "PS15_2", false },  { 2, "MA06_2", false },  { 2, "BB06_2", false },
	{ 2, "TB06_2", false },  { 2, "HF06_2", false },  { 2, "UG06_2", false },  { 2, "DR06_2", false },  { 2, "NR06_2", false },
	{ 2, "PS06_2", false },  { 2, "CT06_2", false },  { 2, "MA07_2", false },  { 2, "BB07_2", false },  { 2, "TB07_2", false },
	{ 2, "NR07_2", false },  { 2, "PS07_2", false },  { 2, "BB08_2", false },  { 2, "NR08_2", false },  { 2, "CT08_2", false },
	{ 2, "BB09_2", false },  { 2, "NR09_2", false },  { 2, "PS09_2", false },  { 2, "CT09_2", false },  { 2, "NR11OVER", true },
	{ 2, "CT01SPNR", true }, { 2, "MA02OVER", true }, { 2, "CT02OVER", true }, { 2, "BB12OVER", true }, { 2, "MA04OVER", true },
	{ 2, "DR04OVER", true }, { 2, "NR04OVER", true }, { 2, "BB05OVER", true }, { 2, "DR05OVER", true }, { 2, "PS05OVER", true },
	{ 2, "CT05OVER", true }, { 2, "BB06OVER", true }, { 2, "DR06OVER", true }, { 2, "BB07OVER", true }, { 2, "BB08OVER", true },

	{ 3, "UG10_3", false },  { 3, "NR10_3", false },  { 3, "CT10_3", false },  { 3, "BB01_3", false },  { 3, "HC01_3", false },
	{ 3, "RC01_3", false },  { 3, "HF01_3", false },  { 3, "UG01_3", false },  { 3, "KP01_3", false },  { 3, "AR01_3", false },
	{ 3, "DR01_3", false },  { 3, "NR01_3", false },  { 3, "CT01_3", false },  { 3, "NR11_3", false },  { 3, "CT11_3", false },
	{ 3, "BB51_3", false },  { 3, "RC51_3", false },  { 3, "CT51_3", false },  { 3, "MA02_3", false },  { 3, "BB02_3", false },
	{ 3, "TB02_3", false },  { 3, "HC02_3", false },  { 3, "RC02_3", false },  { 3, "HF02_3", false },  { 3, "UG02_3", false },
	{ 3, "KP02_3", false },  { 3, "AR02_3", false },  { 3, "DR02_3", false },  { 3, "NR02_3", false },  { 3, "CT02_3", false },
	{ 3, "UG12_3", false },  { 3, "CT12_3", false },  { 3, "MA04OVR2", true }, { 3, "BB03_3", false },  { 3, "TB03_3", false },
	{ 3, "HC03_3", false },  { 3, "RC03_3", false },  { 3, "HF03_3", false },  { 3, "UG03_3", false },  { 3, "KP03_3", false },
	{ 3, "DR03_3", false },  { 3, "NR03_3", false },  { 3, "CT03_3", false },  { 3, "UG13_3", false },  { 3, "MA04_3", false },
	{ 3, "BB04_3", false },  { 3, "HC04_3", false },  { 3, "RC04_3", false },  { 3, "HF04_3", false },  { 3, "UG04_3", false },
	{ 3, "KP04_3", false },  { 3, "DR04_3", false },  { 3, "NR04_3", false },  { 3, "CT04_3", false },  { 3, "UG14_3", false },
	{ 3, "PS14_3", false },  { 3, "DR06OVR2", true }, { 3, "MA05_3", false },  { 3, "HF05_3", false },  { 3, "UG05_3", false },
	{ 3, "KP05_3", false },  { 3, "DR05_3", false },  { 3, "NR05_3", false },  { 3, "CT05_3", false },  { 3, "UG15_3", false },
	{ 3, "MA06_3", false },  { 3, "HF06_3", false },  { 3, "UG06_3", false },  { 3, "KP06_3", false },  { 3, "DR06_3", false },
	{ 3, "NR06_3", false },  { 3, "CT06_3", false },  { 3, "UG16_3", false },  { 3, "UG18OVR2", true }, { 3, "UG19OVR1", true },
	{ 3, "MA07_3", false },  { 3, "TB07_3", false },  { 3, "HF07_3", false },  { 3, "UG07_3", false },  { 3, "KP07_3", false },
	{ 3, "NR07_3", false },  { 3, "UG17_3", false },  { 3, "UG08_3", false },  { 3, "NR08_3", false },  { 3, "CT08_3", false },
	{ 3, "UG18_3", false },  { 3, "UG09_3", false },  { 3, "NR09_3", false },  { 3, "PS09_3", false },  { 3, "CT09_3", false },
	{ 3, "UG19_3", false },  { 3, "NR11OVER", true }, { 3, "CT01SPNR", true }, { 3, "MA02OVER", true }, { 3, "CT02OVER", true },
	{ 3, "MA04OVER", true }, { 3, "DR04OVER", true }, { 3, "NR04OVER", true }, { 3, "UG14OVER", true }, { 3, "DR05OVER", true },
	{ 3, "CT05OVER", true }, { 3, "UG15OVER", true }, { 3, "DR06OVER", true }, { 3, "UG17OVER", true }, { 3, "UG18OVER", true },

	{ 6, "VKLUCY", true },   { 6, "VKRUNC", true },   { 6, "KIA_CLUE", false },{ 6, "KIA_INGM", false }, { 6, "KIA_CRIM", false },
	{ 6, "KIA_SUSP", false },{ 6, "HC01ESP1", false },{ 6, "HC01ESP2", false },{ 6, "HC01ESP3", false }, { 6, "RC02ESP1", false },
	{ 6, "HC02ESP2", false },{ 6, "RC02ESP2", false },{ 6, "HC02ESP3", false },{ 6, "RC02ESP3", false }, { 6, "HC02ESP4", false },
	{ 6, "RC02ESP4", false },{ 6, "HC02ESP5", false },{ 6, "RC02ESP5", false },{ 6, "RC02ESP6", false }, { 6, "RC02ESP7", false },
	{ 6, "TB06ESP1", false },{ 6, "KP06ESP1", false },{ 6, "NR06ESP1", false },{ 6, "TB06ESP2", false }, { 6, "KP06ESP2", false },
	{ 6, "NR06ESP2", false },{ 6, "TB06ESP3", false },{ 6, "KP06ESP3", false },{ 6, "NR07ESP1", false }, { 6, "TB06ESP4", false },
	{ 6, "KP06ESP4", false },{ 6, "NR07ESP2", false },{ 6, "SPINNER", false }, { 6, "KIAOVER", false },  { 6, "VK", false },
	{ 6, "VKKASH", true },   { 6, "PS02ELEV", false },{ 6, "ESPER", false },   { 6, "VKDEKT", true },   { 6, "MA06ELEV", false },
	{ 6, "VKBOB", true },    { 6, "SCORE", false },

	{ 0, NULL, false }
};

/**
* Will use overlay videos that the game has loaded for the scene
* at the time of running the command
* or otherwise will attempt to load the specified overlay to the scene,
* if it exists in the currently loaded (VQAx, MODE) MIX resources.
* Use "overlay reset" to clear up all loaded overlays (and/or custom scene video)
*
* Note: Loading MODE.MIX here (and a VQA from it) may lead to buggy results,
*       if the player then invokes and closes an actual game mode (KIA, ESPER, GPS etc).
*       This is because the game itself will unload MODE.MIX when closing the game mode
*       and that can lead to a assertion fault for a missing file handle.
*       A viable solution would be to have MODE.MIX loaded all the time,
*       but that is unnecessary since a developer could just uncomment a few lines below
*       (look for "force-load MODE.MIX") and make use of it with caution, if needed.
*/
bool Debugger::cmdOverlay(int argc, const char **argv) {
	bool invalidSyntax = false;
	bool modeMixOverlaysAvailableFlg = false;
	int chapterIdOverlaysAvailableInt = -1;

	if (_vm->_kia->isOpen()
		|| _vm->_esper->isOpen()
		|| _vm->_spinner->isOpen()
		|| _vm->_elevator->isOpen()
		|| _vm->_vk->isOpen()
		|| _vm->_scores->isOpen()
	) {
		debugPrintf("Sorry, playing custom overlays in KIA, ESPER, Voigt-Kampff, Spinner GPS,\nScores or Elevator mode is not supported\n");
		return true;
	}

	if (argc != 1 && argc != 2 && argc != 3 && argc != 5) {
		invalidSyntax = true;
	}

	if (_vm->_chapters->hasOpenResources()) {
		chapterIdOverlaysAvailableInt = MIN(_vm->_chapters->currentResourceId(), 3);
	}
	if (chapterIdOverlaysAvailableInt == -1) {
		debugPrintf("No available open resources to load VQAs from.\n Giving up.\n");
		return true;
	}

	// Normally, don't force-load the MODE.MIX resource
	if (!_vm->isArchiveOpen("MODE.MIX")) {
//		if (_vm->openArchive("MODE.MIX") { // Note: This will force-load MODE.MIX. Use with caution!
//			debugPrintf("Warning: MODE.MIX resources were force-loaded.\n Please, don't use game's menu modes (KIA, ESPER, Voigt-Kampff, Spinner GPS, Scores or Elevator) before executing an \"%s reset\" from the debugger!\n", argv[0]);
//			modeMixOverlaysAvailableFlg = true;
//		}
	} else {
		modeMixOverlaysAvailableFlg = true;
	}

	if (argc == 1) {
		// print info for all overlays loaded for the scene
		uint8 countOfLoadedOverlaysInScene = 0;
		debugPrintf("name animationId startFrame endFrame\n");

		for (int i = 0; i < _vm->_overlays->kOverlayVideos; ++i) {
			if (_vm->_overlays->_videos[i].loaded) {
				countOfLoadedOverlaysInScene++;
				VQADecoder::LoopInfo &loopInfo =_vm->_overlays->_videos[i].vqaPlayer->_decoder._loopInfo;
				for (int j = 0; j < loopInfo.loopCount; ++j) {
					debugPrintf("%s %2d %4d %4d\n", _vm->_overlays->_videos[i].name.c_str(), j, loopInfo.loops[j].begin, loopInfo.loops[j].end);
				}
			}
		}

		if ( countOfLoadedOverlaysInScene > 0) {
			debugPrintf("  ** %d overlays are loaded in scene **\n", countOfLoadedOverlaysInScene);
		} else {
			debugPrintf("  ** No overlays loaded in scene **\n");
		}

		return true;
	}

	if (argc == 2) {
		Common::String argName = argv[1];

		if (argName == "reset") {
		// Reset (remove) the overlays loaded for the scene
			_vm->_overlays->removeAll();
			// And return to original VQA for this scene
			const Common::String origSceneName = _vm->_gameInfo->getSceneName(_vm->_scene->_sceneId);

			Common::String origVqaName;
			if (chapterIdOverlaysAvailableInt == 1) {
				origVqaName = Common::String::format("%s.VQA", origSceneName.c_str());
			} else {
				origVqaName = Common::String::format("%s_%d.VQA", origSceneName.c_str(), chapterIdOverlaysAvailableInt);
			}

			if (_vm->_scene->_vqaPlayer != nullptr) {
				delete _vm->_scene->_vqaPlayer;
			}

			_vm->_scene->_vqaPlayer = new VQAPlayer(_vm, &_vm->_surfaceBack, origVqaName);
			if (!_vm->_scene->_vqaPlayer->open()) {
				debugPrintf("Error: Could not open player while reseting\nto scene VQA named: %s!\n", (origVqaName + ".VQA").c_str());
				return true;
			}
			_vm->_scene->startDefaultLoop();
			_vm->_scene->advanceFrame();


		} else if (argName == "avail") {
		// List the available overlays in the loaded resources
			const uint dispColCount = 5;
			uint colCountIter = 0;
			uint16 itemIter = 0;

			debugPrintf("Available overlays in the loaded resources:\n");
			for (itemIter = 0; overlaysList[itemIter].resourceId != 0; ++itemIter) {
				if ( (overlaysList[itemIter].resourceId == chapterIdOverlaysAvailableInt)
					|| ( modeMixOverlaysAvailableFlg && overlaysList[itemIter].resourceId == 6)
				) {
					debugPrintf("%s ", overlaysList[itemIter].name);
					colCountIter = (colCountIter + 1) % dispColCount;
					if ( colCountIter == 0) {
						debugPrintf("\n");
					}
				}
			}
			// final new line if needed
			if ( colCountIter % dispColCount != 0) {
				debugPrintf("\n");
			}
			if (!modeMixOverlaysAvailableFlg) {
				debugPrintf("Note: MODE.MIX resources are currently not loaded.\n");
			}

		} else if (argName.size() > 12) {
			debugPrintf("The specified name is too long. It should be up to 12 characters.\n");
			invalidSyntax = true;
		} else {
			debugPrintf("Invalid command usage\n");
			invalidSyntax = true;
		}
	}

	if (argc == 3 || argc == 5) {
		Common::String overlayName = argv[1];
		overlayName.toUppercase();

		int overlayAnimationId = atoi(argv[2]);
		bool loopForever = false;
		LoopSetModes startNowFlag = kLoopSetModeEnqueue;

		if (argc == 5 && atoi(argv[3]) != 0) {
			loopForever = true;
		}

		if (argc == 5 && atoi(argv[4]) != 0) {
			startNowFlag = kLoopSetModeImmediate;
		}

		if (overlayAnimationId < 0) {
			debugPrintf("Animation id value must be >= 0!\n");
			return true;
		}

		// Check if specified overlay name exists AND is available
		uint16 itemIter = 0;
		for (itemIter = 0; overlaysList[itemIter].resourceId != 0; ++itemIter) {
			if ( (overlaysList[itemIter].resourceId == chapterIdOverlaysAvailableInt)
				|| ( modeMixOverlaysAvailableFlg && overlaysList[itemIter].resourceId == 6)
			) {
				if (strcmp(overlaysList[itemIter].name, overlayName.c_str()) == 0){
					break;
				}
			}
		}
		if (overlaysList[itemIter].resourceId == 0 ) {
			debugPrintf("No available resource was found by that name!\nPerhaps it exists in another chapter.\n");
			return true;
		}

		if (overlaysList[itemIter].isOverlayVQA) {
			//
			// Attempt to load the overlay in an empty slot
			// even if it's not already loaded for the scene (in _vm->_overlays->_videos)
			int overlayVideoIdx = _vm->_overlays->play(overlayName, overlayAnimationId, loopForever, startNowFlag, 0);
			if( overlayVideoIdx == -1 ) {
				debugPrintf("Could not load the overlay animation: %s in this scene. Try reseting overlays first to free up slots!\n", overlayName.c_str());
			} else {
				debugPrintf("Loading overlay animation: %s...\n", overlayName.c_str());

				VQADecoder::LoopInfo &loopInfo =_vm->_overlays->_videos[overlayVideoIdx].vqaPlayer->_decoder._loopInfo;
				int overlayAnimationLoopCount = loopInfo.loopCount;
				if (overlayAnimationLoopCount == 0) {
					debugPrintf("Error: No valid loops were found for overlay animation named: %s!\n", overlayName.c_str());
					_vm->_overlays->remove(overlayName.c_str());
				} else if (overlayAnimationId >= overlayAnimationLoopCount) {
					debugPrintf("Invalid loop id: %d for overlay animation: %s. Try from 0 to %d.\n",  overlayAnimationId, overlayName.c_str(), overlayAnimationLoopCount-1);
				} else {
					// print info about available loops too
					debugPrintf("Animation: %s loaded. Running loop %d...\n", overlayName.c_str(), overlayAnimationId);
					for (int j = 0; j < overlayAnimationLoopCount; ++j) {
						debugPrintf("%s %2d %4d %4d\n", _vm->_overlays->_videos[overlayVideoIdx].name.c_str(), j, loopInfo.loops[j].begin, loopInfo.loops[j].end);
					}
				}
			}
		} else {
			if (_vm->_scene->_vqaPlayer != nullptr) {
				delete _vm->_scene->_vqaPlayer;
			}
			_vm->_scene->_vqaPlayer = new VQAPlayer(_vm, &_vm->_surfaceBack, overlayName + ".VQA");
			if (!_vm->_scene->_vqaPlayer->open()) {
				debugPrintf("Error: Could not open player for scene VQA named: %s!\n", (overlayName + ".VQA").c_str());
				return true;
			}

			VQADecoder::LoopInfo &loopInfo =_vm->_scene->_vqaPlayer->_decoder._loopInfo;
			int sceneAnimationLoopCount = loopInfo.loopCount;
			if (sceneAnimationLoopCount == 0) {
				debugPrintf("Error: No valid loops were found for scene animation named: %s!\n", (overlayName + ".VQA").c_str());
			} else if (overlayAnimationId >= sceneAnimationLoopCount) {
				debugPrintf("Invalid loop id: %d for scene animation: %s. Try from 0 to %d.\n",  overlayAnimationId, overlayName.c_str(), sceneAnimationLoopCount-1);
			} else {
				// ignore the specified loopForever and startNow flags
				// just do a kSceneLoopModeOnce, without immediate start
				_vm->_scene->loopStartSpecial(kSceneLoopModeOnce, overlayAnimationId, false);
				debugPrintf("Scene animation: %s loaded. Running loop %d...\n", overlayName.c_str(), overlayAnimationId);
				for (int j = 0; j < sceneAnimationLoopCount; ++j) {
					debugPrintf("%s %2d %4d %4d\n", overlayName.c_str(), j, loopInfo.loops[j].begin, loopInfo.loops[j].end);
				}
			}
		}
	}

	if (invalidSyntax) {
		debugPrintf("Load, list, play or reset (clear) loaded overlay animations.\nValues for loopForever and startNow are boolean.\n");
		debugPrintf("Usage: %s [[<name> <animationId> [<loopForever> <startNow>]] | avail | reset ]\n", argv[0]);
	}
	return true;
}

/**
*
* Show an explicitly specified string as a subtitle
*/
bool Debugger::cmdSubtitle(int argc, const char **argv) {
	bool invalidSyntax = false;

	if (argc != 2) {
		invalidSyntax = true;
	} else {
		Common::String subtitleText = argv[1];
		if (subtitleText == "reset") {
			_vm->_subtitles->setGameSubsText("", false);
		} else {
			debugPrintf("Showing text: %s\n", subtitleText.c_str());
			_vm->_subtitles->setGameSubsText(subtitleText, true);
			_vm->_subtitles->show();
		}
	}

	if (invalidSyntax) {
		debugPrintf("Show specified text as subtitle or clear the current subtitle (with the reset option).\n");
		debugPrintf("Use double quotes to encapsulate the text.\n");
		debugPrintf("Usage: %s (\"<text_to_display>\" | reset)\n", argv[0]);
	}
	return true;

}

/**
*
* Similar to draw but only list items instead of drawing
* Maybe keep this separate from the draw command, even though some code gets repeated here
* DONE: Provide more info (bbox or position)
* DONE: For actors show current goal too.
* TODO: For items provide friendly name (enum in constants!)
* DONE: Split items in items, actors and objects?
*/
bool Debugger::cmdList(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Enables debug listing of actors, scene objects, items, waypoints, regions, lights, fogs and walk-boxes.\n");
		debugPrintf("Usage: %s (act | obj | items | way | reg | lit | fog | walk )\n", argv[0]);
		return true;
	}

	Common::String arg = argv[1];
	if (arg == "act") {
		debugPrintf("Listing scene actors: \n");
		int count = 0;
		for (int i = 0; i < _vm->_sceneObjects->_count; i++) {
			SceneObjects::SceneObject *sceneObject = &_vm->_sceneObjects->_sceneObjects[_vm->_sceneObjects->_sceneObjectsSortedByDistance[i]];

			if (sceneObject->type == kSceneObjectTypeActor) {
				debugPrintf("%02d. %s (Clk: %s, Trg: %s, Prs: %s, Obs: %s, Mvg: %s), Goal: %d, Pos(%02.2f,%02.2f,%02.2f)\n",
				             count, _vm->_textActorNames->getText(sceneObject->id - kSceneObjectOffsetActors),
				             sceneObject->isClickable? "T" : "F",
				             sceneObject->isTarget?    "T" : "F",
				             sceneObject->isPresent?   "T" : "F",
				             sceneObject->isObstacle?  "T" : "F",
				             sceneObject->isMoving?    "T" : "F",
				             _vm->_actors[sceneObject->id - kSceneObjectOffsetActors]->getGoal(),
				             _vm->_actors[sceneObject->id - kSceneObjectOffsetActors]->getPosition().x,
				             _vm->_actors[sceneObject->id - kSceneObjectOffsetActors]->getPosition().y,
				             _vm->_actors[sceneObject->id - kSceneObjectOffsetActors]->getPosition().z);
				++count;
			}
		}
		debugPrintf("%d actors were found in scene.\n", count);
	} else if (arg == "obj") {
		debugPrintf("Listing scene objects: \n");
		int count = 0;
		for (int i = 0; i < _vm->_sceneObjects->_count; i++) {
			SceneObjects::SceneObject *sceneObject = &_vm->_sceneObjects->_sceneObjects[_vm->_sceneObjects->_sceneObjectsSortedByDistance[i]];
			const BoundingBox &bbox = sceneObject->boundingBox;
			Vector3 a, b;
			bbox.getXYZ(&a.x, &a.y, &a.z, &b.x, &b.y, &b.z);
			Vector3 pos = _vm->_view->calculateScreenPosition(0.5 * (a + b));

			if (sceneObject->type == kSceneObjectTypeUnknown) {
				debugPrintf("%02d. Unknown object type\n", count);
				++count;
			} else if (sceneObject->type == kSceneObjectTypeObject) {
				debugPrintf("%02d. %s (Clk: %s, Trg: %s, Prs: %s, Obs: %s, Mvg: %s), Pos(%02.2f,%02.2f,%02.2f)\n     Bbox(%02.2f,%02.2f,%02.2f) ~ (%02.2f,%02.2f,%02.2f)\n",
				             count, _vm->_scene->objectGetName(sceneObject->id - kSceneObjectOffsetObjects).c_str(),
				             sceneObject->isClickable? "T" : "F",
				             sceneObject->isTarget?    "T" : "F",
				             sceneObject->isPresent?   "T" : "F",
				             sceneObject->isObstacle?  "T" : "F",
				             sceneObject->isMoving?    "T" : "F",
				             pos.x, pos.y, pos.z,
				             a.x, a.y, a.z, b.x, b.y, b.z);
				++count;
			}
		}
		debugPrintf("%d objects were found in scene.\n", count);
	} else if (arg == "items") {
		debugPrintf("Listing scene items: \n");
		int count = 0;
		for (int i = 0; i < _vm->_sceneObjects->_count; i++) {
			SceneObjects::SceneObject *sceneObject = &_vm->_sceneObjects->_sceneObjects[_vm->_sceneObjects->_sceneObjectsSortedByDistance[i]];

			if (sceneObject->type == kSceneObjectTypeItem) {
				const BoundingBox &bbox = sceneObject->boundingBox;
				Vector3 a, b;
				bbox.getXYZ(&a.x, &a.y, &a.z, &b.x, &b.y, &b.z);
				Vector3 pos = _vm->_view->calculateScreenPosition(0.5 * (a + b));
				char itemText[40];
				sprintf(itemText, "item %i", sceneObject->id - kSceneObjectOffsetItems);
				debugPrintf("%02d. %s (Clk: %s, Trg: %s, Prs: %s, Obs: %s, Mvg: %s), Pos(%02.2f,%02.2f,%02.2f)\n    Bbox(%02.2f,%02.2f,%02.2f) ~ (%02.2f,%02.2f,%02.2f)\n",
				             count, itemText,
				             sceneObject->isClickable? "T" : "F",
				             sceneObject->isTarget?    "T" : "F",
				             sceneObject->isPresent?   "T" : "F",
				             sceneObject->isObstacle?  "T" : "F",
				             sceneObject->isMoving?    "T" : "F",
				             pos.x, pos.y, pos.z,
				             a.x, a.y, a.z, b.x, b.y, b.z);
				++count;
			}
		}
		debugPrintf("%d items were found in scene.\n", count);
	} else if (arg == "reg") {
		debugPrintf("Listing regions: \n");
		int count = 0;
		//list regions
		for (int i = 0; i < 10; i++) {
			Regions::Region *region = &_vm->_scene->_regions->_regions[i];
			if (!region->present) continue;
			debugPrintf("%02d. Region slot: %d\n", count, i);
			++count;
		}

		//list exits
		for (int i = 0; i < 10; i++) {
			Regions::Region *region = &_vm->_scene->_exits->_regions[i];
			if (!region->present) continue;
			debugPrintf("%02d. Exit slot: %d\n", count, i);
			++count;
		}
		debugPrintf("%d regions were found in scene.\n", count);
	} else if (arg == "way") {
		debugPrintf("Listing waypoints: \n");
		int count = 0;
		for (int i = 0; i < _vm->_waypoints->_count; i++) {
			Waypoints::Waypoint *waypoint = &_vm->_waypoints->_waypoints[i];
			if(waypoint->setId != _vm->_scene->getSetId()) {
				continue;
			}
			char waypointText[40];
			sprintf(waypointText, "waypoint %i", i);
			debugPrintf("%02d. %s\n", count, waypointText);
			++count;
		}

		// list combat cover waypoints
		for (int i = 0; i < (int)_vm->_combat->_coverWaypoints.size(); i++) {
			Combat::CoverWaypoint *cover = &_vm->_combat->_coverWaypoints[i];
			if (cover->setId != _vm->_scene->getSetId()) {
				continue;
			}
			char coverText[40];
			sprintf(coverText, "cover %i", i);
			debugPrintf("%02d. %s\n", count, coverText);
			++count;
		}

		// list combat flee waypoints
		for (int i = 0; i < (int)_vm->_combat->_fleeWaypoints.size(); i++) {
			Combat::FleeWaypoint *flee = &_vm->_combat->_fleeWaypoints[i];
			if (flee->setId != _vm->_scene->getSetId()) {
				continue;
			}
			char fleeText[40];
			sprintf(fleeText, "flee %i", i);
			debugPrintf("%02d. %s\n", count, fleeText);
			++count;
		}
		debugPrintf("%d waypoints were found in scene.\n", count);
	} else if (arg == "walk") {
		debugPrintf("Listing walkboxes: \n");
		// list walkboxes
		for (int i = 0; i < _vm->_scene->_set->_walkboxCount; i++) {
			Set::Walkbox *walkbox = &_vm->_scene->_set->_walkboxes[i];

			debugPrintf("%02d. Walkbox %s, vertices: %d\n", i, walkbox->name.c_str(), walkbox->vertexCount);
		}
		debugPrintf("%d walkboxes were found in scene.\n", _vm->_scene->_set->_walkboxCount);
	} else if (arg == "fog") {
		debugPrintf("Listing fogs: \n");
		int count = 0;
		for (Fog *fog = _vm->_scene->_set->_effects->_fogs; fog != nullptr; fog = fog->_next) {
			debugPrintf("%02d. Fog %s\n", count, fog->_name.c_str());
			++count;
		}
		debugPrintf("%d fogs were found in scene.\n", count);
	} else if (arg == "lit") {
		debugPrintf("Listing lights: \n");
		// list lights
		for (int i = 0; i < (int)_vm->_lights->_lights.size(); i++) {
			Light *light = _vm->_lights->_lights[i];
			debugPrintf("%02d. Light %s\n", i, light->_name.c_str());
		}
		debugPrintf("%d lights were found in scene.\n", (int)_vm->_lights->_lights.size());
	} else {
		debugPrintf("Invalid item type was specified.\n");
	}

	return true;
}


void Debugger::drawDebuggerOverlay() {
	if (_viewSceneObjects) drawSceneObjects();
	if (_viewScreenEffects) drawScreenEffects();
	if (_viewLights) drawLights();
	if (_viewFogs) drawFogs();
	if (_viewRegions) drawRegions();
	if (_viewWaypoints) drawWaypoints();
	if (_viewWalkboxes) drawWalkboxes();
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
				color = _vm->_surfaceFront.format.RGBToColor(255, 0, 0);
				drawBBox(a, b, _vm->_view, &_vm->_surfaceFront, color);
				_vm->_surfaceFront.frameRect(sceneObject->screenRectangle, color);
				_vm->_mainFont->drawColor(_vm->_textActorNames->getText(sceneObject->id - kSceneObjectOffsetActors), _vm->_surfaceFront, pos.x, pos.y, color);
				break;
			case kSceneObjectTypeItem:
				color = _vm->_surfaceFront.format.RGBToColor(0, 255, 0);
				char itemText[40];
				drawBBox(a, b, _vm->_view, &_vm->_surfaceFront, color);
				sprintf(itemText, "item %i", sceneObject->id - kSceneObjectOffsetItems);
				_vm->_surfaceFront.frameRect(sceneObject->screenRectangle, color);
				_vm->_mainFont->drawColor(itemText, _vm->_surfaceFront, pos.x, pos.y, color);
				break;
			case kSceneObjectTypeObject:
				color = _vm->_surfaceFront.format.RGBToColor(127, 127, 127);
				if (sceneObject->isClickable) {
					color = _vm->_surfaceFront.format.RGBToColor(0, 255, 0);
				}
				drawBBox(a, b, _vm->_view, &_vm->_surfaceFront, color);
				_vm->_surfaceFront.frameRect(sceneObject->screenRectangle, color);
				_vm->_mainFont->drawColor(_vm->_scene->objectGetName(sceneObject->id - kSceneObjectOffsetObjects), _vm->_surfaceFront, pos.x, pos.y, color);
				break;
			}
		}
	}
}

void Debugger::drawLights() {
	// draw lights
	for (int i = 0; i < (int)_vm->_lights->_lights.size(); i++) {
		Light *light = _vm->_lights->_lights[i];
		Matrix4x3 m = light->_matrix;
		m = invertMatrix(m);
		Vector3 posOrigin = m * Vector3(0.0f, 0.0f, 0.0f);
		float t = posOrigin.y;
		posOrigin.y = posOrigin.z;
		posOrigin.z = -t;

		Vector3 posTarget = m * Vector3(0.0f, 0.0f, -100.0f);
		t = posTarget.y;
		posTarget.y = posTarget.z;
		posTarget.z = -t;

		Vector3 size = Vector3(5.0f, 5.0f, 5.0f);
		int color = _vm->_surfaceFront.format.RGBToColor(light->_color.r * 255.0f, light->_color.g * 255.0f, light->_color.b * 255.0f);

		drawBBox(posOrigin - size, posOrigin + size, _vm->_view, &_vm->_surfaceFront, color);

		Vector3 posOriginT = _vm->_view->calculateScreenPosition(posOrigin);
		Vector3 posTargetT = _vm->_view->calculateScreenPosition(posTarget);

		_vm->_surfaceFront.drawLine(posOriginT.x, posOriginT.y, posTargetT.x, posTargetT.y, color);

		_vm->_mainFont->drawColor(light->_name, _vm->_surfaceFront, posOriginT.x, posOriginT.y, color);
	}
}

void Debugger::drawFogs() {
	for (Fog *fog = _vm->_scene->_set->_effects->_fogs; fog != nullptr; fog = fog->_next) {

		// Matrix4x3 m = fog->_matrix;
		// m = invertMatrix(m);
		Matrix4x3 m = fog->_inverted;

		Vector3 posOrigin = m * Vector3(0.0f, 0.0f, 0.0f);
		float t = posOrigin.y;
		posOrigin.y = posOrigin.z;
		posOrigin.z = -t;

		Vector3 posTarget = m * Vector3(0.0f, 0.0f, -100.0f);
		t = posTarget.y;
		posTarget.y = posTarget.z;
		posTarget.z = -t;

		Vector3 size = Vector3(5.0f, 5.0f, 5.0f);
		int color = _vm->_surfaceFront.format.RGBToColor(fog->_fogColor.r * 255.0f, fog->_fogColor.g * 255.0f, fog->_fogColor.b * 255.0f);

		drawBBox(posOrigin - size, posOrigin + size, _vm->_view, &_vm->_surfaceFront, color);

		Vector3 posOriginT = _vm->_view->calculateScreenPosition(posOrigin);
		Vector3 posTargetT = _vm->_view->calculateScreenPosition(posTarget);

		// TODO: draw line only for cone fogs, draw boxes or circles for the other types
		_vm->_surfaceFront.drawLine(posOriginT.x, posOriginT.y, posTargetT.x, posTargetT.y, color);

		_vm->_mainFont->drawColor(fog->_name, _vm->_surfaceFront, posOriginT.x, posOriginT.y, color);
	}
}

void Debugger::drawRegions() {
	//draw regions
	for (int i = 0; i < 10; i++) {
		Regions::Region *region = &_vm->_scene->_regions->_regions[i];
		if (!region->present) continue;
		_vm->_surfaceFront.frameRect(region->rectangle, _vm->_surfaceFront.format.RGBToColor(0, 0, 255));
	}

	//draw exits
	for (int i = 0; i < 10; i++) {
		Regions::Region *region = &_vm->_scene->_exits->_regions[i];
		if (!region->present) continue;
		_vm->_surfaceFront.frameRect(region->rectangle, _vm->_surfaceFront.format.RGBToColor(255, 255, 255));
	}
}

void Debugger::drawWaypoints() {
	//draw world waypoints
	for (int i = 0; i < _vm->_waypoints->_count; i++) {
		Waypoints::Waypoint *waypoint = &_vm->_waypoints->_waypoints[i];
		if(waypoint->setId != _vm->_scene->getSetId()) {
			continue;
		}
		Vector3 pos = waypoint->position;
		Vector3 size = Vector3(3.0f, 3.0f, 3.0f);
		int color = _vm->_surfaceFront.format.RGBToColor(255, 255, 255);
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
		int color = _vm->_surfaceFront.format.RGBToColor(255, 0, 255);
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
		int color = _vm->_surfaceFront.format.RGBToColor(0, 255, 255);
		drawBBox(pos - size, pos + size, _vm->_view, &_vm->_surfaceFront, color);
		Vector3 spos = _vm->_view->calculateScreenPosition(pos);
		char fleeText[40];
		sprintf(fleeText, "flee %i", i);
		_vm->_mainFont->drawColor(fleeText, _vm->_surfaceFront, spos.x, spos.y, color);
	}
}

void Debugger::drawWalkboxes() {
	//draw walkboxes
	for (int i = 0; i < _vm->_scene->_set->_walkboxCount; i++) {
		Set::Walkbox *walkbox = &_vm->_scene->_set->_walkboxes[i];

		for (int j = 0; j < walkbox->vertexCount; j++) {
			Vector3 start = _vm->_view->calculateScreenPosition(walkbox->vertices[j]);
			Vector3 end = _vm->_view->calculateScreenPosition(walkbox->vertices[(j + 1) % walkbox->vertexCount]);
			_vm->_surfaceFront.drawLine(start.x, start.y, end.x, end.y, _vm->_surfaceFront.format.RGBToColor(255, 255, 0));
			Vector3 pos = _vm->_view->calculateScreenPosition(0.5 * (start + end));
			_vm->_mainFont->drawColor(walkbox->name, _vm->_surfaceFront, pos.x, pos.y, _vm->_surfaceFront.format.RGBToColor(255, 255, 0));
		}
	}
}

void Debugger::drawScreenEffects() {
	//draw aesc
	for (uint i = 0; i < _vm->_screenEffects->_entries.size(); i++) {
		ScreenEffects::Entry &entry = _vm->_screenEffects->_entries[i];
		int j = 0;
		for (int y = 0; y < entry.height; y++) {
			for (int x = 0; x < entry.width; x++) {
				Common::Rect r((entry.x + x) * 2, (entry.y + y) * 2, (entry.x + x) * 2 + 2, (entry.y + y) * 2 + 2);

				int ec = entry.data[j++];
				const int bladeToScummVmConstant = 256 / 16;

				int color = _vm->_surfaceFront.format.RGBToColor(
					CLIP(entry.palette[ec].r * bladeToScummVmConstant, 0, 255),
					CLIP(entry.palette[ec].g * bladeToScummVmConstant, 0, 255),
					CLIP(entry.palette[ec].b * bladeToScummVmConstant, 0, 255));
				_vm->_surfaceFront.fillRect(r, color);
			}
		}
	}
}

} // End of namespace BladeRunner
