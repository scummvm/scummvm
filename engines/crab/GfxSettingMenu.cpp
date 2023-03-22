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

#include "GfxSettingMenu.h"

using namespace pyrodactyl::ui;

//------------------------------------------------------------------------
// Purpose: Load components from file
//------------------------------------------------------------------------
void GfxSettingMenu::Load(rapidxml::xml_node<char> *node) {
	if (NodeValid("resolution", node))
		resolution.Load(node->first_node("resolution"));

	if (NodeValid("fullscreen", node))
		fullscreen.Load(node->first_node("fullscreen"));

	if (NodeValid("vsync", node))
		vsync.Load(node->first_node("vsync"));

	if (NodeValid("border", node))
		border.Load(node->first_node("border"));

	if (NodeValid("quality", node)) {
		rapidxml::xml_node<char> *qnode = node->first_node("quality");
		quality.Load(qnode);

		if (NodeValid("message", qnode))
			notice_quality.Load(qnode->first_node("message"), &quality);
	}

	if (NodeValid("brightness", node))
		brightness.Load(node->first_node("brightness"), 0, 100, gScreenSettings.gamma * 100);
}

//------------------------------------------------------------------------
// Purpose: Draw stuff
//------------------------------------------------------------------------
void GfxSettingMenu::Draw() {
	// Window border doesn't matter if you are in fullscreen
	if (!gScreenSettings.fullscreen)
		border.Draw();

	// Draw toggle buttons
	brightness.Draw();
	fullscreen.Draw();
	vsync.Draw();

	// Quality and resolution can only be changed in the main menu
	if (!gScreenSettings.in_game) {
		// Tree quality button
		quality.Draw();
	} else
		notice_quality.Draw(); // Notice about quality settings

	// Draw resolution menu
	resolution.Draw();
}

//------------------------------------------------------------------------
// Purpose: Handle input
//------------------------------------------------------------------------
int GfxSettingMenu::HandleEvents(const SDL_Event &Event) {
	if (fullscreen.HandleEvents(Event)) {
		// Setting video flags is necessary when toggling fullscreen
		gScreenSettings.fullscreen = !gScreenSettings.fullscreen;
		gScreenSettings.SetFullscreen();
	}

	// Vsync doesn't need to set the change value
	if (vsync.HandleEvents(Event)) {
		gScreenSettings.vsync = !gScreenSettings.vsync;
		gScreenSettings.SetVsync();
	}

	// Quality and resolution can only be changed in the main menu
	if (!gScreenSettings.in_game) {
		if (quality.HandleEvents(Event))
			gScreenSettings.quality = !gScreenSettings.quality;
	}

	// Window border doesn't matter if you are in fullscreen
	if (border.HandleEvents(Event) && !gScreenSettings.fullscreen) {
		gScreenSettings.border = !gScreenSettings.border;
		gScreenSettings.SetWindowBorder();
	}

	if (brightness.HandleEvents(Event)) {
		gScreenSettings.gamma = static_cast<float>(brightness.Value()) / 100.0f;
		gScreenSettings.SetGamma();
	}

	return resolution.HandleEvents(Event);
}

//------------------------------------------------------------------------
// Purpose: Keep button settings synced with our screen settings
//------------------------------------------------------------------------
void GfxSettingMenu::InternalEvents() {
	fullscreen.state = gScreenSettings.fullscreen;
	vsync.state = gScreenSettings.vsync;
	border.state = gScreenSettings.border;
	quality.state = gScreenSettings.quality;
}

//------------------------------------------------------------------------
// Purpose: Rearrange UI when resolution changes
//------------------------------------------------------------------------
void GfxSettingMenu::SetUI() {
	resolution.SetUI();

	fullscreen.SetUI();
	vsync.SetUI();
	border.SetUI();
	quality.SetUI();

	notice_quality.SetUI();
	brightness.SetUI();
}
