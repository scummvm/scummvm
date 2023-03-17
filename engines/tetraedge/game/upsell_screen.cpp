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
#include "tetraedge/game/upsell_screen.h"

namespace Tetraedge {

UpsellScreen::UpsellScreen() {
}

void UpsellScreen::enter() {
	if (!load("menus/upsell/UpsellScreen.lua")) {
		error("Failed to load UpsellScreen.lua");
	}

	// Simplified logic from original as we don't support in-app purhcases.
	Application *app = g_engine->getApplication();
	app->frontLayout().addChild(layoutChecked("upsellMenu"));

	TeButtonLayout *btn;
	btn = buttonLayoutChecked("unlockLaterButton");
	btn->onMouseClickValidated().add(this, &UpsellScreen::onQuitButton);

	btn = buttonLayoutChecked("purchaseNowButton");
	btn->onMouseClickValidated().add(this, &UpsellScreen::onPurchaseButton);
	btn->setEnable(true);

	btn = buttonLayoutChecked("alreadyPaidButton");
	btn->onMouseClickValidated().add(this, &UpsellScreen::onAlreadyPurchasedButton);
	btn->setEnable(true);
}

void UpsellScreen::leave() {
	if (loaded())
		unload();
}

bool UpsellScreen::onAlreadyPurchasedButton() {
	warning("Ignoring button. In-game purchases not available in ScummVM");
	return false;
}

bool UpsellScreen::onPurchaseButton() {
	warning("Ignoring button. In-game purchases not available in ScummVM");
	return false;
}

bool UpsellScreen::onQuitButton() {
	leave();
	Application *app = g_engine->getApplication();
	app->captureFade();
	app->mainMenu().enter();
	app->fade();
	return true;
}

} // end namespace Tetraedge
