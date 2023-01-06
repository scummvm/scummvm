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
#include "tetraedge/game/global_bonus_menu.h"

namespace Tetraedge {

GlobalBonusMenu::GlobalBonusMenu() : _entered(false) {
}

void GlobalBonusMenu::enter() {
	Application *app = g_engine->getApplication();
	app->appSpriteLayout().setVisible(true);
	app->captureFade();
	_entered = true;
	load("menus/bonusmenu/GlobalBonusMenu.lua");
	TeLayout *menu = layoutChecked("menu");
	app->frontLayout().addChild(menu);

	// Original checks each layout's existence
	TeButtonLayout *btn;
	btn = buttonLayout("Val");
	if (btn)
		btn->onMouseClickValidated().add(this, &GlobalBonusMenu::onValButtonValidated);
	btn = buttonLayout("Bar");
	if (btn)
		btn->onMouseClickValidated().add(this, &GlobalBonusMenu::onBarButtonValidated);
	btn = buttonLayout("Cit");
	if (btn)
		btn->onMouseClickValidated().add(this, &GlobalBonusMenu::onCitButtonValidated);
	btn = buttonLayout("Ara");
	if (btn)
		btn->onMouseClickValidated().add(this, &GlobalBonusMenu::onAraButtonValidated);
	btn = buttonLayout("Syb2");
	if (btn)
		btn->onMouseClickValidated().add(this, &GlobalBonusMenu::onSyb2ButtonValidated);
	btn = buttonLayout("Syb3");
	if (btn)
		btn->onMouseClickValidated().add(this, &GlobalBonusMenu::onSyb3ButtonValidated);
	btn = buttonLayout("Back");
	if (btn)
		btn->onMouseClickValidated().add(this, &GlobalBonusMenu::onQuitButton);
}

void GlobalBonusMenu::leave() {
	if (!_entered)
		return;

	Application *app = g_engine->getApplication();
	app->captureFade();
	TeLuaGUI::unload();
	app->fade();
	_entered = false;
}

bool GlobalBonusMenu::onSomeButtonValidated(const char *script) {
	Application *app = g_engine->getApplication();
	app->captureFade();
	leave();
	app->bonusMenu().enter(script);
	app->fade();
	return false;
}

bool GlobalBonusMenu::onAraButtonValidated() {
	return onSomeButtonValidated("menus/bonusmenu/Ara.lua");
}

bool GlobalBonusMenu::onBarButtonValidated() {
	return onSomeButtonValidated("menus/bonusmenu/Bar.lua");
}

bool GlobalBonusMenu::onCitButtonValidated() {
	return onSomeButtonValidated("menus/bonusmenu/Cit.lua");
}

bool GlobalBonusMenu::onSyb2ButtonValidated() {
	return onSomeButtonValidated("menus/bonusmenu/Syb2.lua");
}

bool GlobalBonusMenu::onSyb3ButtonValidated() {
	return onSomeButtonValidated("menus/bonusmenu/Syb3.lua");
}

bool GlobalBonusMenu::onValButtonValidated() {
	return onSomeButtonValidated("menus/bonusmenu/Val.lua");
}

bool GlobalBonusMenu::onQuitButton() {
	Application *app = g_engine->getApplication();
	app->captureFade();
	leave();
	app->mainMenu().enter();
	app->fade();
	return true;
}

} // end namespace Tetraedge
