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

#include "common/debug.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/rect.h"
#include "common/system.h"
#include "common/util.h"
#include "common/translation.h"

#include "graphics/cursorman.h"
#include "graphics/surface.h"
#include "graphics/primitives.h"

#include "dgds/dgds.h"
#include "dgds/includes.h"
#include "dgds/resource.h"
#include "dgds/request.h"
#include "dgds/scene.h"
#include "dgds/ads.h"
#include "dgds/menu.h"
#include "dgds/font.h"
#include "dgds/globals.h"
#include "dgds/image.h"
#include "dgds/inventory.h"

namespace Dgds {

template<class C> static Common::String _dumpStructList(const Common::String &indent, const Common::String &name, const C &list) {
	if (list.empty())
		return "";

	const Common::String nextind = indent + "    ";
	Common::String str = Common::String::format("\n%s%s=", Common::String(indent + "  ").c_str(), name.c_str());
	for (const auto &s : list) {
		str += "\n";
		str += s.dump(nextind);
	}
	return str;
}


Common::String _sceneConditionStr(SceneCondition cflag) {
	Common::String ret;

	if (cflag & kSceneCondOr)
		return "or";

	if (cflag & kSceneCondSceneState)
		ret += "state|";
	if (cflag & kSceneCondNeedItemSceneNum)
		ret += "itemsnum|";
	if (cflag & kSceneCondNeedItemQuality)
		ret += "quality|";
	if ((cflag & (kSceneCondSceneState | kSceneCondNeedItemSceneNum | kSceneCondNeedItemQuality)) == 0)
		ret += "global|";

	cflag = static_cast<SceneCondition>(cflag & ~(kSceneCondSceneState | kSceneCondNeedItemSceneNum | kSceneCondNeedItemQuality));
	if (cflag == kSceneCondNone)
		ret += "nocond";
	if (cflag & kSceneCondLessThan)
		ret += "less";
	if (cflag & kSceneCondEqual)
		ret += "equal";
	if (cflag & kSceneCondNegate)
		ret += "-not";

	return ret;
}

Common::String SceneConditions::dump(const Common::String &indent) const {
	return Common::String::format("%sSceneCondition<flg 0x%02x(%s) num %d val %d>", indent.c_str(),
			_flags, _sceneConditionStr(_flags).c_str(), _num, _val);
}


Common::String HotArea::dump(const Common::String &indent) const {
	Common::String str = Common::String::format("%sHotArea<%s num %d cursor %d unk1 %d unk2 %d",
			indent.c_str(), _rect.dump("").c_str(), _num, _cursorNum, _unk1, _unk2);
	str += _dumpStructList(indent, "enableConditions", enableConditions);
	str += _dumpStructList(indent, "onRClickOps", onRClickOps);
	str += _dumpStructList(indent, "onLDownOps", onLDownOps);
	str += _dumpStructList(indent, "onLClickOps", onLClickOps);
	str += "\n";
	str += indent + ">";
	return str;
}


static Common::String _sceneOpCodeName(SceneOpCode code) {
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
	case kSceneOpShowClock:		return "sceneOpShowClock";
	case kSceneOpHideClock:		return "sceneOpHideClock";
	case kSceneOpShowMouse:		return "sceneOpShowMouse";
	case kSceneOpHideMouse:		return "sceneOpHideMouse";
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
		return Common::String::format("sceneOp%d", (int)code);
	}
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
	Common::String str = Common::String::format("%sSceneOp<op: %s args: %s", indent.c_str(), _sceneOpCodeName(_opCode).c_str(), argsStr.c_str());

	str += _dumpStructList(indent, "conditionList", _conditionList);
	if (!_conditionList.empty()) {
		str += "\n";
		str += indent;
	}
	str += ">";
	return str;
}

Common::String GameItem::dump(const Common::String &indent) const {
	Common::String super = HotArea::dump(indent + "  ");

	Common::String str = Common::String::format(
			"%sGameItem<\n%s\n%saltCursor %d icon %d sceneNum %d flags %d quality %d",
			indent.c_str(), super.c_str(), indent.c_str(), _altCursor,
			_iconNum, _inSceneNum, _flags, _quality);
	str += _dumpStructList(indent, "onDragFinishedOps", onDragFinishedOps);
	str += _dumpStructList(indent, "opList5", opList5);
	str += "\n";
	str += indent + ">";
	return str;
}


Common::String MouseCursor::dump(const Common::String &indent) const {
	return Common::String::format("%sMouseCursor<%d %d>", indent.c_str(), _hot.x, _hot.y);
}


Common::String ObjectInteraction::dump(const Common::String &indent) const {
	Common::String str = Common::String::format("%sObjectInteraction<dropped %d target %d", indent.c_str(), _droppedItemNum, _targetItemNum);

	str += _dumpStructList(indent, "opList", opList);
	str += "\n";
	str += indent + ">";
	return str;
}


Common::String SceneTrigger::dump(const Common::String &indent) const {
	Common::String str = Common::String::format("%sSceneTrigger<num %d %s %d", indent.c_str(), _num, _enabled ? "enabled" : "disabled", _unk);
	str += _dumpStructList(indent, "conditionList", conditionList);
	str += _dumpStructList(indent, "opList", sceneOpList);
	str += "\n";
	str += indent + ">";
	return str;
}


Common::String PerSceneGlobal::dump(const Common::String &indent) const {
	return Common::String::format("%sPerSceneGlobal<num %d scene %d val %d>", indent.c_str(), _num, _sceneNo, _val);
}

// //////////////////////////////////// //

//
// Check that a list length seems "sensible" so we can crash with
// a nice error message instead of crash trying to allocate a
// massive list.
//
static void _checkListNotTooLong(uint16 len, const char *list_type) {
	if (len > 1000)
		error("Too many %s in list (%d), scene data is likely corrupt.", list_type, len);
}

Scene::Scene() : _magic(0) {
}

bool Scene::isVersionOver(const char *version) const {
	assert(!_version.empty());
	return strncmp(_version.c_str(), version, _version.size()) > 0;
}

bool Scene::isVersionUnder(const char *version) const {
	assert(!_version.empty());
	return strncmp(_version.c_str(), version, _version.size()) < 0;
}


bool Scene::readConditionList(Common::SeekableReadStream *s, Common::Array<SceneConditions> &list) const {
	uint16 num = s->readUint16LE();
	_checkListNotTooLong(num, "scene conditions");
	for (uint16 i = 0; i < num; i++) {
		uint16 cnum = s->readUint16LE();
		SceneCondition cond = static_cast<SceneCondition>(s->readUint16LE());
		uint16 val = s->readUint16LE();
		list.push_back(SceneConditions(cnum, cond, val));
	}
	return !s->err();
}


bool Scene::readHotArea(Common::SeekableReadStream *s, HotArea &dst) const {
	dst._rect.x = s->readUint16LE();
	dst._rect.y = s->readUint16LE();
	dst._rect.width = s->readUint16LE();
	dst._rect.height = s->readUint16LE();
	dst._num = s->readUint16LE();
	dst._cursorNum = s->readUint16LE();
	if (isVersionOver(" 1.217"))
		dst._unk1 = s->readUint16LE();
	else
		dst._unk1 = 0;

	if (isVersionOver(" 1.218")) {
		dst._unk2 = s->readUint16LE();
		if (dst._unk2) {
			dst._rect = DgdsRect();
		}
	} else {
		dst._unk2 = 0;
	}
	readConditionList(s, dst.enableConditions);
	readOpList(s, dst.onRClickOps);
	readOpList(s, dst.onLDownOps);
	readOpList(s, dst.onLClickOps);
	return !s->err();
}


bool Scene::readHotAreaList(Common::SeekableReadStream *s, Common::List<HotArea> &list) const {
	uint16 num = s->readUint16LE();
	_checkListNotTooLong(num, "hot areas");
	for (uint16 i = 0; i < num; i++) {
		HotArea dst;
		readHotArea(s, dst);
		list.push_back(dst);
	}
	return !s->err();
}


