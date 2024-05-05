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
#include "graphics/cursorman.h"

#include "graphics/surface.h"
#include "graphics/primitives.h"

#include "dgds/dgds.h"
#include "dgds/includes.h"
#include "dgds/resource.h"
#include "dgds/request.h"
#include "dgds/scene.h"
#include "dgds/scripts.h"
#include "dgds/font.h"
#include "dgds/globals.h"
#include "dgds/image.h"
#include "dgds/inventory.h"

namespace Dgds {

template<class S> Common::String _dumpStructList(const Common::String &indent, const Common::String &name, const Common::Array<S> &list) {
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
	Common::String str = Common::String::format("%sHotArea<%s num %d cursor %d",
			indent.c_str(), rect.dump("").c_str(), _num, _cursorNum);
	str += _dumpStructList(indent, "enableConditions", enableConditions);
	str += _dumpStructList(indent, "onRClickOps", onRClickOps);
	str += _dumpStructList(indent, "opList2", opList2);
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
	case kSceneOpSegmentStateOps:   return "sceneOpSegmentStateOps";
	case kSceneOpSetItemAttr:   return "setItemAttr";
	case kSceneOpSetDragItem:   return "setDragItem";
	case kSceneOpOpenInventory: return "openInventory";
	case kSceneOpShowDlg:		return "showdlg";
	case kSceneOpEnableTrigger: return "enabletrigger";
	case kSceneOpChangeSceneToStored: return "changeSceneToStored";
	case kSceneOpMoveItemsBetweenScenes: return "moveItemsBetweenScenes";
	case kSceneOpLeaveSceneAndOpenInventory:   return "leaveSceneOpenInventory";
	case kSceneOpShowClock:		return "sceneOpShowClock";
	case kSceneOpHideClock:		return "sceneOpHideClock";
	case kSceneOpShowMouse:		return "sceneOpShowMouse";
	case kSceneOpHideMouse:		return "sceneOpHideMouse";
	case kSceneOpMeanwhile:   	return "meanwhile";
	case kSceneOpOpenPlaySkipIntroMenu: return "openPlaySkipIntroMovie";
	case kSceneOpShowInvButton:	return "showInvButton";
	case kSceneOpHideInvButton:	return "hideInvButton";
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
	str += _dumpStructList(indent, "opList4", opList4);
	str += _dumpStructList(indent, "opList5", opList5);
	str += "\n";
	str += indent + ">";
	return str;
}


Common::String MouseCursor::dump(const Common::String &indent) const {
	return Common::String::format("%sMouseCursor<%d %d>", indent.c_str(), _hotX, _hotY);
}


Common::String ObjectInteraction::dump(const Common::String &indent) const {
	Common::String str = Common::String::format("%sObjectInteraction<dropped %d target %d", indent.c_str(), _droppedItemNum, _targetItemNum);

	str += _dumpStructList(indent, "opList", opList);
	str += "\n";
	str += indent + ">";
	return str;
}


Common::String SceneTrigger::dump(const Common::String &indent) const {
	Common::String str = Common::String::format("%sSceneTrigger<num %d %s", indent.c_str(), _num, _enabled ? "enabled" : "disabled");
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

Scene::Scene() : _magic(0) {
}

bool Scene::isVersionOver(const char *version) const {
	return strncmp(_version.c_str(), version, _version.size()) > 0;
}

bool Scene::isVersionUnder(const char *version) const {
	return strncmp(_version.c_str(), version, _version.size()) < 0;
}


bool Scene::readConditionList(Common::SeekableReadStream *s, Common::Array<SceneConditions> &list) const {
	list.resize(s->readUint16LE());
	for (SceneConditions &dst : list) {
		dst._num = s->readUint16LE();
		dst._flags = static_cast<SceneCondition>(s->readUint16LE());
		dst._val = s->readUint16LE();
	}
	return !s->err();
}


bool Scene::readHotArea(Common::SeekableReadStream *s, HotArea &dst) const {
	dst.rect.x = s->readUint16LE();
	dst.rect.y = s->readUint16LE();
	dst.rect.width = s->readUint16LE();
	dst.rect.height = s->readUint16LE();
	dst._num = s->readUint16LE();
	dst._cursorNum = s->readUint16LE();
	readConditionList(s, dst.enableConditions);
	readOpList(s, dst.onRClickOps);
	readOpList(s, dst.opList2);
	readOpList(s, dst.onLClickOps);
	return !s->err();
}


bool Scene::readHotAreaList(Common::SeekableReadStream *s, Common::Array<HotArea> &list) const {
	list.resize(s->readUint16LE());
	for (HotArea &dst : list) {
		readHotArea(s, dst);
	}
	return !s->err();
}


bool Scene::readGameItemList(Common::SeekableReadStream *s, Common::Array<GameItem> &list) const {
	list.resize(s->readUint16LE());
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
			readOpList(s, dst.opList4);
			readOpList(s, dst.opList5);
		}
	}
	return !s->err();
}


