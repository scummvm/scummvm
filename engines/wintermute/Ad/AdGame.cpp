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
#include "engines/wintermute/Ad/AdGame.h"
#include "engines/wintermute/Ad/AdResponseBox.h"
#include "engines/wintermute/Ad/AdInventoryBox.h"
#include "engines/wintermute/Ad/AdSceneState.h"
#include "engines/wintermute/Base/PartEmitter.h"
#include "engines/wintermute/Base/BParser.h"
#include "engines/wintermute/Base/BSurfaceStorage.h"
#include "engines/wintermute/Base/BTransitionMgr.h"
#include "engines/wintermute/Base/BObject.h"
#include "engines/wintermute/Base/BSound.h"
#include "engines/wintermute/UIWindow.h"
#include "engines/wintermute/Base/scriptables/ScValue.h"
#include "engines/wintermute/UIEntity.h"
#include "engines/wintermute/Ad/AdScene.h"
#include "engines/wintermute/Ad/AdEntity.h"
#include "engines/wintermute/Ad/AdActor.h"
#include "engines/wintermute/Ad/AdInventory.h"
#include "engines/wintermute/Ad/AdResponseContext.h"
#include "engines/wintermute/Ad/AdItem.h"
#include "engines/wintermute/Base/BViewport.h"
#include "engines/wintermute/Base/BFont.h"
#include "engines/wintermute/Base/scriptables/ScEngine.h"
#include "engines/wintermute/Base/BStringTable.h"
#include "engines/wintermute/Ad/AdSentence.h"
#include "engines/wintermute/Ad/AdResponse.h"
#include "engines/wintermute/Base/scriptables/ScScript.h"
#include "engines/wintermute/Base/scriptables/ScStack.h"
#include "engines/wintermute/Base/BSprite.h"
#include "engines/wintermute/Base/BFileManager.h"
#include "engines/wintermute/utils.h"
#include "common/str.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CAdGame, true)