bool Scene::readGameItemList(Common::SeekableReadStream *s, Common::Array<GameItem> &list) const {
	uint16 num = s->readUint16LE();
	_checkListNotTooLong(num, "game items");
	list.resize(num);

	for (GameItem &dst : list) {
		readHotArea(s, dst);
	}
	for (GameItem &dst : list) {
		dst._iconNum = s->readUint16LE();
		dst._inSceneNum = s->readUint16LE();
		dst._quality = s->readUint16LE();
		if (!isVersionUnder(" 1.211"))
			dst._flags = s->readUint16LE() & 0xfffe;
		if (!isVersionUnder(" 1.204")) {
			dst._altCursor = s->readUint16LE();
			readOpList(s, dst.onDragFinishedOps);
			readOpList(s, dst.opList5);
		}
	}
	return !s->err();
}


bool Scene::readMouseHotspotList(Common::SeekableReadStream *s, Common::Array<MouseCursor> &list) const {
	uint16 num = s->readUint16LE();
	_checkListNotTooLong(num, "mouse hotspots");

	for (uint16 i = 0; i < num; i++) {
		list.push_back(MouseCursor(s->readUint16LE(), s->readUint16LE()));
	}
	return !s->err();
}


bool Scene::readObjInteractionList(Common::SeekableReadStream *s, Common::Array<ObjectInteraction> &list) const {
	uint16 num = s->readUint16LE();
	_checkListNotTooLong(num, "interactions");

	for (uint16 i = 0; i < num; i++) {
		uint16 dropped, target;
		if (!isVersionOver(" 1.205")) {
			target = s->readUint16LE();
			dropped = s->readUint16LE();
			target += s->readUint16LE();
		} else {
			dropped = s->readUint16LE();
			target = s->readUint16LE();
		}
		list.push_back(ObjectInteraction(dropped, target));
		readOpList(s, list.back().opList);
	}
	return !s->err();
}


bool Scene::readOpList(Common::SeekableReadStream *s, Common::Array<SceneOp> &list) const {
	uint16 nitems = s->readUint16LE();
	_checkListNotTooLong(nitems, "scene ops");
	list.resize(nitems);
	for (SceneOp &dst : list) {
		readConditionList(s, dst._conditionList);
		dst._opCode = static_cast<SceneOpCode>(s->readUint16LE());
		if (dst._opCode > kSceneOpMaxCode || dst._opCode == kSceneOpNone)
			error("Unexpected scene opcode %d", (int)dst._opCode);
		uint16 nvals = s->readUint16LE();
		_checkListNotTooLong(nvals, "scene op args");
		for (uint16 i = 0; i < nvals / 2; i++) {
			dst._args.push_back(s->readUint16LE());
		}
	}

	return !s->err();
}


bool Scene::readDialogList(Common::SeekableReadStream *s, Common::Array<Dialog> &list) const {
	// Some data on this format here https://www.oldgamesitalia.net/forum/index.php?showtopic=24055&st=25&p=359214&#entry359214

	uint16 nitems = s->readUint16LE();
	_checkListNotTooLong(nitems, "dialogs");
	list.resize(nitems);

	for (Dialog &dst : list) {
		dst._num = s->readUint16LE();
		dst._rect.x = s->readUint16LE();
		dst._rect.y = s->readUint16LE();
		dst._rect.width = s->readUint16LE();
		dst._rect.height = s->readUint16LE();
		dst._bgColor = s->readUint16LE();
		dst._fontColor = s->readUint16LE(); // 0 = black, 0xf = white
		if (isVersionUnder(" 1.209")) {
			dst._selectionBgCol = dst._bgColor;
			dst._selectonFontCol = dst._fontColor;
		} else {
			dst._selectionBgCol = s->readUint16LE();
			dst._selectonFontCol = s->readUint16LE();
		}
		dst._fontSize = s->readUint16LE(); // 01 = 8x8, 02 = 6x6, 03 = 4x5
		if (isVersionUnder(" 1.210")) {
			dst._flags = static_cast<DialogFlags>(s->readUint16LE());
		} else {
			// Game reads a 32 bit int but then truncates anyway..
			// probably never used the full thing in SDS files
			// as most higher bits are render state.
			dst._flags = static_cast<DialogFlags>(s->readUint32LE() & 0xffff);
		}

		dst._frameType = static_cast<DialogFrameType>(s->readUint16LE());
		dst._time = s->readUint16LE();
		if (isVersionOver(" 1.215")) {
			s->readUint16LE();
			error("TODO: what is this extra int in dialog action list?");
		}
		if (isVersionOver(" 1.207")) {
			dst._nextDialogNum = s->readUint16LE();
		}

		uint16 nbytes = s->readUint16LE();
		if (nbytes > 0) {
			dst._str = s->readString('\0', nbytes);
		} else {
			dst._str.clear();
		}
		readDialogActionList(s, dst._action);

		if (isVersionUnder(" 1.209") && !dst._action.empty()) {
			if (dst._fontColor == 0)
				dst._selectonFontCol = 4;
			else if (dst._fontColor == 0xff)
				dst._fontColor = 7;
			else
				dst._fontColor = dst._fontColor ^ 8;
		}

		dst.fixupStringAndActions();
	}

	return !s->err();
}


bool Scene::readTriggerList(Common::SeekableReadStream *s, Common::Array<SceneTrigger> &list) const {
	uint16 num = s->readUint16LE();
	_checkListNotTooLong(num, "triggers");
	for (uint16 i = 0; i < num; i++) {
		list.push_back(SceneTrigger(s->readUint16LE()));
		if (isVersionOver(" 1.219"))
			list.back()._unk = s->readUint16LE();
		readConditionList(s, list.back().conditionList);
		readOpList(s, list.back().sceneOpList);
	}

	return !s->err();
}


bool Scene::readDialogActionList(Common::SeekableReadStream *s, Common::Array<DialogAction> &list) const {
	uint16 num = s->readUint16LE();
	_checkListNotTooLong(num, "dialog actions");
	list.resize(num);

	// The original initializes a field in the first entry to 1 here, but it seems
	// only used for memory management so we don't need it?
	// if (!list.empty())
	//	list[0].val = 1;

	for (DialogAction &dst : list) {
		dst.strStart = s->readUint16LE();
		dst.strEnd = s->readUint16LE();
		readOpList(s, dst.sceneOpList);
	}

	return !s->err();
}


void Scene::setItemAttrOp(const Common::Array<uint16> &args) {
	if (args.size() < 3)
		error("Expect 3 args for item attr opcode.");

	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	for (auto &item : engine->getGDSScene()->getGameItems()) {
		if (item._num != args[0])
			continue;

		if (args[1] != 0xffff) {
			//bool doDraw = item._inSceneNum != args[1] && engine->getScene()->getNum() == args[1];
			item._inSceneNum = args[1];
		}

		if (args[2])
			item._quality = args[2];

		break;
	}
}

void Scene::setDragItemOp(const Common::Array<uint16> &args) {
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);

	for (auto &item : engine->getGDSScene()->getGameItems()) {
		if (item._num != args[0])
			continue;

		bool inScene = (item._inSceneNum == engine->getScene()->getNum());
		engine->getScene()->setDragItem(&item);
		if (!inScene)
			item._inSceneNum = engine->getScene()->getNum(); // else do some redraw??

		Common::Point lastMouse = engine->getLastMouse();
		item._rect.x = lastMouse.x;
		item._rect.y = lastMouse.y;
		engine->setMouseCursor(item._iconNum);
	}
}

