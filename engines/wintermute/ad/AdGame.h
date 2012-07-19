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

#include "engines/wintermute/ad/AdTypes.h"
#include "engines/wintermute/base/BGame.h"

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
	virtual bool onScriptShutdown(CScScript *script);

	virtual bool onMouseLeftDown();
	virtual bool onMouseLeftUp();
	virtual bool onMouseLeftDblClick();
	virtual bool onMouseRightDown();
	virtual bool onMouseRightUp();

	virtual bool displayDebugInfo();


	virtual bool initAfterLoad();
	static void afterLoadScene(void *scene, void *data);

	bool _smartItemCursor;

	CBArray<char *, char *> _speechDirs;
	bool addSpeechDir(const char *dir);
	bool removeSpeechDir(const char *dir);
	char *findSpeechFile(char *StringID);

	bool deleteItem(CAdItem *Item);
	char *_itemsFile;
	bool _tempDisableSaveState;
	virtual bool resetContent();
	bool addItem(CAdItem *item);
	CAdItem *getItemByName(const char *name);
	CBArray<CAdItem *, CAdItem *> _items;
	CAdObject *_inventoryOwner;
	bool isItemTaken(char *itemName);
	bool registerInventory(CAdInventory *inv);
	bool unregisterInventory(CAdInventory *inv);

	CAdObject *_invObject;
	CBArray<CAdInventory *, CAdInventory *> _inventories;
	virtual bool displayContent(bool update = true, bool displayAll = false);
	char *_debugStartupScene;
	char *_startupScene;
	bool _initialScene;
	bool gameResponseUsed(int ID);
	bool addGameResponse(int ID);
	bool resetResponse(int ID);

	bool branchResponseUsed(int ID);
	bool addBranchResponse(int ID);
	bool clearBranchResponses(char *name);
	bool startDlgBranch(const char *branchName, const char *scriptName, const char *eventName);
	bool endDlgBranch(const char *branchName, const char *scriptName, const char *eventName);
	virtual bool windowLoadHook(CUIWindow *win, char **buf, char **params);
	virtual bool windowScriptMethodHook(CUIWindow *win, CScScript *script, CScStack *stack, const char *name);

	CAdSceneState *getSceneState(const char *filename, bool saving);
	CBViewport *_sceneViewport;
	int _texItemLifeTime;
	int _texWalkLifeTime;
	int _texStandLifeTime;
	int _texTalkLifeTime;

	TTalkSkipButton _talkSkipButton;

	virtual bool getVersion(byte *verMajor, byte *verMinor, byte *extMajor, byte *extMinor);
	bool scheduleChangeScene(const char *filename, bool fadeIn);
	char *_scheduledScene;
	bool _scheduledFadeIn;
	void setPrevSceneName(const char *name);
	void setPrevSceneFilename(const char *name);
	char *_prevSceneName;
	char *_prevSceneFilename;
	virtual bool loadGame(const char *filename);
	CAdItem *_selectedItem;
	bool cleanup();
	DECLARE_PERSISTENT(CAdGame, CBGame)

	void finishSentences();
	bool showCursor();
	TGameStateEx _stateEx;
	CAdResponseBox *_responseBox;
	CAdInventoryBox *_inventoryBox;
	bool displaySentences(bool frozen);
	void addSentence(CAdSentence *sentence);
	bool changeScene(const char *filename, bool fadeIn);
	bool removeObject(CAdObject *object);
	bool addObject(CAdObject *object);
	CAdScene *_scene;
	bool initLoop();
	CAdGame();
	virtual ~CAdGame();
	CBArray<CAdObject *, CAdObject *> _objects;
	CBArray<CAdSentence *, CAdSentence *> _sentences;

	CBArray<CAdSceneState *, CAdSceneState *> _sceneStates;
	CBArray<char *, char *> _dlgPendingBranches;

	CBArray<CAdResponseContext *, CAdResponseContext *> _responsesBranch;
	CBArray<CAdResponseContext *, CAdResponseContext *> _responsesGame;

	virtual bool loadFile(const char *filename);
	virtual bool loadBuffer(byte *buffer, bool complete = true);

	bool loadItemsFile(const char *filename, bool merge = false);
	bool loadItemsBuffer(byte *buffer, bool merge = false);


	virtual bool ExternalCall(CScScript *script, CScStack *stack, CScStack *thisStack, char *name);

	// scripting interface
	virtual CScValue *scGetProperty(const char *name);
	virtual bool scSetProperty(const char *name, CScValue *value);
	virtual bool scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name);
	bool validMouse();
};

} // end of namespace WinterMute

#endif
