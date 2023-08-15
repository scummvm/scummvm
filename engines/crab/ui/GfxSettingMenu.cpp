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

#include "crab/crab.h"
#include "crab/ui/GfxSettingMenu.h"

namespace Crab {

using namespace pyrodactyl::ui;

//------------------------------------------------------------------------
// Purpose: Load components from file
//------------------------------------------------------------------------
void GfxSettingMenu::load(rapidxml::xml_node<char> *node) {
	if (nodeValid("resolution", node))
		_resolution.load(node->first_node("resolution"));

	if (nodeValid("fullscreen", node))
		_fullscreen.load(node->first_node("fullscreen"));

	if (nodeValid("vsync", node))
		_vsync.load(node->first_node("vsync"));

	if (nodeValid("border", node))
		_border.load(node->first_node("border"));

	if (nodeValid("quality", node)) {
		rapidxml::xml_node<char> *qnode = node->first_node("quality");
		_quality.load(qnode);

		if (nodeValid("message", qnode))
			_noticeQuality.load(qnode->first_node("message"), &_quality);
	}

	if (nodeValid("brightness", node))
		_brightness.load(node->first_node("brightness"), 0, 100, g_engine->_screenSettings->_gamma * 100);

	// This functionality has been disabled in ScummVM.
	_brightness.setEnabled(false);
}

//------------------------------------------------------------------------
// Purpose: Draw stuff
//------------------------------------------------------------------------
void GfxSettingMenu::draw() {
	// Window border doesn't matter if you are in fullscreen
	if (!g_engine->_screenSettings->_fullscreen)
		_border.draw();

	// Draw toggle buttons
	_brightness.draw();
	_fullscreen.draw();
	_vsync.draw();

	// Quality and resolution can only be changed in the main menu
	if (!g_engine->_screenSettings->_inGame) {
		// Tree quality button
		_quality.draw();
	} else
		_noticeQuality.draw(); // Notice about quality settings

	// Draw resolution menu
	_resolution.draw();
}

//------------------------------------------------------------------------
// Purpose: Handle input
//------------------------------------------------------------------------
int GfxSettingMenu::handleEvents(const Common::Event &event) {
	if (_fullscreen.handleEvents(event) != BUAC_IGNORE) {
		// Setting video flags is necessary when toggling fullscreen
		g_engine->_screenSettings->_fullscreen = !g_engine->_screenSettings->_fullscreen;
	}

	// Vsync doesn't need to set the change value
	if (_vsync.handleEvents(event) != BUAC_IGNORE) {
		g_engine->_screenSettings->_vsync = !g_engine->_screenSettings->_vsync;
	}

	// Quality and resolution can only be changed in the main menu
	if (!g_engine->_screenSettings->_inGame) {
		if (_quality.handleEvents(event) != BUAC_IGNORE)
			g_engine->_screenSettings->_quality = !g_engine->_screenSettings->_quality;
	}

	// Window border doesn't matter if you are in fullscreen
	if (_border.handleEvents(event) && !g_engine->_screenSettings->_fullscreen != BUAC_IGNORE) {
		g_engine->_screenSettings->_border = !g_engine->_screenSettings->_border;
	}

	if (_brightness.handleEvents(event)) {
		g_engine->_screenSettings->_gamma = static_cast<float>(_brightness.Value()) / 100.0f;
	}

	return _resolution.handleEvents(event);
}

//------------------------------------------------------------------------
// Purpose: Keep button settings synced with our screen settings
//------------------------------------------------------------------------
void GfxSettingMenu::internalEvents() {
	_fullscreen._state = g_engine->_screenSettings->_fullscreen;
	_vsync._state = g_engine->_screenSettings->_vsync;
	_border._state = g_engine->_screenSettings->_border;
	_quality._state = g_engine->_screenSettings->_quality;
}

//------------------------------------------------------------------------
// Purpose: Rearrange UI when resolution changes
//------------------------------------------------------------------------
void GfxSettingMenu::setUI() {
	_resolution.setUI();

	_fullscreen.setUI();
	_vsync.setUI();
	_border.setUI();
	_quality.setUI();

	_noticeQuality.setUI();
	_brightness.setUI();
}

} // End of namespace Crab
