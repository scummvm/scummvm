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

	_debugger = NULL;
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

	if (_decodingDirectArticles)
		delete _decodingDirectArticles;

	if (_decodingNamingArticles)
		delete _decodingNamingArticles;

	if (_decodingIndirectArticles)
		delete _decodingIndirectArticles;

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
	_decodingDirectArticles = new StringTable(this, _resourceManager, kCommonArticlesStringTableID);
	_decodingNamingArticles = new StringTable(this, _resourceManager, kNamingArticlesStringTableID);
	_decodingIndirectArticles = new StringTable(this, _resourceManager, kIndirectArticlesStringTableID);

	// Big class instantiation
	_gui = new Gui(this, _resourceManager);
	_world = new World(this, _resourceManager);
	_scriptEngine = new ScriptEngine(this, _world);

	_paused = false;
	_halted = false;
	_cmdReady = false;
	_haltedAtEnd = false;
	_haltedInSelection = false;
	_clickToContinue = true;
	_gameState = kGameStateInit;
	_destObject = 0;
	_prepared = true;

	_cmdReady = true;
	_selectedControl = kStartOrResume;
	ObjID playerParent = _world->getObjAttr(1, kAttrParentObject);
	_currentSelection.push_back(playerParent);// Push the parent of the player
	_world->setObjAttr(playerParent, kAttrContainerOpen, 1);

	_gui->addChild(kSelfWindow, 1);
	_gui->updateWindow(kSelfWindow, false);

	while (_gameState != kGameStateQuitting) {
		processEvents();

		if (_gameState != kGameStateQuitting) {

			if (_prepared && !_gui->isDialogOpen()) {
				_prepared = false;

				if (!_halted)
					updateState();

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
					return Common::kNoError;
				}
			}
			_gui->draw();

			g_system->updateScreen();
			g_system->delayMillis(50);
		}
	}

	return Common::kNoError;
}

Common::Error MacVentureEngine::loadGameState(int slot) {
	Common::InSaveFile *file = getSaveFileManager()->openForLoading("Shadowgate.1");
	_world->loadGameFrom(file);
	reset();
	return Common::kNoError;
}

Common::Error MacVentureEngine::saveGameState(int slot, const Common::String &desc) {
	Common::SaveFileManager *manager = getSaveFileManager();
	// HACK Get a real name!
	Common::OutSaveFile *file = manager->openForSaving("Shadowgate.1");
	_world->saveGameInto(file);
	delete file;
}

void MacVentureEngine::reset() {
	resetInternals();
	resetGui();
}

void MacVentureEngine::resetInternals() {
	_scriptEngine->reset();
	_currentSelection.clear();
	_selectedObjs.clear();
	_objQueue.clear();
	_textQueue.clear();
}

void MacVentureEngine::resetGui() {
	_gui->updateWindowInfo(kMainGameWindow, getParent(1), _world->getChildren(getParent(1), true));
	// HACK! should update all inventories
	_gui->updateWindowInfo(kInventoryStart, 1, _world->getChildren(1, true));
	updateControls();
	updateExits();
}


void MacVentureEngine::requestQuit() {
	// TODO: Display save game dialog and such
	_gameState = kGameStateQuitting;
}

void MacVentureEngine::requestUnpause() {
	_paused = false;
	_gameState = kGameStatePlaying;
}

void MacVentureEngine::selectControl(ControlAction id) {
	debug(2, "Select control %x", id);
	_selectedControl = id;
}

