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
#ifndef WINTERMUTE_ADGAME_H
#define WINTERMUTE_ADGAME_H

#include "engines/wintermute/Ad/AdTypes.h"
#include "engines/wintermute/Base/BGame.h"

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

	bool _smartItemCursor;

	CBArray<char *, char *> _speechDirs;
	HRESULT AddSpeechDir(const char *Dir);
	HRESULT RemoveSpeechDir(const char *Dir);
	char *FindSpeechFile(char *StringID);

	HRESULT DeleteItem(CAdItem *Item);
	char *_itemsFile;
	bool _tempDisableSaveState;
	virtual HRESULT ResetContent();
	HRESULT AddItem(CAdItem *Item);
	CAdItem *GetItemByName(const char *Name);
	CBArray<CAdItem *, CAdItem *> _items;
	CAdObject *_inventoryOwner;
	bool IsItemTaken(char *ItemName);
	HRESULT RegisterInventory(CAdInventory *Inv);
	HRESULT UnregisterInventory(CAdInventory *Inv);

	CAdObject *_invObject;
	CBArray<CAdInventory *, CAdInventory *> _inventories;
	virtual HRESULT DisplayContent(bool Update = true, bool DisplayAll = false);
	char *_debugStartupScene;
	char *_startupScene;
	bool _initialScene;
	bool GameResponseUsed(int ID);
	HRESULT AddGameResponse(int ID);
	HRESULT ResetResponse(int ID);

	bool BranchResponseUsed(int ID);
	HRESULT AddBranchResponse(int ID);
	HRESULT ClearBranchResponses(char *Name);
	HRESULT StartDlgBranch(const char *BranchName, const char *ScriptName, const char *EventName);
	HRESULT EndDlgBranch(const char *BranchName, const char *ScriptName, const char *EventName);
	virtual HRESULT WindowLoadHook(CUIWindow *Win, char **Buf, char **Params);
	virtual HRESULT WindowScriptMethodHook(CUIWindow *Win, CScScript *Script, CScStack *Stack, const char *Name);

	CAdSceneState *GetSceneState(const char *Filename, bool Saving);
	CBViewport *_sceneViewport;
	int _texItemLifeTime;
	int _texWalkLifeTime;
	int _texStandLifeTime;
	int _texTalkLifeTime;

	TTalkSkipButton _talkSkipButton;

	virtual HRESULT GetVersion(byte  *VerMajor, byte *VerMinor, byte *ExtMajor, byte *ExtMinor);
	HRESULT ScheduleChangeScene(const char *Filename, bool FadeIn);
	char *_scheduledScene;
	bool _scheduledFadeIn;
	void SetPrevSceneName(const char *Name);
	void SetPrevSceneFilename(const char *Name);
	char *_prevSceneName;
	char *_prevSceneFilename;
	virtual HRESULT LoadGame(const char *Filename);
	CAdItem *_selectedItem;
	HRESULT Cleanup();
	DECLARE_PERSISTENT(CAdGame, CBGame)

	void FinishSentences();
	HRESULT ShowCursor();
	TGameStateEx _stateEx;
	CAdResponseBox *_responseBox;
	CAdInventoryBox *_inventoryBox;
	HRESULT DisplaySentences(bool Frozen);
	void AddSentence(CAdSentence *Sentence);
	HRESULT ChangeScene(const char *Filename, bool FadeIn);
	HRESULT RemoveObject(CAdObject *Object);
	HRESULT AddObject(CAdObject *Object);
	CAdScene *_scene;
	HRESULT InitLoop();
	CAdGame();
	virtual ~CAdGame();
	CBArray<CAdObject *, CAdObject *> _objects;
	CBArray<CAdSentence *, CAdSentence *> _sentences;

	CBArray<CAdSceneState *, CAdSceneState *> _sceneStates;
	CBArray<char *, char *> _dlgPendingBranches;

	CBArray<CAdResponseContext *, CAdResponseContext *> _responsesBranch;
	CBArray<CAdResponseContext *, CAdResponseContext *> _responsesGame;

	virtual HRESULT LoadFile(const char *Filename);
	virtual HRESULT LoadBuffer(byte  *Buffer, bool Complete = true);

	HRESULT LoadItemsFile(const char *Filename, bool Merge = false);
	HRESULT LoadItemsBuffer(byte  *Buffer, bool Merge = false);


	virtual void PublishNatives();
	virtual HRESULT ExternalCall(CScScript *Script, CScStack *Stack, CScStack *ThisStack, char *Name);

	// scripting interface
	virtual CScValue *ScGetProperty(const char *Name);
	virtual HRESULT ScSetProperty(const char *Name, CScValue *Value);
	virtual HRESULT ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, const char *Name);
	bool ValidMouse();
};

} // end of namespace WinterMute

#endif
