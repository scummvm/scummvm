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

#include "common/debug.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/rect.h"
#include "common/system.h"
#include "common/util.h"

#include "graphics/surface.h"

#include "dgds/dgds.h"
#include "dgds/includes.h"
#include "dgds/resource.h"
#include "dgds/scene.h"
#include "dgds/ads.h"
#include "dgds/menu.h"
#include "dgds/globals.h"
#include "dgds/inventory.h"
#include "dgds/debug_util.h"
#include "dgds/game_palettes.h"

namespace Dgds {


Common::String HotArea::dump(const Common::String &indent) const {
	Common::String str = Common::String::format("%sHotArea<%s num %d cursor %d cursor2 %d interactionRectNum %d",
			indent.c_str(), _rect.dump("").c_str(), _num, _cursorNum, _cursorNum2, _objInteractionRectNum);
	str += DebugUtil::dumpStructList(indent, "enableConditions", enableConditions);
	str += DebugUtil::dumpStructList(indent, "onRClickOps", onRClickOps);
	str += DebugUtil::dumpStructList(indent, "onLDownOps", onLDownOps);
	str += DebugUtil::dumpStructList(indent, "onLClickOps", onLClickOps);
	str += "\n";
	str += indent + ">";
	return str;
}


Common::String GameItem::dump(const Common::String &indent) const {
	Common::String super = HotArea::dump(indent + "  ");

	Common::String str = Common::String::format(
			"%sGameItem<\n%s\n%saltCursor %d icon %d sceneNum %d flags %d quality %d",
			indent.c_str(), super.c_str(), indent.c_str(), _altCursor,
			_iconNum, _inSceneNum, _flags, _quality);
	str += DebugUtil::dumpStructList(indent, "onDragFinishedOps", onDragFinishedOps);
	str += DebugUtil::dumpStructList(indent, "onBothButtonsOps", onBothButtonsOps);
	str += "\n";
	str += indent + ">";
	return str;
}


Common::String MouseCursor::dump(const Common::String &indent) const {
	return Common::String::format("%sMouseCursor<%d %d>", indent.c_str(), _hot.x, _hot.y);
}


Common::String ObjectInteraction::dump(const Common::String &indent) const {
	Common::String str = Common::String::format("%sObjectInteraction<dropped %d target %d", indent.c_str(), _droppedItemNum, _targetItemNum);

	str += DebugUtil::dumpStructList(indent, "opList", opList);
	str += "\n";
	str += indent + ">";
	return str;
}


Common::String SceneTrigger::dump(const Common::String &indent) const {
	Common::String str = Common::String::format("%sSceneTrigger<num %d %s %d", indent.c_str(), _num, _enabled ? "enabled" : "disabled", _timesToCheckBeforeRunning);
	str += DebugUtil::dumpStructList(indent, "conditionList", conditionList);
	str += DebugUtil::dumpStructList(indent, "opList", sceneOpList);
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
		int16 val = s->readSint16LE();
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
		dst._cursorNum2 = s->readUint16LE();
	else
		dst._cursorNum2 = 0;

	if (isVersionOver(" 1.218")) {
		dst._objInteractionRectNum = s->readUint16LE();
		if (dst._objInteractionRectNum) {
			dst._rect = DgdsRect();
		}
	} else {
		dst._objInteractionRectNum = 0;
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
			readOpList(s, dst.onBothButtonsOps);
		}
	}
	return !s->err();
}


bool Scene::readMouseHotspotList(Common::SeekableReadStream *s, Common::Array<MouseCursor> &list) const {
	uint16 num = s->readUint16LE();
	uint16 hotX, hotY;
	_checkListNotTooLong(num, "mouse hotspots");

	for (uint16 i = 0; i < num; i++) {
		hotX = s->readUint16LE();
		hotY = s->readUint16LE();
		list.push_back(MouseCursor(hotX, hotY));
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
		if ((dst._opCode & ~kSceneOpHasConditionalOpsFlag) > kSceneOpMaxCode || dst._opCode == kSceneOpNone)
			error("Unexpected scene opcode %d", (int)dst._opCode);
		uint16 nvals = s->readUint16LE();
		_checkListNotTooLong(nvals, "scene op args");
		for (uint16 i = 0; i < nvals / 2; i++) {
			dst._args.push_back(s->readUint16LE());
		}
	}

	return !s->err();
}


