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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/system.h"
#include "common/debug-channels.h"
#include "common/debug.h"
#include "common/error.h"
#include "engines/util.h"

#include "macventure/macventure.h"

// To move
#include "common/file.h"

namespace MacVenture {

enum {
	kMaxMenuTitleLength = 30
};

MacVentureEngine::MacVentureEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst) {
	_gameDescription = gameDesc;
	_rnd = new Common::RandomSource("macventure");

	_debugger= NULL;
	_gui = NULL;

	debug("MacVenture::MacVentureEngine()");
}

MacVentureEngine::~MacVentureEngine() {
	debug("MacVenture::~MacVentureEngine()");

	DebugMan.clearAllDebugChannels();
	delete _rnd;
	delete _debugger;
	delete _gui;
	delete _scriptEngine;

	if (_filenames)
		delete _filenames;

	if (_decodingArticles)
		delete _decodingArticles;

	if (_textHuffman)
		delete _textHuffman;
}

Common::Error MacVentureEngine::run() {
	debug("MacVenture::MacVentureEngine::init()");

	initGraphics(kScreenWidth, kScreenHeight, true);

	_debugger = new Console(this);

	// Additional setup.
	debug("MacVentureEngine::init");


	_resourceManager = new Common::MacResManager();
	if (!_resourceManager->open(getGameFileName()))
		error("Could not open %s as a resource fork", getGameFileName());

	// Engine-wide loading
	if (!loadGlobalSettings())
		error("Could not load the engine settings");

	_oldTextEncoding = !loadTextHuffman();

	_filenames = new StringTable(this, _resourceManager, kFilenamesStringTableID);
	_decodingArticles = new StringTable(this, _resourceManager, kCommonArticlesStringTableID);

	// Big class instantiation
	_gui = new Gui(this, _resourceManager);
	_world = new World(this, _resourceManager);
	_scriptEngine = new ScriptEngine(this, _world);

	_paused = false;
	_halted = true;
	_cmdReady = false;
	_haltedAtEnd = false;
	_haltedInSelection = false;

	//if !savegame
	_cmdReady = true;
	_selectedControl = kStartOrResume;
	ObjID playerParent = _world->getObjAttr(1, kAttrParentObject);
	_currentSelection.push_back(playerParent);// Push the parent of the player
	_world->setObjAttr(playerParent, 6, 1);

	_prepared = true;
	while (!(_gameState == kGameStateQuitting)) {
		processEvents();

		if (_prepared) {
			_prepared = false;	

			if (!_halted)
				updateState();

			if (_cmdReady || _halted) {
				_halted = false;
				if (runScriptEngine()) {
					_halted = true;
					_paused = true;
				} else {
					_paused = false;
					if (!updateState()) {
						updateControls();
					}
				}
			}

			if (_gameState == kGameStateWinnig || _gameState == kGameStateLosing) {
				endGame();
			}			
		}
		_gui->draw();

		g_system->updateScreen();
		g_system->delayMillis(50);
	}

	return Common::kNoError;
}

void MacVentureEngine::requestQuit() {
	// TODO: Display save game dialog and such
	_gameState = kGameStateQuitting;
}

void MacVentureEngine::requestUnpause() {
	_paused = false;
	_gameState = kGameStatePlaying;
}

void MacVentureEngine::selectControl(ControlReference id) {
	ControlAction action = referenceToAction(id);
	debug(4, "Select control %x", action);
	_selectedControl = action;
}

void MacVentureEngine::activateCommand(ControlReference id) {
	ControlAction action = referenceToAction(id);
	if (action != _activeControl) {
		if (_activeControl)
			_activeControl = kNoCommand;
		_activeControl = action;
	}
	debug(4, "Activating Command %x... Command %x is active", action, _activeControl);
}

void MacVentureEngine::refreshReady() {
	switch (getInvolvedObjects()) {
	case 0: // No selected object
		_cmdReady = true;
		break;
	case 1: // We have some selected object
		_cmdReady = _currentSelection.size() != 0;
		break;
	case 2:
		if (_destObject > 0) // We have a destination seleted
			_cmdReady = true;
		break;
	}
}

void MacVentureEngine::preparedToRun() {
	_prepared = true;
}

void MacVentureEngine::gameChanged() {
	_gameChanged = true;
}

void MacVentureEngine::winGame() {
	_gameState = kGameStateWinnig;
}

void MacVentureEngine::loseGame() {
	_gameState = kGameStateLosing;
}

