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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/Ad/AdActor.h"
#include "engines/wintermute/Ad/AdGame.h"
#include "engines/wintermute/Ad/AdEntity.h"
#include "engines/wintermute/Ad/AdInventory.h"
#include "engines/wintermute/Ad/AdInventoryBox.h"
#include "engines/wintermute/Ad/AdItem.h"
#include "engines/wintermute/Ad/AdResponse.h"
#include "engines/wintermute/Ad/AdResponseBox.h"
#include "engines/wintermute/Ad/AdResponseContext.h"
#include "engines/wintermute/Ad/AdScene.h"
#include "engines/wintermute/Ad/AdSceneState.h"
#include "engines/wintermute/Ad/AdSentence.h"
#include "engines/wintermute/Base/BFileManager.h"
#include "engines/wintermute/Base/BFont.h"
#include "engines/wintermute/Base/BObject.h"
#include "engines/wintermute/Base/BParser.h"
#include "engines/wintermute/Base/BSound.h"
#include "engines/wintermute/Base/BStringTable.h"
#include "engines/wintermute/Base/BSurfaceStorage.h"
#include "engines/wintermute/Base/BTransitionMgr.h"
#include "engines/wintermute/Base/BSprite.h"
#include "engines/wintermute/Base/BViewport.h"
#include "engines/wintermute/Base/PartEmitter.h"
#include "engines/wintermute/Base/scriptables/ScEngine.h"
#include "engines/wintermute/Base/scriptables/ScScript.h"
#include "engines/wintermute/Base/scriptables/ScStack.h"
#include "engines/wintermute/Base/scriptables/ScValue.h"
#include "engines/wintermute/UI/UIEntity.h"
#include "engines/wintermute/UI/UIWindow.h"
#include "engines/wintermute/utils/utils.h"
#include "engines/wintermute/video/VidPlayer.h"
#include "engines/wintermute/video/VidTheoraPlayer.h"
#include "common/str.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CAdGame, true)

//////////////////////////////////////////////////////////////////////////
CAdGame::CAdGame(): CBGame() {
	_responseBox = NULL;
	_inventoryBox = NULL;

	_scene = new CAdScene(Game);
	_scene->setName("");
	registerObject(_scene);

	_prevSceneName = NULL;
	_prevSceneFilename = NULL;
	_scheduledScene = NULL;
	_scheduledFadeIn = false;


	_stateEx = GAME_NORMAL;

	_selectedItem = NULL;


	_texItemLifeTime = 10000;
	_texWalkLifeTime = 10000;
	_texStandLifeTime = 10000;
	_texTalkLifeTime = 10000;

	_talkSkipButton = TALK_SKIP_LEFT;

	_sceneViewport = NULL;

	_initialScene = true;
	_debugStartupScene = NULL;
	_startupScene = NULL;

	_invObject = new CAdObject(this);
	_inventoryOwner = _invObject;

	_tempDisableSaveState = false;
	_itemsFile = NULL;

	_smartItemCursor = false;

	addSpeechDir("speech");
}