bool Scene::readMouseHotspotList(Common::SeekableReadStream *s, Common::Array<MouseCursor> &list) const {
	list.resize(s->readUint16LE());
	for (MouseCursor &dst : list) {
		dst._hotX = s->readUint16LE();
		dst._hotY = s->readUint16LE();
	}
	return !s->err();
}


bool Scene::readObjInteractionList(Common::SeekableReadStream *s, Common::Array<ObjectInteraction> &list) const {
	list.resize(s->readUint16LE());
	for (ObjectInteraction &dst : list) {
		if (!isVersionOver(" 1.205")) {
			dst._targetItemNum = s->readUint16LE();
			dst._droppedItemNum = s->readUint16LE();
			dst._targetItemNum += s->readUint16LE();
		} else {
			dst._droppedItemNum = s->readUint16LE();
			dst._targetItemNum = s->readUint16LE();
		}
		readOpList(s, dst.opList);
	}
	return !s->err();
}


bool Scene::readOpList(Common::SeekableReadStream *s, Common::Array<SceneOp> &list) const {
	list.resize(s->readUint16LE());
	for (SceneOp &dst : list) {
		readConditionList(s, dst._conditionList);
		dst._opCode = static_cast<SceneOpCode>(s->readUint16LE());
		int nvals = s->readUint16LE();
		for (int i = 0; i < nvals / 2; i++) {
			dst._args.push_back(s->readUint16LE());
		}
	}

	return !s->err();
}


bool Scene::readDialogList(Common::SeekableReadStream *s, Common::Array<Dialog> &list) const {
	// Some data on this format here https://www.oldgamesitalia.net/forum/index.php?showtopic=24055&st=25&p=359214&#entry359214

	list.resize(s->readUint16LE());
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
	}

	return !s->err();
}


bool Scene::readTriggerList(Common::SeekableReadStream *s, Common::Array<SceneTrigger> &list) const {
	list.resize(s->readUint16LE());
	for (SceneTrigger &dst : list) {
		dst._num = s->readUint16LE();
		dst._enabled = false;
		readConditionList(s, dst.conditionList);
		readOpList(s, dst.sceneOpList);
	}

	return !s->err();
}


bool Scene::readDialogActionList(Common::SeekableReadStream *s, Common::Array<DialogAction> &list) const {
	list.resize(s->readUint16LE());

	if (!list.empty())
		list[0].val = 1;

	for (DialogAction &dst : list) {
		dst.strStart = s->readUint16LE();
		dst.strEnd = s->readUint16LE();
		readOpList(s, dst.sceneOpList);
	}

	return !s->err();
}


