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

//=============================================================================
// Author:   Arvind
// Purpose:  The tray where you have inventory, map and journal icons
//=============================================================================
#ifndef CRAB_HUD_H
#define CRAB_HUD_H

#include "crab/FileMenu.h"
#include "crab/GameEventInfo.h"
#include "crab/GameOverMenu.h"
#include "crab/HealthIndicator.h"
#include "crab/PauseMenu.h"
#include "crab/SlideShow.h"
#include "crab/StateButton.h"
#include "crab/common_header.h"
#include "crab/input.h"
#include "crab/level_objects.h"
#include "crab/talknotify.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
enum HUDSignal { HS_NONE = -1,
				 HS_MAP,
				 HS_PAUSE,
				 HS_CHAR,
				 HS_JOURNAL,
				 HS_INV };

// The world map, inventory and objective buttons are in the button menu
class HUD {
	// The background image
	ImageData bg;

	// The launcher menu for stuff like inventory, character, pause etc
	Menu<StateButton> menu;

	// The health gem thingy
	// HealthIndicator health;

	// Sprite sheet for animated notify icon
	ImageKey notify_anim;

	// Animated notification icon needs a clip sign
	Rect clip;

	// The amount of time to wait before incrementing clip
	Timer timer;

	// The original tooltips as provided in the xml
	std::vector<std::string> tooltip;

public:
	GameOverMenu gom;
	PauseMenu pause;
	pyrodactyl::input::HotKey pausekey;
	Button back;

	HUD() {
		pausekey.Set(pyrodactyl::input::IG_PAUSE);
		notify_anim = 0;
	}
	~HUD() {}

	void InternalEvents(bool ShowMap);
	void PlayerImg(const StateButtonImage &img) { menu.element.at(HS_CHAR).Img(img); }

	void State(const int &val);

	void Load(const std::string &filename, pyrodactyl::level::TalkNotify &tn, pyrodactyl::level::PlayerDestMarker &pdm);
	HUDSignal HandleEvents(pyrodactyl::event::Info &info, const SDL_Event &Event);

	void Draw(pyrodactyl::event::Info &info, const std::string &id);

	// Set the tooltips for the buttons in the menu
	// The tooltips are of the style <Name> (<Hotkey>), with Name being provided by the xml
	void SetTooltip();

	void SetUI();
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_HUD_H
