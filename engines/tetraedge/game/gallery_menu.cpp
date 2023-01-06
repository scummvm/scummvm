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
#include "tetraedge/game/application.h"
#include "tetraedge/game/game.h"
#include "tetraedge/game/gallery_menu.h"

namespace Tetraedge {

static const char *AMBIENT_SND_BIKE = "sounds/Ambiances/b_automatebike.ogg";
static const char *AMBIENT_SND_ENGR = "sounds/Ambiances/b_engrenagebg.ogg";

GalleryMenu::GalleryMenu() {
}

bool GalleryMenu::onLockVideoButtonValidated() {
	onSkipVideoButtonValidated();
	return false;
}

bool GalleryMenu::onSkipVideoButtonValidated() {
	Application *app = g_engine->getApplication();
	app->music().play();
	Game *game = g_engine->getGame();

	game->stopSound(AMBIENT_SND_BIKE);
	game->playSound(AMBIENT_SND_BIKE, -1, 0.1f);

	game->stopSound(AMBIENT_SND_ENGR);
	game->playSound(AMBIENT_SND_ENGR, -1, 0.09f);

	TeSpriteLayout *video = spriteLayoutChecked("video");
	video->stop();
	video->setVisible(false);
	buttonLayoutChecked("videoBackgroundButton")->setVisible(false);
	buttonLayoutChecked("skipVideoButton")->setVisible(false);
	_music.stop();
	return false;
}

bool GalleryMenu::onQuitButton() {
	Application *app = g_engine->getApplication();
	app->captureFade();
	leave();
	app->mainMenu().enter();
	app->fade();
	return true;
}

bool GalleryMenu::onVideoFinished() {
	if (_loaded) {
		Application *app = g_engine->getApplication();
		app->captureFade();
		onSkipVideoButtonValidated();
		app->music().play();
		app->fade();
	}
	return false;
}

void GalleryMenu::enter() {
	Application *app = g_engine->getApplication();
	Game *game = g_engine->getGame();

	load("menus/galleryMenu/galleryMenu.lua");
	TeLayout *menu = layoutChecked("galleryMenu");
	app->frontLayout().addChild(menu);

	game->stopSound(AMBIENT_SND_BIKE);
	game->playSound(AMBIENT_SND_BIKE, -1, 0.1f);

	game->stopSound(AMBIENT_SND_ENGR);
	game->playSound(AMBIENT_SND_ENGR, -1, 0.09f);

	TeButtonLayout *btn = buttonLayoutChecked("quitButton");
	btn->onMouseClickValidated().add(this, &GalleryMenu::onQuitButton);

	//TeLayout *list = layoutChecked("galleryList");

	error("TODO: Finish GalleryMenu::enter");
}

void GalleryMenu::leave() {
	if (!_loaded)
		return;

	Game *game = g_engine->getGame();
	game->stopSound(AMBIENT_SND_BIKE);
	game->stopSound(AMBIENT_SND_ENGR);
	unload();
	for (GalleryBtnObject *btn : _btnObjects) {
		delete btn;
	}
	_btnObjects.clear();
}

bool GalleryMenu::GalleryBtnObject::onValidated() {
	error("TODO: Implement GalleryMenu::GalleryBtnObject::onValidated");
}


} // end namespace Tetraedge