void MacVentureEngine::activateCommand(ControlAction id) {
	if (id == kClickToContinue) {
		_clickToContinue = false;
		_paused = true;
		return;
	}
	if (id != _activeControl) {
		if (_activeControl)
			_activeControl = kNoCommand;
		_activeControl = id;
	}
	debug(2, "Activating Command %x... Command %x is active", id, _activeControl);
	refreshReady();
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

void MacVentureEngine::clickToContinue() {
	_clickToContinue = true;
}

void MacVentureEngine::enqueueObject(ObjectQueueID type, ObjID objID, ObjID target) {
	QueuedObject obj;
	obj.id = type;

	if (type == kUpdateObject && isObjEnqueued(objID)) {
		return;
	}

	if (type == kUpdateWindow) { obj.target = target; }

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

bool MacVentureEngine::isObjEnqueued(ObjID objID) {
	Common::Array<QueuedObject>::const_iterator it;
	for (it = _objQueue.begin(); it != _objQueue.end(); it++) {
		if ((*it).object == objID) return true;
	}
	return false;
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
	for (uint i = 0; i < _textQueue.size(); i++) {
		QueuedText text = _textQueue.front();
		_textQueue.remove_at(0);
		switch (text.id) {
		case kTextNumber:
			_gui->printText(Common::String(text.asset));
			gameChanged();
			break;
		case kTextNewLine:
			_gui->printText(Common::String(""));
			gameChanged();
			break;
		case kTextPlain:
			_gui->printText(_world->getText(text.asset, text.source, text.destination));
			gameChanged();
			break;
		}
	}
	return false;
}

void MacVentureEngine::handleObjectSelect(ObjID objID, WindowReference win, bool shiftPressed, bool isDoubleClick) {
	if (win == kExitsWindow) {
		win = kMainGameWindow;
	}

	bool canDrag = (objID && !_world->getObjAttr(objID, kAttrInvisible));

	const WindowData &windata = _gui->getWindowData(win);

	if (shiftPressed) {
		// Do shift ;)
	} else {
		if (_selectedControl && _currentSelection.size() > 0 && getInvolvedObjects() > 1) {
			if (objID == 0)
				selectPrimaryObject(windata.objRef);
			else
				selectPrimaryObject(objID);
			preparedToRun();
		} else {
			if (objID == 0) {
				unselectAll();
				//if (windata.type == kAnimateBack) {
					//doLasso(win, event, canDrag);
				//}
				//else {
					objID = win;
				//}
			}
			if (objID > 0) {
				int i = findObjectInArray(objID, _currentSelection);

				if (isDoubleClick) {
					if (i >= 0)
						unselectAll();
					selectObject(objID);
					_destObject = objID;
					_deltaPoint = Common::Point(0, 0);
					if (!_cmdReady) {
						selectControl(kActivateObject);
						_activeControl = kActivateObject;
						_cmdReady = true;
					}
					preparedToRun();
				} else {
					if (i >= 0)
						unselectAll();
					selectObject(objID);
					if (getInvolvedObjects() == 1)
						_cmdReady = true;
					preparedToRun();
				}
			}
		}
	}
}

void MacVentureEngine::handleObjectDrop(ObjID objID, Common::Point delta, ObjID newParent) {
	_destObject = newParent;
	updateDelta(delta);
	selectControl(kMoveObject);
	activateCommand(kMoveObject);
	refreshReady();
	preparedToRun();
}

void MacVentureEngine::updateDelta(Common::Point newPos) {
	debug("Update delta: Old(%d, %d), New(%d, %d)",
		_deltaPoint.x, _deltaPoint.y,
		newPos.x, newPos.y);
	_deltaPoint = newPos;
}

void MacVentureEngine::focusObjWin(ObjID objID) {
	_gui->bringToFront(getObjWindow(objID));
}

void MacVentureEngine::updateWindow(WindowReference winID) {
	_gui->updateWindow(winID, true);
}

bool MacVentureEngine::showTextEntry(ObjID text, ObjID srcObj, ObjID destObj) {
	debug("Showing speech dialog, asset %d from %d to %d", text, srcObj, destObj);
	_gui->getTextFromUser();

	// HACK WITH FLAGS
	_prepared = false;
	warning("Show text entry: not fully tested");
	return true;
}

void MacVentureEngine::setTextInput(Common::String content) {
	// HACK WITH FLAGS
	_prepared = true;
	_userInput = content;
	_clickToContinue = false;
}

Common::String MacVentureEngine::getUserInput() {
	return _userInput;
}


Common::String MacVentureEngine::getStartGameFileName() {
	Common::SeekableReadStream *res;
	res = _resourceManager->getResource(MKTAG('S', 'T', 'R', ' '), kStartGameFilenameID);
	if (!res)
		return "";

	byte length = res->readByte();
	char *fileName = new char[length + 1];
	res->read(fileName, length);
	fileName[length] = '\0';
  return Common::String(fileName, length);
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

	else if (_gameState == kGameStateInit || _gameState == kGameStatePlaying) {
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
	_gui->invertWindowColors(kMainGameWindow);
	preparedToRun();
}

void MacVentureEngine::runObjQueue() {
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
			reflectSwap(obj.object, obj.target);
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
	_gui->clearControls();
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

void MacVentureEngine::unselectAll() {
	while (!_currentSelection.empty()) {
		unselectObject(_currentSelection.front());
	}
}

void MacVentureEngine::selectObject(ObjID objID) {
	if (!_currentSelection.empty()) {
		if (findParentWindow(objID) != findParentWindow(_currentSelection[0]))
			unselectAll();
	}
	if (findObjectInArray(objID, _currentSelection) == -1)
		_currentSelection.push_back(objID);
	if (findObjectInArray(objID, _selectedObjs) == -1) {
		_selectedObjs.push_back(objID);
		highlightExit(objID);
	}
}

void MacVentureEngine::unselectObject(ObjID objID) {
	int idxCur = findObjectInArray(objID, _currentSelection);
	int idxSel = findObjectInArray(objID, _selectedObjs);
	if (idxCur != -1) _currentSelection.remove_at(idxCur);
	if (idxSel != -1) {
		_selectedObjs.remove_at(idxSel);
		highlightExit(objID);
	}
}


void MacVentureEngine::updateExits() {
	_gui->clearExits();
	_gui->unselectExits();

	Common::Array<ObjID> exits = _world->getChildren(_world->getObjAttr(1, kAttrParentObject), true);
	for (uint i = 0; i < exits.size(); i++)
		_gui->updateExit(exits[i]);

}

int MacVentureEngine::findObjectInArray(ObjID objID, const Common::Array<ObjID> &list) {
	// Find the object in the current selection
	bool found = false;
	uint i = 0;
	while (i < list.size() && !found) {
		if (list[i] == objID) found = true;
		else i++;
	}
	// HACK, should use iterator
	return found ? i : -1;
}

uint MacVentureEngine::getPrefixNdx(ObjID obj) {
	return _world->getObjAttr(obj, kAttrPrefixes);
}

Common::String MacVentureEngine::getPrefixString(uint flag, ObjID obj) {
	uint ndx = getPrefixNdx(obj);
	ndx = ((ndx) >> flag) & 3;
	if (ndx) {
		return _decodingNamingArticles->getString(ndx);
	} else {
		return Common::String("m1551gn0 ");
	}
}

Common::String MacVentureEngine::getNoun(ObjID ndx) {
	return _decodingIndirectArticles->getString(ndx);
}

void MacVentureEngine::highlightExit(ObjID objID) {
	//ObjID ctl = _gui->getWinChild(obj);
	/*if (ctl) {
		if (findObjectInArray(obj, _selectedObjs) != -1)
			_gui->selectExit(ctl);
		else
			_gui->unselectExit(ctl);
	}
	if (obj == _world->getObjAttr(1, kAttrParentObject)) {
		if (findObjectInArray(obj, _selectedObjs) != -1)
			_gui->selectExit(obj);
		else
			_gui->unselectExit(obj);
	}*/
	//updateWindow(findParentWindow(obj));
}

void MacVentureEngine::selectPrimaryObject(ObjID objID) {
	if (objID == _destObject) return;
	int idx;
	if (_destObject > 0 &&
		(idx = findObjectInArray(_destObject, _selectedObjs)) != -1 &&
		findObjectInArray(_destObject, _currentSelection) == -1)
	{
		_selectedObjs.remove_at(idx);
		highlightExit(_destObject);
	}
	_destObject = objID;
	if (findObjectInArray(_destObject, _selectedObjs) == -1) {
		_selectedObjs.push_back(_destObject);
		highlightExit(_destObject);
	}
	_cmdReady = true;
}

void MacVentureEngine::focusObjectWindow(ObjID objID) {
	if (objID) {
		WindowReference win = getObjWindow(objID);
		if (win)
			_gui->bringToFront(win);
	}
}

void MacVentureEngine::openObject(ObjID objID) {

	debug("Open Object[%d] parent[%d] x[%d] y[%d]",
		objID,
		_world->getObjAttr(objID, kAttrParentObject),
		_world->getObjAttr(objID, kAttrPosX),
		_world->getObjAttr(objID, kAttrPosY));

	if (getObjWindow(objID)) return;
	if (objID == _world->getObjAttr(1, kAttrParentObject)) {
		_gui->updateWindowInfo(kMainGameWindow, objID, _world->getChildren(objID, true));
		_gui->updateWindow(kMainGameWindow, _world->getObjAttr(objID, kAttrContainerOpen));
		updateExits();
		_gui->setWindowTitle(kMainGameWindow, _world->getText(objID, objID, objID)); // it ignores source and target in the original
	} else { // Open inventory window
		Common::Point p(_world->getObjAttr(objID, kAttrPosX), _world->getObjAttr(objID, kAttrPosY));
		//getParentWin(obj).localToGlobal(p);
		//globalToDesktop(p);
		WindowReference invID = _gui->createInventoryWindow(objID);
		_gui->setWindowTitle(invID, _world->getText(objID, objID, objID));
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
		old.parent,
		old.x,
		old.y);
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

		WindowReference newWin = findParentWindow(id);
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
			_gui->updateExit(id);
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

void MacVentureEngine::reflectSwap(ObjID fromID, ObjID toID) {
	//warning("reflectSwap: untested");
	WindowReference from = getObjWindow(fromID);
	WindowReference to = getObjWindow(toID);
	WindowReference tmp = to;
	debug("Swap Object[%d] to Object[%d], from win[%d] to win[%d] ",
		fromID, toID, from, to);

	if (!to) {
		tmp = from;
	}
	if (tmp) {
		Common::String newTitle = _world->getText(toID, 0, 0); // Ignores src and targ in the original
		_gui->setWindowTitle(tmp, newTitle);
		_gui->updateWindowInfo(tmp, toID, _world->getChildren(toID, true));
		updateWindow(tmp);
	}
}

void MacVentureEngine::toggleExits() {
	while (!_selectedObjs.empty()) {
		ObjID obj = _selectedObjs.front();
		_selectedObjs.remove_at(0);
		highlightExit(obj);
		updateWindow(findParentWindow(obj));
	}
}

void MacVentureEngine::zoomObject(ObjID objID) {
	warning("zoomObject: unimplemented");
}

ControlAction MacVenture::MacVentureEngine::referenceToAction(ControlType id) {
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

bool MacVentureEngine::needsClickToContinue() {
	return _clickToContinue;
}

Common::String MacVentureEngine::getCommandsPausedString() const {
	return Common::String("Click to continue");
}

Common::String MacVentureEngine::getFilePath(FilePathID id) const {
	const Common::Array<Common::String> &names = _filenames->getStrings();
	if (id <= 3) { // We don't want a file in the subdirectory
		return Common::String(names[id]);
	} else { // We want a game file
		return Common::String(names[3] + "/" + names[id]);
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
	int idx = findObjectInArray(objID, _currentSelection);
	return idx != -1;
}

bool MacVentureEngine::isObjExit(ObjID objID) {
	return _world->getObjAttr(objID, kAttrIsExit);
}

bool MacVentureEngine::isHiddenExit(ObjID objID) {
	return _world->getObjAttr(objID, kAttrHiddenExit);
}

Common::Point MacVentureEngine::getObjExitPosition(ObjID objID) {
	uint x = _world->getObjAttr(objID, kAttrExitX);
	uint y = _world->getObjAttr(objID, kAttrExitY);
	return Common::Point(x, y);
}

ObjID MacVentureEngine::getParent(ObjID objID) {
	return _world->getObjAttr(objID, kAttrParentObject);
}

Common::Rect MacVentureEngine::getObjBounds(ObjID objID) {
	Common::Point pos = getObjPosition(objID);

	WindowReference win  = findParentWindow(objID);
	if (win != kNoWindow) { // If it's not in a window YET, we don't really care about the border
		BorderBounds bounds = _gui->borderBounds(_gui->getWindowData(win).type); // HACK
		pos.x += bounds.leftOffset;
		pos.y += bounds.topOffset;
	}
	Common::Point measures = _gui->getObjMeasures(objID);
	uint w = measures.x;
	uint h = measures.y;
	return Common::Rect(pos.x, pos.y, pos.x + w, pos.y + h);
}

uint MacVentureEngine::getOverlapPercent(ObjID one, ObjID other) {
	//not the same parent? 0 overlap
	if (_world->getObjAttr(one, kAttrParentObject) !=
		_world->getObjAttr(other, kAttrParentObject))
		return 0;

	Common::Rect oneBounds = getObjBounds(one);
	Common::Rect otherBounds = getObjBounds(other);
	if (otherBounds.intersects(oneBounds) ||
		oneBounds.intersects(otherBounds))
	{
		uint areaOne = oneBounds.width() * oneBounds.height();
		uint areaOther = otherBounds.width() * otherBounds.height();
		return (areaOther * 100 / areaOne) | 0;
	}
	return 0;
}

WindowReference MacVentureEngine::getObjWindow(ObjID objID) {
	return _gui->getObjWindow(objID);
}

WindowReference MacVentureEngine::findParentWindow(ObjID objID) {
	if (objID == 1) return kSelfWindow;
	ObjID parent = _world->getObjAttr(objID, kAttrParentObject);
	if (parent == 0) return kNoWindow;
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
