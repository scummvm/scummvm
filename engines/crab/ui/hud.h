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

#include "crab/ui/FileMenu.h"
#include "crab/event/GameEventInfo.h"
#include "crab/ui/GameOverMenu.h"
#include "crab/ui/HealthIndicator.h"
#include "crab/ui/PauseMenu.h"
#include "crab/ui/SlideShow.h"
#include "crab/ui/StateButton.h"
#include "crab/common_header.h"
#include "crab/input/input.h"
#include "crab/level/level_objects.h"
#include "crab/level/talknotify.h"

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
	Common::Array<Common::String> tooltip;

public:
	GameOverMenu gom;
	PauseMenu pause;
	pyrodactyl::input::HotKey pausekey;
	Button back;

	HUD() {
		pausekey.set(pyrodactyl::input::IG_PAUSE);
		notify_anim = 0;
	}
	~HUD() {}

	void internalEvents(bool ShowMap);
	void PlayerImg(const StateButtonImage &img) { menu.element[HS_CHAR].img(img); }

	void State(const int &val);

	void load(const Common::String &filename, pyrodactyl::level::TalkNotify &tn, pyrodactyl::level::PlayerDestMarker &pdm);

	HUDSignal handleEvents(pyrodactyl::event::Info &info, const Common::Event &Event);
#if 0
	HUDSignal handleEvents(pyrodactyl::event::Info &info, const SDL_Event &Event);
#endif

	void draw(pyrodactyl::event::Info &info, const Common::String &id);

	// Set the tooltips for the buttons in the menu
	// The tooltips are of the style <Name> (<Hotkey>), with Name being provided by the xml
	void SetTooltip();

	void setUI();
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_HUD_H