void MacVentureEngine::enqueueObject(ObjectQueueID type, ObjID objID) {
	QueuedObject obj;
	obj.id = type;
	if (type != kHightlightExits) {		
		obj.object = objID;
		obj.parent = _world->getObjAttr(objID, kAttrParentObject);
		obj.x = _world->getObjAttr(objID, kAttrPosX);
		obj.y = _world->getObjAttr(objID, kAttrPosY);
		obj.exitx = _world->getObjAttr(objID, kAttrExitX);
		obj.exity = _world->getObjAttr(objID, kAttrExitY);
		obj.hidden = _world->getObjAttr(objID, kAttrHiddenExit);
		obj.offscreen = _world->getObjAttr(objID, kAttrInvisible);
		obj.invisible = _world->getObjAttr(objID, kAttrUnclickable);
	}
	_objQueue.push_back(obj);
}

void MacVentureEngine::enqueueText(TextQueueID type, ObjID target, ObjID source, ObjID text) {
	QueuedText newText;
	newText.id = type;
	newText.destination = target;
	newText.source = source;
	newText.asset = text;
	_textQueue.push_back(newText);
}

bool MacVentureEngine::printTexts() {
	warning("printTexts: unimplemented");
	for (uint i = 0; i < _textQueue.size(); i++) {
		QueuedText text = _textQueue.front();
		_textQueue.remove_at(0);
		switch (text.id) {
		case kTextNumber:
			debug("Print Number: %d", text.asset);
			gameChanged();
			break;
		case kTextNewLine:
			debug("Print Newline: ");
			gameChanged();
			break;
		case kTextPlain:
			debug("Print Plain Text: %s", _world->getText(text.asset).c_str());
			gameChanged();
			break;		
		}
	}
}

void MacVentureEngine::selectObject(ObjID objID) {
	bool found = false;
	uint i = 0;
	while (i < _currentSelection.size() && !found) {
		if (_currentSelection[i] == objID) found = true;
		else i++;
	}		
	
	if (!found) _currentSelection.push_back(objID);

	found = false;
	i = 0;
	while (i < _selectedObjs.size() && !found) {
		if (_selectedObjs[i] == objID) found = true;
		else i++;
	}

	if (!found) _selectedObjs.push_back(objID);
}

void MacVentureEngine::focusObjWin(ObjID objID) {
	_gui->bringToFront(getObjWindow(objID));
}

void MacVentureEngine::updateWindow(WindowReference winID) {
	_gui->updateWindow(winID, true);
}

const GlobalSettings& MacVentureEngine::getGlobalSettings() const {
	return _globalSettings;
}


// Private engine methods
void MacVentureEngine::processEvents() {
	Common::Event event;

	while (_eventMan->pollEvent(event)) {
		if (_gui->processEvent(event))
			continue;

		switch (event.type) {
		case Common::EVENT_QUIT:
			_gameState = kGameStateQuitting;
			break;
		default:
			break;
		}
	}
}

bool MacVenture::MacVentureEngine::runScriptEngine() {
	debug(4, "MAIN: Running script engine");
	if (_haltedAtEnd) {
		_haltedAtEnd = false;
		if (_scriptEngine->resume(false)) {
			_haltedAtEnd = true;
			return true;
		}
		return false;
	}

	if (_haltedInSelection) {
		_haltedInSelection = false;
		if (_scriptEngine->resume(false)) {
			_haltedInSelection = true;
			return true;
		}
		if (updateState())
			return true;
	}

	while (!_currentSelection.empty()) {
		ObjID obj = _currentSelection.front();
		_currentSelection.remove_at(0);
		if ((_gameState == kGameStateInit || _gameState == kGameStatePlaying) && _world->isObjActive(obj)) {
			if (_scriptEngine->runControl(_selectedControl, obj, _destObject, _deltaPoint)) {
				_haltedInSelection = true;
				return true;
			}
			if (updateState()) {
				return true;
			}
		}
	}
	if (_selectedControl == 1)
		_gameChanged = false;

	else if (_gameState == kGameStateInit || _gameState == kGameStatePlaying){
		if (_scriptEngine->runControl(kTick, _selectedControl, _destObject, _deltaPoint)) {
			_haltedAtEnd = true;
			return true;
		}
	}
	return false;
}

void MacVentureEngine::endGame() {
	requestQuit();
}

bool MacVentureEngine::updateState() {
	runObjQueue();
	bool wait = printTexts();
	return wait;
}

