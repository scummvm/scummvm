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

/*
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#include "crab/ScreenSettings.h"
#include "crab/crab.h"
#include "crab/ui/GeneralSettingMenu.h"

namespace Crab {

using namespace pyrodactyl::ui;
using namespace pyrodactyl::music;

//------------------------------------------------------------------------
// Purpose: Load components from file
//------------------------------------------------------------------------
void GeneralSettingMenu::load(rapidxml::xml_node<char> *node) {
	if (nodeValid("volume", node)) {
		rapidxml::xml_node<char> *musnode = node->first_node("volume");

		if (nodeValid("desc", musnode))
			_noticeVolume.load(musnode->first_node("desc"));

		if (nodeValid("music", musnode)) {
			int val = g_engine->_musicManager->volMusic();
			if (ConfMan.hasKey("mute") && ConfMan.getBool("mute"))
				val = 0;
			_volMusic.load(musnode->first_node("music"), 0, 255, val);
		}

		if (nodeValid("effects", musnode)) {
			int val = g_engine->_musicManager->volEffects();
			if (ConfMan.hasKey("mute") && ConfMan.getBool("mute"))
				val = 0;
			_volEffects.load(musnode->first_node("effects"), 0, 255, val);
		}
	}

	if (nodeValid("mouse_trap", node))
		_mouseTrap.load(node->first_node("mouse_trap"));

	if (nodeValid("save_on_exit", node))
		_saveOnExit.load(node->first_node("save_on_exit"));

	if (nodeValid("text_speed", node))
		_textSpeed.load(node->first_node("text_speed"));

	// Sync popup text value with actual value
	for (auto &i : _textSpeed._element)
		i._state = (i._val == g_engine->_screenSettings->_textSpeed);

	setUI();
	createBackup();
}

//------------------------------------------------------------------------
// Purpose: Handle user input
//------------------------------------------------------------------------
void GeneralSettingMenu::handleEvents(const Common::Event &event) {
	if (_volMusic.handleEvents(event)) {
		bool unmute = _volMusic.Value() > 0 && ConfMan.hasKey("mute") && ConfMan.getBool("mute");
		g_engine->_musicManager->volMusic(_volMusic.Value(), unmute);
	}

	if (_volEffects.handleEvents(event)) {
		bool unmute = _volEffects.Value() > 0 && ConfMan.hasKey("mute") && ConfMan.getBool("mute");
		g_engine->_musicManager->volEffects(_volEffects.Value(), unmute);
	}

	// No need to change screen here
	if (_saveOnExit.handleEvents(event) != BUAC_IGNORE)
		g_engine->_screenSettings->_saveOnExit = !g_engine->_screenSettings->_saveOnExit;

	if (_mouseTrap.handleEvents(event) != BUAC_IGNORE) {
		g_engine->_screenSettings->_mouseTrap = !g_engine->_screenSettings->_mouseTrap;
	}

	int result = _textSpeed.handleEvents(event);
	if (result >= 0)
		g_engine->_screenSettings->_textSpeed = _textSpeed._element[result]._val;
}

//------------------------------------------------------------------------
// Purpose: Sync our buttons with screen settings
//------------------------------------------------------------------------
void GeneralSettingMenu::internalEvents() {
	_saveOnExit._state = g_engine->_screenSettings->_saveOnExit;
	_mouseTrap._state = g_engine->_screenSettings->_mouseTrap;
}

//------------------------------------------------------------------------
// Purpose: Draw stuff
//------------------------------------------------------------------------
void GeneralSettingMenu::draw() {
	// Draw volume sliders
	_noticeVolume.draw();
	_volMusic.draw();
	_volEffects.draw();

	// Draw the auto-save on exit option
	_saveOnExit.draw();
	_mouseTrap.draw();

	// Text speed radio button menu
	_textSpeed.draw();
}

//------------------------------------------------------------------------
// Purpose: Revert to previously backed up settings
//------------------------------------------------------------------------
void GeneralSettingMenu::restoreBackup() {
	_volMusic.restoreBackup();
	g_engine->_musicManager->volMusic(_volMusic.Value());

	_volEffects.restoreBackup();
	g_engine->_musicManager->volEffects(_volEffects.Value());
}

//------------------------------------------------------------------------
// Purpose: Set UI positioned on screen size change
//------------------------------------------------------------------------
void GeneralSettingMenu::setUI() {
	_saveOnExit.setUI();
	_mouseTrap.setUI();

	_volMusic.setUI();
	_volEffects.setUI();

	_textSpeed.setUI();
}

} // End of namespace Crab
