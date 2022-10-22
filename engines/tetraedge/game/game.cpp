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

#include "common/file.h"
#include "common/path.h"
#include "common/str-array.h"
#include "common/system.h"

#include "tetraedge/tetraedge.h"
#include "tetraedge/game/application.h"
#include "tetraedge/game/character.h"
#include "tetraedge/game/in_game_scene.h"
#include "tetraedge/game/game.h"
#include "tetraedge/game/game_achievements.h"
#include "tetraedge/game/lua_binds.h"
#include "tetraedge/game/object3d.h"

#include "tetraedge/te/te_camera.h"
#include "tetraedge/te/te_core.h"
#include "tetraedge/te/te_input_mgr.h"
#include "tetraedge/te/te_ray_intersection.h"
#include "tetraedge/te/te_variant.h"

namespace Tetraedge {

Game::Game() : _objectsTakenVal(0), _score(0), _entered(false), _gameLoadState(0),
_noScaleLayout(nullptr), _noScaleLayout2(nullptr), _warped(false), _saveRequested(false),
_firstInventory(true), _movePlayerCharacterDisabled(false), _enteredFlag2(false),
_luaShowOwnerError(false), _markersVisible(true), _running(false), _loadName("save.xml") {
	for (int i = 0; i < NUM_OBJECTS_TAKEN_IDS; i++) {
		_objectsTakenBits[i] = false;
	}
}

/*static*/ const char *Game::OBJECTS_TAKEN_IDS[5] = {
	"BCylindreBarr",
	"VCylindreMusique",
	"VCylindreVal",
	"CCylindreCite",
	"VPoupeeMammouth"
};

bool Game::addAnimToSet(const Common::String &anim) {
	// Get path to lua script, eg scenes/ValVoralberg/14040/Set14040.lua
	const Common::Path animPath(Common::String("scenes/") + anim + "/");

	bool retval = false;
	if (Common::File::exists(animPath)) {
		Common::StringArray parts = TetraedgeEngine::splitString(anim, '/');
		assert(parts.size() >= 2);

		Common::String layoutName = parts[1];
		Common::String path = Common::String("scenes/") + parts[0] + "/" + parts[1] + "/Set" + parts[1];

		_gui2.load(path + ".lua");
		/*
		TeILayout *layout = _gui2.layout("root");
		TeSpriteLayout *spriteLayout2 = findSpriteLayoutByName(layout, layoutName);

		TeLayout *layout2 = TeLuaGUI::layout(&(this->scene).field_0x170,"root");
		long lVar5 = 0;
		if (spriteLayout2) {
			lVar5 = (long)plVar3 + *(long *)(*plVar3 + -0x198);
		}
		(**(code **)(*(long *)((long)&pTVar2->vptr + (long)pTVar2->vptr[-0x33]) + 0x30))
					((long)&pTVar2->vptr + (long)pTVar2->vptr[-0x33],lVar5);
		 */
	  retval = true;
	}

	return retval;
}

void Game::addArtworkUnlocked(const Common::String &name, bool notify) {
	if (_unlockedArtwork.contains(name))
		return;
	_unlockedArtwork[name] = true;
	if (notify) {
		_notifier.push("BONUS!", "Inventory/Objects/VPapierCrayon.png");
	}
}

void Game::addNoScale2Child(TeLayout *layout) {
	if (_noScaleLayout2 && layout) {
		_noScaleLayout2->addChild(layout);
	}
}

void Game::addNoScale2Children() {
	if (!_noScaleLayout2)
		return;

	TeLayout *vidbtn = _gui4.layout("videoButtonLayout");
	if (vidbtn)
		_noScaleLayout2->addChild(vidbtn);

	TeLayout *bg = _inventory.cellphone()->gui().layout("background");
	if (bg)
		_noScaleLayout2->addChild(bg);

	TeButtonLayout *bgbtn = _objectif.gui1().buttonLayout("background");
	if (bgbtn)
		_noScaleLayout2->addChild(bgbtn);
}

void Game::addNoScaleChildren() {
	if (!_noScaleLayout)
		return;
	TeLayout *inGame = _gui4.layout("inGame");
	if (inGame)
		_noScaleLayout->addChild(inGame);

	_noScaleLayout->addChild(&_question2);

	Application *app = g_engine->getApplication();
	app->_frontLayout.addChild(&_dialog2);

	_noScaleLayout->addChild(&_inventory);
	_noScaleLayout->addChild(&_inventoryMenu);
	_noScaleLayout->addChild(&_documentsBrowser);
	_noScaleLayout->addChild(&_documentsBrowser.zoomedLayout());
}

void Game::addRandomSound(const Common::String &name, const Common::String &path, float f1, float f2) {
	if (!_randomSounds.contains(name)) {
		_randomSounds[name] = Common::Array<RandomSound*>();
	}
	RandomSound *randsound = new RandomSound();
	randsound->_path = path;
	randsound->_f1 = f1;
	randsound->_f2 = f2;
	randsound->_name = name;
	_randomSounds[name].push_back(randsound);
}

void Game::addToBag(const Common::String &objname) {
	if (_inventory.objectCount(objname) != 0)
		return;
	_inventory.addObject(objname);
	Common::String imgpath("Inventory/Objects/");
	imgpath += _inventory.objectName(objname);
	imgpath += ".png";
	_notifier.push(objname, imgpath);
	for (int i = 0; i < NUM_OBJECTS_TAKEN_IDS; i++) {
		if (objname == OBJECTS_TAKEN_IDS[i] && !_objectsTakenBits[i]) {
			_objectsTakenBits[i] = true;
			_objectsTakenVal++;
		}
	}
	// Seeems strange as we're already in Game, but that's what original does?
	Game *game = g_engine->getGame();
	game->_score += 10;
	debug("Updated score: %d", game->_score);
}

void Game::addToHand(const Common::String &objname) {
	_inventory.addObject(objname);
	_inventory.selectedObject(objname);
}

void Game::addToScore(int score) {
	_score += score;
}

bool Game::changeWarp(const Common::String &zone, const Common::String &scene, bool fadeFlag) {
	debug("Game::changeWarp(%s, %s, %s)", zone.c_str(), scene.c_str(), fadeFlag ? "true" : "false");
	Application *app = g_engine->getApplication();
	if (fadeFlag) {
		app->blackFade();
	} else {
		app->captureFade();
	}
	_warpZone = zone;
	_warpScene = scene;
	_warpFadeFlag = fadeFlag;
	_warped = true;
	return true;
}

bool Game::changeWarp2(const Common::String &zone, const Common::String &scene, bool fadeFlag) {
	debug("Game::changeWarp2(%s, %s, %s)", zone.c_str(), scene.c_str(), fadeFlag ? "true" : "false");
	_warped = false;
	_movePlayerCharacterDisabled = false;
	_sceneCharacterVisibleFromLoad = false;
	// TODO: set 3 other fields here (0x3f40 = -1, 0x4249 = 1, 0x424b = 0)
	Common::Path luapath("scenes");
	luapath.joinInPlace(zone);
	luapath.joinInPlace(scene);
	luapath.joinInPlace("Logic");
	luapath.appendInPlace(scene);
	luapath.appendInPlace(".lua");

	if (Common::File::exists(luapath)) {
		_luaScript.execute("OnLeave");
		_luaContext.removeGlobal("On");
		_luaContext.removeGlobal("OnEnter");
		_luaContext.removeGlobal("OnWarpObjectHit");
		_luaContext.removeGlobal("OnButtonDown");
		_luaContext.removeGlobal("OnButtonUp");
		_luaContext.removeGlobal("OnFinishedAnim");
		_luaContext.removeGlobal("OnCharacterAnimationFinished");
		_luaContext.removeGlobal("OnCharacterAnimationPlayerFinished");
		_luaContext.removeGlobal("OnDisplacementFinished");
		_luaContext.removeGlobal("OnFreeSoundFinished");
		_luaContext.removeGlobal("OnDocumentClosed");
		_luaContext.removeGlobal("OnSelectedObject");
		_luaContext.removeGlobal("OnDialogFinished");
		_luaContext.removeGlobal("OnAnswered");
		_luaContext.removeGlobal("OnLeave");
		_luaScript.unload();
	}

	_gui3.unload();
	_prevSceneName = _currentScene;
	if (fadeFlag)
		g_engine->getApplication()->fade();

	return initWarp(zone, scene, false);
}

void Game::deleteNoScale() {
	if (_noScaleLayout) {
		removeNoScaleChildren();
		delete _noScaleLayout;
		_noScaleLayout = nullptr;
	}
	if (_noScaleLayout2) {
		removeNoScale2Children();
		delete _noScaleLayout2;
		_noScaleLayout2 = nullptr;
	}
}

void Game::draw() {
	if (_running) {
	  _frameCounter++;
	  _scene.draw();
	}
}

void Game::enter(bool newgame) {
	_enteredFlag2 = true;
	_entered = true;
	_luaShowOwnerError = false;
	_score = 0;
	Application *app = g_engine->getApplication();
	app->visualFade().init();
	Common::SharedPtr<TeCallback1Param<Game, const Common::Point &>> callbackptr(new TeCallback1Param<Game, const Common::Point &>(this, &Game::onMouseClick, -1000.0f));
	g_engine->getInputMgr()->_mouseLUpSignal.insert(callbackptr);
	_movePlayerCharacterDisabled = false;
	warning("TODO: Game::enter set some other fields here");
	_sceneCharacterVisibleFromLoad = false;
	Character::loadSettings("models/ModelsSettings.xml");
	Object3D::loadSettings("objects/ObjectsSettings.xml");
	if (_scene._character) {
		_scene._character->onFinished().remove(this, &Game::onDisplacementFinished);
		_scene.unloadCharacter(_scene._character->_model->name());
	}
	bool loaded = loadPlayerCharacter("Kate");
	if (!loaded)
		error("[Game::enter] Can't load player character");

	_scene._character->_model->setVisible(true);
	_running = true;
	_luaContext.create();
	GameAchievements::registerAchievements(_luaContext);

	_luaContext.setGlobal("BUTTON_VALID", 1);
	_luaContext.setGlobal("BUTTON_CANCEL", 2);
	_luaContext.setGlobal("BUTTON_EXTRA1", 4);
	_luaContext.setGlobal("BUTTON_EXTRA2", 8);
	_luaContext.setGlobal("BUTTON_L1", 0x10);
	_luaContext.setGlobal("BUTTON_R1", 0x20);
	_luaContext.setGlobal("BUTTON_START", 0x40);
	_luaContext.setGlobal("BUTTON_UP", 0x80);
	_luaContext.setGlobal("BUTTON_DOWN", 0x100);
	_luaContext.setGlobal("BUTTON_LEFT", 0x200);
	_luaContext.setGlobal("BUTTON_RIGHT", 0x400);
	_luaContext.setGlobal("BUTTON_LS_CLIC", 0x800);
	_luaContext.setGlobal("BUTTON_RS_CLIC", 0x1000);
	_luaContext.setGlobal("BUTTON_BACK", 0x2000);
	_luaContext.setGlobal("BUTTON_SELECT", 0x4000);
	_luaContext.setGlobal("BUTTON_L2", 0x8000);
	_luaContext.setGlobal("BUTTON_R2", 0x10000);
	_luaContext.setGlobal("BUTTON_LS_UP", 0x20000);
	_luaContext.setGlobal("BUTTON_LS_DOWN", 0x40000);
	_luaContext.setGlobal("BUTTON_LS_LEFT", 0x80000);
	_luaContext.setGlobal("BUTTON_LS_RIGHT", 0x100000);
	_luaContext.setGlobal("BUTTON_RS_UP", 0x200000);
	_luaContext.setGlobal("BUTTON_RS_DOWN", 0x400000);
	_luaContext.setGlobal("BUTTON_RS_LEFT", 0x800000);
	_luaContext.setGlobal("BUTTON_RS_RIGHT", 0x1000000);

	_luaScript.attachToContext(&_luaContext);

	if (!_objectif.gui1().loaded()) {
		_objectif.load();
	}
	_question2.load();
	_dialog2.load();
	_documentsBrowser.load();
	_documentsBrowser.loadZoomed();
	_inventory.load();

	_inventory.cellphone()->onCallNumber().add(this, &Game::onCallNumber);

	if (!newgame) {
		loadBackup(_loadName);
	} else {
		_gameLoadState = 1;
		onFinishedLoadingBackup("");
	}
	_sceneCharacterVisibleFromLoad = true;
	_scene._character->onFinished().remove(this, &Game::onDisplacementFinished);
	_scene._character->onFinished().add(this, &Game::onDisplacementFinished);
	_dialog2.prevSceneName().clear();
	_notifier.load();
}

/*static*/ TeI3DObject2 *Game::findLayoutByName(TeLayout *parent, const Common::String &name) {
	error("TODO: Implement Game::findLayoutByName - although maybe never used?");
}

/*static*/ TeSpriteLayout *Game::findSpriteLayoutByName(TeLayout *parent, const Common::String &name) {
	if (!parent)
		return nullptr;

	if (parent->name() == name)
		return dynamic_cast<TeSpriteLayout *>(parent);

	for (auto &child : parent->childList()) {
		TeSpriteLayout *val = findSpriteLayoutByName(dynamic_cast<TeLayout *>(child), name);
		if (val)
			return val;
	}

	return nullptr;
}

void Game::finishFreemium() {
	Application *app = g_engine->getApplication();
	app->_finishedGame = true;
	app->_finishedFremium = true;
}

void Game::finishGame() {
	Application *app = g_engine->getApplication();
	app->_finishedGame = true;
	_playedTimer.stop();
	/* Game does this but does nothing with result?
	if (app->difficulty() == 2) {
	  _playedTimer.getTimeFromStart();
	} */
	app->credits().enter(false);
}

void Game::initLoadedBackupData() {
	if (!_loadName.empty()) {
		error("TODO: Implemet Game::initLoadedBackupData loading part");
	}
	Application *app = g_engine->getApplication();
	const Common::String firstWarpPath = app->_firstWarpPath;
	_currentScene = app->_firstScene;
	_currentZone = app->_firstZone;
	_playedTimer.start();
	_objectsTakenVal = 0;
	for (int i = 0; i < ARRAYSIZE(_objectsTakenBits); i++) {
		_objectsTakenBits[i] = 0;
	}
	_dialogsTold = 0;
	if (_loadName == "NO_OWNER")
		_luaShowOwnerError = true;
	_gameLoadState = 0;
	app->showLoadingIcon(false);
	_loadName.clear();
	initScene(true, firstWarpPath);
}

void Game::initNoScale() {
	if (!_noScaleLayout) {
		_noScaleLayout = new TeLayout();
		_noScaleLayout->setName("noScaleLayout");
		_noScaleLayout->setSizeType(TeILayout::RELATIVE_TO_PARENT);
		_noScaleLayout->setSize(TeVector3f32(1.0f, 1.0f, 0.0f));
	}

	if (!_noScaleLayout2) {
		_noScaleLayout2 = new TeLayout();
		_noScaleLayout2->setName("noScaleLayout2");
		_noScaleLayout2->setSizeType(TeILayout::RELATIVE_TO_PARENT);
		_noScaleLayout2->setSize(TeVector3f32(1.0f, 1.0f, 0.0f));
	}
}

void Game::initScene(bool fade, const Common::String &scenePath) {
	_luaContext.setGlobal("SHOW_OWNER_ERROR", _luaShowOwnerError);
	initWarp(_currentZone, _currentScene, fade);
	loadScene(scenePath);
	if (_scene._character->_model.get() && !_scene.findKate()) {
		_scene.models().push_back(_scene._character->_model);
	}
	_scene._character->_model->setVisible(true);
}

bool Game::initWarp(const Common::String &zone, const Common::String &scene, bool fadeFlag) {
	debug("Game::initWarp(%s, %s, %s)", zone.c_str(), scene.c_str(), fadeFlag ? "true" : "false");
	_inventoryMenu.unload();
	_gui4.unload();
	_movePlayerCharacterDisabled = false;
	_sceneCharacterVisibleFromLoad = true;

	if (_scene._character) {
		_scene._character->_model->setVisible(false);
		_scene._character->deleteAllCallback();
		_scene._character->stop();
		_scene._character->setAnimation(_scene._character->characterSettings()._walkFileName, true, false, false, -1, 9999);
		if (!_scene.findKate()) {
			_scene.models().push_back(_scene._character->_model);
			_scene.models().push_back(_scene._character->_shadowModel[0]);
			_scene.models().push_back(_scene._character->_shadowModel[1]);
		}
	}

	_currentZone = zone;
	_currentScene = scene;

	_scene.loadBlockers();
	Common::Path scenePath("scenes");
	scenePath.joinInPlace(zone);
	scenePath.joinInPlace(scene);
	_sceneZonePath = scenePath;

	const Common::Path intLuaPath = scenePath.join(Common::String::format("Int%s.lua", scene.c_str()));
	const Common::Path logicLuaPath = scenePath.join(Common::String::format("Logic%s.lua", scene.c_str()));
	const Common::Path setLuaPath = scenePath.join(Common::String::format("Set%s.lua", scene.c_str()));
	const Common::Path forLuaPath = scenePath.join(Common::String::format("For%s.lua", scene.c_str()));
	const Common::Path markerLuaPath = scenePath.join(Common::String::format("Marker%s.lua", scene.c_str()));

	bool intLuaExists = Common::File::exists(intLuaPath);
	bool logicLuaExists = Common::File::exists(logicLuaPath);
	bool setLuaExists = Common::File::exists(setLuaPath);
	bool forLuaExists = Common::File::exists(forLuaPath);
	bool markerLuaExists = Common::File::exists(markerLuaPath);

	if (!intLuaExists && !logicLuaExists && !setLuaExists && !forLuaExists && !markerLuaExists) {
		debug("No lua scripts for scene %s zone %s", scene.c_str(), zone.c_str());
		return false;
	}

	if (!_gameSounds.empty())
		warning("TODO: Game::initWarp: stop game sounds");

	if (logicLuaExists) {
		_luaContext.addBindings(LuaBinds::LuaOpenBinds);
		_luaScript.attachToContext(&_luaContext);
		_luaScript.load("menus/help/help.lua");
		_luaScript.load(logicLuaPath);
	}

	if (_gui3.loaded())
		_gui3.unload();

	_scene.reset();
	_scene.bgGui().unload();
	_scene.markerGui().unload();
	_scene.hitObjectGui().unload();
	Common::Path geomPath(Common::String::format("scenes/%s/Geometry%s.bin",
												 zone.c_str(), zone.c_str()));
	_scene.load(geomPath);
	_scene.loadBackground(setLuaPath);

	Application *app = g_engine->getApplication();
	if (forLuaExists) {
		_gui3.load(forLuaPath);
		TeLayout *bg = _gui3.layout("background");
		bg->setRatioMode(TeILayout::RATIO_MODE_NONE);
		app->_frontLayout.addChild(bg);
		TeLayout *cellbg = _inventory.cellphone()->gui().buttonLayout("background");
		app->_frontLayout.removeChild(cellbg);
		app->_frontLayout.addChild(cellbg);
		_objectif.reattachLayout(&app->_frontLayout);
	}

	if (intLuaExists) {
		_scene.loadInteractions(intLuaPath);
		TeLuaGUI::StringMap<TeButtonLayout *> &blayouts = _scene.hitObjectGui().buttonLayouts();
		for (auto &entry : blayouts) {
			HitObject *hobj = new HitObject();
			TeButtonLayout *btn = entry._value;
			hobj->_game = this;
			hobj->_button = btn;
			hobj->_name = btn->name();
			btn->onMouseClickValidated().add(hobj, &HitObject::onValidated);
			btn->onButtonChangedToStateDownSignal().add(hobj, &HitObject::onDown);
			btn->onButtonChangedToStateUpSignal().add(hobj, &HitObject::onUp);
			_gameHitObjects.push_back(hobj);
		}
	}

	_inventoryMenu.load();
	_gui4.load("InGame.lua");

	TeButtonLayout *skipbtn = _gui4.buttonLayout("skipVideoButton");
	skipbtn->setVisible(false);
	skipbtn->onMouseClickValidated().remove(this, &Game::onSkipVideoButtonValidated);
	skipbtn->onMouseClickValidated().add(this, &Game::onSkipVideoButtonValidated);

	TeButtonLayout *vidbgbtn = _gui4.buttonLayout("videoBackgroundButton");
	vidbgbtn->setVisible(false);
	/* TODO: Restore the original behavior here (onLockVideoButtonValidated) */
	vidbgbtn->onMouseClickValidated().remove(this, &Game::onSkipVideoButtonValidated);
	vidbgbtn->onMouseClickValidated().add(this, &Game::onSkipVideoButtonValidated);

	TeSpriteLayout *video = _gui4.spriteLayout("video");
	video->setVisible(false);
	video->_tiledSurfacePtr->_frameAnim.onFinished().remove(this, &Game::onVideoFinished);
	video->_tiledSurfacePtr->_frameAnim.onFinished().add(this, &Game::onVideoFinished);

	TeButtonLayout *invbtn = _gui4.buttonLayout("inventoryButton");
	invbtn->setSizeType(TeILayout::RELATIVE_TO_PARENT); // TODO: Double-check if this is the right virt fn.
	invbtn->onMouseClickValidated().remove(this, &Game::onInventoryButtonValidated);
	invbtn->onMouseClickValidated().add(this, &Game::onInventoryButtonValidated);

	const TeVector3f32 winSize = app->getMainWindow().size();
	if (g_engine->getCore()->fileFlagSystemFlag("definition") == "SD") {
		invbtn->setSize(TeVector3f32(0.12, (4.0 / ((winSize.y() / winSize.x()) * 4.0)) * 0.12, 0.0));
	} else {
		invbtn->setSize(TeVector3f32(0.08, (4.0 / ((winSize.y() / winSize.x()) * 4.0)) * 0.08, 0.0));
	}

	TeCheckboxLayout *markersCheckbox = _gui4.checkboxLayout("markersVisibleButton");
	markersCheckbox->setVisible(!_markersVisible);
	markersCheckbox->onStateChangedSignal().add(this, &Game::onMarkersVisible);

	initNoScale();
	removeNoScale2Children();
	app->_frontLayout.removeChild(_noScaleLayout2);

	TeLayout *vidLayout = _gui4.layout("videoLayout");
	app->_frontLayout.removeChild(vidLayout);
	removeNoScaleChildren();
	app->_frontLayout.removeChild(_noScaleLayout);

	app->_frontLayout.addChild(_noScaleLayout);
	addNoScaleChildren();
	app->_frontLayout.addChild(vidLayout);
	app->_frontLayout.addChild(_noScaleLayout2);
	addNoScale2Children();
	if (!fadeFlag) {
		if (_inventory.selectedObject().size()) {
			_inventory.selectedObject(*_inventory.selectedInventoryObject());
		}
		_inventory.setVisible(false);
		_objectif.setVisibleObjectif(false);
		_objectif.setVisibleButtonHelp(true);
		_running = true;
		loadScene("save.xml");
	}

	app->_backLayout.addChild(_scene.background());

	if (markerLuaExists) {
		TeLayout *bg = _gui2.layout("background");
		app->_frontLayout.addChild(bg);
	}

	Common::String camname = Common::String("Camera") + scene;
	_scene.setCurrentCamera(camname);

	// Special hacks for certain scenes (don't blame me, original does this..)
	if (scene == "14050") {
		TeIntrusivePtr<TeCamera> curcamera = _scene.currentCamera();
		const TeVector3f32 coords(1200.6f, -1937.5f, 1544.1f);
		curcamera->setPosition(coords);
	} else if (scene == "34610") {
		TeIntrusivePtr<TeCamera> curcamera = _scene.currentCamera();
		const TeVector3f32 coords(-328.243f, 340.303f, -1342.84f);
		curcamera->setPosition(coords);
		const TeQuaternion rot(0.003194f, 0.910923f, -0.009496f, -0.412389f);
		curcamera->setRotation(rot);
	}

	if (logicLuaExists) {
		_exitZone.clear();
		_luaScript.execute();
		_luaScript.execute("OnEnter", _prevSceneName);
		_luaScript.execute("OnSelectedObject", _inventory.selectedObject());
	}

	if (!_gameSounds.empty()) {
		//for (auto & sound : _gameSounds) {
		warning("TODO: Game::initWarp: Do game sound stuff here");
	}

	for (auto &randsoundlist : _randomSounds) {
		for (auto *randsound : randsoundlist._value) {
			delete randsound;
		}
		randsoundlist._value.clear();
	}
	_randomSounds.clear();

	_scene.initScroll();
	return true;
}

bool Game::isDocumentOpened() {
	return _documentsBrowser.layoutChecked("zoomed")->visible();
}

bool Game::isMoviePlaying() {
	TeButtonLayout *vidButton = _gui4.buttonLayout("videoBackgroundButton");
	if (vidButton)
		return vidButton->visible();
	return false;
}

bool Game::launchDialog(const Common::String &param_1, uint param_2, const Common::String &charname,
				  const Common::String &animfile, float param_5) {
	error("TODO: Implemet Game::launchDialog %s %d %s %s %f", param_1.c_str(),
			param_2, charname.c_str(), animfile.c_str(), param_5);
}

void Game::leave(bool flag) {
	if (!_enteredFlag2)
		return;

	deleteNoScale();
	_entered = false;
	_running = false;
	_notifier.unload();
	g_engine->getInputMgr()->_mouseLUpSignal.remove(this, &Game::onMouseClick);
	_question2.unload();
	_inventory.cellphone()->unload();
	_dialog2.unload();
	_inventory.unload();
	_documentsBrowser.unload();
	_inventoryMenu.unload();
	_gui1.unload();
	_scene.close();
	_gui3.unload();
	if (_scene._character) {
		_scene._character->deleteAllCallback();
		_scene._character->stop();
		_scene.unloadCharacter(_scene._character->_model->name());
	}
	warning("TODO: Game::leave: clear game sounds");

	for (auto *hitobj : _gameHitObjects) {
		delete hitobj;
	}
	_gameHitObjects.clear();

	// TODO: clear Game::HitObject tree here?

	_luaContext.destroy();
	_running = false;
	_gui4.buttonLayoutChecked("skipVideoButton")->onMouseClickValidated().remove(this, &Game::onSkipVideoButtonValidated);
	_gui4.buttonLayoutChecked("videoBackgroundButton")->onMouseClickValidated().remove(this, &Game::onLockVideoButtonValidated);
	_gui4.spriteLayoutChecked("video")->_tiledSurfacePtr->_frameAnim.onFinished().remove(this, &Game::onSkipVideoButtonValidated);
	_gui4.buttonLayoutChecked("inventoryButton")->onMouseClickValidated().remove(this, &Game::onInventoryButtonValidated);
	_gui4.unload();
	_playedTimer.stop();

	Application *app = g_engine->getApplication();
	app->_lockCursorButton.setVisible(false);
	app->_lockCursorFromActionButton.setVisible(false);
	// TODO: Set some inputmgr flag here?
	Character::animCacheFreeAll();
}

bool Game::loadBackup(const Common::String &path) {
	error("TODO: Implemet Game::loadBackup %s", path.c_str());
}

bool Game::loadCharacter(const Common::String &name) {
	bool result = true;
	Character *character = _scene.character(name);
	if (!character) {
		result = false;
		bool loaded = _scene.loadCharacter(name);
		if (loaded) {
			character = _scene.character(name);
			character->_onCharacterAnimFinishedSignal.remove<Game>(this, &Game::onCharacterAnimationFinished);
			character->_onCharacterAnimFinishedSignal.add<Game>(this, &Game::onCharacterAnimationFinished);
			character->onFinished().add<Game>(this, &Game::onDisplacementFinished);
		}
	}
	return result;
}

bool Game::loadPlayerCharacter(const Common::String &name) {
	bool result = _scene.loadPlayerCharacter(name);
	if (result) {
		_scene._character->_characterAnimPlayerFinishedSignal.remove<Game>(this, &Game::onCharacterAnimationPlayerFinished);
		_scene._character->_characterAnimPlayerFinishedSignal.add<Game>(this, &Game::onCharacterAnimationPlayerFinished);
		_scene._character->onFinished().remove<Game>(this, &Game::onDisplacementFinished);
		_scene._character->onFinished().add<Game>(this, &Game::onDisplacementFinished);
	}
	return result;
}

bool Game::loadScene(const Common::String &name) {
	_luaScript.load("scenes/OnGameEnter.lua");
	_luaScript.execute();
	Character *character = _scene._character;
	if (character && character->_model->visible()) {
		_sceneCharacterVisibleFromLoad = true;
	}
	return false;
}

bool Game::onAnswered(const Common::String &val) {
	_luaScript.execute("OnAnswered", TeVariant(val));
	return false;
}

bool Game::onCallNumber(Common::String val) {
	_luaScript.execute("OnCallNumber", TeVariant(val));
	return false;
}

bool Game::onCharacterAnimationFinished(const Common::String &val) {
	error("TODO: Implemet Game::onCharacterAnimationFinished %s", val.c_str());
}

bool Game::onCharacterAnimationPlayerFinished(const Common::String &val) {
	error("TODO: Implemet Game::onCharacterAnimationPlayerFinished %s", val.c_str());
}

bool Game::onDialogFinished(const Common::String &val) {
	error("TODO: Implemet Game::onDialogFinished %s", val.c_str());
}

bool Game::onDisplacementFinished() {
	error("TODO: Implemet Game::onDisplacementFinished");
}

bool Game::onFinishedCheckBackup(bool result) {
	if (_gameLoadState == 1) {
		_gameLoadState = 0;
		return true;
	}
	return false;
}

bool Game::onFinishedLoadingBackup(const Common::String &val) {
	if (_gameLoadState == 1) {
		_loadName = val;
		_gameLoadState = 2;
		return true;
	}
	return false;
}

bool Game::onFinishedSavingBackup(int something) {
	if (something) {
		g_engine->getGame()->_returnToMainMenu = true;
	}
	g_engine->getApplication()->showLoadingIcon(false);
	return true;
}

bool Game::onInventoryButtonValidated() {
	_inventoryMenu.enter();
	return false;
}

bool Game::onLockVideoButtonValidated() {
	TeButtonLayout *btn = _gui4.buttonLayoutChecked("skipVideoButton");
	btn->setVisible(!btn->visible());
	return true;
}

bool Game::onMarkersVisible(TeCheckboxLayout::State state) {
	_markersVisible = (state == 0);
	showMarkers(state == 0);
	return false;
}

static
TePickMesh2 *findNearestMesh(TeIntrusivePtr<TeCamera> &camera, const TeVector2s32 &frompt,
			Common::Array<TePickMesh2*> &pickMeshes, TeVector3f32 *outloc, bool lastHitFirst) {
	TeVector3f32 locresult;
	TePickMesh2 *nearest = nullptr;
	float furthest = camera->_orthFarVal;
	if (!pickMeshes.empty()) {
		TeVector3f32 v1;
		TeVector3f32 v2;
		for (unsigned int i = 0; i < pickMeshes.size(); i++) {
			TePickMesh2 *mesh = pickMeshes[i];
			const TeMatrix4x4 transform = mesh->worldTransformationMatrix();
			if (lastHitFirst) {
				unsigned int tricount = mesh->verticies().size() / 3;
				unsigned int vert = mesh->lastTriangleHit() * 3;
				if (mesh->lastTriangleHit() >= tricount)
					vert = 0;
				const TeVector3f32 v3 = transform * mesh->verticies()[vert];
				const TeVector3f32 v4 = transform * mesh->verticies()[vert + 1];
				const TeVector3f32 v5 = transform * mesh->verticies()[vert + 2];
				TeVector3f32 result;
				float fresult;
				int intresult = TeRayIntersection::intersect(v1, v2, v3, v4, v5, result, fresult);
				if (intresult == 1 && fresult < furthest && fresult >= camera->_orthNearVal)
					return mesh;
			}
			for (unsigned int tri = 0; tri < mesh->verticies().size() / 3; tri++) {
				const TeVector3f32 v3 = transform * mesh->verticies()[tri * 3];
				const TeVector3f32 v4 = transform * mesh->verticies()[tri * 3 + 1];
				const TeVector3f32 v5 = transform * mesh->verticies()[tri * 3 + 1];
				camera->getRay(frompt, v1, v2);
				TeVector3f32 result;
				float fresult;
				int intresult = TeRayIntersection::intersect(v1, v2, v3, v4, v5, result, fresult);
				if (intresult == 1 && fresult < furthest && fresult >= camera->_orthNearVal) {
					mesh->setLastTriangleHit(tri);
					locresult = result;
					furthest = fresult;
					nearest = mesh;
					if (lastHitFirst)
						break;
				}
			}
		}
	}
	if (outloc) {
		*outloc = locresult;
	}
	return nearest;
}

bool Game::onMouseClick(const Common::Point &pt) {
	Application *app = g_engine->getApplication();

	if (app->isFading())
		return true;

	_posPlayer = TeVector3f32(-1.0f, -1.0f, -1.0f);
	if (_previousMousePos == TeVector2s32(-1, -1)) {
		_previousMousePos = pt;
	} else {
		TeVector3f32 winSize = app->getMainWindow().size();
		TeVector2s32 lastMousePos = _previousMousePos;
		_previousMousePos = pt;
		float xdist = pt.x / winSize.x() - lastMousePos._x / winSize.x();
		float ydist = pt.y / winSize.y() - lastMousePos._y / winSize.y();
		float sqrdist = xdist * xdist + ydist * ydist;
		if (sqrdist > 0.0001 && (!_walkTimer.running() || _walkTimer.timeElapsed() > 300000.0
						 || (_scene._character && _scene._character->walkModeStr() != "Walk"))) {
			return false;
			// Double-click, but already jogging
		}
	}

	if (!app->_frontLayout.isMouseIn(pt))
		return false;

	Common::String nearestMeshName = "None";
	TeIntrusivePtr<TeCamera> curCamera = _scene.currentCamera();
	Common::Array<TePickMesh2*> pickMeshes = _scene.pickMeshes();
	TePickMesh2 *nearestMesh = findNearestMesh(curCamera, pt, pickMeshes, nullptr, false);
	if (nearestMesh) {
		nearestMeshName = nearestMesh->name();
		_lastCharMoveMousePos = TeVector2s32(0, 0);
	}

	if (app->isLockCursor() || _movePlayerCharacterDisabled)
		return false;
	
	Character *character = _scene._character;
	const Common::String &charAnim = character->curAnimName();
	
	if (charAnim == character->characterSettings()._walkFileName
		|| charAnim == character->walkAnim(Character::WalkPart_Start)
		|| charAnim == character->walkAnim(Character::WalkPart_Loop)
		|| charAnim == character->walkAnim(Character::WalkPart_EndD)
		|| charAnim == character->walkAnim(Character::WalkPart_EndG)) {
		_luaScript.execute("On");
		if (!_scene.isObjectBlocking(nearestMeshName)) {
			if (character->freeMoveZone()) {
				TeVector3f32 charPos = character->_model->position();
				TeIntrusivePtr<TeBezierCurve> curve = character->freeMoveZone()->curve(charPos, TeVector2s32(pt), 8.0, true);
				if (curve) {
					_scene.setCurve(curve);
					// TODO: set character field_0x214 to TeVector3f32(0,0,0)
					if (curve->controlPoints().size() == 1) {
						character->endMove();
					} else {
						if (!_walkTimer.running() || _walkTimer.timeElapsed() > 300000) {
							_walkTimer.stop();
							_walkTimer.start();
							character->walkMode("Walk");
						} else {
							// Note: original checks the timer elapsed again here.. why?
							_walkTimer.stop();
							character->walkMode("Jog");
						}
						character->placeOnCurve(curve);
						character->setCurveOffset(0.0);
						if (charAnim != character->walkAnim(Character::WalkPart_Start)) {
							character->setAnimation(character->walkAnim(Character::WalkPart_Start), false, false, false, -1, 9999);
						}
						character->walkTo(1.0, false);
						_sceneCharacterVisibleFromLoad = false;
						_lastCharMoveMousePos = pt;
					}
				} else {
					return false;
				}
			}
			// TOOD: Finis this (line ~180 onward)
			warning("TODO: Finish Game::onMouseClick");
		}
		TeVector3f32 lastPoint = _scene.curve()->controlPoints().back();
		character->setAnimation(character->walkAnim(Character::WalkPart_Loop), true, false, false, -1, 9999);
		character->walkTo(1.0, false);
		// TODO: Set app field field_0x910b
		_posPlayer = lastPoint;
	}
	
	if (!_sceneCharacterVisibleFromLoad || (character->curAnimName() == character->characterSettings()._walkFileName)) {
		_lastCharMoveMousePos = TeVector2s32(0, 0);
		_movePlayerCharacterDisabled = true;
		// TODO: Set field 	0x4249 to false
		if (nearestMesh) {
			character->stop();
			_luaScript.execute("OnWarpObjectHit", nearestMeshName);
		}
	}

	return false;
}

// Note: None of these cursor files seem to be actually shipped with the game
// but the logic is reproduced here just in case there's some different
// version that uses them.
static const char cursorsTable[][2][80] = {
	{"H000", "pictures/F000.png"},
	{"H045", "pictures/F045.png"},
	{"H090", "pictures/F090.png"},
	{"H135", "pictures/F135.png"},
	{"H180", "pictures/F180.png"},
	{"H225", "pictures/F225.png"},
	{"H270", "pictures/F270.png"},
	{"H315", "pictures/F315.png"},
	{"Main", "pictures/Main.png"},
	{"Action", "pictures/Action.png"},
	{"Parler", "pictures/Cursor_Large/Anim_Cursor_Talking.anim"},
	{"Type01", "pictures/Type01.png"},
	{"Type02", "pictures/Type02.png"},
	{"Type03", "pictures/Type03.png"},
	{"Type04", "pictures/Type04.png"},
	{"Type05", "pictures/Type05.png"}
};

bool Game::onMouseMove() {
	if (!_entered)
		return false;

	static const Common::Path DEFAULT_CURSOR("pictures/cursor.png");

	Application *app = g_engine->getApplication();

	if (app->isLockCursor()) {
		app->mouseCursorLayout().load(DEFAULT_CURSOR);
		return false;
	}

	// TODO: All the logic below is a bit suspicious and unfinished.
	//
	// The original game goes through a series of checks of mouseIn and
	// visible before deciding whether to do a full search for a mouse
	// cursor to apply.  But after all that, in practice, none of the
	// mouse cursors above actually exist in the game data.
	//
	// So maybe all this is useless?

#if ENABLE_CUSTOM_CURSOR_CHECKS
	TeVector2s32 mouseLoc = g_engine->getInputMgr()->lastMousePos();
	bool skipFullSearch = false;

	TeLayout *cellphone = _inventory.cellphone()->gui().layoutChecked("cellphone");
	TeLayout *cellbg = _inventory.cellphone()->gui().layoutChecked("background");
	if (cellphone->isMouseIn(mouseLoc)) {
		skipFullSearch = true;
		if (!cellbg->visible() && _objectif.isMouseIn(mouseLoc)) {
			app->mouseCursorLayout().load(DEFAULT_CURSOR);
			return false;
		}
	}
	TeLayout *imgDialog = _dialog2.gui().layoutChecked("imgDialog");
	bool mouseInImgDialog = imgDialog->isMouseIn(mouseLoc);
	if (mouseInImgDialog || !imgDialog->visible()) {
		if (!mouseInImgDialog)
			skipFullSearch = true;
		//warning("TODO: Finish Game::onMouseMove");
	}

	bool checkedCursor = false;
	if (!skipFullSearch && _scene.gui2().loaded()) {
		TeLayout *bglayout = _scene.gui2().layoutChecked("background");
		for (unsigned int i = 0; i < bglayout->childCount(); i++) {
			TeLayout *childlayout = dynamic_cast<TeLayout *>(bglayout->child(i));
			if (childlayout && childlayout->isMouseIn(mouseLoc) && childlayout->visible()) {
				for (int i = 0; i < ARRAYSIZE(cursorsTable); i++) {
					if (childlayout->name().contains(cursorsTable[i][0])) {
						app->mouseCursorLayout().load(cursorsTable[i][1]);
						if (Common::String(cursorsTable[i][0]).contains(".anim")) {
							app->mouseCursorLayout()._tiledSurfacePtr->_frameAnim._loopCount = -1;
							app->mouseCursorLayout()._tiledSurfacePtr->_frameAnim.play();
						}
					}
				}
				checkedCursor = true;
			}
		}
	}

	if (!checkedCursor)
		app->mouseCursorLayout().load(DEFAULT_CURSOR);
#endif // ENABLE_CUSTOM_CURSOR_CHECKS
	return false;
}

bool Game::onSkipVideoButtonValidated() {
	TeSpriteLayout *sprite = _gui4.spriteLayoutChecked("video");
	TeButtonLayout *btn = _gui4.buttonLayoutChecked("videoBackgroundButton");
	sprite->stop();
	btn->setVisible(false);
	return false;
}

bool Game::onVideoFinished() {
	if (!_gui4.loaded())
		return false;

	Application *app = g_engine->getApplication();

	app->captureFade();

	TeSpriteLayout *video = _gui4.spriteLayoutChecked("video");
	TeButtonLayout *btn = _gui4.buttonLayoutChecked("videoBackgroundButton");
	btn->setVisible(false);
	btn = _gui4.buttonLayoutChecked("skipVideoButton");
	btn->setVisible(false);
	video->setVisible(false);
	_music.stop();
	_running = true;
	warning("TODO: Game::onVideoFinished: update yieldedCallbacks %s", video->_tiledSurfacePtr->path().toString().c_str());
	_luaScript.execute("OnMovieFinished", video->_tiledSurfacePtr->path().toString());
	app->fade();
	return false;
}

void Game::pauseMovie() {
	_music.pause();
	TeSpriteLayout *sprite = _gui4.spriteLayoutChecked("video");
	sprite->pause();
}

void Game::playMovie(const Common::String &vidPath, const Common::String &musicPath) {
	Application *app = g_engine->getApplication();
	app->captureFade();
	TeButtonLayout *videoBackgroundButton = _gui4.buttonLayoutChecked("videoBackgroundButton");
	videoBackgroundButton->setVisible(true);

	TeButtonLayout *skipVideoButton = _gui4.buttonLayoutChecked("skipVideoButton");
	skipVideoButton->setVisible(false);

	TeMusic &music = app->music();
	music.stop();
	music.setChannelName("video");
	music.repeat(false);
	music.volume(1.0f);
	music.load(musicPath);

	_running = false;

	TeSpriteLayout *videoSpriteLayout = _gui4.spriteLayoutChecked("video");
	videoSpriteLayout->load(vidPath);
	videoSpriteLayout->setVisible(true);
	music.play();
	videoSpriteLayout->play();

	// FIXME TODO!! Stop the movie and soundearly for testing.
	videoSpriteLayout->_tiledSurfacePtr->_frameAnim._nbFrames = 10;
	music.stop();

	app->fade();
}

void Game::playRandomSound(const Common::String &name) {
	if (!_randomSounds.contains(name)) {
		warning("Game::playRandomSound: can't find sound list %s", name.c_str());
		return;
    }
    error("TODO: Implemet Game::playRandomSound");
}

void Game::playSound(const Common::String &name, int param_2, float param_3) {
	error("TODO: Implemet Game::playSound");
}

void Game::removeNoScale2Child(TeLayout *layout) {
	if (!_noScaleLayout2 || !layout)
		return;
	_noScaleLayout2->removeChild(layout);
}

void Game::removeNoScale2Children() {
	if (!_noScaleLayout2)
		return;

	TeLayout *vidbtn = _gui4.layout("videoButtonLayout");
	if (vidbtn)
		_noScaleLayout2->removeChild(vidbtn);

	TeLayout *bg = _inventory.cellphone()->gui().layout("background");
	if (bg)
		_noScaleLayout2->removeChild(bg);

	TeButtonLayout *bgbtn = _objectif.gui1().buttonLayout("background");
	if (bgbtn)
		_noScaleLayout2->removeChild(bgbtn);

	TeLayout *notifier = _notifier.gui().layout("notifier");
	if (notifier)
		_noScaleLayout2->removeChild(notifier);
}

void Game::removeNoScaleChildren() {
	if (!_noScaleLayout)
		return;
	_noScaleLayout->removeChild(&_question2);
	Application *app = g_engine->getApplication();
	app->_frontLayout.removeChild(&_dialog2);
	_noScaleLayout->removeChild(&_inventory);
	_noScaleLayout->removeChild(&_inventoryMenu);
	_noScaleLayout->removeChild(&_documentsBrowser);
	_noScaleLayout->removeChild(&_documentsBrowser.zoomedLayout());
}

void Game::resetPreviousMousePos() {
	_previousMousePos = TeVector2s32(-1, -1);
}

void Game::resumeMovie() {
	_music.play();
	_gui4.spriteLayout("video")->play();
}

void Game::saveBackup(const Common::String &saveName) {
	error("TODO: Implemet me");
}

void Game::setBackground(const Common::String &name) {
	_scene.changeBackground(name);
}

void Game::setCurrentObjectSprite(const Common::String &spritePath) {
	TeSpriteLayout *currentSprite = _gui4.spriteLayout("currentObjectSprite");
	if (currentSprite) {
		if (!spritePath.empty()) {
			currentSprite->unload();
		} else {
			currentSprite->load(spritePath);
	  }
	}
}

bool Game::showMarkers(bool val) {
	error("TODO: Implemet me");
}

bool Game::startAnimation(const Common::String &animName, int param_2, bool param_3) {
	error("TODO: Implemet me");
}

void Game::stopSound(const Common::String &name) {
	error("TODO: Implemet me");
}

bool Game::unloadCharacter(const Common::String &charname) {
	Character *c = _scene.character(charname);
	if (!c)
		return false;

	for (unsigned int i = 0; i < _scene.models().size(); i++) {
		if (_scene.models()[i] == c->_model) {
			_scene.models().remove_at(i);
			break;
		}
	}
	c->_onCharacterAnimFinishedSignal.remove(this, &Game::onCharacterAnimationFinished);
	c->removeAnim();
	c->onFinished().remove(this, &Game::onDisplacementFinished);
	_scene.unloadCharacter(charname);
	return true;
}

bool Game::unloadCharacters() {
	// Loop will update the array, take a copy first.
	Common::Array<Character *> chars = _scene._characters;
	for (Character *c : chars) {
		unloadCharacter(c->_model->name());
	}
	return true;
}

bool Game::unloadPlayerCharacter(const Common::String &character) {
	_scene.unloadCharacter(character);
	return true;
}

void Game::update() {
	if (!_entered)
		return;

	TeTextLayout *debugTimeTextLayout = _gui4.textLayout("debugTimeText1");
	if (debugTimeTextLayout) {
		warning("TODO: Game::update: Fill out debugTimeTextLayout");
	}

	if (!_warped) {
		if (_gameLoadState == 2) {
			initLoadedBackupData();
			return;
		} else if (_gameLoadState != 0) {
			return;
		}

		Application *app = g_engine->getApplication();

		if (_scene._character) {
			if (!_scene._character->_model->visible())
				app->_lockCursorButton.setVisible(false);
		}

		TeButtonLayout *invbtn = _gui4.buttonLayout("inventoryButton");
		if (invbtn)
			invbtn->setVisible(!app->isLockCursor() && !_dialog2.isDialogPlaying());
		else
			warning("Game::update: InventoryButton is null.");

		if (_scene._character) {
			TeIntrusivePtr<TeModel> model = _scene._character->_model;
			bool modelVisible = model->visible();
			if (!model->anim().get())
				_scene._character->permanentUpdate();
			if (modelVisible) {
				if (_scene._character->needsSomeUpdate()) {
					Game *game = g_engine->getGame();
					game->_sceneCharacterVisibleFromLoad = false;
					TeVector3f32 charPos = _scene._character->_model->position();
					const Common::String charName = _scene._character->_model->name();
					TeFreeMoveZone *zone = _scene.pathZone(_scene._character->freeMoveZoneName());
					if (zone) {
						TeIntrusivePtr<TeCamera> cam = _scene.currentCamera();
						zone->setCamera(cam, false);
						_scene._character->setFreeMoveZone(zone);
						_scene.setPositionCharacter(charName, _scene._character->freeMoveZoneName(), charPos);
						error("TODO: Game::update: Finish bit after permanentUpdate");
					}
					_scene._character->setNeedsSomeUpdate(false);
				}

				const Common::String &charAnim = _scene._character->curAnimName();
				bool lockCursor = (charAnim == _scene._character->walkAnim(Character::WalkPart_Start) ||
						charAnim == _scene._character->walkAnim(Character::WalkPart_Loop) ||
						charAnim == _scene._character->walkAnim(Character::WalkPart_EndD) ||
						charAnim == _scene._character->walkAnim(Character::WalkPart_EndG) ||
						charAnim == _scene._character->characterSettings()._walkFileName);
				app->lockCursor(lockCursor);
			}
		}

		Common::Array<Character *> characters = _scene._characters;
		for (Character *c : characters) {
			if (!c->_model->anim().get())
				c->permanentUpdate();
		}

		Common::Point mousePos = g_engine->getInputMgr()->lastMousePos();
		if (_lastUpdateMousePos != mousePos) {
			onMouseMove();
			_lastUpdateMousePos = mousePos;
		}
		if (_saveRequested) {
			_saveRequested = false;
			error("TODO: Game::update: Save game");
		}

		_luaScript.execute("Update");
		_objectif.update();
		_scene.update();
	} else {
		warning("TODO: Game::update: Stop sounds before warping");
		changeWarp2(_warpZone, _warpScene, _warpFadeFlag);
	}
}


bool Game::HitObject::onChangeWarp() {
	// Seems like this function is never used?
	error("TODO: Implement Game::HitObject::onChangeWarp");
	return false;
}

bool Game::HitObject::onDown() {
	_game->luaScript().execute("OnButtonDown", _name);
	// TODO: set this field _game->field_0x4249 = 1;
	return false;
}

bool Game::HitObject::onUp() {
	_game->luaScript().execute("OnButtonUp", _name);
	// TODO: set this field _game->field_0x4249 = 1;
	return false;
}

bool Game::HitObject::onValidated() {
	if (!g_engine->getApplication()->isLockCursor()) {
		_game->luaScript().execute("OnWarpObjectHit", _name);
		// TODO: set this field _game->field_0x4249 = 1;
	}
	return false;
}

} // end namespace Tetraedge
