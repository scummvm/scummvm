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
#include "engines/wintermute/AdGame.h"
#include "engines/wintermute/AdResponseBox.h"
#include "engines/wintermute/AdInventoryBox.h"
#include "engines/wintermute/AdSceneState.h"
#include "engines/wintermute/PartEmitter.h"
#include "engines/wintermute/BParser.h"
#include "engines/wintermute/BSurfaceStorage.h"
#include "engines/wintermute/BTransitionMgr.h"
#include "engines/wintermute/BObject.h"
#include "engines/wintermute/BSound.h"
#include "engines/wintermute/UIWindow.h"
#include "engines/wintermute/scriptables/ScValue.h"
#include "engines/wintermute/UIEntity.h"
#include "engines/wintermute/AdScene.h"
#include "engines/wintermute/AdEntity.h"
#include "engines/wintermute/AdActor.h"
#include "engines/wintermute/AdInventory.h"
#include "engines/wintermute/AdResponseContext.h"
#include "engines/wintermute/AdItem.h"
#include "engines/wintermute/BViewport.h"
#include "engines/wintermute/BFont.h"
#include "engines/wintermute/scriptables/ScEngine.h"
#include "engines/wintermute/BStringTable.h"
#include "engines/wintermute/AdSentence.h"
#include "engines/wintermute/AdResponse.h"
#include "engines/wintermute/scriptables/ScScript.h"
#include "engines/wintermute/scriptables/ScStack.h"
#include "engines/wintermute/BSprite.h"
#include "engines/wintermute/BFileManager.h"
#include "engines/wintermute/utils.h"
#include "common/str.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CAdGame, true)

//////////////////////////////////////////////////////////////////////////
CAdGame::CAdGame(): CBGame() {
	m_ResponseBox = NULL;
	m_InventoryBox = NULL;

	m_Scene = new CAdScene(Game);
	m_Scene->SetName("");
	RegisterObject(m_Scene);

	m_PrevSceneName = NULL;
	m_PrevSceneFilename = NULL;
	m_ScheduledScene = NULL;
	m_ScheduledFadeIn = false;


	m_StateEx = GAME_NORMAL;

	m_SelectedItem = NULL;


	m_TexItemLifeTime = 10000;
	m_TexWalkLifeTime = 10000;
	m_TexStandLifeTime = 10000;
	m_TexTalkLifeTime = 10000;

	m_TalkSkipButton = TALK_SKIP_LEFT;

	m_SceneViewport = NULL;

	m_InitialScene = true;
	m_DebugStartupScene = NULL;
	m_StartupScene = NULL;

	m_InvObject = new CAdObject(this);
	m_InventoryOwner = m_InvObject;

	m_TempDisableSaveState = false;
	m_ItemsFile = NULL;

	m_SmartItemCursor = false;

	AddSpeechDir("speech");
}


