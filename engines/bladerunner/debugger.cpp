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

#include "bladerunner/debugger.h"

#include "bladerunner/actor.h"
#include "bladerunner/ambient_sounds.h"
#include "bladerunner/audio_player.h"
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
#include "bladerunner/music.h"
#include "bladerunner/regions.h"
#include "bladerunner/savefile.h"
#include "bladerunner/scene.h"
#include "bladerunner/scene_objects.h"
#include "bladerunner/items.h"
#include "bladerunner/item_pickup.h"
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
#include "bladerunner/script/vk_script.h"
#include "bladerunner/overlays.h"
#include "bladerunner/subtitles.h"


#include "common/debug.h"
#include "common/str.h"

#include "graphics/surface.h"

namespace BladeRunner {

Debugger::Debugger(BladeRunnerEngine *vm) : GUI::Debugger() {
	_vm = vm;

	_isDebuggerOverlay = false;

	_viewActorsToggle = false;
	_view3dObjectsToggle = false;
	_viewItemsToggle = false;
	_viewLights = false;
	_viewFogs = false;
	_viewScreenEffects = false;
	_viewObstacles = false;
	_viewRegionsNormalToggle = false;
	_viewRegionsExitsToggle = false;
	_viewUI = false;
	_viewWaypointsNormalToggle = false;
	_viewWaypointsFleeToggle = false;
	_viewWaypointsCoverToggle = false;
	_viewWalkboxes = false;
	_viewZBuffer = false;

	_specificActorsDrawn = false;
	_specific3dObjectsDrawn = false;
	_specificItemsDrawn = false;
	_specificEffectsDrawn = false;
	_specificLightsDrawn = false;
	_specificFogsDrawn = false;
	_specificRegionNormalDrawn = false;
	_specificRegionExitsDrawn = false;
	_specificWaypointNormalDrawn = false;
	_specificWaypointFleeDrawn = false;
	_specificWaypointCoverDrawn = false;
	_specificWalkboxesDrawn = false;

	_playFullVk = false;
	_showStatsVk = false;
	_showMazeScore = false;
	_showMouseClickInfo = false;
	_useBetaCrosshairsCursor = false;
	_useAdditiveDrawModeForMouseCursorMode0 = false;
	_useAdditiveDrawModeForMouseCursorMode1 = false;
	resetPendingOuttake();

	registerCmd("anim", WRAP_METHOD(Debugger, cmdAnimation));
	registerCmd("health", WRAP_METHOD(Debugger, cmdHealth));
	registerCmd("draw", WRAP_METHOD(Debugger, cmdDraw));
	registerCmd("list", WRAP_METHOD(Debugger, cmdList));
	registerCmd("flag", WRAP_METHOD(Debugger, cmdFlag));
	registerCmd("goal", WRAP_METHOD(Debugger, cmdGoal));
	registerCmd("loop", WRAP_METHOD(Debugger, cmdLoop));
	registerCmd("pos", WRAP_METHOD(Debugger, cmdPosition));
	registerCmd("music", WRAP_METHOD(Debugger, cmdMusic));
	registerCmd("sound", WRAP_METHOD(Debugger, cmdSoundFX));
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
	registerCmd("vk", WRAP_METHOD(Debugger, cmdVk));
	registerCmd("mazescore", WRAP_METHOD(Debugger, cmdMazeScore));
	registerCmd("object", WRAP_METHOD(Debugger, cmdObject));
	registerCmd("item", WRAP_METHOD(Debugger, cmdItem));
	registerCmd("region", WRAP_METHOD(Debugger, cmdRegion));
	registerCmd("mouse", WRAP_METHOD(Debugger, cmdMouse));
	registerCmd("difficulty", WRAP_METHOD(Debugger, cmdDifficulty));
	registerCmd("outtake", WRAP_METHOD(Debugger, cmdOuttake));
	registerCmd("playvqa", WRAP_METHOD(Debugger, cmdPlayVqa));
	registerCmd("ammo", WRAP_METHOD(Debugger, cmdAmmo));
	registerCmd("cheat", WRAP_METHOD(Debugger, cmdCheatReport));
#if BLADERUNNER_ORIGINAL_BUGS
#else
	registerCmd("effect", WRAP_METHOD(Debugger, cmdEffect));
#endif // BLADERUNNER_ORIGINAL_BUGS
}

Debugger::~Debugger() {
	if (!_specificDrawnObjectsList.empty()) {
		_specificDrawnObjectsList.clear();
	}
}

const struct AnimationsList {
	Actors actorId;
	int animationModelIdStart;
	int animationModelIdEnd;
	int animationModelIdSpecial; // for characters that use an animation outside their "own" range (eg. kActorGenwalkerA using Bob's Gun)
	int animationModeMax;
	int animationStateMax;
} animationsList[] = {
	{ kActorMcCoy,             0,  53, -1, 85, 71 },
	{ kActorSteele,           54,  92, -1, 86, 41 },
	{ kActorGordo,            93, 133, -1, 84, 39 },
	{ kActorDektora,         134, 171, -1, 79, 41 },
	{ kActorGuzza,           172, 207, -1, 61, 32 },
	{ kActorClovis,          208, 252, -1, 88, 42 },
	{ kActorLucy,            253, 276, -1, 48, 21 },
	{ kActorIzo,             277, 311, -1, 48, 35 },
	{ kActorSadik,           312, 345, -1, 63, 34 },
	{ kActorLuther,          346, 359, -1, 50, 12 },
	{ kActorEarlyQ,          360, 387, -1, 85, 28 },
	{ kActorZuben,           388, 421, -1, 49, 28 },
	{ kActorGenwalkerA,      422, 437, 440, 4,  3 },
	{ kActorGenwalkerB,      422, 437, -1,  1,  2 },
	{ kActorGenwalkerC,      422, 437, -1,  1,  2 },
	{ kActorHysteriaPatron3, 438, 439, -1, -1,  2 },
	// skip animations 441-450 that refer to targets in shooting range (which are items kItemPS10Target1 to kItemPS10Target9)
	{ kActorBaker,           451, 451, -1, -1,  0 },
	{ kActorCrazylegs,       452, 469, -1, 43, 19 },
	{ kActorGrigorian,       470, 486, -1, 16,  7 },
	{ kActorTransient,       487, 505, -1, 89, 19 },
	{ kActorBulletBob,       506, 525, -1, 88, 16 },
	{ kActorRunciter,        526, 544, -1, 48, 15 },
	{ kActorInsectDealer,    545, 554, -1, 23,  8 },
	{ kActorTyrellGuard,     555, 565, -1, 55, 11 },
	{ kActorMia,             566, 570, -1, 23,  4 },
	{ kActorOfficerLeary,    571, 604, -1, 58, 32 },
	{ kActorOfficerGrayford, 605, 641, -1, 58, 37 },
	{ kActorHanoi,           642, 660, -1, 78, 20 },
	{ kActorDeskClerk,       661, 670, -1, 72,  8 },
	{ kActorHowieLee,        671, 681, -1, 43,  8 },
	{ kActorFishDealer,      682, 687, -1, 23,  5 },
	{ kActorKlein,           688, 697, -1, 16,  8 },
	{ kActorMurray,          698, 704, -1, 15,  6 },
	{ kActorHawkersBarkeep,  705, 715, -1, 16,  9 },
	{ kActorHolloway,        716, 721, -1, 15,  7 },
	{ kActorSergeantWalls,   722, 731, -1, 23,  9 },
	{ kActorMoraji,          732, 743, -1, 48, 14 },
	{ kActorPhotographer,    744, 750, -1, 43,  6 },
	{ kActorRajif,           751, 751, -1,  0,  0 },
	{ kActorEarlyQBartender, 752, 757, -1, 23,  4 },
	{ kActorShoeshineMan,    758, 764, -1, 29,  6 },
	{ kActorTyrell,          765, 772, -1, 15,  6 },
	{ kActorChew,            773, 787, -1, 48, 13 },
	{ kActorGaff,            788, 804, -1, 41,  8 },
	{ kActorBryant,          805, 808, -1, 48,  3 },
	{ kActorSebastian,       809, 821, -1, 48, 11 },
	{ kActorRachael,         822, 832, -1, 18,  9 },
	{ kActorGeneralDoll,     833, 837, -1, 48,  4 },
	{ kActorIsabella,        838, 845, -1, 17,  9 },
	{ kActorLeon,            846, 856, -1, 72, 10 },
	{ kActorFreeSlotA,       857, 862, -1, 48,  8 },
	{ kActorFreeSlotB,       857, 862, -1, 48,  8 },
	{ kActorMaggie,          863, 876, -1, 88, 16 },
	{ kActorHysteriaPatron1, 877, 884, -1, -1, 26 },
	{ kActorHysteriaPatron2, 885, 892, -1, -1, 29 },
	{ kActorMutant1,         893, 900, -1, 88, 10 },
	{ kActorMutant2,         901, 907, -1, 88,  8 },
	{ kActorMutant3,         908, 917, -1, 88, 11 },
	{ kActorTaffyPatron,     918, 919, -1, 48,  2 },
	{ kActorHasan,           920, 930, -1, 16,  6 }
	// skip animations 931-996 which refer to item models/animations
};

bool Debugger::cmdAnimation(int argc, const char **argv) {
	if (argc != 2 && argc != 4) {
		debugPrintf("Get or set animation mode of the actor.\n");
		debugPrintf("Usage: %s <actorId> [<animationMode> <showDamageAnimationWhenMoving>]\n", argv[0]);
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

	if (argc == 4) {
		int animationMode = atoi(argv[2]);
		int showDmgWhenMoving = atoi(argv[3]);
		actor->setFlagDamageAnimIfMoving(showDmgWhenMoving!=0);
		actor->changeAnimationMode(animationMode, true);
		debugPrintf("actorAnimationMode(%i) = %i, showDamageWhenMoving = %i\n", actorId, animationMode, actor->getFlagDamageAnimIfMoving());
		return false;
	}

	int animationState = -1;
	int animationFrame = -1;
	int animationStateNext = -1;
	int animationNext = -1;
	actor->queryAnimationState(&animationState, &animationFrame, &animationStateNext, &animationNext);

	(void)animationsList;

	debugPrintf("actorAnimationMode(%s) = %i, model: %i, goal: %i, state:%i, frame:%i, stateNext: %i, nextModelId: %i, showDamageWhenMoving = %i, inCombat = %i\n",
	             _vm->_textActorNames->getText(actorId),
	             actor->getAnimationMode(),
	             actor->getAnimationId(),
	             actor->getGoal(),
	             animationState,
	             animationFrame,
	             animationStateNext,
	             animationNext,
	             actor->getFlagDamageAnimIfMoving(),
	             actor->inCombat());
	return true;
}

bool Debugger::cmdHealth(int argc, const char **argv) {
	if (argc != 2 && argc != 4) {
		debugPrintf("Get or set health for the actor.\n");
		debugPrintf("Usage: %s <actorId> [<health> <max health>]\n", argv[0]);
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

	if (argc == 4) {
		int currHealth = atoi(argv[2]);
		int maxHealth = atoi(argv[3]);
		currHealth = CLIP(currHealth, 0, 100);
		maxHealth = CLIP(maxHealth, 0, 100);
		if (currHealth > maxHealth) {
			debugPrintf("Actor's current health cannot be greater than their max health\n");
			return true;
		}
		actor->setHealth(currHealth, maxHealth);
	}

	debugPrintf("actor health(%i) = %i, max: %i\n", actorId, actor->getCurrentHP(), actor->getMaxHP());
	return true;
}

bool Debugger::cmdDraw(int argc, const char **argv) {
	bool invalidSyntax = false;

	if (argc != 2 && argc != 3) {
		invalidSyntax = true;
	} else {
		Common::String arg = argv[1];

		DebuggerDrawnObject dbgDrawnObj;
		dbgDrawnObj.type = debuggerObjTypeUndefined;

		if (argc == 3) {
			int specificObjectId = atoi(argv[2]);
			dbgDrawnObj.objId = specificObjectId;
			dbgDrawnObj.sceneId = _vm->_scene->getSceneId();
			dbgDrawnObj.setId = _vm->_scene->getSetId();
		}

		if (arg == "allobj") {
			if (_viewActorsToggle && _view3dObjectsToggle && _viewItemsToggle) {
				_viewActorsToggle = false;
				_view3dObjectsToggle  = false;
				_viewItemsToggle = false;
			} else {
				_viewActorsToggle = true;
				_view3dObjectsToggle  = true;
				_viewItemsToggle = true;
			}
			debugPrintf("Drawing all scene objects (actors, 3d objects, items) = %i\n", _viewActorsToggle && _view3dObjectsToggle && _viewItemsToggle);
		} else if (arg == "act") {
			if (argc == 2) {
				_viewActorsToggle = !_viewActorsToggle;
				debugPrintf("Drawing all actors in scene = %s\n", _viewActorsToggle? "true" : "false");
			} else {
				_viewActorsToggle = false;
				dbgDrawnObj.setId = -1;
				dbgDrawnObj.sceneId = -1;
				dbgDrawnObj.type = debuggerObjTypeActor;
			}
		} else if (arg == "obj") {
			if (argc == 2) {
				_view3dObjectsToggle = !_view3dObjectsToggle;
				debugPrintf("Drawing all 3d objects in scene = %s\n", _view3dObjectsToggle? "true" : "false");
			} else {
				_view3dObjectsToggle = false;
				dbgDrawnObj.type = debuggerObjType3dObject;
			}
		} else if (arg == "item") {
			if (argc == 2) {
				_viewItemsToggle = !_viewItemsToggle;
				debugPrintf("Drawing all items in scene = %s\n", _viewItemsToggle? "true" : "false");
			} else {
				_viewItemsToggle = false;
				dbgDrawnObj.setId = -1;
				dbgDrawnObj.sceneId = -1;
				dbgDrawnObj.type = debuggerObjTypeItem;
			}
		} else if (arg == "eff") {
			if (argc == 2) {
				_viewScreenEffects = !_viewScreenEffects;
				debugPrintf("Drawing all screen effects = %s\n", _viewScreenEffects? "true" : "false");
			} else {
				_viewScreenEffects = false;
				dbgDrawnObj.type = debuggerObjTypeEffect;
			}
		} else if (arg == "fog") {
			if (argc == 2) {
				_viewFogs = !_viewFogs;
				debugPrintf("Drawing all fogs = %s\n", _viewFogs? "true" : "false");
			} else {
				_viewFogs = false;
				dbgDrawnObj.type = debuggerObjTypeFog;
			}
		} else if (arg == "lit") {
			if (argc == 2) {
				_viewLights = !_viewLights;
				debugPrintf("Drawing all lights = %s\n", _viewLights? "true" : "false");
			} else {
				_viewLights = false;
				dbgDrawnObj.type = debuggerObjTypeLight;
			}
		} else if (arg == "allreg") {
			if (_viewRegionsNormalToggle && _viewRegionsExitsToggle) {
				_viewRegionsNormalToggle = false;
				_viewRegionsExitsToggle  = false;
			} else {
				_viewRegionsNormalToggle = true;
				_viewRegionsExitsToggle  = true;
			}
			debugPrintf("Drawing all scene regions (regular, exits) = %s\n", (_viewRegionsNormalToggle && _viewRegionsExitsToggle)? "true" : "false");
		} else if (arg == "regnorm") {
			if (argc == 2) {
				_viewRegionsNormalToggle = !_viewRegionsNormalToggle;
				debugPrintf("Drawing all normal regions = %s\n", _viewRegionsNormalToggle? "true" : "false");
			} else {
				_viewRegionsNormalToggle = false;
				dbgDrawnObj.type = debuggerObjTypeRegionNormal;
			}
		} else if (arg == "regexit") {
			if (argc == 2) {
				_viewRegionsExitsToggle = !_viewRegionsExitsToggle;
				debugPrintf("Drawing all exit regions = %s\n", _viewRegionsExitsToggle? "true" : "false");
			} else {
				_viewRegionsExitsToggle = false;
				dbgDrawnObj.type = debuggerObjTypeRegionExit;
			}
		} else if (arg == "obstacles") {
			_viewObstacles = !_viewObstacles;
			debugPrintf("Drawing obstacles = %s\n", _viewObstacles? "true" : "false");
		} else if (arg == "ui") {
			_viewUI = !_viewUI;
			debugPrintf("Drawing all UI elements = %s\n", _viewUI? "true" : "false");
		} else if (arg == "allway") {
			if (_viewWaypointsNormalToggle && _viewWaypointsFleeToggle && _viewWaypointsCoverToggle) {
				_viewWaypointsNormalToggle = false;
				_viewWaypointsFleeToggle  = false;
				_viewWaypointsCoverToggle = false;
			} else {
				_viewWaypointsNormalToggle = true;
				_viewWaypointsFleeToggle = true;
				_viewWaypointsCoverToggle = true;
			}
			debugPrintf("Drawing all waypoints (regular, cover, flee) = %s\n", (_viewWaypointsNormalToggle && _viewWaypointsFleeToggle && _viewWaypointsCoverToggle)? "true" : "false");
		} else if (arg == "waynorm") {
			if (argc == 2) {
				_viewWaypointsNormalToggle = !_viewWaypointsNormalToggle;
				debugPrintf("Drawing all normal waypoints = %s\n", _viewWaypointsNormalToggle? "true" : "false");
			} else {
				_viewWaypointsNormalToggle = false;
				dbgDrawnObj.setId = -1;
				dbgDrawnObj.sceneId = -1;
				dbgDrawnObj.type = debuggerObjTypeWaypointNorm;
			}
		} else if (arg == "wayflee") {
			if (argc == 2) {
				_viewWaypointsFleeToggle = !_viewWaypointsFleeToggle;
				debugPrintf("Drawing all flee waypoints = %s\n", _viewWaypointsFleeToggle? "true" : "false");
			} else {
				_viewWaypointsFleeToggle = false;
				dbgDrawnObj.setId = -1;
				dbgDrawnObj.sceneId = -1;
				dbgDrawnObj.type = debuggerObjTypeWaypoingFlee;
			}
		} else if (arg == "waycov") {
			if (argc == 2) {
				_viewWaypointsCoverToggle = !_viewWaypointsCoverToggle;
				debugPrintf("Drawing all cover waypoints = %s\n", _viewWaypointsCoverToggle? "true" : "false");
			} else {
				_viewWaypointsCoverToggle = false;
				dbgDrawnObj.setId = -1;
				dbgDrawnObj.sceneId = -1;
				dbgDrawnObj.type = debuggerObjTypeWaypointCover;
			}
		} else if (arg == "walk") {
			if (argc == 2) {
				_viewWalkboxes = !_viewWalkboxes;
				debugPrintf("Drawing all walk boxes = %s\n", _viewWalkboxes? "true" : "false");
			} else {
				_viewWalkboxes = false;
				dbgDrawnObj.type = debuggerObjTypeWalkbox;
			}
		} else if (arg == "zbuf") {
			_viewZBuffer = !_viewZBuffer;
			debugPrintf("Drawing Z buffer = %s\n", _viewZBuffer? "true" : "false");
		} else if (arg == "reset") {

			if (!_specificDrawnObjectsList.empty()) {
				_specificDrawnObjectsList.clear();
			}

			_viewActorsToggle = false;
			_view3dObjectsToggle = false;
			_viewItemsToggle = false;
			_viewObstacles = false;
			_viewRegionsNormalToggle = false;
			_viewRegionsExitsToggle = false;
			_viewScreenEffects = false;
			_viewFogs = false;
			_viewLights = false;
			_viewUI = false;
			_viewWaypointsNormalToggle = false;
			_viewWaypointsFleeToggle = false;
			_viewWaypointsCoverToggle = false;
			_viewWalkboxes = false;
			_viewZBuffer = false;

			debugPrintf("Drawing all scene objects (actors, 3d objects, items) = %s\n", (_viewActorsToggle && _view3dObjectsToggle && _viewItemsToggle)? "true" : "false");
			debugPrintf("Drawing scene actors = %s\n", _viewActorsToggle? "true" : "false");
			debugPrintf("Drawing scene 3d objects = %s\n", _view3dObjectsToggle? "true" : "false");
			debugPrintf("Drawing scene items = %s\n", _viewItemsToggle? "true" : "false");
			debugPrintf("Drawing obstacles = %s\n", _viewObstacles? "true" : "false");
			debugPrintf("Drawing all regions (regular, exits) = %s\n", (_viewRegionsNormalToggle && _viewRegionsExitsToggle)? "true" : "false");
			debugPrintf("Drawing regular regions = %s\n",  _viewRegionsNormalToggle? "true" : "false");
			debugPrintf("Drawing exit regions = %s\n",  _viewRegionsExitsToggle? "true" : "false");
			debugPrintf("Drawing screen effects = %s\n", _viewScreenEffects? "true" : "false");
			debugPrintf("Drawing fogs = %s\n", _viewFogs? "true" : "false");
			debugPrintf("Drawing lights = %s\n", _viewLights? "true" : "false");
			debugPrintf("Drawing UI elements = %s\n", _viewUI? "true" : "false");
			debugPrintf("Drawing all waypoints (regular, cover, flee) = %s\n", (_viewWaypointsNormalToggle && _viewWaypointsFleeToggle && _viewWaypointsCoverToggle)? "true" : "false");
			debugPrintf("Drawing regular waypoints = %s\n", _viewWaypointsNormalToggle? "true" : "false");
			debugPrintf("Drawing flee waypoints = %s\n", _viewWaypointsFleeToggle? "true" : "false");
			debugPrintf("Drawing cover waypoints = %s\n", _viewWaypointsCoverToggle? "true" : "false");
			debugPrintf("Drawing walkboxes = %s\n", _viewWalkboxes? "true" : "false");
			debugPrintf("Drawing Z buffer = %s\n", _viewZBuffer? "true" : "false");
		} else {
			invalidSyntax = true;
		}

		if (!invalidSyntax) {
			if (dbgDrawnObj.type != debuggerObjTypeUndefined) {
				toggleObjectInDbgDrawList(dbgDrawnObj);
			}
			updateTogglesForDbgDrawListInCurrentSetAndScene();
		}
	}

	if (invalidSyntax) {
		debugPrintf("Enables debug rendering of actors, screen effect, fogs, lights, scene objects\nobstacles, regions, ui elements, walk boxes, waypoints, zbuffer or disables debug rendering.\n");
		debugPrintf("Usage 1: %s (allobj | obstacles | allreg | ui | allway | zbuf | reset)\n", argv[0]);
		debugPrintf("Usage 2a: %s (act | obj | item | regnorm | regexit | waynorm | wayflee | waycov) [<id>]\n", argv[0]);
		debugPrintf("Usage 2b: %s (eff | fog | lit | walk) [<id>]\n", argv[0]);
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
	if (flag >= 0 && flag < flagCount) {
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

/**
 * @brief Auxiliary function to determine if a String is comprised exclusively of "0"
 *
 * This is basically a very simplified (and smaller scope) version of
 * checking the String with isdigit() (which is banned).
 *
 * @param valStr The String to examine
 * @return true if String is all zeroes, false otherwise
*/
bool isAllZeroes(Common::String valStr) {
	for (uint i = 0; i < valStr.size();  ++i) {
		if (valStr.c_str()[i] != '0') {
			return false;
		}
	}
	return true;
}

// Tracks marked as (G) are only available in-game ie. not in the official OST by Frank Klepacki on his site.
//
// Note, that there are a few tracks that are not proper music tracks but rather SFX tracks.
// For example, the re-used track "Iron Fist" from Command & Conquer - The Covert Operations (OST),
// which is played at the NightClub Row (NR01), is "kSfxMUSBLEED" (looping).
// TODO maybe support those too?
const char* kMusicTracksArr[] = {"Animoid Row (G)",                 // kMusicArabLoop
								 "Battle Theme",                    // kMusicBatl226M
								 "Blade Runner Blues",              // kMusicBRBlues
								 "Etsuko Theme",                    // kMusicKyoto
								 "One More Time, Love (G)",         // kMusicOneTime
								 "Gothic Club 2",                   // kMusicGothic3
								 "Arcane Dragon Fly (G)",           // kMusicArkdFly1
								 "Arcane Dance (G)",                // kMusicArkDnce1
								 "Taffy's Club 2",                  // kMusicTaffy2
								 "Enigma Drift",                    // kMusicTaffy3
								 "Late Call",                       // kMusicTaffy4
								 "Nexus (aka Beating 1)",           // kMusicBeating1
								 "Awakenings (aka Crystal Dies 1)", // kMusicCrysDie1
								 "Gothic Club",                     // kMusicGothic1
								 "Transition",                      // kMusicGothic2
								 "The Eyes Follow",                 // kMusicStrip1
								 "Dektora's Dance (G)",             // kMusicDkoDnce1
								 "End Credits",                     // kMusicCredits
								 "Ending (aka Moraji)",             // kMusicMoraji
								 "Remorse (aka Clovis Dies 1)",     // kMusicClovDie1
								 "Solitude (aka Clovis Dies)",      // kMusicClovDies
								 "Love Theme"};                     // kMusicLoveSong

bool Debugger::cmdMusic(int argc, const char** argv) {
	bool invalidSyntax = false;

	if (argc == 2) {
		Common::String trackArgStr = argv[1];
		if (trackArgStr == "list") {
			for (int i = 0; i < (int)_vm->_gameInfo->getMusicTrackCount(); ++i) {
				debugPrintf("%2d - %s\n", i, kMusicTracksArr[i]);
			}
			return true;

		} else if (trackArgStr == "stop") {
			_vm->_music->stop(0u);
			//_vm->_ambientSounds->removeLoopingSound(kSfxMUSBLEED, 0);
		} else if (Common::isDigit(*argv[1])) {
			int musicId = atoi(argv[1]);

			if ((musicId == 0 && !isAllZeroes(trackArgStr))
				|| musicId < 0
				|| musicId >= (int)_vm->_gameInfo->getMusicTrackCount()) {
				debugPrintf("Invalid music track id specified.\nPlease choose an integer between 0 and %d.\n", (int)_vm->_gameInfo->getMusicTrackCount() - 1);
				return true;

			} else {
				_vm->_music->stop(0u);
				_vm->_music->play(_vm->_gameInfo->getMusicTrack(musicId), 100, 0, 0, -1, kMusicLoopPlayOnce, 0);
				//debugPrintf("Now playing track %2d - \"%s\" (%s)\n", musicId, kMusicTracksArr[musicId], _vm->_gameInfo->getMusicTrack(musicId).c_str());
				debugPrintf("Now playing track %2d - \"%s\"\n", musicId, kMusicTracksArr[musicId]);
				return false;

			}
			//_vm->_ambientSounds->removeLoopingSound(kSfxMUSBLEED, 0);
			//_vm->_ambientSounds->addLoopingSound(kSfxMUSBLEED, 100, 0, 0);
		} else {
			invalidSyntax = true;
		}
	} else {
		invalidSyntax = true;
	}

	if (invalidSyntax) {
		debugPrintf("Play the specified music track, list the available tracks\nor stop the current playing track.\n");
		debugPrintf("Usage: %s (list|stop|<musicId>)\n", argv[0]);
		debugPrintf("musicId can be in [0, %d]\n", (int)_vm->_gameInfo->getMusicTrackCount() - 1);
	}
	return true;
}

bool Debugger::cmdSoundFX(int argc, const char** argv) {
	bool invalidSyntax = false;
	// Play the specified (by id) Sound Effect (similar to ScriptBase::Sound_Play())
	if (argc == 2 && Common::isDigit(*argv[1])) {
		int sfxId = atoi(argv[1]);
		if (sfxId >= 0 && sfxId < (int)_vm->_gameInfo->getSfxTrackCount()) {
			_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(sfxId), 100, 0, 0, 50);
			return false;

		} else {
			debugPrintf("soundId can be in [0, %d]\n", (int)_vm->_gameInfo->getSfxTrackCount() - 1);
		}
	} else {
		invalidSyntax = true;
	}

	if (invalidSyntax) {
		debugPrintf("Play the specified sound effect id.\n");
		debugPrintf("Usage: %s <soundFXId>\n", argv[0]);
		debugPrintf("soundId can be in [0, %d]\n", (int)_vm->_gameInfo->getSfxTrackCount() - 1);
	}
	return true;
}

bool Debugger::cmdSay(int argc, const char **argv) {
	bool invalidSyntax = false;

	if (argc == 3 && Common::isDigit(*argv[1]) && Common::isDigit(*argv[2])) {
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

	} else {
		invalidSyntax = true;
	}

	if (invalidSyntax) {
		debugPrintf("Actor will say the specified line.\n");
		debugPrintf("Usage: %s <actorId> <sentenceId>\n", argv[0]);
	}
	return true;
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
	{ 4, "PS09", 67, 72 },   { 4, "PS14", 68, 77 },  { 4, "RC01", 69, 78 },   { 4, "RC02", 16, 79 },
	{ 4, "RC03", 70, 80 },   { 4, "RC04", 71, 81 },  { 4, "RC51", 16, 107 },  { 4, "TB02", 17, 82 },
	{ 4, "TB03", 17, 83 },   { 4, "TB07", 18, 108 }, { 4, "UG01", 74, 86 },   { 4, "UG02", 75, 87 },
	{ 4, "UG03", 76, 88 },   { 4, "UG04", 77, 89 },  { 4, "UG05", 78, 90 },   { 4, "UG06", 79, 91 },
	{ 4, "UG07", 80, 92 },   { 4, "UG08", 81, 93 },  { 4, "UG09", 82, 94 },   { 4, "UG10", 83, 95 },
	{ 4, "UG12", 84, 96 },   { 4, "UG12", 6, 96 },   { 4, "UG13", 85, 97 },   { 4, "UG14", 86, 98 },
	{ 4, "UG15", 87, 99 },   { 4, "UG16", 19, 100 }, { 4, "UG17", 88, 101 },  { 4, "UG18", 89, 102 },
	{ 4, "UG19", 90, 103 },

	{ 0, nullptr, 0, 0 }
};

// Auxialliary method to validate chapter, set and scene combination
// and if the triad is valid, then load that scene
bool Debugger::dbgAttemptToLoadChapterSetScene(int chapterId, int setId, int sceneId) {
	if (chapterId < 1 || chapterId > 5) {
		debugPrintf("chapterID must be between 1 and 5\n");
		return false;
	}

	int chapterIdNormalized = chapterId;

	if (chapterId == 3 || chapterId == 5) {
		chapterIdNormalized = chapterId - 1;
	}

	// Sanity check
	uint i;
	for (i = 0; sceneList[i].chapter != 0; ++i) {
		if (sceneList[i].chapter == chapterIdNormalized &&
		    sceneList[i].set == setId &&
		    sceneList[i].scene == sceneId
		) {
			break;
		}
	}

	if (sceneList[i].chapter == 0) { // end of list
		debugPrintf("chapterId, setId and sceneId combination is not valid.\n");
		return false;
	}

	if (chapterId != _vm->_settings->getChapter()) {
		_vm->_settings->setChapter(chapterId);
	}
	_vm->_settings->setNewSetAndScene(setId, sceneId);
	return true;
}

// Note: The chapterId that is set with this command affects loading
//       of game resources and the return value of _vm->_settings->getChapter()
//       However, it will NOT change the value of the game's global variable (1) for the current chapter.
//       The user has to explicitly set that as well, after executing this debugger command,
//       using eg. var 1 3 (for chapter 3)
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

		return !dbgAttemptToLoadChapterSetScene(chapterId, setId, sceneId);
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
		for (i = 0; sceneList[i].chapter != 0; ++i) {
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
	for (i = 0; sceneList[i].chapter != 0; ++i) {
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
		debugPrintf("Gets or changes clue for an actor.\n");
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
		debugPrintf("Gets or changes timers for an actor.\n");
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
		debugPrintf("actorTimer(%i, %i) = %d ms\n", actorId, i, actor->timerLeft(i));
	}

	return true;
}

bool Debugger::cmdFriend(int argc, const char **argv) {
	if (argc != 3 && argc != 4) {
		debugPrintf("Gets or changes friendliness for an actor towards another actor.\n");
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

	// NOTE Using FSNode here means that ScummVM will not use SearchMan to find the file,
	//      so the file should be at folder where ScummVM was launched from.
	// TODO Consider using Common::File instead similar to how
	//      BladeRunnerEngine::getResourceStream() is implemented)
	Common::FSNode fs(argv[1]);

	if (!fs.isReadable()) {
		debugPrintf("Warning: File %s does not exist or is not readable\n", argv[1]);
		return true;
	}

	if (fs.isDirectory()) {
		debugPrintf("Warning: Given path %s is a folder. Please provide a path to a file!\n", argv[1]);
		return true;
	}

	Common::SeekableReadStream *saveFile = fs.createReadStream();

	_vm->loadGame(*saveFile, 3);

	delete saveFile;

	return false;
}

bool Debugger::cmdSave(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Saves game to original format.\n");
		debugPrintf("Usage: %s <file path>\n", argv[0]);
		return true;
	}

	// NOTE Using FSNode here means that ScummVM will ouput the saved game file
	//      into the folder ScummVM was launched from.
	// TODO Maybe output the saved game file into the game data (top most) folder?
	Common::FSNode fs(argv[1]);

	if (fs.exists() && !fs.isWritable()) {
		debugPrintf("Warning: File %s is not writable\n", argv[1]);
		return true;
	}

	if (fs.isDirectory()) {
		debugPrintf("Warning: Given path %s is a folder. Please provide a path to a file!\n", argv[1]);
		return true;
	}

	Common::WriteStream *saveFile = fs.createWriteStream();

	Graphics::Surface thumbnail = _vm->generateThumbnail();

	_vm->_time->pause();
	_vm->saveGame(*saveFile, &thumbnail, true);
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

	{ 0, nullptr, false }
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

	if (_vm->_kia->isOpen()
	    || _vm->_esper->isOpen()
	    || _vm->_spinner->isOpen()
	    || _vm->_elevator->isOpen()
	    || _vm->_vk->isOpen()
	    || _vm->_scores->isOpen() ) {
		debugPrintf("Sorry, playing custom overlays in KIA, ESPER, Voigt-Kampff, Spinner GPS,\nScores or Elevator mode is not supported\n");
		return true;
	}

	if (argc != 1 && argc != 2 && argc != 3 && argc != 5) {
		invalidSyntax = true;
	} else {
		bool modeMixOverlaysAvailableFlg = false;
		int chapterIdOverlaysAvailableInt = -1;

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
					++countOfLoadedOverlaysInScene;
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
					debugPrintf("Error: Could not open player while resetting\nto scene VQA named: %s!\n", (origVqaName + ".VQA").c_str());
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

		if (!invalidSyntax && (argc == 3 || argc == 5)) {
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
					if (strcmp(overlaysList[itemIter].name, overlayName.c_str()) == 0) {
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
				if ( overlayVideoIdx == -1 ) {
					debugPrintf("Could not load the overlay animation: %s in this scene. Try resetting overlays first to free up slots!\n", overlayName.c_str());
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

	if (argc != 2 && argc != 3) {
		invalidSyntax = true;
	} else {
		if (!_vm->_subtitles->isSystemActive()) {
			debugPrintf("Subtitles system is currently disabled\n");
		}

		Common::String subtitleText = argv[1];
		int subtitleRole = BladeRunner::Subtitles::kSubtitlesPrimary;
		if (argc == 3) {
			subtitleRole = atoi(argv[2]);
			// Just interpret any number other than 0 as secondary subtitles
			if (subtitleRole != BladeRunner::Subtitles::kSubtitlesPrimary) {
				subtitleRole = BladeRunner::Subtitles::kSubtitlesSecondary;
			}
		}

		if (subtitleText == "info") {
			debugPrintf("Subtitles version info: v%s (%s) %s\nCredits:\n%s\n",
			            _vm->_subtitles->getSubtitlesInfo().versionStr.c_str(),
			            _vm->_subtitles->getSubtitlesInfo().dateOfCompile.c_str(),
			            _vm->_subtitles->getSubtitlesInfo().languageMode.c_str(),
			            _vm->_subtitles->getSubtitlesInfo().credits.c_str());
			debugPrintf("Subtitles font loaded: %s\n",
			            _vm->_subtitles->getSubtitlesInfo().fontName.c_str());

		} else if (subtitleText == "reset") {
			if (argc == 2) {
				_vm->_subtitles->setGameSubsText(BladeRunner::Subtitles::kSubtitlesPrimary, "", false);
				_vm->_subtitles->setGameSubsText(BladeRunner::Subtitles::kSubtitlesSecondary, "", false);
			} else {
				_vm->_subtitles->setGameSubsText(subtitleRole, "", false);
			}
		} else if (subtitleText == "printExtAscii") {
			// Test displaying all glyphs in subtitles font
			Common::String allGlyphQuote;
			int strpos = 0;
			for (int j = 1; j < 8; ++j) {
				for (int i = j * 32; i < (j + 1) * 32 && i < 255 ; ++i) {
					allGlyphQuote.insertChar((char)i, strpos++);
					allGlyphQuote.insertChar(' ', strpos++);
				}
				if (j < 7) allGlyphQuote.insertChar('\n', strpos++);
			}
			_vm->_subtitles->setGameSubsText(subtitleRole, allGlyphQuote, true);
			_vm->_subtitles->show(subtitleRole);
		} else {
			debugPrintf("Showing text: %s\n", subtitleText.c_str());
			_vm->_subtitles->setGameSubsText(subtitleRole, subtitleText, true);
			_vm->_subtitles->show(subtitleRole);
		}
	}

	if (invalidSyntax) {
		debugPrintf("Show subtitles info, or display and clear (reset) a specified text as subtitle or clear the current subtitle.\n");
		debugPrintf("Use double quotes to encapsulate the text.\n");
		debugPrintf("SubtitleRole can be 0 (primary) or 1 (secondary).\n");
		debugPrintf("Usage: %s (info | \"<text_to_display>\" [subtitleRole] | printExtAscii [subtitleRole]  | reset [subtitleRole])\n", argv[0]);
	}
	return true;
}

/**
* Toggle showing Maze score on the fly as subtitle during the Police Maze Course
*/
bool Debugger::cmdMazeScore(int argc, const char **argv) {
	bool invalidSyntax = false;

	if (argc != 2) {
		invalidSyntax = true;
	} else {
		if (_vm->_scene->getSetId() != kSetPS10_PS11_PS12_PS13) {
			debugPrintf("Error:Command %s is only valid during the Police Maze course\n",  argv[0]);
			return true;
		}
		//
		// set a debug variable to enable showing the maze score
		//
		Common::String argName = argv[1];
		argName.toLowercase();
		if (argc == 2 && argName == "toggle") {
			_showMazeScore = !_showMazeScore;
			debugPrintf("Showing maze score = %s\n", _showMazeScore ? "True":"False");
		} else {
			invalidSyntax = true;
		}
	}

	if (invalidSyntax) {
		debugPrintf("Toggle showing the Maze Score as a subtitle during the Shooting Grounds Course\n");
		debugPrintf("Usage: %s toggle\n", argv[0]);
	}
	return true;
}


/**
* Add, remove or edit flags or bounding box for an object to debug some issues whereby existing obstacle layout does not prevent
* characters from walking where they shouldn't
*
* Notes: with regard to Bound box / the clickable area for an object, item or region
* 1. The code for what a mouse click will refer to is in BladeRunnerEngine::handleMouseAction() -- under if (mainButton) clause
*    Clickable 3d areas for boxes are determined based on scene information that translates the 2d mouse position into
*    a "3d" position in the view. (see: Mouse::getXYZ)
*    This 3d position must be within the bounding box of the 3d object
*
* 2. For items, the mouse-over (Item::isUnderMouse) depends on the screenRectangle dimensions (Common::Rect) and specifies the
*    targetable area for the item. The screenRectangle is calculated at every tick() of the item and depends on the item's animation,
*    current frame and thus facing angle.
*    The item's bounding box still determines the clickable area for the item. It is calculated at item's setup() or setXYZ()
*    and depends on the item's position (vector), height and width.
*/
bool Debugger::cmdObject(int argc, const char **argv) {
	bool invalidSyntax = false;

	if (argc < 3) {
		invalidSyntax = true;
	} else {
		Common::String modeName = argv[1];
		modeName.toLowercase();
		if (modeName == "add" && argc == 9) {
			// add object mode
			// first add to set and then to scene
			Common::String custObjName = "CUSTOMOBJ";
			Common::String custObjNameSuffix = argv[2];
			custObjName = custObjName + custObjNameSuffix;
			//
			// plain 3d objects (non-items) are added to the Set by reading the specific data for that Set
			// Their count is also stored in the Set's data,
			// if there's a need for a permanent extra obstacle in the Set, the code in set.cpp will have to be overridden (hard coded patch)
			//
			// For the debugger's purposes it should be harmless to add custom objects to test blocking pathways or other functions
			// they will persist in Save/Load but should go away when exiting a Set
			if (_vm->_scene->_set->_objectCount > 85) { //85 is the size of the _objects array in set.cpp
				debugPrintf("Unable to add more objects in the set\n");
			} else {
				int objectId = _vm->_scene->_set->_objectCount;
				++(_vm->_scene->_set->_objectCount);
				_vm->_scene->_set->_objects[objectId].name = custObjName.c_str();

				float x0, y0, z0, x1, y1, z1;
				x0 = atof(argv[3]);
				y0 = atof(argv[4]);
				z0 = atof(argv[5]);
				x1 = atof(argv[6]);
				y1 = atof(argv[7]);
				z1 = atof(argv[8]);

				_vm->_scene->_set->_objects[objectId].bbox = BoundingBox(x0, y0, z0, x1, y1, z1);
				_vm->_scene->_set->_objects[objectId].isObstacle = 0; // init as false
				_vm->_scene->_set->_objects[objectId].isClickable = 0;// init as false
				_vm->_scene->_set->_objects[objectId].isHotMouse = 0;
				_vm->_scene->_set->_objects[objectId].unknown1 = 0;
				_vm->_scene->_set->_objects[objectId].isTarget = 0;   // init as false
				//
				if (_vm->_sceneObjects->addObject(objectId + kSceneObjectOffsetObjects,
				                                  _vm->_scene->_set->_objects[objectId].bbox,
				                                  false,
				                                  false,
				                                  _vm->_scene->_set->_objects[objectId].unknown1,
				                                  false)
				) {
					debugPrintf("Object: %d: %s was added to set and scene\n", objectId, custObjName.c_str());
				} else {
					debugPrintf("Failed to add object: %d: %s to the scene\n", objectId, custObjName.c_str());
				}
				return true;
			}
		} else if ((modeName == "remove" && argc == 3)
		            || (modeName == "flags" && argc == 6)
					|| (modeName == "bounds" && argc == 9)
		            || (modeName == "list" && argc == 3)
		) {
			// remove object mode, show properties or edit flags
			int objectId = atoi(argv[2]);
			if (objectId >= 0 && objectId < _vm->_scene->_set->_objectCount) {
				Common::String objName = _vm->_scene->objectGetName(objectId);
				if (modeName == "list") {
					// list properties
					const BoundingBox &bbox = _vm->_scene->_set->_objects[objectId].bbox;
					Vector3 a, b;
					bbox.getXYZ(&a.x, &a.y, &a.z, &b.x, &b.y, &b.z);
					Vector3 pos = _vm->_view->calculateScreenPosition(0.5 * (a + b));
					// Intentional? When loading a game, in the loaded Scene:
					// an object can be a non-obstacle as a sceneObject but an obstacle as a Set object.
					// and this seems to be considered as a non-obstacle by the game in that Scene.
					// These are objects that the Unobstacle_Object() is called for when a scene is loaded (SceneLoaded())
					// So the sceneObject property overrides the Set object property in these cases
					// Eg. in PS01 BOX38 (object id: 19)
					// This inconsistency is fixed if you exit and re-enter the scene.
					debugPrintf("%d: %s (Clk: %s, Trg: %s, Obs: %s), Pos(%02.2f,%02.2f,%02.2f)\n     Bbox(%02.2f,%02.2f,%02.2f) ~ (%02.2f,%02.2f,%02.2f)\n",
					            objectId, objName.c_str(),
					            _vm->_scene->_set->_objects[objectId].isClickable? "T" : "F",
					            _vm->_scene->_set->_objects[objectId].isTarget?    "T" : "F",
					            _vm->_scene->_set->_objects[objectId].isObstacle?  "T" : "F",
					            pos.x, pos.y, pos.z,
					            a.x, a.y, a.z, b.x, b.y, b.z);
				} else if (modeName == "remove") {
					// scene's objectSetIsObstacle will update obstacle and walkpath
					_vm->_scene->objectSetIsObstacle(objectId, false, !_vm->_sceneIsLoading, true);
					// remove only from scene objects (keep in Set)
					if (!_vm->_sceneIsLoading && _vm->_sceneObjects->remove(objectId + kSceneObjectOffsetObjects)) {
						debugPrintf("Object: %d: %s was removed\n", objectId, objName.c_str());
					} else {
						debugPrintf("Failed to remove object: %d: %s\n", objectId, objName.c_str());
					}
				} else if (modeName == "bounds") {
					Vector3 positionBottomRight(atof(argv[3]), atof(argv[4]), atof(argv[5]));
					Vector3 positionTopLeft(atof(argv[6]), atof(argv[7]), atof(argv[8]));
					_vm->_scene->_set->_objects[objectId].bbox.setXYZ(positionBottomRight.x, positionBottomRight.y, positionBottomRight.z, positionTopLeft.x, positionTopLeft.y, positionTopLeft.z);
					if (!_vm->_sceneIsLoading && _vm->_sceneObjects->remove(objectId + kSceneObjectOffsetObjects)) {
						_vm->_sceneObjects->addObject(objectId + kSceneObjectOffsetObjects,
						                               _vm->_scene->_set->_objects[objectId].bbox,
						                               _vm->_scene->_set->_objects[objectId].isClickable,
						                               _vm->_scene->_set->_objects[objectId].isObstacle,
						                               _vm->_scene->_set->_objects[objectId].unknown1,
						                               _vm->_scene->_set->_objects[objectId].isTarget);
						// scene's objectSetIsObstacle will update obstacle and walkpath
						_vm->_scene->objectSetIsObstacle(objectId, _vm->_scene->_set->_objects[objectId].isObstacle, !_vm->_sceneIsLoading, true);
						debugPrintf("New bounds: (%02.2f,%02.2f,%02.2f) ~ (%02.2f,%02.2f,%02.2f)\n",
						             positionBottomRight.x, positionBottomRight.y, positionBottomRight.z,
						             positionTopLeft.x,   positionTopLeft.y,   positionTopLeft.z);
					}
				} else {
					// edit flags
					bool newClickable = atoi(argv[3])? true : false;
					bool newTarget    = atoi(argv[4])? true : false;
					bool newObstacle  = atoi(argv[5])? true : false;
					// scene's objectSetIsObstacle will update obstacle and walkpath
					_vm->_scene->objectSetIsObstacle(objectId, newObstacle, !_vm->_sceneIsLoading, true);
					_vm->_scene->objectSetIsClickable(objectId, newClickable, !_vm->_sceneIsLoading);
					_vm->_scene->objectSetIsTarget(objectId, newTarget, !_vm->_sceneIsLoading);

					debugPrintf("Setting obj %d: %s as clickable: %s, target: %s, obstacle: %s\n", objectId, objName.c_str(), newClickable? "T":"F", newTarget? "T":"F", newObstacle? "T":"F");
				}
				return true;
			} else {
				debugPrintf("Invalid object id %d was specified\n", objectId);
				return true;
			}
		} else {
			invalidSyntax = true;
		}
	}

	if (invalidSyntax) {
		debugPrintf("Add, edit flags, bounds or remove a 3d object obstacle in the current scene\n");
		debugPrintf("Use debugger command List with \"obj\" argument to view available targets for this command\n");
		debugPrintf("Floats:   brX, brY, brZ, tlX, tlY, tlZ, posX, posY, posZ\n");
		debugPrintf("Integers: id, incId\n");
		debugPrintf("Boolean (1: true, 0: false): isObstacle, isClickable, isTarget\n");
		debugPrintf("Usage 1: %s add    <incId> <brX> <brY> <brZ>  <tlX> <tlY> <tlZ>\n", argv[0]);
		debugPrintf("Usage 2: %s list   <id>\n", argv[0]);
		debugPrintf("Usage 3: %s flags  <id> <isClickable> <isTarget> <isObstacle>\n", argv[0]);
		debugPrintf("Usage 4: %s bounds <id> <brX> <brY> <brZ>  <tlX> <tlY> <tlZ>\n", argv[0]);
		debugPrintf("Usage 5: %s remove <id>\n", argv[0]);
	}
	return true;
}

/**
* Add a new, remove or edit flags and bounds for an existing item
*/
bool Debugger::cmdItem(int argc, const char **argv) {
	bool invalidSyntax = false;

	if (argc < 3) {
		invalidSyntax = true;
	} else {
		Common::String modeName = argv[1];
		modeName.toLowercase();
		int itemId = atoi(argv[2]);
		if (itemId < 0) {
			debugPrintf("Invalid item id: %d specified. Item id must be an integer >=0\n", itemId);
			return true;
		}

		if (modeName == "add" && argc == 10) {
			// add item mode
			if (_vm->_sceneObjects->findById(itemId + kSceneObjectOffsetItems) == -1) {
				Vector3 itemPosition(atof(argv[3]), atof(argv[4]), atof(argv[5]));
				int itemFacing = atoi(argv[6]);
				int itemHeight = atoi(argv[7]);
				int itemWidth = atoi(argv[8]);
				int itemAnimationId = atoi(argv[9]);
				if (_vm->_items->addToWorld(itemId, itemAnimationId, _vm->_scene->_setId, itemPosition, itemFacing, itemHeight, itemWidth, false, true, false, true)) {
					debugPrintf("Item: %d was added to set and scene\n", itemId);
				} else {
					debugPrintf("Failed to add item: %d to the scene\n", itemId);
				}
				return true;
			} else {
				debugPrintf("Item: %d is already in the scene\n", itemId);
			}
		} else if ((modeName == "remove" && argc == 3)
		            || (modeName == "flags" && argc == 5)
		            || (modeName == "bounds" && argc == 9)
		            || (modeName == "list" && argc == 3)
		) {
			// remove item mode, show properties or edit flags
			if (_vm->_sceneObjects->findById(itemId + kSceneObjectOffsetItems) != -1) {
				if (modeName == "list") {
					// list properties
					float xpos_curr, ypos_curr, zpos_curr, x0_curr, y0_curr, z0_curr, x1_curr, y1_curr, z1_curr;
					int currHeight, currWidth;
					int itemAnimationId;
					int facing_curr = _vm->_items->getFacing(itemId);
					_vm->_items->getWidthHeight(itemId, &currWidth, &currHeight);
					_vm->_items->getXYZ(itemId, &xpos_curr, &ypos_curr, &zpos_curr );
					_vm->_items->getBoundingBox(itemId).getXYZ(&x0_curr, &y0_curr, &z0_curr, &x1_curr, &y1_curr, &z1_curr);
					_vm->_items->getAnimationId(itemId, &itemAnimationId);
					const Common::Rect &screenRect = _vm->_items->getScreenRectangle(itemId);
					debugPrintf("Item %d (Trg: %s, Vis/Clk: %s) Pos(%02.2f,%02.2f,%02.2f), Face: %d, Height: %d, Width: %d AnimId: %d\n ScrRct(%d,%d,%d,%d) Bbox(%02.2f,%02.2f,%02.2f) ~ (%02.2f,%02.2f,%02.2f)\n",
					            itemId,
					            _vm->_items->isTarget(itemId)?  "T" : "F",
					            _vm->_items->isVisible(itemId)? "T" : "F",
					            xpos_curr, ypos_curr, zpos_curr,
					            facing_curr, currHeight, currWidth, itemAnimationId,
					            screenRect.top, screenRect.left, screenRect.bottom, screenRect.right,
					            x0_curr, y0_curr, z0_curr, x1_curr, y1_curr, z1_curr);
				} else if (modeName == "remove") {
					if (_vm->_sceneObjects->remove(itemId + kSceneObjectOffsetItems)) {
						debugPrintf("Item: %d was removed\n", itemId);
					} else {
						debugPrintf("Failed to remove item: %d\n", itemId);
					}
				} else if (modeName == "bounds") {
					// change position and facing to affect the screen rectangle
					Vector3 newPosition(atof(argv[3]), atof(argv[4]), atof(argv[5]));
					int newFacing = atoi(argv[6]);
					int newHeight = atoi(argv[7]);
					int newWidth =  atoi(argv[8]);
					//// setXYZ recalculates the item's bounding box
					//_vm->_items->setXYZ(itemId, newPosition);
					//// facing affects the angle and thus the screenRect in the next tick()
					_vm->_items->setFacing(itemId, newFacing);
					if (_vm->_sceneObjects->remove(itemId + kSceneObjectOffsetItems)) {
						float x0_new, y0_new, z0_new, x1_new, y1_new, z1_new;
						bool targetable = _vm->_items->isTarget(itemId);
						bool obstacle = _vm->_items->isVisible(itemId);
						bool polizeMazeEnemy = _vm->_items->isPoliceMazeEnemy(itemId);
						int itemAnimationId = -1;
						_vm->_items->getAnimationId(itemId, &itemAnimationId);
						_vm->_items->addToWorld(itemId, itemAnimationId, _vm->_scene->_setId, newPosition, newFacing, newHeight, newWidth, targetable, obstacle, polizeMazeEnemy, true);
						_vm->_items->getBoundingBox(itemId).getXYZ(&x0_new, &y0_new, &z0_new, &x1_new, &y1_new, &z1_new);
						debugPrintf("New Pos(%02.2f,%02.2f,%02.2f), Face: %d, Height: %d, Width: %d\n Bbox(%02.2f,%02.2f,%02.2f) ~ (%02.2f,%02.2f,%02.2f)\n",
						            newPosition.x, newPosition.y, newPosition.z,
						            newFacing, newHeight, newWidth,
						            x0_new, y0_new, z0_new, x1_new, y1_new, z1_new);
					}
				} else {
					// edit flags
					bool newObstacle  = (atoi(argv[3]) != 0);
					bool newTarget    = (atoi(argv[4]) != 0);
					_vm->_items->setIsObstacle(itemId, newObstacle);
					_vm->_items->setIsTarget(itemId, newTarget);
					debugPrintf("Setting item %d as visible/clickable: %s and target: %s\n", itemId, newObstacle? "T":"F", newTarget? "T":"F");
				}
				return true;
			} else {
				debugPrintf("No item was found with the specified id: %d in the scene\n", itemId);
				return true;
			}
		} else if (modeName == "spin" && argc == 3) {
			int itemAnimationId = atoi(argv[2]);
			if (itemAnimationId >=0 && itemAnimationId <= 996) {
				_vm->_itemPickup->setup(itemAnimationId, 320, 240);
				return false; // close the debugger
			} else {
				debugPrintf("Valid value range for item animation Ids is [0, 996]\n");
				return true;
			}
		} else {
			invalidSyntax = true;
		}
	}

	if (invalidSyntax) {
		debugPrintf("Add, edit flags, bounds or remove an item in the current scene\n");
		debugPrintf("Use debugger command List with \"items\" argument to view available targets for this command\n");
		debugPrintf("Floats:   posX, posY, posZ\n");
		debugPrintf("Integers: id, facing, height, width, animationId\n");
		debugPrintf("Boolean (1: true, 0: false): isVisible, isTarget\n");
		debugPrintf("Usage 1: %s add    <id> <posX> <posY> <posZ> <facing> <height> <width> <animationId>\n", argv[0]);
		debugPrintf("Usage 2: %s list   <id>\n", argv[0]);
		debugPrintf("Usage 3: %s flags  <id> <isVisible> <isTarget>\n", argv[0]);
		debugPrintf("Usage 4: %s bounds <id> <posX> <posY> <posZ> <facing> <height> <width>\n", argv[0]);
		debugPrintf("Usage 5: %s remove <id>\n", argv[0]);
		debugPrintf("Usage 6: %s spin   <animationId>\n", argv[0]);
	}
	return true;
}

/**
* Add a new or remove an existing region (plain or exit) into/from a specified slot in the _regions or _exits arrays respectively
*/
bool Debugger::cmdRegion(int argc, const char **argv) {
	bool invalidSyntax = false;

	if (argc < 4) {
		invalidSyntax = true;
	} else {
		Common::String regionTypeName = argv[1];
		regionTypeName.toLowercase();

		Regions *regions = nullptr;
		if (regionTypeName == "reg") {
			regions = _vm->_scene->_regions;
		} else if (regionTypeName == "exit") {
			regions = _vm->_scene->_exits;
		} else {
			debugPrintf("Invalid region name type was specified: %s\n", regionTypeName.c_str());
			return true;
		}

		Common::String modeName = argv[2];
		modeName.toLowercase();
		int regionID = atoi(argv[3]);
		if (regionID < 0 || regionID >= 10) {
			debugPrintf("A region id has to be an integer within [0, 9]\n");
			return true;
		}
		if (modeName == "add" && ((regionTypeName == "reg" && argc == 8) || (regionTypeName == "exit" && argc == 9)) ) {
			// add region mode
			if (!regions->_regions[regionID].present) {
				int type = 0;
				int topY    = atoi(argv[4]);
				int leftX   = atoi(argv[5]);
				int bottomY = atoi(argv[6]);
				int rightX  = atoi(argv[7]);
				if (regionTypeName == "exit") {
					type = atoi(argv[8]);
				}
				Common::Rect newRect(leftX, topY, rightX, bottomY);
				regions->add(regionID, newRect, type);
				debugPrintf("Adding %s: %d (t:%d l:%d b:%d r:%d) of type %d\n", regionTypeName.c_str(), regionID, newRect.top, newRect.left, newRect.bottom, newRect.right, type);
				return true;
			} else {
				debugPrintf("There already is an %s with the specified id: %d. Please select another slot id\n", regionTypeName.c_str(), regionID);
				return true;
			}
		} else if ((modeName == "remove" && argc == 4)
		           || (modeName == "list" && argc == 4)
		           || (modeName == "bounds" && argc == 8)) {
			if (regions->_regions[regionID].present) {
				Common::Rect origRect = regions->_regions[regionID].rectangle;
				int type = regions->_regions[regionID].type;
				if (modeName == "remove") {
					if (regions->remove(regionID)) {
						debugPrintf("Removed %s: %d (t:%d l:%d b:%d r:%d) of type: %d\n", regionTypeName.c_str(), regionID, origRect.top, origRect.left, origRect.bottom, origRect.right, type);
					} else {
						debugPrintf("Unable to remove %s: %d for unexpected reasons\n", regionTypeName.c_str(), regionID);
					}
				} else if (modeName == "bounds") {
					int topY, leftX, bottomY, rightX = 0;
					topY    = atoi(argv[4]);
					leftX   = atoi(argv[5]);
					bottomY = atoi(argv[6]);
					rightX  = atoi(argv[7]);

					if (regions->remove(regionID)) {
						Common::Rect newRect(leftX, topY, rightX, bottomY);
						regions->add(regionID, newRect, type);
						debugPrintf("Bounds %s: %d (t:%d l:%d b:%d r:%d)\n", regionTypeName.c_str(), regionID, newRect.top, newRect.left, newRect.bottom, newRect.right);
					}
				} else {
					// list properties
					debugPrintf("%s: %d (t:%d l:%d b:%d r:%d) of type: %d\n", regionTypeName.c_str(), regionID, origRect.top, origRect.left, origRect.bottom, origRect.right, type);
				}
				return true;
			} else {
				debugPrintf("The %s id %d specified does not exist in the scene\n", regionTypeName.c_str(), regionID);
				return true;
			}
		} else {
			invalidSyntax = true;
		}
	}

	if (invalidSyntax) {
		debugPrintf("Add, edit bounds or remove a region (\"reg\": for plain region, \"exit\": for exit) in the current scene\n");
		debugPrintf("Use debugger command List with \"reg\" argument to view available targets for this command\n");
		debugPrintf("An exit type can be in [0, 3] and determines the type of arrow icon on mouse-over\n0: Upward , 1: Right, 2: Downward, 3: Left\n");
		debugPrintf("Integers: id, topY, leftX, bottomY, rightX, type\n");
		debugPrintf("Usage 1: %s reg  add    <id> <topY> <leftX> <bottomY> <rightX>\n", argv[0]);
		debugPrintf("Usage 2: %s reg  remove <id>\n", argv[0]);
		debugPrintf("Usage 3: %s reg  list   <id>\n", argv[0]);
		debugPrintf("Usage 4: %s reg  bounds <id> <topY> <leftX> <bottomY> <rightX>\n", argv[0]);
		debugPrintf("Usage 5: %s exit add    <id> <topY> <leftX> <bottomY> <rightX> <type>\n", argv[0]);
		debugPrintf("Usage 6: %s exit remove <id>\n", argv[0]);
		debugPrintf("Usage 7: %s exit list   <id>\n", argv[0]);
		debugPrintf("Usage 8: %s exit bounds <id> <topY> <leftX> <bottomY> <rightX>\n", argv[0]);
	}
	return true;
}

/**
* click:  Toggle showing mouse click info in the text console (not the debugger window)
* beta:   Toggle beta crosshairs for aiming in combat mode, exit cursors (custom) and ESPER edge cursors (custom)
* add0:   Toggle semi-transparent hotspot cursor (additive draw mode 0)
* add1:   Toggle semi-transparent hotspot cursor (additive draw mode 1)
*/
bool Debugger::cmdMouse(int argc, const char **argv) {
	bool invalidSyntax = false;

	if (argc == 2) {
		//
		// set a debug variable to enable showing the mouse click info
		//
		Common::String argName = argv[1];
		argName.toLowercase();

		invalidSyntax = false;
		if (argName == "click") {
			_showMouseClickInfo = !_showMouseClickInfo;
		} else if (argName == "beta") {
			_useBetaCrosshairsCursor = !_useBetaCrosshairsCursor;
		} else if (argName == "add0") {
			_useAdditiveDrawModeForMouseCursorMode0  = !_useAdditiveDrawModeForMouseCursorMode0;
			_useAdditiveDrawModeForMouseCursorMode1  = false;
		} else if (argName == "add1") {
			_useAdditiveDrawModeForMouseCursorMode0  = false;
			_useAdditiveDrawModeForMouseCursorMode1  = !_useAdditiveDrawModeForMouseCursorMode1;
		} else {
			invalidSyntax = true;
		}

		if (!invalidSyntax) {
			debugPrintf("Showing mouse click info   = %s\n", _showMouseClickInfo ? "True":"False");
			debugPrintf("Showing beta crosshairs    = %s\n", _useBetaCrosshairsCursor ? "True":"False");
			debugPrintf("Mouse draw additive mode 0 = %s\n", _useAdditiveDrawModeForMouseCursorMode0 ? "True":"False");
			debugPrintf("Mouse draw additive mode 1 = %s\n", _useAdditiveDrawModeForMouseCursorMode1 ? "True":"False");
		}
	} else {
		invalidSyntax = true;
	}

	if (invalidSyntax) {
		debugPrintf("click: Toggle showing mouse info (on mouse click) in the text console\n");
		debugPrintf("beta:  Toggle beta crosshairs cursor\n");
		debugPrintf("add0:  Toggle semi-transparent hotspot cursor (additive mode 0)\n");
		debugPrintf("add1:  Toggle semi-transparent hotspot cursor (additive mode 1)\n");
		debugPrintf("Usage 1: %s click\n", argv[0]);
		debugPrintf("Usage 2: %s beta\n", argv[0]);
		debugPrintf("Usage 3: %s add0\n", argv[0]);
		debugPrintf("Usage 4: %s add1\n", argv[0]);
	}
	return true;
}

/**
* Auxiliary function to get a descriptive string for a given difficulty value
*/
Common::String Debugger::getDifficultyDescription(int difficultyValue) {
	Common::String difficultyStr;
	switch (difficultyValue) {
	default:
		// fall through
	case kGameDifficultyEasy:
		difficultyStr = Common::String::format("Easy (%d)", kGameDifficultyEasy);
		break;
	case kGameDifficultyMedium:
		difficultyStr = Common::String::format("Normal (%d)", kGameDifficultyMedium);
		break;
	case kGameDifficultyHard:
		difficultyStr = Common::String::format("Hard (%d)", kGameDifficultyHard);
		break;
	}
	return difficultyStr;
}

/**
* Show or set current game's difficulty mode
*/
bool Debugger::cmdDifficulty(int argc, const char **argv) {
	bool invalidSyntax = false;

	if (argc == 1) {
		debugPrintf("Current game difficulty is %s\n", getDifficultyDescription(_vm->_settings->getDifficulty()).c_str());
	} else if (argc == 2) {
		int difficultyID = atoi(argv[1]);
		if (difficultyID < kGameDifficultyEasy || difficultyID > kGameDifficultyHard) {
			debugPrintf("The difficulty value must be an integer within [0, 2]\n");
			return true;
		} else {
			_vm->_settings->setDifficulty(difficultyID);
			debugPrintf("Current game difficulty is set to %s\n", getDifficultyDescription(_vm->_settings->getDifficulty()).c_str());
		}
	} else {
		invalidSyntax = true;
	}

	if (invalidSyntax) {
		debugPrintf("Show or set current game's difficulty mode\n");
		debugPrintf("Valid difficulty values: \n");
		debugPrintf("0: Easy\n");
		debugPrintf("1: Normal\n");
		debugPrintf("2: Hard\n");
		debugPrintf("Usage 1: %s\n", argv[0]);
		debugPrintf("Usage 2: %s <difficulty>\n", argv[0]);
	}
	return true;
}
#if BLADERUNNER_ORIGINAL_BUGS
#else
bool Debugger::cmdEffect(int argc, const char **argv) {
	bool invalidSyntax = false;

	if (argc != 3) {
		invalidSyntax = true;
	} else {
		int effectId = atoi(argv[2]);
		Common::String modeName = argv[1];
		modeName.toLowercase();

		if (modeName == "list") {
			if (effectId >= 0 && effectId < (int)_vm->_screenEffects->_entries.size()) {
				ScreenEffects::Entry &entry = _vm->_screenEffects->_entries[effectId];
				debugPrintf("%2d. Effect (h: %d, x: %d, y: %d, z: %d\n", effectId, (int)entry.height, (int)entry.x, (int)entry.y, (int)entry.z);
			} else {
				debugPrintf("There is no such effect in the scene!\n");
			}
		} else if (modeName == "skip") {
			if (effectId >= 0 && effectId < (int)_vm->_screenEffects->_entries.size()) {
				_vm->_screenEffects->toggleEntry(effectId, true);
				debugPrintf("Skipped effect %2d\n", effectId);
			} else {
				debugPrintf("There is no such effect to remove in the scene!\n");
			}
		} else if (modeName == "restore") {
			if (effectId >= 0 && effectId < (int)_vm->_screenEffects->kMaxEffectsInScene) {
				_vm->_screenEffects->toggleEntry(effectId, false);
				debugPrintf("Attempting to restored effect %2d\n", effectId);
			}
		} else {
			invalidSyntax = true;
		}
	}

	if (invalidSyntax) {
		debugPrintf("List properties or skip/restore a screen-effect obj in the current scene\n");
		debugPrintf("Usage 1: %s  list     <id>\n", argv[0]);
		debugPrintf("Usage 2: %s  (skip | restore) <id>\n", argv[0]);
	}
	return true;
}
#endif // BLADERUNNER_ORIGINAL_BUGS

/**
* Toggle playing a full VK session (full) and showing current test statistics as subtitles
* Only available in VK mode
*/
bool Debugger::cmdVk(int argc, const char **argv) {
	bool invalidSyntax = false;

	if (argc != 2) {
		invalidSyntax = true;
	} else {
		if (!_vm->_vk->isOpen()) {
			debugPrintf("Error:Command %s is only valid within a Voigt-Kampff session\n",  argv[0]);
			return true;
		}
		//
		// set a debug variable to enable going through the (remaining) VK session
		// enabling all the remaining VK questions
		//
		Common::String argName = argv[1];
		argName.toLowercase();
		if (argc == 2 && argName == "full") {
			_playFullVk = !_playFullVk;
			debugPrintf("Playing full V-K session = %s\n", _playFullVk ? "True":"False");
		} else if (argc == 2 && argName == "stats") {
			_showStatsVk = !_showStatsVk;
			debugPrintf("Showing V-K session statistics= %s\n", _showStatsVk ? "True":"False");
		} else {
			invalidSyntax = true;
		}
	}

	if (invalidSyntax) {
		debugPrintf("Toggle playing the full VK session instead of the at most 10 questions of the vanilla mode\n");
		debugPrintf("Also, toggle showing statistics for the current session\n");
		debugPrintf("Usage: %s (full|stats)\n", argv[0]);
	}
	return true;

}
/**
*
* Similar to draw but only list items instead of drawing
* Maybe keep this separate from the draw command, even though some code gets repeated here
*/
bool Debugger::cmdList(int argc, const char **argv) {
	bool invalidSyntax = false;

	if (argc < 2) {
		invalidSyntax = true;
	} else {
		Common::String arg = argv[1];
		if (arg == "act") {
			if (argc == 2) {
				debugPrintf("Listing scene actors: \n");
				int count = 0;
				for (int i = 0; i < _vm->_sceneObjects->_count; ++i) {
					SceneObjects::SceneObject *sceneObject = &_vm->_sceneObjects->_sceneObjects[_vm->_sceneObjects->_sceneObjectsSortedByDistance[i]];

					if (sceneObject->type == kSceneObjectTypeActor) {

						Actor *actor = _vm->_actors[sceneObject->id - kSceneObjectOffsetActors];
						const Common::Rect &screenRect = actor->getScreenRectangle();
						const BoundingBox &bbox = actor->getBoundingBox();
						Vector3 a, b;
						bbox.getXYZ(&a.x, &a.y, &a.z, &b.x, &b.y, &b.z);

						debugPrintf("%d: %s (Clk: %s, Trg: %s, Prs: %s, Obs: %s, Mvg: %s)\n",
						             sceneObject->id - kSceneObjectOffsetActors,
						             _vm->_textActorNames->getText(sceneObject->id - kSceneObjectOffsetActors),
						             sceneObject->isClickable? "T" : "F",
						             sceneObject->isTarget?    "T" : "F",
						             sceneObject->isPresent?   "T" : "F",
						             sceneObject->isObstacle?  "T" : "F",
						             sceneObject->isMoving?    "T" : "F");
						debugPrintf("    Goal: %d, Set: %d, Anim mode: %d id:%d fps: %d showDmg: %s inCombat: %s\n",
						             actor->getGoal(),
						             actor->getSetId(),
						             actor->getAnimationMode(),
						             actor->getAnimationId(),
						             actor->getFPS(),
						             actor->getFlagDamageAnimIfMoving()? "T" : "F",
						             actor->inCombat()? "T" : "F");
						debugPrintf("    Pos(%02.2f,%02.2f,%02.2f)\n",
						             actor->getPosition().x,
						             actor->getPosition().y,
						             actor->getPosition().z);
						debugPrintf("    ScreenRect(%03d,%03d,%03d,%03d)\n",
						             screenRect.top, screenRect.left, screenRect.bottom, screenRect.right);
						debugPrintf("    Bbox(%02.2f,%02.2f,%02.2f) ~ (%02.2f,%02.2f,%02.2f)\n",
						             a.x, a.y, a.z, b.x, b.y, b.z);
						++count;
					}
				}
				debugPrintf("%d actors were found in scene.\n", count);
			} else if (argc == 3) {
				// list properties for specific actor regardless of the set/ scene they are in
				int actorId = atoi(argv[2]);
				if (actorId >= 0 && actorId < _vm->kActorCount) {
					debugPrintf("Showing properties for actor: %d:%s \n", actorId, _vm->_textActorNames->getText(actorId));
					Actor *actor = _vm->_actors[actorId];

					bool isReplicant = false;
					switch (actorId) {
					case kActorIzo:
						isReplicant = _vm->_gameFlags->query(kFlagIzoIsReplicant);
						break;
					case kActorGordo:
						isReplicant = _vm->_gameFlags->query(kFlagGordoIsReplicant);
						break;
					case kActorLucy:
						isReplicant = _vm->_gameFlags->query(kFlagLucyIsReplicant);
						break;
					case kActorDektora:
						isReplicant = _vm->_gameFlags->query(kFlagDektoraIsReplicant);
						break;
					case kActorSadik:
						isReplicant = _vm->_gameFlags->query(kFlagSadikIsReplicant);
						break;
					case kActorLuther:
						isReplicant = _vm->_gameFlags->query(kFlagLutherLanceIsReplicant);
						break;
					default:
						isReplicant = false;
						break;
					}

					debugPrintf("%d: %s (Mvg: %s, Walk:%s, Run:%s, Ret:%s, Trg: %s, Rep: %s)\n    Hp: %d, Fac: %d, Friend: %d\n    Goal: %d, Set: %d, Anim mode: %d id:%d showDmg: %s inCombat: %s\n    Pos(%02.2f,%02.2f,%02.2f), Walkbx:%d\n",
						actorId,
						_vm->_textActorNames->getText(actorId),
						actor->isMoving()?  "T" : "F",
						actor->isWalking()? "T" : "F",
						actor->isRunning()? "T" : "F",
						actor->isRetired()? "T" : "F",
						actor->isTarget()?  "T" : "F",
						isReplicant?        "T" : "F",
						actor->getCurrentHP(),
						actor->getFacing(),
						actor->getFriendlinessToOther(kActorMcCoy),
						actor->getGoal(),
						actor->getSetId(),
						actor->getAnimationMode(),
						actor->getAnimationId(),
						actor->getFlagDamageAnimIfMoving()? "T" : "F",
						actor->inCombat()? "T" : "F",
						actor->getPosition().x,
						actor->getPosition().y,
						actor->getPosition().z,
						actor->getWalkbox());
				} else {
					debugPrintf("Invalid actor id: %d was specified\n", actorId);
					return true;
				}
			} else {
				invalidSyntax = true;
			}
		} else if (arg == "obj") {
			debugPrintf("View info\nCamera position: (%5.2f, %5.2f, %5.2f), Viewport position: (%5.2f, %5.2f, %5.2f), FoVx: %2.2f\n",
						_vm->_view->_cameraPosition.x,
						_vm->_view->_cameraPosition.y,
						_vm->_view->_cameraPosition.z,
						_vm->_view->_viewportPosition.x,
						_vm->_view->_viewportPosition.y,
						_vm->_view->_viewportPosition.z,
						_vm->_view->_fovX);
			debugPrintf("Listing scene objects: \n");
			int count = 0;
			for (int i = 0; i < _vm->_sceneObjects->_count; ++i) {
				SceneObjects::SceneObject *sceneObject = &_vm->_sceneObjects->_sceneObjects[_vm->_sceneObjects->_sceneObjectsSortedByDistance[i]];
				const BoundingBox &bbox = sceneObject->boundingBox;
				Vector3 a, b;
				bbox.getXYZ(&a.x, &a.y, &a.z, &b.x, &b.y, &b.z);
				Vector3 pos = _vm->_view->calculateScreenPosition(0.5 * (a + b));

				if (sceneObject->type == kSceneObjectTypeUnknown) {
					debugPrintf("%02d. Unknown object type\n", count);
					++count;
				} else if (sceneObject->type == kSceneObjectTypeObject) {
					debugPrintf("%d: %s (Clk: %s, Trg: %s, Prs: %s, Obs: %s, Mvg: %s), Pos(%02.2f,%02.2f,%02.2f)\n     Bbox(%02.2f,%02.2f,%02.2f) ~ (%02.2f,%02.2f,%02.2f)\n",
								 sceneObject->id - kSceneObjectOffsetObjects,
								 _vm->_scene->objectGetName(sceneObject->id - kSceneObjectOffsetObjects).c_str(),
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
		} else if (arg == "item") {
			debugPrintf("Listing scene items: \n");
			int count = 0;
			for (int i = 0; i < _vm->_sceneObjects->_count; ++i) {
				SceneObjects::SceneObject *sceneObject = &_vm->_sceneObjects->_sceneObjects[_vm->_sceneObjects->_sceneObjectsSortedByDistance[i]];

				if (sceneObject->type == kSceneObjectTypeItem) {
					const BoundingBox &bbox = sceneObject->boundingBox;
					Vector3 a, b;
					bbox.getXYZ(&a.x, &a.y, &a.z, &b.x, &b.y, &b.z);
					//Vector3 pos = _vm->_view->calculateScreenPosition(0.5 * (a + b));
					Vector3 pos;
					int currHeight, currWidth;
					_vm->_items->getXYZ(sceneObject->id - kSceneObjectOffsetItems, &pos.x, &pos.y, &pos.z);
					_vm->_items->getWidthHeight(sceneObject->id - kSceneObjectOffsetItems, &currWidth, &currHeight);
					const Common::Rect &screenRect = _vm->_items->getScreenRectangle(sceneObject->id - kSceneObjectOffsetItems);
					debugPrintf("Id %i, Pos(%02.2f,%02.2f,%02.2f), Face: %d, Height: %d, Width: %d, ScrRct(%d,%d,%d,%d)\n Clk: %s, Trg: %s, Prs: %s, Vis: %s, Mvg: %s Bbox(%02.2f,%02.2f,%02.2f)~(%02.2f,%02.2f,%02.2f)\n",
								 sceneObject->id - kSceneObjectOffsetItems,
								 pos.x, pos.y, pos.z,
								 _vm->_items->getFacing(sceneObject->id - kSceneObjectOffsetItems), currHeight, currWidth,
								 screenRect.top, screenRect.left, screenRect.bottom, screenRect.right,
								 sceneObject->isClickable? "T" : "F",
								 sceneObject->isTarget?    "T" : "F",
								 sceneObject->isPresent?   "T" : "F",
								 sceneObject->isObstacle?  "T" : "F",
								 sceneObject->isMoving?    "T" : "F",
								 a.x, a.y, a.z, b.x, b.y, b.z);
					++count;
				}
			}
			debugPrintf("%d items were found in scene.\n", count);
		} else if (arg == "reg") {
			debugPrintf("Listing plain regions: \n");
			int count = 0;
			//list regions
			for (int i = 0; i < 10; ++i) {
				Regions::Region *region = &_vm->_scene->_regions->_regions[i];
				if (!region->present) continue;
				Common::Rect r = region->rectangle;
				debugPrintf("Region slot: %d (t:%d l:%d b:%d r:%d)\n", i, r.top, r.left, r.bottom, r.right);
				++count;
			}

			debugPrintf("Listing exits: \n");
			//list exits
			for (int i = 0; i < 10; ++i) {
				Regions::Region *region = &_vm->_scene->_exits->_regions[i];
				if (!region->present) continue;
				Common::Rect r = region->rectangle;
				debugPrintf("Exit slot: %d (t:%d l:%d b:%d r:%d)\n", i, r.top, r.left, r.bottom, r.right);
				++count;
			}
			debugPrintf("%d regions (plain and exits) were found in scene.\n", count);
		} else if (arg == "way") {
			debugPrintf("Listing waypoints: \n");
			int count = 0;
			for (int i = 0; i < _vm->_waypoints->_count; ++i) {
				Waypoints::Waypoint *waypoint = &_vm->_waypoints->_waypoints[i];
				if (waypoint->setId != _vm->_scene->getSetId()) {
					continue;
				}
				Vector3 a = waypoint->position;
				debugPrintf("Waypoint %i, Pos(%02.2f,%02.2f,%02.2f)\n", i, a.x, a.y, a.z);
				++count;
			}

			// list combat cover waypoints
			for (int i = 0; i < (int)_vm->_combat->_coverWaypoints.size(); ++i) {
				Combat::CoverWaypoint *cover = &_vm->_combat->_coverWaypoints[i];
				if (cover->setId != _vm->_scene->getSetId()) {
					continue;
				}
				Vector3 a = cover->position;
				debugPrintf("Cover %i, Pos(%02.2f,%02.2f,%02.2f)\n", i, a.x, a.y, a.z);
				++count;
			}

			// list combat flee waypoints
			for (int i = 0; i < (int)_vm->_combat->_fleeWaypoints.size(); ++i) {
				Combat::FleeWaypoint *flee = &_vm->_combat->_fleeWaypoints[i];
				if (flee->setId != _vm->_scene->getSetId()) {
					continue;
				}
				Vector3 a = flee->position;
				debugPrintf("Flee %i, Pos(%02.2f,%02.2f,%02.2f)\n", i, a.x, a.y, a.z);
				++count;
			}
			debugPrintf("%d waypoints were found in scene.\n", count);
		} else if (arg == "walk") {
			debugPrintf("Listing walkboxes: \n");
			// list walkboxes
			for (int i = 0; i < _vm->_scene->_set->_walkboxCount; ++i) {
				Set::Walkbox *walkbox = &_vm->_scene->_set->_walkboxes[i];

				debugPrintf("%2d. Walkbox %s, vertices: %d\n", i, walkbox->name.c_str(), walkbox->vertexCount);
			}
			debugPrintf("%d walkboxes were found in scene.\n", _vm->_scene->_set->_walkboxCount);
		} else if (arg == "fog") {
			debugPrintf("Listing fogs: \n");
			int count = 0;
			for (Fog *fog = _vm->_scene->_set->_effects->_fogs; fog != nullptr; fog = fog->_next) {
				debugPrintf("%2d. Fog %s\n", count, fog->_name.c_str());
				++count;
			}
			debugPrintf("%d fogs were found in scene.\n", count);
		} else if (arg == "lit") {
			debugPrintf("Listing lights: \n");
			// list lights
			for (int i = 0; i < (int)_vm->_lights->_lights.size(); ++i) {
				Light *light = _vm->_lights->_lights[i];
				debugPrintf("%2d. Light %s\n", i, light->_name.c_str());
			}
			debugPrintf("%d lights were found in scene.\n", (int)_vm->_lights->_lights.size());
		} else if (arg == "eff") {
			debugPrintf("Listing scene effects: \n");
			// list scene effects
			for (uint i = 0; i < _vm->_screenEffects->_entries.size(); ++i) {
				ScreenEffects::Entry &entry = _vm->_screenEffects->_entries[i];
				debugPrintf("%2d. Effect (h: %d, x: %d, y: %d, z: %d\n", i, (int)entry.height, (int)entry.x, (int)entry.y, (int)entry.z);
			}
			debugPrintf("%d scene effects were found in scene.\n", (int)_vm->_screenEffects->_entries.size());
		} else {
			debugPrintf("Invalid item type was specified.\n");
		}
	}

	if (invalidSyntax) {
		debugPrintf("Enables debug listing of actors, scene objects, items, waypoints, regions, lights, fogs and walk-boxes.\n");
		debugPrintf("Usage 1: %s (act | obj | item | way | reg | eff | lit | fog | walk )\n", argv[0]);
		debugPrintf("Usage 2: %s act <actorId>\n", argv[0]);
	}
	return true;
}


void Debugger::drawDebuggerOverlay() {

	updateTogglesForDbgDrawListInCurrentSetAndScene();

	if (_viewActorsToggle || _specificActorsDrawn
	    || _view3dObjectsToggle || _specific3dObjectsDrawn
	    || _viewItemsToggle || _specificItemsDrawn
	) {
		drawSceneObjects();
	}
	if (_viewScreenEffects || _specificEffectsDrawn) drawScreenEffects();
	if (_viewLights || _specificLightsDrawn) drawLights();
	if (_viewFogs || _specificFogsDrawn) drawFogs();
	if (_viewRegionsNormalToggle || _specificRegionNormalDrawn
	    || _viewRegionsExitsToggle || _specificRegionExitsDrawn
	) {
		drawRegions();
	}
	if (_viewWaypointsNormalToggle || _specificWaypointNormalDrawn
	    || _viewWaypointsFleeToggle || _specificWaypointFleeDrawn
	    || _viewWaypointsCoverToggle || _specificWaypointCoverDrawn
	) {
		drawWaypoints();
	}
	if (_viewWalkboxes || _specificWalkboxesDrawn) drawWalkboxes();
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
		for (int i = 0; i < count; ++i) {
			SceneObjects::SceneObject *sceneObject = &_vm->_sceneObjects->_sceneObjects[_vm->_sceneObjects->_sceneObjectsSortedByDistance[i]];

			const BoundingBox &bbox = sceneObject->boundingBox;
			Vector3 a, b;
			bbox.getXYZ(&a.x, &a.y, &a.z, &b.x, &b.y, &b.z);
			Vector3 pos = _vm->_view->calculateScreenPosition(0.5 * (a + b));
			int color;

			switch (sceneObject->type) {
			default:
				// fallthrough intended
			case kSceneObjectTypeUnknown:
				break;
			case kSceneObjectTypeActor:
				if (_viewActorsToggle
				    || (_specificActorsDrawn && findInDbgDrawList(debuggerObjTypeActor, sceneObject->id - kSceneObjectOffsetActors, -1, -1) != -1)
				) {
					color = _vm->_surfaceFront.format.RGBToColor(255, 0, 0);
					drawBBox(a, b, _vm->_view, &_vm->_surfaceFront, color);
					Actor *actor = _vm->_actors[sceneObject->id - kSceneObjectOffsetActors];
					//const Common::Rect &screenRect = actor->getScreenRectangle();
					//_vm->_surfaceFront.frameRect(screenRect, color);
					_vm->_surfaceFront.frameRect(sceneObject->screenRectangle, color);
					_vm->_mainFont->drawString(&_vm->_surfaceFront, _vm->_textActorNames->getText(actor->getId()), pos.x, pos.y, _vm->_surfaceFront.w, color);
				}
				break;
			case kSceneObjectTypeItem:
				if (_viewItemsToggle
				    || (_specificItemsDrawn && findInDbgDrawList(debuggerObjTypeItem, sceneObject->id - kSceneObjectOffsetItems, -1, -1) != -1)
				) {
					color = _vm->_surfaceFront.format.RGBToColor(0, 255, 0);
					drawBBox(a, b, _vm->_view, &_vm->_surfaceFront, color);
					Common::String itemText = Common::String::format("item %i", sceneObject->id - kSceneObjectOffsetItems);
					_vm->_surfaceFront.frameRect(sceneObject->screenRectangle, color);
					_vm->_mainFont->drawString(&_vm->_surfaceFront, itemText, pos.x, pos.y, _vm->_surfaceFront.w, color);
				}
				break;
			case kSceneObjectTypeObject:
				if (_view3dObjectsToggle
				    || (_specific3dObjectsDrawn && findInDbgDrawList(debuggerObjType3dObject, sceneObject->id - kSceneObjectOffsetObjects, _vm->_scene->getSetId(), _vm->_scene->getSceneId()) != -1)
				) {
					color = _vm->_surfaceFront.format.RGBToColor(127, 127, 127);
					if (sceneObject->isClickable) {
						color = _vm->_surfaceFront.format.RGBToColor(0, 255, 0);
					}
					drawBBox(a, b, _vm->_view, &_vm->_surfaceFront, color);
					_vm->_surfaceFront.frameRect(sceneObject->screenRectangle, color);
					_vm->_mainFont->drawString(&_vm->_surfaceFront, _vm->_scene->objectGetName(sceneObject->id - kSceneObjectOffsetObjects), pos.x, pos.y, _vm->_surfaceFront.w, color);
				}
				break;
			}
		}
	}
}

void Debugger::drawLights() {
	// draw lights
	for (int i = 0; i < (int)_vm->_lights->_lights.size(); ++i) {
		if (_viewLights
		    || (_specificLightsDrawn && findInDbgDrawList(debuggerObjTypeLight, i, _vm->_scene->getSetId(), _vm->_scene->getSceneId()) != -1)
		) {
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

			_vm->_mainFont->drawString(&_vm->_surfaceFront, light->_name, posOriginT.x, posOriginT.y, _vm->_surfaceFront.w, color);
		}
	}
}

void Debugger::drawFogs() {
	Fog *fog = _vm->_scene->_set->_effects->_fogs;
	for (int i = 0; fog != nullptr; ++i) {
		if (_viewFogs
		    || (_specificFogsDrawn && findInDbgDrawList(debuggerObjTypeFog, i, _vm->_scene->getSetId(), _vm->_scene->getSceneId()) != -1)
		) {
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

			_vm->_mainFont->drawString(&_vm->_surfaceFront, fog->_name, posOriginT.x, posOriginT.y, _vm->_surfaceFront.w, color);
		}
		fog = fog->_next;
	}
}

void Debugger::drawRegions() {
	if (_viewRegionsNormalToggle || _specificRegionNormalDrawn) {
		//draw regions
		for (int i = 0; i < 10; ++i) {
			Regions::Region *region = &_vm->_scene->_regions->_regions[i];
			if (!region->present) continue;
			if (_viewRegionsNormalToggle
				|| (_specificRegionNormalDrawn && findInDbgDrawList(debuggerObjTypeRegionNormal, i, _vm->_scene->getSetId(), _vm->_scene->getSceneId()) != -1)
			) {
				_vm->_surfaceFront.frameRect(region->rectangle, _vm->_surfaceFront.format.RGBToColor(0, 0, 255));
			}
		}
	}

	if (_viewRegionsExitsToggle || _specificRegionExitsDrawn) {
		//draw exits
		for (int i = 0; i < 10; ++i) {
			Regions::Region *region = &_vm->_scene->_exits->_regions[i];
			if (!region->present) continue;
			if (_viewRegionsExitsToggle
				|| (_specificRegionExitsDrawn && findInDbgDrawList(debuggerObjTypeRegionExit, i, _vm->_scene->getSetId(), _vm->_scene->getSceneId()) != -1)
			) {
				_vm->_surfaceFront.frameRect(region->rectangle, _vm->_surfaceFront.format.RGBToColor(255, 255, 255));
			}
		}
	}
}

void Debugger::drawWaypoints() {
	if (_viewWaypointsNormalToggle || _specificWaypointNormalDrawn)  {
		//draw world waypoints
		for (int i = 0; i < _vm->_waypoints->_count; ++i) {
			Waypoints::Waypoint *waypoint = &_vm->_waypoints->_waypoints[i];
			if (waypoint->setId != _vm->_scene->getSetId()) {
				continue;
			}
			if (_viewWaypointsNormalToggle
				|| (_specificWaypointNormalDrawn && findInDbgDrawList(debuggerObjTypeWaypointNorm, i, -1, -1) != -1)
			) {
				Vector3 pos = waypoint->position;
				Vector3 size = Vector3(3.0f, 3.0f, 3.0f);
				int color = _vm->_surfaceFront.format.RGBToColor(255, 255, 255);
				drawBBox(pos - size, pos + size, _vm->_view, &_vm->_surfaceFront, color);
				Vector3 spos = _vm->_view->calculateScreenPosition(pos);
				Common::String waypointText = Common::String::format("waypoint %i", i);
				_vm->_mainFont->drawString(&_vm->_surfaceFront, waypointText, spos.x, spos.y, _vm->_surfaceFront.w, color);
			}
		}
	}

	if (_viewWaypointsCoverToggle || _specificWaypointCoverDrawn) {
		//draw combat cover waypoints
		for (int i = 0; i < (int)_vm->_combat->_coverWaypoints.size(); ++i) {
			Combat::CoverWaypoint *cover = &_vm->_combat->_coverWaypoints[i];
			if (cover->setId != _vm->_scene->getSetId()) {
				continue;
			}
			if (_viewWaypointsCoverToggle
				|| (_specificWaypointCoverDrawn && findInDbgDrawList(debuggerObjTypeWaypointCover, i, -1, -1) != -1)
			) {
				Vector3 pos = cover->position;
				Vector3 size = Vector3(3.0f, 3.0f, 3.0f);
				int color = _vm->_surfaceFront.format.RGBToColor(255, 0, 255);
				drawBBox(pos - size, pos + size, _vm->_view, &_vm->_surfaceFront, color);
				Vector3 spos = _vm->_view->calculateScreenPosition(pos);
				Common::String coverText = Common::String::format("cover %i", i);
				_vm->_mainFont->drawString(&_vm->_surfaceFront, coverText, spos.x, spos.y, _vm->_surfaceFront.w, color);
			}
		}
	}

	if (_viewWaypointsFleeToggle || _specificWaypointFleeDrawn) {
		//draw combat flee waypoints
		for (int i = 0; i < (int)_vm->_combat->_fleeWaypoints.size(); ++i) {
			Combat::FleeWaypoint *flee = &_vm->_combat->_fleeWaypoints[i];
			if (flee->setId != _vm->_scene->getSetId()) {
				continue;
			}
			if (_viewWaypointsFleeToggle
				|| (_specificWaypointFleeDrawn && findInDbgDrawList(debuggerObjTypeWaypoingFlee, i, -1, -1) != -1)
			) {
				Vector3 pos = flee->position;
				Vector3 size = Vector3(3.0f, 3.0f, 3.0f);
				int color = _vm->_surfaceFront.format.RGBToColor(0, 255, 255);
				drawBBox(pos - size, pos + size, _vm->_view, &_vm->_surfaceFront, color);
				Vector3 spos = _vm->_view->calculateScreenPosition(pos);
				Common::String fleeText = Common::String::format("flee %i", i);
				_vm->_mainFont->drawString(&_vm->_surfaceFront, fleeText, spos.x, spos.y, _vm->_surfaceFront.w, color);
			}
		}
	}
}

void Debugger::drawWalkboxes() {
	//draw walkboxes
	for (int i = 0; i < _vm->_scene->_set->_walkboxCount; ++i) {
		if (_viewWalkboxes
		    || (_specificWalkboxesDrawn && findInDbgDrawList(debuggerObjTypeWalkbox, i, _vm->_scene->getSetId(), _vm->_scene->getSceneId()) != -1)
		) {
			Set::Walkbox *walkbox = &_vm->_scene->_set->_walkboxes[i];
			for (int j = 0; j < walkbox->vertexCount; ++j) {
				Vector3 start = _vm->_view->calculateScreenPosition(walkbox->vertices[j]);
				Vector3 end = _vm->_view->calculateScreenPosition(walkbox->vertices[(j + 1) % walkbox->vertexCount]);
				_vm->_surfaceFront.drawLine(start.x, start.y, end.x, end.y, _vm->_surfaceFront.format.RGBToColor(255, 255, 0));
				Vector3 pos = _vm->_view->calculateScreenPosition(0.5 * (walkbox->vertices[j] + walkbox->vertices[(j + 1) % walkbox->vertexCount]));
				_vm->_mainFont->drawString(&_vm->_surfaceFront, walkbox->name, pos.x, pos.y, _vm->_surfaceFront.w, _vm->_surfaceFront.format.RGBToColor(255, 255, 0));
			}
		}
	}
}

void Debugger::drawScreenEffects() {
	//draw aesc
	for (uint i = 0; i < _vm->_screenEffects->_entries.size(); ++i) {
		if (_viewScreenEffects
		    || (_specificEffectsDrawn && findInDbgDrawList(debuggerObjTypeEffect, i, _vm->_scene->getSetId(), _vm->_scene->getSceneId()) != -1)
		) {
			ScreenEffects::Entry &entry = _vm->_screenEffects->_entries[i];
			int j = 0;
			for (int y = 0; y < entry.height; ++y) {
				for (int x = 0; x < entry.width; ++x) {
					Common::Rect r((entry.x + x) * 2, (entry.y + y) * 2, (entry.x + x) * 2 + 2, (entry.y + y) * 2 + 2);

					int ec = entry.data[j++];
					// We need to convert from 5 bits per channel (r,g,b) to 8 bits
					int color = _vm->_surfaceFront.format.RGBToColor(
						Color::get8BitColorFrom5Bit(entry.palette[ec].r),
						Color::get8BitColorFrom5Bit(entry.palette[ec].g),
						Color::get8BitColorFrom5Bit(entry.palette[ec].b));
					_vm->_surfaceFront.fillRect(r, color);
				}
			}
		}
	}
}

void Debugger::toggleObjectInDbgDrawList(DebuggerDrawnObject &drObj) {
	if (drObj.type == debuggerObjTypeUndefined || drObj.objId < 0) {
		return;
	}

	//  Check if there already exists such object in the list:
	//  if it exists then do a remove action
	//  else do a push_back action (provided that size() < kMaxSpecificObjectsDrawnCount)
	int foundAt = findInDbgDrawList(drObj.type, drObj.objId, drObj.setId, drObj.sceneId);

	if (foundAt >= 0) {
		_specificDrawnObjectsList.remove_at(foundAt);
	} else {
		if (_specificDrawnObjectsList.size() < kMaxSpecificObjectsDrawnCount) {
			// TODO check if there actually is an object of such an ID in the current scene!
			_specificDrawnObjectsList.push_back(drObj);
		} else {
			debugPrintf("The specific drawn objects list is full. Try running a draw reset or explicitly removing objects from it\n");
		}
	}
}

/**
* drObjType can be a valid object type, or debuggerObjTypeUndefined as a wildcard for all object types
* drObjId can be a valid object id or -1 as a wildcard
* drObjSetId can be a valid Set id or -1 as a wildcard
* drObjSceneId can be a valid Scene id or -1 as a wildcard
* return the position in the list, if the specific drawn objects list contains the object of the specified type and id
*        and it belongs to the current set and scene,
*        or -1 otherwise
*/
int Debugger::findInDbgDrawList(DebuggerDrawnObjectType drObjType, int drObjId, int drObjSetId, int drObjSceneId) {
	if (_specificDrawnObjectsList.empty()) {
		return -1;
	}

	for (int i = 0; i < (int) _specificDrawnObjectsList.size(); ++i) {
		if ((drObjType == debuggerObjTypeUndefined || drObjType == _specificDrawnObjectsList[i].type)
			&& (drObjId == -1 || drObjId == _specificDrawnObjectsList[i].objId)
		    && (drObjSetId == -1 || _specificDrawnObjectsList[i].setId == -1 || drObjSetId == _specificDrawnObjectsList[i].setId)
		    && (drObjSceneId == -1 || _specificDrawnObjectsList[i].sceneId == -1 || drObjSceneId == _specificDrawnObjectsList[i].sceneId)
		) {
			// TODO for actors, 3d objects, items and waypoints it's probably preferable to ignore the sceneId (?)
			return i;
		}
	}
	return -1;
}

void Debugger::updateTogglesForDbgDrawListInCurrentSetAndScene() {
	_specificActorsDrawn = false;
	_specific3dObjectsDrawn = false;
	_specificItemsDrawn = false;
	_specificEffectsDrawn = false;
	_specificLightsDrawn = false;
	_specificFogsDrawn = false;
	_specificRegionNormalDrawn = false;
	_specificRegionExitsDrawn = false;
	_specificWaypointNormalDrawn = false;
	_specificWaypointFleeDrawn = false;
	_specificWaypointCoverDrawn = false;
	_specificWalkboxesDrawn = false;
	for (int i = 0; i < (int) _specificDrawnObjectsList.size(); ++i) {
		if ((_specificDrawnObjectsList[i].sceneId == -1 ||  _specificDrawnObjectsList[i].sceneId == _vm->_scene->getSceneId() )
		    && (_specificDrawnObjectsList[i].setId == -1 || _specificDrawnObjectsList[i].setId == _vm->_scene->getSetId())
		) {
			switch (_specificDrawnObjectsList[i].type) {
			case debuggerObjTypeActor:
				_specificActorsDrawn = true;
				break;
			case debuggerObjType3dObject:
				_specific3dObjectsDrawn = true;
				break;
			case debuggerObjTypeItem:
				_specificItemsDrawn = true;
				break;
			case debuggerObjTypeRegionNormal:
				_specificRegionNormalDrawn = true;
				break;
			case debuggerObjTypeRegionExit:
				_specificRegionExitsDrawn = true;
				break;
			case debuggerObjTypeWaypointNorm:
				_specificWaypointNormalDrawn = true;
				break;
			case debuggerObjTypeWaypoingFlee:
				_specificWaypointFleeDrawn = true;
				break;
			case debuggerObjTypeWaypointCover:
				_specificWaypointCoverDrawn = true;
				break;
			case debuggerObjTypeWalkbox:
				_specificWalkboxesDrawn = true;
				break;
			case debuggerObjTypeEffect:
				_specificEffectsDrawn = true;
				break;
			case debuggerObjTypeLight:
				_specificLightsDrawn = true;
				break;
			case debuggerObjTypeFog:
				_specificFogsDrawn = true;
				break;
			default:
				break;
			}
		}
	}
	_isDebuggerOverlay = _viewActorsToggle || _specificActorsDrawn
					 || _view3dObjectsToggle || _specific3dObjectsDrawn
					 || _viewItemsToggle || _specificItemsDrawn
					 || _viewRegionsNormalToggle || _specificRegionNormalDrawn
					 || _viewRegionsExitsToggle || _specificRegionExitsDrawn
					 || _viewScreenEffects || _specificEffectsDrawn
					 || _viewLights || _specificLightsDrawn
					 || _viewFogs || _specificFogsDrawn
					 || _viewWaypointsNormalToggle || _specificWaypointNormalDrawn
					 || _viewWaypointsFleeToggle || _specificWaypointFleeDrawn
					 || _viewWaypointsCoverToggle || _specificWaypointCoverDrawn
					 || _viewWalkboxes || _specificWalkboxesDrawn
					 || !_specificDrawnObjectsList.empty();
}

// NOTE The Flythrough (FLYTRU_E) outtake has sound only in Restored Content mode.
const struct OuttakesVQAsList {
	int resourceId;
	const char* name;
	bool notLocalized;
	int container;
	const char* description;
} outtakesList[] = {
	{  0, "INTRO",   false,  1, "Act 1 Intro - Prologue" },
	{  1, "MW_A",    false,  1, "Act 2 Intro" },
	{  2, "MW_B01",  false,  2, "Act 3 Intro - Start" },
	{  3, "MW_B02",  false,  2, "Act 3 Intro - Mid A" }, // Lucy is Replicant
	{  4, "MW_B03",  false,  2, "Act 3 Intro - Mid B" }, // Dektora is Replicant
	{  5, "MW_B04",  false,  2, "Act 3 Intro - Mid C" }, // Lucy and Dektora are Human
	{  6, "MW_B05",  false,  2, "Act 3 Intro - End" },
	{  7, "MW_C01",  false,  3, "Act 4 Intro - Start" },
	{  8, "MW_C02",  false,  3, "Act 4 Intro - End A" }, // Clovis with INCEPT PHOTO - Twins are Humans
	{  9, "MW_C03",  false,  3, "Act 4 Intro - End B" }, // Clovis without INCEPT PHOTO - Twins are Replicants
	{ 10, "MW_D",    false,  3, "Act 5 Intro" },
	{ 11, "INTRGT",  false,  1, "Interrogation scene" },
	{ 12, "END01A",  false,  4, "Underground Ending - A" }, // with Lucy (Human)
	{ 13, "END01B",  false,  4, "Underground Ending - B" }, // with Lucy (Replicant) and enough DNA data
	{ 14, "END01C",  false,  4, "Underground Ending - C" }, // with Lucy (Replicant) but insufficient DNA data
	{ 15, "END01D",  false,  4, "Underground Ending - D" }, // with Dektora (Human)
	{ 16, "END01E",  false,  4, "Underground Ending - E" }, // with Dektora (Replicant) and enough DNA data
	{ 17, "END01F",  false,  4, "Underground Ending - F" }, // with Dektora (Replicant) but insufficient DNA data
	{ 18, "END02",   false,  4, "Underground Enging - Clovis" }, // Clovis dying alone in Moonbus
	{ 19, "END03",   false,  4, "Underground Ending - McCoy" },  // McCoy alone
	{ 20, "END04A",  false,  4, "Moonbus Ending - Start" },
	{ 21, "END04B",  false,  4, "Moonbus Ending - Mid A" }, // With Lucy
	{ 22, "END04C",  false,  4, "Moonbus Ending - Mid B" }, // With Dektora
	{ 23, "END04D",  false,  4, "Moonbus Ending - End" },   // Moonbus take-off
	{ 24, "END05",   false,  4, "End 5 - Gaff's Origami" },
	{ 25, "END06",   false,  4, "Kipple Ending - 6" }, // With Steele
	{ 26, "END07",   false,  4, "Kipple Ending - 7" }, // McCoy picks up dog origami
	{ 27, "TB_FLY",  false,  2, "Flying to Tyrell Pyramid" },
	{ 28, "WSTLGO_E", true, -1, "Westwood Studios Partnership Intro"}, // STARTUP.MIX
	{ 29, "FLYTRU_E", true,  1, "Spinner Fly-Through"},
	{ 30, "AWAY01_E", true,  2, "Spinner Flying Away 01"},
	{ 31, "AWAY02_E", true,  1, "Spinner Flying Away 02"},
	{ 32, "ASCENT_E", true, -1, "Spinner Ascending"},
	{ 33, "DSCENT_E", true, -1, "Spinner Descending"},
	{ 34, "INSD01_E", true,  1, "Spinner Flying (Inside Camera) 01"},
	{ 35, "INSD02_E", true, -1, "Spinner Flying (Inside Camera) 02"},
	{ 36, "TWRD01_E", true,  1, "Spinner Flying Towards 01"},
	{ 37, "TWRD02_E", true,  1, "Spinner Flying Towards 02"},
	{ 38, "TWRD03_E", true, -1, "Spinner Flying Towards 03"},
	{ 39, "RACHEL_E", true,  2, "Rachael walks in"},
	{ 40, "DEKTRA_E", true,  2, "Dektora's (Hecuba's) dance"},
	{ 41, "BRLOGO_E", true, -1, "Blade Runner Logo"}, // STARTUP.MIX
	{ -1, nullptr,    true, -1, nullptr}
};

bool Debugger::cmdOuttake(int argc, const char** argv) {
	bool invalidSyntax = false;

	if (argc != 2) {
		invalidSyntax = true;
	} else {
		if (_vm->_kia->isOpen()
		    || _vm->_esper->isOpen()
		    || _vm->_spinner->isOpen()
		    || _vm->_elevator->isOpen()
		    || _vm->_vk->isOpen()
		    || _vm->_scores->isOpen() ) {
			debugPrintf("Sorry, playing custom outtakes in KIA, ESPER, Voigt-Kampff, Spinner GPS,\nScores or Elevator mode is not supported\n");
			return true;
		}

		if (!_vm->canSaveGameStateCurrently()) {
			debugPrintf("Sorry, playing custom outtakes while player control is disabled or an in-game script is running, is not supported\n");
			return true;
		}

		Common::String outtakeArgStr = argv[1];
		if (outtakeArgStr == "list") {
			for (int i = 0; i < (int)_vm->_gameInfo->getOuttakeCount(); ++i) {
				debugPrintf("%2d - %s\n", outtakesList[i].resourceId, outtakesList[i].description);
			}
			return true;
		} else {
			int argId = atoi(argv[1]);

			if ((argId == 0 && !isAllZeroes(outtakeArgStr))
			    || argId < 0
			    || argId >= (int)_vm->_gameInfo->getOuttakeCount()) {
				debugPrintf("Invalid outtake id specified.\nPlease choose an integer between 0 and %d.\n", (int)_vm->_gameInfo->getOuttakeCount() - 1);
				return true;
			} else {
				_dbgPendingOuttake.container = outtakesList[argId].container;
				if (argId == 35 || argId == 38) {
					// These outtakes exist in containers: OUTTAKE1 and OUTTAKE2
					if (_vm->_chapters->currentResourceId() != 1
					    && _vm->_chapters->currentResourceId() != 2) {
						_dbgPendingOuttake.container = (int)_vm->_rnd.getRandomNumberRng(1, 2);
					}
				} else if (argId == 32 || argId == 33) {
					// These outtakes exist in containers: OUTTAKE1, OUTTAKE3, OUTTAKE4
					if (_vm->_chapters->currentResourceId() != 1
					    && _vm->_chapters->currentResourceId() != 3
					    && _vm->_chapters->currentResourceId() != 4) {
						_dbgPendingOuttake.container = (int)_vm->_rnd.getRandomNumberRng(2, 4);
						if (_dbgPendingOuttake.container == 2)
							_dbgPendingOuttake.container = 1;
					}
				}
				// We need to close the debugger console first before playing back the outtake.
				// The following prepares the outtake video for playback within BladeRunnerEngine::gameTick()
				_dbgPendingOuttake.pending = true;
				_dbgPendingOuttake.outtakeId = outtakesList[argId].resourceId;
				_dbgPendingOuttake.notLocalized = outtakesList[argId].notLocalized;
			}
		}
	}

	if (invalidSyntax) {
		debugPrintf("Play an outtake video.\n");
		debugPrintf("Usage: %s [<outtakeId> | list]\n", argv[0]);
		debugPrintf("outtakeId can be in [0, %d]\n", (int)_vm->_gameInfo->getOuttakeCount() - 1);
		return true;
	}
	// close debugger (to play the outtake)
	return false;
}

void Debugger::resetPendingOuttake() {
	_dbgPendingOuttake.pending = false;
	_dbgPendingOuttake.outtakeId = -1;
	_dbgPendingOuttake.notLocalized = false;
	_dbgPendingOuttake.container = -1;
	_dbgPendingOuttake.externalFilename.clear();
}

bool Debugger::cmdPlayVqa(int argc, const char** argv) {
	if (argc != 2) {
		debugPrintf("Loads a VQA file to play.\n");
		debugPrintf("Usage: %s <file path>\n", argv[0]);
		return true;
	}

	if (_vm->_kia->isOpen()
	    || _vm->_esper->isOpen()
	    || _vm->_spinner->isOpen()
	    || _vm->_elevator->isOpen()
	    || _vm->_vk->isOpen()
	    || _vm->_scores->isOpen() ) {
		debugPrintf("Sorry, playing custom outtakes in KIA, ESPER, Voigt-Kampff, Spinner GPS,\nScores or Elevator mode is not supported\n");
		return true;
	}

	if (!_vm->canSaveGameStateCurrently()) {
		debugPrintf("Sorry, playing custom outtakes while player control is disabled or an in-game script is running, is not supported\n");
		return true;
	}

	Common::String filenameArg = argv[1];
	Common::String basename = filenameArg;

	// Strip the base name of the file of any extension given
	// to check for existence of basename.VQP and basename.VQA files
	size_t startOfExt = basename.findLastOf('.');
	if (startOfExt != Common::String::npos && (basename.size() - startOfExt - 1) == 3) {
		basename.erase(startOfExt);
	}

	Common::Path basenameVQA(Common::String::format("%s.VQA", basename.c_str()));
	Common::Path basenameVQP(Common::String::format("%s.VQP", basename.c_str()));

	// Check for existence of VQP
	bool vqpFileExists = false;

	// Use Common::File exists() check instead of Common::FSNode directly
	// to allow the file to be placed within SearchMan accessible locations
	if (!Common::File::exists(basenameVQP)) {
		debugPrintf("Warning: VQP file %s does not exist\n", basenameVQP.toString(Common::Path::kNativeSeparator).c_str());
	} else {
		vqpFileExists = true;
	}

	if (!Common::File::exists(basenameVQA)) {
		debugPrintf("Warning: VQA file %s does not exist\n", basenameVQA.toString(Common::Path::kNativeSeparator).c_str());
		return true;
	}

	_dbgPendingOuttake.pending = true;
	_dbgPendingOuttake.outtakeId = -1;
	if (vqpFileExists) {
		_dbgPendingOuttake.container = -2; // indicates that an external outtake file with possible VQP companion should be read
	} else {
		_dbgPendingOuttake.container = -3; // indicates that an external outtake file but no VQP companion was found so don't check again
	}
	_dbgPendingOuttake.notLocalized = true;
	_dbgPendingOuttake.externalFilename = basename; // external base filename

	// close debugger (to play the outtake)
	return false;
}

/**
* Auxiliary function to get a descriptive string for a given ammo type
*/
Common::String Debugger::getAmmoTypeDescription(int ammoType) {
	Common::String ammoTypeStr;
	switch (ammoType) {
	default:
		// fall through
	case 0:
		ammoTypeStr = Common::String::format("Plain (%d)", 0);
		break;
	case 1:
		ammoTypeStr = Common::String::format("Bob's bullets (%d)", 1);
		break;
	case 2:
		ammoTypeStr = Common::String::format("Izo's stash (%d)", 2);
		break;
	}
	return ammoTypeStr;

}

/**
* Show or add to McCoy's ammo for an ammo type
* Note: We add and not set, as adding is directly supported by the scripts,
* whereas setting the value explicitly is not and could cause side-effects (eg. ammoType 0 should never get 0 ammo).
*/
bool Debugger::cmdAmmo(int argc, const char** argv) {
	bool invalidSyntax = false;

	if (_vm->_settings->getDifficulty() == kGameDifficultyEasy) {
		debugPrintf("---\nNote: Currently playing in Easy Mode.\nAll ammo is infinite, regardless of the amount shown or added here\n---\n");
	}

	if (argc == 1) {
		for (int i = 0; i < _vm->_settings->getAmmoTypesCount(); ++i) {
			if (i == 0 || _vm->_settings->getDifficulty() == kGameDifficultyEasy) {
				debugPrintf("Current ammo for ammo type: %s is infinite (%d)\n", getAmmoTypeDescription(i).c_str(), _vm->_settings->getAmmo(i));
			} else {
				debugPrintf("Current ammo for ammo type: %s is: %d\n", getAmmoTypeDescription(i).c_str(), _vm->_settings->getAmmo(i));
			}
		}
		return true;

	} else if (argc == 2 || argc == 3) {
		if (Common::isDigit(*argv[1])) {
			int argAmmoType = atoi(argv[1]);
			if (argAmmoType >= 0 && argAmmoType < _vm->_settings->getAmmoTypesCount()) {
				if (argc == 2) {
					if (argAmmoType == 0 || _vm->_settings->getDifficulty() == kGameDifficultyEasy) {
						debugPrintf("Current ammo for ammo type: %s is infinite (%d)\n", getAmmoTypeDescription(argAmmoType).c_str(), _vm->_settings->getAmmo(argAmmoType));
					} else {
						debugPrintf("Current ammo for ammo type: %s is: %d\n", getAmmoTypeDescription(argAmmoType).c_str(), _vm->_settings->getAmmo(argAmmoType));
					}
					return true;

				} else { // argc == 3
					if (Common::isDigit(*argv[2])) {
						int argAmmoAmmount = atoi(argv[2]);
						if (argAmmoAmmount >= 0) {
							if (argAmmoType == 0) {
								debugPrintf("Current ammo for ammo type: %s is infinite (%d)\n", getAmmoTypeDescription(argAmmoType).c_str(), _vm->_settings->getAmmo(argAmmoType));
							} else {
								if (_vm->_kia->isOpen()) {
									debugPrintf("Sorry, modifying ammo when KIA is open is not supported\n");
								} else {
									_vm->_settings->addAmmo(argAmmoType, argAmmoAmmount);
									if ( _vm->_settings->getDifficulty() == kGameDifficultyEasy) {
										debugPrintf("Current ammo for ammo type: %s is infinite (%d)\n", getAmmoTypeDescription(argAmmoType).c_str(), _vm->_settings->getAmmo(argAmmoType));
									} else {
										debugPrintf("Current ammo for ammo type: %s set to: %d\n", getAmmoTypeDescription(argAmmoType).c_str(), _vm->_settings->getAmmo(argAmmoType));
									}
								}
							}
							return true;

						} else {
							debugPrintf("Error - Please specify and valid ammo amount to add\n");
							return true;

						}
					} else {
						invalidSyntax = true;
					}
				}
			} else {
				debugPrintf("Invalid ammo type specified. Valid values are 0 - %d\n", _vm->_settings->getAmmoTypesCount() - 1);
				return true;

			}
		} else {
			invalidSyntax = true;
		}
	} else {
		invalidSyntax = true;
	}

	if (invalidSyntax) {
		// invalid  syntax
		debugPrintf("Show or add to McCoy's ammo amount for an ammo type\n");
		debugPrintf("Valid ammo types: \n");
		for (int i = 0; i < _vm->_settings->getAmmoTypesCount(); ++i) {
			debugPrintf("%d: %s\n", i, getAmmoTypeDescription(i).c_str());
		}
		debugPrintf("Usage 1: %s\n", argv[0]);
		debugPrintf("Usage 2: %s <ammoType>\n", argv[0]);
		debugPrintf("Usage 3: %s <ammo type> <ammo amount to add>\n", argv[0]);
	}
	return true;
}

bool Debugger::cmdCheatReport(int argc, const char** argv) {
	bool invalidSyntax = false;
	if (argc == 1) {
		debugPrintf("---\nNote: Currently playing in %s mode.\n---\n", getDifficultyDescription(_vm->_settings->getDifficulty()).c_str());
		debugPrintf("Izo is ");
		if ( _vm->_gameFlags->query(kFlagIzoIsReplicant)) {
			debugPrintf("a Replicant\n");
		} else {
			debugPrintf("Human\n");
		}

		debugPrintf("Gordo is ");
		if ( _vm->_gameFlags->query(kFlagGordoIsReplicant)) {
			debugPrintf("a Replicant\n");
		} else {
			debugPrintf("Human\n");
		}

		debugPrintf("Lucy is ");
		if ( _vm->_gameFlags->query(kFlagLucyIsReplicant)) {
			debugPrintf("a Replicant\n");
		} else {
			debugPrintf("Human\n");
		}

		debugPrintf("Dektora is ");
		if ( _vm->_gameFlags->query(kFlagDektoraIsReplicant)) {
			debugPrintf("a Replicant\n");
		} else {
			debugPrintf("Human\n");
		}

		debugPrintf("Sadik is ");
		if ( _vm->_gameFlags->query(kFlagSadikIsReplicant)) {
			debugPrintf("a Replicant\n");
		} else {
			debugPrintf("Human\n");
		}

		debugPrintf("Luther/Lance is ");
		if ( _vm->_gameFlags->query(kFlagLutherLanceIsReplicant)) {
			debugPrintf("Replicant\n");
		} else {
			debugPrintf("Human\n");
		}
		debugPrintf("---\n");
		debugPrintf("Random seed is ");
		debugPrintf("%u\n", _vm->_newGameRandomSeed);
		debugPrintf("---\n");
		return true;

	} else {
		invalidSyntax = true;
	}

	if (invalidSyntax) {
		// invalid  syntax
		debugPrintf("Show the random seed for the game and the nature (human or Replicant) for the potentially variable characters\n");
		if (argv != nullptr && argv[0] != nullptr) {
			debugPrintf("Usage: %s\n", argv[0]);
		}
	}

	return true;
}

} // End of namespace BladeRunner