//////////////////////////////////////////////////////////////////////////
CAdGame::CAdGame(): CBGame() {
	_responseBox = NULL;
	_inventoryBox = NULL;

	_scene = new CAdScene(Game);
	_scene->SetName("");
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
	Cleanup();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::Cleanup() {
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

	return CBGame::Cleanup();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::InitLoop() {
	if (_scheduledScene && _transMgr->IsReady()) {
		ChangeScene(_scheduledScene, _scheduledFadeIn);
		delete[] _scheduledScene;
		_scheduledScene = NULL;

		Game->_activeObject = NULL;
	}


	HRESULT res;
	res = CBGame::InitLoop();
	if (FAILED(res)) return res;

	if (_scene) res = _scene->InitLoop();

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
		HRESULT Res = _scene->RemoveObject(Object);
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
		_scene->ApplyEvent("SceneShutdown", true);

		SetPrevSceneName(_scene->_name);
		SetPrevSceneFilename(_scene->_filename);

		if (!_tempDisableSaveState) _scene->SaveState();
		_tempDisableSaveState = false;
	}

	if (_scene) {
		// reset objects
		for (int i = 0; i < _objects.GetSize(); i++) _objects[i]->Reset();

		// reset scene properties
		_scene->_sFXVolume = 100;
		if (_scene->_scProp) _scene->_scProp->Cleanup();

		HRESULT ret;
		if (_initialScene && _dEBUG_DebugMode && _debugStartupScene) {
			_initialScene = false;
			ret = _scene->LoadFile(_debugStartupScene);
		} else ret = _scene->LoadFile(Filename);

		if (SUCCEEDED(ret)) {
			// invalidate references to the original scene
			for (int i = 0; i < _objects.GetSize(); i++) {
				_objects[i]->InvalidateCurrRegions();
				_objects[i]->_stickRegion = NULL;
			}

			_scene->LoadState();
		}
		if (FadeIn) Game->_transMgr->Start(TRANSITION_FADE_IN);
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
		else _sentences[i]->Display();
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CAdGame::FinishSentences() {
	for (int i = 0; i < _sentences.GetSize(); i++) {
		if (_sentences[i]->CanSkip()) {
			_sentences[i]->_duration = 0;
			if (_sentences[i]->_sound) _sentences[i]->_sound->Stop();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, const char *Name) {
	//////////////////////////////////////////////////////////////////////////
	// ChangeScene
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "ChangeScene") == 0) {
		Stack->CorrectParams(3);
		const char *Filename = Stack->Pop()->GetString();
		CScValue *valFadeOut = Stack->Pop();
		CScValue *valFadeIn = Stack->Pop();

		bool TransOut = valFadeOut->IsNULL() ? true : valFadeOut->GetBool();
		bool TransIn  = valFadeIn->IsNULL() ? true : valFadeIn->GetBool();

		ScheduleChangeScene(Filename, TransIn);
		if (TransOut) _transMgr->Start(TRANSITION_FADE_OUT, true);
		Stack->PushNULL();


		//HRESULT ret = ChangeScene(Stack->Pop()->GetString());
		//if(FAILED(ret)) Stack->PushBool(false);
		//else Stack->PushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LoadActor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "LoadActor") == 0) {
		Stack->CorrectParams(1);
		CAdActor *act = new CAdActor(Game);
		if (act && SUCCEEDED(act->LoadFile(Stack->Pop()->GetString()))) {
			AddObject(act);
			Stack->PushNative(act, true);
		} else {
			delete act;
			act = NULL;
			Stack->PushNULL();
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LoadEntity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "LoadEntity") == 0) {
		Stack->CorrectParams(1);
		CAdEntity *ent = new CAdEntity(Game);
		if (ent && SUCCEEDED(ent->LoadFile(Stack->Pop()->GetString()))) {
			AddObject(ent);
			Stack->PushNative(ent, true);
		} else {
			delete ent;
			ent = NULL;
			Stack->PushNULL();
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// UnloadObject / UnloadActor / UnloadEntity / DeleteEntity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "UnloadObject") == 0 || strcmp(Name, "UnloadActor") == 0 || strcmp(Name, "UnloadEntity") == 0 || strcmp(Name, "DeleteEntity") == 0) {
		Stack->CorrectParams(1);
		CScValue *val = Stack->Pop();
		CAdObject *obj = (CAdObject *)val->GetNative();
		RemoveObject(obj);
		if (val->GetType() == VAL_VARIABLE_REF) val->SetNULL();

		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CreateEntity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "CreateEntity") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();

		CAdEntity *Ent = new CAdEntity(Game);
		AddObject(Ent);
		if (!Val->IsNULL()) Ent->SetName(Val->GetString());
		Stack->PushNative(Ent, true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CreateItem
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "CreateItem") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();

		CAdItem *Item = new CAdItem(Game);
		AddItem(Item);
		if (!Val->IsNULL()) Item->SetName(Val->GetString());
		Stack->PushNative(Item, true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeleteItem
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "DeleteItem") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();

		CAdItem *Item = NULL;
		if (Val->IsNative()) Item = (CAdItem *)Val->GetNative();
		else Item = GetItemByName(Val->GetString());

		if (Item) {
			DeleteItem(Item);
		}

		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// QueryItem
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "QueryItem") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();

		CAdItem *Item = NULL;
		if (Val->IsInt()) {
			int Index = Val->GetInt();
			if (Index >= 0 && Index < _items.GetSize()) Item = _items[Index];
		} else {
			Item = GetItemByName(Val->GetString());
		}

		if (Item) Stack->PushNative(Item, true);
		else Stack->PushNULL();

		return S_OK;
	}


	//////////////////////////////////////////////////////////////////////////
	// AddResponse/AddResponseOnce/AddResponseOnceGame
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AddResponse") == 0 || strcmp(Name, "AddResponseOnce") == 0 || strcmp(Name, "AddResponseOnceGame") == 0) {
		Stack->CorrectParams(6);
		int id = Stack->Pop()->GetInt();
		const char *text = Stack->Pop()->GetString();
		CScValue *val1 = Stack->Pop();
		CScValue *val2 = Stack->Pop();
		CScValue *val3 = Stack->Pop();
		CScValue *val4 = Stack->Pop();

		if (_responseBox) {
			CAdResponse *res = new CAdResponse(Game);
			if (res) {
				res->_iD = id;
				res->SetText(text);
				_stringTable->Expand(&res->_text);
				if (!val1->IsNULL()) res->SetIcon(val1->GetString());
				if (!val2->IsNULL()) res->SetIconHover(val2->GetString());
				if (!val3->IsNULL()) res->SetIconPressed(val3->GetString());
				if (!val4->IsNULL()) res->SetFont(val4->GetString());

				if (strcmp(Name, "AddResponseOnce") == 0) res->_responseType = RESPONSE_ONCE;
				else if (strcmp(Name, "AddResponseOnceGame") == 0) res->_responseType = RESPONSE_ONCE_GAME;

				_responseBox->_responses.Add(res);
			}
		} else {
			Script->RuntimeError("Game.AddResponse: response box is not defined");
		}
		Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ResetResponse
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ResetResponse") == 0) {
		Stack->CorrectParams(1);
		int ID = Stack->Pop()->GetInt(-1);
		ResetResponse(ID);
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ClearResponses
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ClearResponses") == 0) {
		Stack->CorrectParams(0);
		_responseBox->ClearResponses();
		_responseBox->ClearButtons();
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetResponse
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetResponse") == 0) {
		Stack->CorrectParams(1);
		bool AutoSelectLast = Stack->Pop()->GetBool();

		if (_responseBox) {
			_responseBox->WeedResponses();

			if (_responseBox->_responses.GetSize() == 0) {
				Stack->PushNULL();
				return S_OK;
			}


			if (_responseBox->_responses.GetSize() == 1 && AutoSelectLast) {
				Stack->PushInt(_responseBox->_responses[0]->_iD);
				_responseBox->HandleResponse(_responseBox->_responses[0]);
				_responseBox->ClearResponses();
				return S_OK;
			}

			_responseBox->CreateButtons();
			_responseBox->_waitingScript = Script;
			Script->WaitForExclusive(_responseBox);
			_state = GAME_SEMI_FROZEN;
			_stateEx = GAME_WAITING_RESPONSE;
		} else {
			Script->RuntimeError("Game.GetResponse: response box is not defined");
			Stack->PushNULL();
		}
		return S_OK;
	}


	//////////////////////////////////////////////////////////////////////////
	// GetNumResponses
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetNumResponses") == 0) {
		Stack->CorrectParams(0);
		if (_responseBox) {
			_responseBox->WeedResponses();
			Stack->PushInt(_responseBox->_responses.GetSize());
		} else {
			Script->RuntimeError("Game.GetNumResponses: response box is not defined");
			Stack->PushNULL();
		}
		return S_OK;
	}


	//////////////////////////////////////////////////////////////////////////
	// StartDlgBranch
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "StartDlgBranch") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();
		Common::String BranchName;
		if (Val->IsNULL()) {
			BranchName.format("line%d", Script->_currentLine);
		} else BranchName = Val->GetString();

		StartDlgBranch(BranchName.c_str(), Script->_filename == NULL ? "" : Script->_filename, Script->_threadEvent == NULL ? "" : Script->_threadEvent);
		Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// EndDlgBranch
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "EndDlgBranch") == 0) {
		Stack->CorrectParams(1);

		const char *BranchName = NULL;
		CScValue *Val = Stack->Pop();
		if (!Val->IsNULL()) BranchName = Val->GetString();
		EndDlgBranch(BranchName, Script->_filename == NULL ? "" : Script->_filename, Script->_threadEvent == NULL ? "" : Script->_threadEvent);

		Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetCurrentDlgBranch
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetCurrentDlgBranch") == 0) {
		Stack->CorrectParams(0);

		if (_dlgPendingBranches.GetSize() > 0) {
			Stack->PushString(_dlgPendingBranches[_dlgPendingBranches.GetSize() - 1]);
		} else Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// TakeItem
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "TakeItem") == 0) {
		return _invObject->ScCallMethod(Script, Stack, ThisStack, Name);
	}

	//////////////////////////////////////////////////////////////////////////
	// DropItem
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "DropItem") == 0) {
		return _invObject->ScCallMethod(Script, Stack, ThisStack, Name);
	}

	//////////////////////////////////////////////////////////////////////////
	// GetItem
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetItem") == 0) {
		return _invObject->ScCallMethod(Script, Stack, ThisStack, Name);
	}

	//////////////////////////////////////////////////////////////////////////
	// HasItem
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "HasItem") == 0) {
		return _invObject->ScCallMethod(Script, Stack, ThisStack, Name);
	}

	//////////////////////////////////////////////////////////////////////////
	// IsItemTaken
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "IsItemTaken") == 0) {
		Stack->CorrectParams(1);

		CScValue *val = Stack->Pop();
		if (!val->IsNULL()) {
			for (int i = 0; i < _inventories.GetSize(); i++) {
				CAdInventory *Inv = _inventories[i];

				for (int j = 0; j < Inv->_takenItems.GetSize(); j++) {
					if (val->GetNative() == Inv->_takenItems[j]) {
						Stack->PushBool(true);
						return S_OK;
					} else if (scumm_stricmp(val->GetString(), Inv->_takenItems[j]->_name) == 0) {
						Stack->PushBool(true);
						return S_OK;
					}
				}
			}
		} else Script->RuntimeError("Game.IsItemTaken: item name expected");

		Stack->PushBool(false);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetInventoryWindow
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetInventoryWindow") == 0) {
		Stack->CorrectParams(0);
		if (_inventoryBox && _inventoryBox->_window)
			Stack->PushNative(_inventoryBox->_window, true);
		else
			Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetResponsesWindow
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetResponsesWindow") == 0 || strcmp(Name, "GetResponseWindow") == 0) {
		Stack->CorrectParams(0);
		if (_responseBox && _responseBox->_window)
			Stack->PushNative(_responseBox->_window, true);
		else
			Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LoadResponseBox
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "LoadResponseBox") == 0) {
		Stack->CorrectParams(1);
		const char *Filename = Stack->Pop()->GetString();

		Game->UnregisterObject(_responseBox);
		_responseBox = new CAdResponseBox(Game);
		if (_responseBox && !FAILED(_responseBox->LoadFile(Filename))) {
			RegisterObject(_responseBox);
			Stack->PushBool(true);
		} else {
			delete _responseBox;
			_responseBox = NULL;
			Stack->PushBool(false);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LoadInventoryBox
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "LoadInventoryBox") == 0) {
		Stack->CorrectParams(1);
		const char *Filename = Stack->Pop()->GetString();

		Game->UnregisterObject(_inventoryBox);
		_inventoryBox = new CAdInventoryBox(Game);
		if (_inventoryBox && !FAILED(_inventoryBox->LoadFile(Filename))) {
			RegisterObject(_inventoryBox);
			Stack->PushBool(true);
		} else {
			delete _inventoryBox;
			_inventoryBox = NULL;
			Stack->PushBool(false);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LoadItems
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "LoadItems") == 0) {
		Stack->CorrectParams(2);
		const char *Filename = Stack->Pop()->GetString();
		bool Merge = Stack->Pop()->GetBool(false);

		HRESULT Ret = LoadItemsFile(Filename, Merge);
		Stack->PushBool(SUCCEEDED(Ret));

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AddSpeechDir
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AddSpeechDir") == 0) {
		Stack->CorrectParams(1);
		const char *Dir = Stack->Pop()->GetString();
		Stack->PushBool(SUCCEEDED(AddSpeechDir(Dir)));

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RemoveSpeechDir
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "RemoveSpeechDir") == 0) {
		Stack->CorrectParams(1);
		const char *Dir = Stack->Pop()->GetString();
		Stack->PushBool(SUCCEEDED(RemoveSpeechDir(Dir)));

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetSceneViewport
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetSceneViewport") == 0) {
		Stack->CorrectParams(4);
		int X = Stack->Pop()->GetInt();
		int Y = Stack->Pop()->GetInt();
		int Width = Stack->Pop()->GetInt();
		int Height = Stack->Pop()->GetInt();

		if (Width <= 0) Width = _renderer->_width;
		if (Height <= 0) Height = _renderer->_height;

		if (!_sceneViewport) _sceneViewport = new CBViewport(Game);
		if (_sceneViewport) _sceneViewport->SetRect(X, Y, X + Width, Y + Height);

		Stack->PushBool(true);

		return S_OK;
	}


	else return CBGame::ScCallMethod(Script, Stack, ThisStack, Name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CAdGame::ScGetProperty(const char *Name) {
	_scValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		_scValue->SetString("game");
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Scene
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Scene") == 0) {
		if (_scene) _scValue->SetNative(_scene, true);
		else _scValue->SetNULL();

		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// SelectedItem
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SelectedItem") == 0) {
		//if(_selectedItem) _scValue->SetString(_selectedItem->_name);
		if (_selectedItem) _scValue->SetNative(_selectedItem, true);
		else _scValue->SetNULL();

		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// NumItems
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "NumItems") == 0) {
		return _invObject->ScGetProperty(Name);
	}

	//////////////////////////////////////////////////////////////////////////
	// SmartItemCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SmartItemCursor") == 0) {
		_scValue->SetBool(_smartItemCursor);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// InventoryVisible
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "InventoryVisible") == 0) {
		_scValue->SetBool(_inventoryBox && _inventoryBox->_visible);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// InventoryScrollOffset
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "InventoryScrollOffset") == 0) {
		if (_inventoryBox) _scValue->SetInt(_inventoryBox->_scrollOffset);
		else _scValue->SetInt(0);

		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ResponsesVisible (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ResponsesVisible") == 0) {
		_scValue->SetBool(_stateEx == GAME_WAITING_RESPONSE);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// PrevScene / PreviousScene (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "PrevScene") == 0 || strcmp(Name, "PreviousScene") == 0) {
		if (!_prevSceneName) _scValue->SetString("");
		else _scValue->SetString(_prevSceneName);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// PrevSceneFilename / PreviousSceneFilename (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "PrevSceneFilename") == 0 || strcmp(Name, "PreviousSceneFilename") == 0) {
		if (!_prevSceneFilename) _scValue->SetString("");
		else _scValue->SetString(_prevSceneFilename);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// LastResponse (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "LastResponse") == 0) {
		if (!_responseBox || !_responseBox->_lastResponseText) _scValue->SetString("");
		else _scValue->SetString(_responseBox->_lastResponseText);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// LastResponseOrig (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "LastResponseOrig") == 0) {
		if (!_responseBox || !_responseBox->_lastResponseTextOrig) _scValue->SetString("");
		else _scValue->SetString(_responseBox->_lastResponseTextOrig);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// InventoryObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "InventoryObject") == 0) {
		if (_inventoryOwner == _invObject) _scValue->SetNative(this, true);
		else _scValue->SetNative(_inventoryOwner, true);

		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// TotalNumItems
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "TotalNumItems") == 0) {
		_scValue->SetInt(_items.GetSize());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// TalkSkipButton
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "TalkSkipButton") == 0) {
		_scValue->SetInt(_talkSkipButton);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ChangingScene
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ChangingScene") == 0) {
		_scValue->SetBool(_scheduledScene != NULL);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// StartupScene
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "StartupScene") == 0) {
		if (!_startupScene) _scValue->SetNULL();
		else _scValue->SetString(_startupScene);
		return _scValue;
	}

	else return CBGame::ScGetProperty(Name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::ScSetProperty(const char *Name, CScValue *Value) {

	//////////////////////////////////////////////////////////////////////////
	// SelectedItem
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "SelectedItem") == 0) {
		if (Value->IsNULL()) _selectedItem = NULL;
		else {
			if (Value->IsNative()) {
				_selectedItem = NULL;
				for (int i = 0; i < _items.GetSize(); i++) {
					if (_items[i] == Value->GetNative()) {
						_selectedItem = (CAdItem *)Value->GetNative();
						break;
					}
				}
			} else {
				// try to get by name
				_selectedItem = GetItemByName(Value->GetString());
			}
		}

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SmartItemCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SmartItemCursor") == 0) {
		_smartItemCursor = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// InventoryVisible
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "InventoryVisible") == 0) {
		if (_inventoryBox) _inventoryBox->_visible = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// InventoryObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "InventoryObject") == 0) {
		if (_inventoryOwner && _inventoryBox) _inventoryOwner->GetInventory()->_scrollOffset = _inventoryBox->_scrollOffset;

		if (Value->IsNULL()) _inventoryOwner = _invObject;
		else {
			CBObject *Obj = (CBObject *)Value->GetNative();
			if (Obj == this) _inventoryOwner = _invObject;
			else if (Game->ValidObject(Obj)) _inventoryOwner = (CAdObject *)Obj;
		}

		if (_inventoryOwner && _inventoryBox) _inventoryBox->_scrollOffset = _inventoryOwner->GetInventory()->_scrollOffset;

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// InventoryScrollOffset
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "InventoryScrollOffset") == 0) {
		if (_inventoryBox) _inventoryBox->_scrollOffset = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// TalkSkipButton
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "TalkSkipButton") == 0) {
		int Val = Value->GetInt();
		if (Val < 0) Val = 0;
		if (Val > TALK_SKIP_NONE) Val = TALK_SKIP_NONE;
		_talkSkipButton = (TTalkSkipButton)Val;
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// StartupScene
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "StartupScene") == 0) {
		if (Value == NULL) {
			delete[] _startupScene;
			_startupScene = NULL;
		} else CBUtils::SetString(&_startupScene, Value->GetString());

		return S_OK;
	}

	else return CBGame::ScSetProperty(Name, Value);
}


