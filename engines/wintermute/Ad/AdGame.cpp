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
	RegisterObject(_scene);

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

	AddSpeechDir("speech");
}


//////////////////////////////////////////////////////////////////////////
CAdGame::~CAdGame() {
	cleanup();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::cleanup() {
	int i;

	for (i = 0; i < _objects.GetSize(); i++) {
		UnregisterObject(_objects[i]);
		_objects[i] = NULL;
	}
	_objects.RemoveAll();


	for (i = 0; i < _dlgPendingBranches.GetSize(); i++) {
		delete [] _dlgPendingBranches[i];
	}
	_dlgPendingBranches.RemoveAll();

	for (i = 0; i < _speechDirs.GetSize(); i++) {
		delete [] _speechDirs[i];
	}
	_speechDirs.RemoveAll();


	UnregisterObject(_scene);
	_scene = NULL;

	// remove items
	for (i = 0; i < _items.GetSize(); i++) Game->UnregisterObject(_items[i]);
	_items.RemoveAll();


	// clear remaining inventories
	delete _invObject;
	_invObject = NULL;

	for (i = 0; i < _inventories.GetSize(); i++) {
		delete _inventories[i];
	}
	_inventories.RemoveAll();


	if (_responseBox) {
		Game->UnregisterObject(_responseBox);
		_responseBox = NULL;
	}

	if (_inventoryBox) {
		Game->UnregisterObject(_inventoryBox);
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

	for (i = 0; i < _sceneStates.GetSize(); i++) delete _sceneStates[i];
	_sceneStates.RemoveAll();

	for (i = 0; i < _responsesBranch.GetSize(); i++) delete _responsesBranch[i];
	_responsesBranch.RemoveAll();

	for (i = 0; i < _responsesGame.GetSize(); i++) delete _responsesGame[i];
	_responsesGame.RemoveAll();

	return CBGame::cleanup();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::InitLoop() {
	if (_scheduledScene && _transMgr->isReady()) {
		ChangeScene(_scheduledScene, _scheduledFadeIn);
		delete[] _scheduledScene;
		_scheduledScene = NULL;

		Game->_activeObject = NULL;
	}


	HRESULT res;
	res = CBGame::initLoop();
	if (FAILED(res)) return res;

	if (_scene) res = _scene->initLoop();

	_sentences.RemoveAll();

	return res;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::AddObject(CAdObject *Object) {
	_objects.Add(Object);
	return RegisterObject(Object);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::RemoveObject(CAdObject *Object) {
	// in case the user called Scene.CreateXXX() and Game.DeleteXXX()
	if (_scene) {
		HRESULT Res = _scene->removeObject(Object);
		if (SUCCEEDED(Res)) return Res;
	}

	for (int i = 0; i < _objects.GetSize(); i++) {
		if (_objects[i] == Object) {
			_objects.RemoveAt(i);
			break;
		}
	}
	return UnregisterObject(Object);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::ChangeScene(const char *Filename, bool FadeIn) {
	if (_scene == NULL) {
		_scene = new CAdScene(Game);
		RegisterObject(_scene);
	} else {
		_scene->applyEvent("SceneShutdown", true);

		SetPrevSceneName(_scene->_name);
		SetPrevSceneFilename(_scene->_filename);

		if (!_tempDisableSaveState) _scene->saveState();
		_tempDisableSaveState = false;
	}

	if (_scene) {
		// reset objects
		for (int i = 0; i < _objects.GetSize(); i++) _objects[i]->reset();

		// reset scene properties
		_scene->_sFXVolume = 100;
		if (_scene->_scProp) _scene->_scProp->cleanup();

		HRESULT ret;
		if (_initialScene && _dEBUG_DebugMode && _debugStartupScene) {
			_initialScene = false;
			ret = _scene->loadFile(_debugStartupScene);
		} else ret = _scene->loadFile(Filename);

		if (SUCCEEDED(ret)) {
			// invalidate references to the original scene
			for (int i = 0; i < _objects.GetSize(); i++) {
				_objects[i]->invalidateCurrRegions();
				_objects[i]->_stickRegion = NULL;
			}

			_scene->loadState();
		}
		if (FadeIn) Game->_transMgr->start(TRANSITION_FADE_IN);
		return ret;
	} else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
void CAdGame::AddSentence(CAdSentence *Sentence) {
	_sentences.Add(Sentence);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::DisplaySentences(bool Frozen) {
	for (int i = 0; i < _sentences.GetSize(); i++) {
		if (Frozen && _sentences[i]->_freezable) continue;
		else _sentences[i]->display();
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CAdGame::FinishSentences() {
	for (int i = 0; i < _sentences.GetSize(); i++) {
		if (_sentences[i]->CanSkip()) {
			_sentences[i]->_duration = 0;
			if (_sentences[i]->_sound) _sentences[i]->_sound->stop();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// ChangeScene
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "ChangeScene") == 0) {
		stack->correctParams(3);
		const char *Filename = stack->pop()->getString();
		CScValue *valFadeOut = stack->pop();
		CScValue *valFadeIn = stack->pop();

		bool TransOut = valFadeOut->isNULL() ? true : valFadeOut->getBool();
		bool TransIn  = valFadeIn->isNULL() ? true : valFadeIn->getBool();

		ScheduleChangeScene(Filename, TransIn);
		if (TransOut) _transMgr->start(TRANSITION_FADE_OUT, true);
		stack->pushNULL();


		//HRESULT ret = ChangeScene(stack->pop()->getString());
		//if(FAILED(ret)) stack->pushBool(false);
		//else stack->pushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LoadActor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LoadActor") == 0) {
		stack->correctParams(1);
		CAdActor *act = new CAdActor(Game);
		if (act && SUCCEEDED(act->loadFile(stack->pop()->getString()))) {
			AddObject(act);
			stack->pushNative(act, true);
		} else {
			delete act;
			act = NULL;
			stack->pushNULL();
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LoadEntity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LoadEntity") == 0) {
		stack->correctParams(1);
		CAdEntity *ent = new CAdEntity(Game);
		if (ent && SUCCEEDED(ent->loadFile(stack->pop()->getString()))) {
			AddObject(ent);
			stack->pushNative(ent, true);
		} else {
			delete ent;
			ent = NULL;
			stack->pushNULL();
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// UnloadObject / UnloadActor / UnloadEntity / DeleteEntity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "UnloadObject") == 0 || strcmp(name, "UnloadActor") == 0 || strcmp(name, "UnloadEntity") == 0 || strcmp(name, "DeleteEntity") == 0) {
		stack->correctParams(1);
		CScValue *val = stack->pop();
		CAdObject *obj = (CAdObject *)val->getNative();
		RemoveObject(obj);
		if (val->getType() == VAL_VARIABLE_REF) val->setNULL();

		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CreateEntity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CreateEntity") == 0) {
		stack->correctParams(1);
		CScValue *Val = stack->pop();

		CAdEntity *Ent = new CAdEntity(Game);
		AddObject(Ent);
		if (!Val->isNULL()) Ent->setName(Val->getString());
		stack->pushNative(Ent, true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CreateItem
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CreateItem") == 0) {
		stack->correctParams(1);
		CScValue *Val = stack->pop();

		CAdItem *Item = new CAdItem(Game);
		AddItem(Item);
		if (!Val->isNULL()) Item->setName(Val->getString());
		stack->pushNative(Item, true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeleteItem
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DeleteItem") == 0) {
		stack->correctParams(1);
		CScValue *Val = stack->pop();

		CAdItem *Item = NULL;
		if (Val->isNative()) Item = (CAdItem *)Val->getNative();
		else Item = GetItemByName(Val->getString());

		if (Item) {
			DeleteItem(Item);
		}

		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// QueryItem
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "QueryItem") == 0) {
		stack->correctParams(1);
		CScValue *Val = stack->pop();

		CAdItem *Item = NULL;
		if (Val->isInt()) {
			int Index = Val->getInt();
			if (Index >= 0 && Index < _items.GetSize()) Item = _items[Index];
		} else {
			Item = GetItemByName(Val->getString());
		}

		if (Item) stack->pushNative(Item, true);
		else stack->pushNULL();

		return S_OK;
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
				res->SetText(text);
				_stringTable->Expand(&res->_text);
				if (!val1->isNULL()) res->SetIcon(val1->getString());
				if (!val2->isNULL()) res->SetIconHover(val2->getString());
				if (!val3->isNULL()) res->SetIconPressed(val3->getString());
				if (!val4->isNULL()) res->SetFont(val4->getString());

				if (strcmp(name, "AddResponseOnce") == 0) res->_responseType = RESPONSE_ONCE;
				else if (strcmp(name, "AddResponseOnceGame") == 0) res->_responseType = RESPONSE_ONCE_GAME;

				_responseBox->_responses.Add(res);
			}
		} else {
			script->RuntimeError("Game.AddResponse: response box is not defined");
		}
		stack->pushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ResetResponse
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ResetResponse") == 0) {
		stack->correctParams(1);
		int ID = stack->pop()->getInt(-1);
		ResetResponse(ID);
		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ClearResponses
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ClearResponses") == 0) {
		stack->correctParams(0);
		_responseBox->clearResponses();
		_responseBox->clearButtons();
		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetResponse
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetResponse") == 0) {
		stack->correctParams(1);
		bool AutoSelectLast = stack->pop()->getBool();

		if (_responseBox) {
			_responseBox->weedResponses();

			if (_responseBox->_responses.GetSize() == 0) {
				stack->pushNULL();
				return S_OK;
			}


			if (_responseBox->_responses.GetSize() == 1 && AutoSelectLast) {
				stack->pushInt(_responseBox->_responses[0]->_iD);
				_responseBox->handleResponse(_responseBox->_responses[0]);
				_responseBox->clearResponses();
				return S_OK;
			}

			_responseBox->createButtons();
			_responseBox->_waitingScript = script;
			script->WaitForExclusive(_responseBox);
			_state = GAME_SEMI_FROZEN;
			_stateEx = GAME_WAITING_RESPONSE;
		} else {
			script->RuntimeError("Game.GetResponse: response box is not defined");
			stack->pushNULL();
		}
		return S_OK;
	}


	//////////////////////////////////////////////////////////////////////////
	// GetNumResponses
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetNumResponses") == 0) {
		stack->correctParams(0);
		if (_responseBox) {
			_responseBox->weedResponses();
			stack->pushInt(_responseBox->_responses.GetSize());
		} else {
			script->RuntimeError("Game.GetNumResponses: response box is not defined");
			stack->pushNULL();
		}
		return S_OK;
	}


	//////////////////////////////////////////////////////////////////////////
	// StartDlgBranch
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "StartDlgBranch") == 0) {
		stack->correctParams(1);
		CScValue *Val = stack->pop();
		Common::String BranchName;
		if (Val->isNULL()) {
			BranchName.format("line%d", script->_currentLine);
		} else BranchName = Val->getString();

		StartDlgBranch(BranchName.c_str(), script->_filename == NULL ? "" : script->_filename, script->_threadEvent == NULL ? "" : script->_threadEvent);
		stack->pushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// EndDlgBranch
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "EndDlgBranch") == 0) {
		stack->correctParams(1);

		const char *BranchName = NULL;
		CScValue *Val = stack->pop();
		if (!Val->isNULL()) BranchName = Val->getString();
		EndDlgBranch(BranchName, script->_filename == NULL ? "" : script->_filename, script->_threadEvent == NULL ? "" : script->_threadEvent);

		stack->pushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetCurrentDlgBranch
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetCurrentDlgBranch") == 0) {
		stack->correctParams(0);

		if (_dlgPendingBranches.GetSize() > 0) {
			stack->pushString(_dlgPendingBranches[_dlgPendingBranches.GetSize() - 1]);
		} else stack->pushNULL();

		return S_OK;
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
			for (int i = 0; i < _inventories.GetSize(); i++) {
				CAdInventory *Inv = _inventories[i];

				for (int j = 0; j < Inv->_takenItems.GetSize(); j++) {
					if (val->getNative() == Inv->_takenItems[j]) {
						stack->pushBool(true);
						return S_OK;
					} else if (scumm_stricmp(val->getString(), Inv->_takenItems[j]->_name) == 0) {
						stack->pushBool(true);
						return S_OK;
					}
				}
			}
		} else script->RuntimeError("Game.IsItemTaken: item name expected");

		stack->pushBool(false);
		return S_OK;
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

		return S_OK;
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

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LoadResponseBox
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LoadResponseBox") == 0) {
		stack->correctParams(1);
		const char *Filename = stack->pop()->getString();

		Game->UnregisterObject(_responseBox);
		_responseBox = new CAdResponseBox(Game);
		if (_responseBox && !FAILED(_responseBox->loadFile(Filename))) {
			RegisterObject(_responseBox);
			stack->pushBool(true);
		} else {
			delete _responseBox;
			_responseBox = NULL;
			stack->pushBool(false);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LoadInventoryBox
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LoadInventoryBox") == 0) {
		stack->correctParams(1);
		const char *Filename = stack->pop()->getString();

		Game->UnregisterObject(_inventoryBox);
		_inventoryBox = new CAdInventoryBox(Game);
		if (_inventoryBox && !FAILED(_inventoryBox->loadFile(Filename))) {
			RegisterObject(_inventoryBox);
			stack->pushBool(true);
		} else {
			delete _inventoryBox;
			_inventoryBox = NULL;
			stack->pushBool(false);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LoadItems
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LoadItems") == 0) {
		stack->correctParams(2);
		const char *Filename = stack->pop()->getString();
		bool Merge = stack->pop()->getBool(false);

		HRESULT Ret = LoadItemsFile(Filename, Merge);
		stack->pushBool(SUCCEEDED(Ret));

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AddSpeechDir
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AddSpeechDir") == 0) {
		stack->correctParams(1);
		const char *Dir = stack->pop()->getString();
		stack->pushBool(SUCCEEDED(AddSpeechDir(Dir)));

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RemoveSpeechDir
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "RemoveSpeechDir") == 0) {
		stack->correctParams(1);
		const char *Dir = stack->pop()->getString();
		stack->pushBool(SUCCEEDED(RemoveSpeechDir(Dir)));

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetSceneViewport
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetSceneViewport") == 0) {
		stack->correctParams(4);
		int X = stack->pop()->getInt();
		int Y = stack->pop()->getInt();
		int Width = stack->pop()->getInt();
		int Height = stack->pop()->getInt();

		if (Width <= 0) Width = _renderer->_width;
		if (Height <= 0) Height = _renderer->_height;

		if (!_sceneViewport) _sceneViewport = new CBViewport(Game);
		if (_sceneViewport) _sceneViewport->setRect(X, Y, X + Width, Y + Height);

		stack->pushBool(true);

		return S_OK;
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
		_scValue->setInt(_items.GetSize());
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
HRESULT CAdGame::scSetProperty(const char *name, CScValue *value) {

	//////////////////////////////////////////////////////////////////////////
	// SelectedItem
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SelectedItem") == 0) {
		if (value->isNULL()) _selectedItem = NULL;
		else {
			if (value->isNative()) {
				_selectedItem = NULL;
				for (int i = 0; i < _items.GetSize(); i++) {
					if (_items[i] == value->getNative()) {
						_selectedItem = (CAdItem *)value->getNative();
						break;
					}
				}
			} else {
				// try to get by name
				_selectedItem = GetItemByName(value->getString());
			}
		}

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SmartItemCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SmartItemCursor") == 0) {
		_smartItemCursor = value->getBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// InventoryVisible
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "InventoryVisible") == 0) {
		if (_inventoryBox) _inventoryBox->_visible = value->getBool();
		return S_OK;
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
			else if (Game->ValidObject(Obj)) _inventoryOwner = (CAdObject *)Obj;
		}

		if (_inventoryOwner && _inventoryBox) _inventoryBox->_scrollOffset = _inventoryOwner->getInventory()->_scrollOffset;

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// InventoryScrollOffset
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "InventoryScrollOffset") == 0) {
		if (_inventoryBox) _inventoryBox->_scrollOffset = value->getInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// TalkSkipButton
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "TalkSkipButton") == 0) {
		int Val = value->getInt();
		if (Val < 0) Val = 0;
		if (Val > TALK_SKIP_NONE) Val = TALK_SKIP_NONE;
		_talkSkipButton = (TTalkSkipButton)Val;
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// StartupScene
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "StartupScene") == 0) {
		if (value == NULL) {
			delete[] _startupScene;
			_startupScene = NULL;
		} else CBUtils::SetString(&_startupScene, value->getString());

		return S_OK;
	}

	else return CBGame::scSetProperty(name, value);
}


//////////////////////////////////////////////////////////////////////////
void CAdGame::PublishNatives() {
	if (!_scEngine || !_scEngine->_compilerAvailable) return;

	CBGame::PublishNatives();

	_scEngine->ExtDefineFunction("Actor");
	_scEngine->ExtDefineFunction("Entity");
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::ExternalCall(CScScript *script, CScStack *stack, CScStack *thisStack, char *name) {
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


	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::showCursor() {
	if (_cursorHidden) return S_OK;

	if (_selectedItem && Game->_state == GAME_RUNNING && _stateEx == GAME_NORMAL && _interactive) {
		if (_selectedItem->_cursorCombined) {
			CBSprite *OrigLastCursor = _lastCursor;
			CBGame::showCursor();
			_lastCursor = OrigLastCursor;
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
HRESULT CAdGame::loadFile(const char *Filename) {
	byte *Buffer = _fileManager->readWholeFile(Filename);
	if (Buffer == NULL) {
		Game->LOG(0, "CAdGame::LoadFile failed for file '%s'", Filename);
		return E_FAIL;
	}

	HRESULT ret;

	_filename = new char [strlen(Filename) + 1];
	strcpy(_filename, Filename);

	if (FAILED(ret = loadBuffer(Buffer, true))) Game->LOG(0, "Error parsing GAME file '%s'", Filename);


	delete [] Buffer;

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
HRESULT CAdGame::loadBuffer(byte  *Buffer, bool Complete) {
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

	bool ItemFound = false, ItemsFound = false;

	while (cmd > 0 && (cmd = parser.GetCommand((char **)&Buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_GAME:
			if (FAILED(CBGame::loadBuffer(params, false))) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_AD_GAME:
			while (cmd > 0 && (cmd = parser.GetCommand((char **)&params, commands, (char **)&params2)) > 0) {
				switch (cmd) {
				case TOKEN_RESPONSE_BOX:
					delete _responseBox;
					_responseBox = new CAdResponseBox(Game);
					if (_responseBox && !FAILED(_responseBox->loadFile((char *)params2)))
						RegisterObject(_responseBox);
					else {
						delete _responseBox;
						_responseBox = NULL;
						cmd = PARSERR_GENERIC;
					}
					break;

				case TOKEN_INVENTORY_BOX:
					delete _inventoryBox;
					_inventoryBox = new CAdInventoryBox(Game);
					if (_inventoryBox && !FAILED(_inventoryBox->loadFile((char *)params2)))
						RegisterObject(_inventoryBox);
					else {
						delete _inventoryBox;
						_inventoryBox = NULL;
						cmd = PARSERR_GENERIC;
					}
					break;

				case TOKEN_ITEMS:
					ItemsFound = true;
					CBUtils::SetString(&_itemsFile, (char *)params2);
					if (FAILED(LoadItemsFile(_itemsFile))) {
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
					RECT rc;
					parser.ScanStr((char *)params2, "%d,%d,%d,%d", &rc.left, &rc.top, &rc.right, &rc.bottom);
					if (!_sceneViewport) _sceneViewport = new CBViewport(Game);
					if (_sceneViewport) _sceneViewport->setRect(rc.left, rc.top, rc.right, rc.bottom);
				}
				break;

				case TOKEN_EDITOR_PROPERTY:
					parseEditorProperty(params2, false);
					break;

				case TOKEN_STARTUP_SCENE:
					CBUtils::SetString(&_startupScene, (char *)params2);
					break;

				case TOKEN_DEBUG_STARTUP_SCENE:
					CBUtils::SetString(&_debugStartupScene, (char *)params2);
					break;
				}
			}
			break;
		}
	}

	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in GAME definition");
		return E_FAIL;
	}
	if (cmd == PARSERR_GENERIC) {
		Game->LOG(0, "Error loading GAME definition");
		return E_FAIL;
	}

	if (ItemFound && !ItemsFound) {
		Game->LOG(0, "**Warning** Please put the items definition to a separate file.");
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::persist(CBPersistMgr *persistMgr) {
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


	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::LoadGame(const char *Filename) {
	HRESULT ret = CBGame::LoadGame(Filename);
	if (SUCCEEDED(ret)) CSysClassRegistry::getInstance()->enumInstances(afterLoadRegion, "CAdRegion", NULL);
	return ret;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::InitAfterLoad() {
	CBGame::InitAfterLoad();
	CSysClassRegistry::getInstance()->enumInstances(AfterLoadScene,   "CAdScene",   NULL);
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
void CAdGame::AfterLoadScene(void *Scene, void *Data) {
	((CAdScene *)Scene)->afterLoad();
}


//////////////////////////////////////////////////////////////////////////
void CAdGame::SetPrevSceneName(const char *name) {
	delete[] _prevSceneName;
	_prevSceneName = NULL;
	if (name) {
		_prevSceneName = new char[strlen(name) + 1];
		if (_prevSceneName) strcpy(_prevSceneName, name);
	}
}


//////////////////////////////////////////////////////////////////////////
void CAdGame::SetPrevSceneFilename(const char *name) {
	delete[] _prevSceneFilename;
	_prevSceneFilename = NULL;
	if (name) {
		_prevSceneFilename = new char[strlen(name) + 1];
		if (_prevSceneFilename) strcpy(_prevSceneFilename, name);
	}
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::ScheduleChangeScene(const char *Filename, bool FadeIn) {
	delete[] _scheduledScene;
	_scheduledScene = NULL;

	if (_scene && !_scene->_initialized) return ChangeScene(Filename, FadeIn);
	else {
		_scheduledScene = new char [strlen(Filename) + 1];
		strcpy(_scheduledScene, Filename);

		_scheduledFadeIn = FadeIn;

		return S_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::GetVersion(byte  *VerMajor, byte *VerMinor, byte *ExtMajor, byte *ExtMinor) {
	CBGame::GetVersion(VerMajor, VerMinor, NULL, NULL);

	if (ExtMajor) *ExtMajor = 0;
	if (ExtMinor) *ExtMinor = 0;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::LoadItemsFile(const char *Filename, bool Merge) {
	byte *Buffer = Game->_fileManager->readWholeFile(Filename);
	if (Buffer == NULL) {
		Game->LOG(0, "CAdGame::LoadItemsFile failed for file '%s'", Filename);
		return E_FAIL;
	}

	HRESULT ret;

	//_filename = new char [strlen(Filename)+1];
	//strcpy(_filename, Filename);

	if (FAILED(ret = LoadItemsBuffer(Buffer, Merge))) Game->LOG(0, "Error parsing ITEMS file '%s'", Filename);


	delete [] Buffer;

	return ret;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::LoadItemsBuffer(byte  *Buffer, bool Merge) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(ITEM)
	TOKEN_TABLE_END

	byte *params;
	int cmd;
	CBParser parser(Game);

	if (!Merge) {
		while (_items.GetSize() > 0) DeleteItem(_items[0]);
	}

	while ((cmd = parser.GetCommand((char **)&Buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_ITEM: {
			CAdItem *item = new CAdItem(Game);
			if (item && !FAILED(item->loadBuffer(params, false))) {
				// delete item with the same name, if exists
				if (Merge) {
					CAdItem *PrevItem = GetItemByName(item->_name);
					if (PrevItem) DeleteItem(PrevItem);
				}
				AddItem(item);
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
		return E_FAIL;
	}
	if (cmd == PARSERR_GENERIC) {
		Game->LOG(0, "Error loading ITEMS definition");
		return E_FAIL;
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
CAdSceneState *CAdGame::GetSceneState(const char *Filename, bool Saving) {
	char *FilenameCor = new char[strlen(Filename) + 1];
	strcpy(FilenameCor, Filename);
	for (uint32 i = 0; i < strlen(FilenameCor); i++) {
		if (FilenameCor[i] == '/') FilenameCor[i] = '\\';
	}

	for (int i = 0; i < _sceneStates.GetSize(); i++) {
		if (scumm_stricmp(_sceneStates[i]->_filename, FilenameCor) == 0) {
			delete [] FilenameCor;
			return _sceneStates[i];
		}
	}

	if (Saving) {
		CAdSceneState *ret = new CAdSceneState(Game);
		ret->setFilename(FilenameCor);

		_sceneStates.Add(ret);

		delete [] FilenameCor;
		return ret;
	} else {
		delete [] FilenameCor;
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::WindowLoadHook(CUIWindow *Win, char **Buffer, char **params) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(ENTITY_CONTAINER)
	TOKEN_TABLE_END

	int cmd = PARSERR_GENERIC;
	CBParser parser(Game);

	cmd = parser.GetCommand(Buffer, commands, params);
	switch (cmd) {
	case TOKEN_ENTITY_CONTAINER: {
		CUIEntity *ent = new CUIEntity(Game);
		if (!ent || FAILED(ent->loadBuffer((byte *)*params, false))) {
			delete ent;
			ent = NULL;
			cmd = PARSERR_GENERIC;
		} else {
			ent->_parent = Win;
			Win->_widgets.Add(ent);
		}
	}
	break;
	}

	if (cmd == PARSERR_TOKENNOTFOUND || cmd == PARSERR_GENERIC) {
		return E_FAIL;
	}

	return S_OK;

}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::WindowScriptMethodHook(CUIWindow *Win, CScScript *script, CScStack *stack, const char *name) {
	if (strcmp(name, "CreateEntityContainer") == 0) {
		stack->correctParams(1);
		CScValue *Val = stack->pop();

		CUIEntity *Ent = new CUIEntity(Game);
		if (!Val->isNULL()) Ent->setName(Val->getString());
		stack->pushNative(Ent, true);

		Ent->_parent = Win;
		Win->_widgets.Add(Ent);

		return S_OK;
	} else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::StartDlgBranch(const char *BranchName, const char *ScriptName, const char *EventName) {
	char *name = new char[strlen(BranchName) + 1 + strlen(ScriptName) + 1 + strlen(EventName) + 1];
	if (name) {
		sprintf(name, "%s.%s.%s", BranchName, ScriptName, EventName);
		_dlgPendingBranches.Add(name);
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::EndDlgBranch(const char *BranchName, const char *ScriptName, const char *EventName) {
	char *name = NULL;
	bool DeleteName = false;
	if (BranchName == NULL && _dlgPendingBranches.GetSize() > 0) {
		name = _dlgPendingBranches[_dlgPendingBranches.GetSize() - 1];
	} else {
		if (BranchName != NULL) {
			name = new char[strlen(BranchName) + 1 + strlen(ScriptName) + 1 + strlen(EventName) + 1];
			if (name) {
				sprintf(name, "%s.%s.%s", BranchName, ScriptName, EventName);
				DeleteName = true;
			}
		}
	}

	if (name == NULL) return S_OK;


	int StartIndex = -1;
	for (int i = _dlgPendingBranches.GetSize() - 1; i >= 0; i--) {
		if (scumm_stricmp(name, _dlgPendingBranches[i]) == 0) {
			StartIndex = i;
			break;
		}
	}
	if (StartIndex >= 0) {
		for (int i = StartIndex; i < _dlgPendingBranches.GetSize(); i++) {
			//ClearBranchResponses(_dlgPendingBranches[i]);
			delete [] _dlgPendingBranches[i];
			_dlgPendingBranches[i] = NULL;
		}
		_dlgPendingBranches.RemoveAt(StartIndex, _dlgPendingBranches.GetSize() - StartIndex);
	}

	// dialogue is over, forget selected responses
	if (_dlgPendingBranches.GetSize() == 0) {
		for (int i = 0; i < _responsesBranch.GetSize(); i++) delete _responsesBranch[i];
		_responsesBranch.RemoveAll();
	}

	if (DeleteName) delete [] name;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::ClearBranchResponses(char *name) {
	for (int i = 0; i < _responsesBranch.GetSize(); i++) {
		if (scumm_stricmp(name, _responsesBranch[i]->_context) == 0) {
			delete _responsesBranch[i];
			_responsesBranch.RemoveAt(i);
			i--;
		}
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::AddBranchResponse(int ID) {
	if (BranchResponseUsed(ID)) return S_OK;
	CAdResponseContext *r = new CAdResponseContext(Game);
	r->_iD = ID;
	r->setContext(_dlgPendingBranches.GetSize() > 0 ? _dlgPendingBranches[_dlgPendingBranches.GetSize() - 1] : NULL);
	_responsesBranch.Add(r);
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CAdGame::BranchResponseUsed(int ID) {
	char *Context = _dlgPendingBranches.GetSize() > 0 ? _dlgPendingBranches[_dlgPendingBranches.GetSize() - 1] : NULL;
	for (int i = 0; i < _responsesBranch.GetSize(); i++) {
		if (_responsesBranch[i]->_iD == ID) {
			if ((Context == NULL && _responsesBranch[i]->_context == NULL) || scumm_stricmp(Context, _responsesBranch[i]->_context) == 0) return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::AddGameResponse(int ID) {
	if (GameResponseUsed(ID)) return S_OK;
	CAdResponseContext *r = new CAdResponseContext(Game);
	r->_iD = ID;
	r->setContext(_dlgPendingBranches.GetSize() > 0 ? _dlgPendingBranches[_dlgPendingBranches.GetSize() - 1] : NULL);
	_responsesGame.Add(r);
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CAdGame::GameResponseUsed(int ID) {
	char *Context = _dlgPendingBranches.GetSize() > 0 ? _dlgPendingBranches[_dlgPendingBranches.GetSize() - 1] : NULL;
	for (int i = 0; i < _responsesGame.GetSize(); i++) {
		CAdResponseContext *RespContext = _responsesGame[i];
		if (RespContext->_iD == ID) {
			if ((Context == NULL && RespContext->_context == NULL) || ((Context != NULL && RespContext->_context != NULL) && scumm_stricmp(Context, RespContext->_context) == 0)) return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::ResetResponse(int ID) {
	char *Context = _dlgPendingBranches.GetSize() > 0 ? _dlgPendingBranches[_dlgPendingBranches.GetSize() - 1] : NULL;

	int i;

	for (i = 0; i < _responsesGame.GetSize(); i++) {
		if (_responsesGame[i]->_iD == ID) {
			if ((Context == NULL && _responsesGame[i]->_context == NULL) || scumm_stricmp(Context, _responsesGame[i]->_context) == 0) {
				delete _responsesGame[i];
				_responsesGame.RemoveAt(i);
				break;
			}
		}
	}

	for (i = 0; i < _responsesBranch.GetSize(); i++) {
		if (_responsesBranch[i]->_iD == ID) {
			if ((Context == NULL && _responsesBranch[i]->_context == NULL) || scumm_stricmp(Context, _responsesBranch[i]->_context) == 0) {
				delete _responsesBranch[i];
				_responsesBranch.RemoveAt(i);
				break;
			}
		}
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::DisplayContent(bool Update, bool DisplayAll) {
	// init
	if (Update) InitLoop();

	// fill black
	_renderer->fill(0, 0, 0);
	if (!_editorMode) _renderer->setScreenViewport();

	// playing exclusive video?
	if (_videoPlayer->isPlaying()) {
		if (Update) _videoPlayer->update();
		_videoPlayer->display();
	} else if (_theoraPlayer) {
		if (_theoraPlayer->isPlaying()) {
			if (Update) _theoraPlayer->update();
			_theoraPlayer->display();
		}
		if (_theoraPlayer->isFinished()) {
			delete _theoraPlayer;
			_theoraPlayer = NULL;
		}
	} else {

		// process scripts
		if (Update) _scEngine->Tick();

		POINT p;
		getMousePos(&p);

		_scene->update();
		_scene->display();


		// display in-game windows
		DisplayWindows(true);
		if (_inventoryBox) _inventoryBox->display();
		if (_stateEx == GAME_WAITING_RESPONSE) _responseBox->display();
		if (_indicatorDisplay) displayIndicator();


		if (Update || DisplayAll) {
			// display normal windows
			DisplayWindows(false);

			setActiveObject(Game->_renderer->getObjectAt(p.x, p.y));

			// textual info
			DisplaySentences(_state == GAME_FROZEN);

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

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::RegisterInventory(CAdInventory *Inv) {
	for (int i = 0; i < _inventories.GetSize(); i++) {
		if (_inventories[i] == Inv) return S_OK;
	}
	RegisterObject(Inv);
	_inventories.Add(Inv);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::UnregisterInventory(CAdInventory *Inv) {
	for (int i = 0; i < _inventories.GetSize(); i++) {
		if (_inventories[i] == Inv) {
			UnregisterObject(_inventories[i]);
			_inventories.RemoveAt(i);
			return S_OK;
		}
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
bool CAdGame::IsItemTaken(char *ItemName) {
	int i;

	for (i = 0; i < _inventories.GetSize(); i++) {
		CAdInventory *Inv = _inventories[i];

		for (int j = 0; j < Inv->_takenItems.GetSize(); j++) {
			if (scumm_stricmp(ItemName, Inv->_takenItems[j]->_name) == 0) {
				return true;
			}
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
CAdItem *CAdGame::GetItemByName(const char *name) {
	for (int i = 0; i < _items.GetSize(); i++) {
		if (scumm_stricmp(_items[i]->_name, name) == 0) return _items[i];
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::AddItem(CAdItem *Item) {
	_items.Add(Item);
	return Game->RegisterObject(Item);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::ResetContent() {
	int i;

	// clear pending dialogs
	for (i = 0; i < _dlgPendingBranches.GetSize(); i++) {
		delete [] _dlgPendingBranches[i];
	}
	_dlgPendingBranches.RemoveAll();


	// clear inventories
	for (i = 0; i < _inventories.GetSize(); i++) {
		_inventories[i]->_takenItems.RemoveAll();
	}

	// clear scene states
	for (i = 0; i < _sceneStates.GetSize(); i++) delete _sceneStates[i];
	_sceneStates.RemoveAll();

	// clear once responses
	for (i = 0; i < _responsesBranch.GetSize(); i++) delete _responsesBranch[i];
	_responsesBranch.RemoveAll();

	// clear once game responses
	for (i = 0; i < _responsesGame.GetSize(); i++) delete _responsesGame[i];
	_responsesGame.RemoveAll();

	// reload inventory items
	if (_itemsFile) LoadItemsFile(_itemsFile);

	_tempDisableSaveState = true;

	return CBGame::ResetContent();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::DeleteItem(CAdItem *Item) {
	if (!Item) return E_FAIL;

	if (_selectedItem == Item) _selectedItem = NULL;
	_scene->handleItemAssociations(Item->_name, false);

	// remove from all inventories
	for (int i = 0; i < _inventories.GetSize(); i++) {
		_inventories[i]->RemoveItem(Item);
	}

	// remove object
	for (int i = 0; i < _items.GetSize(); i++) {
		if (_items[i] == Item) {
			UnregisterObject(_items[i]);
			_items.RemoveAt(i);
			break;
		}
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::AddSpeechDir(const char *Dir) {
	if (!Dir || Dir[0] == '\0') return E_FAIL;

	char *Temp = new char[strlen(Dir) + 2];
	strcpy(Temp, Dir);
	if (Temp[strlen(Temp) - 1] != '\\' && Temp[strlen(Temp) - 1] != '/')
		strcat(Temp, "\\");

	for (int i = 0; i < _speechDirs.GetSize(); i++) {
		if (scumm_stricmp(_speechDirs[i], Temp) == 0) {
			delete [] Temp;
			return S_OK;
		}
	}
	_speechDirs.Add(Temp);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::RemoveSpeechDir(const char *Dir) {
	if (!Dir || Dir[0] == '\0') return E_FAIL;

	char *Temp = new char[strlen(Dir) + 2];
	strcpy(Temp, Dir);
	if (Temp[strlen(Temp) - 1] != '\\' && Temp[strlen(Temp) - 1] != '/')
		strcat(Temp, "\\");

	bool Found = false;
	for (int i = 0; i < _speechDirs.GetSize(); i++) {
		if (scumm_stricmp(_speechDirs[i], Temp) == 0) {
			delete [] _speechDirs[i];
			_speechDirs.RemoveAt(i);
			Found = true;
			break;
		}
	}
	delete [] Temp;

	if (Found) return S_OK;
	else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
char *CAdGame::FindSpeechFile(char *StringID) {
	char *Ret = new char[MAX_PATH];

	for (int i = 0; i < _speechDirs.GetSize(); i++) {
		sprintf(Ret, "%s%s.ogg", _speechDirs[i], StringID);
		Common::SeekableReadStream *File = _fileManager->openFile(Ret);
		if (File) {
			_fileManager->closeFile(File);
			return Ret;
		}

		sprintf(Ret, "%s%s.wav", _speechDirs[i], StringID);
		File = _fileManager->openFile(Ret);
		if (File) {
			_fileManager->closeFile(File);
			return Ret;
		}
	}
	delete [] Ret;
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
bool CAdGame::ValidMouse() {
	POINT Pos;
	CBPlatform::GetCursorPos(&Pos);

	return _renderer->pointInViewport(&Pos);
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::OnMouseLeftDown() {
	if (!ValidMouse()) return S_OK;
	if (_state == GAME_RUNNING && !_interactive) {
		if (_talkSkipButton == TALK_SKIP_LEFT || _talkSkipButton == TALK_SKIP_BOTH) {
			FinishSentences();
		}
		return S_OK;
	}

	if (_activeObject) _activeObject->handleMouse(MOUSE_CLICK, MOUSE_BUTTON_LEFT);

	bool Handled = _state == GAME_RUNNING && SUCCEEDED(applyEvent("LeftClick"));
	if (!Handled) {
		if (_activeObject != NULL) {
			_activeObject->applyEvent("LeftClick");
		} else if (_state == GAME_RUNNING && _scene && _scene->pointInViewport(_mousePos.x, _mousePos.y)) {
			_scene->applyEvent("LeftClick");
		}
	}

	if (_activeObject != NULL) Game->_capturedObject = Game->_activeObject;
	_mouseLeftDown = true;
	CBPlatform::SetCapture(_renderer->_window);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::OnMouseLeftUp() {
	if (_activeObject) _activeObject->handleMouse(MOUSE_RELEASE, MOUSE_BUTTON_LEFT);

	CBPlatform::ReleaseCapture();
	_capturedObject = NULL;
	_mouseLeftDown = false;

	bool Handled = /*_state==GAME_RUNNING &&*/ SUCCEEDED(applyEvent("LeftRelease"));
	if (!Handled) {
		if (_activeObject != NULL) {
			_activeObject->applyEvent("LeftRelease");
		} else if (_state == GAME_RUNNING && _scene && _scene->pointInViewport(_mousePos.x, _mousePos.y)) {
			_scene->applyEvent("LeftRelease");
		}
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::OnMouseLeftDblClick() {
	if (!ValidMouse()) return S_OK;

	if (_state == GAME_RUNNING && !_interactive) return S_OK;

	if (_activeObject) _activeObject->handleMouse(MOUSE_DBLCLICK, MOUSE_BUTTON_LEFT);

	bool Handled = _state == GAME_RUNNING && SUCCEEDED(applyEvent("LeftDoubleClick"));
	if (!Handled) {
		if (_activeObject != NULL) {
			_activeObject->applyEvent("LeftDoubleClick");
		} else if (_state == GAME_RUNNING && _scene && _scene->pointInViewport(_mousePos.x, _mousePos.y)) {
			_scene->applyEvent("LeftDoubleClick");
		}
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::OnMouseRightDown() {
	if (!ValidMouse()) return S_OK;
	if (_state == GAME_RUNNING && !_interactive) {
		if (_talkSkipButton == TALK_SKIP_RIGHT || _talkSkipButton == TALK_SKIP_BOTH) {
			FinishSentences();
		}
		return S_OK;
	}

	if ((_state == GAME_RUNNING && !_interactive) || _stateEx == GAME_WAITING_RESPONSE) return S_OK;

	if (_activeObject) _activeObject->handleMouse(MOUSE_CLICK, MOUSE_BUTTON_RIGHT);

	bool Handled = _state == GAME_RUNNING && SUCCEEDED(applyEvent("RightClick"));
	if (!Handled) {
		if (_activeObject != NULL) {
			_activeObject->applyEvent("RightClick");
		} else if (_state == GAME_RUNNING && _scene && _scene->pointInViewport(_mousePos.x, _mousePos.y)) {
			_scene->applyEvent("RightClick");
		}
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::OnMouseRightUp() {
	if (_activeObject) _activeObject->handleMouse(MOUSE_RELEASE, MOUSE_BUTTON_RIGHT);

	bool Handled = _state == GAME_RUNNING && SUCCEEDED(applyEvent("RightRelease"));
	if (!Handled) {
		if (_activeObject != NULL) {
			_activeObject->applyEvent("RightRelease");
		} else if (_state == GAME_RUNNING && _scene && _scene->pointInViewport(_mousePos.x, _mousePos.y)) {
			_scene->applyEvent("RightRelease");
		}
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::DisplayDebugInfo() {
	char str[100];
	if (Game->_dEBUG_DebugMode) {
		sprintf(str, "Mouse: %d, %d (scene: %d, %d)", _mousePos.x, _mousePos.y, _mousePos.x + _scene->getOffsetLeft(), _mousePos.y + _scene->getOffsetTop());
		_systemFont->drawText((byte *)str, 0, 90, _renderer->_width, TAL_RIGHT);

		sprintf(str, "Scene: %s (prev: %s)", (_scene && _scene->_name) ? _scene->_name : "???", _prevSceneName ? _prevSceneName : "???");
		_systemFont->drawText((byte *)str, 0, 110, _renderer->_width, TAL_RIGHT);
	}
	return CBGame::DisplayDebugInfo();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::OnScriptShutdown(CScScript *script) {
	if (_responseBox && _responseBox->_waitingScript == script)
		_responseBox->_waitingScript = NULL;

	return S_OK;
}

} // end of namespace WinterMute