void MacVentureEngine::revert() {
	warning("revert: unimplemented");
}

void MacVentureEngine::runObjQueue() {
	warning("runObjQueue: not fully implemented");
	while (!_objQueue.empty()) {
		uint32 biggest = 0;
		uint32 index = 0;
		uint32 temp;
		for (uint i = 0; i < _objQueue.size(); i++) {
			temp = _objQueue[i].id;
			if (temp > biggest) {
				biggest = temp;
				index = i;
			}
		}
		QueuedObject obj = _objQueue[index];
		_objQueue.remove_at(index);
		switch (obj.id) {
		case 0x2:
			focusObjectWindow(obj.object);
			break;
		case 0x3:
			openObject(obj.object);
			break;
		case 0x4:
			closeObject(obj.object);
			break;
		case 0x7:
			checkObject(obj);
			break;
		case 0x8:
			reflectSwap(obj.object);
			break;
		case 0xc:
			_world->setObjAttr(_gui->getWindowData(kMainGameWindow).refcon, kAttrContainerOpen, 0);
			_world->setObjAttr(_world->getObjAttr(1, kAttrParentObject), kAttrContainerOpen, 1);
			break;
		case 0xd:
			toggleExits();
			break;
		case 0xe:
			zoomObject(obj.object);
			break;
		}
	}
}

void MacVentureEngine::updateControls() {
	if (_activeControl)
		_activeControl = kNoCommand;
	toggleExits();	
	resetVars();
}

void MacVentureEngine::resetVars() {
	_selectedControl = kNoCommand;
	_activeControl = kNoCommand;
	_currentSelection.clear();
	_destObject = 0;
	_deltaPoint = Common::Point(0, 0);
	_cmdReady = false;
}

void MacVentureEngine::focusObjectWindow(ObjID objID) {
	if (objID) {
		WindowReference win = getObjWindow(objID);
		if (win)
			_gui->bringToFront(win);
	}
}

void MacVentureEngine::openObject(ObjID objID) {

	debug("openObject: %d", objID);
	if (getObjWindow(objID)) return;
	if (objID == _world->getObjAttr(1, kAttrParentObject)) {
		_gui->updateWindowInfo(kMainGameWindow, objID, _world->getChildren(objID, true)); // FIXME: Find better name
		_gui->updateWindow(kMainGameWindow, _world->getObjAttr(objID, kAttrContainerOpen));
		//_gui->drawExits();
		_gui->setWindowTitle(kMainGameWindow, _world->getText(objID));
	} else { // Open inventory window
		Common::Point p(_world->getObjAttr(objID, kAttrPosX), _world->getObjAttr(objID, kAttrPosY));
		//getParentWin(obj).localToGlobal(p);
		//globalToDesktop(p);
		WindowReference invID = _gui->createInventoryWindow();
		_gui->setWindowTitle(invID, _world->getText(objID));
		_gui->updateWindowInfo(invID, objID, _world->getChildren(objID, true));
		_gui->updateWindow(invID, _world->getObjAttr(objID, kAttrContainerOpen));
	}
}

void MacVentureEngine::closeObject(ObjID objID) {
	warning("closeObject: not fully implemented");
	bool success = _gui->tryCloseWindow(getObjWindow(objID));
	return;
}

