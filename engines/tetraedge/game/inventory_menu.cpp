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
#include "tetraedge/game/inventory_menu.h"

namespace Tetraedge {

InventoryMenu::InventoryMenu() {
}

void InventoryMenu::enter() {
	Application *app = g_engine->getApplication();
	app->mouseCursorLayout().load(app->defaultCursor());

	_gui.layoutChecked("inventoryMenu")->setVisible(true);
	onInventoryButton();
}

void InventoryMenu::leave() {
	Game *game = g_engine->getGame();
	game->inventory().leave();
	game->documentsBrowser().leave();
	TeLayout *invMenu = _gui.layout("inventoryMenu");
	if (invMenu)
		invMenu->setVisible(false);
}

void InventoryMenu::load() {
	setName("inventoryMenu");
	setSizeType(RELATIVE_TO_PARENT);
	TeVector3f32 usersz = userSize();
	setSize(TeVector3f32(1.0f, 1.0f, usersz.z()));

	_gui.load("InventoryMenu/InventoryMenu.lua");
	addChild(_gui.layoutChecked("inventoryMenu"));
	_gui.buttonLayoutChecked("quitButton")->onMouseClickValidated()
				.add(this, &InventoryMenu::onQuitButton);
	_gui.buttonLayoutChecked("quitBackground")->onMouseClickValidated()
				.add(this, &InventoryMenu::onQuitButton);
	_gui.buttonLayoutChecked("mainMenuButton")->onMouseClickValidated()
				.add(this, &InventoryMenu::onMainMenuButton);
	_gui.buttonLayoutChecked("documentsButton")->onMouseClickValidated()
				.add(this, &InventoryMenu::onDocumentsButton);
	_gui.buttonLayoutChecked("inventoryButton")->onMouseClickValidated()
				.add(this, &InventoryMenu::onInventoryButton);

	_gui.layoutChecked("inventoryMenu")->setVisible(false);

	setVisible(false);
}

void InventoryMenu::unload() {
	leave();
	_gui.unload();
}

bool InventoryMenu::isVisible() {
	TeLayout *menuLayout = _gui.layout("inventoryMenu");
	return menuLayout->visible();
}

bool InventoryMenu::onDocumentsButton() {
	_gui.buttonLayoutChecked("mainMenuButton")->setEnable(true);
	_gui.buttonLayoutChecked("documentsButton")->setEnable(false);
	_gui.buttonLayoutChecked("inventoryButton")->setEnable(true);
	Game *game = g_engine->getGame();
	game->inventory().leave();
	game->documentsBrowser().enter();
	return false;
}

bool InventoryMenu::onInventoryButton() {
	_gui.buttonLayoutChecked("mainMenuButton")->setEnable(true);
	_gui.buttonLayoutChecked("documentsButton")->setEnable(true);
	_gui.buttonLayoutChecked("inventoryButton")->setEnable(false);
	Game *game = g_engine->getGame();
	game->inventory().enter();
	game->documentsBrowser().leave();
	return false;
}

bool InventoryMenu::onMainMenuButton() {
	Application *app = g_engine->getApplication();
	app->captureFade();
	Game *game = g_engine->getGame();
	game->_returnToMainMenu = true;
	app->fade();
	return false;
}

bool InventoryMenu::onQuitButton() {
	leave();
	return false;
}

bool InventoryMenu::onSaveButton(){
	return false;
}

} // end namespace Tetraedge
