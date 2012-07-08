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
	virtual HRESULT onScriptShutdown(CScScript *script);

	virtual HRESULT onMouseLeftDown();
	virtual HRESULT onMouseLeftUp();
	virtual HRESULT onMouseLeftDblClick();
	virtual HRESULT onMouseRightDown();
	virtual HRESULT onMouseRightUp();

	virtual HRESULT displayDebugInfo();


	virtual HRESULT initAfterLoad();
	static void afterLoadScene(void *scene, void *data);

	bool _smartItemCursor;

	CBArray<char *, char *> _speechDirs;
	HRESULT addSpeechDir(const char *dir);
	HRESULT removeSpeechDir(const char *dir);
	char *findSpeechFile(char *StringID);

	HRESULT deleteItem(CAdItem *Item);
	char *_itemsFile;
	bool _tempDisableSaveState;
	virtual HRESULT resetContent();
	HRESULT addItem(CAdItem *item);
	CAdItem *getItemByName(const char *name);
	CBArray<CAdItem *, CAdItem *> _items;
	CAdObject *_inventoryOwner;
	bool isItemTaken(char *itemName);
	HRESULT registerInventory(CAdInventory *inv);
	HRESULT unregisterInventory(CAdInventory *inv);

	CAdObject *_invObject;
	CBArray<CAdInventory *, CAdInventory *> _inventories;
	virtual HRESULT displayContent(bool update = true, bool displayAll = false);
	char *_debugStartupScene;
	char *_startupScene;
	bool _initialScene;
	bool gameResponseUsed(int ID);
	HRESULT addGameResponse(int ID);
	HRESULT resetResponse(int ID);

	bool branchResponseUsed(int ID);
	HRESULT addBranchResponse(int ID);
	HRESULT clearBranchResponses(char *name);
	HRESULT startDlgBranch(const char *branchName, const char *scriptName, const char *eventName);
	HRESULT endDlgBranch(const char *branchName, const char *scriptName, const char *eventName);
	virtual HRESULT windowLoadHook(CUIWindow *win, char **buf, char **params);
	virtual HRESULT windowScriptMethodHook(CUIWindow *win, CScScript *script, CScStack *stack, const char *name);

	CAdSceneState *getSceneState(const char *filename, bool saving);
	CBViewport *_sceneViewport;
	int _texItemLifeTime;
	int _texWalkLifeTime;
	int _texStandLifeTime;
	int _texTalkLifeTime;

	TTalkSkipButton _talkSkipButton;

	virtual HRESULT getVersion(byte *verMajor, byte *verMinor, byte *extMajor, byte *extMinor);
	HRESULT scheduleChangeScene(const char *filename, bool fadeIn);
	char *_scheduledScene;
	bool _scheduledFadeIn;
	void setPrevSceneName(const char *name);
	void setPrevSceneFilename(const char *name);
	char *_prevSceneName;
	char *_prevSceneFilename;
	virtual HRESULT loadGame(const char *filename);
	CAdItem *_selectedItem;
	HRESULT cleanup();
	DECLARE_PERSISTENT(CAdGame, CBGame)

	void finishSentences();
	HRESULT showCursor();
	TGameStateEx _stateEx;
	CAdResponseBox *_responseBox;
	CAdInventoryBox *_inventoryBox;
	HRESULT displaySentences(bool frozen);
	void addSentence(CAdSentence *sentence);
	HRESULT changeScene(const char *filename, bool fadeIn);
	HRESULT removeObject(CAdObject *object);
	HRESULT addObject(CAdObject *object);
	CAdScene *_scene;
	HRESULT initLoop();
	CAdGame();
	virtual ~CAdGame();
	CBArray<CAdObject *, CAdObject *> _objects;
	CBArray<CAdSentence *, CAdSentence *> _sentences;

	CBArray<CAdSceneState *, CAdSceneState *> _sceneStates;
	CBArray<char *, char *> _dlgPendingBranches;

	CBArray<CAdResponseContext *, CAdResponseContext *> _responsesBranch;
	CBArray<CAdResponseContext *, CAdResponseContext *> _responsesGame;

	virtual HRESULT loadFile(const char *filename);
	virtual HRESULT loadBuffer(byte *buffer, bool complete = true);

	HRESULT loadItemsFile(const char *filename, bool merge = false);
	HRESULT loadItemsBuffer(byte *buffer, bool merge = false);


	virtual HRESULT ExternalCall(CScScript *script, CScStack *stack, CScStack *thisStack, char *name);

	// scripting interface
	virtual CScValue *scGetProperty(const char *name);
	virtual HRESULT scSetProperty(const char *name, CScValue *value);
	virtual HRESULT scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name);
	bool validMouse();
};

} // end of namespace WinterMute

#endif