//////////////////////////////////////////////////////////////////////////
CAdGame::~CAdGame() {
	Cleanup();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::Cleanup() {
	int i;

	for (i = 0; i < m_Objects.GetSize(); i++) {
		UnregisterObject(m_Objects[i]);
		m_Objects[i] = NULL;
	}
	m_Objects.RemoveAll();


	for (i = 0; i < m_DlgPendingBranches.GetSize(); i++) {
		delete [] m_DlgPendingBranches[i];
	}
	m_DlgPendingBranches.RemoveAll();

	for (i = 0; i < m_SpeechDirs.GetSize(); i++) {
		delete [] m_SpeechDirs[i];
	}
	m_SpeechDirs.RemoveAll();


	UnregisterObject(m_Scene);
	m_Scene = NULL;

	// remove items
	for (i = 0; i < m_Items.GetSize(); i++) Game->UnregisterObject(m_Items[i]);
	m_Items.RemoveAll();


	// clear remaining inventories
	delete m_InvObject;
	m_InvObject = NULL;

	for (i = 0; i < m_Inventories.GetSize(); i++) {
		delete m_Inventories[i];
	}
	m_Inventories.RemoveAll();


	if (m_ResponseBox) {
		Game->UnregisterObject(m_ResponseBox);
		m_ResponseBox = NULL;
	}

	if (m_InventoryBox) {
		Game->UnregisterObject(m_InventoryBox);
		m_InventoryBox = NULL;
	}

	delete[] m_PrevSceneName;
	delete[] m_PrevSceneFilename;
	delete[] m_ScheduledScene;
	delete[] m_DebugStartupScene;
	delete[] m_ItemsFile;
	m_PrevSceneName = NULL;
	m_PrevSceneFilename = NULL;
	m_ScheduledScene = NULL;
	m_DebugStartupScene = NULL;
	m_StartupScene = NULL;
	m_ItemsFile = NULL;

	delete m_SceneViewport;
	m_SceneViewport = NULL;

	for (i = 0; i < m_SceneStates.GetSize(); i++) delete m_SceneStates[i];
	m_SceneStates.RemoveAll();

	for (i = 0; i < m_ResponsesBranch.GetSize(); i++) delete m_ResponsesBranch[i];
	m_ResponsesBranch.RemoveAll();

	for (i = 0; i < m_ResponsesGame.GetSize(); i++) delete m_ResponsesGame[i];
	m_ResponsesGame.RemoveAll();

	return CBGame::Cleanup();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::InitLoop() {
	if (m_ScheduledScene && m_TransMgr->IsReady()) {
		ChangeScene(m_ScheduledScene, m_ScheduledFadeIn);
		SAFE_DELETE_ARRAY(m_ScheduledScene);

		Game->m_ActiveObject = NULL;
	}


	HRESULT res;
	res = CBGame::InitLoop();
	if (FAILED(res)) return res;

	if (m_Scene) res = m_Scene->InitLoop();

	m_Sentences.RemoveAll();

	return res;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::AddObject(CAdObject *Object) {
	m_Objects.Add(Object);
	return RegisterObject(Object);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::RemoveObject(CAdObject *Object) {
	// in case the user called Scene.CreateXXX() and Game.DeleteXXX()
	if (m_Scene) {
		HRESULT Res = m_Scene->RemoveObject(Object);
		if (SUCCEEDED(Res)) return Res;
	}

	for (int i = 0; i < m_Objects.GetSize(); i++) {
		if (m_Objects[i] == Object) {
			m_Objects.RemoveAt(i);
			break;
		}
	}
	return UnregisterObject(Object);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::ChangeScene(char *Filename, bool FadeIn) {
	if (m_Scene == NULL) {
		m_Scene = new CAdScene(Game);
		RegisterObject(m_Scene);
	} else {
		m_Scene->ApplyEvent("SceneShutdown", true);

		SetPrevSceneName(m_Scene->m_Name);
		SetPrevSceneFilename(m_Scene->m_Filename);

		if (!m_TempDisableSaveState) m_Scene->SaveState();
		m_TempDisableSaveState = false;
	}

	if (m_Scene) {
		// reset objects
		for (int i = 0; i < m_Objects.GetSize(); i++) m_Objects[i]->Reset();

		// reset scene properties
		m_Scene->m_SFXVolume = 100;
		if (m_Scene->m_ScProp) m_Scene->m_ScProp->Cleanup();

		HRESULT ret;
		if (m_InitialScene && m_DEBUG_DebugMode && m_DebugStartupScene) {
			m_InitialScene = false;
			ret = m_Scene->LoadFile(m_DebugStartupScene);
		} else ret = m_Scene->LoadFile(Filename);

		if (SUCCEEDED(ret)) {
			// invalidate references to the original scene
			for (int i = 0; i < m_Objects.GetSize(); i++) {
				m_Objects[i]->InvalidateCurrRegions();
				m_Objects[i]->m_StickRegion = NULL;
			}

			m_Scene->LoadState();
		}
		if (FadeIn) Game->m_TransMgr->Start(TRANSITION_FADE_IN);
		return ret;
	} else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
void CAdGame::AddSentence(CAdSentence *Sentence) {
	m_Sentences.Add(Sentence);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::DisplaySentences(bool Frozen) {
	for (int i = 0; i < m_Sentences.GetSize(); i++) {
		if (Frozen && m_Sentences[i]->m_Freezable) continue;
		else m_Sentences[i]->Display();
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CAdGame::FinishSentences() {
	for (int i = 0; i < m_Sentences.GetSize(); i++) {
		if (m_Sentences[i]->CanSkip()) {
			m_Sentences[i]->m_Duration = 0;
			if (m_Sentences[i]->m_Sound) m_Sentences[i]->m_Sound->Stop();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, char *Name) {
	//////////////////////////////////////////////////////////////////////////
	// ChangeScene
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "ChangeScene") == 0) {
		Stack->CorrectParams(3);
		char *Filename = Stack->Pop()->GetString();
		CScValue *valFadeOut = Stack->Pop();
		CScValue *valFadeIn = Stack->Pop();

		bool TransOut = valFadeOut->IsNULL() ? true : valFadeOut->GetBool();
		bool TransIn  = valFadeIn->IsNULL() ? true : valFadeIn->GetBool();

		ScheduleChangeScene(Filename, TransIn);
		if (TransOut) m_TransMgr->Start(TRANSITION_FADE_OUT, true);
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
			if (Index >= 0 && Index < m_Items.GetSize()) Item = m_Items[Index];
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
		char *text = Stack->Pop()->GetString();
		CScValue *val1 = Stack->Pop();
		CScValue *val2 = Stack->Pop();
		CScValue *val3 = Stack->Pop();
		CScValue *val4 = Stack->Pop();

		if (m_ResponseBox) {
			CAdResponse *res = new CAdResponse(Game);
			if (res) {
				res->m_ID = id;
				res->SetText(text);
				m_StringTable->Expand(&res->m_Text);
				if (!val1->IsNULL()) res->SetIcon(val1->GetString());
				if (!val2->IsNULL()) res->SetIconHover(val2->GetString());
				if (!val3->IsNULL()) res->SetIconPressed(val3->GetString());
				if (!val4->IsNULL()) res->SetFont(val4->GetString());

				if (strcmp(Name, "AddResponseOnce") == 0) res->m_ResponseType = RESPONSE_ONCE;
				else if (strcmp(Name, "AddResponseOnceGame") == 0) res->m_ResponseType = RESPONSE_ONCE_GAME;

				m_ResponseBox->m_Responses.Add(res);
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
		m_ResponseBox->ClearResponses();
		m_ResponseBox->ClearButtons();
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetResponse
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetResponse") == 0) {
		Stack->CorrectParams(1);
		bool AutoSelectLast = Stack->Pop()->GetBool();

		if (m_ResponseBox) {
			m_ResponseBox->WeedResponses();

			if (m_ResponseBox->m_Responses.GetSize() == 0) {
				Stack->PushNULL();
				return S_OK;
			}


			if (m_ResponseBox->m_Responses.GetSize() == 1 && AutoSelectLast) {
				Stack->PushInt(m_ResponseBox->m_Responses[0]->m_ID);
				m_ResponseBox->HandleResponse(m_ResponseBox->m_Responses[0]);
				m_ResponseBox->ClearResponses();
				return S_OK;
			}

			m_ResponseBox->CreateButtons();
			m_ResponseBox->m_WaitingScript = Script;
			Script->WaitForExclusive(m_ResponseBox);
			m_State = GAME_SEMI_FROZEN;
			m_StateEx = GAME_WAITING_RESPONSE;
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
		if (m_ResponseBox) {
			m_ResponseBox->WeedResponses();
			Stack->PushInt(m_ResponseBox->m_Responses.GetSize());
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
		char *BranchName = NULL;
		bool DeleteName = false;
		if (Val->IsNULL()) {
			BranchName = new char[20];
			sprintf(BranchName, "line%d", Script->m_CurrentLine);
			DeleteName = true;
		} else BranchName = Val->GetString();

		StartDlgBranch(BranchName, Script->m_Filename == NULL ? "" : Script->m_Filename, Script->m_ThreadEvent == NULL ? "" : Script->m_ThreadEvent);
		Stack->PushNULL();
		if (DeleteName) delete[] BranchName;

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// EndDlgBranch
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "EndDlgBranch") == 0) {
		Stack->CorrectParams(1);

		char *BranchName = NULL;
		CScValue *Val = Stack->Pop();
		if (!Val->IsNULL()) BranchName = Val->GetString();
		EndDlgBranch(BranchName, Script->m_Filename == NULL ? "" : Script->m_Filename, Script->m_ThreadEvent == NULL ? "" : Script->m_ThreadEvent);

		Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetCurrentDlgBranch
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetCurrentDlgBranch") == 0) {
		Stack->CorrectParams(0);

		if (m_DlgPendingBranches.GetSize() > 0) {
			Stack->PushString(m_DlgPendingBranches[m_DlgPendingBranches.GetSize() - 1]);
		} else Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// TakeItem
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "TakeItem") == 0) {
		return m_InvObject->ScCallMethod(Script, Stack, ThisStack, Name);
	}

	//////////////////////////////////////////////////////////////////////////
	// DropItem
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "DropItem") == 0) {
		return m_InvObject->ScCallMethod(Script, Stack, ThisStack, Name);
	}

	//////////////////////////////////////////////////////////////////////////
	// GetItem
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetItem") == 0) {
		return m_InvObject->ScCallMethod(Script, Stack, ThisStack, Name);
	}

	//////////////////////////////////////////////////////////////////////////
	// HasItem
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "HasItem") == 0) {
		return m_InvObject->ScCallMethod(Script, Stack, ThisStack, Name);
	}

	//////////////////////////////////////////////////////////////////////////
	// IsItemTaken
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "IsItemTaken") == 0) {
		Stack->CorrectParams(1);

		CScValue *val = Stack->Pop();
		if (!val->IsNULL()) {
			for (int i = 0; i < m_Inventories.GetSize(); i++) {
				CAdInventory *Inv = m_Inventories[i];

				for (int j = 0; j < Inv->m_TakenItems.GetSize(); j++) {
					if (val->GetNative() == Inv->m_TakenItems[j]) {
						Stack->PushBool(true);
						return S_OK;
					} else if (scumm_stricmp(val->GetString(), Inv->m_TakenItems[j]->m_Name) == 0) {
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
		if (m_InventoryBox && m_InventoryBox->m_Window)
			Stack->PushNative(m_InventoryBox->m_Window, true);
		else
			Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetResponsesWindow
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetResponsesWindow") == 0 || strcmp(Name, "GetResponseWindow") == 0) {
		Stack->CorrectParams(0);
		if (m_ResponseBox && m_ResponseBox->m_Window)
			Stack->PushNative(m_ResponseBox->m_Window, true);
		else
			Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LoadResponseBox
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "LoadResponseBox") == 0) {
		Stack->CorrectParams(1);
		char *Filename = Stack->Pop()->GetString();

		Game->UnregisterObject(m_ResponseBox);
		m_ResponseBox = new CAdResponseBox(Game);
		if (m_ResponseBox && !FAILED(m_ResponseBox->LoadFile(Filename))) {
			RegisterObject(m_ResponseBox);
			Stack->PushBool(true);
		} else {
			SAFE_DELETE(m_ResponseBox);
			Stack->PushBool(false);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LoadInventoryBox
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "LoadInventoryBox") == 0) {
		Stack->CorrectParams(1);
		char *Filename = Stack->Pop()->GetString();

		Game->UnregisterObject(m_InventoryBox);
		m_InventoryBox = new CAdInventoryBox(Game);
		if (m_InventoryBox && !FAILED(m_InventoryBox->LoadFile(Filename))) {
			RegisterObject(m_InventoryBox);
			Stack->PushBool(true);
		} else {
			delete m_InventoryBox;
			m_InventoryBox = NULL;
			Stack->PushBool(false);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LoadItems
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "LoadItems") == 0) {
		Stack->CorrectParams(2);
		char *Filename = Stack->Pop()->GetString();
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
		char *Dir = Stack->Pop()->GetString();
		Stack->PushBool(SUCCEEDED(AddSpeechDir(Dir)));

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RemoveSpeechDir
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "RemoveSpeechDir") == 0) {
		Stack->CorrectParams(1);
		char *Dir = Stack->Pop()->GetString();
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

		if (Width <= 0) Width = m_Renderer->m_Width;
		if (Height <= 0) Height = m_Renderer->m_Height;

		if (!m_SceneViewport) m_SceneViewport = new CBViewport(Game);
		if (m_SceneViewport) m_SceneViewport->SetRect(X, Y, X + Width, Y + Height);

		Stack->PushBool(true);

		return S_OK;
	}


	else return CBGame::ScCallMethod(Script, Stack, ThisStack, Name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CAdGame::ScGetProperty(char *Name) {
	m_ScValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		m_ScValue->SetString("game");
		return m_ScValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Scene
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Scene") == 0) {
		if (m_Scene) m_ScValue->SetNative(m_Scene, true);
		else m_ScValue->SetNULL();

		return m_ScValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// SelectedItem
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SelectedItem") == 0) {
		//if(m_SelectedItem) m_ScValue->SetString(m_SelectedItem->m_Name);
		if (m_SelectedItem) m_ScValue->SetNative(m_SelectedItem, true);
		else m_ScValue->SetNULL();

		return m_ScValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// NumItems
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "NumItems") == 0) {
		return m_InvObject->ScGetProperty(Name);
	}

	//////////////////////////////////////////////////////////////////////////
	// SmartItemCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SmartItemCursor") == 0) {
		m_ScValue->SetBool(m_SmartItemCursor);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// InventoryVisible
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "InventoryVisible") == 0) {
		m_ScValue->SetBool(m_InventoryBox && m_InventoryBox->m_Visible);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// InventoryScrollOffset
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "InventoryScrollOffset") == 0) {
		if (m_InventoryBox) m_ScValue->SetInt(m_InventoryBox->m_ScrollOffset);
		else m_ScValue->SetInt(0);

		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ResponsesVisible (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ResponsesVisible") == 0) {
		m_ScValue->SetBool(m_StateEx == GAME_WAITING_RESPONSE);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// PrevScene / PreviousScene (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "PrevScene") == 0 || strcmp(Name, "PreviousScene") == 0) {
		if (!m_PrevSceneName) m_ScValue->SetString("");
		else m_ScValue->SetString(m_PrevSceneName);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// PrevSceneFilename / PreviousSceneFilename (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "PrevSceneFilename") == 0 || strcmp(Name, "PreviousSceneFilename") == 0) {
		if (!m_PrevSceneFilename) m_ScValue->SetString("");
		else m_ScValue->SetString(m_PrevSceneFilename);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// LastResponse (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "LastResponse") == 0) {
		if (!m_ResponseBox || !m_ResponseBox->m_LastResponseText) m_ScValue->SetString("");
		else m_ScValue->SetString(m_ResponseBox->m_LastResponseText);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// LastResponseOrig (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "LastResponseOrig") == 0) {
		if (!m_ResponseBox || !m_ResponseBox->m_LastResponseTextOrig) m_ScValue->SetString("");
		else m_ScValue->SetString(m_ResponseBox->m_LastResponseTextOrig);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// InventoryObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "InventoryObject") == 0) {
		if (m_InventoryOwner == m_InvObject) m_ScValue->SetNative(this, true);
		else m_ScValue->SetNative(m_InventoryOwner, true);

		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// TotalNumItems
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "TotalNumItems") == 0) {
		m_ScValue->SetInt(m_Items.GetSize());
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// TalkSkipButton
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "TalkSkipButton") == 0) {
		m_ScValue->SetInt(m_TalkSkipButton);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ChangingScene
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ChangingScene") == 0) {
		m_ScValue->SetBool(m_ScheduledScene != NULL);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// StartupScene
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "StartupScene") == 0) {
		if (!m_StartupScene) m_ScValue->SetNULL();
		else m_ScValue->SetString(m_StartupScene);
		return m_ScValue;
	}

	else return CBGame::ScGetProperty(Name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::ScSetProperty(char *Name, CScValue *Value) {

	//////////////////////////////////////////////////////////////////////////
	// SelectedItem
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "SelectedItem") == 0) {
		if (Value->IsNULL()) m_SelectedItem = NULL;
		else {
			if (Value->IsNative()) {
				m_SelectedItem = NULL;
				for (int i = 0; i < m_Items.GetSize(); i++) {
					if (m_Items[i] == Value->GetNative()) {
						m_SelectedItem = (CAdItem *)Value->GetNative();
						break;
					}
				}
			} else {
				// try to get by name
				m_SelectedItem = GetItemByName(Value->GetString());
			}
		}

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SmartItemCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SmartItemCursor") == 0) {
		m_SmartItemCursor = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// InventoryVisible
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "InventoryVisible") == 0) {
		if (m_InventoryBox) m_InventoryBox->m_Visible = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// InventoryObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "InventoryObject") == 0) {
		if (m_InventoryOwner && m_InventoryBox) m_InventoryOwner->GetInventory()->m_ScrollOffset = m_InventoryBox->m_ScrollOffset;

		if (Value->IsNULL()) m_InventoryOwner = m_InvObject;
		else {
			CBObject *Obj = (CBObject *)Value->GetNative();
			if (Obj == this) m_InventoryOwner = m_InvObject;
			else if (Game->ValidObject(Obj)) m_InventoryOwner = (CAdObject *)Obj;
		}

		if (m_InventoryOwner && m_InventoryBox) m_InventoryBox->m_ScrollOffset = m_InventoryOwner->GetInventory()->m_ScrollOffset;

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// InventoryScrollOffset
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "InventoryScrollOffset") == 0) {
		if (m_InventoryBox) m_InventoryBox->m_ScrollOffset = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// TalkSkipButton
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "TalkSkipButton") == 0) {
		int Val = Value->GetInt();
		if (Val < 0) Val = 0;
		if (Val > TALK_SKIP_NONE) Val = TALK_SKIP_NONE;
		m_TalkSkipButton = (TTalkSkipButton)Val;
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// StartupScene
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "StartupScene") == 0) {
		if (Value == NULL) {
			delete[] m_StartupScene;
			m_StartupScene = NULL;
		} else CBUtils::SetString(&m_StartupScene, Value->GetString());

		return S_OK;
	}

	else return CBGame::ScSetProperty(Name, Value);
}


