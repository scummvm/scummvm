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
#ifndef PELROCK_CDPLAYER_H
#define PELROCK_CDPLAYER_H

#include "common/rect.h"
#include "common/str.h"
#include "graphics/managed_surface.h"

#include "pelrock/pelrock.h"

namespace Pelrock {

class CDPlayer {

	enum CDControls {
		STOP_BUTTON,
		PAUSE_BUTTON,
		PLAY_BUTTON,
		PREVIOUS_BUTTON,
		NEXT_BUTTON,
		NO_CDBUTTON
	};

public:
	CDPlayer(PelrockEventManager *eventMan, ResourceManager *res, SoundManager *sound);
	~CDPlayer();

	void run();

private:
	void init();
	void loadTrackNames();
	void drawScreen();
	void drawButtons();
	void loadBackground();
	void loadControls();
	void checkMouse(int x, int y);
	void cleanup();
	CDControls isButtonClicked(int x, int y);

	ResourceManager *_res;
	SoundManager *_sound;
	PelrockEventManager *_events;
	Graphics::ManagedSurface _backgroundScreen;
	Graphics::ManagedSurface _compositeScreen;
	byte *_palette;
	byte *_controls;
	Common::String trackNames[31];
	byte *buttons[5][2];
	Common::Rect _buttonRects[5] = {
		Common::Rect(Common::Point(17, 46), 37, 26),  // Stop
		Common::Rect(Common::Point(57, 48), 33, 23),  // Pause
		Common::Rect(Common::Point(92, 44), 34, 28),  // Play
		Common::Rect(Common::Point(128, 45), 38, 24), // Previous
		Common::Rect(Common::Point(168, 44), 41, 28)  // Next
	};
	int _selectedTrack = 2;
	CDControls _selectedButton = NO_CDBUTTON;
};

} // End of namespace Pelrock

#endif
