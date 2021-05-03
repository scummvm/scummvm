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

#include "twine/debugger/console.h"
#include "common/scummsys.h"
#include "common/util.h"
#include "twine/debugger/debug_grid.h"
#include "twine/debugger/debug_scene.h"
#include "twine/holomap.h"
#include "twine/renderer/redraw.h"
#include "twine/resources/hqr.h"
#include "twine/scene/gamestate.h"
#include "twine/scene/scene.h"
#include "twine/text.h"
#include "twine/twine.h"

namespace TwinE {

TwinEConsole::TwinEConsole(TwinEEngine *engine) : _engine(engine), GUI::Debugger() {
	registerCmd("give_item", WRAP_METHOD(TwinEConsole, doGiveItem));
	registerCmd("give_allitems", WRAP_METHOD(TwinEConsole, doGiveAllItems));
	registerCmd("give_key", WRAP_METHOD(TwinEConsole, doGiveKey));
	registerCmd("give_gas", WRAP_METHOD(TwinEConsole, doGiveGas));
	registerCmd("give_kashes", WRAP_METHOD(TwinEConsole, doGiveKashes));
	registerCmd("play_video", WRAP_METHOD(TwinEConsole, doPlayVideo));
	registerCmd("change_scene", WRAP_METHOD(TwinEConsole, doChangeScene));
	registerCmd("toggle_scenery_view", WRAP_METHOD(TwinEConsole, doToggleSceneryView));
	registerCmd("magic_points", WRAP_METHOD(TwinEConsole, doAddMagicPoints));
	registerCmd("dumpfile", WRAP_METHOD(TwinEConsole, doDumpFile));
	registerCmd("list_menutext", WRAP_METHOD(TwinEConsole, doListMenuText));
	registerCmd("toggle_debug", WRAP_METHOD(TwinEConsole, doToggleDebug));
	registerCmd("toggle_zones", WRAP_METHOD(TwinEConsole, doToggleZoneRendering));
	registerCmd("toggle_tracks", WRAP_METHOD(TwinEConsole, doToggleTrackRendering));
	registerCmd("toggle_autoagressive", WRAP_METHOD(TwinEConsole, doToggleAutoAggressive));
	registerCmd("toggle_actors", WRAP_METHOD(TwinEConsole, doToggleActorRendering));
	registerCmd("toggle_clips", WRAP_METHOD(TwinEConsole, doToggleClipRendering));
	registerCmd("toggle_freecamera", WRAP_METHOD(TwinEConsole, doToggleFreeCamera));
	registerCmd("toggle_scenechanges", WRAP_METHOD(TwinEConsole, doToggleSceneChanges));
	registerCmd("scene_actor", WRAP_METHOD(TwinEConsole, doSkipSceneActorsBut));
	registerCmd("hero_pos", WRAP_METHOD(TwinEConsole, doSetHeroPosition));
	registerCmd("set_life", WRAP_METHOD(TwinEConsole, doSetLife));
	registerCmd("set_game_flag", WRAP_METHOD(TwinEConsole, doSetGameFlag));
	registerCmd("show_game_flag", WRAP_METHOD(TwinEConsole, doPrintGameFlag));
	registerCmd("set_inventory_flag", WRAP_METHOD(TwinEConsole, doSetInventoryFlag));
	registerCmd("show_inventory_flag", WRAP_METHOD(TwinEConsole, doPrintGameFlag));
	registerCmd("set_holomap_flag", WRAP_METHOD(TwinEConsole, doSetHolomapFlag));
	registerCmd("set_holomap_trajectory", WRAP_METHOD(TwinEConsole, doSetHolomapTrajectory));
	registerCmd("show_holomap_flag", WRAP_METHOD(TwinEConsole, doPrintGameFlag));
	registerCmd("toggle_scene_patches", WRAP_METHOD(TwinEConsole, doToggleScenePatches));
}

TwinEConsole::~TwinEConsole() {
}

#define TOGGLE_DEBUG(var, description)         \
	if ((var)) {                               \
		debugPrintf("Disabling " description); \
		(var) = false;                         \
	} else {                                   \
		debugPrintf("Enabling " description);  \
		(var) = true;                          \
	}                                          \
	if ((var) && !_engine->cfgfile.Debug) {    \
		doToggleDebug(0, nullptr);             \
	}

bool TwinEConsole::doToggleZoneRendering(int argc, const char **argv) {
	TOGGLE_DEBUG(_engine->_debugScene->showingZones, "zone rendering\n")
	return true;
}

bool TwinEConsole::doToggleActorRendering(int argc, const char **argv) {
	TOGGLE_DEBUG(_engine->_debugScene->showingActors, "actor rendering\n")
	return true;
}

bool TwinEConsole::doToggleTrackRendering(int argc, const char **argv) {
	TOGGLE_DEBUG(_engine->_debugScene->showingTracks, "tracks rendering\n")
	return true;
}

bool TwinEConsole::doToggleScenePatches(int argc, const char **argv) {
	TOGGLE_DEBUG(_engine->_debugScene->useScenePatches, "use scene patches\n")
	return true;
}

bool TwinEConsole::doToggleClipRendering(int argc, const char **argv) {
	TOGGLE_DEBUG(_engine->_debugScene->showingClips, "clip rendering\n")
	return true;
}

bool TwinEConsole::doToggleSceneryView(int argc, const char **argv) {
	TOGGLE_DEBUG(_engine->_redraw->inSceneryView, "scenery view\n")
	return true;
}

bool TwinEConsole::doToggleAutoAggressive(int argc, const char **argv) {
	TOGGLE_DEBUG(_engine->_actor->autoAggressive, "auto aggressive\n")
	return true;
}

bool TwinEConsole::doAddMagicPoints(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: specify the magic points\n");
		return true;
	}
	const int16 magicPoints = atoi(argv[1]);
	_engine->_gameState->magicLevelIdx = CLIP<int16>(magicPoints, 0, 4);
	_engine->_gameState->setMaxMagicPoints();
	return true;
}