//////////////////////////////////////////////////////////////////////////
void CAdGame::PublishNatives() {
	if (!m_ScEngine || !m_ScEngine->m_CompilerAvailable) return;

	CBGame::PublishNatives();

	m_ScEngine->ExtDefineFunction("Actor");
	m_ScEngine->ExtDefineFunction("Entity");
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
	if (m_CursorHidden) return S_OK;

	if (m_SelectedItem && Game->m_State == GAME_RUNNING && m_StateEx == GAME_NORMAL && m_Interactive) {
		if (m_SelectedItem->m_CursorCombined) {
			CBSprite *OrigLastCursor = m_LastCursor;
			CBGame::ShowCursor();
			m_LastCursor = OrigLastCursor;
		}
		if (m_ActiveObject && m_SelectedItem->m_CursorHover && m_ActiveObject->GetExtendedFlag("usable")) {
			if (!m_SmartItemCursor || m_ActiveObject->CanHandleEvent(m_SelectedItem->m_Name))
				return DrawCursor(m_SelectedItem->m_CursorHover);
			else
				return DrawCursor(m_SelectedItem->m_CursorNormal);
		} else return DrawCursor(m_SelectedItem->m_CursorNormal);
	} else return CBGame::ShowCursor();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::LoadFile(const char *Filename) {
	byte *Buffer = m_FileManager->ReadWholeFile(Filename);
	if (Buffer == NULL) {
		Game->LOG(0, "CAdGame::LoadFile failed for file '%s'", Filename);
		return E_FAIL;
	}

	HRESULT ret;

	m_Filename = new char [strlen(Filename) + 1];
	strcpy(m_Filename, Filename);

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
					delete m_ResponseBox;
					m_ResponseBox = new CAdResponseBox(Game);
					if (m_ResponseBox && !FAILED(m_ResponseBox->LoadFile((char *)params2)))
						RegisterObject(m_ResponseBox);
					else {
						delete m_ResponseBox;
						m_ResponseBox = NULL;
						cmd = PARSERR_GENERIC;
					}
					break;

				case TOKEN_INVENTORY_BOX:
					delete m_InventoryBox;
					m_InventoryBox = new CAdInventoryBox(Game);
					if (m_InventoryBox && !FAILED(m_InventoryBox->LoadFile((char *)params2)))
						RegisterObject(m_InventoryBox);
					else {
						delete m_InventoryBox;
						m_InventoryBox = NULL;
						cmd = PARSERR_GENERIC;
					}
					break;

				case TOKEN_ITEMS:
					ItemsFound = true;
					CBUtils::SetString(&m_ItemsFile, (char *)params2);
					if (FAILED(LoadItemsFile(m_ItemsFile))) {
						delete[] m_ItemsFile;
						m_ItemsFile = NULL;
						cmd = PARSERR_GENERIC;
					}
					break;

				case TOKEN_TALK_SKIP_BUTTON:
					if (scumm_stricmp((char *)params2, "right") == 0) m_TalkSkipButton = TALK_SKIP_RIGHT;
					else if (scumm_stricmp((char *)params2, "both") == 0) m_TalkSkipButton = TALK_SKIP_BOTH;
					else m_TalkSkipButton = TALK_SKIP_LEFT;
					break;

				case TOKEN_SCENE_VIEWPORT: {
					RECT rc;
					parser.ScanStr((char *)params2, "%d,%d,%d,%d", &rc.left, &rc.top, &rc.right, &rc.bottom);
					if (!m_SceneViewport) m_SceneViewport = new CBViewport(Game);
					if (m_SceneViewport) m_SceneViewport->SetRect(rc.left, rc.top, rc.right, rc.bottom);
				}
				break;

				case TOKEN_EDITOR_PROPERTY:
					ParseEditorProperty(params2, false);
					break;

				case TOKEN_STARTUP_SCENE:
					CBUtils::SetString(&m_StartupScene, (char *)params2);
					break;

				case TOKEN_DEBUG_STARTUP_SCENE:
					CBUtils::SetString(&m_DebugStartupScene, (char *)params2);
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
	if (!PersistMgr->m_Saving) Cleanup();
	CBGame::Persist(PersistMgr);

	m_DlgPendingBranches.Persist(PersistMgr);

	m_Inventories.Persist(PersistMgr);
	PersistMgr->Transfer(TMEMBER(m_InventoryBox));

	m_Objects.Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(m_PrevSceneName));
	PersistMgr->Transfer(TMEMBER(m_PrevSceneFilename));

	PersistMgr->Transfer(TMEMBER(m_ResponseBox));
	m_ResponsesBranch.Persist(PersistMgr);
	m_ResponsesGame.Persist(PersistMgr);
	PersistMgr->Transfer(TMEMBER(m_Scene));
	m_SceneStates.Persist(PersistMgr);
	PersistMgr->Transfer(TMEMBER(m_ScheduledFadeIn));
	PersistMgr->Transfer(TMEMBER(m_ScheduledScene));
	PersistMgr->Transfer(TMEMBER(m_SelectedItem));
	PersistMgr->Transfer(TMEMBER_INT(m_TalkSkipButton));

	m_Sentences.Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(m_SceneViewport));
	PersistMgr->Transfer(TMEMBER_INT(m_StateEx));
	PersistMgr->Transfer(TMEMBER(m_InitialScene));
	PersistMgr->Transfer(TMEMBER(m_DebugStartupScene));

	PersistMgr->Transfer(TMEMBER(m_InvObject));
	PersistMgr->Transfer(TMEMBER(m_InventoryOwner));
	PersistMgr->Transfer(TMEMBER(m_TempDisableSaveState));
	m_Items.Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(m_ItemsFile));

	m_SpeechDirs.Persist(PersistMgr);
	PersistMgr->Transfer(TMEMBER(m_SmartItemCursor));

	if (!PersistMgr->m_Saving) m_InitialScene = false;

	PersistMgr->Transfer(TMEMBER(m_StartupScene));


	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::LoadGame(char *Filename) {
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
void CAdGame::SetPrevSceneName(char *Name) {
	delete[] m_PrevSceneName;
	m_PrevSceneName = NULL;
	if (Name) {
		m_PrevSceneName = new char[strlen(Name) + 1];
		if (m_PrevSceneName) strcpy(m_PrevSceneName, Name);
	}
}


//////////////////////////////////////////////////////////////////////////
void CAdGame::SetPrevSceneFilename(char *Name) {
	delete[] m_PrevSceneFilename;
	m_PrevSceneFilename = NULL;
	if (Name) {
		m_PrevSceneFilename = new char[strlen(Name) + 1];
		if (m_PrevSceneFilename) strcpy(m_PrevSceneFilename, Name);
	}
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::ScheduleChangeScene(char *Filename, bool FadeIn) {
	delete[] m_ScheduledScene;
	m_ScheduledScene = NULL;

	if (m_Scene && !m_Scene->m_Initialized) return ChangeScene(Filename, FadeIn);
	else {
		m_ScheduledScene = new char [strlen(Filename) + 1];
		strcpy(m_ScheduledScene, Filename);

		m_ScheduledFadeIn = FadeIn;

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
HRESULT CAdGame::LoadItemsFile(char *Filename, bool Merge) {
	byte *Buffer = Game->m_FileManager->ReadWholeFile(Filename);
	if (Buffer == NULL) {
		Game->LOG(0, "CAdGame::LoadItemsFile failed for file '%s'", Filename);
		return E_FAIL;
	}

	HRESULT ret;

	//m_Filename = new char [strlen(Filename)+1];
	//strcpy(m_Filename, Filename);

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
		while (m_Items.GetSize() > 0) DeleteItem(m_Items[0]);
	}

	while ((cmd = parser.GetCommand((char **)&Buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_ITEM: {
			CAdItem *item = new CAdItem(Game);
			if (item && !FAILED(item->LoadBuffer(params, false))) {
				// delete item with the same name, if exists
				if (Merge) {
					CAdItem *PrevItem = GetItemByName(item->m_Name);
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
CAdSceneState *CAdGame::GetSceneState(char *Filename, bool Saving) {
	char *FilenameCor = new char[strlen(Filename) + 1];
	strcpy(FilenameCor, Filename);
	for (int i = 0; i < strlen(FilenameCor); i++) {
		if (FilenameCor[i] == '/') FilenameCor[i] = '\\';
	}

	for (int i = 0; i < m_SceneStates.GetSize(); i++) {
		if (scumm_stricmp(m_SceneStates[i]->m_Filename, FilenameCor) == 0) {
			delete [] FilenameCor;
			return m_SceneStates[i];
		}
	}

	if (Saving) {
		CAdSceneState *ret = new CAdSceneState(Game);
		ret->SetFilename(FilenameCor);

		m_SceneStates.Add(ret);

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
		if (!ent || FAILED(ent->LoadBuffer((byte  *)*params, false))) {
			delete ent;
			ent = NULL;
			cmd = PARSERR_GENERIC;
		} else {
			ent->m_Parent = Win;
			Win->m_Widgets.Add(ent);
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
HRESULT CAdGame::WindowScriptMethodHook(CUIWindow *Win, CScScript *Script, CScStack *Stack, char *Name) {
	if (strcmp(Name, "CreateEntityContainer") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();

		CUIEntity *Ent = new CUIEntity(Game);
		if (!Val->IsNULL()) Ent->SetName(Val->GetString());
		Stack->PushNative(Ent, true);

		Ent->m_Parent = Win;
		Win->m_Widgets.Add(Ent);

		return S_OK;
	} else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::StartDlgBranch(const char *BranchName, const char *ScriptName, const char *EventName) {
	char *Name = new char[strlen(BranchName) + 1 + strlen(ScriptName) + 1 + strlen(EventName) + 1];
	if (Name) {
		sprintf(Name, "%s.%s.%s", BranchName, ScriptName, EventName);
		m_DlgPendingBranches.Add(Name);
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::EndDlgBranch(const char *BranchName, const char *ScriptName, const char *EventName) {
	char *Name = NULL;
	bool DeleteName = false;
	if (BranchName == NULL && m_DlgPendingBranches.GetSize() > 0) {
		Name = m_DlgPendingBranches[m_DlgPendingBranches.GetSize() - 1];
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
	for (i = m_DlgPendingBranches.GetSize() - 1; i >= 0; i--) {
		if (scumm_stricmp(Name, m_DlgPendingBranches[i]) == 0) {
			StartIndex = i;
			break;
		}
	}
	if (StartIndex >= 0) {
		for (i = StartIndex; i < m_DlgPendingBranches.GetSize(); i++) {
			//ClearBranchResponses(m_DlgPendingBranches[i]);
			delete [] m_DlgPendingBranches[i];
			m_DlgPendingBranches[i] = NULL;
		}
		m_DlgPendingBranches.RemoveAt(StartIndex, m_DlgPendingBranches.GetSize() - StartIndex);
	}

	// dialogue is over, forget selected responses
	if (m_DlgPendingBranches.GetSize() == 0) {
		for (int i = 0; i < m_ResponsesBranch.GetSize(); i++) delete m_ResponsesBranch[i];
		m_ResponsesBranch.RemoveAll();
	}

	if (DeleteName) delete [] Name;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::ClearBranchResponses(char *Name) {
	for (int i = 0; i < m_ResponsesBranch.GetSize(); i++) {
		if (scumm_stricmp(Name, m_ResponsesBranch[i]->m_Context) == 0) {
			delete m_ResponsesBranch[i];
			m_ResponsesBranch.RemoveAt(i);
			i--;
		}
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::AddBranchResponse(int ID) {
	if (BranchResponseUsed(ID)) return S_OK;
	CAdResponseContext *r = new CAdResponseContext(Game);
	r->m_ID = ID;
	r->SetContext(m_DlgPendingBranches.GetSize() > 0 ? m_DlgPendingBranches[m_DlgPendingBranches.GetSize() - 1] : NULL);
	m_ResponsesBranch.Add(r);
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CAdGame::BranchResponseUsed(int ID) {
	char *Context = m_DlgPendingBranches.GetSize() > 0 ? m_DlgPendingBranches[m_DlgPendingBranches.GetSize() - 1] : NULL;
	for (int i = 0; i < m_ResponsesBranch.GetSize(); i++) {
		if (m_ResponsesBranch[i]->m_ID == ID) {
			if ((Context == NULL && m_ResponsesBranch[i]->m_Context == NULL) || scumm_stricmp(Context, m_ResponsesBranch[i]->m_Context) == 0) return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::AddGameResponse(int ID) {
	if (GameResponseUsed(ID)) return S_OK;
	CAdResponseContext *r = new CAdResponseContext(Game);
	r->m_ID = ID;
	r->SetContext(m_DlgPendingBranches.GetSize() > 0 ? m_DlgPendingBranches[m_DlgPendingBranches.GetSize() - 1] : NULL);
	m_ResponsesGame.Add(r);
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CAdGame::GameResponseUsed(int ID) {
	char *Context = m_DlgPendingBranches.GetSize() > 0 ? m_DlgPendingBranches[m_DlgPendingBranches.GetSize() - 1] : NULL;
	for (int i = 0; i < m_ResponsesGame.GetSize(); i++) {
		CAdResponseContext *RespContext = m_ResponsesGame[i];
		if (RespContext->m_ID == ID) {
			if ((Context == NULL && RespContext->m_Context == NULL) || ((Context != NULL && RespContext->m_Context != NULL) && scumm_stricmp(Context, RespContext->m_Context) == 0)) return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::ResetResponse(int ID) {
	char *Context = m_DlgPendingBranches.GetSize() > 0 ? m_DlgPendingBranches[m_DlgPendingBranches.GetSize() - 1] : NULL;

	int i;

	for (i = 0; i < m_ResponsesGame.GetSize(); i++) {
		if (m_ResponsesGame[i]->m_ID == ID) {
			if ((Context == NULL && m_ResponsesGame[i]->m_Context == NULL) || scumm_stricmp(Context, m_ResponsesGame[i]->m_Context) == 0) {
				delete m_ResponsesGame[i];
				m_ResponsesGame.RemoveAt(i);
				break;
			}
		}
	}

	for (i = 0; i < m_ResponsesBranch.GetSize(); i++) {
		if (m_ResponsesBranch[i]->m_ID == ID) {
			if ((Context == NULL && m_ResponsesBranch[i]->m_Context == NULL) || scumm_stricmp(Context, m_ResponsesBranch[i]->m_Context) == 0) {
				delete m_ResponsesBranch[i];
				m_ResponsesBranch.RemoveAt(i);
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
	m_Renderer->Fill(0, 0, 0);
	if (!m_EditorMode) m_Renderer->SetScreenViewport();

	// process scripts
	if (Update) m_ScEngine->Tick();

	POINT p;
	GetMousePos(&p);

	m_Scene->Update();
	m_Scene->Display();


	// display in-game windows
	DisplayWindows(true);
	if (m_InventoryBox) m_InventoryBox->Display();
	if (m_StateEx == GAME_WAITING_RESPONSE) m_ResponseBox->Display();
	if (m_IndicatorDisplay) DisplayIndicator();


	if (Update || DisplayAll) {
		// display normal windows
		DisplayWindows(false);

		SetActiveObject(Game->m_Renderer->GetObjectAt(p.x, p.y));

		// textual info
		DisplaySentences(m_State == GAME_FROZEN);

		ShowCursor();

		if (m_Fader) m_Fader->Display();
		m_TransMgr->Update();
	}


	if (m_LoadingIcon) {
		m_LoadingIcon->Display(m_LoadingIconX, m_LoadingIconY);
		if (!m_LoadingIconPersistent) {
			delete m_LoadingIcon;
			m_LoadingIcon = NULL;
		}
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::RegisterInventory(CAdInventory *Inv) {
	for (int i = 0; i < m_Inventories.GetSize(); i++) {
		if (m_Inventories[i] == Inv) return S_OK;
	}
	RegisterObject(Inv);
	m_Inventories.Add(Inv);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::UnregisterInventory(CAdInventory *Inv) {
	for (int i = 0; i < m_Inventories.GetSize(); i++) {
		if (m_Inventories[i] == Inv) {
			UnregisterObject(m_Inventories[i]);
			m_Inventories.RemoveAt(i);
			return S_OK;
		}
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
bool CAdGame::IsItemTaken(char *ItemName) {
	int i;

	for (i = 0; i < m_Inventories.GetSize(); i++) {
		CAdInventory *Inv = m_Inventories[i];

		for (int j = 0; j < Inv->m_TakenItems.GetSize(); j++) {
			if (scumm_stricmp(ItemName, Inv->m_TakenItems[j]->m_Name) == 0) {
				return true;
			}
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
CAdItem *CAdGame::GetItemByName(char *Name) {
	for (int i = 0; i < m_Items.GetSize(); i++) {
		if (scumm_stricmp(m_Items[i]->m_Name, Name) == 0) return m_Items[i];
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::AddItem(CAdItem *Item) {
	m_Items.Add(Item);
	return Game->RegisterObject(Item);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::ResetContent() {
	int i;

	// clear pending dialogs
	for (i = 0; i < m_DlgPendingBranches.GetSize(); i++) {
		delete [] m_DlgPendingBranches[i];
	}
	m_DlgPendingBranches.RemoveAll();


	// clear inventories
	for (i = 0; i < m_Inventories.GetSize(); i++) {
		m_Inventories[i]->m_TakenItems.RemoveAll();
	}

	// clear scene states
	for (i = 0; i < m_SceneStates.GetSize(); i++) delete m_SceneStates[i];
	m_SceneStates.RemoveAll();

	// clear once responses
	for (i = 0; i < m_ResponsesBranch.GetSize(); i++) delete m_ResponsesBranch[i];
	m_ResponsesBranch.RemoveAll();

	// clear once game responses
	for (i = 0; i < m_ResponsesGame.GetSize(); i++) delete m_ResponsesGame[i];
	m_ResponsesGame.RemoveAll();

	// reload inventory items
	if (m_ItemsFile) LoadItemsFile(m_ItemsFile);

	m_TempDisableSaveState = true;

	return CBGame::ResetContent();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::DeleteItem(CAdItem *Item) {
	if (!Item) return E_FAIL;

	if (m_SelectedItem == Item) m_SelectedItem = NULL;
	m_Scene->HandleItemAssociations(Item->m_Name, false);

	// remove from all inventories
	for (int i = 0; i < m_Inventories.GetSize(); i++) {
		m_Inventories[i]->RemoveItem(Item);
	}

	// remove object
	for (int i = 0; i < m_Items.GetSize(); i++) {
		if (m_Items[i] == Item) {
			UnregisterObject(m_Items[i]);
			m_Items.RemoveAt(i);
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

	for (int i = 0; i < m_SpeechDirs.GetSize(); i++) {
		if (scumm_stricmp(m_SpeechDirs[i], Temp) == 0) {
			delete [] Temp;
			return S_OK;
		}
	}
	m_SpeechDirs.Add(Temp);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::RemoveSpeechDir(char *Dir) {
	if (!Dir || Dir[0] == '\0') return E_FAIL;

	char *Temp = new char[strlen(Dir) + 2];
	strcpy(Temp, Dir);
	if (Temp[strlen(Temp) - 1] != '\\' && Temp[strlen(Temp) - 1] != '/')
		strcat(Temp, "\\");

	bool Found = false;
	for (int i = 0; i < m_SpeechDirs.GetSize(); i++) {
		if (scumm_stricmp(m_SpeechDirs[i], Temp) == 0) {
			delete [] m_SpeechDirs[i];
			m_SpeechDirs.RemoveAt(i);
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

	for (int i = 0; i < m_SpeechDirs.GetSize(); i++) {
		sprintf(Ret, "%s%s.ogg", m_SpeechDirs[i], StringID);
		CBFile *File = m_FileManager->OpenFile(Ret);
		if (File) {
			m_FileManager->CloseFile(File);
			return Ret;
		}

		sprintf(Ret, "%s%s.wav", m_SpeechDirs[i], StringID);
		File = m_FileManager->OpenFile(Ret);
		if (File) {
			m_FileManager->CloseFile(File);
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

	return m_Renderer->PointInViewport(&Pos);
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::OnMouseLeftDown() {
	if (!ValidMouse()) return S_OK;
	if (m_State == GAME_RUNNING && !m_Interactive) {
		if (m_TalkSkipButton == TALK_SKIP_LEFT || m_TalkSkipButton == TALK_SKIP_BOTH) {
			FinishSentences();
		}
		return S_OK;
	}

	if (m_ActiveObject) m_ActiveObject->HandleMouse(MOUSE_CLICK, MOUSE_BUTTON_LEFT);

	bool Handled = m_State == GAME_RUNNING && SUCCEEDED(ApplyEvent("LeftClick"));
	if (!Handled) {
		if (m_ActiveObject != NULL) {
			m_ActiveObject->ApplyEvent("LeftClick");
		} else if (m_State == GAME_RUNNING && m_Scene && m_Scene->PointInViewport(m_MousePos.x, m_MousePos.y)) {
			m_Scene->ApplyEvent("LeftClick");
		}
	}

	if (m_ActiveObject != NULL) Game->m_CapturedObject = Game->m_ActiveObject;
	m_MouseLeftDown = true;
	CBPlatform::SetCapture(m_Renderer->m_Window);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::OnMouseLeftUp() {
	if (m_ActiveObject) m_ActiveObject->HandleMouse(MOUSE_RELEASE, MOUSE_BUTTON_LEFT);

	CBPlatform::ReleaseCapture();
	m_CapturedObject = NULL;
	m_MouseLeftDown = false;

	bool Handled = /*m_State==GAME_RUNNING &&*/ SUCCEEDED(ApplyEvent("LeftRelease"));
	if (!Handled) {
		if (m_ActiveObject != NULL) {
			m_ActiveObject->ApplyEvent("LeftRelease");
		} else if (m_State == GAME_RUNNING && m_Scene && m_Scene->PointInViewport(m_MousePos.x, m_MousePos.y)) {
			m_Scene->ApplyEvent("LeftRelease");
		}
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::OnMouseLeftDblClick() {
	if (!ValidMouse()) return S_OK;

	if (m_State == GAME_RUNNING && !m_Interactive) return S_OK;

	if (m_ActiveObject) m_ActiveObject->HandleMouse(MOUSE_DBLCLICK, MOUSE_BUTTON_LEFT);

	bool Handled = m_State == GAME_RUNNING && SUCCEEDED(ApplyEvent("LeftDoubleClick"));
	if (!Handled) {
		if (m_ActiveObject != NULL) {
			m_ActiveObject->ApplyEvent("LeftDoubleClick");
		} else if (m_State == GAME_RUNNING && m_Scene && m_Scene->PointInViewport(m_MousePos.x, m_MousePos.y)) {
			m_Scene->ApplyEvent("LeftDoubleClick");
		}
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::OnMouseRightDown() {
	if (!ValidMouse()) return S_OK;
	if (m_State == GAME_RUNNING && !m_Interactive) {
		if (m_TalkSkipButton == TALK_SKIP_RIGHT || m_TalkSkipButton == TALK_SKIP_BOTH) {
			FinishSentences();
		}
		return S_OK;
	}

	if ((m_State == GAME_RUNNING && !m_Interactive) || m_StateEx == GAME_WAITING_RESPONSE) return S_OK;

	if (m_ActiveObject) m_ActiveObject->HandleMouse(MOUSE_CLICK, MOUSE_BUTTON_RIGHT);

	bool Handled = m_State == GAME_RUNNING && SUCCEEDED(ApplyEvent("RightClick"));
	if (!Handled) {
		if (m_ActiveObject != NULL) {
			m_ActiveObject->ApplyEvent("RightClick");
		} else if (m_State == GAME_RUNNING && m_Scene && m_Scene->PointInViewport(m_MousePos.x, m_MousePos.y)) {
			m_Scene->ApplyEvent("RightClick");
		}
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::OnMouseRightUp() {
	if (m_ActiveObject) m_ActiveObject->HandleMouse(MOUSE_RELEASE, MOUSE_BUTTON_RIGHT);

	bool Handled = m_State == GAME_RUNNING && SUCCEEDED(ApplyEvent("RightRelease"));
	if (!Handled) {
		if (m_ActiveObject != NULL) {
			m_ActiveObject->ApplyEvent("RightRelease");
		} else if (m_State == GAME_RUNNING && m_Scene && m_Scene->PointInViewport(m_MousePos.x, m_MousePos.y)) {
			m_Scene->ApplyEvent("RightRelease");
		}
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::DisplayDebugInfo() {
	char str[100];
	if (Game->m_DEBUG_DebugMode) {
		sprintf(str, "Mouse: %d, %d (scene: %d, %d)", m_MousePos.x, m_MousePos.y, m_MousePos.x + m_Scene->GetOffsetLeft(), m_MousePos.y + m_Scene->GetOffsetTop());
		m_SystemFont->DrawText((byte  *)str, 0, 90, m_Renderer->m_Width, TAL_RIGHT);

		sprintf(str, "Scene: %s (prev: %s)", (m_Scene && m_Scene->m_Name) ? m_Scene->m_Name : "???", m_PrevSceneName ? m_PrevSceneName : "???");
		m_SystemFont->DrawText((byte  *)str, 0, 110, m_Renderer->m_Width, TAL_RIGHT);
	}
	return CBGame::DisplayDebugInfo();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdGame::OnScriptShutdown(CScScript *Script) {
	if (m_ResponseBox && m_ResponseBox->m_WaitingScript == Script)
		m_ResponseBox->m_WaitingScript = NULL;

	return S_OK;
}

} // end of namespace WinterMute
