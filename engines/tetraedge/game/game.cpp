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
#include "tetraedge/game/cellphone.h"
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
_running(false), _noScaleLayout2(nullptr), _luaShowOwnerError(false),
_firstInventory(true), _dialogsTold(0) {
	for (int i = 0; i < NUM_OBJECTS_TAKEN_IDS; i++) {
		_objectsTakenBits[i] = false;
	}
	_dialog2.onAnimationDownFinishedSignal().add(this, &Game::onDialogFinished);
	_question2.onAnswerSignal().add(this, &Game::onAnswered);
}

Game::~Game() {
}

/*static*/ const char *Game::OBJECTS_TAKEN_IDS[5] = {
	"BCylindreBarr",
	"VCylindreMusique",
	"VCylindreVal",
	"CCylindreCite",
	"VPoupeeMammouth"
};

void Game::addNoScale2Child(TeLayout *layout) {
	if (!layout)
		return;

	if (!g_engine->gameIsAmerzone()) {
		if (_noScaleLayout2) {
			_noScaleLayout2->addChild(layout);
		}
	} else {
		// No _noScaleLayout in Amerzone, just use front
		g_engine->getApplication()->frontLayout().addChild(layout);
	}
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

bool Game::isDocumentOpened() {
	return _documentsBrowser.gui().layoutChecked("zoomed")->visible();
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

bool Game::onAnswered(const Common::String &val) {
	_luaScript.execute("OnAnswered", val);
	return false;
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

bool Game::onMouseMove(const Common::Point &pt) {
	if (!_entered)
		return false;

	Application *app = g_engine->getApplication();

	if (app->isLockCursor()) {
		app->mouseCursorLayout().load(app->defaultCursor());
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
			app->mouseCursorLayout().load(app->defaultCursor());
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
	sprite->stop();
	TeButtonLayout *btn = _inGameGui.buttonLayout("videoBackgroundButton");
	if (btn)
		btn->setVisible(false);
	return false;
}

/* Unused
void Game::pauseMovie() {
	_videoMusic.pause();
	TeSpriteLayout *sprite = _inGameGui.spriteLayoutChecked("video");
	sprite->pause();
}
*/

bool Game::playMovie(const Common::String &vidPath, const Common::String &musicPath, float volume /* = 1.0f */) {
	Application *app = g_engine->getApplication();
	app->captureFade();
	TeButtonLayout *videoBackgroundButton = _inGameGui.buttonLayoutChecked("videoBackgroundButton");
	videoBackgroundButton->setVisible(true);

	TeButtonLayout *skipVideoButton = _inGameGui.buttonLayoutChecked("skipVideoButton");
	skipVideoButton->setVisible(false);

	app->music().stop();
	_videoMusic.stop();
	_videoMusic.setChannelName("video");
	_videoMusic.repeat(false);
	_videoMusic.volume(volume);
	_videoMusic.load(musicPath);

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
		_videoMusic.play();
		videoSpriteLayout->play();

		// Stop the movie and sound early for testing if skip_videos set
		if (ConfMan.getBool("skip_videos")) {
			videoSpriteLayout->_tiledSurfacePtr->_frameAnim.setNbFrames(10);
			_videoMusic.stop();
		}

		app->fade();
		return true;
	} else {
		warning("Failed to load movie %s", vidPath.c_str());
		return false;
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
	if (!layout)
		return;

	if (!g_engine->gameIsAmerzone()) {
		if (_noScaleLayout2)
			_noScaleLayout2->removeChild(layout);
	} else {
		// No _noScaleLayout in Amerzone, just use front
		g_engine->getApplication()->frontLayout().removeChild(layout);
	}
}

void Game::resumeMovie() {
	_videoMusic.play();
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

	//
	// Note: Early versions of this code didn't sync a version number so it was
	// the inventory item count.  We use a large version number which would never
	// be the inventory count.
	//
	if (!s.syncVersion(1000))
		error("Save game version too new: %d", s.getVersion());

	if (s.getVersion() < 1000) {
		warning("Loading as old un-versioned save data");
		inventory().syncStateWithCount(s, s.getVersion());
	} else {
		inventory().syncState(s);
	}

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
	Common::String mpath = _videoMusic.rawPath();
	s.syncString(mpath);
	if (s.isLoading())
		_videoMusic.load(mpath);
	s.syncString(_scene._character->walkModeStr());
	s.syncAsByte(_firstInventory);
	s.syncAsByte(app->tutoActivated());

	app->showLoadingIcon(false);
	return Common::kNoError;
}


} // end namespace Tetraedge
