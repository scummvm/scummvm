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
	_scriptEngine = new ScriptEngine();

	_paused = false;
	_halted = true;
	_cmdReady = false;
	_haltedAtEnd = false;
	_haltedInSelection = false;
	_prepared = true;
	while (!(_gameState == kGameStateQuitting)) {
		processEvents();

		if (_prepared) {
			_prepared = false;

			if (!_halted) {
				_gui->draw();
			}

			if (_cmdReady || _halted) {
				_halted = false;
				if (runScriptEngine()) {
					_halted = true;
					_paused = true;
				}
				else {
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
	switch (objectsToApplyCommand()) {
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

void MacVentureEngine::enqueueObject(ObjID id) {
	QueuedObject obj;
	obj.parent = _world->getObjAttr(id, kAttrParentObject);
	obj.x = _world->getObjAttr(id, kAttrPosX);
	obj.y = _world->getObjAttr(id, kAttrPosY);
	obj.exitx = _world->getObjAttr(id, kAttrExitX);
	obj.exity = _world->getObjAttr(id, kAttrExitY);
	obj.hidden = _world->getObjAttr(id, kAttrHiddenExit);
	obj.offsecreen = _world->getObjAttr(id, kAttrInvisible);
	obj.invisible = _world->getObjAttr(id, kAttrUnclickable);
	_objQueue.push_back(obj);
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
		if (_scriptEngine->resume()) {
			_haltedAtEnd = true;
			return true;
		}
		return false;
	}

	if (_haltedInSelection) {
		_haltedInSelection = false;
		if (_scriptEngine->resume()) {
			_haltedInSelection = true;
			return true;
		}
		if (updateState())
			return true;
	}

	while (!_currentSelection.empty()) {
		ObjID obj = _currentSelection.front();
		_currentSelection.pop_front();
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
	return true;
}

void MacVentureEngine::runObjQueue() {

}

void MacVentureEngine::updateControls() {
	if (_activeControl)
		_activeControl = kNoCommand;
	// toggleExits();
	// resetVars();
}

void MacVentureEngine::resetVars() {
	_selectedControl = kNoCommand;
	_activeControl = kNoCommand;
	_currentSelection.clear();
	_destObject = 0;
	_deltaPoint = Common::Point(0, 0);
	_cmdReady = false;
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

uint MacVentureEngine::objectsToApplyCommand() {
	return uint();
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
