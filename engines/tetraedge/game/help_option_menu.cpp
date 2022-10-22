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
#include "tetraedge/game/help_option_menu.h"

namespace Tetraedge {

HelpOptionMenu::HelpOptionMenu() : _entered(false) {
}

void HelpOptionMenu::enter() {
	if (!_entered) {
		Application *app = g_engine->getApplication();
		app->captureFade();
		load("menus/helpoptionMenu/optionsMenu.lua");

		TeLayout *menu = layoutChecked("menu");
		app->appSpriteLayout().addChild(menu);
		app->fade();
	}
}

void HelpOptionMenu::leave() {
	Application *app = g_engine->getApplication();
	app->captureFade();
	unload();
	app->fade();
	_entered = false;
}

} // end namespace Tetraedge
