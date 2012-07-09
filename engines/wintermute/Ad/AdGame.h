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
	virtual ERRORCODE onScriptShutdown(CScScript *script);

	virtual ERRORCODE onMouseLeftDown();
	virtual ERRORCODE onMouseLeftUp();
	virtual ERRORCODE onMouseLeftDblClick();
	virtual ERRORCODE onMouseRightDown();
	virtual ERRORCODE onMouseRightUp();

	virtual ERRORCODE displayDebugInfo();


	virtual ERRORCODE initAfterLoad();
	static void afterLoadScene(void *scene, void *data);

	bool _smartItemCursor;

	CBArray<char *, char *> _speechDirs;
	ERRORCODE addSpeechDir(const char *dir);
	ERRORCODE removeSpeechDir(const char *dir);
	char *findSpeechFile(char *StringID);

	ERRORCODE deleteItem(CAdItem *Item);
	char *_itemsFile;
	bool _tempDisableSaveState;
	virtual ERRORCODE resetContent();
	ERRORCODE addItem(CAdItem *item);
	CAdItem *getItemByName(const char *name);
	CBArray<CAdItem *, CAdItem *> _items;
	CAdObject *_inventoryOwner;
	bool isItemTaken(char *itemName);
	ERRORCODE registerInventory(CAdInventory *inv);
	ERRORCODE unregisterInventory(CAdInventory *inv);

	CAdObject *_invObject;
	CBArray<CAdInventory *, CAdInventory *> _inventories;
	virtual ERRORCODE displayContent(bool update = true, bool displayAll = false);
	char *_debugStartupScene;
	char *_startupScene;
	bool _initialScene;
	bool gameResponseUsed(int ID);
	ERRORCODE addGameResponse(int ID);
	ERRORCODE resetResponse(int ID);

	bool branchResponseUsed(int ID);
	ERRORCODE addBranchResponse(int ID);
	ERRORCODE clearBranchResponses(char *name);
	ERRORCODE startDlgBranch(const char *branchName, const char *scriptName, const char *eventName);
	ERRORCODE endDlgBranch(const char *branchName, const char *scriptName, const char *eventName);
	virtual ERRORCODE windowLoadHook(CUIWindow *win, char **buf, char **params);
	virtual ERRORCODE windowScriptMethodHook(CUIWindow *win, CScScript *script, CScStack *stack, const char *name);

	CAdSceneState *getSceneState(const char *filename, bool saving);
	CBViewport *_sceneViewport;
	int _texItemLifeTime;
	int _texWalkLifeTime;
	int _texStandLifeTime;
	int _texTalkLifeTime;

	TTalkSkipButton _talkSkipButton;

	virtual ERRORCODE getVersion(byte *verMajor, byte *verMinor, byte *extMajor, byte *extMinor);
	ERRORCODE scheduleChangeScene(const char *filename, bool fadeIn);
	char *_scheduledScene;
	bool _scheduledFadeIn;
	void setPrevSceneName(const char *name);
	void setPrevSceneFilename(const char *name);
	char *_prevSceneName;
	char *_prevSceneFilename;
	virtual ERRORCODE loadGame(const char *filename);
	CAdItem *_selectedItem;
	ERRORCODE cleanup();
	DECLARE_PERSISTENT(CAdGame, CBGame)

	void finishSentences();
	ERRORCODE showCursor();
	TGameStateEx _stateEx;
	CAdResponseBox *_responseBox;
	CAdInventoryBox *_inventoryBox;
	ERRORCODE displaySentences(bool frozen);
	void addSentence(CAdSentence *sentence);
	ERRORCODE changeScene(const char *filename, bool fadeIn);
	ERRORCODE removeObject(CAdObject *object);
	ERRORCODE addObject(CAdObject *object);
	CAdScene *_scene;
	ERRORCODE initLoop();
	CAdGame();
	virtual ~CAdGame();
	CBArray<CAdObject *, CAdObject *> _objects;
	CBArray<CAdSentence *, CAdSentence *> _sentences;

	CBArray<CAdSceneState *, CAdSceneState *> _sceneStates;
	CBArray<char *, char *> _dlgPendingBranches;

	CBArray<CAdResponseContext *, CAdResponseContext *> _responsesBranch;
	CBArray<CAdResponseContext *, CAdResponseContext *> _responsesGame;

	virtual ERRORCODE loadFile(const char *filename);
	virtual ERRORCODE loadBuffer(byte *buffer, bool complete = true);

	ERRORCODE loadItemsFile(const char *filename, bool merge = false);
	ERRORCODE loadItemsBuffer(byte *buffer, bool merge = false);


	virtual ERRORCODE ExternalCall(CScScript *script, CScStack *stack, CScStack *thisStack, char *name);

	// scripting interface
	virtual CScValue *scGetProperty(const char *name);
	virtual ERRORCODE scSetProperty(const char *name, CScValue *value);
	virtual ERRORCODE scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name);
	bool validMouse();
};

} // end of namespace WinterMute

#endif