//////////////////////////////////////////////////////////////////////////
CAdGame::~CAdGame() {
	cleanup();
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::cleanup() {
	int i;

	for (i = 0; i < _objects.getSize(); i++) {
		unregisterObject(_objects[i]);
		_objects[i] = NULL;
	}
	_objects.removeAll();


	for (i = 0; i < _dlgPendingBranches.getSize(); i++) {
		delete [] _dlgPendingBranches[i];
	}
	_dlgPendingBranches.removeAll();

	for (i = 0; i < _speechDirs.getSize(); i++) {
		delete [] _speechDirs[i];
	}
	_speechDirs.removeAll();


	unregisterObject(_scene);
	_scene = NULL;

	// remove items
	for (i = 0; i < _items.getSize(); i++) Game->unregisterObject(_items[i]);
	_items.removeAll();


	// clear remaining inventories
	delete _invObject;
	_invObject = NULL;

	for (i = 0; i < _inventories.getSize(); i++) {
		delete _inventories[i];
	}
	_inventories.removeAll();


	if (_responseBox) {
		Game->unregisterObject(_responseBox);
		_responseBox = NULL;
	}

	if (_inventoryBox) {
		Game->unregisterObject(_inventoryBox);
		_inventoryBox = NULL;
	}

	delete[] _prevSceneName;
	delete[] _prevSceneFilename;
	delete[] _scheduledScene;
	delete[] _debugStartupScene;
	delete[] _itemsFile;
	_prevSceneName = NULL;
	_prevSceneFilename = NULL;
	_scheduledScene = NULL;
	_debugStartupScene = NULL;
	_startupScene = NULL;
	_itemsFile = NULL;

	delete _sceneViewport;
	_sceneViewport = NULL;

	for (i = 0; i < _sceneStates.getSize(); i++) delete _sceneStates[i];
	_sceneStates.removeAll();

	for (i = 0; i < _responsesBranch.getSize(); i++) delete _responsesBranch[i];
	_responsesBranch.removeAll();

	for (i = 0; i < _responsesGame.getSize(); i++) delete _responsesGame[i];
	_responsesGame.removeAll();

	return CBGame::cleanup();
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::initLoop() {
	if (_scheduledScene && _transMgr->isReady()) {
		changeScene(_scheduledScene, _scheduledFadeIn);
		delete[] _scheduledScene;
		_scheduledScene = NULL;

		Game->_activeObject = NULL;
	}


	ERRORCODE res;
	res = CBGame::initLoop();
	if (DID_FAIL(res)) return res;

	if (_scene) res = _scene->initLoop();

	_sentences.removeAll();

	return res;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::addObject(CAdObject *object) {
	_objects.add(object);
	return registerObject(object);
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::removeObject(CAdObject *object) {
	// in case the user called Scene.CreateXXX() and Game.DeleteXXX()
	if (_scene) {
		ERRORCODE Res = _scene->removeObject(object);
		if (DID_SUCCEED(Res)) return Res;
	}

	for (int i = 0; i < _objects.getSize(); i++) {
		if (_objects[i] == object) {
			_objects.removeAt(i);
			break;
		}
	}
	return unregisterObject(object);
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::changeScene(const char *filename, bool fadeIn) {
	if (_scene == NULL) {
		_scene = new CAdScene(Game);
		registerObject(_scene);
	} else {
		_scene->applyEvent("SceneShutdown", true);

		setPrevSceneName(_scene->_name);
		setPrevSceneFilename(_scene->_filename);

		if (!_tempDisableSaveState) _scene->saveState();
		_tempDisableSaveState = false;
	}

	if (_scene) {
		// reset objects
		for (int i = 0; i < _objects.getSize(); i++) _objects[i]->reset();

		// reset scene properties
		_scene->_sFXVolume = 100;
		if (_scene->_scProp) _scene->_scProp->cleanup();

		ERRORCODE ret;
		if (_initialScene && _dEBUG_DebugMode && _debugStartupScene) {
			_initialScene = false;
			ret = _scene->loadFile(_debugStartupScene);
		} else ret = _scene->loadFile(filename);

		if (DID_SUCCEED(ret)) {
			// invalidate references to the original scene
			for (int i = 0; i < _objects.getSize(); i++) {
				_objects[i]->invalidateCurrRegions();
				_objects[i]->_stickRegion = NULL;
			}

			_scene->loadState();
		}
		if (fadeIn) Game->_transMgr->start(TRANSITION_FADE_IN);
		return ret;
	} else return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
void CAdGame::addSentence(CAdSentence *sentence) {
	_sentences.add(sentence);
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::displaySentences(bool frozen) {
	for (int i = 0; i < _sentences.getSize(); i++) {
		if (frozen && _sentences[i]->_freezable) continue;
		else _sentences[i]->display();
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
void CAdGame::finishSentences() {
	for (int i = 0; i < _sentences.getSize(); i++) {
		if (_sentences[i]->CanSkip()) {
			_sentences[i]->_duration = 0;
			if (_sentences[i]->_sound) _sentences[i]->_sound->stop();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// ChangeScene
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "ChangeScene") == 0) {
		stack->correctParams(3);
		const char *filename = stack->pop()->getString();
		CScValue *valFadeOut = stack->pop();
		CScValue *valFadeIn = stack->pop();

		bool transOut = valFadeOut->isNULL() ? true : valFadeOut->getBool();
		bool transIn  = valFadeIn->isNULL() ? true : valFadeIn->getBool();

		scheduleChangeScene(filename, transIn);
		if (transOut) _transMgr->start(TRANSITION_FADE_OUT, true);
		stack->pushNULL();


		//ERRORCODE ret = ChangeScene(stack->pop()->getString());
		//if(DID_FAIL(ret)) stack->pushBool(false);
		//else stack->pushBool(true);

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LoadActor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LoadActor") == 0) {
		stack->correctParams(1);
		CAdActor *act = new CAdActor(Game);
		if (act && DID_SUCCEED(act->loadFile(stack->pop()->getString()))) {
			addObject(act);
			stack->pushNative(act, true);
		} else {
			delete act;
			act = NULL;
			stack->pushNULL();
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LoadEntity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LoadEntity") == 0) {
		stack->correctParams(1);
		CAdEntity *ent = new CAdEntity(Game);
		if (ent && DID_SUCCEED(ent->loadFile(stack->pop()->getString()))) {
			addObject(ent);
			stack->pushNative(ent, true);
		} else {
			delete ent;
			ent = NULL;
			stack->pushNULL();
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// UnloadObject / UnloadActor / UnloadEntity / DeleteEntity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "UnloadObject") == 0 || strcmp(name, "UnloadActor") == 0 || strcmp(name, "UnloadEntity") == 0 || strcmp(name, "DeleteEntity") == 0) {
		stack->correctParams(1);
		CScValue *val = stack->pop();
		CAdObject *obj = (CAdObject *)val->getNative();
		removeObject(obj);
		if (val->getType() == VAL_VARIABLE_REF) val->setNULL();

		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CreateEntity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CreateEntity") == 0) {
		stack->correctParams(1);
		CScValue *val = stack->pop();

		CAdEntity *ent = new CAdEntity(Game);
		addObject(ent);
		if (!val->isNULL()) ent->setName(val->getString());
		stack->pushNative(ent, true);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CreateItem
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CreateItem") == 0) {
		stack->correctParams(1);
		CScValue *val = stack->pop();

		CAdItem *item = new CAdItem(Game);
		addItem(item);
		if (!val->isNULL()) item->setName(val->getString());
		stack->pushNative(item, true);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeleteItem
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DeleteItem") == 0) {
		stack->correctParams(1);
		CScValue *val = stack->pop();

		CAdItem *item = NULL;
		if (val->isNative()) item = (CAdItem *)val->getNative();
		else item = getItemByName(val->getString());

		if (item) {
			deleteItem(item);
		}

		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// QueryItem
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "QueryItem") == 0) {
		stack->correctParams(1);
		CScValue *val = stack->pop();

		CAdItem *item = NULL;
		if (val->isInt()) {
			int index = val->getInt();
			if (index >= 0 && index < _items.getSize()) item = _items[index];
		} else {
			item = getItemByName(val->getString());
		}

		if (item) stack->pushNative(item, true);
		else stack->pushNULL();

		return STATUS_OK;
	}


	//////////////////////////////////////////////////////////////////////////
	// AddResponse/AddResponseOnce/AddResponseOnceGame
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AddResponse") == 0 || strcmp(name, "AddResponseOnce") == 0 || strcmp(name, "AddResponseOnceGame") == 0) {
		stack->correctParams(6);
		int id = stack->pop()->getInt();
		const char *text = stack->pop()->getString();
		CScValue *val1 = stack->pop();
		CScValue *val2 = stack->pop();
		CScValue *val3 = stack->pop();
		CScValue *val4 = stack->pop();

		if (_responseBox) {
			CAdResponse *res = new CAdResponse(Game);
			if (res) {
				res->_iD = id;
				res->setText(text);
				_stringTable->expand(&res->_text);
				if (!val1->isNULL()) res->setIcon(val1->getString());
				if (!val2->isNULL()) res->setIconHover(val2->getString());
				if (!val3->isNULL()) res->setIconPressed(val3->getString());
				if (!val4->isNULL()) res->setFont(val4->getString());

				if (strcmp(name, "AddResponseOnce") == 0) res->_responseType = RESPONSE_ONCE;
				else if (strcmp(name, "AddResponseOnceGame") == 0) res->_responseType = RESPONSE_ONCE_GAME;

				_responseBox->_responses.add(res);
			}
		} else {
			script->runtimeError("Game.AddResponse: response box is not defined");
		}
		stack->pushNULL();

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ResetResponse
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ResetResponse") == 0) {
		stack->correctParams(1);
		int ID = stack->pop()->getInt(-1);
		resetResponse(ID);
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ClearResponses
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ClearResponses") == 0) {
		stack->correctParams(0);
		_responseBox->clearResponses();
		_responseBox->clearButtons();
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetResponse
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetResponse") == 0) {
		stack->correctParams(1);
		bool autoSelectLast = stack->pop()->getBool();

		if (_responseBox) {
			_responseBox->weedResponses();

			if (_responseBox->_responses.getSize() == 0) {
				stack->pushNULL();
				return STATUS_OK;
			}


			if (_responseBox->_responses.getSize() == 1 && autoSelectLast) {
				stack->pushInt(_responseBox->_responses[0]->_iD);
				_responseBox->handleResponse(_responseBox->_responses[0]);
				_responseBox->clearResponses();
				return STATUS_OK;
			}

			_responseBox->createButtons();
			_responseBox->_waitingScript = script;
			script->waitForExclusive(_responseBox);
			_state = GAME_SEMI_FROZEN;
			_stateEx = GAME_WAITING_RESPONSE;
		} else {
			script->runtimeError("Game.GetResponse: response box is not defined");
			stack->pushNULL();
		}
		return STATUS_OK;
	}


	//////////////////////////////////////////////////////////////////////////
	// GetNumResponses
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetNumResponses") == 0) {
		stack->correctParams(0);
		if (_responseBox) {
			_responseBox->weedResponses();
			stack->pushInt(_responseBox->_responses.getSize());
		} else {
			script->runtimeError("Game.GetNumResponses: response box is not defined");
			stack->pushNULL();
		}
		return STATUS_OK;
	}


	//////////////////////////////////////////////////////////////////////////
	// StartDlgBranch
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "StartDlgBranch") == 0) {
		stack->correctParams(1);
		CScValue *val = stack->pop();
		Common::String branchName;
		if (val->isNULL()) {
			branchName.format("line%d", script->_currentLine);
		} else branchName = val->getString();

		startDlgBranch(branchName.c_str(), script->_filename == NULL ? "" : script->_filename, script->_threadEvent == NULL ? "" : script->_threadEvent);
		stack->pushNULL();

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// EndDlgBranch
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "EndDlgBranch") == 0) {
		stack->correctParams(1);

		const char *branchName = NULL;
		CScValue *val = stack->pop();
		if (!val->isNULL()) branchName = val->getString();
		endDlgBranch(branchName, script->_filename == NULL ? "" : script->_filename, script->_threadEvent == NULL ? "" : script->_threadEvent);

		stack->pushNULL();

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetCurrentDlgBranch
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetCurrentDlgBranch") == 0) {
		stack->correctParams(0);

		if (_dlgPendingBranches.getSize() > 0) {
			stack->pushString(_dlgPendingBranches[_dlgPendingBranches.getSize() - 1]);
		} else stack->pushNULL();

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// TakeItem
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "TakeItem") == 0) {
		return _invObject->scCallMethod(script, stack, thisStack, name);
	}

	//////////////////////////////////////////////////////////////////////////
	// DropItem
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DropItem") == 0) {
		return _invObject->scCallMethod(script, stack, thisStack, name);
	}

	//////////////////////////////////////////////////////////////////////////
	// GetItem
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetItem") == 0) {
		return _invObject->scCallMethod(script, stack, thisStack, name);
	}

	//////////////////////////////////////////////////////////////////////////
	// HasItem
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "HasItem") == 0) {
		return _invObject->scCallMethod(script, stack, thisStack, name);
	}

	//////////////////////////////////////////////////////////////////////////
	// IsItemTaken
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "IsItemTaken") == 0) {
		stack->correctParams(1);

		CScValue *val = stack->pop();
		if (!val->isNULL()) {
			for (int i = 0; i < _inventories.getSize(); i++) {
				CAdInventory *Inv = _inventories[i];

				for (int j = 0; j < Inv->_takenItems.getSize(); j++) {
					if (val->getNative() == Inv->_takenItems[j]) {
						stack->pushBool(true);
						return STATUS_OK;
					} else if (scumm_stricmp(val->getString(), Inv->_takenItems[j]->_name) == 0) {
						stack->pushBool(true);
						return STATUS_OK;
					}
				}
			}
		} else script->runtimeError("Game.IsItemTaken: item name expected");

		stack->pushBool(false);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetInventoryWindow
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetInventoryWindow") == 0) {
		stack->correctParams(0);
		if (_inventoryBox && _inventoryBox->_window)
			stack->pushNative(_inventoryBox->_window, true);
		else
			stack->pushNULL();

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetResponsesWindow
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetResponsesWindow") == 0 || strcmp(name, "GetResponseWindow") == 0) {
		stack->correctParams(0);
		if (_responseBox && _responseBox->_window)
			stack->pushNative(_responseBox->_window, true);
		else
			stack->pushNULL();

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LoadResponseBox
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LoadResponseBox") == 0) {
		stack->correctParams(1);
		const char *filename = stack->pop()->getString();

		Game->unregisterObject(_responseBox);
		_responseBox = new CAdResponseBox(Game);
		if (_responseBox && !DID_FAIL(_responseBox->loadFile(filename))) {
			registerObject(_responseBox);
			stack->pushBool(true);
		} else {
			delete _responseBox;
			_responseBox = NULL;
			stack->pushBool(false);
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LoadInventoryBox
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LoadInventoryBox") == 0) {
		stack->correctParams(1);
		const char *filename = stack->pop()->getString();

		Game->unregisterObject(_inventoryBox);
		_inventoryBox = new CAdInventoryBox(Game);
		if (_inventoryBox && !DID_FAIL(_inventoryBox->loadFile(filename))) {
			registerObject(_inventoryBox);
			stack->pushBool(true);
		} else {
			delete _inventoryBox;
			_inventoryBox = NULL;
			stack->pushBool(false);
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LoadItems
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LoadItems") == 0) {
		stack->correctParams(2);
		const char *filename = stack->pop()->getString();
		bool merge = stack->pop()->getBool(false);

		ERRORCODE ret = loadItemsFile(filename, merge);
		stack->pushBool(DID_SUCCEED(ret));

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AddSpeechDir
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AddSpeechDir") == 0) {
		stack->correctParams(1);
		const char *dir = stack->pop()->getString();
		stack->pushBool(DID_SUCCEED(addSpeechDir(dir)));

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RemoveSpeechDir
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "RemoveSpeechDir") == 0) {
		stack->correctParams(1);
		const char *dir = stack->pop()->getString();
		stack->pushBool(DID_SUCCEED(removeSpeechDir(dir)));

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetSceneViewport
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetSceneViewport") == 0) {
		stack->correctParams(4);
		int x = stack->pop()->getInt();
		int y = stack->pop()->getInt();
		int width = stack->pop()->getInt();
		int height = stack->pop()->getInt();

		if (width <= 0) width = _renderer->_width;
		if (height <= 0) height = _renderer->_height;

		if (!_sceneViewport) _sceneViewport = new CBViewport(Game);
		if (_sceneViewport) _sceneViewport->setRect(x, y, x + width, y + height);

		stack->pushBool(true);

		return STATUS_OK;
	}


	else return CBGame::scCallMethod(script, stack, thisStack, name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CAdGame::scGetProperty(const char *name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Type") == 0) {
		_scValue->setString("game");
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Scene
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Scene") == 0) {
		if (_scene) _scValue->setNative(_scene, true);
		else _scValue->setNULL();

		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// SelectedItem
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SelectedItem") == 0) {
		//if(_selectedItem) _scValue->setString(_selectedItem->_name);
		if (_selectedItem) _scValue->setNative(_selectedItem, true);
		else _scValue->setNULL();

		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// NumItems
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "NumItems") == 0) {
		return _invObject->scGetProperty(name);
	}

	//////////////////////////////////////////////////////////////////////////
	// SmartItemCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SmartItemCursor") == 0) {
		_scValue->setBool(_smartItemCursor);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// InventoryVisible
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "InventoryVisible") == 0) {
		_scValue->setBool(_inventoryBox && _inventoryBox->_visible);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// InventoryScrollOffset
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "InventoryScrollOffset") == 0) {
		if (_inventoryBox) _scValue->setInt(_inventoryBox->_scrollOffset);
		else _scValue->setInt(0);

		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ResponsesVisible (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ResponsesVisible") == 0) {
		_scValue->setBool(_stateEx == GAME_WAITING_RESPONSE);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// PrevScene / PreviousScene (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PrevScene") == 0 || strcmp(name, "PreviousScene") == 0) {
		if (!_prevSceneName) _scValue->setString("");
		else _scValue->setString(_prevSceneName);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// PrevSceneFilename / PreviousSceneFilename (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PrevSceneFilename") == 0 || strcmp(name, "PreviousSceneFilename") == 0) {
		if (!_prevSceneFilename) _scValue->setString("");
		else _scValue->setString(_prevSceneFilename);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// LastResponse (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LastResponse") == 0) {
		if (!_responseBox || !_responseBox->_lastResponseText) _scValue->setString("");
		else _scValue->setString(_responseBox->_lastResponseText);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// LastResponseOrig (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LastResponseOrig") == 0) {
		if (!_responseBox || !_responseBox->_lastResponseTextOrig) _scValue->setString("");
		else _scValue->setString(_responseBox->_lastResponseTextOrig);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// InventoryObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "InventoryObject") == 0) {
		if (_inventoryOwner == _invObject) _scValue->setNative(this, true);
		else _scValue->setNative(_inventoryOwner, true);

		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// TotalNumItems
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "TotalNumItems") == 0) {
		_scValue->setInt(_items.getSize());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// TalkSkipButton
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "TalkSkipButton") == 0) {
		_scValue->setInt(_talkSkipButton);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ChangingScene
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ChangingScene") == 0) {
		_scValue->setBool(_scheduledScene != NULL);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// StartupScene
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "StartupScene") == 0) {
		if (!_startupScene) _scValue->setNULL();
		else _scValue->setString(_startupScene);
		return _scValue;
	}

	else return CBGame::scGetProperty(name);
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::scSetProperty(const char *name, CScValue *value) {

	//////////////////////////////////////////////////////////////////////////
	// SelectedItem
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SelectedItem") == 0) {
		if (value->isNULL()) _selectedItem = NULL;
		else {
			if (value->isNative()) {
				_selectedItem = NULL;
				for (int i = 0; i < _items.getSize(); i++) {
					if (_items[i] == value->getNative()) {
						_selectedItem = (CAdItem *)value->getNative();
						break;
					}
				}
			} else {
				// try to get by name
				_selectedItem = getItemByName(value->getString());
			}
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SmartItemCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SmartItemCursor") == 0) {
		_smartItemCursor = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// InventoryVisible
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "InventoryVisible") == 0) {
		if (_inventoryBox) _inventoryBox->_visible = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// InventoryObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "InventoryObject") == 0) {
		if (_inventoryOwner && _inventoryBox) _inventoryOwner->getInventory()->_scrollOffset = _inventoryBox->_scrollOffset;

		if (value->isNULL()) _inventoryOwner = _invObject;
		else {
			CBObject *Obj = (CBObject *)value->getNative();
			if (Obj == this) _inventoryOwner = _invObject;
			else if (Game->validObject(Obj)) _inventoryOwner = (CAdObject *)Obj;
		}

		if (_inventoryOwner && _inventoryBox) _inventoryBox->_scrollOffset = _inventoryOwner->getInventory()->_scrollOffset;

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// InventoryScrollOffset
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "InventoryScrollOffset") == 0) {
		if (_inventoryBox) _inventoryBox->_scrollOffset = value->getInt();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// TalkSkipButton
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "TalkSkipButton") == 0) {
		int Val = value->getInt();
		if (Val < 0) Val = 0;
		if (Val > TALK_SKIP_NONE) Val = TALK_SKIP_NONE;
		_talkSkipButton = (TTalkSkipButton)Val;
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// StartupScene
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "StartupScene") == 0) {
		if (value == NULL) {
			delete[] _startupScene;
			_startupScene = NULL;
		} else CBUtils::setString(&_startupScene, value->getString());

		return STATUS_OK;
	}

	else return CBGame::scSetProperty(name, value);
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::ExternalCall(CScScript *script, CScStack *stack, CScStack *thisStack, char *name) {
	CScValue *this_obj;

	//////////////////////////////////////////////////////////////////////////
	// Actor
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Actor") == 0) {
		stack->correctParams(0);
		this_obj = thisStack->getTop();

		this_obj->setNative(new CAdActor(Game));
		stack->pushNULL();
	}

	//////////////////////////////////////////////////////////////////////////
	// Entity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Entity") == 0) {
		stack->correctParams(0);
		this_obj = thisStack->getTop();

		this_obj->setNative(new CAdEntity(Game));
		stack->pushNULL();
	}


	//////////////////////////////////////////////////////////////////////////
	// call parent
	else return CBGame::ExternalCall(script, stack, thisStack, name);


	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::showCursor() {
	if (_cursorHidden) return STATUS_OK;

	if (_selectedItem && Game->_state == GAME_RUNNING && _stateEx == GAME_NORMAL && _interactive) {
		if (_selectedItem->_cursorCombined) {
			CBSprite *origLastCursor = _lastCursor;
			CBGame::showCursor();
			_lastCursor = origLastCursor;
		}
		if (_activeObject && _selectedItem->_cursorHover && _activeObject->getExtendedFlag("usable")) {
			if (!_smartItemCursor || _activeObject->canHandleEvent(_selectedItem->_name))
				return drawCursor(_selectedItem->_cursorHover);
			else
				return drawCursor(_selectedItem->_cursorNormal);
		} else return drawCursor(_selectedItem->_cursorNormal);
	} else return CBGame::showCursor();
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::loadFile(const char *filename) {
	byte *buffer = _fileManager->readWholeFile(filename);
	if (buffer == NULL) {
		Game->LOG(0, "CAdGame::LoadFile failed for file '%s'", filename);
		return STATUS_FAILED;
	}

	ERRORCODE ret;

	_filename = new char [strlen(filename) + 1];
	strcpy(_filename, filename);

	if (DID_FAIL(ret = loadBuffer(buffer, true))) Game->LOG(0, "Error parsing GAME file '%s'", filename);


	delete [] buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(GAME)
TOKEN_DEF(AD_GAME)
TOKEN_DEF(RESPONSE_BOX)
TOKEN_DEF(INVENTORY_BOX)
TOKEN_DEF(ITEMS)
TOKEN_DEF(ITEM)
TOKEN_DEF(TALK_SKIP_BUTTON)
TOKEN_DEF(SCENE_VIEWPORT)
TOKEN_DEF(ENTITY_CONTAINER)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF(STARTUP_SCENE)
TOKEN_DEF(DEBUG_STARTUP_SCENE)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::loadBuffer(byte *buffer, bool complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(GAME)
	TOKEN_TABLE(AD_GAME)
	TOKEN_TABLE(RESPONSE_BOX)
	TOKEN_TABLE(INVENTORY_BOX)
	TOKEN_TABLE(ITEMS)
	TOKEN_TABLE(TALK_SKIP_BUTTON)
	TOKEN_TABLE(SCENE_VIEWPORT)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE(STARTUP_SCENE)
	TOKEN_TABLE(DEBUG_STARTUP_SCENE)
	TOKEN_TABLE_END

	byte *params;
	byte *params2;
	int cmd = 1;
	CBParser parser(Game);

	bool itemFound = false, itemsFound = false;

	while (cmd > 0 && (cmd = parser.getCommand((char **)&buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_GAME:
			if (DID_FAIL(CBGame::loadBuffer(params, false))) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_AD_GAME:
			while (cmd > 0 && (cmd = parser.getCommand((char **)&params, commands, (char **)&params2)) > 0) {
				switch (cmd) {
				case TOKEN_RESPONSE_BOX:
					delete _responseBox;
					_responseBox = new CAdResponseBox(Game);
					if (_responseBox && !DID_FAIL(_responseBox->loadFile((char *)params2)))
						registerObject(_responseBox);
					else {
						delete _responseBox;
						_responseBox = NULL;
						cmd = PARSERR_GENERIC;
					}
					break;

				case TOKEN_INVENTORY_BOX:
					delete _inventoryBox;
					_inventoryBox = new CAdInventoryBox(Game);
					if (_inventoryBox && !DID_FAIL(_inventoryBox->loadFile((char *)params2)))
						registerObject(_inventoryBox);
					else {
						delete _inventoryBox;
						_inventoryBox = NULL;
						cmd = PARSERR_GENERIC;
					}
					break;

				case TOKEN_ITEMS:
					itemsFound = true;
					CBUtils::setString(&_itemsFile, (char *)params2);
					if (DID_FAIL(loadItemsFile(_itemsFile))) {
						delete[] _itemsFile;
						_itemsFile = NULL;
						cmd = PARSERR_GENERIC;
					}
					break;

				case TOKEN_TALK_SKIP_BUTTON:
					if (scumm_stricmp((char *)params2, "right") == 0) _talkSkipButton = TALK_SKIP_RIGHT;
					else if (scumm_stricmp((char *)params2, "both") == 0) _talkSkipButton = TALK_SKIP_BOTH;
					else _talkSkipButton = TALK_SKIP_LEFT;
					break;

				case TOKEN_SCENE_VIEWPORT: {
					Common::Rect rc;
					parser.scanStr((char *)params2, "%d,%d,%d,%d", &rc.left, &rc.top, &rc.right, &rc.bottom);
					if (!_sceneViewport) _sceneViewport = new CBViewport(Game);
					if (_sceneViewport) _sceneViewport->setRect(rc.left, rc.top, rc.right, rc.bottom);
				}
				break;

				case TOKEN_EDITOR_PROPERTY:
					parseEditorProperty(params2, false);
					break;

				case TOKEN_STARTUP_SCENE:
					CBUtils::setString(&_startupScene, (char *)params2);
					break;

				case TOKEN_DEBUG_STARTUP_SCENE:
					CBUtils::setString(&_debugStartupScene, (char *)params2);
					break;
				}
			}
			break;
		}
	}

	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in GAME definition");
		return STATUS_FAILED;
	}
	if (cmd == PARSERR_GENERIC) {
		Game->LOG(0, "Error loading GAME definition");
		return STATUS_FAILED;
	}

	if (itemFound && !itemsFound) {
		Game->LOG(0, "**Warning** Please put the items definition to a separate file.");
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::persist(CBPersistMgr *persistMgr) {
	if (!persistMgr->_saving) cleanup();
	CBGame::persist(persistMgr);

	_dlgPendingBranches.persist(persistMgr);

	_inventories.persist(persistMgr);
	persistMgr->transfer(TMEMBER(_inventoryBox));

	_objects.persist(persistMgr);

	persistMgr->transfer(TMEMBER(_prevSceneName));
	persistMgr->transfer(TMEMBER(_prevSceneFilename));

	persistMgr->transfer(TMEMBER(_responseBox));
	_responsesBranch.persist(persistMgr);
	_responsesGame.persist(persistMgr);
	persistMgr->transfer(TMEMBER(_scene));
	_sceneStates.persist(persistMgr);
	persistMgr->transfer(TMEMBER(_scheduledFadeIn));
	persistMgr->transfer(TMEMBER(_scheduledScene));
	persistMgr->transfer(TMEMBER(_selectedItem));
	persistMgr->transfer(TMEMBER_INT(_talkSkipButton));

	_sentences.persist(persistMgr);

	persistMgr->transfer(TMEMBER(_sceneViewport));
	persistMgr->transfer(TMEMBER_INT(_stateEx));
	persistMgr->transfer(TMEMBER(_initialScene));
	persistMgr->transfer(TMEMBER(_debugStartupScene));

	persistMgr->transfer(TMEMBER(_invObject));
	persistMgr->transfer(TMEMBER(_inventoryOwner));
	persistMgr->transfer(TMEMBER(_tempDisableSaveState));
	_items.persist(persistMgr);

	persistMgr->transfer(TMEMBER(_itemsFile));

	_speechDirs.persist(persistMgr);
	persistMgr->transfer(TMEMBER(_smartItemCursor));

	if (!persistMgr->_saving) _initialScene = false;

	persistMgr->transfer(TMEMBER(_startupScene));


	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::loadGame(const char *filename) {
	ERRORCODE ret = CBGame::loadGame(filename);
	if (DID_SUCCEED(ret)) CSysClassRegistry::getInstance()->enumInstances(afterLoadRegion, "CAdRegion", NULL);
	return ret;
}

//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::initAfterLoad() {
	CBGame::initAfterLoad();
	CSysClassRegistry::getInstance()->enumInstances(afterLoadScene,   "CAdScene",   NULL);
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
void CAdGame::afterLoadScene(void *scene, void *data) {
	((CAdScene *)scene)->afterLoad();
}


//////////////////////////////////////////////////////////////////////////
void CAdGame::setPrevSceneName(const char *name) {
	delete[] _prevSceneName;
	_prevSceneName = NULL;
	if (name) {
		_prevSceneName = new char[strlen(name) + 1];
		if (_prevSceneName) strcpy(_prevSceneName, name);
	}
}


//////////////////////////////////////////////////////////////////////////
void CAdGame::setPrevSceneFilename(const char *name) {
	delete[] _prevSceneFilename;
	_prevSceneFilename = NULL;
	if (name) {
		_prevSceneFilename = new char[strlen(name) + 1];
		if (_prevSceneFilename) strcpy(_prevSceneFilename, name);
	}
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::scheduleChangeScene(const char *filename, bool fadeIn) {
	delete[] _scheduledScene;
	_scheduledScene = NULL;

	if (_scene && !_scene->_initialized) return changeScene(filename, fadeIn);
	else {
		_scheduledScene = new char [strlen(filename) + 1];
		strcpy(_scheduledScene, filename);

		_scheduledFadeIn = fadeIn;

		return STATUS_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::getVersion(byte *verMajor, byte *verMinor, byte *extMajor, byte *extMinor) {
	CBGame::getVersion(verMajor, verMinor, NULL, NULL);

	if (extMajor) *extMajor = 0;
	if (extMinor) *extMinor = 0;

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::loadItemsFile(const char *filename, bool merge) {
	byte *buffer = Game->_fileManager->readWholeFile(filename);
	if (buffer == NULL) {
		Game->LOG(0, "CAdGame::LoadItemsFile failed for file '%s'", filename);
		return STATUS_FAILED;
	}

	ERRORCODE ret;

	//_filename = new char [strlen(filename)+1];
	//strcpy(_filename, filename);

	if (DID_FAIL(ret = loadItemsBuffer(buffer, merge))) Game->LOG(0, "Error parsing ITEMS file '%s'", filename);


	delete [] buffer;

	return ret;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::loadItemsBuffer(byte *buffer, bool merge) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(ITEM)
	TOKEN_TABLE_END

	byte *params;
	int cmd;
	CBParser parser(Game);

	if (!merge) {
		while (_items.getSize() > 0) deleteItem(_items[0]);
	}

	while ((cmd = parser.getCommand((char **)&buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_ITEM: {
			CAdItem *item = new CAdItem(Game);
			if (item && !DID_FAIL(item->loadBuffer(params, false))) {
				// delete item with the same name, if exists
				if (merge) {
					CAdItem *PrevItem = getItemByName(item->_name);
					if (PrevItem) deleteItem(PrevItem);
				}
				addItem(item);
			} else {
				delete item;
				item = NULL;
				cmd = PARSERR_GENERIC;
			}
		}
		break;
		}
	}

	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in ITEMS definition");
		return STATUS_FAILED;
	}
	if (cmd == PARSERR_GENERIC) {
		Game->LOG(0, "Error loading ITEMS definition");
		return STATUS_FAILED;
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
CAdSceneState *CAdGame::getSceneState(const char *filename, bool saving) {
	char *filenameCor = new char[strlen(filename) + 1];
	strcpy(filenameCor, filename);
	for (uint32 i = 0; i < strlen(filenameCor); i++) {
		if (filenameCor[i] == '/') filenameCor[i] = '\\';
	}

	for (int i = 0; i < _sceneStates.getSize(); i++) {
		if (scumm_stricmp(_sceneStates[i]->_filename, filenameCor) == 0) {
			delete [] filenameCor;
			return _sceneStates[i];
		}
	}

	if (saving) {
		CAdSceneState *ret = new CAdSceneState(Game);
		ret->setFilename(filenameCor);

		_sceneStates.add(ret);

		delete [] filenameCor;
		return ret;
	} else {
		delete [] filenameCor;
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::windowLoadHook(CUIWindow *win, char **buffer, char **params) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(ENTITY_CONTAINER)
	TOKEN_TABLE_END

	int cmd = PARSERR_GENERIC;
	CBParser parser(Game);

	cmd = parser.getCommand(buffer, commands, params);
	switch (cmd) {
	case TOKEN_ENTITY_CONTAINER: {
		CUIEntity *ent = new CUIEntity(Game);
		if (!ent || DID_FAIL(ent->loadBuffer((byte *)*params, false))) {
			delete ent;
			ent = NULL;
			cmd = PARSERR_GENERIC;
		} else {
			ent->_parent = win;
			win->_widgets.add(ent);
		}
	}
	break;
	}

	if (cmd == PARSERR_TOKENNOTFOUND || cmd == PARSERR_GENERIC) {
		return STATUS_FAILED;
	}

	return STATUS_OK;

}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::windowScriptMethodHook(CUIWindow *win, CScScript *script, CScStack *stack, const char *name) {
	if (strcmp(name, "CreateEntityContainer") == 0) {
		stack->correctParams(1);
		CScValue *val = stack->pop();

		CUIEntity *ent = new CUIEntity(Game);
		if (!val->isNULL()) ent->setName(val->getString());
		stack->pushNative(ent, true);

		ent->_parent = win;
		win->_widgets.add(ent);

		return STATUS_OK;
	} else return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::startDlgBranch(const char *branchName, const char *scriptName, const char *eventName) {
	char *name = new char[strlen(branchName) + 1 + strlen(scriptName) + 1 + strlen(eventName) + 1];
	if (name) {
		sprintf(name, "%s.%s.%s", branchName, scriptName, eventName);
		_dlgPendingBranches.add(name);
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::endDlgBranch(const char *branchName, const char *scriptName, const char *eventName) {
	char *name = NULL;
	bool deleteName = false;
	if (branchName == NULL && _dlgPendingBranches.getSize() > 0) {
		name = _dlgPendingBranches[_dlgPendingBranches.getSize() - 1];
	} else {
		if (branchName != NULL) {
			name = new char[strlen(branchName) + 1 + strlen(scriptName) + 1 + strlen(eventName) + 1];
			if (name) {
				sprintf(name, "%s.%s.%s", branchName, scriptName, eventName);
				deleteName = true;
			}
		}
	}

	if (name == NULL) return STATUS_OK;


	int startIndex = -1;
	for (int i = _dlgPendingBranches.getSize() - 1; i >= 0; i--) {
		if (scumm_stricmp(name, _dlgPendingBranches[i]) == 0) {
			startIndex = i;
			break;
		}
	}
	if (startIndex >= 0) {
		for (int i = startIndex; i < _dlgPendingBranches.getSize(); i++) {
			//ClearBranchResponses(_dlgPendingBranches[i]);
			delete [] _dlgPendingBranches[i];
			_dlgPendingBranches[i] = NULL;
		}
		_dlgPendingBranches.removeAt(startIndex, _dlgPendingBranches.getSize() - startIndex);
	}

	// dialogue is over, forget selected responses
	if (_dlgPendingBranches.getSize() == 0) {
		for (int i = 0; i < _responsesBranch.getSize(); i++) delete _responsesBranch[i];
		_responsesBranch.removeAll();
	}

	if (deleteName) delete [] name;

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::clearBranchResponses(char *name) {
	for (int i = 0; i < _responsesBranch.getSize(); i++) {
		if (scumm_stricmp(name, _responsesBranch[i]->_context) == 0) {
			delete _responsesBranch[i];
			_responsesBranch.removeAt(i);
			i--;
		}
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::addBranchResponse(int ID) {
	if (branchResponseUsed(ID)) return STATUS_OK;
	CAdResponseContext *r = new CAdResponseContext(Game);
	r->_iD = ID;
	r->setContext(_dlgPendingBranches.getSize() > 0 ? _dlgPendingBranches[_dlgPendingBranches.getSize() - 1] : NULL);
	_responsesBranch.add(r);
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CAdGame::branchResponseUsed(int ID) {
	char *Context = _dlgPendingBranches.getSize() > 0 ? _dlgPendingBranches[_dlgPendingBranches.getSize() - 1] : NULL;
	for (int i = 0; i < _responsesBranch.getSize(); i++) {
		if (_responsesBranch[i]->_iD == ID) {
			if ((Context == NULL && _responsesBranch[i]->_context == NULL) || scumm_stricmp(Context, _responsesBranch[i]->_context) == 0) return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::addGameResponse(int ID) {
	if (gameResponseUsed(ID)) return STATUS_OK;
	CAdResponseContext *r = new CAdResponseContext(Game);
	r->_iD = ID;
	r->setContext(_dlgPendingBranches.getSize() > 0 ? _dlgPendingBranches[_dlgPendingBranches.getSize() - 1] : NULL);
	_responsesGame.add(r);
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CAdGame::gameResponseUsed(int ID) {
	char *Context = _dlgPendingBranches.getSize() > 0 ? _dlgPendingBranches[_dlgPendingBranches.getSize() - 1] : NULL;
	for (int i = 0; i < _responsesGame.getSize(); i++) {
		CAdResponseContext *RespContext = _responsesGame[i];
		if (RespContext->_iD == ID) {
			if ((Context == NULL && RespContext->_context == NULL) || ((Context != NULL && RespContext->_context != NULL) && scumm_stricmp(Context, RespContext->_context) == 0)) return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::resetResponse(int ID) {
	char *Context = _dlgPendingBranches.getSize() > 0 ? _dlgPendingBranches[_dlgPendingBranches.getSize() - 1] : NULL;

	int i;

	for (i = 0; i < _responsesGame.getSize(); i++) {
		if (_responsesGame[i]->_iD == ID) {
			if ((Context == NULL && _responsesGame[i]->_context == NULL) || scumm_stricmp(Context, _responsesGame[i]->_context) == 0) {
				delete _responsesGame[i];
				_responsesGame.removeAt(i);
				break;
			}
		}
	}

	for (i = 0; i < _responsesBranch.getSize(); i++) {
		if (_responsesBranch[i]->_iD == ID) {
			if ((Context == NULL && _responsesBranch[i]->_context == NULL) || scumm_stricmp(Context, _responsesBranch[i]->_context) == 0) {
				delete _responsesBranch[i];
				_responsesBranch.removeAt(i);
				break;
			}
		}
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::displayContent(bool doUpdate, bool displayAll) {
	// init
	if (doUpdate) initLoop();

	// fill black
	_renderer->fill(0, 0, 0);
	if (!_editorMode) _renderer->setScreenViewport();

	// playing exclusive video?
	if (_videoPlayer->isPlaying()) {
		if (doUpdate) _videoPlayer->update();
		_videoPlayer->display();
	} else if (_theoraPlayer) {
		if (_theoraPlayer->isPlaying()) {
			if (doUpdate) _theoraPlayer->update();
			_theoraPlayer->display();
		}
		if (_theoraPlayer->isFinished()) {
			delete _theoraPlayer;
			_theoraPlayer = NULL;
		}
	} else {

		// process scripts
		if (doUpdate) _scEngine->tick();

		Common::Point p;
		getMousePos(&p);

		_scene->update();
		_scene->display();


		// display in-game windows
		displayWindows(true);
		if (_inventoryBox) _inventoryBox->display();
		if (_stateEx == GAME_WAITING_RESPONSE) _responseBox->display();
		if (_indicatorDisplay) displayIndicator();


		if (doUpdate || displayAll) {
			// display normal windows
			displayWindows(false);

			setActiveObject(Game->_renderer->getObjectAt(p.x, p.y));

			// textual info
			displaySentences(_state == GAME_FROZEN);

			showCursor();

			if (_fader) _fader->display();
			_transMgr->update();
		}

	}
	if (_loadingIcon) {
		_loadingIcon->display(_loadingIconX, _loadingIconY);
		if (!_loadingIconPersistent) {
			delete _loadingIcon;
			_loadingIcon = NULL;
		}
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::registerInventory(CAdInventory *inv) {
	for (int i = 0; i < _inventories.getSize(); i++) {
		if (_inventories[i] == inv) return STATUS_OK;
	}
	registerObject(inv);
	_inventories.add(inv);

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::unregisterInventory(CAdInventory *inv) {
	for (int i = 0; i < _inventories.getSize(); i++) {
		if (_inventories[i] == inv) {
			unregisterObject(_inventories[i]);
			_inventories.removeAt(i);
			return STATUS_OK;
		}
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool CAdGame::isItemTaken(char *itemName) {
	for (int i = 0; i < _inventories.getSize(); i++) {
		CAdInventory *Inv = _inventories[i];

		for (int j = 0; j < Inv->_takenItems.getSize(); j++) {
			if (scumm_stricmp(itemName, Inv->_takenItems[j]->_name) == 0) {
				return true;
			}
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
CAdItem *CAdGame::getItemByName(const char *name) {
	for (int i = 0; i < _items.getSize(); i++) {
		if (scumm_stricmp(_items[i]->_name, name) == 0) return _items[i];
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::addItem(CAdItem *item) {
	_items.add(item);
	return Game->registerObject(item);
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::resetContent() {
	// clear pending dialogs
	for (int i = 0; i < _dlgPendingBranches.getSize(); i++) {
		delete [] _dlgPendingBranches[i];
	}
	_dlgPendingBranches.removeAll();


	// clear inventories
	for (int i = 0; i < _inventories.getSize(); i++) {
		_inventories[i]->_takenItems.removeAll();
	}

	// clear scene states
	for (int i = 0; i < _sceneStates.getSize(); i++) delete _sceneStates[i];
	_sceneStates.removeAll();

	// clear once responses
	for (int i = 0; i < _responsesBranch.getSize(); i++) delete _responsesBranch[i];
	_responsesBranch.removeAll();

	// clear once game responses
	for (int i = 0; i < _responsesGame.getSize(); i++) delete _responsesGame[i];
	_responsesGame.removeAll();

	// reload inventory items
	if (_itemsFile) loadItemsFile(_itemsFile);

	_tempDisableSaveState = true;

	return CBGame::resetContent();
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::deleteItem(CAdItem *item) {
	if (!item) return STATUS_FAILED;

	if (_selectedItem == item) _selectedItem = NULL;
	_scene->handleItemAssociations(item->_name, false);

	// remove from all inventories
	for (int i = 0; i < _inventories.getSize(); i++) {
		_inventories[i]->removeItem(item);
	}

	// remove object
	for (int i = 0; i < _items.getSize(); i++) {
		if (_items[i] == item) {
			unregisterObject(_items[i]);
			_items.removeAt(i);
			break;
		}
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::addSpeechDir(const char *dir) {
	if (!dir || dir[0] == '\0') return STATUS_FAILED;

	char *temp = new char[strlen(dir) + 2];
	strcpy(temp, dir);
	if (temp[strlen(temp) - 1] != '\\' && temp[strlen(temp) - 1] != '/')
		strcat(temp, "\\");

	for (int i = 0; i < _speechDirs.getSize(); i++) {
		if (scumm_stricmp(_speechDirs[i], temp) == 0) {
			delete [] temp;
			return STATUS_OK;
		}
	}
	_speechDirs.add(temp);

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::removeSpeechDir(const char *dir) {
	if (!dir || dir[0] == '\0') return STATUS_FAILED;

	char *temp = new char[strlen(dir) + 2];
	strcpy(temp, dir);
	if (temp[strlen(temp) - 1] != '\\' && temp[strlen(temp) - 1] != '/')
		strcat(temp, "\\");

	bool Found = false;
	for (int i = 0; i < _speechDirs.getSize(); i++) {
		if (scumm_stricmp(_speechDirs[i], temp) == 0) {
			delete [] _speechDirs[i];
			_speechDirs.removeAt(i);
			Found = true;
			break;
		}
	}
	delete [] temp;

	if (Found) return STATUS_OK;
	else return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
char *CAdGame::findSpeechFile(char *stringID) {
	char *ret = new char[MAX_PATH_LENGTH];

	for (int i = 0; i < _speechDirs.getSize(); i++) {
		sprintf(ret, "%s%s.ogg", _speechDirs[i], stringID);
		Common::SeekableReadStream *file = _fileManager->openFile(ret); // TODO: Replace with hasFile
		if (file) {
			_fileManager->closeFile(file);
			return ret;
		}

		sprintf(ret, "%s%s.wav", _speechDirs[i], stringID);
		file = _fileManager->openFile(ret);
		if (file) {
			_fileManager->closeFile(file);
			return ret;
		}
	}
	delete [] ret;
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
bool CAdGame::validMouse() {
	Common::Point pos;
	CBPlatform::getCursorPos(&pos);

	return _renderer->pointInViewport(&pos);
}

//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::onMouseLeftDown() {
	if (!validMouse()) return STATUS_OK;
	if (_state == GAME_RUNNING && !_interactive) {
		if (_talkSkipButton == TALK_SKIP_LEFT || _talkSkipButton == TALK_SKIP_BOTH) {
			finishSentences();
		}
		return STATUS_OK;
	}

	if (_activeObject) _activeObject->handleMouse(MOUSE_CLICK, MOUSE_BUTTON_LEFT);

	bool handled = _state == GAME_RUNNING && DID_SUCCEED(applyEvent("LeftClick"));
	if (!handled) {
		if (_activeObject != NULL) {
			_activeObject->applyEvent("LeftClick");
		} else if (_state == GAME_RUNNING && _scene && _scene->pointInViewport(_mousePos.x, _mousePos.y)) {
			_scene->applyEvent("LeftClick");
		}
	}

	if (_activeObject != NULL) Game->_capturedObject = Game->_activeObject;
	_mouseLeftDown = true;
	CBPlatform::setCapture(/*_renderer->_window*/);

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::onMouseLeftUp() {
	if (_activeObject) _activeObject->handleMouse(MOUSE_RELEASE, MOUSE_BUTTON_LEFT);

	CBPlatform::releaseCapture();
	_capturedObject = NULL;
	_mouseLeftDown = false;

	bool handled = /*_state==GAME_RUNNING &&*/ DID_SUCCEED(applyEvent("LeftRelease"));
	if (!handled) {
		if (_activeObject != NULL) {
			_activeObject->applyEvent("LeftRelease");
		} else if (_state == GAME_RUNNING && _scene && _scene->pointInViewport(_mousePos.x, _mousePos.y)) {
			_scene->applyEvent("LeftRelease");
		}
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::onMouseLeftDblClick() {
	if (!validMouse()) return STATUS_OK;

	if (_state == GAME_RUNNING && !_interactive) return STATUS_OK;

	if (_activeObject) _activeObject->handleMouse(MOUSE_DBLCLICK, MOUSE_BUTTON_LEFT);

	bool handled = _state == GAME_RUNNING && DID_SUCCEED(applyEvent("LeftDoubleClick"));
	if (!handled) {
		if (_activeObject != NULL) {
			_activeObject->applyEvent("LeftDoubleClick");
		} else if (_state == GAME_RUNNING && _scene && _scene->pointInViewport(_mousePos.x, _mousePos.y)) {
			_scene->applyEvent("LeftDoubleClick");
		}
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::onMouseRightDown() {
	if (!validMouse()) return STATUS_OK;
	if (_state == GAME_RUNNING && !_interactive) {
		if (_talkSkipButton == TALK_SKIP_RIGHT || _talkSkipButton == TALK_SKIP_BOTH) {
			finishSentences();
		}
		return STATUS_OK;
	}

	if ((_state == GAME_RUNNING && !_interactive) || _stateEx == GAME_WAITING_RESPONSE) return STATUS_OK;

	if (_activeObject) _activeObject->handleMouse(MOUSE_CLICK, MOUSE_BUTTON_RIGHT);

	bool handled = _state == GAME_RUNNING && DID_SUCCEED(applyEvent("RightClick"));
	if (!handled) {
		if (_activeObject != NULL) {
			_activeObject->applyEvent("RightClick");
		} else if (_state == GAME_RUNNING && _scene && _scene->pointInViewport(_mousePos.x, _mousePos.y)) {
			_scene->applyEvent("RightClick");
		}
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::onMouseRightUp() {
	if (_activeObject) _activeObject->handleMouse(MOUSE_RELEASE, MOUSE_BUTTON_RIGHT);

	bool handled = _state == GAME_RUNNING && DID_SUCCEED(applyEvent("RightRelease"));
	if (!handled) {
		if (_activeObject != NULL) {
			_activeObject->applyEvent("RightRelease");
		} else if (_state == GAME_RUNNING && _scene && _scene->pointInViewport(_mousePos.x, _mousePos.y)) {
			_scene->applyEvent("RightRelease");
		}
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::displayDebugInfo() {
	char str[100];
	if (Game->_dEBUG_DebugMode) {
		sprintf(str, "Mouse: %d, %d (scene: %d, %d)", _mousePos.x, _mousePos.y, _mousePos.x + _scene->getOffsetLeft(), _mousePos.y + _scene->getOffsetTop());
		_systemFont->drawText((byte *)str, 0, 90, _renderer->_width, TAL_RIGHT);

		sprintf(str, "Scene: %s (prev: %s)", (_scene && _scene->_name) ? _scene->_name : "???", _prevSceneName ? _prevSceneName : "???");
		_systemFont->drawText((byte *)str, 0, 110, _renderer->_width, TAL_RIGHT);
	}
	return CBGame::displayDebugInfo();
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CAdGame::onScriptShutdown(CScScript *script) {
	if (_responseBox && _responseBox->_waitingScript == script)
		_responseBox->_waitingScript = NULL;

	return STATUS_OK;
}

} // end of namespace WinterMute