void Scene::segmentStateOps(const Common::Array<uint16> &args) {
	ADSInterpreter *interp = static_cast<DgdsEngine *>(g_engine)->adsInterpreter();

	for (uint i = 0; i < args.size(); i += 2) {
		uint16 subop = args[i];
		uint16 arg = args[i + 1];
		if (!subop && !arg)
			return;
		switch (subop) {
		case 1:
			interp->segmentOrState(arg, 3);
			break;
		case 2:
			interp->segmentOrState(arg, 4);
			break;
		case 3:
			interp->segmentSetState(arg, 6);
			break;
		case 4:
			interp->segmentSetState(arg, 5);
			break;
		case 9:
			warning("TODO: Apply segment state 3 to all loaded ADS texts");
			interp->segmentOrState(arg, 3);
			break;
		case 10:
			warning("TODO: Apply segment state 4 to all loaded ADS texts");
			interp->segmentOrState(arg, 4);
			break;
		case 11:
			warning("TODO: Apply segment state 6 to all loaded ADS texts");
			interp->segmentSetState(arg, 6);
			break;
		case 12:
			warning("TODO: Apply segment state 5 to all loaded ADS texts");
			interp->segmentSetState(arg, 5);
			break;
		default:
			error("Unknown scene op 4 sub-opcode %d", subop);
		}
	}
}

static void _drawDragonCountdown(FontManager::FontType fontType, int16 x, int16 y) {
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	int16 countdownEnd = engine->getGameGlobals()->getGlobal(0x22);
	int16 currentMins = engine->getClock().getMins();
	const DgdsFont *fnt = engine->getFontMan()->getFont(fontType);
	Common::String str = Common::String::format("%d", countdownEnd - currentMins);
	fnt->drawString(&engine->_compositionBuffer, str, x, y, 320 - x, 10);
}


bool Scene::runSceneOp(const SceneOp &op) {
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	switch (op._opCode) {
	case kSceneOpChangeScene:
		if (engine->changeScene(op._args[0]))
			// This probably reset the list - stop now.
			return false;
		break;
	case kSceneOpNoop:
		break;
	case kSceneOpGlobal:
		globalOps(op._args);
		break;
	case kSceneOpSegmentStateOps:
		segmentStateOps(op._args);
		break;
	case kSceneOpSetItemAttr:
		setItemAttrOp(op._args);
		break;
	case kSceneOpSetDragItem:
		setDragItemOp(op._args);
		break;
	case kSceneOpOpenInventory:
		engine->getInventory()->open();
		// This implicitly changes scene num
		return false;
	case kSceneOpShowDlg:
		showDialog(op._args[0]);
		break;
	case kSceneOpShowInvButton:
		engine->getScene()->addInvButtonToHotAreaList();
		break;
	case kSceneOpHideInvButton:
		engine->getScene()->removeInvButtonFromHotAreaList();
		break;
	case kSceneOpEnableTrigger:
		enableTrigger(op._args[0]);
		break;
	case kSceneOpChangeSceneToStored: {
		uint16 sceneNo = engine->getGameGlobals()->getGlobal(0x61);
		if (engine->changeScene(sceneNo))
			// This probably reset the list - stop now.
			return false;
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
		return false;
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
	default:
		warning("TODO: Implement generic scene op %d", op._opCode);
		break;
	}
	return true;
}

bool Scene::runDragonOp(const SceneOp &op) {
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	switch (op._opCode) {
	case kSceneOpPasscode:
		engine->getScene()->sceneOpUpdatePasscodeGlobal();
		break;
	case kSceneOpMeanwhile:
		// TODO: Should we draw "meanwhile" like the original? it just gets overwritten with the image anyway.
		// Probably need to do something here to avoid flashing..
		//engine->_compositionBuffer.fillRect(Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 0);
		break;
	case kSceneOpOpenGameOverMenu:
		engine->setMenuToTrigger(kMenuGameOver);
		break;
	case kSceneOpTiredDialog:
		engine->getInventory()->close();
		engine->getScene()->addAndShowTiredDialog();
		break;
	case kSceneOpArcadeTick:
		// TODO: Implement this properly! for now just
		// set the global arcade state variable to the "skip" value.
		warning("Setting arcade global to 8 (skip)");
		g_system->displayMessageOnOSD(_("Skipping DGDS arcade sequence"));
		engine->getGameGlobals()->setGlobal(0x21, 6);
		break;
	case kSceneOpDrawDragonCountdown1:
		_drawDragonCountdown(FontManager::k4x5Font, 141, 56);
		break;
	case kSceneOpDrawDragonCountdown2:
		_drawDragonCountdown(FontManager::k8x8Font, 250, 42);
		break;
	case kSceneOpOpenPlaySkipIntroMenu:
		engine->setMenuToTrigger(kMenuSkipPlayIntro);
		break;
	case kSceneOpOpenBetterSaveGameMenu:
		engine->setMenuToTrigger(kMenuSaveBeforeArcade);
		break;
	default:
		error("Unexpected Dragon scene opcode %d", op._opCode);
		break;
	}
	return true;
}

bool Scene::runChinaOp(const SceneOp &op) {
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	switch (op._opCode) {
	case kSceneOpOpenChinaOpenGameOverMenu:
		engine->setMenuToTrigger(kMenuGameOver);
		break;
	case kSceneOpOpenChinaOpenSkipCreditsMenu:
		engine->setMenuToTrigger(kMenuSkipPlayIntro);
		break;
	case kSceneOpOpenChinaStartIntro:
		warning("TODO: Implement start intro opcode");
		break;
	default:
		warning("TODO: Implement china-specific scene opcode %d", op._opCode);
		break;
	}
	return true;
}

bool Scene::runBeamishOp(const SceneOp &op) {
	error("TODO: Implement beamish-specific scene op %d", op._opCode);
	/*
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	switch (op._opCode) {
	default:
		warning("TODO: Implement beamish-specific scene opcode %d", op._opCode);
		break;
	}
	return true;
	*/
}

bool Scene::runOps(const Common::Array<SceneOp> &ops, int16 addMinuites /* = 0 */) {
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	for (const SceneOp &op : ops) {
		if (!checkConditions(op._conditionList))
			continue;
		debug(10, "Exec %s", op.dump("").c_str());
		if (addMinuites) {
			engine->getClock().addGameTime(addMinuites);
			addMinuites = 0;
		}
		bool keepGoing = true;
		if (op._opCode < 100) {
			keepGoing = runSceneOp(op);
		} else {
			// Game-specific opcode
			switch (engine->getGameId()) {
			case GID_DRAGON:
				keepGoing = runDragonOp(op);
				break;
			case GID_CHINA:
				keepGoing = runChinaOp(op);
				break;
			case GID_BEAMISH:
				keepGoing = runBeamishOp(op);
				break;
			default:
				error("TODO: Implement game-specific scene op for this game");
			}
			continue;
		}
		if (!keepGoing)
			return false;
	}
	return true;
}

bool Scene::checkConditions(const Common::Array<SceneConditions> &conds) const {
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);

	uint cnum = 0;
	while (cnum < conds.size()) {
		const SceneConditions &c = conds[cnum];
		int16 refval = c.getVal();
		int16 checkval = -1;
		SceneCondition cflag = c.getCond();
		// Hit an "or" here means the last result was true.
		if (cflag & kSceneCondOr)
			return true;

		if (cflag & kSceneCondSceneState) {
			refval = 1;
			checkval = engine->adsInterpreter()->getStateForSceneOp(c.getNum());
			SceneCondition equalOrNegate = static_cast<SceneCondition>(cflag & (kSceneCondEqual | kSceneCondNegate));
			if (equalOrNegate != kSceneCondEqual && equalOrNegate != kSceneCondNegate)
				refval = 0;
			cflag = kSceneCondEqual;
		} else if (cflag & kSceneCondNeedItemQuality || cflag & kSceneCondNeedItemSceneNum) {
			const Common::Array<GameItem> &items = engine->getGDSScene()->getGameItems();
			for (const auto &item : items) {
				if (item._num == c.getNum()) {
					if (cflag & kSceneCondNeedItemSceneNum)
						checkval = item._inSceneNum;
					else // cflag & kSceneCondNeedItemQuality
						checkval = item._quality;
					break;
				}
			}
		} else {
			checkval = engine->getGDSScene()->getGlobal(c.getNum());
			if (!(cflag & kSceneCondAbsVal))
				refval = engine->getGDSScene()->getGlobal((uint16)refval);
		}

		bool result = false;
		cflag = static_cast<SceneCondition>(cflag & ~(kSceneCondSceneState | kSceneCondNeedItemSceneNum | kSceneCondNeedItemQuality));
		if (cflag == kSceneCondNone)
			cflag = static_cast<SceneCondition>(kSceneCondEqual | kSceneCondNegate);
		if ((cflag & kSceneCondLessThan) && checkval < refval)
			result = true;
		if ((cflag & kSceneCondEqual) && checkval == refval)
			result = true;
		if (cflag & kSceneCondNegate)
			result = !result;

		debug(11, "Cond: %s -> %s", c.dump("").c_str(), result ? "true": "false");

		if (!result) {
			// Skip just past the next or, or to the end.
			while (cnum < conds.size() && !(conds[cnum].getCond() & kSceneCondOr))
				cnum++;
			if (cnum >= conds.size())
				return false;
		}
		cnum++;
	}
	return true;
}


