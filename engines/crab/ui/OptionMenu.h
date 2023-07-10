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

#ifndef CRAB_OPTIONMENU_H
#define CRAB_OPTIONMENU_H

#include "crab/ui/GeneralSettingMenu.h"
#include "crab/ui/GfxSettingMenu.h"
#include "crab/ui/ImageData.h"
#include "crab/ui/KeyBindMenu.h"
#include "crab/ui/StateButton.h"
#include "crab/ui/ToggleButton.h"
#include "crab/common_header.h"
#include "crab/ui/slider.h"
#include "crab/timer.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
class OptionMenu {
	// What overall state the menu is in
	enum { STATE_GENERAL,
		   STATE_GRAPHICS,
		   STATE_KEYBOARD,
		   STATE_CONTROLLER,
		   STATE_ENTER_W,
		   STATE_ENTER_H,
		   STATE_CONFIRM } state;

	// The overall menu for switching between states
	// The second last button is save, the last button is cancel
	Menu<StateButton> menu;

	// The background image
	ImageData bg;

	// The graphical settings menu
	GfxSettingMenu gfx;

	// The general settings menu
	GeneralSettingMenu general;

	// Keyboard controls menu
	KeyBindMenu keybind;

	// The controller controls are just drawn in a single image, no reassign options
	ImageData conbind;

	// The UI for accepting/rejecting change in resolution
	HoverInfo notice_res;
	ImageData questionbox;
	Button accept, cancel;

	// If the user wants to input a custom resolution, these are used along with the question box
	TextArea prompt_w, prompt_h;

	// The countdown until the timer resets
	TextData countdown;
	Timer timer;

	// Function to draw the main menu (controls, settings, save, cancel)
	bool HandleTabs(Button &back, const Common::Event &Event);
#if 0
	bool HandleTabs(Button &back, const SDL_Event &Event);
#endif

public:
	bool loaded;

	OptionMenu(void) {
		loaded = false;
		state = STATE_GENERAL;
		menu.UseKeyboard(true);
	}
	~OptionMenu(void) {}

	void reset();
	bool DisableHotkeys() { return keybind.DisableHotkeys(); }

	void load(const Common::String &filename);
	void draw(Button &back);
	bool HandleEvents(Button &back, const Common::Event &Event);
#if 0
	bool HandleEvents(Button &back, const SDL_Event &Event);
#endif
	void internalEvents();

	void SetUI();
	void SaveState();
};

} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_OPTIONMENU_H