void Scene::setItemAttrOp(const Common::Array<uint16> &args) {
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	if (args.size() < 3)
		error("Expect 3 args for item attr opcode.");

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
		item.rect.x = lastMouse.x;
		item.rect.y = lastMouse.y;
		// TODO: Update hot x/y here ?
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

bool Scene::runOps(const Common::Array<SceneOp> &ops) {
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	for (const SceneOp &op : ops) {
		if (!checkConditions(op._conditionList))
			continue;
		debug(10, "Exec %s", op.dump("").c_str());
		switch(op._opCode) {
		case kSceneOpChangeScene:
			if (engine->changeScene(op._args[0], true))
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
		case kSceneOpEnableTrigger:
			enableTrigger(op._args[0]);
			break;
		case kSceneOpChangeSceneToStored: {
			uint16 sceneNo = engine->getGameGlobals()->getGlobal(0x61);
			if (engine->changeScene(sceneNo, true))
				// This probably reset the list - stop now.
				return false;
			break;
		}
		case kSceneOpLeaveSceneAndOpenInventory:
			engine->getInventory()->open();
			// This implicitly changes scene num
			warning("TODO: Check leave scene and open inventory scene op");
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
		case kSceneOpMeanwhile:
			warning("TODO: Implement meanwhile screen");
			break;
		case kSceneOpShowInvButton:
			static_cast<DgdsEngine *>(g_engine)->getScene()->addInvButtonToHotAreaList();
			break;
		case kSceneOpHideInvButton:
			static_cast<DgdsEngine *>(g_engine)->getScene()->removeInvButtonFromHotAreaList();
			break;
		case kSceneOpOpenPlaySkipIntroMenu:
			warning("TODO: Implement scene op 107 (inject key code 0xfc, open menu to play intro or not)");
			break;
		default:
			warning("TODO: Implement scene op %d", op._opCode);
			break;
		}
	}
	return true;
}

bool Scene::checkConditions(const Common::Array<struct SceneConditions> &conds) {
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);

	uint cnum = 0;
	while (cnum < conds.size()) {
		const struct SceneConditions &c = conds[cnum];
		int16 refval = c._val;
		int16 checkval = -1;
		SceneCondition cflag = c._flags;
		// Two "or"s in a row, or an "or" at the start means true
		// (as one side is empty)
		if (cflag & kSceneCondOr)
			return true;

		if (cflag & kSceneCondSceneState) {
			refval = 1;
			checkval = engine->adsInterpreter()->getStateForSceneOp(c._num);
			SceneCondition equalOrNegate = static_cast<SceneCondition>(cflag & (kSceneCondEqual | kSceneCondNegate));
			if (equalOrNegate != kSceneCondEqual && equalOrNegate != kSceneCondNegate)
				refval = 0;
			cflag = kSceneCondEqual;
		} else if (cflag & kSceneCondNeedItemQuality || cflag & kSceneCondNeedItemSceneNum) {
			const Common::Array<GameItem> &items = engine->getGDSScene()->getGameItems();
			for (const auto &item : items) {
				if (item._num == c._num) {
					if (cflag & kSceneCondNeedItemSceneNum)
						checkval = item._inSceneNum;
					else // cflag & kSceneCondNeedItemField14
						checkval = item._quality;
					break;
				}
			}
		} else {
			checkval = engine->getGDSScene()->getGlobal(c._num);
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

		debug(10, "Cond: %s -> %s", c.dump("").c_str(), result ? "true": "false");

		if (!result) {
			// Skip to the next or, or the end.
			while (cnum < conds.size() && !(conds[cnum]._flags & kSceneCondOr))
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
	if (isVersionOver(" 1.216")) { // HoC
	//if (isVersionOver(" 1.224")) { // Beamish
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

	return !stream->err();
}

void SDSScene::unload() {
	_num = 0;
	_dragItem = nullptr;
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


void SDSScene::enableTrigger(uint16 num) {
	for (auto &trigger : _triggers) {
		if (trigger._num == num)
			trigger._enabled = true;
	}
}

void SDSScene::checkTriggers() {
	// scene can change on these triggers.  if that happens we stop.
	int startSceneNum = _num;

	for (struct SceneTrigger &trigger : _triggers) {
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
		if (dlg._state->_hideTime &&
			((dlg._state->_hideTime > timeNow && clearDlgFlag) || timeNow >= dlg._state->_hideTime)) {
			finished = true;
		}

		bool no_options = false;
		if ((dlg._state->_hideTime == 0) && dlg._action.size() < 2)
			no_options = true;

		if ((!finished && !no_options) || dlg.hasFlag(kDlgFlagHi20) || dlg.hasFlag(kDlgFlagHi40)) {
			if (!finished && dlg._action.size() > 1 && !dlg.hasFlag(kDlgFlagHiFinished)) {
				struct DialogAction *action = dlg.pickAction(false);
				if (dlg._state->_selectedAction != action) {
					dlg._state->_selectedAction = action;
					dlg.clearFlag(kDlgFlagHi10);
					dlg.setFlag(kDlgFlagRedrawSelectedActionChanged);
				}
			}
		} else {
			// this dialog is finished - call the ops and maybe show the next one
			_dlgWithFlagLo8IsClosing = dlg.hasFlag(kDlgFlagLo8);
			struct DialogAction *action = dlg.pickAction(true);
			if (action || dlg._action.empty()) {
				dlg.setFlag(kDlgFlagHiFinished);
				if (action) {
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
					dlg._state->_selectedAction = dlg.pickAction(false);
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
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);

	int16 cursorNum = (!dlg && area) ? area->_cursorNum : 0;
	if (_dragItem)
		cursorNum = _dragItem->_iconNum;

	engine->setMouseCursor(cursorNum);

	if (area && area->_num == 0) {
		// Object dragged over the inventory button
		// static_cast<DgdsEngine *>(g_engine)->getInventory()-> ...
	}
}

void SDSScene::mouseLDown(const Common::Point &pt) {
	Dialog *dlg = getVisibleDialog();
	if (dlg) {
		_shouldClearDlg = true;
		_ignoreMouseUp = true;
		return;
	}

	HotArea *area = findAreaUnderMouse(pt);
	if (!area)
		return;

	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	runOps(area->opList2);
	GameItem *item = dynamic_cast<GameItem *>(area);
	if (item) {
		_dragItem = item;
		if (item->_iconNum)
			engine->setMouseCursor(item->_iconNum);
	}
}

void SDSScene::mouseLUp(const Common::Point &pt) {
	if (_ignoreMouseUp) {
		_ignoreMouseUp = false;
		return;
	}

	const HotArea *area = findAreaUnderMouse(pt);
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);

	GameItem *dragItem = _dragItem;
	_dragItem = nullptr;

	if (dragItem && !area)
		engine->setMouseCursor(0);

	if (!area)
		return;

	engine->setMouseCursor(area->_cursorNum);

	if (area->_num == 0) {
		// dropped on the inventory button
		if (dragItem) {
			debug("Item %d dropped on inventory.", dragItem->_num);
			dragItem->_inSceneNum = 2;
			engine->setMouseCursor(0);
		} else {
			debug("Mouseup on inventory.");
			static_cast<DgdsEngine *>(g_engine)->getInventory()->open();
		}
	} else {
		if (dragItem) {
			const GameItem *targetItem = dynamic_cast<const GameItem *>(area);
			// Dropping one item on another -> use interactions from GDS
			// Dropping item on an area -> interactions are in SDS
			const Common::Array<struct ObjectInteraction> &interactions =
				targetItem ? engine->getGDSScene()->getObjInteractions2()
						: engine->getScene()->getObjInteractions1();
			for (const auto &i : interactions) {
				if (i._droppedItemNum == dragItem->_num && i._targetItemNum == area->_num) {
					runOps(i.opList);
					break;
				}
			}
		} else {
			runOps(area->onLClickOps);
		}
	}
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
	runOps(area->onRClickOps);
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

HotArea *SDSScene::findAreaUnderMouse(const Common::Point &pt) {
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);

	for (auto &item : engine->getGDSScene()->getGameItems()) {
		if (item._inSceneNum == _num && checkConditions(item.enableConditions) &&
			item.rect.x < pt.x && (item.rect.x + item.rect.width) > pt.x
			&& item.rect.y < pt.y && (item.rect.y + item.rect.height) > pt.y) {
			return &item;
		}
	}

	for (auto &area : _hotAreaList) {
		if (checkConditions(area.enableConditions) &&
			area.rect.x < pt.x && (area.rect.x + area.rect.width) > pt.x
			&& area.rect.y < pt.y && (area.rect.y + area.rect.height) > pt.y) {
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

	if (_hotAreaList.size() && _hotAreaList[0]._num == 0)
		return;

	HotArea area;
	area._num = 0;
	area._cursorNum = 0;
	area.rect.width = icons->width(2);
	area.rect.height = icons->height(2);
	area.rect.x = SCREEN_WIDTH - area.rect.width;
	area.rect.y = SCREEN_HEIGHT - area.rect.height;

	_hotAreaList.insert_at(0, area);
}

void SDSScene::removeInvButtonFromHotAreaList() {
	if (_hotAreaList.size() && _hotAreaList[0]._num == 0)
		_hotAreaList.remove_at(0);
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

GDSScene::GDSScene() {
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

	delete sceneFile;

	return result;
}

bool GDSScene::readPerSceneGlobals(Common::SeekableReadStream *s) {
	_perSceneGlobals.resize(s->readUint16LE());
	for (PerSceneGlobal &dst : _perSceneGlobals) {
		dst._num = s->readUint16LE();
		dst._sceneNo = s->readUint16LE();
		dst._val = s->readSint16LE();
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
		if (global._num == num && (global._sceneNo == 0 || global._sceneNo == curSceneNum))
			return global._val;
	}
	Globals *gameGlobals = engine->getGameGlobals();
	return gameGlobals->getGlobal(num);
}

int16 GDSScene::setGlobal(uint16 num, int16 val) {
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	int curSceneNum = engine->getScene()->getNum();
	for (auto &global : _perSceneGlobals) {
		if (global._num == num && (global._sceneNo == 0 || global._sceneNo == curSceneNum)) {
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
	if (icons->loadedFrameCount() < 3)
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
				Common::SharedPtr<Graphics::ManagedSurface> icon = icons->getSurface(item._iconNum);
				if (icon) {
					item.rect.width = MIN((int)icon->w, item.rect.width);
					item.rect.height = MIN((int)icon->h, item.rect.height);
				}
				if (xoff + item.rect.width > maxx)
					xoff = 20;
				int yoff = SCREEN_HEIGHT - (item.rect.height + 2);
				item.rect.x = xoff;
				item.rect.y = yoff;
				icons->drawBitmap(item._iconNum, xoff, yoff, screenWin, surf);
				xoff += (item.rect.width + 6);
			} else {
				icons->drawBitmap(item._iconNum, item.rect.x, item.rect.y, screenWin, surf);
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
	// with the values in case the order changed in some other version of the game data?  This assumes they will be
	// the same order.

	uint16 nitems = _gameItems.size();
	s.syncAsUint16LE(nitems);
	if (nitems != _gameItems.size()) {
		error("Item count in save doesn't match count in game (%d vs %d)",
				nitems, _gameItems.size());
	}
	for (auto &item : _gameItems) {
		s.syncAsUint16LE(item._inSceneNum);
		s.syncAsUint16LE(item._quality);
	}

	uint16 nglobals = _perSceneGlobals.size();
	s.syncAsUint16LE(nglobals);
	if (nglobals != _perSceneGlobals.size()) {
		error("Scene global count in save doesn't match count in game (%d vs %d)",
				nglobals, _perSceneGlobals.size());
	}
	for (auto &glob : _perSceneGlobals) {
		s.syncAsUint16LE(glob._val);
	}

	return Common::kNoError;
}

} // End of namespace Dgds