bool SDSScene::_dlgWithFlagLo8IsClosing = false;;
DialogFlags SDSScene::_sceneDialogFlags = kDlgFlagNone;

SDSScene::SDSScene() : _num(-1), _dragItem(nullptr), _shouldClearDlg(false), _ignoreMouseUp(false) {
}

bool SDSScene::load(const Common::String &filename, ResourceManager *resourceManager, Decompressor *decompressor) {
	Common::SeekableReadStream *sceneFile = resourceManager->getResource(filename);
	if (!sceneFile)
		error("Scene file %s not found", filename.c_str());

	DgdsChunkReader chunk(sceneFile);

	bool result = false;

	while (chunk.readNextHeader(EX_SDS, filename)) {
		if (chunk.isContainer()) {
			continue;
		}

		chunk.readContent(decompressor);
		Common::SeekableReadStream *stream = chunk.getContent();

		if (chunk.isSection(ID_SDS)) {
			result = parse(stream);
		}
	}

	delete sceneFile;

	return result;
}


bool SDSScene::parse(Common::SeekableReadStream *stream) {
	_magic = stream->readUint32LE();
	_version = stream->readString();
	//if (isVersionOver(" 1.211")) { // Dragon
	//if (isVersionOver(" 1.216")) { // HoC
	if (isVersionOver(" 1.224")) { // Beamish
		error("Unsupported scene version '%s'", _version.c_str());
	}
	_num = stream->readUint16LE();
	readOpList(stream, _enterSceneOps);
	readOpList(stream, _leaveSceneOps);
	if (isVersionOver(" 1.206")) {
		readOpList(stream, _preTickOps);
	}
	readOpList(stream, _postTickOps);
	_field6_0x14 = stream->readUint16LE();
	_adsFile = stream->readString();
	readHotAreaList(stream, _hotAreaList);
	readObjInteractionList(stream, _objInteractions1);
	if (isVersionOver(" 1.205")) {
		readObjInteractionList(stream, _objInteractions2);
	}
	if (isVersionUnder(" 1.214")) {
		readDialogList(stream, _dialogs);
	}
	if (isVersionOver(" 1.203")) {
		readTriggerList(stream, _triggers);
	}
	if (isVersionOver(" 1.223")) {
		warning("TODO: Read another list here.");
	}

	return !stream->err();
}

void SDSScene::unload() {
	_num = 0;
	_enterSceneOps.clear();
	_leaveSceneOps.clear();
	_preTickOps.clear();
	_postTickOps.clear();
	_field6_0x14 = 0;
	_adsFile.clear();
	_hotAreaList.clear();
	_objInteractions1.clear();
	_objInteractions2.clear();
	_dialogs.clear();
	_triggers.clear();
	_sceneDialogFlags = kDlgFlagNone;
}


Common::String SDSScene::dump(const Common::String &indent) const {
	Common::String str = Common::String::format("%sSDSScene<num %d %d ads %s", indent.c_str(), _num, _field6_0x14, _adsFile.c_str());
	str += _dumpStructList(indent, "enterSceneOps", _enterSceneOps);
	str += _dumpStructList(indent, "leaveSceneOps", _leaveSceneOps);
	str += _dumpStructList(indent, "preTickOps", _preTickOps);
	str += _dumpStructList(indent, "postTickOps", _postTickOps);
	str += _dumpStructList(indent, "hotAreaList", _hotAreaList);
	str += _dumpStructList(indent, "objInteractions1", _objInteractions1);
	str += _dumpStructList(indent, "objInteractions2", _objInteractions2);
	str += _dumpStructList(indent, "dialogues", _dialogs);
	str += _dumpStructList(indent, "triggers", _triggers);

	str += "\n";
	str += indent + ">";
	return str;
}


void SDSScene::enableTrigger(uint16 num, bool enable /* = true */) {
	for (auto &trigger : _triggers) {
		if (trigger.getNum() == num) {
			trigger._enabled = enable;
			break;
		}
	}
}

void SDSScene::checkTriggers() {
	// scene can change on these triggers.  if that happens we stop.
	int startSceneNum = _num;

	for (SceneTrigger &trigger : _triggers) {
		if (!trigger._enabled)
			continue;

		if (!checkConditions(trigger.conditionList))
			continue;

		trigger._enabled = false;
		runOps(trigger.sceneOpList);

		// If the scene changed, the list is no longer valid. Abort!
		if (_num != startSceneNum)
			return;
	}
}

static const uint16 TIRED_DLG_ID = 7777;

void SDSScene::addAndShowTiredDialog() {
	bool haveTiredDlg = false;
	for (auto &d : _dialogs) {
		if (d._num == TIRED_DLG_ID) {
			haveTiredDlg = true;
			break;
		}
	}
	if (!haveTiredDlg) {
		Dialog dlg;
		dlg._num = TIRED_DLG_ID;
		dlg._rect = DgdsRect(4, 18, 208, 91);
		dlg._bgColor = 15;
		dlg._fontColor = 0;
		dlg._selectionBgCol = 15;
		dlg._selectonFontCol = 0;
		dlg._flags = static_cast<DialogFlags>(kDlgFlagLo8 | kDlgFlagLeftJust | kDlgFlagFlatBg);
		dlg._frameType = kDlgFrameThought;
		dlg._time = 420;
		dlg._str = "Boy, am I tired.  Better get some sleep in about an hour.";
		_dialogs.push_back(dlg);
	}
	showDialog(TIRED_DLG_ID);
}


// The first row of this array corresponds to the
// positions of buttons in game passcode
// RYP YWP YRPWRY PBW
static const uint16 DRAGON_PASSCODE[] = {
	1, 4, 3, 4, 0, 3, 4, 1, 3, 0, 1, 4, 3, 2, 0,
	4, 4, 2, 3, 4, 0, 0, 4, 3, 2, 1, 1, 2, 4, 0,
	4, 1, 3, 2, 0, 2, 1, 4, 3, 4, 1, 3, 2, 0, 1
};

static uint16 passcodeBlockNum = 0;
static uint16 passcodeVal1 = 0;
static uint16 passcodeVal2 = 0;
static uint16 passcodeVal3 = 0;
static uint16 passcodeVal4 = 0;

