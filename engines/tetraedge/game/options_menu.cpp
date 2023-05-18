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

#include "tetraedge/tetraedge.h"
#include "tetraedge/game/options_menu.h"
#include "tetraedge/game/application.h"
#include "tetraedge/game/game.h"
#include "tetraedge/te/te_sound_manager.h"

namespace Tetraedge {

static const float EPSILON = 1.192093e-07f;

static Common::String pageStr(int i) {
	return Common::String::format("page%d", i);
}

OptionsMenu::OptionsMenu() : _tutoPage(1) {
}

void OptionsMenu::enter() {
	Application *app = g_engine->getApplication();

	if (!app->appSpriteLayout()._tiledSurfacePtr->isLoaded()) {
		// TODO: Check
		app->appSpriteLayout().load("menus/menu.ogv");
		app->appSpriteLayout().play();
	}

	if (!g_engine->gameIsAmerzone()) {
		load("menus/options/optionsMenu.lua");
		_gui2.load("menus/options/tuto.lua");
	} else {
		load("GUI/OptionsMenu.lua");
		_gui2.load("menus/options/tuto.lua");	// TODO: This is wrong
	}

	app->frontLayout().addChild(layoutChecked("menu2"));
	app->frontLayout().addChild(_gui2.buttonLayoutChecked("tuto"));
	_gui2.buttonLayoutChecked("tuto")->setVisible(false);

	const Common::Path musicPath(value("musicPath").toString());
	if (!app->music().isPlaying() || (app->music().getAccessName() != musicPath)) {
		app->music().load(musicPath);
		app->music().play();
		app->music().volume(1.0);
	}

	Tetraedge::TeButtonLayout *quitButton = buttonLayout("quitButton");
	if (quitButton) {
		quitButton->onMouseClickValidated().add(this, &OptionsMenu::onQuitButton);
	}
	buttonLayoutChecked("creditsButton")->onMouseClickValidated().add(this, &OptionsMenu::onCreditsButton);
	TeButtonLayout *supportBtn = buttonLayout("supportButton");
	if (supportBtn) {
		supportBtn->onMouseClickValidated().add(this, &OptionsMenu::onSupportButton);
	}
	TeButtonLayout *termsBtn = buttonLayout("termsButton");
	if (termsBtn) {
		termsBtn->onMouseClickValidated().add(this, &OptionsMenu::onTermsOfServiceButton);
	}
	TeButtonLayout *privBtn = buttonLayout("privacyButton");
	if (privBtn) {
		privBtn->onMouseClickValidated().add(this, &OptionsMenu::onPrivacyPolicyButton);
	}

	buttonLayoutChecked("sfxVolumeMinusButton")->onMouseClickValidated().add(this, &OptionsMenu::onSFXVolumeMinusButton);
	buttonLayoutChecked("sfxVolumePlusButton")->onMouseClickValidated().add(this, &OptionsMenu::onSFXVolumePlusButton);
	buttonLayoutChecked("musicVolumeMinusButton")->onMouseClickValidated().add(this, &OptionsMenu::onMusicVolumeMinusButton);
	buttonLayoutChecked("musicVolumePlusButton")->onMouseClickValidated().add(this, &OptionsMenu::onMusicVolumePlusButton);
	buttonLayoutChecked("dialogVolumeMinusButton")->onMouseClickValidated().add(this, &OptionsMenu::onDialogVolumeMinusButton);
	buttonLayoutChecked("dialogVolumePlusButton")->onMouseClickValidated().add(this, &OptionsMenu::onDialogVolumePlusButton);
	buttonLayoutChecked("videoVolumeMinusButton")->onMouseClickValidated().add(this, &OptionsMenu::onVideoVolumeMinusButton);
	buttonLayoutChecked("videoVolumePlusButton")->onMouseClickValidated().add(this, &OptionsMenu::onVideoVolumePlusButton);
	buttonLayoutChecked("sfxVolumeMinusButton")->setDoubleValidationProtectionEnabled(false);
	buttonLayoutChecked("sfxVolumePlusButton")->setDoubleValidationProtectionEnabled(false);
	buttonLayoutChecked("musicVolumeMinusButton")->setDoubleValidationProtectionEnabled(false);
	buttonLayoutChecked("musicVolumePlusButton")->setDoubleValidationProtectionEnabled(false);
	buttonLayoutChecked("dialogVolumeMinusButton")->setDoubleValidationProtectionEnabled(false);
	buttonLayoutChecked("dialogVolumePlusButton")->setDoubleValidationProtectionEnabled(false);
	buttonLayoutChecked("videoVolumeMinusButton")->setDoubleValidationProtectionEnabled(false);
	buttonLayoutChecked("videoVolumePlusButton")->setDoubleValidationProtectionEnabled(false);

	_tutoPage = 1;
	buttonLayoutChecked("tutoButton")->onMouseClickValidated().add(this, &OptionsMenu::onVisibleTuto);

	TeLayout *bg = _gui2.layoutChecked("background");
	for (int i = 1; i <= bg->childCount(); i++) {
		TeButtonLayout *page = _gui2.buttonLayoutChecked(pageStr(i));
		if (i == bg->childCount()) {
			page->onMouseClickValidated().add(this, &OptionsMenu::onCloseTuto);
		} else {
			page->onMouseClickValidated().add(this, &OptionsMenu::onVisibleTutoNextPage);
		}
	}

	//
	// WORKAROUND: This is set to PanScan ratio 1.0, but with our code
	// but that shrinks it down to pillarboxed.  Force back to full size.
	//
	layoutChecked("background")->setRatioMode(TeILayout::RATIO_MODE_NONE);

	updateSFXVolumeJauge();
	updateMusicVolumeJauge();
	updateDialogVolumeJauge();
	updateVideoVolumeJauge();
	return;
}

void OptionsMenu::leave() {
	if (loaded()) {
		unload();
		_gui2.unload();
	}
}

bool OptionsMenu::onCloseTuto() {
	Application *app = g_engine->getApplication();
	app->captureFade();
	_gui2.buttonLayoutChecked("tuto")->setVisible(false);
	_tutoPage = 1;
	app->fade();
	return false;
}

bool OptionsMenu::onCreditsButton() {
	Game *game = g_engine->getGame();
	game->stopSound("sounds/Ambiances/b_automatebike.ogg");
	game->stopSound("sounds/Ambiances/b_engrenagebg.ogg");
	Application *app = g_engine->getApplication();
	app->captureFade();
	leave();
	app->credits().enter(true);
	// TODO: app->appSpriteLayout().something
	app->fade();
	return false;
}

bool OptionsMenu::onDialogVolumeMinusButton() {
	int n = 0;
	while (layout(Common::String("dialogVolumeSprite%d", n)) != nullptr)
		n++;
	float diff = (n ? (1.0f / n) : 0.1f);
	TeSoundManager *sndmgr = g_engine->getSoundManager();
	float curvol = sndmgr->getChannelVolume("dialog");
	sndmgr->setChannelVolume("dialog", MAX(0.0f, curvol - diff));
	updateDialogVolumeJauge();
	_music2.stop();
	if (!_music1.isPlaying()) {
		_music1.setChannelName("dialog");
		_music1.repeat(false);
		_music1.load(Common::Path(value("dialogTestPath").toString()));
		_music1.play();
	}
	return false;
}

bool OptionsMenu::onDialogVolumePlusButton() {
	int n = 0;
	while (layout(Common::String("dialogVolumeSprite%d", n)) != nullptr)
		n++;
	float diff = (n ? (1.0f / n) : 0.1f);
	TeSoundManager *sndmgr = g_engine->getSoundManager();
	float curvol = sndmgr->getChannelVolume("dialog");
	sndmgr->setChannelVolume("dialog", MIN(1.0f, curvol + diff));
	updateDialogVolumeJauge();
	_music2.stop();
	if (!_music1.isPlaying()) {
		_music1.setChannelName("dialog");
		_music1.repeat(false);
		_music1.load(Common::Path(value("dialogTestPath").toString()));
		_music1.play();
	}
	return false;
}

bool OptionsMenu::onMusicVolumeMinusButton() {
	int n = 0;
	while (layout(Common::String("musicVolumeSprite%d", n)) != nullptr)
		n++;
	float diff = (n ? (1.0f / n) : 0.1f);
	TeSoundManager *sndmgr = g_engine->getSoundManager();
	float curvol = sndmgr->getChannelVolume("music");
	sndmgr->setChannelVolume("music", MAX(0.0f, curvol - diff));
	updateMusicVolumeJauge();
	return false;
}

bool OptionsMenu::onMusicVolumePlusButton() {
	int n = 0;
	while (layout(Common::String("musicVolumeSprite%d", n)) != nullptr)
		n++;
	float diff = (n ? (1.0f / n) : 0.1f);
	TeSoundManager *sndmgr = g_engine->getSoundManager();
	float curvol = sndmgr->getChannelVolume("music");
	sndmgr->setChannelVolume("music", MIN(1.0f, curvol + diff));
	updateMusicVolumeJauge();
	return false;
}

bool OptionsMenu::onQuitButton() {
	Application *app = g_engine->getApplication();
	app->captureFade();
	leave();
	app->mainMenu().enter();
	app->fade();
	return true;
}

bool OptionsMenu::onSFXVolumeMinusButton() {
	int n = 0;
	while (layout(Common::String("sfxVolumeSprite%d", n)) != nullptr)
		n++;
	float diff = (n ? (1.0f / n) : 0.1f);
	TeSoundManager *sndmgr = g_engine->getSoundManager();
	float curvol = sndmgr->getChannelVolume("sfx");
	sndmgr->setChannelVolume("sfx", MAX(0.0f, curvol - diff));
	updateSFXVolumeJauge();
	return false;
}

bool OptionsMenu::onSFXVolumePlusButton() {
	int n = 0;
	while (layout(Common::String("sfxVolumeSprite%d", n)) != nullptr)
		n++;
	float diff = (n ? (1.0f / n) : 0.1f);
	TeSoundManager *sndmgr = g_engine->getSoundManager();
	float curvol = sndmgr->getChannelVolume("sfx");
	sndmgr->setChannelVolume("sfx", MIN(1.0f, curvol + diff));
	updateSFXVolumeJauge();
	return false;
}

bool OptionsMenu::onVideoVolumeMinusButton() {
	int n = 0;
	while (layout(Common::String("videoVolumeSprite%d", n)) != nullptr)
		n++;
	//float diff = (n ? (1.0f / n) : 0.1f);
	//TeSoundManager *sndmgr = g_engine->getSoundManager();
	//float curvol = sndmgr->getChannelVolume("video");
	warning("TODO: Implement onVideoVolumeMinusButton");
	updateVideoVolumeJauge();
	return false;
}

bool OptionsMenu::onVideoVolumePlusButton() {
	int n = 0;
	while (layout(Common::String("videoVolumeSprite%d", n)) != nullptr)
		n++;
	//float diff = (n ? (1.0f / n) : 0.1f);
	//TeSoundManager *sndmgr = g_engine->getSoundManager();
	//float curvol = sndmgr->getChannelVolume("video");
	warning("TODO: Implement onVideoVolumePlusButton");
	updateVideoVolumeJauge();
	return false;
}

bool OptionsMenu::onVisibleTuto() {
	Application *app = g_engine->getApplication();
	app->captureFade();
	TeButtonLayout *tutobtn = _gui2.buttonLayoutChecked("tuto");
	TeLayout *background = _gui2.layoutChecked("background");
	tutobtn->setVisible(true);
	for (int i = 1; i <= background->childCount(); i++) {
		_gui2.buttonLayoutChecked(pageStr(i))->setVisible(false);
	}
	_gui2.buttonLayoutChecked(pageStr(1))->setVisible(true);
	app->fade();
	return false;
}

bool OptionsMenu::onVisibleTutoNextPage() {
	TeButtonLayout *tutobtn = _gui2.buttonLayoutChecked("tuto");
	tutobtn->setVisible(true);
	TeLayout *bg = _gui2.layoutChecked("background");

	for (int i = 1; i <= bg->childCount(); i++) {
		_gui2.buttonLayoutChecked(pageStr(i))->setVisible(false);
	}

	_gui2.buttonLayoutChecked(pageStr(_tutoPage))->setVisible(false);
	_tutoPage++;
	_gui2.buttonLayoutChecked(pageStr(_tutoPage))->setVisible(true);
	return false;
}

void OptionsMenu::updateJauge(const Common::String &chan, const Common::String &spriteName) {
	TeSoundManager *sndMgr = g_engine->getSoundManager();
	const float chanVol = sndMgr->getChannelVolume(chan);
	TeSpriteLayout *volSprite = spriteLayout(spriteName);
	if (volSprite)
		volSprite->_tiledSurfacePtr->setLeftCropping(sndMgr->getChannelVolume(chan));

	int n = 0;
	while (layout(Common::String::format("%s%d", spriteName.c_str(), n)) != nullptr)
		n++;

	int i = 0;
	while (true) {
		TeLayout *sprite = layout(Common::String::format("%s%d", spriteName.c_str(), i));
		if (!sprite)
			break;
		bool enableSprite = false;
		float mul = n ? 1.0f / n : 1.0f;
		if (i * mul - EPSILON <= chanVol) {
			enableSprite = chanVol < (i + 1) * mul - EPSILON;
		}
		sprite->setVisible(enableSprite);

		TeLayout *offSprite = layout(Common::String::format("%s%dOff", spriteName.c_str(), i));
		if (offSprite) {
			offSprite->setVisible(!enableSprite);
		}
		i++;
	}
}

void OptionsMenu::updateDialogVolumeJauge() {
	updateJauge("dialog", "dialogVolumeSprite");
}

void OptionsMenu::updateMusicVolumeJauge() {
	updateJauge("music", "musicVolumeSprite");
}

void OptionsMenu::updateSFXVolumeJauge() {
	updateJauge("sfx", "sfxVolumeSprite");
}

void OptionsMenu::updateVideoVolumeJauge() {
	updateJauge("video", "videoVolumeSprite");
}

} // end namespace Tetraedge
