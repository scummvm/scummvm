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
#include "common/ptr.h"

#include "engines/nancy/state/state.h"
#include "engines/nancy/ui/fullscreenimage.h"
#include "engines/nancy/ui/button.h"
#include "engines/nancy/font.h"

namespace Nancy {

struct LOAD;

namespace State {

class LoadSaveMenu : public State, public Common::Singleton<LoadSaveMenu> {
public:
	virtual ~LoadSaveMenu();

protected:
	enum State { kInit, kRun, kEnterFilename, kSave, kLoad, kSuccess, kStop };

	LoadSaveMenu() :
		_blinkingCursorOverlay(6),
		_successOverlay(8) {};

	virtual void init() = 0;
	virtual void run() = 0;
	virtual void enterFilename();
	virtual bool save();
	virtual void load();
	virtual void success();
	virtual void stop();

	virtual int scummVMSaveSlotToLoad() const = 0;

	virtual void registerGraphics();

	virtual uint16 writeToTextbox(int textboxID, const Common::String &text, const Font *font);

	void scummVMSave();
	void scummVMLoad();

	// State API
	void process() override;
	void onStateEnter(const NancyState::NancyState prevState) override;
	bool onStateExit(const NancyState::NancyState nextState) override;

	const Font *_baseFont = nullptr;
	const Font *_highlightFont = nullptr;
	const Font *_disabledFont = nullptr;

	State _state = kInit;
	bool _enteringNewState = true;
	bool _destroyOnExit = true;
	const LOAD *_loadSaveData = nullptr;

	int16 _selectedSave = -1;
	RenderObject *_textboxReceivingInput = nullptr;

	// UI elements common to both menus
	Common::Array<Common::ScopedPtr<RenderObject>> _textboxes;
	Common::ScopedPtr<UI::Button> _exitButton;

	RenderObject _blinkingCursorOverlay;
	RenderObject _successOverlay;

	uint32 _nextBlink = 0;
	Common::String _enteredString;
};

class LoadSaveMenu_V1 : public LoadSaveMenu {
private:
	void init() override;
	void run() override;
	void enterFilename() override;
	bool save() override;

	virtual int scummVMSaveSlotToLoad() const override;

	void registerGraphics() override;

	UI::FullScreenImage _background;

	Common::Array<Common::String> _filenameStrings;
	Common::Array<bool> _saveExists;
	Common::Array<Common::ScopedPtr<UI::Button>> _loadButtons;
	Common::Array<Common::ScopedPtr<UI::Button>> _saveButtons;
	Common::Array<Common::ScopedPtr<RenderObject>> _cancelButtonOverlays;
	Common::ScopedPtr<UI::Button> _cancelButton;
};

class LoadSaveMenu_V2 : public LoadSaveMenu {
private:
	void init() override;
	void run() override;
	void enterFilename() override;
	bool save() override;
	void success() override;

	virtual int scummVMSaveSlotToLoad() const override;

	void registerGraphics() override;

	uint16 writeToTextbox(int textboxID, const Common::String &text, const Font *font) override;
	uint16 writeToInputTextbox(const Font *font);

	void filterAndSortSaveStates();
	void extractSaveNames(uint pageID);
	void goToPage(uint pageID);
	void reloadSaves();
	void setConfig();

	UI::FullScreenImage _background1;
	UI::FullScreenImage _background2;
	Graphics::ManagedSurface _buttonsImage;

	Common::Array<Common::String> _filenameStrings;
	Common::Array<bool> _saveExists;

	Common::ScopedPtr<RenderObject> _inputTextbox;
	Common::ScopedPtr<UI::Button> _loadButton;
	Common::ScopedPtr<UI::Button> _saveButton;
	Common::ScopedPtr<UI::Button> _pageUpButton;
	Common::ScopedPtr<UI::Button> _pageDownButton;

	int16 _hoveredSave = -1;
	uint _currentPage = 0;

	SaveStateList _sortedSavesList;
};

} // End of namespace State
} // End of namespace Nancy

namespace Common {

template<>
Nancy::State::LoadSaveMenu *Singleton<Nancy::State::LoadSaveMenu>::makeInstance();

} // End of namespace Common

#endif // NANCY_STATE_LOADSAVE_H