void SDSScene::sceneOpUpdatePasscodeGlobal() {
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	int16 globalval = engine->getGDSScene()->getGlobal(0x20);

	if (globalval > 34)
		return;

	if (globalval >= 30) {
		// One of the keypad buttons
		if (DRAGON_PASSCODE[passcodeVal4 + passcodeBlockNum * 15] == globalval - 30) {
			debug("sceneOpUpdatePasscodeGlobal CORRECT: variables %d %d %d %d block %d, curval %d",
				passcodeVal1, passcodeVal2, passcodeVal3, passcodeVal4, passcodeBlockNum, globalval);

			// Correct entry! Increment the expected button
			passcodeVal4++;
			if (passcodeVal4 < passcodeVal3) {
				globalval = 0;
			} else if (passcodeVal3 < 15) {
				globalval = 5;
			} else {
				// Finished!
				globalval = 6;
			}
		} else {
			// Mistake
			debug("sceneOpUpdatePasscodeGlobal WRONG: variables %d %d %d %d block %d, curval %d",
				passcodeVal1, passcodeVal2, passcodeVal3, passcodeVal4, passcodeBlockNum, globalval);
			passcodeVal1 = 0;
			passcodeVal2 = 5;
			globalval = 7;
		}
	} else {
		if (globalval > 4 || globalval == 0)
			return;

		debug("sceneOpUpdatePasscodeGlobal OTHER: variables %d %d %d %d block %d, curval %d",
				passcodeVal1, passcodeVal2, passcodeVal3, passcodeVal4, passcodeBlockNum, globalval);

		if (globalval < 4) {
			passcodeBlockNum = globalval - 1; // expect block globalval-1
			passcodeVal1 = 5;
			passcodeVal2 = 0;
			passcodeVal3 = 15;	// 15 buttons expected
			passcodeVal4 = 0;
			return;
		} else if (passcodeVal2 > passcodeVal1) {
			passcodeVal1++;
			globalval = DRAGON_PASSCODE[passcodeVal1 + passcodeBlockNum * 15] + 20;
		} else if (passcodeVal2 > 14) {
			passcodeVal1 = 0;
			passcodeVal3 = passcodeVal2;
			passcodeVal4 = 0;
			globalval = 8;
		} else {
			passcodeVal1 = 0;
			passcodeVal2 += 5;
			passcodeVal3 = passcodeVal1;
			passcodeVal4 = 0;
			globalval = 8;
		}
	}

	engine->getGDSScene()->setGlobal(0x20, globalval);
}

void SDSScene::showDialog(uint16 num) {
	for (auto &dialog : _dialogs) {
		if (dialog._num == num) {
			dialog.clearFlag(kDlgFlagHiFinished);
			dialog.clearFlag(kDlgFlagRedrawSelectedActionChanged);
			dialog.clearFlag(kDlgFlagHi10);
			//dialog.clearFlag(kDlgFlagHi20);
			dialog.clearFlag(kDlgFlagHi40);
			dialog.setFlag(kDlgFlagHi20);
			dialog.setFlag(kDlgFlagVisible);
			dialog.setFlag(kDlgFlagOpening);
			// hide time gets set the first time it's drawn.
			if (_dlgWithFlagLo8IsClosing && dialog.hasFlag(kDlgFlagLo8)) {
				_sceneDialogFlags = static_cast<DialogFlags>(_sceneDialogFlags | kDlgFlagLo8 | kDlgFlagVisible);
			}
			if (_dlgWithFlagLo8IsClosing) {
				// TODO: call some function (FUN_1f1a_4205) here.
			}
		}
	}
}

bool SDSScene::checkDialogActive() {
	uint32 timeNow = g_engine->getTotalPlayTime();
	bool retval = false;

	_sceneDialogFlags = kDlgFlagNone;

	bool clearDlgFlag = _shouldClearDlg; // ((g_gameStateFlag_41f6 | UINT_39e5_41f8) & 6) != 0); ??
	_shouldClearDlg = false;

	for (auto &dlg : _dialogs) {
		if (!dlg.hasFlag(kDlgFlagVisible))
			continue;

		if (!dlg._state)
			dlg._state.reset(new DialogState());

		// FIXME: double-check this logic.
		// Mark finished if we are manually clearing *or* the timer has expired.
		bool finished = false;
		if (clearDlgFlag || (dlg._state->_hideTime && timeNow >= dlg._state->_hideTime)) {
			finished = true;
		}

		bool no_options = false;
		if ((dlg._state->_hideTime == 0) && dlg._action.size() < 2)
			no_options = true;

		if ((!finished && !no_options) || dlg.hasFlag(kDlgFlagHi20) || dlg.hasFlag(kDlgFlagHi40)) {
			if (!finished && dlg._action.size() > 1 && !dlg.hasFlag(kDlgFlagHiFinished)) {
				DialogAction *action = dlg.pickAction(false, clearDlgFlag);
				if (dlg._state->_selectedAction != action) {
					dlg._state->_selectedAction = action;
					dlg.clearFlag(kDlgFlagHi10);
					dlg.setFlag(kDlgFlagRedrawSelectedActionChanged);
				}
			}
		} else {
			// this dialog is finished - call the ops and maybe show the next one
			_dlgWithFlagLo8IsClosing = dlg.hasFlag(kDlgFlagLo8);
			DialogAction *action = dlg.pickAction(true, clearDlgFlag);
			if (action || dlg._action.empty()) {
				dlg.setFlag(kDlgFlagHiFinished);
				if (action) {
					debug("Dialog closing: run action (%d ops)", action->sceneOpList.size());
					if (!runOps(action->sceneOpList)) {
						_dlgWithFlagLo8IsClosing = false;
						return true;
					}
				}
			}
			if (dlg._nextDialogNum) {
				dlg.setFlag(kDlgFlagHiFinished);
				showDialog(dlg._nextDialogNum);
			}
		}
		if (dlg.hasFlag(kDlgFlagVisible)) {
			_sceneDialogFlags = static_cast<DialogFlags>(_sceneDialogFlags | kDlgFlagVisible);
		}
	}
	return retval;
}

void SDSScene::drawActiveDialogBgs(Graphics::ManagedSurface *dst) {
	for (auto &dlg : _dialogs) {
		if (dlg.hasFlag(kDlgFlagVisible) && !dlg.hasFlag(kDlgFlagOpening)) {
			dlg.draw(dst, kDlgDrawStageBackground);
			// FIXME: Original clears Hi20 and sets Hi40 here, but with our
			// call sequence that means the time never works right in
			// drawAndUpdateDialogs??
			//dlg.clearFlag(kDlgFlagHi20);
			//dlg.setFlag(kDlgFlagHi40);
		}
	}
}

bool SDSScene::checkForClearedDialogs() {
	bool result = false;
	bool have8 = false;
	for (auto &dlg : _dialogs) {
		if (!dlg.hasFlag(kDlgFlagHiFinished)) {
			if (dlg.hasFlag(kDlgFlagLo8))
				have8 = true;
		} else {
			dlg.clear();
			result = true;
		}
	}

	if (!have8) {
		_sceneDialogFlags = static_cast<DialogFlags>(_sceneDialogFlags & ~kDlgFlagLo8);
	}
	return result;
}