//////////////////////////////////////////////////////////////////////////
void CAdGame::PublishNatives() {
	if (!_scEngine || !_scEngine->_compilerAvailable) return;

	CBGame::PublishNatives();

	_scEngine->ExtDefineFunction("Actor");
	_scEngine->ExtDefineFunction("Entity");
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::ExternalCall(CScScript *Script, CScStack *Stack, CScStack *ThisStack, char *Name) {
	CScValue *this_obj;

	//////////////////////////////////////////////////////////////////////////
	// Actor
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Actor") == 0) {
		Stack->CorrectParams(0);
		this_obj = ThisStack->GetTop();

		this_obj->SetNative(new CAdActor(Game));
		Stack->PushNULL();
	}

	//////////////////////////////////////////////////////////////////////////
	// Entity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Entity") == 0) {
		Stack->CorrectParams(0);
		this_obj = ThisStack->GetTop();

		this_obj->SetNative(new CAdEntity(Game));
		Stack->PushNULL();
	}


	//////////////////////////////////////////////////////////////////////////
	// call parent
	else return CBGame::ExternalCall(Script, Stack, ThisStack, Name);


	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::ShowCursor() {
	if (_cursorHidden) return S_OK;

	if (_selectedItem && Game->_state == GAME_RUNNING && _stateEx == GAME_NORMAL && _interactive) {
		if (_selectedItem->_cursorCombined) {
			CBSprite *OrigLastCursor = _lastCursor;
			CBGame::ShowCursor();
			_lastCursor = OrigLastCursor;
		}
		if (_activeObject && _selectedItem->_cursorHover && _activeObject->GetExtendedFlag("usable")) {
			if (!_smartItemCursor || _activeObject->CanHandleEvent(_selectedItem->_name))
				return DrawCursor(_selectedItem->_cursorHover);
			else
				return DrawCursor(_selectedItem->_cursorNormal);
		} else return DrawCursor(_selectedItem->_cursorNormal);
	} else return CBGame::ShowCursor();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::LoadFile(const char *Filename) {
	byte *Buffer = _fileManager->ReadWholeFile(Filename);
	if (Buffer == NULL) {
		Game->LOG(0, "CAdGame::LoadFile failed for file '%s'", Filename);
		return E_FAIL;
	}

	HRESULT ret;

	_filename = new char [strlen(Filename) + 1];
	strcpy(_filename, Filename);

	if (FAILED(ret = LoadBuffer(Buffer, true))) Game->LOG(0, "Error parsing GAME file '%s'", Filename);


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
HRESULT CAdGame::LoadBuffer(byte  *Buffer, bool Complete) {
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
			if (FAILED(CBGame::LoadBuffer(params, false))) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_AD_GAME:
			while (cmd > 0 && (cmd = parser.GetCommand((char **)&params, commands, (char **)&params2)) > 0) {
				switch (cmd) {
				case TOKEN_RESPONSE_BOX:
					delete _responseBox;
					_responseBox = new CAdResponseBox(Game);
					if (_responseBox && !FAILED(_responseBox->LoadFile((char *)params2)))
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
					if (_inventoryBox && !FAILED(_inventoryBox->LoadFile((char *)params2)))
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
					if (_sceneViewport) _sceneViewport->SetRect(rc.left, rc.top, rc.right, rc.bottom);
				}
				break;

				case TOKEN_EDITOR_PROPERTY:
					ParseEditorProperty(params2, false);
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
HRESULT CAdGame::Persist(CBPersistMgr *PersistMgr) {
	if (!PersistMgr->_saving) Cleanup();
	CBGame::Persist(PersistMgr);

	_dlgPendingBranches.Persist(PersistMgr);

	_inventories.Persist(PersistMgr);
	PersistMgr->Transfer(TMEMBER(_inventoryBox));

	_objects.Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(_prevSceneName));
	PersistMgr->Transfer(TMEMBER(_prevSceneFilename));

	PersistMgr->Transfer(TMEMBER(_responseBox));
	_responsesBranch.Persist(PersistMgr);
	_responsesGame.Persist(PersistMgr);
	PersistMgr->Transfer(TMEMBER(_scene));
	_sceneStates.Persist(PersistMgr);
	PersistMgr->Transfer(TMEMBER(_scheduledFadeIn));
	PersistMgr->Transfer(TMEMBER(_scheduledScene));
	PersistMgr->Transfer(TMEMBER(_selectedItem));
	PersistMgr->Transfer(TMEMBER_INT(_talkSkipButton));

	_sentences.Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(_sceneViewport));
	PersistMgr->Transfer(TMEMBER_INT(_stateEx));
	PersistMgr->Transfer(TMEMBER(_initialScene));
	PersistMgr->Transfer(TMEMBER(_debugStartupScene));

	PersistMgr->Transfer(TMEMBER(_invObject));
	PersistMgr->Transfer(TMEMBER(_inventoryOwner));
	PersistMgr->Transfer(TMEMBER(_tempDisableSaveState));
	_items.Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(_itemsFile));

	_speechDirs.Persist(PersistMgr);
	PersistMgr->Transfer(TMEMBER(_smartItemCursor));

	if (!PersistMgr->_saving) _initialScene = false;

	PersistMgr->Transfer(TMEMBER(_startupScene));


	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::LoadGame(const char *Filename) {
	HRESULT ret = CBGame::LoadGame(Filename);
	if (SUCCEEDED(ret)) CSysClassRegistry::GetInstance()->EnumInstances(AfterLoadRegion, "CAdRegion", NULL);
	return ret;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::InitAfterLoad() {
	CBGame::InitAfterLoad();
	CSysClassRegistry::GetInstance()->EnumInstances(AfterLoadScene,   "CAdScene",   NULL);
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
void CAdGame::AfterLoadScene(void *Scene, void *Data) {
	((CAdScene *)Scene)->AfterLoad();
}


//////////////////////////////////////////////////////////////////////////
void CAdGame::SetPrevSceneName(const char *Name) {
	delete[] _prevSceneName;
	_prevSceneName = NULL;
	if (Name) {
		_prevSceneName = new char[strlen(Name) + 1];
		if (_prevSceneName) strcpy(_prevSceneName, Name);
	}
}


//////////////////////////////////////////////////////////////////////////
void CAdGame::SetPrevSceneFilename(const char *Name) {
	delete[] _prevSceneFilename;
	_prevSceneFilename = NULL;
	if (Name) {
		_prevSceneFilename = new char[strlen(Name) + 1];
		if (_prevSceneFilename) strcpy(_prevSceneFilename, Name);
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
	byte *Buffer = Game->_fileManager->ReadWholeFile(Filename);
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
			if (item && !FAILED(item->LoadBuffer(params, false))) {
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
	for (int i = 0; i < strlen(FilenameCor); i++) {
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
		ret->SetFilename(FilenameCor);

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
		if (!ent || FAILED(ent->LoadBuffer((byte *)*params, false))) {
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
HRESULT CAdGame::WindowScriptMethodHook(CUIWindow *Win, CScScript *Script, CScStack *Stack, const char *Name) {
	if (strcmp(Name, "CreateEntityContainer") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();

		CUIEntity *Ent = new CUIEntity(Game);
		if (!Val->IsNULL()) Ent->SetName(Val->GetString());
		Stack->PushNative(Ent, true);

		Ent->_parent = Win;
		Win->_widgets.Add(Ent);

		return S_OK;
	} else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::StartDlgBranch(const char *BranchName, const char *ScriptName, const char *EventName) {
	char *Name = new char[strlen(BranchName) + 1 + strlen(ScriptName) + 1 + strlen(EventName) + 1];
	if (Name) {
		sprintf(Name, "%s.%s.%s", BranchName, ScriptName, EventName);
		_dlgPendingBranches.Add(Name);
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::EndDlgBranch(const char *BranchName, const char *ScriptName, const char *EventName) {
	char *Name = NULL;
	bool DeleteName = false;
	if (BranchName == NULL && _dlgPendingBranches.GetSize() > 0) {
		Name = _dlgPendingBranches[_dlgPendingBranches.GetSize() - 1];
	} else {
		if (BranchName != NULL) {
			Name = new char[strlen(BranchName) + 1 + strlen(ScriptName) + 1 + strlen(EventName) + 1];
			if (Name) {
				sprintf(Name, "%s.%s.%s", BranchName, ScriptName, EventName);
				DeleteName = true;
			}
		}
	}

	if (Name == NULL) return S_OK;


	int StartIndex = -1;
	int i;
	for (i = _dlgPendingBranches.GetSize() - 1; i >= 0; i--) {
		if (scumm_stricmp(Name, _dlgPendingBranches[i]) == 0) {
			StartIndex = i;
			break;
		}
	}
	if (StartIndex >= 0) {
		for (i = StartIndex; i < _dlgPendingBranches.GetSize(); i++) {
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

	if (DeleteName) delete [] Name;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::ClearBranchResponses(char *Name) {
	for (int i = 0; i < _responsesBranch.GetSize(); i++) {
		if (scumm_stricmp(Name, _responsesBranch[i]->_context) == 0) {
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
	r->SetContext(_dlgPendingBranches.GetSize() > 0 ? _dlgPendingBranches[_dlgPendingBranches.GetSize() - 1] : NULL);
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
	r->SetContext(_dlgPendingBranches.GetSize() > 0 ? _dlgPendingBranches[_dlgPendingBranches.GetSize() - 1] : NULL);
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
	_renderer->Fill(0, 0, 0);
	if (!_editorMode) _renderer->SetScreenViewport();

	// process scripts
	if (Update) _scEngine->Tick();

	POINT p;
	GetMousePos(&p);

	_scene->Update();
	_scene->Display();


	// display in-game windows
	DisplayWindows(true);
	if (_inventoryBox) _inventoryBox->Display();
	if (_stateEx == GAME_WAITING_RESPONSE) _responseBox->Display();
	if (_indicatorDisplay) DisplayIndicator();


	if (Update || DisplayAll) {
		// display normal windows
		DisplayWindows(false);

		SetActiveObject(Game->_renderer->GetObjectAt(p.x, p.y));

		// textual info
		DisplaySentences(_state == GAME_FROZEN);

		ShowCursor();

		if (_fader) _fader->Display();
		_transMgr->Update();
	}


	if (_loadingIcon) {
		_loadingIcon->Display(_loadingIconX, _loadingIconY);
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
CAdItem *CAdGame::GetItemByName(const char *Name) {
	for (int i = 0; i < _items.GetSize(); i++) {
		if (scumm_stricmp(_items[i]->_name, Name) == 0) return _items[i];
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
	_scene->HandleItemAssociations(Item->_name, false);

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
		CBFile *File = _fileManager->OpenFile(Ret);
		if (File) {
			_fileManager->CloseFile(File);
			return Ret;
		}

		sprintf(Ret, "%s%s.wav", _speechDirs[i], StringID);
		File = _fileManager->OpenFile(Ret);
		if (File) {
			_fileManager->CloseFile(File);
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

	return _renderer->PointInViewport(&Pos);
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

	if (_activeObject) _activeObject->HandleMouse(MOUSE_CLICK, MOUSE_BUTTON_LEFT);

	bool Handled = _state == GAME_RUNNING && SUCCEEDED(ApplyEvent("LeftClick"));
	if (!Handled) {
		if (_activeObject != NULL) {
			_activeObject->ApplyEvent("LeftClick");
		} else if (_state == GAME_RUNNING && _scene && _scene->PointInViewport(_mousePos.x, _mousePos.y)) {
			_scene->ApplyEvent("LeftClick");
		}
	}

	if (_activeObject != NULL) Game->_capturedObject = Game->_activeObject;
	_mouseLeftDown = true;
	CBPlatform::SetCapture(_renderer->_window);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::OnMouseLeftUp() {
	if (_activeObject) _activeObject->HandleMouse(MOUSE_RELEASE, MOUSE_BUTTON_LEFT);

	CBPlatform::ReleaseCapture();
	_capturedObject = NULL;
	_mouseLeftDown = false;

	bool Handled = /*_state==GAME_RUNNING &&*/ SUCCEEDED(ApplyEvent("LeftRelease"));
	if (!Handled) {
		if (_activeObject != NULL) {
			_activeObject->ApplyEvent("LeftRelease");
		} else if (_state == GAME_RUNNING && _scene && _scene->PointInViewport(_mousePos.x, _mousePos.y)) {
			_scene->ApplyEvent("LeftRelease");
		}
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::OnMouseLeftDblClick() {
	if (!ValidMouse()) return S_OK;

	if (_state == GAME_RUNNING && !_interactive) return S_OK;

	if (_activeObject) _activeObject->HandleMouse(MOUSE_DBLCLICK, MOUSE_BUTTON_LEFT);

	bool Handled = _state == GAME_RUNNING && SUCCEEDED(ApplyEvent("LeftDoubleClick"));
	if (!Handled) {
		if (_activeObject != NULL) {
			_activeObject->ApplyEvent("LeftDoubleClick");
		} else if (_state == GAME_RUNNING && _scene && _scene->PointInViewport(_mousePos.x, _mousePos.y)) {
			_scene->ApplyEvent("LeftDoubleClick");
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

	if (_activeObject) _activeObject->HandleMouse(MOUSE_CLICK, MOUSE_BUTTON_RIGHT);

	bool Handled = _state == GAME_RUNNING && SUCCEEDED(ApplyEvent("RightClick"));
	if (!Handled) {
		if (_activeObject != NULL) {
			_activeObject->ApplyEvent("RightClick");
		} else if (_state == GAME_RUNNING && _scene && _scene->PointInViewport(_mousePos.x, _mousePos.y)) {
			_scene->ApplyEvent("RightClick");
		}
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::OnMouseRightUp() {
	if (_activeObject) _activeObject->HandleMouse(MOUSE_RELEASE, MOUSE_BUTTON_RIGHT);

	bool Handled = _state == GAME_RUNNING && SUCCEEDED(ApplyEvent("RightRelease"));
	if (!Handled) {
		if (_activeObject != NULL) {
			_activeObject->ApplyEvent("RightRelease");
		} else if (_state == GAME_RUNNING && _scene && _scene->PointInViewport(_mousePos.x, _mousePos.y)) {
			_scene->ApplyEvent("RightRelease");
		}
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::DisplayDebugInfo() {
	char str[100];
	if (Game->_dEBUG_DebugMode) {
		sprintf(str, "Mouse: %d, %d (scene: %d, %d)", _mousePos.x, _mousePos.y, _mousePos.x + _scene->GetOffsetLeft(), _mousePos.y + _scene->GetOffsetTop());
		_systemFont->DrawText((byte *)str, 0, 90, _renderer->_width, TAL_RIGHT);

		sprintf(str, "Scene: %s (prev: %s)", (_scene && _scene->_name) ? _scene->_name : "???", _prevSceneName ? _prevSceneName : "???");
		_systemFont->DrawText((byte *)str, 0, 110, _renderer->_width, TAL_RIGHT);
	}
	return CBGame::DisplayDebugInfo();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::OnScriptShutdown(CScScript *Script) {
	if (_responseBox && _responseBox->_waitingScript == Script)
		_responseBox->_waitingScript = NULL;

	return S_OK;
}

} // end of namespace WinterMute
