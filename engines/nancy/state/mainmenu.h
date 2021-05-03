/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef NANCY_STATE_MAINMENU_H
#define NANCY_STATE_MAINMENU_H

#include "common/singleton.h"

#include "engines/nancy/state/state.h"

#include "engines/nancy/ui/fullscreenimage.h"

namespace Nancy {
namespace State {

class MainMenu : public State, public Common::Singleton<MainMenu> {
	friend class MainMenuButton;
public:
	MainMenu() : _state(kInit), _selected(-1), _playedOKSound(false), _buttonDown(_background, 5) {}

	// State API
	virtual void process() override;
	virtual void onStateExit() override;

private:
	void init();
	void run();
	void stop();

	enum State { kInit, kRun, kStop };

	UI::FullScreenImage _background;
	RenderObject _buttonDown;
	State _state;
	int16 _selected;
	bool _playedOKSound;

	Common::Array<Common::Rect> _destRects;
	Common::Array<Common::Rect> _srcRects;
};

} // End of namespace State
} // End of namespace Nancy

#endif // NANCY_STATE_MAINMENU_H