bool SDSScene::drawAndUpdateDialogs(Graphics::ManagedSurface *dst) {
	bool retval = false;
	const DgdsEngine *engine = static_cast<const DgdsEngine *>(g_engine);
	for (auto &dlg : _dialogs) {
		if (dlg.hasFlag(kDlgFlagVisible) && !dlg.hasFlag(kDlgFlagLo4) &&
				!dlg.hasFlag(kDlgFlagHi20) && !dlg.hasFlag(kDlgFlagHi40)) {
			// TODO: do something with "transfer"s?
			dlg.setFlag(kDlgFlagHi4);
		}
		if (!dlg.hasFlag(kDlgFlagVisible) || (!dlg.hasFlag(kDlgFlagLo4) && !dlg.hasFlag(kDlgFlagHi4) && !dlg.hasFlag(kDlgFlagHi20) && !dlg.hasFlag(kDlgFlagHi40))) {
			if (dlg.hasFlag(kDlgFlagRedrawSelectedActionChanged) || dlg.hasFlag(kDlgFlagHi10)) {
				dlg.draw(dst, kDlgDrawStageForeground);
				if (!dlg.hasFlag(kDlgFlagRedrawSelectedActionChanged)) {
					dlg.clearFlag(kDlgFlagHi10);
				} else {
					dlg.flipFlag(kDlgFlagRedrawSelectedActionChanged);
					dlg.flipFlag(kDlgFlagHi10);
				}
			}
		} else if (!dlg.hasFlag(kDlgFlagOpening)) {
			dlg.draw(dst, kDlgDrawStageBackground);
			// HACK: always draw foreground here too..??? The original doesn't but we never
			// seem to end up calling the foreground draw function..
			dlg.draw(dst, kDlgDrawFindSelectionPointXY);
			dlg.draw(dst, kDlgDrawFindSelectionTxtOffset);
			dlg.draw(dst, kDlgDrawStageForeground);
			if (dlg.hasFlag(kDlgFlagHi20)) {
				// Reset the dialog time and selected action
				int delay = 0xffff;
				if (dlg._time)
					delay = dlg._time;

				int time = delay * (9 - engine->getTextSpeed());
				assert(dlg._state);

				dlg._state->_hideTime = g_engine->getTotalPlayTime() + time;
				dlg._state->_selectedAction = nullptr;
				dlg.updateSelectedAction(0);
				if (dlg._action.size() > 1 && !dlg._state->_selectedAction) {
					dlg._state->_selectedAction = dlg.pickAction(false, false);
					if (dlg._state->_selectedAction)
						dlg.draw(dst, kDlgDrawStageForeground);
				}
			}

			if (!dlg.hasFlag(kDlgFlagHi20)) {
				dlg.clearFlag(kDlgFlagHi40);
			} else {
				dlg.flipFlag(kDlgFlagHi20);
				dlg.flipFlag(kDlgFlagHi40);
			}
			dlg.clearFlag(kDlgFlagHi4);
			retval = true;
		} else if (!engine->justChangedScene1()) {
			dlg.clearFlag(kDlgFlagOpening);
		}

		if (dlg.hasFlag(kDlgFlagVisible) && !dlg.hasFlag(kDlgFlagLo4) &&
				!dlg.hasFlag(kDlgFlagHi20) && !dlg.hasFlag(kDlgFlagHi40)) {
			// TODO: do something with "transfer"s?
			// warning("SDSScene::drawActiveDrawAndUpdateDialogs: Do something with transfers?");
			dlg.setFlag(kDlgFlagHi4);
		}
		if (dlg.hasFlag(kDlgFlagVisible) && !dlg.hasFlag(kDlgFlagOpening)) {
			_sceneDialogFlags = static_cast<DialogFlags>(_sceneDialogFlags | kDlgFlagLo8 | kDlgFlagVisible);
		}
	}
	return retval;
}

void SDSScene::globalOps(const Common::Array<uint16> &args) {
	// The globals are held by the GDS scene
	static_cast<DgdsEngine *>(g_engine)->getGDSScene()->globalOps(args);
}

void SDSScene::mouseMoved(const Common::Point &pt) {
	Dialog *dlg = getVisibleDialog();
	const HotArea *area = findAreaUnderMouse(pt);

	int16 cursorNum = (!dlg && area) ? area->_cursorNum : 0;
	if (_dragItem)
		cursorNum = _dragItem->_iconNum;

	static_cast<DgdsEngine *>(g_engine)->setMouseCursor(cursorNum);
}

void SDSScene::mouseLDown(const Common::Point &pt) {
	if (hasVisibleDialog()) {
		debug(9, "Mouse LDown on at %d,%d clearing visible dialog", pt.x, pt.y);
		_shouldClearDlg = true;
		_ignoreMouseUp = true;
		return;
	}

	HotArea *area = findAreaUnderMouse(pt);
	if (!area)
		return;

	debug(9, "Mouse LDown on area %d (%d,%d,%d,%d) cursor %d", area->_num, area->_rect.x, area->_rect.y,
			area->_rect.width, area->_rect.height, area->_cursorNum);

	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	int16 addmins = static_cast<DragonGlobals *>(engine->getGameGlobals())->getGameMinsToAddOnStartDrag();
	runOps(area->onLDownOps, addmins);
	GameItem *item = dynamic_cast<GameItem *>(area);
	if (item) {
		_dragItem = item;
		if (item->_iconNum)
			engine->setMouseCursor(item->_iconNum);
	}
}

void SDSScene::mouseLUp(const Common::Point &pt) {
	if (_ignoreMouseUp) {
		debug(9, "Ignoring mouseup at %d,%d as it was used to clear a dialog", pt.x, pt.y);
		_ignoreMouseUp = false;
		return;
	}

	if (_dragItem) {
		onDragFinish(pt);
		return;
	}

	const HotArea *area = findAreaUnderMouse(pt);
	if (!area)
		return;

	debug(9, "Mouse LUp on area %d (%d,%d,%d,%d) cursor %d", area->_num, area->_rect.x, area->_rect.y,
			area->_rect.width, area->_rect.height, area->_cursorNum);

	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	engine->setMouseCursor(area->_cursorNum);

	if (area && area->_num == 0) {
		debug("Mouseup on inventory.");
		engine->getInventory()->open();
	} else {
		debug(" --> exec %d click ops for area %d", area->onLClickOps.size(), area->_num);
		int16 addmins = static_cast<DragonGlobals *>(engine->getGameGlobals())->getGameMinsToAddOnLClick();
		runOps(area->onLClickOps, addmins);
	}
}

static bool _isInRect(const Common::Point &pt, const DgdsRect rect) {
	return rect.x <= pt.x && (rect.x + rect.width) > pt.x
			&& rect.y <= pt.y && (rect.y + rect.height) > pt.y;
}

void SDSScene::onDragFinish(const Common::Point &pt) {
	assert(_dragItem);

	debug(9, "Drag finished at %d, %d", pt.x , pt.y);
	// Unlike a click operation, this runs the drop event for *all* areas
	// and items, ignoring enable condition.

	GameItem *dragItem = _dragItem;

	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	const DragonGlobals *globals = static_cast<DragonGlobals *>(engine->getGameGlobals());

	runOps(dragItem->onDragFinishedOps, globals->getGameMinsToAddOnDragFinished());

	// TODO: Both these loops are very similar.. there should be a cleaner way.

	for (const auto &item : engine->getGDSScene()->getGameItems()) {
		if (item._inSceneNum == _num && _isInRect(pt, item._rect)) {
			debug("Dragged item %d onto item %d @ (%d, %d)", dragItem->_num, item._num, pt.x, pt.y);
			for (const auto &i : engine->getGDSScene()->getObjInteractions2()) {
				if (i.matches(dragItem->_num, item._num)) {
					debug(" --> exec %d drag ops for item %d", i.opList.size(), item._num);
					if (!runOps(i.opList, globals->getGameMinsToAddOnObjInteraction()))
						return;
					break;
				}
			}
		}
	}

	for (const auto &area : _hotAreaList) {
		if (!_isInRect(pt, area._rect))
			continue;

		if (area._num == 0) {
			debug("Item %d dropped on inventory.", dragItem->_num);
			dragItem->_inSceneNum = 2;
		} else {
			debug("Dragged item %d onto area %d @ (%d, %d)", dragItem->_num, area._num, pt.x, pt.y);
			for (const auto &i : engine->getScene()->getObjInteractions1()) {
				if (i.matches(dragItem->_num, area._num)) {
					debug(" --> exec %d drag ops for area %d", i.opList.size(), area._num);
					if (!runOps(i.opList, globals->getGameMinsToAddOnObjInteraction()))
						return;
					break;
				}
			}
		}
	}

	engine->setMouseCursor(0);
	_dragItem = nullptr;
}


