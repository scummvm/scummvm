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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
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

#include "engines/wintermute/ad/ad_actor.h"
#ifdef ENABLE_WME3D
#include "engines/wintermute/ad/ad_actor_3dx.h"
#endif
#include "engines/wintermute/ad/ad_game.h"
#include "engines/wintermute/ad/ad_entity.h"
#include "engines/wintermute/ad/ad_inventory.h"
#include "engines/wintermute/ad/ad_inventory_box.h"
#include "engines/wintermute/ad/ad_item.h"
#include "engines/wintermute/ad/ad_response.h"
#include "engines/wintermute/ad/ad_response_box.h"
#include "engines/wintermute/ad/ad_response_context.h"
#include "engines/wintermute/ad/ad_scene.h"
#include "engines/wintermute/ad/ad_scene_state.h"
#include "engines/wintermute/ad/ad_sentence.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/font/base_font.h"
#include "engines/wintermute/base/base_object.h"
#include "engines/wintermute/base/base_parser.h"
#include "engines/wintermute/base/base_region.h"
#include "engines/wintermute/base/sound/base_sound.h"
#include "engines/wintermute/base/base_surface_storage.h"
#include "engines/wintermute/base/base_transition_manager.h"
#include "engines/wintermute/base/base_sprite.h"
#include "engines/wintermute/base/base_viewport.h"
#include "engines/wintermute/base/particles/part_emitter.h"
#include "engines/wintermute/base/saveload.h"
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/base/scriptables/script_engine.h"
#include "engines/wintermute/base/scriptables/script.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/ui/ui_entity.h"
#include "engines/wintermute/ui/ui_window.h"
#include "engines/wintermute/utils/utils.h"
#include "engines/wintermute/video/video_player.h"
#include "engines/wintermute/video/video_theora_player.h"
#include "engines/wintermute/platform_osystem.h"
#include "common/config-manager.h"
#include "common/str.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(AdGame, true)

//////////////////////////////////////////////////////////////////////////
AdGame::AdGame(const Common::String &gameId) : BaseGame(gameId) {
	_responseBox = nullptr;
	_inventoryBox = nullptr;

	_scene = new AdScene(_gameRef);
	_scene->setName("");
	registerObject(_scene);

	_prevSceneName = nullptr;
	_prevSceneFilename = nullptr;
	_scheduledScene = nullptr;
	_scheduledFadeIn = false;


	_stateEx = GAME_NORMAL;

	_selectedItem = nullptr;


	_texItemLifeTime = 10000;
	_texWalkLifeTime = 10000;
	_texStandLifeTime = 10000;
	_texTalkLifeTime = 10000;

	_talkSkipButton = TALK_SKIP_LEFT;

	_sceneViewport = nullptr;

	_initialScene = true;
	_debugStartupScene = nullptr;
	_startupScene = nullptr;

	_invObject = new AdObject(this);
	_inventoryOwner = _invObject;

	_tempDisableSaveState = false;
	_itemsFile = nullptr;

	_smartItemCursor = false;

	addSpeechDir("speech");
}


//////////////////////////////////////////////////////////////////////////
AdGame::~AdGame() {
	cleanup();
}


//////////////////////////////////////////////////////////////////////////
bool AdGame::cleanup() {
	for (uint32 i = 0; i < _objects.size(); i++) {
		unregisterObject(_objects[i]);
		_objects[i] = nullptr;
	}
	_objects.clear();


	for (uint32 i = 0; i < _dlgPendingBranches.size(); i++) {
		delete[] _dlgPendingBranches[i];
	}
	_dlgPendingBranches.clear();

	for (uint32 i = 0; i < _speechDirs.size(); i++) {
		delete[] _speechDirs[i];
	}
	_speechDirs.clear();


	unregisterObject(_scene);
	_scene = nullptr;

	// remove items
	for (uint32 i = 0; i < _items.size(); i++) {
		_gameRef->unregisterObject(_items[i]);
	}
	_items.clear();


	// clear remaining inventories
	delete _invObject;
	_invObject = nullptr;

	for (uint32 i = 0; i < _inventories.size(); i++) {
		delete _inventories[i];
	}
	_inventories.clear();


	if (_responseBox) {
		_gameRef->unregisterObject(_responseBox);
		_responseBox = nullptr;
	}

	if (_inventoryBox) {
		_gameRef->unregisterObject(_inventoryBox);
		_inventoryBox = nullptr;
	}

	delete[] _prevSceneName;
	delete[] _prevSceneFilename;
	delete[] _scheduledScene;
	delete[] _debugStartupScene;
	delete[] _itemsFile;
	_prevSceneName = nullptr;
	_prevSceneFilename = nullptr;
	_scheduledScene = nullptr;
	_debugStartupScene = nullptr;
	_startupScene = nullptr;
	_itemsFile = nullptr;

	delete _sceneViewport;
	_sceneViewport = nullptr;

	for (uint32 i = 0; i < _sceneStates.size(); i++) {
		delete _sceneStates[i];
	}
	_sceneStates.clear();

	for (uint32 i = 0; i < _responsesBranch.size(); i++) {
		delete _responsesBranch[i];
	}
	_responsesBranch.clear();

	for (uint32 i = 0; i < _responsesGame.size(); i++) {
		delete _responsesGame[i];
	}
	_responsesGame.clear();

	return BaseGame::cleanup();
}


//////////////////////////////////////////////////////////////////////////
bool AdGame::initLoop() {
	if (_scheduledScene && _transMgr->isReady()) {
		changeScene(_scheduledScene, _scheduledFadeIn);
		delete[] _scheduledScene;
		_scheduledScene = nullptr;

		_gameRef->_activeObject = nullptr;
	}


	bool res;
	res = BaseGame::initLoop();
	if (DID_FAIL(res)) {
		return res;
	}

	if (_scene) {
		res = _scene->initLoop();
	}

	_sentences.clear();

	return res;
}


//////////////////////////////////////////////////////////////////////////
bool AdGame::addObject(AdObject *object) {
	_objects.add(object);
	return registerObject(object);
}


//////////////////////////////////////////////////////////////////////////
bool AdGame::removeObject(AdObject *object) {
	// in case the user called Scene.CreateXXX() and Game.DeleteXXX()
	if (_scene) {
		bool res = _scene->removeObject(object);
		if (DID_SUCCEED(res)) {
			return res;
		}
	}

	for (uint32 i = 0; i < _objects.size(); i++) {
		if (_objects[i] == object) {
			_objects.remove_at(i);
			break;
		}
	}
	return unregisterObject(object);
}


//////////////////////////////////////////////////////////////////////////
bool AdGame::changeScene(const char *filename, bool fadeIn) {
	if (_scene == nullptr) {
		_scene = new AdScene(_gameRef);
		registerObject(_scene);
	} else {
		_scene->applyEvent("SceneShutdown", true);

		setPrevSceneName(_scene->getName());
		setPrevSceneFilename(_scene->getFilename());

		if (!_tempDisableSaveState) {
			_scene->saveState();
		}
		_tempDisableSaveState = false;
	}

	if (_scene) {
		// reset objects
		for (uint32 i = 0; i < _objects.size(); i++) {
			_objects[i]->reset();
		}

		// reset scene properties
		_scene->_sFXVolume = 100;
		if (_scene->_scProp) {
			_scene->_scProp->cleanup();
		}

		bool ret;
		if (_initialScene && _debugDebugMode && _debugStartupScene) {
			_initialScene = false;
			ret = _scene->loadFile(_debugStartupScene);
		} else {
			ret = _scene->loadFile(filename);
		}

		if (DID_SUCCEED(ret)) {
			// invalidate references to the original scene
			for (uint32 i = 0; i < _objects.size(); i++) {
				_objects[i]->invalidateCurrRegions();
				_objects[i]->_stickRegion = nullptr;
			}

			_scene->loadState();
		}
		if (fadeIn) {
			_gameRef->_transMgr->start(TRANSITION_FADE_IN);
		}
		return ret;
	} else {
		return STATUS_FAILED;
	}
}