void MacVentureEngine::checkObject(QueuedObject old) {
	//warning("checkObject: unimplemented");
	bool hasChanged = false;
	debug("Check Object[%d] parent[%d] x[%d] y[%d]",
		old.object,
		_world->getObjAttr(old.object, kAttrParentObject),
		_world->getObjAttr(old.object, kAttrPosX),
		_world->getObjAttr(old.object, kAttrPosY));
	//bool incoming = isIncomingObj(objID);
	//if (incoming) removeIncoming(objID);
	ObjID id = old.object;
	if (id == 1) {
		if (old.parent != _world->getObjAttr(id, kAttrParentObject)) {
			enqueueObject(kSetToPlayerParent, id);
		}
		if (old.offscreen != _world->getObjAttr(id, kAttrInvisible) ||
			old.invisible != _world->getObjAttr(id, kAttrUnclickable)) {
			updateWindow(findParentWindow(id));
		}
	} else if (old.parent != _world->getObjAttr(id, kAttrParentObject) ||
				old.x != _world->getObjAttr(id, kAttrPosX) ||
				old.y != _world->getObjAttr(id, kAttrPosY)) {
		WindowReference oldWin = getObjWindow(old.parent);
		if (oldWin) {
			_gui->removeChild(oldWin, id);
			hasChanged = true;
		}

		WindowReference newWin = getObjWindow(id);
		if (newWin) {
			_gui->addChild(newWin, id);
			hasChanged = true;
		}
	} else if (old.offscreen != _world->getObjAttr(id, kAttrInvisible) ||
				old.invisible != _world->getObjAttr(id, kAttrUnclickable)) {
		updateWindow(findParentWindow(id));
	}

	if (_world->getObjAttr(id, kAttrIsExit)) {
		if (hasChanged ||
			old.hidden != _world->getObjAttr(id, kAttrHiddenExit) ||
			old.exitx != _world->getObjAttr(id, kAttrExitX) ||
			old.exity != _world->getObjAttr(id, kAttrExitY))
			_gui->drawExit(id);
	}
	WindowReference win = getObjWindow(id);
	ObjID cur = id;
	ObjID root = _world->getObjAttr(1, kAttrParentObject);
	while (cur != root)	{
		if (cur == 0 || !_world->getObjAttr(cur, kAttrContainerOpen)) break;
		cur = _world->getObjAttr(cur, kAttrParentObject);
	}
	if (cur == root) {
		if (win) return;
		enqueueObject(kOpenWindow, id); //open
	} else {
		if (!win) return;
		enqueueObject(kCloseWindow, id); //close
	}

	// Update children
	Common::Array<ObjID> children = _world->getChildren(id, true);
	for (uint i = 0; i < children.size(); i++) {
		enqueueObject(kUpdateObject, children[i]);
	}
}

void MacVentureEngine::reflectSwap(ObjID objID) {
	warning("reflectSwap: unimplemented");
}

void MacVentureEngine::toggleExits() {
	warning("toggleExits: unimplemented");
}

void MacVentureEngine::zoomObject(ObjID objID) {
	warning("zoomObject: unimplemented");
}

ControlAction MacVenture::MacVentureEngine::referenceToAction(ControlReference id) {
	switch (id) {
	case MacVenture::kControlExitBox:
		return kActivateObject;//??
	case MacVenture::kControlExamine:
		return kExamine;
	case MacVenture::kControlOpen:
		return kOpen;
	case MacVenture::kControlClose:
		return kClose;
	case MacVenture::kControlSpeak:
		return kSpeak;
	case MacVenture::kControlOperate:
		return kOperate;
	case MacVenture::kControlGo:
		return kGo;
	case MacVenture::kControlHit:
		return kHit;
	case MacVenture::kControlConsume:
		return kConsume;
	default:
		return kNoCommand;
	}
}

// Data retrieval

bool MacVentureEngine::isPaused() {
	return _paused;
}

Common::String MacVentureEngine::getCommandsPausedString() const {
	return Common::String("Click to continue");
}

Common::String MacVentureEngine::getFilePath(FilePathID id) const {
	const Common::Array<Common::String> *names = _filenames->getStrings();
	if (id <= 3) { // We don't want a file in the subdirectory
		return Common::String((*names)[id]);
	} else { // We want a game file
		return Common::String((*names)[3] + "/" + (*names)[id]);
	}
}

bool MacVentureEngine::isOldText() const {
	return _oldTextEncoding;
}

const HuffmanLists * MacVentureEngine::getDecodingHuffman() const {
	return _textHuffman;
}

uint32 MacVentureEngine::randBetween(uint32 min, uint32 max) {
	return _rnd->getRandomNumber(max - min) + min;
}

uint32 MacVentureEngine::getInvolvedObjects() {
	return (_selectedControl ? _globalSettings.cmdArgCnts[_selectedControl - 1] : 3000);
}

Common::Point MacVentureEngine::getObjPosition(ObjID objID) {
	return Common::Point(_world->getObjAttr(objID, kAttrPosX), _world->getObjAttr(objID, kAttrPosY));
}

bool MacVentureEngine::isObjVisible(ObjID objID) {
	return _world->getObjAttr(objID, kAttrInvisible) == 0;
}

bool MacVentureEngine::isObjClickable(ObjID objID) {
	return _world->getObjAttr(objID, kAttrUnclickable) == 0;
}

bool MacVentureEngine::isObjSelected(ObjID objID) {
	warning("Unimplemented: isObjSelected");
	return false;
}