void SDSScene::mouseRUp(const Common::Point &pt) {
	Dialog *dlg = getVisibleDialog();
	if (dlg) {
		// HACK: Check for dialog action selection! for now, just close
		// it here to make game playable.
		dlg->clear();
		return;
	}

	const HotArea *area = findAreaUnderMouse(pt);
	if (!area)
		return;

	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);

	if (area->_num == 0) {
		debug("Right mouseup on inventory.");
		engine->getInventory()->setShowZoomBox(true);
		engine->getInventory()->open();
		return;
	}

	int16 addmins = static_cast<DragonGlobals *>(engine->getGameGlobals())->getGameMinsToAddOnLClick();
	runOps(area->onRClickOps, addmins);
}

Dialog *SDSScene::getVisibleDialog() {
	for (auto &dlg : _dialogs) {
		if (dlg.hasFlag(kDlgFlagVisible) && !dlg.hasFlag(kDlgFlagOpening)) {
			return &dlg;
		}
	}
	return nullptr;
}

bool SDSScene::hasVisibleDialog() {
	return getVisibleDialog() != nullptr;
}

bool SDSScene::hasVisibleOrOpeningDialog() const {
	for (const auto &dlg : _dialogs) {
		if (dlg.hasFlag(kDlgFlagVisible) || dlg.hasFlag(kDlgFlagOpening)) {
			return true;
		}
	}
	return false;
}

HotArea *SDSScene::findAreaUnderMouse(const Common::Point &pt) {
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);

	for (auto &item : engine->getGDSScene()->getGameItems()) {
		if (item._inSceneNum == _num && checkConditions(item.enableConditions)
			&& _isInRect(pt, item._rect)) {
			return &item;
		}
	}

	for (auto &area : _hotAreaList) {
		if (checkConditions(area.enableConditions) && _isInRect(pt, area._rect)) {
			return &area;
		}
	}
	return nullptr;
}

void SDSScene::addInvButtonToHotAreaList() {
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	const Common::Array<MouseCursor> &cursors = engine->getGDSScene()->getCursorList();
	const Common::SharedPtr<Image> &icons = engine->getIcons();

	if (cursors.empty() || !icons || icons->loadedFrameCount() <= 2 || _num == 2)
		return;

	if (_hotAreaList.size() && _hotAreaList.front()._num == 0)
		return;

	HotArea area;
	area._num = 0;
	area._cursorNum = 0;
	area._rect.width = icons->width(2);
	area._rect.height = icons->height(2);
	area._rect.x = SCREEN_WIDTH - area._rect.width;
	area._rect.y = SCREEN_HEIGHT - area._rect.height;

	_hotAreaList.push_front(area);
}

void SDSScene::removeInvButtonFromHotAreaList() {
	if (_hotAreaList.size() && _hotAreaList.front()._num == 0)
		_hotAreaList.pop_front();
}

Common::Error SDSScene::syncState(Common::Serializer &s) {
	// num should be synced as part of the engine -
	// at this point we are already loaded.
	assert(_num);

	// The dialogs and triggers are stateful, everthing else is stateless.
	uint16 ndlgs = _dialogs.size();
	s.syncAsUint16LE(ndlgs);
	if (ndlgs != _dialogs.size()) {
		error("Dialog count in save doesn't match count in game (%d vs %d)",
				ndlgs, _dialogs.size());
	}
	for (auto &dlg : _dialogs) {
		dlg.syncState(s);
	}

	uint16 ntrig = _triggers.size();
	s.syncAsUint16LE(ntrig);
	if (ntrig != _triggers.size()) {
		error("Trigger count in save doesn't match count in game (%d vs %d)",
				ntrig, _triggers.size());
	}
	for (auto &trg : _triggers)
		s.syncAsByte(trg._enabled);

	return Common::kNoError;
}

GDSScene::GDSScene() : _field38(0), _field3a(0), _field3c(0), _field3e(0), _field40(0) {
}

bool GDSScene::load(const Common::String &filename, ResourceManager *resourceManager, Decompressor *decompressor) {
	Common::SeekableReadStream *sceneFile = resourceManager->getResource(filename);
	if (!sceneFile)
		error("Scene file %s not found", filename.c_str());

	DgdsChunkReader chunk(sceneFile);

	bool result = false;

	while (chunk.readNextHeader(EX_GDS, filename)) {
		if (chunk.isContainer()) {
			continue;
		}

		chunk.readContent(decompressor);
		Common::SeekableReadStream *stream = chunk.getContent();

		if (chunk.isSection(ID_GDS)) {
			// do nothing, this is the container.
			assert(chunk.isContainer());
		} else if (chunk.isSection(ID_INF)) {
			result = parseInf(stream);
		} else if (chunk.isSection(ID_SDS)) {
			result = parse(stream);
		}
	}

	initIconSizes();

	delete sceneFile;

	return result;
}

bool GDSScene::loadRestart(const Common::String &filename, ResourceManager *resourceManager, Decompressor *decompressor) {
	Common::SeekableReadStream *file = resourceManager->getResource(filename);
	if (!file)
		error("Restart data %s not found", filename.c_str());

	uint32 magic = file->readUint32LE();
	if (magic != _magic)
		error("Restart file magic doesn't match (%04X vs %04X)", magic, _magic);

	uint16 num = file->readUint16LE();
	// Find matching game item and load its values
	while (num) {
		bool found = false;
		for (GameItem &item : _gameItems) {
			if (item._num == num) {
				item._rect.x = file->readUint16LE();
				item._rect.y = file->readUint16LE();
				item._rect.width = file->readUint16LE();
				item._rect.height = file->readUint16LE();
				item._inSceneNum = file->readUint16LE();
				item._flags = file->readUint16LE();
				item._quality = file->readUint16LE();
				found = true;
				break;
			}
		}
		if (!found)
			error("Reset file references unknown item %d", num);
		num = file->readUint16LE();
	}
	initIconSizes();

	num = file->readUint16LE();
	while (num) {
		uint16 scene = file->readUint16LE();
		int16 val = file->readSint16LE();
		bool found = false;
		for (PerSceneGlobal &glob : _perSceneGlobals) {
			if (glob.matches(num, scene)) {
				glob._val = val;
				found = true;
				break;
			}
		}
		if (!found)
			error("Reset file references unknown global %d", num);
		num = file->readUint16LE();
	}

	/*uint32 unk = */ file->readUint32LE();

	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	Common::Array<Global *> &globs = engine->getGameGlobals()->getAllGlobals();

	if (globs.size() > 50)
		error("Too many globals to load from RST file");

	int g = 0;
	for (Global *glob : globs) {
		int16 val = file->readUint16LE();
		glob->setRaw(val);
		g++;
	}

	// Always 50 int16s worth of globals in the file, skip any unused.
	if (g < 50)
		file->skip(2 * (50 - g));

	uint16 triggers[100];
	for (int i = 0; i < ARRAYSIZE(triggers); i++) {
		triggers[i] = file->readUint16LE();
	}

	engine->_compositionBuffer.fillRect(Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 0);
	// TODO: FIXME: What should this scene num be? For now hacked to work with Dragon.
	engine->changeScene(3);
	SDSScene *scene = engine->getScene();
	int t = 0;
	num = triggers[t++];
	while (num) {
		uint16 val = triggers[t++];
		scene->enableTrigger(num, (bool)val);
		num = triggers[t++];
	}

	return true;
}

void GDSScene::initIconSizes() {
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	const Common::SharedPtr<Image> icons = engine->getIcons();
	uint16 nicons = icons ? icons->getFrames().size() : 0;
	for (GameItem &item : _gameItems) {
		if (item._iconNum < nicons) {
			item._rect.width = icons->getFrames()[item._iconNum]->w;
			item._rect.height = icons->getFrames()[item._iconNum]->h;
		} else {
			item._rect.width = 32;
			item._rect.height = 32;
		}
	}
}

