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
#include "tetraedge/game/gallery_menu.h"

namespace Tetraedge {

GalleryMenu::GalleryMenu() {
}

bool GalleryMenu::onLockVideoButtonValidated() {
	onSkipVideoButtonValidated();
	return false;
}

bool GalleryMenu::onSkipVideoButtonValidated() {
	error("TODO: Implement onSkipVideoButtonValidated");
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
	error("TODO: implement GalleryMenu::enter");
}

void GalleryMenu::leave() {
	error("TODO: implement GalleryMenu::leave");
}

} // end namespace Tetraedge
