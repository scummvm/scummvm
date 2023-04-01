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

#include "common/config-manager.h"
#include "common/system.h"
#include "common/events.h"
#include "common/savefile.h"

#include "tetraedge/tetraedge.h"
#include "tetraedge/game/confirm.h"
#include "tetraedge/game/game.h"
#include "tetraedge/game/main_menu.h"
#include "tetraedge/game/application.h"

#include "tetraedge/te/te_button_layout.h"
#include "tetraedge/te/te_sprite_layout.h"
#include "tetraedge/te/te_text_layout.h"
#include "tetraedge/te/te_music.h"


namespace Tetraedge {

static const char *LAST_SAVE_CONF = "last_save_slot";

MainMenu::MainMenu() : _entered(false), _confirmingTuto(false) {
	_newGameConfirm.onButtonYesSignal().add(this, &MainMenu::onNewGameConfirmed);
	_tutoConfirm.onButtonYesSignal().add(this, &MainMenu::onActivedTuto);
	_tutoConfirm.onButtonNoSignal().add(this, &MainMenu::onDisabledTuto);
	_quitConfirm.onButtonYesSignal().add(this, &MainMenu::onQuit);
	onFacebookLoggedSignal.add(this, &MainMenu::onFacebookLogged);
}

void MainMenu::enter() {
	Application *app = g_engine->getApplication();

	if (g_engine->gameType() == TetraedgeEngine::kSyberia2) {
		app->backLayout().setRatioMode(TeILayout::RATIO_MODE_LETTERBOX);
		app->backLayout().setRatio(1.333333f);
		app->frontLayout().setRatioMode(TeILayout::RATIO_MODE_LETTERBOX);
		app->frontLayout().setRatio(1.333333f);
	}

	TeSpriteLayout &appSpriteLayout = app->appSpriteLayout();
	appSpriteLayout.setVisible(true);
	if (!appSpriteLayout._tiledSurfacePtr->_frameAnim._runTimer.running()) {
		appSpriteLayout.load("menus/menu.ogv");
		appSpriteLayout._tiledSurfacePtr->_frameAnim.setLoopCount(-1);
		appSpriteLayout._tiledSurfacePtr->play();
	}
	app->captureFade();

	_entered = true;
	const char *luaFile = (g_engine->gameType() == TetraedgeEngine::kAmerzone ? "GUI/MainMenu.lua" : "menus/mainMenu/mainMenu.lua");
	load(luaFile);

	TeLayout *menuLayout = layoutChecked("menu");
	appSpriteLayout.addChild(menuLayout);

	//
	// WORKAROUND: This is set to PanScan ratio 1.0, but with our code
	// but that shrinks it down to pillarboxed.  Force back to full size.
	//
	TeLayout *background;
	if (layout("background"))
		background = layoutChecked("background");
	else
		background = dynamic_cast<TeLayout *>(menuLayout->child(0));
	assert(background);
	background->setRatioMode(TeILayout::RATIO_MODE_NONE);


	app->mouseCursorLayout().setVisible(true);
	app->mouseCursorLayout().load(app->defaultCursor());

	TeMusic &music = app->music();
	if (music.isPlaying()) {
		// TODO: something here??
	}
	music.load(value("musicPath").toString());
	music.play();
	music.volume(1.0f);

	TeButtonLayout *newGameButton = buttonLayout("newGameButton");
	if (newGameButton)
		newGameButton->onMouseClickValidated().add(this, &MainMenu::onNewGameButtonValidated);

	TeButtonLayout *continueGameButton = buttonLayout("continueGameButton");
	if (continueGameButton) {
		continueGameButton->onMouseClickValidated().add(this, &MainMenu::onContinueGameButtonValidated);
		continueGameButton->setEnable(ConfMan.hasKey(LAST_SAVE_CONF));
	}

	TeButtonLayout *loadGameButton = buttonLayout("loadGameButton");
	if (loadGameButton)
		loadGameButton->onMouseClickValidated().add(this, &MainMenu::onLoadGameButtonValidated);

	TeButtonLayout *optionsButton = buttonLayout("optionsButton");
	if (optionsButton)
		optionsButton->onMouseClickValidated().add(this, &MainMenu::onOptionsButtonValidated);

	TeButtonLayout *galleryButton = buttonLayout("galleryButton");
	if (galleryButton)
		galleryButton->onMouseClickValidated().add(this, &MainMenu::onGalleryButtonValidated);

	TeButtonLayout *quitButton = buttonLayout("quitButton");
	if (quitButton)
		quitButton->onMouseClickValidated().add(this, &MainMenu::onQuitButtonValidated);

	// TODO: confirmation (menus/confirm/confirmNotSound.lua)
	// if TeSoundManager is not valid.

	_confirmingTuto = false;
	TeLayout *panel = layout("panel");

	if (panel) {
		const Common::String panelTypoVal = value("panelTypo").toString();
		for (auto *child : panel->childList()) {
			TeTextLayout *childText = dynamic_cast<TeTextLayout *>(child);
			if (!childText)
				continue;
			childText->setName(panelTypoVal + childText->name());
		}
	}
	setCenterButtonsVisibility(true);
	TeITextLayout *versionNum = textLayout("versionNumber");
	if (versionNum) {
		const Common::String versionSectionStr("<section style=\"left\" /><color r=\"255\" g=\"255\" b=\"255\"/><font file=\"Common/Fonts/arial.ttf\" size=\"12\" />");
		versionNum->setText(versionSectionStr + app->getVersionString());
	}

	// Skip the menu if we are loading.
	Game *game = g_engine->getGame();
	if (game->hasLoadName() || ConfMan.getBool("skip_mainmenu")) {
		onNewGameConfirmed();
	}
}

void MainMenu::leave() {
	if (!_entered)
		return;

	Application	*app = g_engine->getApplication();
	app->captureFade();
	Game *game = g_engine->getGame();
	game->stopSound("sounds/Ambiances/b_automatebike.ogg");
	game->stopSound("sounds/Ambiances/b_engrenagebg.ogg");
	TeLuaGUI::unload();
	app->fade();
	_entered= false;
}

bool MainMenu::deleteFile(const Common::String &fname) {
	error("TODO: Implement MainMenu::deleteFile");
}

bool MainMenu::onActivedTuto() {
	Application *app = g_engine->getApplication();
	app->setTutoActivated(true);
	g_engine->getGame()->_firstInventory = true;
	app->captureFade();
	leave();
	app->startGame(true, 1);
	app->fade();
	return false;
}

bool MainMenu::onBFGRateIt2ButtonValidated() {
	error("TODO: Implement MainMenu::onBFGRateIt2ButtonValidated");
}

bool MainMenu::onBFGRateItButtonValidated() {
	error("TODO: Implement MainMenu::onBFGRateItButtonValidated");
}

bool MainMenu::onBFGRateItQuitButtonValidated() {
	error("TODO: Implement MainMenu::onBFGRateItQuitButtonValidated");
}

bool MainMenu::onBFGUnlockGameButtonValidated() {
	error("TODO: Implement MainMenu::onBFGUnlockGameButtonValidated");
}

void MainMenu::tryDisableButton(const Common::String &btnName) {
	TeButtonLayout *button = buttonLayout(btnName);
	if (button)
		button->setEnable(false);
}

bool MainMenu::onContinueGameButtonValidated() {
	Application *app = g_engine->getApplication();
	int lastSave = ConfMan.hasKey(LAST_SAVE_CONF) ? ConfMan.getInt(LAST_SAVE_CONF) : -1;
	if (lastSave >= 0)
		g_engine->loadGameState(lastSave);

	tryDisableButton("newGameButton");
	tryDisableButton("continueGameButton");
	tryDisableButton("loadGameButton");
	tryDisableButton("optionsButton");
	tryDisableButton("galleryButton");
	tryDisableButton("quitButton");

	if (_confirmingTuto)
		return false;

	app->captureFade();
	leave();
	app->startGame(false, 1);
	app->fade();

	if (g_engine->gameType() == TetraedgeEngine::kSyberia2) {
		// TODO: This should probably happen on direct game load too,
		// as it bypasses this code path which always gets called in
		// the original?
		if (app->ratioStretched()) {
			app->backLayout().setRatioMode(TeILayout::RATIO_MODE_NONE);
			app->frontLayout().setRatioMode(TeILayout::RATIO_MODE_NONE);
		} else {
			app->backLayout().setRatioMode(TeILayout::RATIO_MODE_LETTERBOX);
			app->backLayout().setRatio(1.333333f);
			app->frontLayout().setRatioMode(TeILayout::RATIO_MODE_LETTERBOX);
			app->frontLayout().setRatio(1.333333f);
		}
	}
	return false;
}

bool MainMenu::onDisabledTuto() {
	Application *app = g_engine->getApplication();
	app->setTutoActivated(false);
	g_engine->getGame()->_firstInventory = false;
	app->captureFade();
	leave();
	app->startGame(true, 1);
	app->fade();
	return false;
}

bool MainMenu::onEnterGameRotateAnimFinished() {
	error("TODO: Implement MainMenu::onEnterGameRotateAnimFinished");
}

bool MainMenu::onGalleryButtonValidated() {
	Application *app = g_engine->getApplication();
	app->captureFade();
	leave();
	app->globalBonusMenu().enter();
	app->fade();
	return false;
}

bool MainMenu::onHowToButtonValidated() {
	onContinueGameButtonValidated();
	_confirmingTuto = false;
	return false;
}

bool MainMenu::onLoadGameButtonValidated() {
	g_engine->loadGameDialog();
	return false;
}

bool MainMenu::onNewGameButtonValidated() {
	// Note: Original confirms whether to start new game here
	// with "menus/confirm/confirmNewGame.lua"
	// because only one save is allowed.  We just clear last
	// save slot number and go ahead and start.
	ConfMan.setInt(LAST_SAVE_CONF, -1);
	onNewGameConfirmed();
	return false;
}

bool MainMenu::onNewGameConfirmed() {
	// Note: Original game deletes saves here.  Don't do that..
	_confirmingTuto = true;
	_tutoConfirm.enter("menus/confirm/confirmTuto.lua", "");
	onContinueGameButtonValidated();
	return false;
}

bool MainMenu::onOptionsButtonValidated() {
	if (ConfMan.getBool("use_scummvm_options")) {
		g_engine->openConfigDialog();
	} else {
		Application *app = g_engine->getApplication();
		app->captureFade();
		leave();
		app->optionsMenu().enter();
		app->fade();
	}
	return true;
}

bool MainMenu::onQuit() {
	g_engine->quitGame();
	leave();
	return false;
}

bool MainMenu::onQuitButtonValidated() {
	_quitConfirm.enter("menus/confirm/confirmQuit.lua", "");
	return false;
}

bool MainMenu::onUnlockGameButtonValidated() {
	error("TODO: Implement MainMenu::onUnlockGameButtonValidated");
}

void MainMenu::refresh() {
	bool haveSave = ConfMan.hasKey(LAST_SAVE_CONF);
	TeButtonLayout *continueGameButton = buttonLayout("continueGameButton");
	if (continueGameButton) {
		continueGameButton->setEnable(haveSave);
	}
}

void MainMenu::setCenterButtonsVisibility(bool visible) {
	bool haveSave = ConfMan.hasKey(LAST_SAVE_CONF);

	TeButtonLayout *continuegameunlockButton = buttonLayout("continuegameunlockButton");
	if (continuegameunlockButton) {
		continuegameunlockButton->setVisible(haveSave & visible);
	}

	TeButtonLayout *newGameUnlockButton = buttonLayout("newgameunlockButton");
	if (newGameUnlockButton) {
		newGameUnlockButton->setVisible(visible & !haveSave);
	}

	TeButtonLayout *unlockgameButton = buttonLayout("unlockgameButton");
	if (unlockgameButton) {
		unlockgameButton->setVisible(false);
	}

	TeLayout *rateItButton = layout("rateItButton");
	if (rateItButton) {
		rateItButton->setVisible(false);
	}
}



} // end namespace Tetraedge