bool GDSScene::readPerSceneGlobals(Common::SeekableReadStream *s) {
	uint16 numGlobals = s->readUint16LE();
	for (uint16 i = 0; i < numGlobals; i++) {
		_perSceneGlobals.push_back(PerSceneGlobal(s->readUint16LE(), s->readUint16LE()));
		_perSceneGlobals.back()._val = s->readSint16LE();
	}
	return !s->err();
}

bool GDSScene::parseInf(Common::SeekableReadStream *s) {
	_magic = s->readUint32LE();
	_version = s->readString();
	return !s->err();
}

bool GDSScene::parse(Common::SeekableReadStream *stream) {
	readOpList(stream, _startGameOps);
	readOpList(stream, _quitGameOps);
	if (isVersionOver(" 1.206"))
		readOpList(stream, _preTickOps);
	readOpList(stream, _postTickOps);
	if (isVersionOver(" 1.208"))
		readOpList(stream, _onChangeSceneOps);
	readPerSceneGlobals(stream);
	_iconFile = stream->readString();
	readMouseHotspotList(stream, _cursorList);
	readGameItemList(stream, _gameItems);
	readObjInteractionList(stream, _objInteractions2);
	if (isVersionOver(" 1.205"))
		readObjInteractionList(stream, _objInteractions1);

	if (isVersionOver(" 1.218")) {
		_field38 = stream->readUint16LE();
		_field3a = stream->readUint16LE();
		_field3c = stream->readUint16LE();
		_field3e = stream->readUint16LE();
		_field40 = stream->readUint16LE();
	} else {
		_field38 = 0;
		_field3a = 1;
		_field3c = 2;
		_field3e = 0;
		_field40 = 6;
	}

	return !stream->err();
}

Common::String GDSScene::dump(const Common::String &indent) const {
	Common::String str = Common::String::format("%sGDSScene<icons %s", indent.c_str(), _iconFile.c_str());
	str += _dumpStructList(indent, "gameItems", _gameItems);
	str += _dumpStructList(indent, "startGameOps", _startGameOps);
	str += _dumpStructList(indent, "quitGameOps", _quitGameOps);
	str += _dumpStructList(indent, "preTickOps", _preTickOps);
	str += _dumpStructList(indent, "postTickOps", _postTickOps);
	str += _dumpStructList(indent, "onChangeSceneOps", _onChangeSceneOps);
	str += _dumpStructList(indent, "perSceneGlobals", _perSceneGlobals);
	str += _dumpStructList(indent, "objInteractions1", _objInteractions1);
	str += _dumpStructList(indent, "objInteractions2", _objInteractions2);

	str += "\n";
	str += indent + ">";
	return str;
}

void GDSScene::globalOps(const Common::Array<uint16> &args) {
	if (!args.size())
		error("GDSScene::globalOps: Empty arg list");

	// The arg list should be a first value giving the count of operations,
	// then 3 values for each op (num, opcode, val).
	uint nops = args.size() / 3;
	uint nops_in_args = args[0];
	if (args.size() != nops * 3 + 1 || nops != nops_in_args)
		error("GDSScene::globalOps: Op list should be length 3*n+1");

	for (uint i = 0; i < nops; i++) {
		uint16 num = args[i * 3 + 1];
		uint16 op  = args[i * 3 + 2];
		int16 val  = args[i * 3 + 3];

		// CHECK ME: The original uses a different function here, but the
		// result appears to be the same as just calling getGlobal?
		int16 num2 = getGlobal(num);

		// Op bit 3 on means use absolute val of val.
		// Off means val is another global to lookup
		if (op & 8)
			op = op & 0xfff7;
		else
			val = getGlobal((uint16)val);

		if (op == 1)
			val = num2 + val;
		else if (op == 6)
			val = (val == 0);
		else if (op == 5)
			val = num2 - val;

		setGlobal(num, val);
	}
}

int16 GDSScene::getGlobal(uint16 num) {
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	int curSceneNum = engine->getScene()->getNum();
	for (const auto &global : _perSceneGlobals) {
		if (global.matches(num, curSceneNum))
			return global._val;
	}
	Globals *gameGlobals = engine->getGameGlobals();
	return gameGlobals->getGlobal(num);
}

int16 GDSScene::setGlobal(uint16 num, int16 val) {
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	int curSceneNum = engine->getScene()->getNum();
	for (auto &global : _perSceneGlobals) {
		if (global.matches(num, curSceneNum)) {
			global._val = val;
			return val;
		}
	}
	Globals *gameGlobals = engine->getGameGlobals();
	return gameGlobals->setGlobal(num, val);
}

void GDSScene::drawItems(Graphics::ManagedSurface &surf) {
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	const Common::SharedPtr<Image> &icons = engine->getIcons();
	int currentScene = engine->getScene()->getNum();
	if (!icons || icons->loadedFrameCount() < 3)
		return;

	int xoff = 20;
	const Common::Rect screenWin(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	// Don't overlap the inventory icon.
	const int maxx = SCREEN_WIDTH - (icons->width(2) + 10);
	for (auto &item : _gameItems) {
		if (item._inSceneNum == currentScene && &item != engine->getScene()->getDragItem()) {
			if (!(item._flags & 1)) {
				// Dropped item.
				// Update the rect for the icon - Note: original doesn't do this,
				// but then the napent icon is offset??
				/*Common::SharedPtr<Graphics::ManagedSurface> icon = icons->getSurface(item._iconNum);
				if (icon) {
					item._rect.width = MIN((int)icon->w, item._rect.width);
					item._rect.height = MIN((int)icon->h, item._rect.height);
				}*/
				if (xoff + item._rect.width > maxx)
					xoff = 20;
				int yoff = SCREEN_HEIGHT - (item._rect.height + 2);
				item._rect.x = xoff;
				item._rect.y = yoff;
				icons->drawBitmap(item._iconNum, xoff, yoff, screenWin, surf);
				xoff += (item._rect.width + 6);
			} else {
				icons->drawBitmap(item._iconNum, item._rect.x, item._rect.y, screenWin, surf);
			}
		}
	}
}

int GDSScene::countItemsInScene2() const {
	int result = 0;
	for (const auto &item : _gameItems) {
		if (item._inSceneNum == 2)
			result++;
	}
	return result;
}

Common::Error GDSScene::syncState(Common::Serializer &s) {
	// Only items and globals are stateful - everything else is stateless.
	// Game should already be loaded at this point so the lsits are already
	// filled out.

	assert(!_gameItems.empty());
	assert(!_perSceneGlobals.empty());

	// TODO: Maybe it would be nicer to save the item/global numbers
	// with the values in case the order changed in some other version of
	// the game data?  This assumes they will be the same order.

	uint16 nitems = _gameItems.size();
	s.syncAsUint16LE(nitems);
	if (nitems != _gameItems.size()) {
		error("Item count in save doesn't match count in game (%d vs %d)",
				nitems, _gameItems.size());
	}
	for (GameItem &item : _gameItems) {
		s.syncAsUint16LE(item._inSceneNum);
		if (s.getVersion() > 1)
			s.syncAsUint16LE(item._flags);
		s.syncAsUint16LE(item._quality);
		//debug("loaded item: %d %d %d %d", item._num, item._inSceneNum, item._flags, item._quality);
	}

	uint16 nglobals = _perSceneGlobals.size();
	s.syncAsUint16LE(nglobals);
	if (nglobals != _perSceneGlobals.size()) {
		error("Scene global count in save doesn't match count in game (%d vs %d)",
				nglobals, _perSceneGlobals.size());
	}
	for (PerSceneGlobal &glob : _perSceneGlobals) {
		s.syncAsUint16LE(glob._val);
	}

	return Common::kNoError;
}

} // End of namespace Dgds