bool TwinEConsole::doSkipSceneActorsBut(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: give actor id of scene or -1 to disable\n");
		return true;
	}
	const int16 actorIdx = atoi(argv[1]);
	debugPrintf("Only load actor %d in the next scene\n", actorIdx);
	_engine->_debugScene->onlyLoadActor = actorIdx;
	return true;
}

bool TwinEConsole::doToggleFreeCamera(int argc, const char **argv) {
	TOGGLE_DEBUG(_engine->_debugGrid->useFreeCamera, "free camera movement\n")
	return true;
}

bool TwinEConsole::doToggleSceneChanges(int argc, const char **argv) {
	TOGGLE_DEBUG(_engine->_debugGrid->canChangeScenes, "scene switching via keybinding\n")
	return true;
}

bool TwinEConsole::doSetInventoryFlag(int argc, const char **argv) {
	if (argc <= 1) {
		debugPrintf("Expected to get a inventory flag index as first parameter\n");
		return true;
	}

	const uint8 idx = atoi(argv[1]);
	if (idx >= NUM_INVENTORY_ITEMS) {
		debugPrintf("given index exceeds the max allowed value of %i\n", NUM_INVENTORY_ITEMS - 1);
		return true;
	}
	const uint8 val = argc == 3 ? atoi(argv[2]) : 0;
	_engine->_gameState->inventoryFlags[idx] = val;

	return true;
}

bool TwinEConsole::doSetHolomapTrajectory(int argc, const char **argv) {
	if (argc <= 1) {
		debugPrintf("Expected to get a holomap trajectory index as parameter\n");
		return true;
	}
	_engine->_scene->holomapTrajectory = atoi(argv[1]);
	_engine->_scene->reloadCurrentScene();
	return false;
}

bool TwinEConsole::doSetHolomapFlag(int argc, const char **argv) {
	if (argc <= 1) {
		debugPrintf("Expected to get a holomap flag index as first parameter. Use -1 to set all flags\n");
		return true;
	}

	GameState* state = _engine->_gameState;
	state->setGameFlag(InventoryItems::kiHolomap, 1);
	state->inventoryFlags[InventoryItems::kiHolomap] = 1;
	state->setGameFlag(GAMEFLAG_INVENTORY_DISABLED, 0);

	const int idx = atoi(argv[1]);
	if (idx == -1) {
		for (int i = 0; i < NUM_LOCATIONS; ++i) {
			_engine->_holomap->setHolomapPosition(i);
		}
		return true;
	}
	if (idx >= 0 && idx >= NUM_LOCATIONS) {
		debugPrintf("given index exceeds the max allowed value of %i\n", NUM_LOCATIONS - 1);
		return true;
	}
	_engine->_holomap->setHolomapPosition(idx);
	return true;
}

