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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NANCY_STATE_SETUPMENU_H
#define NANCY_STATE_SETUPMENU_H

#include "common/singleton.h"

#include "engines/nancy/state/state.h"

#include "engines/nancy/ui/fullscreenimage.h"

namespace Nancy {

struct SET;

namespace UI {
class Button;
class Toggle;
class Scrollbar;
}

namespace State {

class SetupMenu : public State, public Common::Singleton<SetupMenu> {
public:
	SetupMenu() : _state(kInit), _exitButton(nullptr), _setupData(nullptr) {}
	virtual ~SetupMenu();

	// State API
	void process() override;
	void onStateEnter(const NancyState::NancyState prevState) override;
	bool onStateExit(const NancyState::NancyState nextState) override;

private:
	void init();
	void run();
	void stop();

	void registerGraphics();

	enum State { kInit, kRun, kStop };

	UI::FullScreenImage _background;
	State _state;

	Common::Array<UI::Toggle *> _toggles;
	Common::Array<UI::Scrollbar *> _scrollbars;
	UI::Button *_exitButton;

	SET *_setupData;
};

} // End of namespace State
} // End of namespace Nancy

#endif // NANCY_STATE_SETUPMENU_H