WindowReference MacVentureEngine::getObjWindow(ObjID objID) {
	switch (objID) {
	case 0xfffc: return kExitsWindow;
	case 0xfffd: return kSelfWindow;
	case 0xfffe: return kOutConsoleWindow;
	case 0xffff: return kCommandsWindow;
	}

	return findObjWindow(objID);
}

WindowReference MacVentureEngine::findObjWindow(ObjID objID) {
	// This is a bit of a hack, we take advantage of the consecutive nature of references
	for (uint i = kCommandsWindow; i <= kDiplomaWindow; i++) {
		const WindowData &data = _gui->getWindowData((WindowReference)i);
		if (data.refcon == objID) { return data.refcon; }
	}
	return kNoWindow;
}

WindowReference MacVentureEngine::findParentWindow(ObjID objID) {
	if (objID == 1) return kSelfWindow;
	ObjID parent = _world->getObjAttr(objID, kAttrParentObject);
	return getObjWindow(parent);
}

Common::Point MacVentureEngine::getDeltaPoint() {
	return _deltaPoint;
}

ObjID MacVentureEngine::getDestObject() {
	return _destObject;
}

ControlAction MacVentureEngine::getSelectedControl() {
	return _selectedControl;
}

// Data loading

bool MacVentureEngine::loadGlobalSettings() {
	Common::MacResIDArray resArray;
	Common::SeekableReadStream *res;

	if ((resArray = _resourceManager->getResIDArray(MKTAG('G', 'N', 'R', 'L'))).size() == 0)
		return false;

	res = _resourceManager->getResource(MKTAG('G', 'N', 'R', 'L'), kGlobalSettingsID);
	if (res) {
		_globalSettings.numObjects = res->readUint16BE();
		_globalSettings.numGlobals = res->readUint16BE();
		_globalSettings.numCommands = res->readUint16BE();
		_globalSettings.numAttributes = res->readUint16BE();
		_globalSettings.numGroups = res->readUint16BE();
		res->readUint16BE(); // unknown
		_globalSettings.invTop = res->readUint16BE();
		_globalSettings.invLeft = res->readUint16BE();
		_globalSettings.invWidth = res->readUint16BE();
		_globalSettings.invHeight = res->readUint16BE();
		_globalSettings.invOffsetY = res->readUint16BE();
		_globalSettings.invOffsetX = res->readSint16BE();
		_globalSettings.defaultFont = res->readUint16BE();
		_globalSettings.defaultSize = res->readUint16BE();

		_globalSettings.attrIndices = new uint8[_globalSettings.numAttributes];
		res->read(_globalSettings.attrIndices, _globalSettings.numAttributes);

		_globalSettings.attrMasks = new uint16[_globalSettings.numAttributes];
		for (int i = 0; i < _globalSettings.numAttributes; i++)
			_globalSettings.attrMasks[i] = res->readUint16BE();

		_globalSettings.attrShifts = new uint8[_globalSettings.numAttributes];
		res->read(_globalSettings.attrShifts, _globalSettings.numAttributes);

		_globalSettings.cmdArgCnts = new uint8[_globalSettings.numCommands];
		res->read(_globalSettings.cmdArgCnts, _globalSettings.numCommands);

		_globalSettings.commands = new uint8[_globalSettings.numCommands];
		res->read(_globalSettings.commands, _globalSettings.numCommands);

		return true;
	}

	return false;
}

bool MacVentureEngine::loadTextHuffman() {
	Common::MacResIDArray resArray;
	Common::SeekableReadStream *res;

	if ((resArray = _resourceManager->getResIDArray(MKTAG('G', 'N', 'R', 'L'))).size() == 0)
		return false;

	res = _resourceManager->getResource(MKTAG('G', 'N', 'R', 'L'), kTextHuffmanTableID);
	if (res) {
		uint32 numEntries = res->readUint16BE();
		res->readUint16BE(); // Skip

		uint32 *masks = new uint32[numEntries];
		for (uint i = 0; i < numEntries - 1; i++)
			// For some reason there are one lass mask than entries
			masks[i] = res->readUint16BE();

		uint32 *lengths = new uint32[numEntries];
		for (uint i = 0; i < numEntries; i++)
			lengths[i] = res->readByte();

		uint32 *values = new uint32[numEntries];
		for (uint i = 0; i < numEntries; i++)
			values[i] = res->readByte();

		_textHuffman = new HuffmanLists(numEntries, lengths, masks, values);
		debug(4, "Text is huffman-encoded");
		return true;
	}
	return false;
}



} // End of namespace MacVenture