bool Scene::readDialogList(Common::SeekableReadStream *s, Common::Array<Dialog> &list, int16 filenum /* = 0 */) const {
	// Some data on this format here https://www.oldgamesitalia.net/forum/index.php?showtopic=24055&st=25&p=359214&#entry359214

	uint16 nitems = s->readUint16LE();
	_checkListNotTooLong(nitems, "dialogs");
	uint startsize = list.size();
	list.resize(startsize + nitems);

	for (uint i = startsize; i < list.size(); i++) {
		Dialog &dst = list[i];
		dst._num = s->readUint16LE();
		dst._fileNum = filenum;
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
			dst._nextDialogFileNum = s->readUint16LE();
		} else {
			dst._nextDialogFileNum = 0;
		}
		if (isVersionOver(" 1.207")) {
			dst._nextDialogDlgNum = s->readUint16LE();
		}

		if (isVersionOver(" 1.216")) {
			dst._talkDataNum = s->readUint16LE();
			dst._talkDataHeadNum = s->readUint16LE();
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
	uint16 num = s->readUint16LE();
	_checkListNotTooLong(num, "triggers");
	for (uint16 i = 0; i < num; i++) {
		list.push_back(SceneTrigger(s->readUint16LE()));
		if (isVersionOver(" 1.219"))
			list.back()._timesToCheckBeforeRunning = s->readUint16LE();
		readConditionList(s, list.back().conditionList);
		readOpList(s, list.back().sceneOpList);
	}

	return !s->err();
}

bool Scene::readConditionalSceneOpList(Common::SeekableReadStream *s, Common::Array<ConditionalSceneOp> &list) const {
	uint16 num = s->readUint16LE();
	_checkListNotTooLong(num, "conditional scene ops");
	list.resize(num);

	for (ConditionalSceneOp &dst : list) {
		dst._opCode = static_cast<SceneOpCode>(s->readUint16LE());
		if (dst._opCode > kSceneOpMaxCode || dst._opCode == kSceneOpNone)
			error("Unexpected scene opcode %d", (int)dst._opCode);
		readConditionList(s, dst._conditionList);
		readOpList(s, dst._opList);
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

	for (uint i = 0; i < list.size(); i++) {
		list[i].num = i;
		list[i].strStart = s->readUint16LE();
		list[i].strEnd = s->readUint16LE();
		readOpList(s, list[i].sceneOpList);
	}

	return !s->err();
}


void Scene::setItemAttrOp(const Common::Array<uint16> &args) {
	if (args.size() < 3)
		error("Expect 3 args for item attr opcode.");

	DgdsEngine *engine = DgdsEngine::getInstance();
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
	DgdsEngine *engine = DgdsEngine::getInstance();

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
	ADSInterpreter *interp = DgdsEngine::getInstance()->adsInterpreter();

	for (uint i = 0; i < args.size(); i += 2) {
		uint16 subop = args[i];
		uint16 arg = args[i + 1];
		if (!subop && !arg)
			return;
		switch (subop) {
		case 1: // Restart
			interp->segmentOrState(arg, 3);
			break;
		case 2: // Start
			interp->segmentOrState(arg, 4);
			break;
		case 3: // Stop
			interp->segmentSetState(arg, 6);
			break;
		case 4: // Pause
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


//
// Note: ops list here is not a reference on purpose, it must be copied.
// The underlying list might be freed during execution if the scene changes, but
// we have to finish executing the list if the scene changed into inventory -
// which could have invalidated the op list pointer.  We *don't* finish executing
// if any other scene change happens.
//
// Because scene change can also invalidate the `this` pointer, this is static
// and the runOp functions fetch the scene through the engine.
//
/*static*/
bool Scene::runOps(const Common::Array<SceneOp> ops, int16 addMinuites /* = 0 */) {
	DgdsEngine *engine = DgdsEngine::getInstance();
	bool sceneChanged = false;
	int16 startSceneNum = engine->getScene()->getNum();
	for (const SceneOp &op : ops) {
		if (!SceneConditions::check(op._conditionList))
			continue;
		debug(10, "Exec %s", op.dump("").c_str());
		if (addMinuites) {
			engine->getClock().addGameTime(addMinuites);
			addMinuites = 0;
		}

		sceneChanged = op.runOp();

		if (sceneChanged)
			break;
	}

	//
	// The definition of "scene changed" returned by this function is slightly different -
	// for the purpose of continuing to run ops above, we ignore changes to scene 2 (the
	// inventory), but for the purpose of telling the caller, any change means they
	// need to stop as pointers are no longer valid.
	//
	int16 endSceneNum = engine->getScene()->getNum();
	return (startSceneNum == endSceneNum) && !sceneChanged;
}


bool SDSScene::_dlgWithFlagLo8IsClosing = false;
DialogFlags SDSScene::_sceneDialogFlags = kDlgFlagNone;

SDSScene::SDSScene() : _num(-1), _dragItem(nullptr), _shouldClearDlg(false), _ignoreMouseUp(false),
_field6_0x14(0), _rbuttonDown(false), _lbuttonDown(false), _lookMode(0) {
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
		readConditionalSceneOpList(stream, _conditionalOps);
	}

	return !stream->err();
}

void SDSScene::unload() {
	_num = 0;
	_lookMode = 0;
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
	_talkData.clear();
	_dynamicRects.clear();
	_conversation.unloadData();
	_conditionalOps.clear();
	_sceneDialogFlags = kDlgFlagNone;
}


Common::String SDSScene::dump(const Common::String &indent) const {
	Common::String str = Common::String::format("%sSDSScene<ver %s num %d %d ads %s", indent.c_str(), _version.c_str(), _num, _field6_0x14, _adsFile.c_str());
	str += DebugUtil::dumpStructList(indent, "enterSceneOps", _enterSceneOps);
	str += DebugUtil::dumpStructList(indent, "leaveSceneOps", _leaveSceneOps);
	str += DebugUtil::dumpStructList(indent, "preTickOps", _preTickOps);
	str += DebugUtil::dumpStructList(indent, "postTickOps", _postTickOps);
	str += DebugUtil::dumpStructList(indent, "hotAreaList", _hotAreaList);
	str += DebugUtil::dumpStructList(indent, "objInteractions1", _objInteractions1);
	str += DebugUtil::dumpStructList(indent, "objInteractions2", _objInteractions2);
	str += DebugUtil::dumpStructList(indent, "dialogues", _dialogs);
	str += DebugUtil::dumpStructList(indent, "triggers", _triggers);
	str += DebugUtil::dumpStructList(indent, "conditionalOps", _conditionalOps);

	str += "\n";
	str += indent + ">";
	return str;
}


void SDSScene::enableTrigger(uint16 sceneNum, uint16 num, bool enable /* = true */) {
	if (sceneNum && sceneNum != _num)
		return;

	for (auto &trigger : _triggers) {
		if (trigger.getNum() == num) {
			trigger._enabled = enable;
			if (enable)
				trigger._checksUntilRun = trigger._timesToCheckBeforeRunning;
			return;
		}
	}

	warning("enableTrigger: Trigger %d not found", num);
}

bool SDSScene::isTriggerEnabled(uint16 num) {
	for (auto &trigger : _triggers) {
		if (trigger.getNum() == num) {
			return trigger._enabled;
		}
	}

	warning("isTriggerEnabled: Trigger %d not found", num);
	return false;
}

void SDSScene::checkTriggers() {
	for (SceneTrigger &trigger : _triggers) {
		if (!trigger._enabled)
			continue;

		if (trigger._checksUntilRun) {
			trigger._checksUntilRun--;
			continue;
		}

		if (!SceneConditions::check(trigger.conditionList))
			continue;

		trigger._enabled = false;
		bool keepGoing = runOps(trigger.sceneOpList);

		// If the scene changed, the list is no longer valid. Abort!
		if (!keepGoing)
			return;
	}
}


Dialog *SDSScene::loadDialogData(uint16 num) {
	if (num == 0)
		return &_dialogs.front();

	for (auto &dlg: _dialogs)
		if (dlg._fileNum == num)
			return &dlg;

	const Common::String filename = Common::String::format("D%d.DDS", num);
	DgdsEngine *engine = DgdsEngine::getInstance();
	ResourceManager *resourceManager = engine->getResourceManager();
	Common::SeekableReadStream *dlgFile = resourceManager->getResource(filename);
	if (!dlgFile) {
		//
		// This happens for example if debug mode clicks have been enabled in
		// Willy Beamish, as the debug dialogs were not included in the retail
		// version.
		//
		warning("Dialog file %s not found", filename.c_str());
		return nullptr;
	}

	DgdsChunkReader chunk(dlgFile);
	Decompressor *decompressor = engine->getDecompressor();

	bool result = false;

	uint prevSize = _dialogs.size();

	while (chunk.readNextHeader(EX_DDS, filename)) {
		if (chunk.isContainer()) {
			continue;
		}

		chunk.readContent(decompressor);
		Common::SeekableReadStream *stream = chunk.getContent();

		if (chunk.isSection(ID_DDS)) {
			uint32 magic = stream->readUint32LE();
			if (magic != _magic)
				error("Dialog file magic mismatch %08x vs scene %08x", magic, _magic);
			Common::String fileVersion = stream->readString();
			Common::String fileId = stream->readString();
			// slight hack, set file version while loading
			Common::String oldVer = _version;
			_version = fileVersion;
			result = readDialogList(stream, _dialogs, num);
			_version = oldVer;
		}
	}

	delete dlgFile;

	if (_dialogs.size() != prevSize) {
		debug(10, "Read %d dialogs from DDS %s:", _dialogs.size() - prevSize, filename.c_str());
		for (uint i = prevSize; i < _dialogs.size(); i++)
			debug(10, "%s", _dialogs[i].dump("").c_str());
	}

	if (!result)
		return nullptr;

	for (auto &dlg : _dialogs) {
		if (dlg._nextDialogDlgNum && !dlg._nextDialogFileNum) {
			dlg._nextDialogFileNum = num;
		}
	}

	// TODO: Maybe not this?
	return &_dialogs.front();
}

void SDSScene::freeDialogData(uint16 num) {
	if (!num)
		return;

	for (int i = 0; i < (int)_dialogs.size(); i++) {
		if (_dialogs[i]._num == num) {
			_dialogs.remove_at(i);
			i--;
		}
	}
}

bool SDSScene::readTalkData(Common::SeekableReadStream *s, TalkData &dst) {
	dst._bmpFile = s->readString();

	uint16 nvals = s->readUint16LE();
	_checkListNotTooLong(nvals, "talk data");
	dst._heads.resize(nvals);
	for (auto &h : dst._heads) {
		h._num = s->readUint16LE();
		h._drawType = s->readUint16LE();
		h._drawCol = s->readUint16LE();
		h._rect.x = s->readUint16LE();
		h._rect.y = s->readUint16LE();
		h._rect.width = s->readUint16LE();
		h._rect.height = s->readUint16LE();
		if (isVersionOver(" 1.220")) {
			h._bmpFile = s->readString();
			if (!h._bmpFile.empty()) {
				DgdsEngine *engine = DgdsEngine::getInstance();
				ResourceManager *resMan = engine->getResourceManager();
				if (resMan->hasResource(h._bmpFile)) {
					h._shape.reset(new Image(resMan, engine->getDecompressor()));
					h._shape->loadBitmap(h._bmpFile);
				} else {
					warning("Couldn't load talkdata %d head %d BMP: %s", dst._num, h._num, h._bmpFile.c_str());
				}
			}
		}
		uint16 nsub = s->readUint16LE();
		_checkListNotTooLong(nsub, "talk head frames");
		h._headFrames.resize(nsub);
		for (auto &sub : h._headFrames) {
			sub._frameNo = s->readUint16LE();
			sub._xoff = s->readSint16LE();
			sub._yoff = s->readSint16LE();
			if (isVersionOver(" 1.221")) {
				sub._flipFlags = s->readUint16LE();
			}
		}
	}

	return true;
}

bool SDSScene::loadTalkData(uint16 num) {
	if (!num)
		return false;

	for (auto &talk : _talkData) {
		if (talk._num == num)
			return true;
	}

	const Common::String filename = Common::String::format("T%d.TDS", num);
	DgdsEngine *engine = DgdsEngine::getInstance();
	ResourceManager *resourceManager = engine->getResourceManager();
	Common::SeekableReadStream *dlgFile = resourceManager->getResource(filename);
	if (!dlgFile)
		error("Talk file %s not found", filename.c_str());

	DgdsChunkReader chunk(dlgFile);
	Decompressor *decompressor = engine->getDecompressor();

	bool result = false;

	while (chunk.readNextHeader(EX_TDS, filename)) {
		if (chunk.isContainer()) {
			continue;
		}

		chunk.readContent(decompressor);
		Common::SeekableReadStream *stream = chunk.getContent();

		if (chunk.isSection(ID_THD)) {
			uint32 magic = stream->readUint32LE();
			if (magic != _magic)
				error("Talk file magic mismatch %08x vs scene %08x", magic, _magic);
			Common::String fileVersion = stream->readString();
			Common::String fileId = stream->readString();
			// slight hack, set file version while loading
			Common::String oldVer = _version;
			_version = fileVersion;
			_talkData.insert_at(0, TalkData());
			result = readTalkData(stream, _talkData.front());
			_talkData.front()._num = num;
			_version = oldVer;

			const Common::String &bmpFile = _talkData.front()._bmpFile;
			if (!bmpFile.empty()) {
				if (resourceManager->hasResource(bmpFile)) {
					Image *img = new Image(resourceManager, decompressor);
					img->loadBitmap(bmpFile);
					_talkData.front()._shape.reset(img);
				} else {
					warning("Couldn't load talkdata %d head BMP: %s", num, bmpFile.c_str());
				}
			}
		}
	}

	delete dlgFile;

	return result;
}


bool SDSScene::freeTalkData(uint16 num) {
	bool result = false;
	for (int i = 0; i < (int)_talkData.size(); i++) {
		if (_talkData[i]._num == num) {
			_talkData.remove_at(i);
			i--;
			result = true;
		}
	}
	return result;
}

void SDSScene::updateVisibleTalkers() {
	for (auto &data : _talkData) {
		data.updateVisibleHeads();
	}
}

void SDSScene::drawVisibleHeads(Graphics::ManagedSurface *dst) {
	for (const auto &tds : _talkData) {
		tds.drawVisibleHeads(dst);
	}

	if (_conversation.isForDlg(getVisibleDialog())) {
		_conversation.runScript();
	}
}

bool SDSScene::hasVisibleHead() const {
	for (const auto &tds : _talkData) {
		if (tds.hasVisibleHead())
			return true;
	}
	return false;
}


bool SDSScene::loadTalkDataAndSetFlags(uint16 talknum, uint16 headnum) {
	updateVisibleTalkers();

	_conversation._drawRect = DgdsRect();
	if (loadTalkData(talknum)) {
		for (auto &data : _talkData) {
			if (data._num != talknum)
				continue;

			for (auto &head : data._heads) {
				if (head._num != headnum)
					continue;

				_conversation._drawRect = head._rect;
				head._flags = static_cast<HeadFlags>(head._flags & ~(kHeadFlag1 | kHeadFlag10));
				head._flags = static_cast<HeadFlags>(head._flags | (kHeadFlag8 | kHeadFlagVisible));
				break;
			}
			break;
		}
		return true;
	}
	return false;
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
		if (DgdsEngine::getInstance()->getGameLang() == Common::EN_ANY) {
			dlg._str = "Boy, am I tired.  Better get some sleep in about an hour.";
		} else if (DgdsEngine::getInstance()->getGameLang() == Common::DE_DEU) {
			dlg._str = "Mensch, bin ich m\x81""de!  Am Besten gehe ich bald mal ins Bett.";
		} else {
			error("Unsupported language %d", DgdsEngine::getInstance()->getGameLang());
		}

		_dialogs.push_back(dlg);
	}
	showDialog(0, TIRED_DLG_ID);
}


void SDSScene::showDialog(uint16 fileNum, uint16 dlgNum) {
	// TODO: In Willy Beamish, if the inventory button is visible here then
	// it should be hidden and a flag set to re-enabled it once the dialog
	// is closed.  Other games leave it visible.

	if (fileNum)
		loadDialogData(fileNum);

	for (auto &dialog : _dialogs) {
		if (dialog._num == dlgNum && fileNum == dialog._fileNum) {
			dialog.clearFlag(kDlgFlagHiFinished);
			dialog.clearFlag(kDlgFlagRedrawSelectedActionChanged);
			dialog.clearFlag(kDlgFlagHi10);
			//dialog.clearFlag(kDlgFlagHi20);
			dialog.clearFlag(kDlgFlagHi40);
			dialog.setFlag(kDlgFlagHi20);
			dialog.setFlag(kDlgFlagVisible);
			dialog.setFlag(kDlgFlagOpening);

			// For beamish
			bool haveHeadData = false;
			if (dialog._talkDataHeadNum) {
				haveHeadData = loadTalkDataAndSetFlags(dialog._talkDataNum, dialog._talkDataHeadNum);
			}

			_conversation.loadData(fileNum, dlgNum, -1, haveHeadData);

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
	uint32 timeNow = DgdsEngine::getInstance()->getThisFrameMs();
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

		// If voice acting in Willy Beamish is finished, clear the dialog
		// unless we are waiting for a choice.
		if (dlg._action.size() < 2 && (_conversation.isForDlg(&dlg) && _conversation.isFinished())) {
			finished = true;
			_conversation.clear();
		}

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

			// For Willy Beamish
			bool haveHeadData = false;
			if (dlg._talkDataNum) {
				haveHeadData = freeTalkData(dlg._talkDataNum);
			}

			DialogAction *action = dlg.pickAction(true, clearDlgFlag);
			if (action || dlg._action.empty()) {
				dlg.setFlag(kDlgFlagHiFinished);
				if (action) {
					// Play the response voice acting script.
					_conversation.loadData(dlg._fileNum, dlg._num, action->num, haveHeadData);
					_conversation.runScript();

					// Take a copy of the dialog because the actions might change the scene
					Dialog dlgCopy = dlg;
					if (dlgCopy._state)
						dlgCopy._state->_selectedAction = nullptr;
					debug(1, "Dialog %d closing: run action (%d ops)", dlg._num, action->sceneOpList.size());
					if (!runOps(action->sceneOpList)) {
						// HACK: the scene changed, but we haven't yet drawn the foreground for the
						// dialog, this is our last chance so do it now.  The game does it in a
						// different way that relies on delayed disposal of the dialog data.
						if (dlgCopy.hasFlag(kDlgFlagVisible) && !dlgCopy.hasFlag(kDlgFlagOpening)) {
							DgdsEngine *engine = DgdsEngine::getInstance();
							dlgCopy.draw(&engine->_compositionBuffer, kDlgDrawFindSelectionPointXY);
							dlgCopy.draw(&engine->_compositionBuffer, kDlgDrawFindSelectionTxtOffset);
							dlgCopy.draw(&engine->_compositionBuffer, kDlgDrawStageForeground);
						}
						_dlgWithFlagLo8IsClosing = false;
						return true;
					}
				}
			}

			if (dlg._nextDialogDlgNum) {
				dlg.setFlag(kDlgFlagHiFinished);
				showDialog(dlg._nextDialogFileNum, dlg._nextDialogDlgNum);
			} else {
				// No next dialog .. clear CDS data?
				//_conversation.unloadData();
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
	const DgdsEngine *engine = DgdsEngine::getInstance();
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

				dlg._state->_hideTime = DgdsEngine::getInstance()->getThisFrameMs() + time;
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

void SDSScene::mouseMoved(const Common::Point &pt) {
	Dialog *dlg = getVisibleDialog();
	const HotArea *area = findAreaUnderMouse(pt);
	DgdsEngine *engine = DgdsEngine::getInstance();

	int16 cursorNum = kDgdsMouseGameDefault;
	if (!dlg) {
		// Update mouse cursor if no dialog visible.
		// If lookMode is target (2) then activeItem will change it below.
		if (_lookMode)
			cursorNum = kDgdsMouseLook;
		if (area)
			cursorNum = _lookMode ? area->_cursorNum2 : area->_cursorNum;
	}

	GameItem *activeItem = engine->getGDSScene()->getActiveItem();

	if (_dragItem) {
		if (area && area->_objInteractionRectNum == 1) {
			// drag over Willy Beamish
			engine->getInventory()->open();
			return;
		}

		cursorNum = _dragItem->_iconNum;
	} else if (activeItem) {
		// In HOC or Dragon you need to hold down right button to get the
		// target cursor.  In Willy Beamish it is look mode 2 (target)
		if (_rbuttonDown || _lookMode == 2)
			cursorNum = activeItem->_altCursor;
	}

	engine->setMouseCursor(cursorNum);
}

void SDSScene::mouseLDown(const Common::Point &pt) {
	_lbuttonDown = true;
	if (hasVisibleDialog()) {
		debug(9, "Mouse LDown on at %d,%d clearing visible dialog", pt.x, pt.y);
		_shouldClearDlg = true;
		_ignoreMouseUp = true;
		return;
	}

	_ignoreMouseUp = false;

	// Don't start drag in look/target mode.
	if (_lookMode)
		return;

	HotArea *area = findAreaUnderMouse(pt);
	if (!area)
		return;

	debug(9, "Mouse LDown on area %d (%d,%d,%d,%d) cursor %d cursor2 %d. Run %d ops", area->_num,
			area->_rect.x, area->_rect.y, area->_rect.width, area->_rect.height,
			area->_cursorNum, area->_cursorNum2, area->onLDownOps.size());

	DgdsEngine *engine = DgdsEngine::getInstance();
	int16 addmins = engine->getGameGlobals()->getGameMinsToAddOnStartDrag();
	runOps(area->onLDownOps, addmins);
	GameItem *item = dynamic_cast<GameItem *>(area);
	if (item) {
		_dragItem = item;
		if (item->_iconNum)
			engine->setMouseCursor(item->_iconNum);
	}
}

static bool _isInRect(const Common::Point &pt, const DgdsRect rect) {
	return rect.x <= pt.x && (rect.x + rect.width) > pt.x
			&& rect.y <= pt.y && (rect.y + rect.height) > pt.y;
}

static const ObjectInteraction *_findInteraction(const Common::Array<ObjectInteraction> &interList, int16 droppedNum, uint16 targetNum) {
	for (const auto &i : interList) {
		if (i.matches(droppedNum, targetNum)) {
			return &i;
		}
	}
	return nullptr;
}

void SDSScene::mouseLUp(const Common::Point &pt) {
	_lbuttonDown = false;
	DgdsEngine *engine = DgdsEngine::getInstance();

	if (_ignoreMouseUp) {
		debug(9, "Ignoring mouseup at %d,%d as it was used to clear a dialog", pt.x, pt.y);
		_ignoreMouseUp = false;
		return;
	}

	if (_dragItem) {
		onDragFinish(pt);
		return;
	}

	if (_lookMode == 1) {
		rightButtonAction(pt);
		return;
	}

	const HotArea *area = findAreaUnderMouse(pt);
	if (!area)
		return;

	debug(9, "Mouse LUp on area %d (%d,%d,%d,%d) cursor %d cursor2 %d", area->_num, area->_rect.x, area->_rect.y,
		  area->_rect.width, area->_rect.height, area->_cursorNum, area->_cursorNum2);

	if (!_rbuttonDown)
		engine->setMouseCursor(area->_cursorNum);

	GDSScene *gds = engine->getGDSScene();

	if (area->_num == 0 || area->_objInteractionRectNum == 1) {
		debug(1, "Mouseup on inventory.");
		engine->getInventory()->open();
	} else if (area->_num == 0xffff) {
		debug(1, "Mouseup on swap characters.");
		bool haveInvBtn = _hotAreaList.size() && _hotAreaList.front()._num == 0;
		if (haveInvBtn)
			removeInvButtonFromHotAreaList();
		int16 prevChar = gds->getGlobal(0x33);
		gds->setGlobal(0x33, gds->getGlobal(0x34));
		gds->setGlobal(0x34, prevChar);
		if (haveInvBtn)
			addInvButtonToHotAreaList();
	} else {
		const GameItem *activeItem = engine->getGDSScene()->getActiveItem();
		if (activeItem && (_rbuttonDown || _lookMode == 2)) {
			debug(1, " --> exec both-button click ops for area %d", area->_num);
			// A both-button-click event, find the interaction list.
			if (!runOps(activeItem->onBothButtonsOps))
				return;

			const GameItem *destItem = dynamic_cast<const GameItem *>(area);
			const ObjectInteraction *i;
			if (destItem) {
				i =_findInteraction(gds->getObjInteractions2(), activeItem->_num, area->_num);
			} else {
				i = _findInteraction(_objInteractions2, activeItem->_num, area->_num);
			}
			if (i) {
				debug(1, " --> exec %d both-click ops for item combo %d", i->opList.size(), activeItem->_num);
				if (!runOps(i->opList, engine->getGameGlobals()->getGameMinsToAddOnObjInteraction()))
					return;
			}
		} else {
			debug(1, " --> exec %d click ops for area %d", area->onLClickOps.size(), area->_num);
			int16 addmins = engine->getGameGlobals()->getGameMinsToAddOnLClick();
			runOps(area->onLClickOps, addmins);
		}
	}
}

void SDSScene::onDragFinish(const Common::Point &pt) {
	assert(_dragItem);

	debug(9, "Drag finished at %d, %d", pt.x , pt.y);
	// Unlike a click operation, this runs the drop event for *all* areas
	// and items, ignoring enable condition.

	GameItem *dragItem = _dragItem;

	DgdsEngine *engine = DgdsEngine::getInstance();
	Globals *globals = engine->getGameGlobals();
	GDSScene *gdsScene = engine->getGDSScene();
	int16 dropSceneNum = _num;

	if (engine->getGameId() == GID_WILLY) {
		static_cast<WillyGlobals *>(globals)->setDroppedItemNum(dragItem->_num);
		if (engine->getInventory()->isOpen())
			dropSceneNum = 2;
	}

	runOps(dragItem->onDragFinishedOps, globals->getGameMinsToAddOnDragFinished());

	// TODO: Both these loops are very similar.. there should be a cleaner way.

	for (const auto &item : gdsScene->getGameItems()) {
		if (item._inSceneNum == dropSceneNum && _isInRect(pt, item._rect)) {
			debug(1, "Dragged item %d onto item %d @ (%d, %d)", dragItem->_num, item._num, pt.x, pt.y);
			const ObjectInteraction *i = _findInteraction(gdsScene->getObjInteractions1(), dragItem->_num, item._num);
			if (i) {
				debug(1, " --> exec %d drag ops for item %d", i->opList.size(), item._num);
				if (!runOps(i->opList, globals->getGameMinsToAddOnObjInteraction()))
					return;
			}
		}
	}

	const SDSScene *scene = engine->getScene();
	for (const auto &area : _hotAreaList) {
		if (!_isInRect(pt, area._rect))
			continue;

		if (area._num == 0) {
			debug(1, "Item %d dropped on inventory.", dragItem->_num);
			dragItem->_inSceneNum = 2;
			if (engine->getGameId() == GID_HOC)
				dragItem->_quality = Inventory::HOC_CHARACTER_QUALS[gdsScene->getGlobal(0x33)];

			const ObjectInteraction *i = _findInteraction(gdsScene->getObjInteractions1(), dragItem->_num, 0xffff);
			if (i) {
				debug(1, " --> exec %d drag ops for area %d", i->opList.size(), 0xffff);
				if (!runOps(i->opList, globals->getGameMinsToAddOnObjInteraction()))
					return;
			}
		} else if (area._num == 0xffff) {
			debug(1, "Item %d dropped on other character button.", dragItem->_num);
			dragItem->_inSceneNum = 2;
			if (engine->getGameId() == GID_HOC)
				dragItem->_quality = Inventory::HOC_CHARACTER_QUALS[gdsScene->getGlobal(0x34)];

			const ObjectInteraction *i = _findInteraction(gdsScene->getObjInteractions1(), dragItem->_num, 0xffff);
			if (i) {
				debug(1, " --> exec %d drag ops for area %d", i->opList.size(), 0xffff);
				if (!runOps(i->opList, globals->getGameMinsToAddOnObjInteraction()))
					return;
			}
		} else {
			debug(1, "Dragged item %d onto area %d @ (%d, %d)", dragItem->_num, area._num, pt.x, pt.y);
			const ObjectInteraction *i = _findInteraction(scene->getObjInteractions1(), dragItem->_num, area._num);
			if (i) {
				debug(1, " --> exec %d drag ops for area %d", i->opList.size(), area._num);
				if (!runOps(i->opList, globals->getGameMinsToAddOnObjInteraction()))
					return;
			}
		}
	}

	engine->setMouseCursor(kDgdsMouseGameDefault);
	_dragItem = nullptr;
}

void SDSScene::mouseRDown(const Common::Point &pt) {
	_rbuttonDown = true;
}

void SDSScene::mouseRUp(const Common::Point &pt) {
	_rbuttonDown = false;
	Dialog *dlg = getVisibleDialog();
	if (dlg) {
		// HACK: Check for dialog action selection! for now, just close
		// it here to make game playable.
		dlg->clear();
		return;
	}

	DgdsEngine *engine = DgdsEngine::getInstance();
	if (engine->getGameId() == GID_WILLY) {
		// Willy toggles between look/act/target mode on right click
		if (engine->getGDSScene()->getActiveItem()) {
			_lookMode++;
			if (_lookMode > 2)
				_lookMode = 0;
		} else {
			_lookMode = !_lookMode;
		}
		mouseMoved(pt);
	} else {
		// Other games do right-button action straight away.
		mouseMoved(pt);
		rightButtonAction(pt);
	}
}

void SDSScene::rightButtonAction(const Common::Point &pt) {
	const HotArea *area = findAreaUnderMouse(pt);
	if (!area)
		return;

	DgdsEngine *engine = DgdsEngine::getInstance();

	if (area->_num == 0) {
		debug(1, "Mouse RUp on inventory.");
		engine->getInventory()->setShowZoomBox(true);
		engine->getInventory()->open();
	} else if (area->_num == 0xffff) {
		debug(1, "Mouse RUp on character swap.");
		int16 swapDlgFile = engine->getGDSScene()->getGlobal(0x36);
		int16 swapDlgNum = engine->getGDSScene()->getGlobal(0x35);
		if (swapDlgFile && swapDlgNum)
			showDialog(swapDlgFile, swapDlgNum);
	} else {
		int16 addmins = engine->getGameGlobals()->getGameMinsToAddOnLClick();
		debug(1, "Mouse RUp on area %d, run %d ops (+%d mins)", area->_num, area->onRClickOps.size(), addmins);
		runOps(area->onRClickOps, addmins);
	}
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

void SDSScene::setDynamicSceneRect(int16 num, int16 x, int16 y, int16 width, int16 height) {
	for (auto &dynamicRect : _dynamicRects) {
		if (dynamicRect._num == num) {
			dynamicRect._rect = DgdsRect(x, y, width, height);
			return;
		}
	}

	_dynamicRects.push_back(DynamicRect());
	_dynamicRects.back()._num = num;
	_dynamicRects.back()._rect = DgdsRect(x, y, width, height);
}


void SDSScene::updateHotAreasFromDynamicRects() {
	if (_dynamicRects.empty())
		return;
	for (auto &hotArea : _hotAreaList) {
		if (!hotArea._objInteractionRectNum)
			continue;
		for (const auto &dynamicRect : _dynamicRects) {
			if (hotArea._objInteractionRectNum == dynamicRect._num) {
				hotArea._rect = dynamicRect._rect;
				break;
			}
		}
	}
}

HotArea *SDSScene::findAreaUnderMouse(const Common::Point &pt) {
	for (auto &item : DgdsEngine::getInstance()->getGDSScene()->getGameItems()) {
		if (item._inSceneNum == _num && _isInRect(pt, item._rect)
			&& SceneConditions::check(item.enableConditions)) {
			return &item;
		}
	}

	for (auto &area : _hotAreaList) {
		if (_isInRect(pt, area._rect) && SceneConditions::check(area.enableConditions)) {
			return &area;
		}
	}
	return nullptr;
}

void SDSScene::addInvButtonToHotAreaList() {
	DgdsEngine *engine = DgdsEngine::getInstance();
	const Common::Array<MouseCursor> &cursors = engine->getGDSScene()->getCursorList();
	const Common::SharedPtr<Image> &icons = engine->getIcons();

	if (cursors.empty() || !icons || icons->loadedFrameCount() <= 2 || _num == 2)
		return;

	if (_hotAreaList.size() && _hotAreaList.front()._num == 0)
		return;

	int16 invButtonIcon = engine->getGDSScene()->getInvIconNum();
	if (engine->getGameId() == GID_HOC) {
		static const byte HOC_INV_ICONS[] = { 0, 2, 18, 19 };
		invButtonIcon = HOC_INV_ICONS[engine->getGDSScene()->getGlobal(0x33)];
	}

	HotArea area;
	area._num = 0;
	area._cursorNum = engine->getGDSScene()->getInvIconMouseCursor();
	area._rect.width = icons->width(invButtonIcon);
	area._rect.height = icons->height(invButtonIcon);
	area._rect.x = SCREEN_WIDTH - area._rect.width;
	area._rect.y = SCREEN_HEIGHT - area._rect.height;
	area._cursorNum2 = engine->getGDSScene()->getInvIconMouseCursor();
	area._objInteractionRectNum = 0;

	// Add swap character button for HoC
	if (engine->getGameId() == GID_HOC && engine->getGDSScene()->getGlobal(0x34) != 0) {
		int16 charNum = engine->getGDSScene()->getGlobal(0x34);
		int16 iconNum = DgdsEngine::HOC_CHAR_SWAP_ICONS[charNum];
		HotArea area2;
		area2._num = 0xffff;
		area2._cursorNum = 0;
		area2._rect.width = icons->width(iconNum);
		area2._rect.height = icons->height(iconNum);
		area2._rect.x = 5;
		area2._rect.y = SCREEN_HEIGHT - area2._rect.height - 5;
		area2._cursorNum2 = 0;
		area2._objInteractionRectNum = 0;

		_hotAreaList.push_front(area2);
	}

	_hotAreaList.push_front(area);
}

void SDSScene::removeInvButtonFromHotAreaList() {
	if (_hotAreaList.size() && _hotAreaList.front()._num == 0)
		_hotAreaList.pop_front();
	// Also remove character swap button in HoC
	if (_hotAreaList.size() && _hotAreaList.front()._num == 0xffff)
		_hotAreaList.pop_front();
}

Common::Error SDSScene::syncState(Common::Serializer &s) {
	// num should be synced as part of the engine -
	// at this point we are already loaded.
	assert(_num);

	// The dialogs and triggers are stateful, everything else is stateless.
	uint16 ndlgs = _dialogs.size();
	s.syncAsUint16LE(ndlgs);
	if (_dialogs.size() && ndlgs != _dialogs.size()) {
		error("Dialog count in save doesn't match count in game (%d vs %d)",
				ndlgs, _dialogs.size());
	} else if (_dialogs.size()) {
		for (auto &dlg : _dialogs) {
			dlg.syncState(s);
		}
	} else if (ndlgs && s.isLoading()) {
		warning("Skipping dialog data in save");
		Dialog dlg;
		for (uint i = 0; i < ndlgs; i++)
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

void SDSScene::prevChoice() {
	Dialog *dlg = getVisibleDialog();
	if (!dlg)
		return;
	dlg->updateSelectedAction(-1);
}

void SDSScene::nextChoice() {
	Dialog *dlg = getVisibleDialog();
	if (!dlg)
		return;
	dlg->updateSelectedAction(1);
}

void SDSScene::activateChoice() {
	Dialog *dlg = getVisibleDialog();
	if (!dlg)
		return;
	_shouldClearDlg = true;
}

void SDSScene::drawDebugHotAreas(Graphics::ManagedSurface *dst) const {
	const DgdsPal &pal = DgdsEngine::getInstance()->getGamePals()->getCurPal();
	byte redish = pal.findBestColor(0xff, 0, 0);
	byte greenish = pal.findBestColor(0, 0xff, 0);

	for (const auto &area : _hotAreaList) {
		bool enabled = SceneConditions::check(area.enableConditions);
		uint32 color = enabled ? greenish : redish;
		g_system->getPaletteManager();
		const Common::Rect &r = area._rect.toCommonRect();
		dst->drawLine(r.left, r.top, r.right, r.top, color);
		dst->drawLine(r.left, r.top, r.left, r.bottom, color);
		dst->drawLine(r.left, r.bottom, r.right, r.bottom, color);
		dst->drawLine(r.right, r.top, r.right, r.bottom, color);
	}
}

GDSScene::GDSScene() : _defaultMouseCursor(0), _defaultMouseCursor2(0), _invIconNum(0), _invIconMouseCursor(0), _defaultOtherMouseCursor(0) {
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
	while (num && !file->eos()) {
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

	DgdsEngine *engine = DgdsEngine::getInstance();
	Common::Array<Global *> &globs = engine->getGameGlobals()->getAllGlobals();

	if (engine->getGameId() == GID_DRAGON || engine->getGameId() == GID_HOC) {
		num = file->readUint16LE();
		while (num && !file->eos()) {
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
				error("Reset file references unknown scene global %d", num);
			num = file->readUint16LE();
		}

		/*uint32 unk = */ file->readUint32LE();

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

		engine->_compositionBuffer.fillRect(Common::Rect(SCREEN_WIDTH, SCREEN_HEIGHT), 0);
		// TODO: FIXME: What should this scene num be? For now hacked to work with Dragon.
		engine->changeScene(3);
		SDSScene *scene = engine->getScene();
		int t = 0;
		num = triggers[t++];
		while (num) {
			uint16 val = triggers[t++];
			scene->enableTrigger(0, num, (bool)val);
			num = triggers[t++];
		}
	} else {
		// Willy Beamish stores the globals differently
		num = file->readUint16LE();
		while (num && !file->eos()) {
			int16 val = file->readSint16LE();
			bool found = false;
			for (PerSceneGlobal &glob : _perSceneGlobals) {
				if (glob.numMatches(num)) {
					glob._val = val;
					found = true;
					break;
				}
			}
			if (!found)
				error("Reset file references unknown scene global %d", num);
			num = file->readUint16LE();
		}

		/*uint32 unk = */ file->readUint32LE();

		num = file->readUint16LE();
		while (num && !file->eos()) {
			bool found = false;
			int16 val = file->readUint16LE();
			for (Global *glob : globs) {
				if (glob->getNum() == num) {
					glob->setRaw(val);
					found = true;
					break;
				}
			}
			if (!found)
				error("Reset file references unknown game global %d", num);
			num = file->readUint16LE();
		}

		//
		// TODO: What is this block of data?  In practice there is only one of them
		//
		while (!file->eos()) {
			num = file->readUint16LE();
			if (!num)
				break;
			/*int16 val1 = */ file->readUint16LE();
			/*int16 val2 = */ file->readUint16LE();
			/*int16 val3 = */ file->readUint16LE();
		}

		/*uint16 soundBankNum = */ file->readUint16LE();

		engine->_compositionBuffer.fillRect(Common::Rect(SCREEN_WIDTH, SCREEN_HEIGHT), 0);
		// TODO: FIXME: What should this scene num be?
		engine->changeScene(3);
	}

	return true;
}

void GDSScene::initIconSizes() {
	const Common::SharedPtr<Image> icons = DgdsEngine::getInstance()->getIcons();
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
	uint16 num, scene;
	for (uint16 i = 0; i < numGlobals; i++) {
		num = s->readUint16LE();
		scene = s->readUint16LE();
		_perSceneGlobals.push_back(PerSceneGlobal(num, scene));
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
	readObjInteractionList(stream, _objInteractions1);
	if (isVersionOver(" 1.205"))
		readObjInteractionList(stream, _objInteractions2);

	if (isVersionOver(" 1.218")) {
		_defaultMouseCursor = stream->readSint16LE();
		_defaultMouseCursor2 = stream->readUint16LE();
		_invIconNum = stream->readUint16LE();
		_invIconMouseCursor = stream->readSint16LE();
		_defaultOtherMouseCursor = stream->readSint16LE();
	} else {
		_defaultMouseCursor = 0;
		_defaultMouseCursor2 = 1;
		_invIconNum = 2;
		_invIconMouseCursor = 0;
		_defaultOtherMouseCursor = 6;
	}

	return !stream->err();
}

Common::String GDSScene::dump(const Common::String &indent) const {
	Common::String str = Common::String::format("%sGDSScene<ver %s icons %s", indent.c_str(), _version.c_str(), _iconFile.c_str());
	str += DebugUtil::dumpStructList(indent, "gameItems", _gameItems);
	str += DebugUtil::dumpStructList(indent, "startGameOps", _startGameOps);
	str += DebugUtil::dumpStructList(indent, "quitGameOps", _quitGameOps);
	str += DebugUtil::dumpStructList(indent, "preTickOps", _preTickOps);
	str += DebugUtil::dumpStructList(indent, "postTickOps", _postTickOps);
	str += DebugUtil::dumpStructList(indent, "onChangeSceneOps", _onChangeSceneOps);
	str += DebugUtil::dumpStructList(indent, "perSceneGlobals", _perSceneGlobals);
	str += DebugUtil::dumpStructList(indent, "objInteractions1", _objInteractions1);
	str += DebugUtil::dumpStructList(indent, "objInteractions2", _objInteractions2);

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

int16 GDSScene::getGlobal(uint16 num) const {
	DgdsEngine *engine = DgdsEngine::getInstance();
	int curSceneNum = engine->getScene()->getNum();
	DgdsGameId gameId = engine->getGameId();

	for (const auto &global : _perSceneGlobals) {
		if (global.matches(num, curSceneNum))
			return global._val;
		else if (!global.matches(num, curSceneNum) && global.numMatches(num)) {
			// Don't warn on known reusable scene globals
			if (gameId == GID_WILLY && num == 185)
				return global._val;

			// This looks like a script bug, get it anyway
			warning("getGlobal: scene global %d is not in scene %d", num, curSceneNum);
			return global._val;
		}
	}
	Globals *gameGlobals = engine->getGameGlobals();
	return gameGlobals->getGlobal(num);
}

int16 GDSScene::setGlobal(uint16 num, int16 val) {
	DgdsEngine *engine = DgdsEngine::getInstance();
	int curSceneNum = engine->getScene()->getNum();
	for (auto &global : _perSceneGlobals) {
		if (global.matches(num, curSceneNum)) {
			global._val = val;
			return val;
		} else if (!global.matches(num, curSceneNum) && global.numMatches(num)) {
			// This looks like a script bug, set it anyway
			warning("setGlobal: scene global %d is not in scene %d", num, curSceneNum);
			global._val = val;
			return val;
		}
	}
	Globals *gameGlobals = engine->getGameGlobals();
	return gameGlobals->setGlobal(num, val);
}

void GDSScene::drawItems(Graphics::ManagedSurface &surf) {
	DgdsEngine *engine = DgdsEngine::getInstance();
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

int GDSScene::countItemsInInventory() const {
	int result = 0;
	bool isHoc = DgdsEngine::getInstance()->getGameId() == GID_HOC;
	for (const auto &item : _gameItems) {
		if (item._inSceneNum == 2) {
			if (isHoc) {
				int16 currentCharacter = getGlobal(0x33);
				if (item._quality == Inventory::HOC_CHARACTER_QUALS[currentCharacter])
					result++;
			} else {
				result++;
			}
		}
	}
	return result;
}

GameItem *GDSScene::getActiveItem() {
	int16 itemNum = getGlobal(0x60);
	if (itemNum <= 0)
		return nullptr;
	for (auto &item : _gameItems) {
		if (item._num == (uint16)itemNum)
			return &item;
	}
	return nullptr;
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
		//debug(1, "loaded item: %d %d %d %d", item._num, item._inSceneNum, item._flags, item._quality);
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
