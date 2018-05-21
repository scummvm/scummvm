/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/stark/ui/menu/mainmenu.h"
#include "engines/stark/services/userinterface.h"
#include "engines/stark/services/resourceprovider.h"
#include "engines/stark/services/global.h"

#include "common/config-manager.h"

namespace Stark {

MainMenuScreen::MainMenuScreen(Gfx::Driver *gfx, Cursor *cursor) :
		StaticLocationScreen(gfx, cursor, "MainMenuLocation", Screen::kScreenMainMenu) {
}

MainMenuScreen::~MainMenuScreen() {
}

void MainMenuScreen::open() {
	StaticLocationScreen::open();

	//TODO: Implement each handler
	_widgets.push_back(new StaticLocationWidget(
			"BGImage",
			nullptr,
			nullptr));
	
	_widgets.push_back(new StaticLocationWidget(
			"NewGame",
			CLICK_HANDLER(MainMenuScreen, newGameHandler),
			MOVE_HANDLER(MainMenuScreen, helpTextHandler<7>)));
	_widgets.back()->setupSounds(0, 1);
	
	_widgets.push_back(new StaticLocationWidget(
			"Continue",
			CLICK_HANDLER(MainMenuScreen, loadHandler),
			MOVE_HANDLER(MainMenuScreen, helpTextHandler<8>)));
	_widgets.back()->setupSounds(0, 1);
	
	_widgets.push_back(new StaticLocationWidget(
			"Options",
			CLICK_HANDLER(MainMenuScreen, settingsHandler),
			MOVE_HANDLER(MainMenuScreen, helpTextHandler<6>)));
	_widgets.back()->setupSounds(0, 1);
	
	_widgets.push_back(new StaticLocationWidget(
			"Box",
			nullptr,
			MOVE_HANDLER(MainMenuScreen, helpTextHandler<9>)));
	_widgets.back()->setupSounds(0, 1);
	
	_widgets.push_back(new StaticLocationWidget(
			"Quit",
			CLICK_HANDLER(MainMenuScreen, quitHandler),
			MOVE_HANDLER(MainMenuScreen, helpTextHandler<10>)));
	_widgets.back()->setupSounds(0, 1);
	
	_widgets.push_back(new StaticLocationWidget(
			"OptionHelp",
			nullptr,
			nullptr));
	_widgets.back()->setVisible(false);
	
	_widgets.push_back(new StaticLocationWidget(
			"BeginHelp",
			nullptr,
			nullptr));
	_widgets.back()->setVisible(false);
	
	_widgets.push_back(new StaticLocationWidget(
			"ContinueHelp",
			nullptr,
			nullptr));
	_widgets.back()->setVisible(false);
	
	_widgets.push_back(new StaticLocationWidget(
			"BoxHelp",
			nullptr,
			nullptr));
	_widgets.back()->setVisible(false);
	
	_widgets.push_back(new StaticLocationWidget(
			"QuitHelp",
			nullptr,
			nullptr));
	_widgets.back()->setVisible(false);
	
	_widgets.push_back(new StaticLocationWidget(
			"Credits",
			CLICK_HANDLER(MainMenuScreen, creditsHandler),
			MOVE_HANDLER(MainMenuScreen, helpTextHandler<12>)));
	_widgets.back()->setupSounds(0, 1);
	
	_widgets.push_back(new StaticLocationWidget(
			"CreditHelp",
			nullptr,
			nullptr));
	_widgets.back()->setVisible(false);
	
	_widgets.push_back(new StaticLocationWidget(
			"VERSION INFO",
			nullptr,
			nullptr));
	
	_widgets.push_back(new StaticLocationWidget(
			"VERSION INFO REALLY",
			nullptr,
			nullptr));
}

template<uint N>
void MainMenuScreen::helpTextHandler(StaticLocationWidget &widget, const Common::Point &mousePos) {
	if (widget.isVisible()) {
		_widgets[N]->setVisible(widget.isMouseInside(mousePos));
	}
}

void MainMenuScreen::creditsHandler() {
	if (!isDemo()) {
		StarkUserInterface->requestFMVPlayback("0e02.bbb");
	}
}

void MainMenuScreen::newGameHandler() {
	StarkUserInterface->changeScreen(kScreenGame);

	StarkResourceProvider->initGlobal();

	if (ConfMan.hasKey("startup_chapter")) {
		StarkGlobal->setCurrentChapter(ConfMan.getInt("startup_chapter"));
	} else {
		StarkGlobal->setCurrentChapter(0);
	}

	if (ConfMan.hasKey("startup_level") && ConfMan.hasKey("startup_location")) {
		uint levelIndex = strtol(ConfMan.get("startup_level").c_str(), nullptr, 16);
		uint locationIndex = strtol(ConfMan.get("startup_location").c_str(), nullptr, 16);
		StarkResourceProvider->requestLocationChange(levelIndex, locationIndex);
	} else {
		if (isDemo()) {
			StarkResourceProvider->requestLocationChange(0x4f, 0x00);
		} else {
			// Start us up at the house of all worlds
			StarkResourceProvider->requestLocationChange(0x45, 0x00);
		}
	}
}

void MainMenuScreen::loadHandler() {
	// TODO: Link to the load screen
}

void MainMenuScreen::settingsHandler() {
	StarkUserInterface->changeScreen(Screen::kScreenSettingsMenu);
}

void MainMenuScreen::quitHandler() {
	StarkUserInterface->notifyShouldExit();
}

} // End of namespace Stark