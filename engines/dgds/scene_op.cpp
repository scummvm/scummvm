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

#include "graphics/cursorman.h"

#include "dgds/scene_op.h"
#include "dgds/dgds.h"
#include "dgds/debug_util.h"
#include "dgds/globals.h"
#include "dgds/inventory.h"
#include "dgds/scene.h"
#include "dgds/dragon_native.h"
#include "dgds/hoc_intro.h"
#include "dgds/minigames/dragon_arcade.h"
#include "dgds/minigames/china_train.h"
#include "dgds/minigames/china_tank.h"

namespace Dgds {

static Common::String _sceneOpCodeName(SceneOpCode code) {
	code = static_cast<SceneOpCode>(code & ~kSceneOpHasConditionalOpsFlag);
	switch (code) {
	case kSceneOpNone: 		  	return "none";
	case kSceneOpChangeScene: 	return "changeScene";
	case kSceneOpNoop:		  	return "noop";
	case kSceneOpGlobal:		return "global";
	case kSceneOpSegmentStateOps: return "sceneOpSegmentStateOps";
	case kSceneOpSetItemAttr:   return "setItemAttr";
	case kSceneOpSetDragItem:   return "setDragItem";
	case kSceneOpOpenInventory: return "openInventory";
	case kSceneOpShowDlg:		return "showdlg";
	case kSceneOpShowInvButton:	return "showInvButton";
	case kSceneOpHideInvButton:	return "hideInvButton";
	case kSceneOpEnableTrigger: return "enabletrigger";
	case kSceneOpChangeSceneToStored: 	return "changeSceneToStored";
	case kSceneOpAddFlagToDragItem:		return "addFlagToDragItem";
	case kSceneOpMoveItemsBetweenScenes: return "moveItemsBetweenScenes";
	case kSceneOpOpenInventoryZoom:   	return "openInventoryZoom";
	case kSceneOpShowClock:		return "showClock";
	case kSceneOpHideClock:		return "hideClock";
	case kSceneOpShowMouse:		return "showMouse";
	case kSceneOpHideMouse:		return "hideMouse";
	case kSceneOpLoadTalkDataAndSetFlags: return "loadTalkDataAndSetFlags";
	case kSceneOpDrawVisibleTalkHeads: return "drawVisibleTalksHeads";
	case kSceneOpLoadTalkData: 	return "loadTalkData";
	case kSceneOpLoadDDSData: 	return "loadDDSData";
	case kSceneOpFreeDDSData: 	return "freeDDSData";
	case kSceneOpFreeTalkData: 	return "freeTalkData";

	default:
		break;
	}

	if (DgdsEngine::getInstance()->getGameId() == GID_DRAGON) {
		switch (code) {
		case kSceneOpPasscode:		return "passcode";
		case kSceneOpMeanwhile:   	return "meanwhile";
		case kSceneOpOpenGameOverMenu: return "openGameOverMenu";
		case kSceneOpTiredDialog:	return "openTiredDialog";
		case kSceneOpArcadeTick: 	return "sceneOpArcadeTick";
		case kSceneOpDrawDragonCountdown1: 	return "drawDragonCountdown1";
		case kSceneOpDrawDragonCountdown2:	return "drawDragonCountdown2";
		case kSceneOpOpenPlaySkipIntroMenu: return "openPlaySkipIntroMovie";
		case kSceneOpOpenBetterSaveGameMenu: return "openBetterSaveGameMenu";
		default:
			break;
		}
	} else if (DgdsEngine::getInstance()->getGameId() == GID_HOC) {
		switch (code) {
		case kSceneOpChinaTankInit:			return "tankInit";
		case kSceneOpChinaTankEnd:			return "tankEnd";
		case kSceneOpChinaTankTick:			return "tankTick";
		case kSceneOpChinaScrollLeft:		return "scrollLeft";
		case kSceneOpChinaScrollRight:		return "scrollRight";
		case kSceneOpShellGameInit:			return "shellGameInit";
		case kSceneOpShellGameEnd:			return "shellGameEnd";
		case kSceneOpShellGameTick:			return "shellGameTick";
		case kSceneOpChinaTrainInit:		return "trainInit";
		case kSceneOpChinaTrainEnd:			return "trainEnd";
		case kSceneOpChinaTrainTick:		return "trainTick";
		case kSceneOpChinaOpenGameOverMenu: return "gameOverMenu";
		case kSceneOpChinaOpenSkipCreditsMenu: return "skipCreditsMenu";
		case kSceneOpChinaOnIntroInit:		return "chinaOnIntroInit";
		case kSceneOpChinaOnIntroTick:		return "chinaOnIntroTick";
		case kSceneOpChinaOnIntroEnd:  		return "chinaOnIntroEnd";
		default:
			break;
		}
	} else if (DgdsEngine::getInstance()->getGameId() == GID_WILLY) {
		switch (code) {
		case kSceneOpOpenBeamishGameOverMenu: return "openGameOverMenu";
		case kSceneOpOpenBeamishOpenSkipCreditsMenu: return "skipCreditsMenu";
		default:
			break;
		}
	}

	return Common::String::format("sceneOp%d", (int)code);
}


Common::String SceneOp::dump(const Common::String &indent) const {
	Common::String argsStr;
	if (_args.empty()) {
		argsStr = "[]";
	} else {
		argsStr = "[";
		for  (uint i : _args)
			argsStr += Common::String::format("%d ", i);
		argsStr.setChar(']', argsStr.size() - 1);
	}

	const Common::String opName = _sceneOpCodeName(_opCode);
	const char *isConditional = (_opCode & kSceneOpHasConditionalOpsFlag) ? "(cond)": "";
	Common::String str = Common::String::format("%sSceneOp<op: %s%s args: %s", indent.c_str(), opName.c_str(), isConditional, argsStr.c_str());

	str += DebugUtil::dumpStructList(indent, "conditionList", _conditionList);
	if (!_conditionList.empty()) {
		str += "\n";
		str += indent;
	}
	str += ">";
	return str;
}

bool SceneOp::runOp() const {
	bool sceneChanged;

	if (_opCode < 100) {
		sceneChanged = runCommonOp();
	} else {
		// Game-specific opcode
		switch (DgdsEngine::getInstance()->getGameId()) {
		case GID_DRAGON:
			sceneChanged = runDragonOp();
			break;
		case GID_HOC:
			sceneChanged = runChinaOp();
			break;
		case GID_WILLY:
			sceneChanged = runBeamishOp();
			break;
		default:
			error("TODO: Implement game-specific scene op for this game");
		}
	}
	return sceneChanged;
}

bool SceneOp::runCommonOp() const {
	DgdsEngine *engine = DgdsEngine::getInstance();
	switch (_opCode) {
	case kSceneOpChangeScene:
		if (engine->changeScene(_args[0]))
			return true;
		break;
	case kSceneOpNoop:
		break;
	case kSceneOpGlobal:
		// The globals are held by the GDS scene
		engine->getGDSScene()->globalOps(_args);
		break;
	case kSceneOpSegmentStateOps:
		SDSScene::segmentStateOps(_args);
		break;
	case kSceneOpSetItemAttr:
		SDSScene::setItemAttrOp(_args);
		break;
	case kSceneOpSetDragItem:
		SDSScene::setDragItemOp(_args);
		break;
	case kSceneOpOpenInventory:
		engine->getInventory()->open();
		// This implicitly changes scene num
		break;
	case kSceneOpShowDlg:
		if (_args.size() == 1)
			engine->getScene()->showDialog(0, _args[0]);
		else if (_args.size() > 1)
			engine->getScene()->showDialog(_args[0], _args[1]);
		break;
	case kSceneOpShowInvButton:
		engine->getScene()->addInvButtonToHotAreaList();
		break;
	case kSceneOpHideInvButton:
		engine->getScene()->removeInvButtonFromHotAreaList();
		break;
	case kSceneOpEnableTrigger:
		if (_args.size() == 1)
			engine->getScene()->enableTrigger(0, _args[0]);
		else if (_args.size() > 1)
			engine->getScene()->enableTrigger(_args[1], _args[0]);
		break;
	case kSceneOpChangeSceneToStored: {
		int16 sceneNo = engine->getGameGlobals()->getGlobal(0x61);
		if (engine->changeScene(sceneNo))
			return true;
		break;
	}
	case kSceneOpAddFlagToDragItem: {
		GameItem *item = engine->getScene()->getDragItem();
		if (item) {
			item->_flags |= 1;
			// TODO: Use hot x/y or just position?
			Common::Point lastMouse = engine->getLastMouseMinusHot();
			item->_rect.x = lastMouse.x;
			item->_rect.y = lastMouse.y;
		}
		break;
	}
	case kSceneOpOpenInventoryZoom:
		engine->getInventory()->setShowZoomBox(true);
		engine->getInventory()->open();
		return true;
	case kSceneOpMoveItemsBetweenScenes: {
		int16 fromScene = engine->getGameGlobals()->getGlobal(0x55);
		int16 toScene = engine->getGameGlobals()->getGlobal(0x54);
		for (auto &item : engine->getGDSScene()->getGameItems()) {
			if (item._inSceneNum == fromScene)
				item._inSceneNum = toScene;
		}
		break;
	}
	case kSceneOpShowClock:
		engine->setShowClock(true);
		break;
	case kSceneOpHideClock:
		engine->setShowClock(false);
		break;
	case kSceneOpShowMouse:
		CursorMan.showMouse(true);
		break;
	case kSceneOpHideMouse:
		CursorMan.showMouse(false);
		break;
	case kSceneOpLoadTalkDataAndSetFlags: // args: tdsnum to load, headnum
		engine->getScene()->loadTalkDataAndSetFlags(_args[0], _args[1]);
		break;
	case kSceneOpDrawVisibleTalkHeads: // args: none
		engine->getScene()->updateVisibleTalkers();
		break;
	case kSceneOpLoadTalkData: 	// args: tds num to load
		engine->getScene()->loadTalkData(_args[0]);
		break;
	case kSceneOpLoadDDSData: 	// args: dds num to load
		if (_args[0])
			engine->getScene()->loadDialogData(_args[0]);
		break;
	case kSceneOpFreeDDSData:	// args: dds num to free
		engine->getScene()->freeDialogData(_args[0]);
		break;
	case kSceneOpFreeTalkData: 	// args: tds num to free
		engine->getScene()->freeTalkData(_args[0]);
		break;

	default:
		warning("TODO: Implement generic scene op %d", _opCode);
		break;
	}
	return false;
}

bool SceneOp::runDragonOp() const {
	DgdsEngine *engine = DgdsEngine::getInstance();
	switch (_opCode) {
	case kSceneOpPasscode:
		DragonNative::updatePasscodeGlobal();
		break;
	case kSceneOpMeanwhile:
		// TODO: Should we draw "meanwhile" like the original? it just gets overwritten with the image anyway.
		// Probably need to do something here to avoid flashing..
		//engine->_compositionBuffer.fillRect(Common::Rect(SCREEN_WIDTH, SCREEN_HEIGHT), 0);
		break;
	case kSceneOpOpenGameOverMenu:
		engine->setMenuToTrigger(kMenuGameOver);
		break;
	case kSceneOpTiredDialog:
		engine->getInventory()->close();
		engine->getScene()->addAndShowTiredDialog();
		break;
	case kSceneOpArcadeTick:
		// TODO: Add a configuration option to skip arcade sequence?
		// g_system->displayMessageOnOSD(_("Skipping DGDS arcade sequence"));
		// engine->getGameGlobals()->setGlobal(0x21, 6);
		engine->getDragonArcade()->arcadeTick();
		break;
	case kSceneOpDrawDragonCountdown1:
		DragonNative::drawCountdown(FontManager::k4x5Font, 141, 56);
		break;
	case kSceneOpDrawDragonCountdown2:
		DragonNative::drawCountdown(FontManager::k8x8Font, 250, 42);
		break;
	case kSceneOpOpenPlaySkipIntroMenu:
		engine->setMenuToTrigger(kMenuSkipPlayIntro);
		break;
	case kSceneOpOpenBetterSaveGameMenu:
		engine->setMenuToTrigger(kMenuSaveBeforeArcade);
		break;
	default:
		error("Unexpected Dragon scene opcode %d", _opCode);
		break;
	}
	return false;
}

bool SceneOp::runChinaOp() const {
	DgdsEngine *engine = DgdsEngine::getInstance();
	switch (_opCode) {
	case kSceneOpChinaTankInit:
		engine->getChinaTank()->init();
		break;
	case kSceneOpChinaTankEnd:
		engine->getChinaTank()->end();
		break;
	case kSceneOpChinaTankTick:
		engine->getChinaTank()->tick();
		break;
	case kSceneOpShellGameTick:
		engine->getShellGame()->shellGameTick();
		break;
	case kSceneOpShellGameEnd:
		engine->getShellGame()->shellGameEnd();
		break;
	case kSceneOpChinaTrainInit:
		engine->getChinaTrain()->init();
		break;
	case kSceneOpChinaTrainEnd:
		engine->getChinaTrain()->end();
		break;
	case kSceneOpChinaTrainTick:
		engine->getChinaTrain()->tick();
		break;
	case kSceneOpChinaOpenGameOverMenu:
		engine->setMenuToTrigger(kMenuGameOver);
		break;
	case kSceneOpChinaOpenSkipCreditsMenu:
		engine->setMenuToTrigger(kMenuSkipPlayIntro);
		break;
	case kSceneOpChinaOnIntroInit:
		engine->getHocIntro()->init();
		break;
	case kSceneOpChinaOnIntroTick:
		engine->getHocIntro()->tick();
		break;
	case kSceneOpChinaOnIntroEnd:
		engine->getHocIntro()->end();
		break;
	case kSceneOpChinaScrollIntro:
	case kSceneOpChinaScrollLeft:
	case kSceneOpChinaScrollRight:
		// These map to null functions.
		break;
	default:
		warning("TODO: Implement china-specific scene opcode: (%s)", dump("").c_str());
		break;
	}
	return false;
}

bool SceneOp::runBeamishOp() const {
	DgdsEngine *engine = DgdsEngine::getInstance();

	if (_opCode & kSceneOpHasConditionalOpsFlag) {
		uint16 opcode = _opCode & ~kSceneOpHasConditionalOpsFlag;
		for (const ConditionalSceneOp &cop : engine->getScene()->getConditionalOps()) {
			if (cop._opCode == opcode && engine->getScene()->checkConditions(cop._conditionList)) {
				if (!Scene::runOps(cop._opList))
					return true;
			}
		}
		return false;
	}

	switch (_opCode) {
	case kSceneOpOpenBeamishGameOverMenu:
		engine->setMenuToTrigger(kMenuGameOver);
		break;
	case kSceneOpOpenBeamishOpenSkipCreditsMenu:
		engine->setMenuToTrigger(kMenuSkipPlayIntro);
		break;
	default:
		warning("TODO: Implement beamish-specific scene opcode %d", _opCode);
		break;
	}
	return false;
}


Common::String ConditionalSceneOp::dump(const Common::String &indent) const {
	const Common::String opName = _sceneOpCodeName(static_cast<SceneOpCode>(_opCode));
	Common::String str = Common::String::format("%sConditionalSceneOp<op: %s", indent.c_str(), opName.c_str());

	str += DebugUtil::dumpStructList(indent, "conditionList", _conditionList);
	if (!_conditionList.empty()) {
		str += "\n";
		str += indent;
	}
	str += DebugUtil::dumpStructList(indent, "opList", _opList);
	if (!_opList.empty()) {
		str += "\n";
		str += indent;
	}
	str += ">";
	return str;
}

} // end namespace Dgds
