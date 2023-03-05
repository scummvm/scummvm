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
#include "common/savefile.h"
#include "common/config-manager.h"

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
#include "tetraedge/te/te_sound_manager.h"
#include "tetraedge/te/te_variant.h"
#include "tetraedge/te/te_lua_thread.h"

namespace Tetraedge {

Game::Game() : _objectsTakenVal(0), _returnToMainMenu(false), _entered(false),
_enteredFlag2(false), _running(false), _movePlayerCharacterDisabled(false),
_noScaleLayout(nullptr), _noScaleLayout2(nullptr), _isCharacterIdle(true),
_sceneCharacterVisibleFromLoad(false), _isCharacterWalking(false),
_lastCharMoveMousePos(0.0f, 0.0f), _randomSoundFinished(false),
_previousMousePos(-1, -1), _markersVisible(true), _saveRequested(false),
_gameLoadState(0), _luaShowOwnerError(false), _score(0), _warped(false),
_firstInventory(true), _randomSource("SyberiaGameRandom"), _frameCounter(0),
_warpFadeFlag(false), _dialogsTold(0), _runModeEnabled(true) {
	for (int i = 0; i < NUM_OBJECTS_TAKEN_IDS; i++) {
		_objectsTakenBits[i] = false;
	}
	_randomSound = new RandomSound();
	_dialog2.onAnimationDownFinishedSignal().add(this, &Game::onDialogFinished);
	_question2.onAnswerSignal().add(this, &Game::onAnswered);
}

Game::~Game() {
	if (_entered) {
		leave(true);
	}
	delete _randomSound;
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

	if (Common::File::exists(animPath)) {
		const Common::StringArray parts = TetraedgeEngine::splitString(anim, '/');
		assert(parts.size() >= 2);

		const Common::String layoutName = parts[1];
		const Common::String path = Common::String("scenes/") + parts[0] + "/" + parts[1] + "/Set" + parts[1];

		_setAnimGui.load(path + ".lua");

		// Note: game makes this here, but never uses it..
		// it seems like a random memory leak??
		// TeSpriteLayout *spritelayout = new TeSpriteLayout();

		TeSpriteLayout *spritelayout = findSpriteLayoutByName(_setAnimGui.layoutChecked("root"), layoutName);

		_scene.bgGui().layoutChecked("root")->addChild(spritelayout);
		return true;
	}

	return false;
}

/*static*/
Common::String Game::artworkConfName(const Common::String &name) {
	Common::String configName = Common::String::format("artwork_%s", name.c_str());
	for (uint i = 0; i < configName.size(); i++) {
		if (configName[i] == '/' || configName[i] == '.')
			configName.setChar('_', i);
	}
	return configName;
}

void Game::addArtworkUnlocked(const Common::String &name, bool notify) {
	const Common::String configName = artworkConfName(name);
	if (_unlockedArtwork.contains(configName))
		return;
	ConfMan.setBool(configName, true);
	ConfMan.flushToDisk();
	_unlockedArtwork[configName] = true;
	if (notify)
		_notifier.push("BONUS!", "Inventory/Objects/VPapierCrayon.png");
}

bool Game::isArtworkUnlocked(const Common::String &name) const {
	const Common::String configName = artworkConfName(name);
	return _unlockedArtwork.getValOrDefault(configName, false);
}

void Game::addNoScale2Child(TeLayout *layout) {
	if (_noScaleLayout2 && layout) {
		_noScaleLayout2->addChild(layout);
	}
}

void Game::addNoScale2Children() {
	if (!_noScaleLayout2)
		return;

	TeLayout *vidbtn = _inGameGui.layout("videoButtonLayout");
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
	TeLayout *inGame = _inGameGui.layout("inGame");
	if (inGame)
		_noScaleLayout->addChild(inGame);

	_noScaleLayout->addChild(&_question2);

	Application *app = g_engine->getApplication();
	app->frontLayout().addChild(&_dialog2);

	_noScaleLayout->addChild(&_inventory);
	_noScaleLayout->addChild(&_inventoryMenu);
	_noScaleLayout->addChild(&_documentsBrowser);
	_noScaleLayout->addChild(&_documentsBrowser.zoomedLayout());
}

void Game::addRandomSound(const Common::String &name, const Common::String &path, float f1, float volume) {
	if (!_randomSounds.contains(name)) {
		_randomSounds[name] = Common::Array<RandomSound*>();
	}
	RandomSound *randsound = new RandomSound();
	randsound->_path = path;
	randsound->_f1 = f1;
	randsound->_volume = volume;
	randsound->_name = name;
	_randomSounds[name].push_back(randsound);
}

void Game::addToBag(const Common::String &objid) {
	if (_inventory.objectCount(objid) != 0)
		return;
	_inventory.addObject(objid);
	Common::String imgpath("Inventory/Objects/");
	imgpath += objid;
	imgpath += ".png";
	_notifier.push(_inventory.objectName(objid), imgpath);
	for (int i = 0; i < NUM_OBJECTS_TAKEN_IDS; i++) {
		if (objid == OBJECTS_TAKEN_IDS[i] && !_objectsTakenBits[i]) {
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
	//debug("Game::changeWarp(%s, %s, %s)", zone.c_str(), scene.c_str(), fadeFlag ? "true" : "false");
	Application *app = g_engine->getApplication();
	if (fadeFlag && g_engine->gameType() == TetraedgeEngine::kSyberia) {
		app->blackFade();
	} else {
		app->captureFade();
	}
	// Slight divergence from original.. free after capturing fade so characters don't disappear.
	if (g_engine->gameType() == TetraedgeEngine::kSyberia2)
		_scene.freeGeometry();

	_warpZone = zone;
	_warpScene = scene;
	_warpFadeFlag = fadeFlag;
	_warped = true;
	return true;
}

bool Game::changeWarp2(const Common::String &zone, const Common::String &scene, bool fadeFlag) {
	//debug("Game::changeWarp2(%s, %s, %s)", zone.c_str(), scene.c_str(), fadeFlag ? "true" : "false");
	_warped = false;
	_movePlayerCharacterDisabled = false;
	_sceneCharacterVisibleFromLoad = false;
	// TODO? _charMoveMouseEventNo = -1?
	_isCharacterIdle = true;
	_isCharacterWalking = false;
	Common::Path luapath("scenes");
	luapath.joinInPlace(zone);
	luapath.joinInPlace(scene);
	luapath.joinInPlace("Logic");
	luapath.appendInPlace(scene);
	luapath.appendInPlace(".lua");

	if (g_engine->getCore()->findFile(luapath).exists()) {
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

	_forGui.unload();
	_prevSceneName = _currentScene;
	if (!fadeFlag)
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

void Game::enter() {
	_enteredFlag2 = true;
	_entered = true;
	_luaShowOwnerError = false;
	_score = 0;
	Application *app = g_engine->getApplication();
	app->visualFade().init();
	// TODO: Original puts this click handler at -10000.. but then it never gets hit?
	Common::SharedPtr<TeCallback1Param<Game, const Common::Point &>> callbackptr(new TeCallback1Param<Game, const Common::Point &>(this, &Game::onMouseClick, 10000.0f));
	g_engine->getInputMgr()->_mouseLUpSignal.push_back(callbackptr);
	_movePlayerCharacterDisabled = false;
	// TODO? Set _charMoveMouseEventNo = -1
	_isCharacterIdle = true;
	_sceneCharacterVisibleFromLoad = false;
	Character::loadSettings("models/ModelsSettings.xml");
	Object3D::loadSettings("objects/ObjectsSettings.xml");
	if (_scene._character) {
		_scene._character->onFinished().remove(this, &Game::onDisplacementPlayerFinished);
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

	_gameEnterScript.attachToContext(&_luaContext);

	if (!_objectif.gui1().loaded()) {
		_objectif.load();
	}
	_question2.load();
	_dialog2.load();
	_documentsBrowser.load();
	_documentsBrowser.loadZoomed();
	_inventory.load();

	_inventory.cellphone()->onCallNumber().add(this, &Game::onCallNumber);

	if (hasLoadName()) {
		loadBackup(_loadName);
	} else {
		_gameLoadState = 1;
		onFinishedLoadingBackup("");
	}
	_sceneCharacterVisibleFromLoad = true;
	_scene._character->onFinished().remove(this, &Game::onDisplacementPlayerFinished);
	_scene._character->onFinished().add(this, &Game::onDisplacementPlayerFinished);
	_prevSceneName.clear();
	_notifier.load();
}

/*static*/
TeI3DObject2 *Game::findLayoutByName(TeLayout *parent, const Common::String &name) {
	// Seems like this is never used?
	error("TODO: Implement Game::findLayoutByName");
}

/*static*/
TeSpriteLayout *Game::findSpriteLayoutByName(TeLayout *parent, const Common::String &name) {
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
	app->setFinishedGame(true);
	app->setFinishedFremium(true);
}

void Game::finishGame() {
	Application *app = g_engine->getApplication();
	// FIXME: The original sets this, but then Application::run immediately
	// returns to the menu.. how does the original wait for the credits to end??
	//app->_finishedGame = true;
	_playedTimer.stop();
	/* Game does this but does nothing with result?
	if (app->difficulty() == 2) {
		_playedTimer.getTimeFromStart();
	} */
	app->credits().enter(false);
}

void Game::initLoadedBackupData() {
	bool warpFlag = true;
	Application *app = g_engine->getApplication();
	Common::String firstWarpPath;
	if (!_loadName.empty()) {
		warpFlag = false;
		Common::InSaveFile *saveFile = g_engine->getSaveFileManager()->openForLoading(_loadName);
		Common::Error result = g_engine->loadGameStream(saveFile);
		if (result.getCode() == Common::kNoError) {
			ExtendedSavegameHeader header;
			if (MetaEngine::readSavegameHeader(saveFile, &header))
				g_engine->setTotalPlayTime(header.playtime);
		}
	} else {
		firstWarpPath = app->firstWarpPath();
		_currentScene = app->firstScene();
		_currentZone = app->firstZone();
		_playedTimer.start();
		_objectsTakenVal = 0;
		for (int i = 0; i < ARRAYSIZE(_objectsTakenBits); i++) {
			_objectsTakenBits[i] = 0;
		}
		_dialogsTold = 0;
		if (_loadName == "NO_OWNER")
			_luaShowOwnerError = true;
	}
	_gameLoadState = 0;
	app->showLoadingIcon(false);
	_loadName.clear();
	initScene(warpFlag, firstWarpPath);
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
	_inGameGui.unload();
	_movePlayerCharacterDisabled = false;
	_sceneCharacterVisibleFromLoad = true;

	if (_scene._character) {
		_scene._character->_model->setVisible(true);
		_scene._character->deleteAllCallback();
		_scene._character->stop();
		_scene._character->setAnimation(_scene._character->characterSettings()._idleAnimFileName, true);
		if (!_scene.findKate()) {
			_scene.models().push_back(_scene._character->_model);
			if (_scene._character->_shadowModel[0]) {
				_scene.models().push_back(_scene._character->_shadowModel[0]);
				_scene.models().push_back(_scene._character->_shadowModel[1]);
			}
		}
	}

	_currentZone = zone;
	_currentScene = scene;

	_scene.loadBlockers();
	Common::Path scenePath("scenes");
	scenePath.joinInPlace(zone);
	scenePath.joinInPlace(scene);
	_sceneZonePath = scenePath;

	TeCore *core = g_engine->getCore();

	const Common::FSNode intLuaNode = core->findFile(scenePath.join(Common::String::format("Int%s.lua", scene.c_str())));
	const Common::FSNode logicLuaNode = core->findFile(scenePath.join(Common::String::format("Logic%s.lua", scene.c_str())));
	const Common::FSNode setLuaNode = core->findFile(scenePath.join(Common::String::format("Set%s.lua", scene.c_str())));
	Common::FSNode forLuaNode = core->findFile(scenePath.join(Common::String::format("For%s.lua", scene.c_str())));
	const Common::FSNode markerLuaNode = core->findFile(scenePath.join(Common::String::format("Marker%s.lua", scene.c_str())));

	bool intLuaExists = intLuaNode.exists();
	bool logicLuaExists = logicLuaNode.exists();
	bool setLuaExists = setLuaNode.exists();
	bool forLuaExists = forLuaNode.exists();
	if (!forLuaExists) {
		// slight hack.. try an alternate For lua path.
		forLuaNode = core->findFile(scenePath.join("Android-MacOSX").join(Common::String::format("For%s.lua", scene.c_str())));
		forLuaExists = forLuaNode.exists();
		debug("searched for %s", forLuaNode.getName().c_str());
	}
	bool markerLuaExists = markerLuaNode.exists();

	if (!intLuaExists && !logicLuaExists && !setLuaExists && !forLuaExists && !markerLuaExists) {
		debug("No lua scripts for scene %s zone %s", scene.c_str(), zone.c_str());
		return false;
	}

	for (auto &sound : _gameSounds) {
		sound->setRetain(false);
	}

	if (logicLuaExists) {
		_luaContext.addBindings(LuaBinds::LuaOpenBinds);
		_luaScript.attachToContext(&_luaContext);
		_luaScript.load(core->findFile("menus/help/help.lua"));
		_luaScript.execute();
		_luaScript.load(logicLuaNode);
	}

	if (_forGui.loaded())
		_forGui.unload();

	_scene.reset();
	_scene.bgGui().unload();
	_scene.markerGui().unload();
	_scene.hitObjectGui().unload();
	Common::Path geomPath(Common::String::format("scenes/%s/Geometry%s.bin",
												 zone.c_str(), zone.c_str()));
	Common::FSNode geomFile = core->findFile(geomPath);
	if (geomFile.isReadable()) {
		// Syberia 1, load geom bin
		_scene.load(geomFile);
	} else {
		// Syberia 2, load from xml
		_scene.loadXml(zone, scene);
	}
	_scene.loadBackground(setLuaNode);

	Application *app = g_engine->getApplication();
	if (forLuaExists) {
		_forGui.load(forLuaNode);
		TeLayout *bg = _forGui.layoutChecked("background");
		bg->setRatioMode(TeILayout::RATIO_MODE_NONE);
		app->frontLayout().addChild(bg);
		// Note: Game also adds cellphone to both frontLayout *and* noScaleLayout2,
		// so we reproduce the broken behavior exactly.
		TeLayout *cellbg = _inventory.cellphone()->gui().buttonLayoutChecked("background");
		app->frontLayout().removeChild(cellbg);
		app->frontLayout().addChild(cellbg);
		_objectif.reattachLayout(&app->frontLayout());
	}

	if (intLuaExists) {
		_scene.loadInteractions(intLuaNode);
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
	_inGameGui.load("InGame.lua");

	TeButtonLayout *skipbtn = _inGameGui.buttonLayout("skipVideoButton");
	skipbtn->setVisible(false);
	skipbtn->onMouseClickValidated().remove(this, &Game::onSkipVideoButtonValidated);
	skipbtn->onMouseClickValidated().add(this, &Game::onSkipVideoButtonValidated);

	TeButtonLayout *vidbgbtn = _inGameGui.buttonLayout("videoBackgroundButton");
	vidbgbtn->setVisible(false);
	vidbgbtn->onMouseClickValidated().remove(this, &Game::onLockVideoButtonValidated);
	vidbgbtn->onMouseClickValidated().add(this, &Game::onLockVideoButtonValidated);

	TeSpriteLayout *video = _inGameGui.spriteLayout("video");
	video->setVisible(false);
	video->_tiledSurfacePtr->_frameAnim.onStop().remove(this, &Game::onVideoFinished);
	video->_tiledSurfacePtr->_frameAnim.onStop().add(this, &Game::onVideoFinished);

	TeButtonLayout *invbtn = _inGameGui.buttonLayoutChecked("inventoryButton");
	invbtn->onMouseClickValidated().remove(this, &Game::onInventoryButtonValidated);
	invbtn->onMouseClickValidated().add(this, &Game::onInventoryButtonValidated);
	invbtn->setSizeType(TeILayout::RELATIVE_TO_PARENT);

	const TeVector3f32 winSize = app->getMainWindow().size();
	if (g_engine->getCore()->fileFlagSystemFlag("definition") == "SD") {
		invbtn->setSize(TeVector3f32(0.12f, (4.0f / ((winSize.y() / winSize.x()) * 4.0f)) * 0.12f, 0.0));
	} else {
		invbtn->setSize(TeVector3f32(0.08f, (4.0f / ((winSize.y() / winSize.x()) * 4.0f)) * 0.08f, 0.0));
	}

	TeCheckboxLayout *markersCheckbox = _inGameGui.checkboxLayout("markersVisibleButton");
	markersCheckbox->setState(_markersVisible ? TeCheckboxLayout::CheckboxStateActive : TeCheckboxLayout::CheckboxStateUnactive);
	markersCheckbox->onStateChangedSignal().add(this, &Game::onMarkersVisible);

	initNoScale();
	removeNoScale2Children();
	app->frontLayout().removeChild(_noScaleLayout2);

	TeLayout *vidLayout = _inGameGui.layout("videoLayout");
	app->frontLayout().removeChild(vidLayout);
	removeNoScaleChildren();
	app->frontLayout().removeChild(_noScaleLayout);

	app->frontLayout().addChild(_noScaleLayout);
	addNoScaleChildren();
	app->frontLayout().addChild(vidLayout);
	app->frontLayout().addChild(_noScaleLayout2);
	addNoScale2Children();
	if (!fadeFlag) {
		if (_inventory.selectedObject().size()) {
			_inventory.selectedObject(_inventory.selectedInventoryObject());
		}
		_inventory.setVisible(false);
		_objectif.setVisibleObjectif(false);
		_objectif.setVisibleButtonHelp(true);
		_running = true;
		loadScene("save.xml");
	}

	app->backLayout().addChild(_scene.background());

	if (markerLuaExists) {
		TeLayout *bg = _scene.markerGui().layout("background");
		app->frontLayout().addChild(bg);
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

	for (uint i = 0; i < _gameSounds.size(); i++) {
		if (_gameSounds[i]->retain())
			continue;
		_gameSounds[i]->stop();
		_gameSounds[i]->deleteLater();
		_gameSounds.remove_at(i);
		i--;
	}

	// Take a copy and clear the global list first, as deleting a sound can cause the
	// next sound to trigger (which might have been deleted already).
	Common::HashMap<Common::String, Common::Array<RandomSound *>> rsounds = _randomSounds;
	_randomSounds.clear();
	for (auto &randsoundlist : rsounds) {
		for (auto *randsound : randsoundlist._value) {
			delete randsound;
		}
		randsoundlist._value.clear();
	}

	_scene.initScroll();
	return true;
}

bool Game::isDocumentOpened() {
	return _documentsBrowser.gui1().layoutChecked("zoomed")->visible();
}

bool Game::isMoviePlaying() {
	TeButtonLayout *vidButton = _inGameGui.buttonLayout("videoBackgroundButton");
	if (vidButton)
		return vidButton->visible();
	return false;
}

static const char *DIALOG_IDS[20] = {
	"KFJ1", "KH", "KJ", "KL",
	"KO", "KS", "KCa", "KFE2",
	"KFE3", "KG", "KMa", "KP",
	"KR", "KCo", "KD", "KA",
	"KFJ", "KM", "KN", "KFM"};

bool Game::launchDialog(const Common::String &dname, uint param_2, const Common::String &charname,
						const Common::String &animfile, float animblend) {
	Application *app = g_engine->getApplication();
	const Common::String *locstring = app->loc().value(dname);

	if (!locstring)
		locstring = &dname;

	if (!locstring) // shouldn't happen?
		return false;

	Common::String dstring = *locstring;

	//
	// WORKAROUND: Fix some errors in en version strings
	//
	if (g_engine->getCore()->language() == "en") {
		if (dname == "C_OK_tel03_09") {
			size_t rloc = dstring.find("pleased to here");
			if (rloc != Common::String::npos)
				dstring.replace(rloc + 11, 4, "hear");
		} else if (dname == "B_diapo04") {
			size_t rloc = dstring.find("little imagination ? he draws");
			if (rloc != Common::String::npos)
				dstring.replace(rloc + 19, 1, "-");
		} else if (dname == "V_NK_lettre_01") {
			size_t rloc = dstring.find("you now ? my brother");
			if (rloc != Common::String::npos)
				dstring.replace(rloc + 8, 1, "-");
		}
	}

	for (uint i = 0; i < ARRAYSIZE(DIALOG_IDS); i++) {
		if (dname.contains(Common::String::format("_%s_", DIALOG_IDS[i])))
			_dialogsTold++;
	}

	const Common::String sndfile = dname + ".ogg";
	_dialog2.pushDialog(dname, dstring, sndfile, charname, animfile, animblend);
	return true;
}

void Game::leave(bool flag) {
	if (!_enteredFlag2)
		return;

	Application *app = g_engine->getApplication();

	deleteNoScale();
	_entered = false;
	_running = false;
	_notifier.unload();
	g_engine->getInputMgr()->_mouseLUpSignal.remove(this, &Game::onMouseClick);
	_question2.unload();
	TeLayout *cellbg = _inventory.cellphone()->gui().buttonLayout("background");
	if (cellbg)
		app->frontLayout().removeChild(cellbg);
	_inventory.cellphone()->leave();
	_dialog2.unload();
	_inventory.unload();
	_documentsBrowser.unload();
	_inventoryMenu.unload();
	_gui1.unload();
	_objectif.unload(); // not done in original, but should be.
	_scene.close();
	_forGui.unload();
	if (_scene._character) {
		_scene._character->deleteAllCallback();
		_scene._character->stop();
		_scene.unloadCharacter(_scene._character->_model->name());
	}

	for (auto &sound : _gameSounds) {
		delete sound;
	}
	_gameSounds.clear();

	for (auto &randsoundlist : _randomSounds) {
		for (auto *randsound : randsoundlist._value) {
			delete randsound;
		}
		randsoundlist._value.clear();
	}
	_randomSounds.clear();

	for (auto *hitobj : _gameHitObjects) {
		delete hitobj;
	}
	_gameHitObjects.clear();

	// TODO: clear Game::HitObject tree here?

	_luaContext.destroy();
	_running = false;
	_inGameGui.buttonLayoutChecked("skipVideoButton")->onMouseClickValidated().remove(this, &Game::onSkipVideoButtonValidated);
	_inGameGui.buttonLayoutChecked("videoBackgroundButton")->onMouseClickValidated().remove(this, &Game::onLockVideoButtonValidated);
	_inGameGui.spriteLayoutChecked("video")->_tiledSurfacePtr->_frameAnim.onFinished().remove(this, &Game::onSkipVideoButtonValidated);
	_inGameGui.buttonLayoutChecked("inventoryButton")->onMouseClickValidated().remove(this, &Game::onInventoryButtonValidated);
	_inGameGui.unload();
	_playedTimer.stop();
	_enteredFlag2 = false;

	app->lockCursor(false);
	app->lockCursorFromAction(false);
	// TODO: Set some inputmgr flag here?
	Character::animCacheFreeAll();
}

void Game::loadBackup(const Common::String &path) {
	if (_gameLoadState == 0) {
		_gameLoadState = 1;
		g_engine->getApplication()->showLoadingIcon(true);
		onFinishedLoadingBackup(path);
	}
}

void Game::loadUnlockedArtwork() {
	Common::ConfigManager::Domain *domain = ConfMan.getActiveDomain();
	for (auto &val : *domain) {
		if (val._key.substr(0, 8) == "artwork_") {
			_unlockedArtwork[val._key] = true;
		}
	}
}

bool Game::loadCharacter(const Common::String &name) {
	bool result = true;
	Character *character = _scene.character(name);
	if (!character) {
		result = _scene.loadCharacter(name);
		if (result) {
			character = _scene.character(name);
			assert(character);
			character->_onCharacterAnimFinishedSignal.remove(this, &Game::onCharacterAnimationFinished);
			character->_onCharacterAnimFinishedSignal.add(this, &Game::onCharacterAnimationFinished);
			// Syberia 2 uses a simplified callback here.
			// We have made onDisplacementPlayerFinished more like Syberia 1's onDisplacementFinished.
			if (g_engine->gameType() == TetraedgeEngine::kSyberia)
				character->onFinished().add(this, &Game::onDisplacementPlayerFinished);
			else
				character->onFinished().add(this, &Game::onDisplacementFinished);
		}
	}
	return result;
}

bool Game::loadPlayerCharacter(const Common::String &name) {
	bool result = _scene.loadPlayerCharacter(name);
	if (result) {
		_scene._character->_characterAnimPlayerFinishedSignal.remove(this, &Game::onCharacterAnimationPlayerFinished);
		_scene._character->_characterAnimPlayerFinishedSignal.add(this, &Game::onCharacterAnimationPlayerFinished);
		_scene._character->onFinished().remove(this, &Game::onDisplacementPlayerFinished);
		_scene._character->onFinished().add(this, &Game::onDisplacementPlayerFinished);
	} else {
		debug("failed to load player character %s", name.c_str());
	}
	return result;
}

bool Game::loadScene(const Common::String &name) {
	TeCore *core = g_engine->getCore();
	_gameEnterScript.load(core->findFile("scenes/OnGameEnter.lua"));
	_gameEnterScript.execute();
	Character *character = _scene._character;
	if (character && character->_model->visible()) {
		_sceneCharacterVisibleFromLoad = true;
	}
	return false;
}

bool Game::onAnswered(const Common::String &val) {
	_luaScript.execute("OnAnswered", val);
	return false;
}

bool Game::onCallNumber(Common::String val) {
	_luaScript.execute("OnCallNumber", val);
	return false;
}

bool Game::onCharacterAnimationFinished(const Common::String &charName) {
	if (!_scene._character)
		return false;

	if (g_engine->gameType() == TetraedgeEngine::kSyberia2) {
		Character *character = scene().character(charName);
		const Common::String curAnimName = character->curAnimName();
		if (character && (curAnimName == character->walkAnim(Character::WalkPart_EndD)
			|| curAnimName == character->walkAnim(Character::WalkPart_EndG))) {
			character->updatePosition(1.0);
			character->endMove();
		}
	}

	for (uint i = 0; i < _yieldedCallbacks.size(); i++) {
		YieldedCallback &cb = _yieldedCallbacks[i];
		if (cb._luaFnName == "OnCharacterAnimationFinished" && cb._luaParam == charName) {
			TeLuaThread *lua = cb._luaThread;
			_yieldedCallbacks.remove_at(i);
			if (lua) {
				lua->resume();
				return false;
			}
			break;
		}
	}
	_luaScript.execute("OnCharacterAnimationFinished", charName);
	return false;
}

bool Game::onCharacterAnimationPlayerFinished(const Common::String &anim) {
	if (_gameLoadState != 0)
		return false;

	bool callScripts = true;
	for (uint i = 0; i < _yieldedCallbacks.size(); i++) {
		YieldedCallback &cb = _yieldedCallbacks[i];
		// Yes, even Syberia2 checks for Kate here..
		if (cb._luaFnName == "OnCharacterAnimationFinished" && cb._luaParam == "Kate") {
			TeLuaThread *lua = cb._luaThread;
			_yieldedCallbacks.remove_at(i);
			if (lua) {
				lua->resume();
				callScripts = false;
			}
			break;
		}
	}
	if (callScripts) {
		if (g_engine->gameType() == TetraedgeEngine::kSyberia)
			_luaScript.execute("OnCharacterAnimationFinished", "Kate");
		else
			_luaScript.execute("OnCharacterAnimationPlayerFinished", anim);
		_luaScript.execute("OnCellCharacterAnimationPlayerFinished", anim);
	}

	Character *character = scene()._character;
	assert(character);
	// Note: the above callbacks can change the anim,
	// so we have to fetch this *after* them.
	const Common::String curAnimName = character->curAnimName();
	if (_currentScene == _someSceneName) {
		if (curAnimName == character->walkAnim(Character::WalkPart_Start)
			|| curAnimName == character->walkAnim(Character::WalkPart_Loop)
			|| curAnimName == character->walkAnim(Character::WalkPart_EndD)
			|| curAnimName == character->walkAnim(Character::WalkPart_EndG))
			character->stop();
	} else {
		if (!_sceneCharacterVisibleFromLoad && curAnimName == character->walkAnim(Character::WalkPart_Start)) {
			character->setAnimation(character->walkAnim(Character::WalkPart_Loop), true);
			return false;
		}
		if (curAnimName == character->walkAnim(Character::WalkPart_EndD)
			|| curAnimName == character->walkAnim(Character::WalkPart_EndG)) {
			character->updatePosition(1.0);
			character->endMove();
			// endMove can result in callbacks that change the animation. check again.
			if (character->curAnimName() == character->walkAnim(Character::WalkPart_EndD)
				|| character->curAnimName() == character->walkAnim(Character::WalkPart_EndG))
				character->setAnimation(character->characterSettings()._idleAnimFileName, true);
		}
	}

	return false;
}

bool Game::onDialogFinished(const Common::String &val) {
	for (uint i = 0; i < _yieldedCallbacks.size(); i++) {
		YieldedCallback &cb = _yieldedCallbacks[i];
		if (cb._luaFnName == "OnDialogFinished" && cb._luaParam == val) {
			TeLuaThread *lua = cb._luaThread;
			_yieldedCallbacks.remove_at(i);
			if (lua) {
				lua->resume();
				return false;
			}
			break;
		}
	}

	_luaScript.execute("OnDialogFinished", val);
	_luaScript.execute("OnCellDialogFinished", val);
	return false;
}

// This is the Syberia 2 version of this function, not used in Syb 1.
// Syb 1 uses a function much more like onDisplacementPlayerFinished below.
bool Game::onDisplacementFinished() {
	TeLuaThread *thread = nullptr;
	for (uint i = 0; i < _yieldedCallbacks.size(); i++) {
		YieldedCallback &cb = _yieldedCallbacks[i];
		if (cb._luaFnName == "OnDisplacementFinished") {
			thread = cb._luaThread;
			_yieldedCallbacks.remove_at(i);
			break;
		}
	}
	if (thread) {
		thread->resume();
	} else {
		_luaScript.execute("OnDisplacementFinished");
	}
	return false;
}

bool Game::onDisplacementPlayerFinished() {
	_sceneCharacterVisibleFromLoad = true;
	assert(_scene._character);
	_scene._character->stop();
	_scene._character->walkMode("Walk");
	_scene._character->setAnimation(_scene._character->characterSettings()._idleAnimFileName, true);

	if (_isCharacterWalking) {
		_isCharacterWalking = false;
		_isCharacterIdle = true;
	} else {
		_isCharacterIdle = false;
	}

	TeLuaThread *thread = nullptr;

	const char *cbName = (g_engine->gameType() == TetraedgeEngine::kSyberia ?
						"OnDisplacementFinished" : "OnDisplacementPlayerFinished");

	for (uint i = 0; i < _yieldedCallbacks.size(); i++) {
		YieldedCallback &cb = _yieldedCallbacks[i];
		if (cb._luaFnName == cbName) {
			thread = cb._luaThread;
			_yieldedCallbacks.remove_at(i);
			break;
		}
	}
	if (thread) {
		thread->resume();
	} else {
		_luaScript.execute(cbName);
	}
	return false;
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
	TeButtonLayout *btn = _inGameGui.buttonLayoutChecked("skipVideoButton");
	btn->setVisible(!btn->visible());
	return true;
}

bool Game::onMarkersVisible(TeCheckboxLayout::State state) {
	_markersVisible = (state == TeCheckboxLayout::CheckboxStateActive);
	showMarkers(state == TeCheckboxLayout::CheckboxStateActive);
	return false;
}

bool Game::onMouseClick(const Common::Point &pt) {
	Application *app = g_engine->getApplication();

	if (app->isFading())
		return true;

	// In case we capture a click during a video or dialog (shouldn't happen?)
	if (!_scene.currentCamera() || _dialog2.isDialogPlaying() || _question2.isEntered())
		return false;

	_posPlayer = TeVector3f32(-1.0f, -1.0f, -1.0f);
	if (_previousMousePos == TeVector2s32(-1, -1)) {
		_previousMousePos = pt;
	} else {
		const TeVector3f32 winSize = app->getMainWindow().size();
		const TeVector2s32 prevMousePos = _previousMousePos;
		_previousMousePos = pt;
		float xdist = (pt.x - prevMousePos._x) / winSize.x();
		float ydist = (pt.y - prevMousePos._y) / winSize.y();
		float sqrdist = xdist * xdist + ydist * ydist;
		if (sqrdist < 0.0001 && (!_walkTimer.running() || _walkTimer.timeElapsed() > 300000.0
						 || (_scene._character && _scene._character->walkModeStr() != "Walk"))) {
			return false;
			// Normal walk click
		}
	}

	if (!app->frontLayout().isMouseIn(pt))
		return false;

	Common::String nearestMeshName = "None";
	TeIntrusivePtr<TeCamera> curCamera = _scene.currentCamera();
	Common::Array<TePickMesh2*> pickMeshes = _scene.clickMeshes();
	TePickMesh2 *nearestMesh = TeFreeMoveZone::findNearestMesh(curCamera, pt, pickMeshes, nullptr, false);
	if (nearestMesh) {
		nearestMeshName = nearestMesh->name();
		debug("Game::onMouseClick: Click near mesh %s", nearestMeshName.c_str());
		_lastCharMoveMousePos = TeVector2s32();
	}

	Character *character = _scene._character;
	if (app->isLockCursor() || _movePlayerCharacterDisabled || !character)
		return false;

	const Common::String &charAnim = character->curAnimName();

	if (charAnim == character->characterSettings()._idleAnimFileName
		|| charAnim == character->walkAnim(Character::WalkPart_Start)
		|| charAnim == character->walkAnim(Character::WalkPart_Loop)
		|| charAnim == character->walkAnim(Character::WalkPart_EndD)
		|| charAnim == character->walkAnim(Character::WalkPart_EndG)) {
		_luaScript.execute("On");
		if (!_scene.isObjectBlocking(nearestMeshName) && character->freeMoveZone()) {
			const TeVector3f32 charPos = character->_model->position();
			TeIntrusivePtr<TeBezierCurve> curve = character->freeMoveZone()->curve(charPos, pt, 8.0, true);
			if (!curve)
				return false;

			_scene.setCurve(curve);
			character->setCurveStartLocation(TeVector3f32());
			if (curve->controlPoints().size() == 1) {
				character->endMove();
			} else {
				if (!_walkTimer.running() || _walkTimer.timeElapsed() > 300000 || !_runModeEnabled) {
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
					character->setAnimation(character->walkAnim(Character::WalkPart_Start), false);
				}
				character->walkTo(1.0, false);
				_sceneCharacterVisibleFromLoad = false;
				_lastCharMoveMousePos = pt;
			}
		}
		// FIXME: The original never checks for empty/null curve here..
		// but it seems our curve can possibly become null.
		if (_scene.curve() && _scene.curve()->length()) {
			TeVector3f32 lastPoint = _scene.curve()->controlPoints().back();
			character->setAnimation(character->walkAnim(Character::WalkPart_Loop), true);
			character->walkTo(1.0, false);
			_isCharacterWalking = true;
			_posPlayer = lastPoint;
		}
	}

	// Note: charAnim above may no longer be valid as anim may have changed.
	if (_sceneCharacterVisibleFromLoad || (character->curAnimName() == character->characterSettings()._idleAnimFileName)) {
		_lastCharMoveMousePos = TeVector2s32();
		_movePlayerCharacterDisabled = false;
		_isCharacterIdle = true;
		_isCharacterWalking = false;
		if (nearestMesh) {
			character->stop();
			_luaScript.execute("OnWarpObjectHit", nearestMeshName);
		}
	}

	return false;
}

#ifdef TETRAEDGE_ENABLE_CUSTOM_CURSOR_CHECKS
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
#endif

bool Game::onMouseMove() {
	if (!_entered)
		return false;

	const Common::String DEFAULT_CURSOR("pictures/cursor.png");

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

#ifdef TETRAEDGE_ENABLE_CUSTOM_CURSOR_CHECKS
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
		for (uint i = 0; i < bglayout->childCount(); i++) {
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
#endif
	return false;
}

bool Game::onSkipVideoButtonValidated() {
	TeSpriteLayout *sprite = _inGameGui.spriteLayoutChecked("video");
	TeButtonLayout *btn = _inGameGui.buttonLayoutChecked("videoBackgroundButton");
	sprite->stop();
	btn->setVisible(false);
	return false;
}

bool Game::onVideoFinished() {
	if (!_inGameGui.loaded()) {
		_music.stop();
		return false;
	}

	Application *app = g_engine->getApplication();

	app->captureFade();

	TeSpriteLayout *video = _inGameGui.spriteLayoutChecked("video");
	Common::String vidPath = video->_tiledSurfacePtr->loadedPath();
	TeButtonLayout *btn = _inGameGui.buttonLayoutChecked("videoBackgroundButton");
	btn->setVisible(false);
	btn = _inGameGui.buttonLayoutChecked("skipVideoButton");
	btn->setVisible(false);
	video->setVisible(false);
	_music.stop();
	_running = true;
	bool resumed = false;
	for (uint i = 0; i < _yieldedCallbacks.size(); i++) {
		YieldedCallback &cb = _yieldedCallbacks[i];
		if (cb._luaFnName == "OnMovieFinished" && cb._luaParam == vidPath) {
			TeLuaThread *lua = cb._luaThread;
			_yieldedCallbacks.remove_at(i);
			resumed = true;
			if (lua)
				lua->resume();
			break;
		}
	}
	if (!resumed)
		_luaScript.execute("OnMovieFinished", vidPath);
	app->fade();
	return false;
}

void Game::pauseMovie() {
	_music.pause();
	TeSpriteLayout *sprite = _inGameGui.spriteLayoutChecked("video");
	sprite->pause();
}

bool Game::playMovie(const Common::String &vidPath, const Common::String &musicPath, float volume /* = 1.0f */) {
	Application *app = g_engine->getApplication();
	app->captureFade();
	TeButtonLayout *videoBackgroundButton = _inGameGui.buttonLayoutChecked("videoBackgroundButton");
	videoBackgroundButton->setVisible(true);

	TeButtonLayout *skipVideoButton = _inGameGui.buttonLayoutChecked("skipVideoButton");
	skipVideoButton->setVisible(false);

	TeMusic &music = app->music();
	music.stop();
	music.setChannelName("video");
	music.repeat(false);
	music.volume(volume);
	music.load(musicPath);

	_running = false;

	TeSpriteLayout *videoSpriteLayout = _inGameGui.spriteLayoutChecked("video");
	if (videoSpriteLayout->load(vidPath)) {
		uint vidHeight = videoSpriteLayout->_tiledSurfacePtr->codec()->height();
		uint vidWidth = videoSpriteLayout->_tiledSurfacePtr->codec()->width();

		// Note: Not in original, but original incorrectly stretches
		// videos that should be 16:9.
		if (ConfMan.getBool("correct_movie_aspect")) {
			videoSpriteLayout->setRatioMode(TeILayout::RATIO_MODE_LETTERBOX);
			videoSpriteLayout->setRatio((float)vidWidth / vidHeight);
			videoSpriteLayout->updateSize();
		}

		videoSpriteLayout->setVisible(true);
		music.play();
		videoSpriteLayout->play();

		// Stop the movie and sound early for testing if skip_videos set
		if (ConfMan.getBool("skip_videos")) {
			videoSpriteLayout->_tiledSurfacePtr->_frameAnim.setNbFrames(10);
			music.stop();
		}

		app->fade();
		return true;
	} else {
		warning("Failed to load movie %s", vidPath.c_str());
		return false;
	}
}

void Game::playRandomSound(const Common::String &name) {
	if (!_randomSounds.contains(name)) {
		warning("Game::playRandomSound: can't find sound list %s", name.c_str());
		return;
	}

	if (!_randomSoundFinished) {
		_randomSoundTimer.start();
		int r = _randomSource.getRandomNumber(RAND_MAX);
		float f = (r + 1 + (r / 100) * -100);
		uint64 time = 1000000;
		if (f >= 25.0) {
			time = f * 45000.0;
		}
		_randomSoundTimer.setAlarmIn(time);
		_randomSoundTimer.alarmSignal().remove(_randomSound, &RandomSound::onSoundFinished);
		_randomSoundTimer.alarmSignal().add(_randomSound, &RandomSound::onSoundFinished);
		_randomSound->_name = name;
	} else {
		Common::Array<RandomSound *> &sndlist = _randomSounds[name];
		float total = 0.0;
		for (auto *snd : sndlist) {
			total += snd->_f1;
		}
		int r = _randomSource.getRandomNumber(RAND_MAX);
		float total2 = 0.0;
		uint i = 0;
		while (i < sndlist.size() && total2 <= r * 4.656613e-10 * total) {
			total2 += sndlist[i]->_f1;
			i++;
		}
		assert(i > 0);
		i--;
		RandomSound *sound = sndlist[i];
		sound->_music.volume(sound->_volume);
		sound->_music.onStopSignal().remove(sound, &RandomSound::onSoundFinished);
		sound->_music.onStopSignal().add(sound, &RandomSound::onSoundFinished);
		sound->_music.load(sound->_path.toString());
		sound->_music.repeat(false);
		sound->_music.play();
		// TODO: set a flag that it's playing?
	}
}

void Game::playSound(const Common::String &name, int repeats, float volume) {
	Game *game = g_engine->getGame();

	assert(repeats == 1 || repeats == -1);
	if (repeats == 1) {
		GameSound *sound = new GameSound();
		sound->setName(name);
		sound->setChannelName("sfx");
		sound->repeat(false);
		sound->load(name);
		sound->volume(volume);
		if (!sound->play()) {
			game->luaScript().execute("OnFreeSoundFinished", name);
			game->luaScript().execute("OnCellFreeSoundFinished", name);
			// Note: original leaks sound here, don't do that..
			delete sound;
		} else {
			sound->onStopSignal().add(sound, &GameSound::onSoundStopped);
			sound->setRetain(true);
			_gameSounds.push_back(sound);
		}
	} else if (repeats == -1) {
		for (GameSound *snd : _gameSounds) {
			const Common::String accessName = snd->getAccessName();
			if (accessName == name) {
				snd->setRetain(true);
				return;
			}
		}

		GameSound *sound = new GameSound();
		sound->setChannelName("sfx");
		sound->load(name);
		sound->volume(volume);
		if (!sound->play()) {
			game->luaScript().execute("OnFreeSoundFinished", name);
			game->luaScript().execute("OnCellFreeSoundFinished", name);
			delete sound;
		} else {
			sound->setRetain(true);
			_gameSounds.push_back(sound);
		}
	}
}

void Game::removeNoScale2Child(TeLayout *layout) {
	if (!_noScaleLayout2 || !layout)
		return;
	_noScaleLayout2->removeChild(layout);
}

void Game::removeNoScale2Children() {
	if (!_noScaleLayout2)
		return;

	TeLayout *vidbtn = _inGameGui.layout("videoButtonLayout");
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
	app->frontLayout().removeChild(&_dialog2);
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
	_inGameGui.spriteLayout("video")->play();
}

void Game::saveBackup(const Common::String &saveName) {
	Application *app = g_engine->getApplication();
	app->showLoadingIcon(true);
	if (saveName == "save.xml")
		g_engine->saveAutosaveIfEnabled();
	else
		warning("TODO: Implemet Game::saveBackup %s", saveName.c_str());
	app->showLoadingIcon(false);
}

bool Game::setBackground(const Common::String &name) {
	return _scene.changeBackground(name);
}

void Game::setCurrentObjectSprite(const Common::String &spritePath) {
	TeSpriteLayout *currentSprite = _inGameGui.spriteLayout("currentObjectSprite");
	if (currentSprite) {
		if (spritePath.empty())
			currentSprite->unload();
		else
			currentSprite->load(spritePath);
	}
}

// Note: The naming of this function is bad, but follows the original..
// we really set the visibility to the *opposite* of the parameter.
bool Game::showMarkers(bool val) {
	if (!_forGui.loaded())
		return false;

	TeLayout *bg = _forGui.layoutChecked("background");
	for (int i = 0; i < bg->childCount(); i++) {
		const InGameScene::TeMarker *marker = _scene.findMarker(bg->child(i)->name());
		if (marker)
			bg->child(i)->setVisible(!val);
	}
	return false;
}

bool Game::startAnimation(const Common::String &animName, int loopcount, bool reversed) {
	TeSpriteLayout *layout = _scene.bgGui().spriteLayout(animName);
	if (layout) {
		layout->_tiledSurfacePtr->_frameAnim.setLoopCount(loopcount);
		layout->_tiledSurfacePtr->_frameAnim.setReversed(reversed);
		layout->_tiledSurfacePtr->play();
	}
	return layout != nullptr;
}

void Game::stopSound(const Common::String &name) {
	for (uint i = 0; i < _gameSounds.size(); i++) {
		GameSound *sound = _gameSounds[i];
		if (sound->rawPath() == name) {
			sound->stop();
			sound->deleteLater();
			_gameSounds.remove_at(i);
			break;
		}
	}
	g_engine->getSoundManager()->stopFreeSound(name);
}

Common::Error Game::syncGame(Common::Serializer &s) {
	Application *app = g_engine->getApplication();

	// TODO: should be an error before testing.
	//if (!s.syncVersion(1))
	//	error("Save game version too new: %d", s.getVersion());

	inventory().syncState(s);
	inventory().cellphone()->syncState(s);
	// dialog2().syncState(s); // game saves this here, but doesn't actually save anything
	_luaContext.syncState(s);
	s.syncString(_currentZone);
	s.syncString(_currentScene);
	s.syncAsUint32LE(app->difficulty());
	uint64 elapsed = _playedTimer.timeFromLastTimeElapsed(); // TODO: + _loadedPlayTime;
	s.syncAsDoubleLE(elapsed);
	_playedTimer.stop();
	_playedTimer.start();
	s.syncAsUint32LE(_objectsTakenVal);
	for (uint i = 0; i < ARRAYSIZE(_objectsTakenBits); i++)
		s.syncAsByte(_objectsTakenBits[i]);
	s.syncAsUint32LE(_dialogsTold);
	s.syncString(_prevSceneName);
	Common::String mpath = _music.rawPath();
	s.syncString(mpath);
	if (s.isLoading())
		_music.load(mpath);
	s.syncString(_scene._character->walkModeStr());
	s.syncAsByte(_firstInventory);
	s.syncAsByte(app->tutoActivated());

	app->showLoadingIcon(false);
	return Common::kNoError;
}

bool Game::unloadCharacter(const Common::String &charname) {
	Character *c = _scene.character(charname);
	if (!c)
		return false;

	for (uint i = 0; i < _scene.models().size(); i++) {
		if (_scene.models()[i] == c->_model) {
			_scene.models().remove_at(i);
			break;
		}
	}
	c->_onCharacterAnimFinishedSignal.remove(this, &Game::onCharacterAnimationFinished);
	c->removeAnim();
	// Syberia 2 uses a simplified callback here.
	// We have made onDisplacementPlayerFinished more like Syberia 1's onDisplacementPlayerFinished.
	if (g_engine->gameType() == TetraedgeEngine::kSyberia)
		c->onFinished().remove(this, &Game::onDisplacementPlayerFinished);
	else
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

bool Game::unloadPlayerCharacter(const Common::String &charname) {
	Character *c = _scene.character(charname);
	if (c) {
		c->_onCharacterAnimFinishedSignal.remove(this, &Game::onCharacterAnimationPlayerFinished);
		c->onFinished().remove(this, &Game::onDisplacementPlayerFinished);
		_scene.unloadCharacter(charname);
	}
	return c != nullptr;
}

void Game::update() {
	if (!_entered)
		return;

	TeITextLayout *debugTimeTextLayout = _inGameGui.textLayout("debugTimeText1");
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
				app->lockCursor(false);
		}

		TeButtonLayout *invbtn = _inGameGui.buttonLayout("inventoryButton");
		if (invbtn)
			invbtn->setVisible(!app->isLockCursor() && !_dialog2.isDialogPlaying());

		Character *player = _scene._character;
		if (player) {
			TeIntrusivePtr<TeModel> model = player->_model;
			bool modelVisible = model->visible();
			if (model->anim())
				player->permanentUpdate();
			if (modelVisible) {
				if (player->needsSomeUpdate()) {
					Game *game = g_engine->getGame();
					game->_sceneCharacterVisibleFromLoad = false;
					TeVector3f32 charPos = player->_model->position();
					const Common::String charName = player->_model->name();
					TeFreeMoveZone *zone = _scene.pathZone(player->freeMoveZoneName());
					if (zone) {
						TeIntrusivePtr<TeCamera> cam = _scene.currentCamera();
						zone->setCamera(cam, false);
						player->setFreeMoveZone(zone);
						_scene.setPositionCharacter(charName, player->freeMoveZoneName(), charPos);
						TeIntrusivePtr<TeBezierCurve> curve = zone->curve(model->position(), player->positionCharacter());
						_scene.setCurve(curve);
						player->setCurveStartLocation(TeVector3f32(0, 0, 0));
						player->placeOnCurve(curve);
						player->setCurveOffset(0.0f);
						player->setAnimation(player->walkAnim(Character::WalkPart_Start), false);
						player->walkTo(1.0f, false);
						_isCharacterWalking = true;
					}
					player->setNeedsSomeUpdate(false);
				}

				const Common::String &charAnim = _scene._character->curAnimName();
				bool unlockCursor = (charAnim == _scene._character->walkAnim(Character::WalkPart_Start) ||
						charAnim == _scene._character->walkAnim(Character::WalkPart_Loop) ||
						charAnim == _scene._character->walkAnim(Character::WalkPart_EndD) ||
						charAnim == _scene._character->walkAnim(Character::WalkPart_EndG) ||
						charAnim == _scene._character->characterSettings()._idleAnimFileName);
				app->lockCursor(!unlockCursor);
			}
		}

		Common::Array<Character *> characters = _scene._characters;
		for (Character *c : characters) {
			if (c->_model->anim())
				c->permanentUpdate();
		}

		Common::Point mousePos = g_engine->getInputMgr()->lastMousePos();
		if (_lastUpdateMousePos != mousePos) {
			onMouseMove();
			_lastUpdateMousePos = mousePos;
		}
		if (_saveRequested) {
			_saveRequested = false;
			saveBackup("save.xml");
		}

		_luaScript.execute("Update");
		_objectif.update();
		_scene.update();
	} else {
		TeSoundManager *soundmgr = g_engine->getSoundManager();
		// Take a copy in case the active music objects changes as we iterate.
		Common::Array<TeMusic *> musics = soundmgr->musics();
		for (TeMusic *music : musics) {
			const Common::String &chanName = music->channelName();
			if (chanName != "music" && chanName != "sfx" && chanName != "dialog")
				music->stop();
		}
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
	_game->_isCharacterIdle = true;
	return false;
}

bool Game::HitObject::onUp() {
	// debug("Game::HitObject mouseup: %s", _name.c_str());
	_game->luaScript().execute("OnButtonUp", _name);
	_game->_isCharacterIdle = true;
	return false;
}

bool Game::HitObject::onValidated() {
	if (!g_engine->getApplication()->isLockCursor()) {
		_game->luaScript().execute("OnWarpObjectHit", _name);
		_game->_isCharacterIdle = true;
	}
	return false;
}

bool Game::RandomSound::onSoundFinished() {
	Game *game = g_engine->getGame();
	_music.onStopSignal().remove(this, &RandomSound::onSoundFinished);
	if (game->_randomSoundFinished) {
		game->_randomSoundFinished = false;
	} else {
		game->_randomSoundFinished = true;
		game->_randomSound->_music.onStopSignal().remove(this, &RandomSound::onSoundFinished);
		game->_randomSoundTimer.stop();
	}
	game->playRandomSound(_name);
	return false;
}


} // end namespace Tetraedge