bool TwinEConsole::doSetGameFlag(int argc, const char **argv) {
	if (argc <= 1) {
		debugPrintf("Expected to get a game flag index as first parameter\n");
		return true;
	}

	const uint8 idx = atoi(argv[1]);
	const uint8 val = argc == 3 ? atoi(argv[2]) : 0;
	_engine->_gameState->setGameFlag(idx, val);

	return true;
}

bool TwinEConsole::doPrintGameFlag(int argc, const char **argv) {
	if (argc <= 1) {
		for (int i = 0; i < NUM_GAME_FLAGS; ++i) {
			debugPrintf("[%03d] = %d\n", i, _engine->_gameState->hasGameFlag(i));
		}
		return true;
	}

	const uint8 idx = atoi(argv[1]);
	debugPrintf("[%03d] = %d\n", idx, _engine->_gameState->hasGameFlag(idx));

	return true;
}

bool TwinEConsole::doPrintInventoryFlag(int argc, const char **argv) {
	if (argc <= 1) {
		for (int i = 0; i < NUM_INVENTORY_ITEMS; ++i) {
			debugPrintf("[%03d] = %d\n", i, _engine->_gameState->inventoryFlags[i]);
		}
		return true;
	}

	const uint8 idx = atoi(argv[1]);
	if (idx < NUM_INVENTORY_ITEMS) {
		debugPrintf("[%03d] = %d\n", idx, _engine->_gameState->inventoryFlags[idx]);
	}

	return true;
}

bool TwinEConsole::doPrintHolomapFlag(int argc, const char **argv) {
	if (argc <= 1) {
		for (int i = 0; i < NUM_LOCATIONS; ++i) {
			debugPrintf("[%03d] = %d\n", i, _engine->_gameState->holomapFlags[i]);
		}
		return true;
	}

	const uint8 idx = atoi(argv[1]);
	if (idx < NUM_LOCATIONS) {
		debugPrintf("[%03d] = %d\n", idx, _engine->_gameState->holomapFlags[idx]);
	}

	return true;
}

bool TwinEConsole::doGiveKey(int argc, const char **argv) {
	int amount = 1;
	if (argc >= 2) {
		amount = atoi(argv[1]);
	}
	_engine->_gameState->addKeys(amount);
	return true;
}

bool TwinEConsole::doGiveGas(int argc, const char **argv) {
	int amount = 1;
	if (argc >= 2) {
		amount = atoi(argv[1]);
	}
	_engine->_gameState->addGas(amount);
	return true;
}

bool TwinEConsole::doGiveKashes(int argc, const char **argv) {
	int amount = 1;
	if (argc >= 2) {
		amount = atoi(argv[1]);
	}
	_engine->_gameState->addKashes(amount);
	return true;
}

bool TwinEConsole::doToggleDebug(int argc, const char **argv) {
	if (_engine->cfgfile.Debug) {
		debugPrintf("Disabling debug mode\n");
		_engine->cfgfile.Debug = false;
	} else {
		debugPrintf("Enabling debug mode\n");
		_engine->cfgfile.Debug = true;
	}
	return true;
}

bool TwinEConsole::doListMenuText(int argc, const char **argv) {
	TextBankId textBankId = TextBankId::Inventory_Intro_and_Holomap;
	if (argc >= 2) {
		textBankId = (TextBankId)atoi(argv[1]);
	}
	const TextBankId oldTextBankId = _engine->_text->textBank();
	_engine->_text->initTextBank(textBankId);
	for (int32 i = 0; i < 1000; ++i) {
		char buf[256];
		if (_engine->_text->getMenuText((TextId)i, buf, sizeof(buf))) {
			debugPrintf("%4i: %s\n", i, buf);
		}
	}
	_engine->_text->initTextBank(oldTextBankId);
	return true;
}

