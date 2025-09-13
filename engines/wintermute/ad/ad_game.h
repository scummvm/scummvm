/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */
#ifndef WINTERMUTE_ADGAME_H
#define WINTERMUTE_ADGAME_H

#include "engines/wintermute/ad/ad_types.h"
#include "engines/wintermute/base/base_game.h"

namespace Wintermute {
class AdItem;
class AdInventory;
class AdSceneState;
class AdScene;
class AdItem;
class AdObject;
class AdSentence;
class AdInventoryBox;
class AdResponseContext;
class AdResponseBox;
class AdGame : public BaseGame {
public:
	bool onScriptShutdown(ScScript *script) override;

	bool onMouseLeftDown() override;
	bool onMouseLeftUp() override;
	bool onMouseLeftDblClick() override;
	bool onMouseRightDown() override;
	bool onMouseRightUp() override;

	bool displayDebugInfo() override;

	//virtual CBObject *GetNextAccessObject(CBObject *CurrObject);
	//virtual CBObject *GetPrevAccessObject(CBObject *CurrObject);

	virtual bool getLayerSize(int *layerWidth, int *layerHeight, Common::Rect32 *viewport, bool *customViewport) override;
#ifdef ENABLE_WME3D
	uint32 getAmbientLightColor() override;
	bool getFogParams(bool *fogEnabled, uint32 *fogColor, float *start, float *end) override;

	virtual bool renderShadowGeometry() override;
	TShadowType getMaxShadowType(BaseObject *object = nullptr) override;
#endif

	bool _smartItemCursor;

	BaseArray<char *> _speechDirs;
	bool addSpeechDir(const char *dir);
	bool removeSpeechDir(const char *dir);
	char *findSpeechFile(char *StringID);

	bool deleteItem(AdItem *Item);
	char *_itemsFile;
	bool _tempDisableSaveState;
	bool resetContent() override;
	bool addItem(AdItem *item);
	AdItem *getItemByName(const char *name) const;
	BaseArray<AdItem *> _items;
	AdObject *_inventoryOwner;
	bool isItemTaken(char *itemName);
	bool registerInventory(AdInventory *inv);
	bool unregisterInventory(AdInventory *inv);

	AdObject *_invObject;
	BaseArray<AdInventory *> _inventories;
	bool displayContent(bool update = true, bool displayAll = false) override;
	char *_debugStartupScene;
	char *_startupScene;
	bool _initialScene;
	bool gameResponseUsed(int id) const;
	bool addGameResponse(int id);
	bool resetResponse(int id);

	bool branchResponseUsed(int id) const;
	bool addBranchResponse(int id);
	bool clearBranchResponses(char *name);
	bool startDlgBranch(const char *branchName, const char *scriptName, const char *eventName);
	bool endDlgBranch(const char *branchName, const char *scriptName, const char *eventName);
	bool windowLoadHook(UIWindow *win, char **buf, char **params) override;
	bool windowScriptMethodHook(UIWindow *win, ScScript *script, ScStack *stack, const char *name) override;

	AdSceneState *getSceneState(const char *filename, bool saving);
	BaseViewport *_sceneViewport;
	int32 _texItemLifeTime;
	int32 _texWalkLifeTime;
	int32 _texStandLifeTime;
	int32 _texTalkLifeTime;

	TTalkSkipButton _talkSkipButton;
	TVideoSkipButton _videoSkipButton;

	bool getVersion(byte *verMajor, byte *verMinor, byte *extMajor, byte *extMinor) override;
	bool scheduleChangeScene(const char *filename, bool fadeIn);
	char *_scheduledScene;
	bool _scheduledFadeIn;
	void setPrevSceneName(const char *name);
	void setPrevSceneFilename(const char *name);
	char *_prevSceneName;
	char *_prevSceneFilename;
	AdItem *_selectedItem;
	bool cleanup() override;
	DECLARE_PERSISTENT(AdGame, BaseGame)

	void finishSentences();
	bool showCursor() override;
	TGameStateEx _stateEx;
	AdResponseBox *_responseBox;
	AdInventoryBox *_inventoryBox;
	bool displaySentences(bool frozen);
	void addSentence(AdSentence *sentence);
	bool changeScene(const char *filename, bool fadeIn);
	bool removeObject(AdObject *object);
	bool addObject(AdObject *object);
	AdScene *_scene;
	bool initLoop();
	AdGame(const Common::String &gameId);
	~AdGame() override;
	BaseArray<AdObject *> _objects;
	BaseArray<AdSentence *> _sentences;

	BaseArray<AdSceneState *> _sceneStates;
	BaseArray<char *> _dlgPendingBranches;

	BaseArray<AdResponseContext *> _responsesBranch;
	BaseArray<AdResponseContext *> _responsesGame;

	bool loadFile(const char *filename) override;
	bool loadBuffer(char *buffer, bool complete = true) override;

	bool loadItemsFile(const char *filename, bool merge = false);
	bool loadItemsBuffer(char *buffer, bool merge = false);

	bool externalCall(ScScript *script, ScStack *stack, ScStack *thisStack, char *name) override;

	// scripting interface
	ScValue *scGetProperty(const char *name) override;
	bool scSetProperty(const char *name, ScValue *value) override;
	bool scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) override;
	bool validMouse();


	Common::String debuggerToString() const override;

	bool handleCustomActionStart(BaseGameCustomAction action) override;
	bool handleCustomActionEnd(BaseGameCustomAction action) override;
};

} // End of namespace Wintermute

#endif
