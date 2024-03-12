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

#ifndef NANCY_STATE_SAVEDIALOG_H
#define NANCY_STATE_SAVEDIALOG_H

#include "common/singleton.h"

#include "engines/nancy/state/state.h"

#include "engines/nancy/ui/fullscreenimage.h"

namespace Nancy {

struct SDLG;

namespace UI {
class Button;
}

namespace State {

class SaveDialog : public State, public Common::Singleton<SaveDialog> {
public:
	SaveDialog() : _state(kInit), _yesButton(nullptr), _noButton(nullptr), _cancelButton(nullptr), _selected(-1), _dialogData(nullptr) {}
	virtual ~SaveDialog();

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
	int _selected;

	UI::Button *_yesButton;
	UI::Button *_noButton;
	UI::Button *_cancelButton;

	const SDLG::Dialog *_dialogData;
};

} // End of namespace State
} // End of namespace Nancy

#endif // NANCY_STATE_MAINMENU_H