bool TwinEConsole::doSetHeroPosition(int argc, const char **argv) {
	IVec3 &pos = _engine->_scene->sceneHero->pos;
	if (argc < 4) {
		debugPrintf("Current hero position: %i:%i:%i\n", pos.x, pos.y, pos.z);
		return true;
	}
	pos.x = atoi(argv[1]);
	pos.y = atoi(argv[2]);
	pos.z = atoi(argv[3]);
	return true;
}

bool TwinEConsole::doPlayVideo(int argc, const char **argv) {
	if (argc <= 1) {
		debugPrintf("Expected to get a video filename as first parameter\n");
		return true;
	}
	_engine->queueMovie(argv[1]);
	return true;
}

bool TwinEConsole::doChangeScene(int argc, const char **argv) {
	if (argc <= 1) {
		debugPrintf("Expected to get a scene index as first parameter\n");
		return true;
	}
	byte newSceneIndex = atoi(argv[1]);
	if (newSceneIndex >= LBA1SceneId::SceneIdMax) {
		debugPrintf("Scene index out of bounds\n");
		return true;
	}
	_engine->_scene->needChangeScene = atoi(argv[1]);
	_engine->_scene->changeScene();
	return true;
}

bool TwinEConsole::doDumpFile(int argc, const char **argv) {
	if (argc <= 2) {
		debugPrintf("Expected to get a a hqr file and an index\n");
		return true;
	}
	const char *hqr = argv[1];
	const int index = atoi(argv[2]);
	const Common::String &targetFileName = Common::String::format("dumps/%03i-%s.dump", index, hqr);
	HQR::dumpEntry(hqr, index, targetFileName.c_str());
	return true;
}

static const char *ItemNames[] = {
	"Holomap",
	"MagicBall",
	"UseSabre",
	"GawleysHorn",
	"Tunic",
	"BookOfBu",
	"SendellsMedallion",
	"FlaskOfClearWater",
	"RedCard",
	"BlueCard",
	"IDCard",
	"MrMiesPass",
	"ProtoPack",
	"Snowboard",
	"Pinguin",
	"GasItem",
	"PirateFlag",
	"MagicFlute",
	"SpaceGuitar",
	"HairDryer",
	"AncesteralKey",
	"BottleOfSyrup",
	"EmptyBottle",
	"FerryTicket",
	"Keypad",
	"CoffeeCan",
	"BonusList",
	"CloverLeaf"
};
static_assert(ARRAYSIZE(ItemNames) == InventoryItems::MaxInventoryItems, "Array size doesn't match items");

bool TwinEConsole::doGiveItem(int argc, const char **argv) {
	if (argc <= 1) {
		debugPrintf("Expected to get an item as first parameter\n");
		for (int i = 0; i < ARRAYSIZE(ItemNames); ++i) {
			debugPrintf(" - %2i: %s\n", i, ItemNames[i]);
		}
		return true;
	}
	byte itemIdx = atoi(argv[1]);
	if (itemIdx >= InventoryItems::MaxInventoryItems) {
		debugPrintf("Item index out of bounds\n");
		return true;
	}
	GameState* state = _engine->_gameState;
	state->setGameFlag(itemIdx, 1);
	state->inventoryFlags[itemIdx] = 1;
	state->setGameFlag(GAMEFLAG_INVENTORY_DISABLED, 0);

	return true;
}

bool TwinEConsole::doGiveAllItems(int argc, const char **argv) {
	GameState* state = _engine->_gameState;
	for (int32 i = 0; i < NUM_INVENTORY_ITEMS; ++i) {
		state->setGameFlag(i, 1);
		state->inventoryFlags[i] = 1;
	}
	state->setGameFlag(GAMEFLAG_INVENTORY_DISABLED, 0);
	int amount = 1;
	if (argc >= 2) {
		amount = atoi(argv[1]);
	}
	state->addKeys(amount);
	state->addLeafBoxes(amount);
	state->addKashes(amount);
	state->addLeafs(amount);
	state->addMagicPoints(amount);
	state->addGas(amount);

	return true;
}

bool TwinEConsole::doSetLife(int argc, const char **argv) {
	if (argc <= 1) {
		debugPrintf("Expected to get the life points as parameter\n");
		return true;
	}
	_engine->_scene->sceneHero->setLife(atoi(argv[1]));
	return true;
}

} // End of namespace TwinE