//////////////////////////////////////////////////////////////////////////
void AdGame::addSentence(AdSentence *sentence) {
	_sentences.add(sentence);
}


//////////////////////////////////////////////////////////////////////////
bool AdGame::displaySentences(bool frozen) {
	for (uint32 i = 0; i < _sentences.size(); i++) {
		if (frozen && _sentences[i]->_freezable) {
			continue;
		} else {
			_sentences[i]->display();
		}
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
void AdGame::finishSentences() {
	for (uint32 i = 0; i < _sentences.size(); i++) {
		if (_sentences[i]->canSkip()) {
			_sentences[i]->_duration = 0;
			if (_sentences[i]->_sound) {
				_sentences[i]->_sound->stop();
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
bool AdGame::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// ChangeScene
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "ChangeScene") == 0) {
		stack->correctParams(3);
		const char *filename = stack->pop()->getString();
		ScValue *valFadeOut = stack->pop();
		ScValue *valFadeIn = stack->pop();

		bool transOut = valFadeOut->isNULL() ? true : valFadeOut->getBool();
		bool transIn  = valFadeIn->isNULL() ? true : valFadeIn->getBool();

		scheduleChangeScene(filename, transIn);
		if (transOut) {
			_transMgr->start(TRANSITION_FADE_OUT, true);
		}
		stack->pushNULL();


		//bool ret = ChangeScene(stack->pop()->getString());
		//if (DID_FAIL(ret)) stack->pushBool(false);
		//else stack->pushBool(true);

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LoadActor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LoadActor") == 0) {
		stack->correctParams(1);
		AdActor *act = new AdActor(_gameRef);
		if (act && DID_SUCCEED(act->loadFile(stack->pop()->getString()))) {
			addObject(act);
			stack->pushNative(act, true);
		} else {
			delete act;
			act = nullptr;
			stack->pushNULL();
		}
		return STATUS_OK;
	}

#ifdef ENABLE_WME3D
	//////////////////////////////////////////////////////////////////////////
	// LoadActor3D
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LoadActor3D") == 0) {
		stack->correctParams(1);
		// assume that we have an .X model here
		// wme3d has also support for .ms3d files
		// but they are deprecated
		AdActor3DX *act = new AdActor3DX(_gameRef);
		if (act && DID_SUCCEED(act->loadFile(stack->pop()->getString()))) {
			addObject(act);
			stack->pushNative(act, true);
		} else {
			delete act;
			act = nullptr;
			stack->pushNULL();
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// UnloadActor3D
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "UnloadActor3D") == 0) {
		// this does the same as UnloadActor etc. ..
		// even WmeLite has this script call in AdScene
		stack->correctParams(1);
		ScValue *val = stack->pop();
		AdObject *obj = static_cast<AdObject *>(val->getNative());

		removeObject(obj);
		if (val->getType() == VAL_VARIABLE_REF) {
			val->setNULL();
		}

		stack->pushNULL();
		return STATUS_OK;
	}
#endif

	//////////////////////////////////////////////////////////////////////////
	// LoadEntity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LoadEntity") == 0) {
		stack->correctParams(1);
		AdEntity *ent = new AdEntity(_gameRef);
		if (ent && DID_SUCCEED(ent->loadFile(stack->pop()->getString()))) {
			addObject(ent);
			stack->pushNative(ent, true);
		} else {
			delete ent;
			ent = nullptr;
			stack->pushNULL();
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// UnloadObject / UnloadActor / UnloadEntity / DeleteEntity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "UnloadObject") == 0 || strcmp(name, "UnloadActor") == 0 || strcmp(name, "UnloadEntity") == 0 || strcmp(name, "DeleteEntity") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();
		AdObject *obj = (AdObject *)val->getNative();

		// HACK: We take corrosion screenshot before entering main menu
		// Unused screenshots must be deleted, after main menu is closed
		if (obj && BaseEngine::instance().getGameId() == "corrosion") {
			const char *mm = "interface\\system\\mainmenu.window";
			const char *fn = obj->getFilename();
			if (fn && strcmp(fn, mm) == 0) {
				deleteSaveThumbnail();
			}
		}

		removeObject(obj);
		if (val->getType() == VAL_VARIABLE_REF) {
			val->setNULL();
		}

		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CreateEntity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CreateEntity") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();

		AdEntity *ent = new AdEntity(_gameRef);
		addObject(ent);
		if (!val->isNULL()) {
			ent->setName(val->getString());
		}
		stack->pushNative(ent, true);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CreateItem
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CreateItem") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();

		AdItem *item = new AdItem(_gameRef);
		addItem(item);
		if (!val->isNULL()) {
			item->setName(val->getString());
		}
		stack->pushNative(item, true);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeleteItem
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DeleteItem") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();

		AdItem *item = nullptr;
		if (val->isNative()) {
			item = (AdItem *)val->getNative();
		} else {
			item = getItemByName(val->getString());
		}

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
		ScValue *val = stack->pop();

		AdItem *item = nullptr;
		if (val->isInt()) {
			int index = val->getInt();
			if (index >= 0 && index < (int32)_items.size()) {
				item = _items[index];
			}
		} else {
			item = getItemByName(val->getString());
		}

		if (item) {
			stack->pushNative(item, true);
		} else {
			stack->pushNULL();
		}

		return STATUS_OK;
	}


	//////////////////////////////////////////////////////////////////////////
	// AddResponse/AddResponseOnce/AddResponseOnceGame
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AddResponse") == 0 || strcmp(name, "AddResponseOnce") == 0 || strcmp(name, "AddResponseOnceGame") == 0) {
		stack->correctParams(6);
		int id = stack->pop()->getInt();
		const char *text = stack->pop()->getString();
		ScValue *val1 = stack->pop();
		ScValue *val2 = stack->pop();
		ScValue *val3 = stack->pop();
		ScValue *val4 = stack->pop();

		if (_responseBox) {
			AdResponse *res = new AdResponse(_gameRef);
			if (res) {
				res->setID(id);

				char *expandedText = new char[strlen(text) + 1];
				Common::strlcpy(expandedText, text, strlen(text) + 1);
				expandStringByStringTable(&expandedText);
				res->setText(expandedText);
				delete[] expandedText;

				if (!val1->isNULL()) {
					res->setIcon(val1->getString());
				}
				if (!val2->isNULL()) {
					res->setIconHover(val2->getString());
				}
				if (!val3->isNULL()) {
					res->setIconPressed(val3->getString());
				}
				if (!val4->isNULL()) {
					res->setFont(val4->getString());
				}

				if (strcmp(name, "AddResponseOnce") == 0) {
					res->_responseType = RESPONSE_ONCE;
				} else if (strcmp(name, "AddResponseOnceGame") == 0) {
					res->_responseType = RESPONSE_ONCE_GAME;
				}

				_responseBox->addResponse(res);
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
		int id = stack->pop()->getInt(-1);
		resetResponse(id);
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

			if (_responseBox->getNumResponses() == 0) {
				stack->pushNULL();
				return STATUS_OK;
			}


			if (_responseBox->getNumResponses() == 1 && autoSelectLast) {
				stack->pushInt(_responseBox->getIdForResponseNum(0));
				_responseBox->handleResponseNum(0);
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
			stack->pushInt(_responseBox->getNumResponses());
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
		ScValue *val = stack->pop();
		Common::String branchName;
		if (val->isNULL()) {
			branchName.format("line%d", script->_currentLine);
		} else {
			branchName = val->getString();
		}

		startDlgBranch(branchName.c_str(), script->_filename == nullptr ? "" : script->_filename, script->_threadEvent == nullptr ? "" : script->_threadEvent);
		stack->pushNULL();

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// EndDlgBranch
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "EndDlgBranch") == 0) {
		stack->correctParams(1);

		const char *branchName = nullptr;
		ScValue *val = stack->pop();
		if (!val->isNULL()) {
			branchName = val->getString();
		}
		endDlgBranch(branchName, script->_filename == nullptr ? "" : script->_filename, script->_threadEvent == nullptr ? "" : script->_threadEvent);

		stack->pushNULL();

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetCurrentDlgBranch
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetCurrentDlgBranch") == 0) {
		stack->correctParams(0);

		if (_dlgPendingBranches.size() > 0) {
			stack->pushString(_dlgPendingBranches[_dlgPendingBranches.size() - 1]);
		} else {
			stack->pushNULL();
		}

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

		ScValue *val = stack->pop();
		if (!val->isNULL()) {
			for (uint32 i = 0; i < _inventories.size(); i++) {
				AdInventory *inv = _inventories[i];

				for (uint32 j = 0; j < inv->_takenItems.size(); j++) {
					if (val->getNative() == inv->_takenItems[j]) {
						stack->pushBool(true);
						return STATUS_OK;
					} else if (scumm_stricmp(val->getString(), inv->_takenItems[j]->getName()) == 0) {
						stack->pushBool(true);
						return STATUS_OK;
					}
				}
			}
		} else {
			script->runtimeError("Game.IsItemTaken: item name expected");
		}

		stack->pushBool(false);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetInventoryWindow
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetInventoryWindow") == 0) {
		stack->correctParams(0);
		if (_inventoryBox && _inventoryBox->_window) {
			stack->pushNative(_inventoryBox->_window, true);
		} else {
			stack->pushNULL();
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetResponsesWindow
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetResponsesWindow") == 0 || strcmp(name, "GetResponseWindow") == 0) {
		stack->correctParams(0);
		if (_responseBox && _responseBox->getResponseWindow()) {
			stack->pushNative(_responseBox->getResponseWindow(), true);
		} else {
			stack->pushNULL();
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LoadResponseBox
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LoadResponseBox") == 0) {
		stack->correctParams(1);
		const char *filename = stack->pop()->getString();

		_gameRef->unregisterObject(_responseBox);
		_responseBox = new AdResponseBox(_gameRef);
		if (_responseBox && !DID_FAIL(_responseBox->loadFile(filename))) {
			registerObject(_responseBox);
			stack->pushBool(true);
		} else {
			delete _responseBox;
			_responseBox = nullptr;
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

		_gameRef->unregisterObject(_inventoryBox);
		_inventoryBox = new AdInventoryBox(_gameRef);
		if (_inventoryBox && !DID_FAIL(_inventoryBox->loadFile(filename))) {
			registerObject(_inventoryBox);
			stack->pushBool(true);
		} else {
			delete _inventoryBox;
			_inventoryBox = nullptr;
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

		bool ret = loadItemsFile(filename, merge);
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

		if (width <= 0) {
			width = _renderer->getWidth();
		}
		if (height <= 0) {
			height = _renderer->getHeight();
		}

		if (!_sceneViewport) {
			_sceneViewport = new BaseViewport(_gameRef);
		}
		if (_sceneViewport) {
			_sceneViewport->setRect(x, y, x + width, y + height);
		}

		stack->pushBool(true);

		return STATUS_OK;
	}

#ifdef ENABLE_FOXTAIL
	//////////////////////////////////////////////////////////////////////////
	// [FoxTail] SetInventoryBoxHideSelected
	// Used while changing cursor type at some included script
	// Return value is never used
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetInventoryBoxHideSelected") == 0) {
		stack->correctParams(1);
		_inventoryBox->_hideSelected = stack->pop()->getBool(false);
		stack->pushNULL();
		return STATUS_OK;
	}
#endif

	else {
		return BaseGame::scCallMethod(script, stack, thisStack, name);
	}
}


//////////////////////////////////////////////////////////////////////////
ScValue *AdGame::scGetProperty(const Common::String &name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (name == "Type") {
		_scValue->setString("game");
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Scene
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Scene") {
		if (_scene) {
			_scValue->setNative(_scene, true);
		} else {
			_scValue->setNULL();
		}

		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// SelectedItem
	//////////////////////////////////////////////////////////////////////////
	else if (name == "SelectedItem") {
		//if (_selectedItem) _scValue->setString(_selectedItem->_name);
		if (_selectedItem) {
			_scValue->setNative(_selectedItem, true);
		} else {
			_scValue->setNULL();
		}

		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// NumItems
	//////////////////////////////////////////////////////////////////////////
	else if (name == "NumItems") {
		return _invObject->scGetProperty(name);
	}

	//////////////////////////////////////////////////////////////////////////
	// SmartItemCursor
	//////////////////////////////////////////////////////////////////////////
	else if (name == "SmartItemCursor") {
		_scValue->setBool(_smartItemCursor);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// InventoryVisible
	//////////////////////////////////////////////////////////////////////////
	else if (name == "InventoryVisible") {
		_scValue->setBool(_inventoryBox && _inventoryBox->_visible);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// InventoryScrollOffset
	//////////////////////////////////////////////////////////////////////////
	else if (name == "InventoryScrollOffset") {
		if (_inventoryBox) {
			_scValue->setInt(_inventoryBox->_scrollOffset);
		} else {
			_scValue->setInt(0);
		}

		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ResponsesVisible (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "ResponsesVisible") {
		_scValue->setBool(_stateEx == GAME_WAITING_RESPONSE);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// PrevScene / PreviousScene (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "PrevScene" || name == "PreviousScene") {
		if (!_prevSceneName) {
			_scValue->setString("");
		} else {
			_scValue->setString(_prevSceneName);
		}
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// PrevSceneFilename / PreviousSceneFilename (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "PrevSceneFilename" || name == "PreviousSceneFilename") {
		if (!_prevSceneFilename) {
			_scValue->setString("");
		} else {
			_scValue->setString(_prevSceneFilename);
		}
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// LastResponse (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "LastResponse") {
		if (!_responseBox || !_responseBox->getLastResponseText()) {
			_scValue->setString("");
		} else {
			_scValue->setString(_responseBox->getLastResponseText());
		}
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// LastResponseOrig (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "LastResponseOrig") {
		if (!_responseBox || !_responseBox->getLastResponseTextOrig()) {
			_scValue->setString("");
		} else {
			_scValue->setString(_responseBox->getLastResponseTextOrig());
		}
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// InventoryObject
	//////////////////////////////////////////////////////////////////////////
	else if (name == "InventoryObject") {
		if (_inventoryOwner == _invObject) {
			_scValue->setNative(this, true);
		} else {
			_scValue->setNative(_inventoryOwner, true);
		}

		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// TotalNumItems
	//////////////////////////////////////////////////////////////////////////
	else if (name == "TotalNumItems") {
		_scValue->setInt(_items.size());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// TalkSkipButton
	//////////////////////////////////////////////////////////////////////////
	else if (name == "TalkSkipButton") {
		_scValue->setInt(_talkSkipButton);
		return _scValue;
	}

#ifdef ENABLE_WME3D
	//////////////////////////////////////////////////////////////////////////
	// VideoSkipButton
	//////////////////////////////////////////////////////////////////////////
	else if (name == "VideoSkipButton") {
		warning("AdGame::scGetProperty VideoSkipButton not implemented");
		_scValue->setInt(0);
		return _scValue;
	}
#endif

	//////////////////////////////////////////////////////////////////////////
	// ChangingScene
	//////////////////////////////////////////////////////////////////////////
	else if (name == "ChangingScene") {
		_scValue->setBool(_scheduledScene != nullptr);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// StartupScene
	//////////////////////////////////////////////////////////////////////////
	else if (name == "StartupScene") {
		if (!_startupScene) {
			_scValue->setNULL();
		} else {
			_scValue->setString(_startupScene);
		}
		return _scValue;
	}

	else {
		return BaseGame::scGetProperty(name);
	}
}


//////////////////////////////////////////////////////////////////////////
bool AdGame::scSetProperty(const char *name, ScValue *value) {

	//////////////////////////////////////////////////////////////////////////
	// SelectedItem
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SelectedItem") == 0) {
		if (value->isNULL()) {
			_selectedItem = nullptr;
		} else {
			if (value->isNative()) {
				_selectedItem = nullptr;
				for (uint32 i = 0; i < _items.size(); i++) {
					if (_items[i] == value->getNative()) {
						_selectedItem = (AdItem *)value->getNative();
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
		if (_inventoryBox) {
			_inventoryBox->_visible = value->getBool();
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// InventoryObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "InventoryObject") == 0) {
		if (_inventoryOwner && _inventoryBox) {
			_inventoryOwner->getInventory()->_scrollOffset = _inventoryBox->_scrollOffset;
		}

		if (value->isNULL()) {
			_inventoryOwner = _invObject;
		} else {
			BaseObject *obj = (BaseObject *)value->getNative();
			if (obj == this) {
				_inventoryOwner = _invObject;
			} else if (_gameRef->validObject(obj)) {
				_inventoryOwner = (AdObject *)obj;
			}
		}

		if (_inventoryOwner && _inventoryBox) {
			_inventoryBox->_scrollOffset = _inventoryOwner->getInventory()->_scrollOffset;
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// InventoryScrollOffset
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "InventoryScrollOffset") == 0) {
		if (_inventoryBox) {
			_inventoryBox->_scrollOffset = value->getInt();
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// TalkSkipButton
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "TalkSkipButton") == 0) {
		int val = value->getInt();
		if (val < 0) {
			val = 0;
		}
		if (val > TALK_SKIP_NONE) {
			val = TALK_SKIP_NONE;
		}
		_talkSkipButton = (TTalkSkipButton)val;
		return STATUS_OK;
	}

#ifdef ENABLE_WME3D
	//////////////////////////////////////////////////////////////////////////
	// VideoSkipButton
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "VideoSkipButton") == 0) {
		warning("AdGame::scSetProperty VideoSkipButton not implemented");
		return STATUS_OK;
	}
#endif

	//////////////////////////////////////////////////////////////////////////
	// StartupScene
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "StartupScene") == 0) {
		if (value == nullptr) {
			delete[] _startupScene;
			_startupScene = nullptr;
		} else {
			BaseUtils::setString(&_startupScene, value->getString());
		}

		return STATUS_OK;
	}

	else {
		return BaseGame::scSetProperty(name, value);
	}
}


//////////////////////////////////////////////////////////////////////////
bool AdGame::externalCall(ScScript *script, ScStack *stack, ScStack *thisStack, char *name) {
	ScValue *thisObj;

	//////////////////////////////////////////////////////////////////////////
	// Actor
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Actor") == 0) {
		stack->correctParams(0);
		thisObj = thisStack->getTop();

		thisObj->setNative(new AdActor(_gameRef));
		stack->pushNULL();
	}

	//////////////////////////////////////////////////////////////////////////
	// Entity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Entity") == 0) {
		stack->correctParams(0);
		thisObj = thisStack->getTop();

		thisObj->setNative(new AdEntity(_gameRef));
		stack->pushNULL();
	}


	//////////////////////////////////////////////////////////////////////////
	// call parent
	else {
		return BaseGame::externalCall(script, stack, thisStack, name);
	}


	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdGame::showCursor() {
	if (_cursorHidden) {
		return STATUS_OK;
	}

	if (_selectedItem && _gameRef->_state == GAME_RUNNING && _stateEx == GAME_NORMAL && _interactive) {
		if (_selectedItem->_cursorCombined) {
			BaseSprite *origLastCursor = _lastCursor;
			BaseGame::showCursor();
			_lastCursor = origLastCursor;
		}
		if (_activeObject && _selectedItem->_cursorHover && _activeObject->getExtendedFlag("usable")) {
			if (!_smartItemCursor || _activeObject->canHandleEvent(_selectedItem->getName())) {
				return drawCursor(_selectedItem->_cursorHover);
			} else {
				return drawCursor(_selectedItem->_cursorNormal);
			}
		} else {
			return drawCursor(_selectedItem->_cursorNormal);
		}
	} else {
		return BaseGame::showCursor();
	}
}


//////////////////////////////////////////////////////////////////////////
bool AdGame::loadFile(const char *filename) {
	char *buffer = (char *)BaseFileManager::getEngineInstance()->readWholeFile(filename);
	if (buffer == nullptr) {
		_gameRef->LOG(0, "AdGame::LoadFile failed for file '%s'", filename);
		return STATUS_FAILED;
	}

	bool ret;

	setFilename(filename);

	if (DID_FAIL(ret = loadBuffer(buffer, true))) {
		_gameRef->LOG(0, "Error parsing GAME file '%s'", filename);
	}


	delete[] buffer;

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
bool AdGame::loadBuffer(char *buffer, bool complete) {
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

	char *params;
	char *params2;
	int cmd = 1;
	BaseParser parser;

	bool itemFound = false, itemsFound = false;

	while (cmd > 0 && (cmd = parser.getCommand(&buffer, commands, &params)) > 0) {
		switch (cmd) {
		case TOKEN_GAME:
			if (DID_FAIL(BaseGame::loadBuffer(params, false))) {
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_AD_GAME:
			while (cmd > 0 && (cmd = parser.getCommand(&params, commands, &params2)) > 0) {
				switch (cmd) {
				case TOKEN_RESPONSE_BOX:
					delete _responseBox;
					_responseBox = new AdResponseBox(_gameRef);
					if (_responseBox && !DID_FAIL(_responseBox->loadFile(params2))) {
						registerObject(_responseBox);
					} else {
						delete _responseBox;
						_responseBox = nullptr;
						cmd = PARSERR_GENERIC;
					}
					break;

				case TOKEN_INVENTORY_BOX:
					delete _inventoryBox;
					_inventoryBox = new AdInventoryBox(_gameRef);
					if (_inventoryBox && !DID_FAIL(_inventoryBox->loadFile(params2))) {
						registerObject(_inventoryBox);
					} else {
						delete _inventoryBox;
						_inventoryBox = nullptr;
						cmd = PARSERR_GENERIC;
					}
					break;

				case TOKEN_ITEMS:
					itemsFound = true;
					BaseUtils::setString(&_itemsFile, params2);
					if (DID_FAIL(loadItemsFile(_itemsFile))) {
						delete[] _itemsFile;
						_itemsFile = nullptr;
						cmd = PARSERR_GENERIC;
					}
					break;

				case TOKEN_TALK_SKIP_BUTTON:
					if (scumm_stricmp(params2, "right") == 0) {
						_talkSkipButton = TALK_SKIP_RIGHT;
					} else if (scumm_stricmp(params2, "both") == 0) {
						_talkSkipButton = TALK_SKIP_BOTH;
					} else {
						_talkSkipButton = TALK_SKIP_LEFT;
					}
					break;

				case TOKEN_SCENE_VIEWPORT: {
					Rect32 rc;
					parser.scanStr(params2, "%d,%d,%d,%d", &rc.left, &rc.top, &rc.right, &rc.bottom);
					if (!_sceneViewport) {
						_sceneViewport = new BaseViewport(_gameRef);
					}
					if (_sceneViewport) {
						_sceneViewport->setRect(rc.left, rc.top, rc.right, rc.bottom);
					}
				}
				break;

				case TOKEN_EDITOR_PROPERTY:
					parseEditorProperty(params2, false);
					break;

				case TOKEN_STARTUP_SCENE:
					BaseUtils::setString(&_startupScene, params2);
					break;

				case TOKEN_DEBUG_STARTUP_SCENE:
					BaseUtils::setString(&_debugStartupScene, params2);
					break;

				default:
					break;
				}
			}
			break;

		default:
			break;
		}
	}

	if (cmd == PARSERR_TOKENNOTFOUND) {
		_gameRef->LOG(0, "Syntax error in GAME definition");
		return STATUS_FAILED;
	}
	if (cmd == PARSERR_GENERIC) {
		_gameRef->LOG(0, "Error loading GAME definition");
		return STATUS_FAILED;
	}

	if (itemFound && !itemsFound) {
		_gameRef->LOG(0, "**Warning** Please put the items definition to a separate file.");
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdGame::persist(BasePersistenceManager *persistMgr) {
	if (!persistMgr->getIsSaving()) {
		cleanup();
	}
	BaseGame::persist(persistMgr);

	_dlgPendingBranches.persist(persistMgr);

	_inventories.persist(persistMgr);
	persistMgr->transferPtr(TMEMBER_PTR(_inventoryBox));

	_objects.persist(persistMgr);

	persistMgr->transferCharPtr(TMEMBER(_prevSceneName));
	persistMgr->transferCharPtr(TMEMBER(_prevSceneFilename));

	persistMgr->transferPtr(TMEMBER_PTR(_responseBox));
	_responsesBranch.persist(persistMgr);
	_responsesGame.persist(persistMgr);
	persistMgr->transferPtr(TMEMBER_PTR(_scene));
	_sceneStates.persist(persistMgr);
	persistMgr->transferBool(TMEMBER(_scheduledFadeIn));
	persistMgr->transferCharPtr(TMEMBER(_scheduledScene));
	persistMgr->transferPtr(TMEMBER_PTR(_selectedItem));
	persistMgr->transferSint32(TMEMBER_INT(_talkSkipButton));

	_sentences.persist(persistMgr);

	persistMgr->transferPtr(TMEMBER_PTR(_sceneViewport));
	persistMgr->transferSint32(TMEMBER_INT(_stateEx));
	persistMgr->transferBool(TMEMBER(_initialScene));
	persistMgr->transferCharPtr(TMEMBER(_debugStartupScene));

	persistMgr->transferPtr(TMEMBER_PTR(_invObject));
	persistMgr->transferPtr(TMEMBER_PTR(_inventoryOwner));
	persistMgr->transferBool(TMEMBER(_tempDisableSaveState));
	_items.persist(persistMgr);

	persistMgr->transferCharPtr(TMEMBER(_itemsFile));

	_speechDirs.persist(persistMgr);
	persistMgr->transferBool(TMEMBER(_smartItemCursor));

	if (!persistMgr->getIsSaving()) {
		_initialScene = false;
	}

	persistMgr->transferCharPtr(TMEMBER(_startupScene));


	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
void AdGame::setPrevSceneName(const char *name) {
	delete[] _prevSceneName;
	_prevSceneName = nullptr;
	if (name) {
		_prevSceneName = new char[strlen(name) + 1];
		if (_prevSceneName) {
			strcpy(_prevSceneName, name);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
void AdGame::setPrevSceneFilename(const char *name) {
	delete[] _prevSceneFilename;
	_prevSceneFilename = nullptr;
	if (name) {
		_prevSceneFilename = new char[strlen(name) + 1];
		if (_prevSceneFilename) {
			strcpy(_prevSceneFilename, name);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
bool AdGame::scheduleChangeScene(const char *filename, bool fadeIn) {
	delete[] _scheduledScene;
	_scheduledScene = nullptr;

	if (_scene && !_scene->_initialized) {
		return changeScene(filename, fadeIn);
	} else {
		_scheduledScene = new char [strlen(filename) + 1];
		strcpy(_scheduledScene, filename);

		_scheduledFadeIn = fadeIn;

		return STATUS_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
bool AdGame::handleCustomActionStart(BaseGameCustomAction action) {
	bool isCorrosion = BaseEngine::instance().getGameId() == "corrosion";
	
	if (isCorrosion) {
		// Corrosion Enhanced Edition contain city map screen, which is
		// mouse controlled and conflicts with those custom actions
		const char *m = "items\\street_map\\windows\\street_map_window.script";
		if (_scEngine->isRunningScript(m)) {
			return false;
		}
	}

	int xLeft   = 30;
	int xCenter = _renderer->getWidth() / 2;
	int xRight  = _renderer->getWidth() - 30;

	int yTop    = 35;
	int yCenter = _renderer->getHeight() / 2;
	int yBottom = _renderer->getHeight() - 35;
	if (isCorrosion && !(ConfMan.get("extra").contains("Enhanced"))) {
		// original version of Corrosion has a toolbar under the game screen
		yBottom -= 60;
	}

	BaseArray<AdObject *> objects;

	Point32 p;
	int distance = xCenter * xCenter + yCenter * yCenter;

	switch (action) {
	case kClickAtCenter:
		p.x = xCenter;
		p.y = yCenter;
		break;
	case kClickAtLeft:
		p.x = xLeft;
		p.y = yCenter;
		break;
	case kClickAtRight:
		p.x = xRight;
		p.y = yCenter;
		break;
	case kClickAtTop:
		p.x = xCenter;
		p.y = yTop;
		break;
	case kClickAtBottom:
		p.x = xCenter;
		p.y = yBottom;
		break;
	case kClickAtEntityNearestToCenter:
		p.x = xCenter;
		p.y = yCenter;
		// Looking through all objects for entities near to the center
		if (_scene && _scene->getSceneObjects(objects, true)) {
			for (uint32 i = 0; i < objects.size(); i++) {
				BaseRegion *region;
				if (objects[i]->getType() != OBJECT_ENTITY ||
					!objects[i]->_active || 
					!objects[i]->_registrable ||
					(!(region = ((AdEntity *)objects[i])->_region))
				) {
					continue;
				}

				// Something exactly at center? Great!
				if (region->pointInRegion(xCenter, yCenter)) {
					distance = 0;
					p.x = xCenter;
					p.y = yCenter;
					break;
				}

				// Something at the edge? Available with other actions. 
				if (region->pointInRegion(xLeft, yCenter) ||
					region->pointInRegion(xRight, yCenter) ||
					region->pointInRegion(xCenter, yBottom) ||
					region->pointInRegion(xCenter, yTop)
				) {
					continue;
				}

				// Keep entities that has less distance to center
				int x = ((AdEntity *)objects[i])->_posX;
				int y = ((AdEntity *)objects[i])->_posY - ((AdEntity *)objects[i])->getHeight() / 2;
				int d = (x - xCenter) * (x - xCenter) + (y - yCenter) * (y - yCenter);
				if (distance > d) {
					distance = d;
					p.x = x;
					p.y = y;
				}
			}
		}
		break;
	default:
		return false;
	}

	BasePlatform::setCursorPos(p.x, p.y);
	setActiveObject(_gameRef->_renderer->getObjectAt(p.x, p.y)); 
	onMouseLeftDown();
	onMouseLeftUp();
	return true;
}


//////////////////////////////////////////////////////////////////////////
bool AdGame::handleCustomActionEnd(BaseGameCustomAction action) {
	return false;
}


//////////////////////////////////////////////////////////////////////////
bool AdGame::getVersion(byte *verMajor, byte *verMinor, byte *extMajor, byte *extMinor) const {
	BaseGame::getVersion(verMajor, verMinor, nullptr, nullptr);

	if (extMajor) {
		*extMajor = 0;
	}
	if (extMinor) {
		*extMinor = 0;
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdGame::loadItemsFile(const char *filename, bool merge) {
	char *buffer = (char *)BaseFileManager::getEngineInstance()->readWholeFile(filename);
	if (buffer == nullptr) {
		_gameRef->LOG(0, "AdGame::LoadItemsFile failed for file '%s'", filename);
		return STATUS_FAILED;
	}

	bool ret;

	//_filename = new char [strlen(filename)+1];
	//strcpy(_filename, filename);

	if (DID_FAIL(ret = loadItemsBuffer(buffer, merge))) {
		_gameRef->LOG(0, "Error parsing ITEMS file '%s'", filename);
	}


	delete[] buffer;

	return ret;
}


//////////////////////////////////////////////////////////////////////////
bool AdGame::loadItemsBuffer(char *buffer, bool merge) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(ITEM)
	TOKEN_TABLE_END

	char *params;
	int cmd;
	BaseParser parser;

	if (!merge) {
		while (_items.size() > 0) {
			deleteItem(_items[0]);
		}
	}

	while ((cmd = parser.getCommand(&buffer, commands, &params)) > 0) {
		switch (cmd) {
		case TOKEN_ITEM: {
			AdItem *item = new AdItem(_gameRef);
			if (item && !DID_FAIL(item->loadBuffer(params, false))) {
				// delete item with the same name, if exists
				if (merge) {
					AdItem *prevItem = getItemByName(item->getName());
					if (prevItem) {
						deleteItem(prevItem);
					}
				}
				addItem(item);
			} else {
				delete item;
				item = nullptr;
				cmd = PARSERR_GENERIC;
			}
		}
		break;

		default:
			break;
		}
	}

	if (cmd == PARSERR_TOKENNOTFOUND) {
		_gameRef->LOG(0, "Syntax error in ITEMS definition");
		return STATUS_FAILED;
	}
	if (cmd == PARSERR_GENERIC) {
		_gameRef->LOG(0, "Error loading ITEMS definition");
		return STATUS_FAILED;
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
AdSceneState *AdGame::getSceneState(const char *filename, bool saving) {
	char *filenameCor = new char[strlen(filename) + 1];
	strcpy(filenameCor, filename);
	for (uint32 i = 0; i < strlen(filenameCor); i++) {
		if (filenameCor[i] == '/') {
			filenameCor[i] = '\\';
		}
	}

	for (uint32 i = 0; i < _sceneStates.size(); i++) {
		if (scumm_stricmp(_sceneStates[i]->getFilename(), filenameCor) == 0) {
			delete[] filenameCor;
			return _sceneStates[i];
		}
	}

	if (saving) {
		AdSceneState *ret = new AdSceneState(_gameRef);
		ret->setFilename(filenameCor);

		_sceneStates.add(ret);

		delete[] filenameCor;
		return ret;
	} else {
		delete[] filenameCor;
		return nullptr;
	}
}

#ifdef ENABLE_WME3D
//////////////////////////////////////////////////////////////////////////
uint32 Wintermute::AdGame::getAmbientLightColor() {
	if (_scene) {
		return _scene->_ambientLightColor;
	} else {
		return BaseGame::getAmbientLightColor();
	}
}

Wintermute::TShadowType Wintermute::AdGame::getMaxShadowType(Wintermute::BaseObject *object) {
	TShadowType ret = BaseGame::getMaxShadowType(object);

	return MIN(ret, _scene->_maxShadowType);
}

bool Wintermute::AdGame::getFogParams(FogParameters &fogParameters) {
	if (_scene) {
		fogParameters = _scene->_fogParameters;
		return true;
	} else {
		return BaseGame::getFogParams(fogParameters);
	}
}
#endif


//////////////////////////////////////////////////////////////////////////
bool AdGame::windowLoadHook(UIWindow *win, char **buffer, char **params) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(ENTITY_CONTAINER)
	TOKEN_TABLE_END

	int cmd = PARSERR_GENERIC;
	BaseParser parser;

	cmd = parser.getCommand(buffer, commands, params);
	switch (cmd) {
	case TOKEN_ENTITY_CONTAINER: {
		UIEntity *ent = new UIEntity(_gameRef);
		if (!ent || DID_FAIL(ent->loadBuffer(*params, false))) {
			delete ent;
			ent = nullptr;
			cmd = PARSERR_GENERIC;
		} else {
			ent->_parent = win;
			win->_widgets.add(ent);
		}
	}
	break;

	default:
		break;
	}

	if (cmd == PARSERR_TOKENNOTFOUND || cmd == PARSERR_GENERIC) {
		return STATUS_FAILED;
	}

	return STATUS_OK;

}


//////////////////////////////////////////////////////////////////////////
bool AdGame::windowScriptMethodHook(UIWindow *win, ScScript *script, ScStack *stack, const char *name) {
	if (strcmp(name, "CreateEntityContainer") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();

		UIEntity *ent = new UIEntity(_gameRef);
		if (!val->isNULL()) {
			ent->setName(val->getString());
		}
		stack->pushNative(ent, true);

		ent->_parent = win;
		win->_widgets.add(ent);

		return STATUS_OK;
	} else {
		return STATUS_FAILED;
	}
}


//////////////////////////////////////////////////////////////////////////
bool AdGame::startDlgBranch(const char *branchName, const char *scriptName, const char *eventName) {
	char *name = new char[strlen(branchName) + 1 + strlen(scriptName) + 1 + strlen(eventName) + 1];
	if (name) {
		sprintf(name, "%s.%s.%s", branchName, scriptName, eventName);
		_dlgPendingBranches.add(name);
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdGame::endDlgBranch(const char *branchName, const char *scriptName, const char *eventName) {
	char *name = nullptr;
	bool deleteName = false;
	if (branchName == nullptr && _dlgPendingBranches.size() > 0) {
		name = _dlgPendingBranches[_dlgPendingBranches.size() - 1];
	} else {
		if (branchName != nullptr) {
			name = new char[strlen(branchName) + 1 + strlen(scriptName) + 1 + strlen(eventName) + 1];
			if (name) {
				sprintf(name, "%s.%s.%s", branchName, scriptName, eventName);
				deleteName = true;
			}
		}
	}

	if (name == nullptr) {
		return STATUS_OK;
	}


	int startIndex = -1;
	for (int i = _dlgPendingBranches.size() - 1; i >= 0; i--) {
		if (scumm_stricmp(name, _dlgPendingBranches[i]) == 0) {
			startIndex = i;
			break;
		}
	}
	if (startIndex >= 0) {
		for (uint32 i = startIndex; i < _dlgPendingBranches.size(); i++) {
			//ClearBranchResponses(_dlgPendingBranches[i]);
			delete[] _dlgPendingBranches[i];
			_dlgPendingBranches[i] = nullptr;
		}
		_dlgPendingBranches.remove_at(startIndex, _dlgPendingBranches.size() - startIndex);
	}

	// dialogue is over, forget selected responses
	if (_dlgPendingBranches.size() == 0) {
		for (uint32 i = 0; i < _responsesBranch.size(); i++) {
			delete _responsesBranch[i];
		}
		_responsesBranch.clear();
	}

	if (deleteName) {
		delete[] name;
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdGame::clearBranchResponses(char *name) {
	for (uint32 i = 0; i < _responsesBranch.size(); i++) {
		if (scumm_stricmp(name, _responsesBranch[i]->getContext()) == 0) {
			delete _responsesBranch[i];
			_responsesBranch.remove_at(i);
			i--;
		}
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdGame::addBranchResponse(int id) {
	if (branchResponseUsed(id)) {
		return STATUS_OK;
	}
	AdResponseContext *r = new AdResponseContext(_gameRef);
	r->_id = id;
	r->setContext(_dlgPendingBranches.size() > 0 ? _dlgPendingBranches[_dlgPendingBranches.size() - 1] : nullptr);
	_responsesBranch.add(r);
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdGame::branchResponseUsed(int id) const {
	char *context = _dlgPendingBranches.size() > 0 ? _dlgPendingBranches[_dlgPendingBranches.size() - 1] : nullptr;
	for (uint32 i = 0; i < _responsesBranch.size(); i++) {
		if (_responsesBranch[i]->_id == id) {
			if ((context == nullptr && _responsesBranch[i]->getContext() == nullptr) || scumm_stricmp(context, _responsesBranch[i]->getContext()) == 0) {
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
bool AdGame::addGameResponse(int id) {
	if (gameResponseUsed(id)) {
		return STATUS_OK;
	}
	AdResponseContext *r = new AdResponseContext(_gameRef);
	r->_id = id;
	r->setContext(_dlgPendingBranches.size() > 0 ? _dlgPendingBranches[_dlgPendingBranches.size() - 1] : nullptr);
	_responsesGame.add(r);
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdGame::gameResponseUsed(int id) const {
	char *context = _dlgPendingBranches.size() > 0 ? _dlgPendingBranches[_dlgPendingBranches.size() - 1] : nullptr;
	for (uint32 i = 0; i < _responsesGame.size(); i++) {
		const AdResponseContext *respContext = _responsesGame[i];
		if (respContext->_id == id) {
			if ((context == nullptr && respContext->getContext() == nullptr) || ((context != nullptr && respContext->getContext() != nullptr) && scumm_stricmp(context, respContext->getContext()) == 0)) {
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
bool AdGame::resetResponse(int id) {
	char *context = _dlgPendingBranches.size() > 0 ? _dlgPendingBranches[_dlgPendingBranches.size() - 1] : nullptr;

	for (uint32 i = 0; i < _responsesGame.size(); i++) {
		if (_responsesGame[i]->_id == id) {
			if ((context == nullptr && _responsesGame[i]->getContext() == nullptr) || scumm_stricmp(context, _responsesGame[i]->getContext()) == 0) {
				delete _responsesGame[i];
				_responsesGame.remove_at(i);
				break;
			}
		}
	}

	for (uint32 i = 0; i < _responsesBranch.size(); i++) {
		if (_responsesBranch[i]->_id == id) {
			if ((context == nullptr && _responsesBranch[i]->getContext() == nullptr) || scumm_stricmp(context, _responsesBranch[i]->getContext()) == 0) {
				delete _responsesBranch[i];
				_responsesBranch.remove_at(i);
				break;
			}
		}
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdGame::displayContent(bool doUpdate, bool displayAll) {
	// init
	if (doUpdate) {
		initLoop();
	}

	// fill black
	_renderer->fill(0, 0, 0);
	if (!_editorMode) {
		_renderer->setScreenViewport();
	}

	// playing exclusive video?
	if (_videoPlayer->isPlaying()) {
		if (doUpdate) {
			_videoPlayer->update();
		}
		_videoPlayer->display();
	} else if (_theoraPlayer) {
		if (_theoraPlayer->isPlaying()) {
			if (doUpdate) {
				_theoraPlayer->update();
			}
			_theoraPlayer->display();
		}
		if (_theoraPlayer->isFinished()) {
			delete _theoraPlayer;
			_theoraPlayer = nullptr;
		}
	} else {

		// process scripts
		if (doUpdate) {
			_scEngine->tick();
		}

		Point32 p;
		getMousePos(&p);

		_scene->update();
		_scene->display();


		// display in-game windows
		displayWindows(true);
		if (_inventoryBox) {
			_inventoryBox->display();
		}
		if (_stateEx == GAME_WAITING_RESPONSE) {
			_responseBox->display();
		}
		_renderer->displayIndicator();


		if (doUpdate || displayAll) {
			// display normal windows
			displayWindows(false);

			setActiveObject(_gameRef->_renderer->getObjectAt(p.x, p.y));

			// textual info
			displaySentences(_state == GAME_FROZEN);

			showCursor();

			if (_fader) {
				_fader->display();
			}
			_transMgr->update();
		}

	}
	if (_loadingIcon) {
		_loadingIcon->display(_loadingIconX, _loadingIconY);
		if (!_loadingIconPersistent) {
			delete _loadingIcon;
			_loadingIcon = nullptr;
		}
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool AdGame::registerInventory(AdInventory *inv) {
	for (uint32 i = 0; i < _inventories.size(); i++) {
		if (_inventories[i] == inv) {
			return STATUS_OK;
		}
	}
	registerObject(inv);
	_inventories.add(inv);

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool AdGame::unregisterInventory(AdInventory *inv) {
	for (uint32 i = 0; i < _inventories.size(); i++) {
		if (_inventories[i] == inv) {
			unregisterObject(_inventories[i]);
			_inventories.remove_at(i);
			return STATUS_OK;
		}
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool AdGame::isItemTaken(char *itemName) {
	for (uint32 i = 0; i < _inventories.size(); i++) {
		AdInventory *inv = _inventories[i];

		for (uint32 j = 0; j < inv->_takenItems.size(); j++) {
			if (scumm_stricmp(itemName, inv->_takenItems[j]->getName()) == 0) {
				return true;
			}
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
AdItem *AdGame::getItemByName(const char *name) const {
	for (uint32 i = 0; i < _items.size(); i++) {
		if (scumm_stricmp(_items[i]->getName(), name) == 0) {
			return _items[i];
		}
	}
	return nullptr;
}


//////////////////////////////////////////////////////////////////////////
bool AdGame::addItem(AdItem *item) {
	_items.add(item);
	return _gameRef->registerObject(item);
}


//////////////////////////////////////////////////////////////////////////
bool AdGame::resetContent() {
	// clear pending dialogs
	for (uint32 i = 0; i < _dlgPendingBranches.size(); i++) {
		delete[] _dlgPendingBranches[i];
	}
	_dlgPendingBranches.clear();


	// clear inventories
	for (uint32 i = 0; i < _inventories.size(); i++) {
		_inventories[i]->_takenItems.clear();
	}

	// clear scene states
	for (uint32 i = 0; i < _sceneStates.size(); i++) {
		delete _sceneStates[i];
	}
	_sceneStates.clear();

	// clear once responses
	for (uint32 i = 0; i < _responsesBranch.size(); i++) {
		delete _responsesBranch[i];
	}
	_responsesBranch.clear();

	// clear once game responses
	for (uint32 i = 0; i < _responsesGame.size(); i++) {
		delete _responsesGame[i];
	}
	_responsesGame.clear();

	// reload inventory items
	if (_itemsFile) {
		loadItemsFile(_itemsFile);
	}

	_tempDisableSaveState = true;

	return BaseGame::resetContent();
}


//////////////////////////////////////////////////////////////////////////
bool AdGame::deleteItem(AdItem *item) {
	if (!item) {
		return STATUS_FAILED;
	}

	if (_selectedItem == item) {
		_selectedItem = nullptr;
	}
	_scene->handleItemAssociations(item->getName(), false);

	// remove from all inventories
	for (uint32 i = 0; i < _inventories.size(); i++) {
		_inventories[i]->removeItem(item);
	}

	// remove object
	for (uint32 i = 0; i < _items.size(); i++) {
		if (_items[i] == item) {
			unregisterObject(_items[i]);
			_items.remove_at(i);
			break;
		}
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdGame::addSpeechDir(const char *dir) {
	if (!dir || dir[0] == '\0') {
		return STATUS_FAILED;
	}

	char *temp = new char[strlen(dir) + 2];
	strcpy(temp, dir);
	if (temp[strlen(temp) - 1] != '\\' && temp[strlen(temp) - 1] != '/') {
		strcat(temp, "\\");
	}

	for (uint32 i = 0; i < _speechDirs.size(); i++) {
		if (scumm_stricmp(_speechDirs[i], temp) == 0) {
			delete[] temp;
			return STATUS_OK;
		}
	}
	_speechDirs.add(temp);

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdGame::removeSpeechDir(const char *dir) {
	if (!dir || dir[0] == '\0') {
		return STATUS_FAILED;
	}

	char *temp = new char[strlen(dir) + 2];
	strcpy(temp, dir);
	if (temp[strlen(temp) - 1] != '\\' && temp[strlen(temp) - 1] != '/') {
		strcat(temp, "\\");
	}

	bool found = false;
	for (uint32 i = 0; i < _speechDirs.size(); i++) {
		if (scumm_stricmp(_speechDirs[i], temp) == 0) {
			delete[] _speechDirs[i];
			_speechDirs.remove_at(i);
			found = true;
			break;
		}
	}
	delete[] temp;

	return found;
}


//////////////////////////////////////////////////////////////////////////
char *AdGame::findSpeechFile(char *stringID) {
	char *ret = new char[MAX_PATH_LENGTH];

	for (uint32 i = 0; i < _speechDirs.size(); i++) {
		sprintf(ret, "%s%s.ogg", _speechDirs[i], stringID);
		if (BaseFileManager::getEngineInstance()->hasFile(ret)) {
			return ret;
		}

		sprintf(ret, "%s%s.wav", _speechDirs[i], stringID);
		if (BaseFileManager::getEngineInstance()->hasFile(ret)) {
			return ret;
		}
	}
	delete[] ret;
	return nullptr;
}


//////////////////////////////////////////////////////////////////////////
bool AdGame::validMouse() {
	Point32 pos;
	BasePlatform::getCursorPos(&pos);

	return _renderer->pointInViewport(&pos);
}

//////////////////////////////////////////////////////////////////////////
bool AdGame::onMouseLeftDown() {
	if (!validMouse()) {
		return STATUS_OK;
	}
	if (_state == GAME_RUNNING && !_interactive) {
		if (_talkSkipButton == TALK_SKIP_LEFT || _talkSkipButton == TALK_SKIP_BOTH) {
			finishSentences();
		}
		return STATUS_OK;
	}

	if (_activeObject) {
		_activeObject->handleMouse(MOUSE_CLICK, MOUSE_BUTTON_LEFT);
	}

	bool handled = _state == GAME_RUNNING && DID_SUCCEED(applyEvent("LeftClick"));
	if (!handled) {
		if (_activeObject != nullptr) {
			_activeObject->applyEvent("LeftClick");
		} else if (_state == GAME_RUNNING && _scene && _scene->pointInViewport(_mousePos.x, _mousePos.y)) {
			_scene->applyEvent("LeftClick");
		}
	}

	if (_activeObject != nullptr) {
		_gameRef->_capturedObject = _gameRef->_activeObject;
	}
	_mouseLeftDown = true;

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool AdGame::onMouseLeftUp() {
	if (_activeObject) {
		_activeObject->handleMouse(MOUSE_RELEASE, MOUSE_BUTTON_LEFT);
	}

	_capturedObject = nullptr;
	_mouseLeftDown = false;

	bool handled;
	if (BaseEngine::instance().getTargetExecutable() < WME_LITE) {
		handled = _state==GAME_RUNNING && DID_SUCCEED(applyEvent("LeftRelease"));
	} else {
		handled = DID_SUCCEED(applyEvent("LeftRelease"));
	}
	
	if (!handled) {
		if (_activeObject != nullptr) {
			_activeObject->applyEvent("LeftRelease");
		} else if (_state == GAME_RUNNING && _scene && _scene->pointInViewport(_mousePos.x, _mousePos.y)) {
			_scene->applyEvent("LeftRelease");
		}
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool AdGame::onMouseLeftDblClick() {
	if (!validMouse()) {
		return STATUS_OK;
	}

	if (_state == GAME_RUNNING && !_interactive) {
		return STATUS_OK;
	}

	if (_activeObject) {
		_activeObject->handleMouse(MOUSE_DBLCLICK, MOUSE_BUTTON_LEFT);
	}

	bool handled = _state == GAME_RUNNING && DID_SUCCEED(applyEvent("LeftDoubleClick"));
	if (!handled) {
		if (_activeObject != nullptr) {
			_activeObject->applyEvent("LeftDoubleClick");
		} else if (_state == GAME_RUNNING && _scene && _scene->pointInViewport(_mousePos.x, _mousePos.y)) {
			_scene->applyEvent("LeftDoubleClick");
		}
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool AdGame::onMouseRightDown() {
	if (!validMouse()) {
		return STATUS_OK;
	}
	if (_state == GAME_RUNNING && !_interactive) {
		if (_talkSkipButton == TALK_SKIP_RIGHT || _talkSkipButton == TALK_SKIP_BOTH) {
			finishSentences();
		}
		return STATUS_OK;
	}

	if ((_state == GAME_RUNNING && !_interactive) || _stateEx == GAME_WAITING_RESPONSE) {
		return STATUS_OK;
	}

	if (_activeObject) {
		_activeObject->handleMouse(MOUSE_CLICK, MOUSE_BUTTON_RIGHT);
	}

	bool handled = _state == GAME_RUNNING && DID_SUCCEED(applyEvent("RightClick"));
	if (!handled) {
		if (_activeObject != nullptr) {
			_activeObject->applyEvent("RightClick");
		} else if (_state == GAME_RUNNING && _scene && _scene->pointInViewport(_mousePos.x, _mousePos.y)) {
			_scene->applyEvent("RightClick");
		}
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool AdGame::onMouseRightUp() {
	if (_activeObject) {
		_activeObject->handleMouse(MOUSE_RELEASE, MOUSE_BUTTON_RIGHT);
	}

	bool handled = _state == GAME_RUNNING && DID_SUCCEED(applyEvent("RightRelease"));
	if (!handled) {
		if (_activeObject != nullptr) {
			_activeObject->applyEvent("RightRelease");
		} else if (_state == GAME_RUNNING && _scene && _scene->pointInViewport(_mousePos.x, _mousePos.y)) {
			_scene->applyEvent("RightRelease");
		}
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool AdGame::displayDebugInfo() {
	char str[100];
	if (_gameRef->_debugDebugMode) {
		sprintf(str, "Mouse: %d, %d (scene: %d, %d)", _mousePos.x, _mousePos.y, _mousePos.x + (_scene ? _scene->getOffsetLeft() : 0), _mousePos.y + (_scene ? _scene->getOffsetTop() : 0));
		_systemFont->drawText((byte *)str, 0, 90, _renderer->getWidth(), TAL_RIGHT);

		sprintf(str, "Scene: %s (prev: %s)", (_scene && _scene->getName()) ? _scene->getName() : "???", _prevSceneName ? _prevSceneName : "???");
		_systemFont->drawText((byte *)str, 0, 110, _renderer->getWidth(), TAL_RIGHT);
	}
	return BaseGame::displayDebugInfo();
}


//////////////////////////////////////////////////////////////////////////
bool AdGame::onScriptShutdown(ScScript *script) {
	if (_responseBox && _responseBox->_waitingScript == script) {
		_responseBox->_waitingScript = nullptr;
	}

	return STATUS_OK;
}

Common::String AdGame::debuggerToString() const {
	return Common::String::format("%p: Game \"%s\"", (const void *)this, getName());
}
} // End of namespace Wintermute
