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
#ifndef __WmeAdGame_H__
#define __WmeAdGame_H__

#include "AdTypes.h"
#include "BGame.h"

namespace WinterMute {
class CAdItem;
class CAdInventory;
class CAdSceneState;
class CAdScene;
class CAdItem;
class CAdObject;
class CAdSentence;
class CAdInventoryBox;
class CAdResponseContext;
class CAdResponseBox;
class CAdGame : public CBGame {
public:
	virtual HRESULT OnScriptShutdown(CScScript *Script);

	virtual HRESULT OnMouseLeftDown();
	virtual HRESULT OnMouseLeftUp();
	virtual HRESULT OnMouseLeftDblClick();
	virtual HRESULT OnMouseRightDown();
	virtual HRESULT OnMouseRightUp();

	virtual HRESULT DisplayDebugInfo();


	virtual HRESULT InitAfterLoad();
	static void AfterLoadScene(void *Scene, void *Data);

	bool m_SmartItemCursor;

	CBArray<char *, char *> m_SpeechDirs;
	HRESULT AddSpeechDir(const char *Dir);
	HRESULT RemoveSpeechDir(char *Dir);
	char *FindSpeechFile(char *StringID);

	HRESULT DeleteItem(CAdItem *Item);
	char *m_ItemsFile;
	bool m_TempDisableSaveState;
	virtual HRESULT ResetContent();
	HRESULT AddItem(CAdItem *Item);
	CAdItem *GetItemByName(char *Name);
	CBArray<CAdItem *, CAdItem *> m_Items;
	CAdObject *m_InventoryOwner;
	bool IsItemTaken(char *ItemName);
	HRESULT RegisterInventory(CAdInventory *Inv);
	HRESULT UnregisterInventory(CAdInventory *Inv);

	CAdObject *m_InvObject;
	CBArray<CAdInventory *, CAdInventory *> m_Inventories;
	virtual HRESULT DisplayContent(bool Update = true, bool DisplayAll = false);
	char *m_DebugStartupScene;
	char *m_StartupScene;
	bool m_InitialScene;
	bool GameResponseUsed(int ID);
	HRESULT AddGameResponse(int ID);
	HRESULT ResetResponse(int ID);

	bool BranchResponseUsed(int ID);
	HRESULT AddBranchResponse(int ID);
	HRESULT ClearBranchResponses(char *Name);
	HRESULT StartDlgBranch(const char *BranchName, const char *ScriptName, const char *EventName);
	HRESULT EndDlgBranch(const char *BranchName, const char *ScriptName, const char *EventName);
	virtual HRESULT WindowLoadHook(CUIWindow *Win, char **Buf, char **Params);
	virtual HRESULT WindowScriptMethodHook(CUIWindow *Win, CScScript *Script, CScStack *Stack, char *Name);

	CAdSceneState *GetSceneState(char *Filename, bool Saving);
	CBViewport *m_SceneViewport;
	int m_TexItemLifeTime;
	int m_TexWalkLifeTime;
	int m_TexStandLifeTime;
	int m_TexTalkLifeTime;

	TTalkSkipButton m_TalkSkipButton;

	virtual HRESULT GetVersion(byte  *VerMajor, byte *VerMinor, byte *ExtMajor, byte *ExtMinor);
	HRESULT ScheduleChangeScene(char *Filename, bool FadeIn);
	char *m_ScheduledScene;
	bool m_ScheduledFadeIn;
	void SetPrevSceneName(char *Name);
	void SetPrevSceneFilename(char *Name);
	char *m_PrevSceneName;
	char *m_PrevSceneFilename;
	virtual HRESULT LoadGame(char *Filename);
	CAdItem *m_SelectedItem;
	HRESULT Cleanup();
	DECLARE_PERSISTENT(CAdGame, CBGame)

	void FinishSentences();
	HRESULT ShowCursor();
	TGameStateEx m_StateEx;
	CAdResponseBox *m_ResponseBox;
	CAdInventoryBox *m_InventoryBox;
	HRESULT DisplaySentences(bool Frozen);
	void AddSentence(CAdSentence *Sentence);
	HRESULT ChangeScene(char *Filename, bool FadeIn);
	HRESULT RemoveObject(CAdObject *Object);
	HRESULT AddObject(CAdObject *Object);
	CAdScene *m_Scene;
	HRESULT InitLoop();
	CAdGame();
	virtual ~CAdGame();
	CBArray<CAdObject *, CAdObject *> m_Objects;
	CBArray<CAdSentence *, CAdSentence *> m_Sentences;

	CBArray<CAdSceneState *, CAdSceneState *> m_SceneStates;
	CBArray<char *, char *> m_DlgPendingBranches;

	CBArray<CAdResponseContext *, CAdResponseContext *> m_ResponsesBranch;
	CBArray<CAdResponseContext *, CAdResponseContext *> m_ResponsesGame;

	virtual HRESULT LoadFile(const char *Filename);
	virtual HRESULT LoadBuffer(byte  *Buffer, bool Complete = true);

	HRESULT LoadItemsFile(char *Filename, bool Merge = false);
	HRESULT LoadItemsBuffer(byte  *Buffer, bool Merge = false);


	virtual void PublishNatives();
	virtual HRESULT ExternalCall(CScScript *Script, CScStack *Stack, CScStack *ThisStack, char *Name);

	// scripting interface
	virtual CScValue *ScGetProperty(char *Name);
	virtual HRESULT ScSetProperty(char *Name, CScValue *Value);
	virtual HRESULT ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, char *Name);
	bool ValidMouse();
};

} // end of namespace WinterMute

#endif
