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

#ifndef NANCY_STATE_LOADSAVE_H
#define NANCY_STATE_LOADSAVE_H

#include "common/singleton.h"

#include "engines/nancy/state/state.h"
#include "engines/nancy/ui/fullscreenimage.h"
#include "engines/nancy/font.h"

namespace Nancy {

struct LOAD;

namespace UI {
class Button;
}

namespace State {

class LoadSaveMenu : public State, public Common::Singleton<LoadSaveMenu> {
public:
	LoadSaveMenu() :
		_state(kInit), _selectedSave(-1), _enteringNewState(false), _nextBlink(0),
		_baseFont(nullptr), _highlightFont(nullptr),
		_disabledFont(nullptr), _loadSaveData(nullptr),
		_cancelButton(nullptr), _exitButton(nullptr),
		_blinkingCursorOverlay(6), _successOverlay(8) {}
	virtual ~LoadSaveMenu();

	// State API
	void process() override;
	void onStateEnter(const NancyState::NancyState prevState) override;
	bool onStateExit(const NancyState::NancyState nextState) override;

private:
	void init();
	void run();
	void enterFilename();
	void save();
	void load();
	void success();
	void stop();

	void registerGraphics();

	uint16 writeToTextbox(uint textboxID, const Common::String &text, const Font *font);

	enum State { kInit, kRun, kEnterFilename, kSave, kLoad, kSuccess, kStop };

	State _state;

	UI::FullScreenImage _background;

	const Font *_baseFont;
	const Font *_highlightFont;
	const Font *_disabledFont;

	Common::Array<Common::String> _filenameStrings;
	Common::Array<bool> _saveExists;
	Common::String _enteredString;

	Common::Array<RenderObject *> _textboxes;
	Common::Array<UI::Button *> _loadButtons;
	Common::Array<UI::Button *> _saveButtons;
	Common::Array<RenderObject *> _cancelButtonOverlays;
	UI::Button *_exitButton;
	UI::Button *_cancelButton;
	RenderObject _blinkingCursorOverlay;
	RenderObject _successOverlay;

	int16 _selectedSave;
	bool _enteringNewState;
	uint32 _nextBlink;

	LOAD *_loadSaveData;
};

} // End of namespace State
} // End of namespace Nancy

#endif // NANCY_STATE_LOADSAVE_H
