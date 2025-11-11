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

#include "engines/nancy/nancy.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/graphics.h"

#include "engines/nancy/state/loadsave.h"
#include "engines/nancy/state/scene.h"
#include "engines/nancy/ui/button.h"

#include "common/config-manager.h"
#include "common/translation.h"

#include "gui/message.h"
#include "gui/saveload.h"

namespace Common {
DECLARE_SINGLETON(Nancy::State::LoadSaveMenu);

template<>
Nancy::State::LoadSaveMenu *Singleton<Nancy::State::LoadSaveMenu>::makeInstance() {
	if (Nancy::g_nancy->getGameType() <= Nancy::kGameTypeNancy7) {
		return new Nancy::State::LoadSaveMenu_V1();
	} else {
		return new Nancy::State::LoadSaveMenu_V2();
	}
}

}

namespace Nancy {
namespace State {

LoadSaveMenu::~LoadSaveMenu() {
	g_nancy->_input->setVKEnabled(false);
}

void LoadSaveMenu::process() {
	if (g_nancy->_sound->isSoundPlaying("BUOK") || g_nancy->_sound->isSoundPlaying("BULS") || g_nancy->_sound->isSoundPlaying("BUDE")) {
		return;
	}

	switch (_state) {
	case kInit:
		init();
		// fall through
	case kRun:
		run();
		break;
	case kEnterFilename:
		enterFilename();
		break;
	case kSave:
		save();
		break;
	case kLoad:
		load();
		break;
	case kSuccess:
		success();
		break;
	default:
		break;
	}

	// Make sure stop runs on the same frame
	if (_state == kStop) {
		stop();
	}

	g_nancy->_cursor->setCursorType(CursorManager::kNormalArrow);
}

void LoadSaveMenu::onStateEnter(const NancyState::NancyState prevState) {
	if (!ConfMan.getBool("originalsaveload")) {
		bool saveAndQuit = false;
		if (ConfMan.hasKey("sdlg_save_and_quit", Common::ConfigManager::kTransientDomain)) {
			saveAndQuit = ConfMan.getBool("sdlg_save_and_quit", Common::ConfigManager::kTransientDomain);
			ConfMan.removeKey("sdlg_save_and_quit", Common::ConfigManager::kTransientDomain);
		}

		// Display the question dialog if we are in a scene, and if we are not
		// in the middle of quitting the game, and a save has been requested
		if (Nancy::State::Scene::hasInstance() && !saveAndQuit) {
			GUI::MessageDialog saveOrLoad(_("Would you like to load or save a game?"), _("Load"), _("Save"));

			int choice = saveOrLoad.runModal();
			if (choice == GUI::kMessageOK)
				scummVMLoad();
			else
				scummVMSave();
		} else if (saveAndQuit) {
			scummVMSave();
		} else {
			scummVMLoad();
		}

		return;
	}

	if (_state == kEnterFilename) {
		g_nancy->_input->setVKEnabled(true);
	}
	registerGraphics();
}

bool LoadSaveMenu::onStateExit(const NancyState::NancyState nextState) {
	g_nancy->_input->setVKEnabled(false);
	return _destroyOnExit;
}

void LoadSaveMenu::scummVMSave() {
	GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"), true);
	int slot = dialog->runModalWithCurrentTarget();
	Common::String saveName = dialog->getResultString();
	delete dialog;

	g_nancy->_graphics->suppressNextDraw();
	_destroyOnExit = true;
	_state = kStop;
	_selectedSave = -1; // so that we return to the main menu after saving

	if (slot >= 0) {
		g_nancy->saveGameState(slot, saveName, false);
		g_nancy->_hasJustSaved = true;
	}
}

void LoadSaveMenu::scummVMLoad() {
	GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Load game:"), _("Load"), false);
	int slot = dialog->runModalWithCurrentTarget();
	delete dialog;

	g_nancy->_graphics->suppressNextDraw();
	_destroyOnExit = true;
	_state = kStop;
	_selectedSave = slot;

	if (slot >= 0) {
		if (Nancy::State::Scene::hasInstance())
			Nancy::State::Scene::destroy();

		ConfMan.setInt("save_slot", slot, Common::ConfigManager::kTransientDomain);

		_enteringNewState = true;
	}
}

void LoadSaveMenu::enterFilename() {
	// Handle keyboard input and cursor blinking
	uint32 gameTime = g_nancy->getTotalPlayTime();
	if (_loadSaveData->_blinkingTimeDelay != 0 && gameTime > _nextBlink) {
		_blinkingCursorOverlay.setVisible(!_blinkingCursorOverlay.isVisible());
		_nextBlink = gameTime + _loadSaveData->_blinkingTimeDelay;
	}

	NancyInput input = g_nancy->_input->getInput();

	for (uint i = 0; i < input.otherKbdInput.size(); ++i) {
		Common::KeyState &key = input.otherKbdInput[i];
		if (key.keycode == Common::KEYCODE_BACKSPACE) {
			if (_enteredString.size()) {
				_enteredString.deleteLastChar();
			}
		} else if (Common::isAlnum(key.ascii) || Common::isSpace(key.ascii)) {
			_enteredString += key.ascii;
		}
	}

	if (_exitButton && !_exitButton->_isDisabled) {
		_exitButton->handleInput(input);
		if (_exitButton->_isClicked) {
			_state = kStop;
			g_nancy->_sound->playSound("BUOK");
			g_nancy->_input->setVKEnabled(false);
			return;
		}
	}
}

void LoadSaveMenu::load() {
	auto *sdlg = GetEngineData(SDLG);

	if (sdlg && sdlg->dialogs.size() > 1 && Nancy::State::Scene::hasInstance() && !g_nancy->_hasJustSaved) {
		// nancy6 added a "Do you want load without saving" dialog.
		if (!ConfMan.hasKey("sdlg_return", Common::ConfigManager::kTransientDomain)) {
			// Request the dialog
			ConfMan.setInt("sdlg_id", 2, Common::ConfigManager::kTransientDomain);
			_destroyOnExit = false;
			g_nancy->setState(NancyState::kSaveDialog);
			return;
		} else {
			// Dialog has returned
			_destroyOnExit = true;
			g_nancy->_graphics->suppressNextDraw();
			uint ret = ConfMan.getInt("sdlg_return", Common::ConfigManager::kTransientDomain);
			ConfMan.removeKey("sdlg_return", Common::ConfigManager::kTransientDomain);
			switch (ret) {
			case 1 :
				// "No" keeps us in the LoadSave state but doesn't load
				_state = kRun;
				return;
			case 2 :
				// "Cancel" returns to the main menu
				g_nancy->setState(NancyState::kMainMenu);
				return;
			default:
				// "Yes" actually loads
				break;
			}
		}
	}

	if (Nancy::State::Scene::hasInstance()) {
		Nancy::State::Scene::destroy();
	}

	ConfMan.setInt("save_slot", scummVMSaveSlotToLoad(), Common::ConfigManager::kTransientDomain);
	ConfMan.setInt("display_slot", scummVMSaveSlotToLoad(), Common::ConfigManager::kTransientDomain); // Used to load the save name

	_state = kStop;
	_enteringNewState = true;
}

void LoadSaveMenu::success() {
	if (_enteringNewState) {
		_nextBlink = g_nancy->getTotalPlayTime() + 2000; // Hardcoded
		_successOverlay.setVisible(true);
		_enteringNewState = false;
	}

	if (g_nancy->getTotalPlayTime() > _nextBlink) {
		_state = kStop;
		_selectedSave = 0;

		_enteringNewState = true;
	}
}

void LoadSaveMenu::stop() {
	if (_selectedSave != -1) {
		g_nancy->setState(NancyState::kScene);
	} else {
		g_nancy->setState(NancyState::kMainMenu);
	}
}

void LoadSaveMenu::registerGraphics() {
	for (auto &tb : _textboxes) {
		tb->registerGraphics();
	}

	if (_exitButton) {
		_exitButton->registerGraphics();
	}
}

uint16 LoadSaveMenu::writeToTextbox(int textboxID, const Common::String &text, const Font *font) {
	assert(font);

	_textboxes[textboxID]->_drawSurface.clear(g_nancy->_graphics->getTransColor());
	Common::Point destPoint(_loadSaveData->_fontXOffset, _loadSaveData->_fontYOffset + _textboxes[textboxID]->_drawSurface.h - font->getFontHeight());
	font->drawString(&_textboxes[textboxID]->_drawSurface, text, destPoint.x, destPoint.y, _textboxes[textboxID]->_drawSurface.w, 0);
	_textboxes[textboxID]->setVisible(true);

	return font->getStringWidth(text);
}

void LoadSaveMenu_V1::registerGraphics() {
	LoadSaveMenu::registerGraphics();

	_background.registerGraphics();

	for (auto &button : _loadButtons) {
		button->registerGraphics();
	}

	for (auto &button : _saveButtons) {
		button->registerGraphics();
	}

	for (auto &overlay : _cancelButtonOverlays) {
		overlay->registerGraphics();
	}

	if (_cancelButton) {
		_cancelButton->registerGraphics();
	}

	_blinkingCursorOverlay.registerGraphics();
	_successOverlay.registerGraphics();

	g_nancy->_graphics->redrawAll();
}

void LoadSaveMenu_V1::init() {
	_loadSaveData = GetEngineData(LOAD);
	assert(_loadSaveData);

	_background.init(_loadSaveData->_image1Name);

	_baseFont = g_nancy->_graphics->getFont(_loadSaveData->_mainFontID);

	if (_loadSaveData->_highlightFontID != -1) {
		_highlightFont = g_nancy->_graphics->getFont(_loadSaveData->_highlightFontID);
		_disabledFont = g_nancy->_graphics->getFont(_loadSaveData->_disabledFontID);
	} else {
		_highlightFont = _disabledFont = _baseFont;
	}

	_filenameStrings.resize(_loadSaveData->_textboxBounds.size());
	_saveExists.resize(_filenameStrings.size(), false);
	_textboxes.resize(_filenameStrings.size());
	for (uint i = 0; i < _textboxes.size(); ++i) {
		// Load textbox objects
		RenderObject *newTb = new RenderObject(5);
		_textboxes[i].reset(newTb);
		const Common::Rect &bounds = _loadSaveData->_textboxBounds[i];
		newTb->_drawSurface.create(bounds.width(), bounds.height(), g_nancy->_graphics->getScreenPixelFormat());
		newTb->_drawSurface.clear(g_nancy->_graphics->getTransColor());
		newTb->moveTo(bounds);
		newTb->setTransparent(true);
		newTb->setVisible(true);
		newTb->init();

		// Check for valid save and write its name in the textbox
		SaveStateDescriptor desc = g_nancy->getMetaEngine()->querySaveMetaInfos(ConfMan.getActiveDomainName().c_str(), i + 1);
		if (desc.isValid()) {
			_saveExists[i] = true;
			_filenameStrings[i] = desc.getDescription();
		} else {
			// If no valid save, copy over the empty save string
			if (_loadSaveData->_emptySaveText.size()) {
				_filenameStrings[i] = _loadSaveData->_emptySaveText;
			} else {
				_filenameStrings[i] = g_nancy->getStaticData().emptySaveText;
			}
		}
	}

	bool hasHighlights = _loadSaveData->_loadButtonHighlightSrcs.size();

	_loadButtons.resize(_textboxes.size());
	_saveButtons.resize(_textboxes.size());
	_cancelButtonOverlays.resize(_textboxes.size());
	for (uint i = 0; i < _loadButtons.size(); ++i) {
		// Load Save and Load buttons, and Cancel overlays
		_loadButtons[i].reset(new UI::Button(1, _background._drawSurface,
			_loadSaveData->_loadButtonDownSrcs[i], _loadSaveData->_loadButtonDests[i],
			hasHighlights ? _loadSaveData->_loadButtonHighlightSrcs[i] : Common::Rect(),
			hasHighlights ? _loadSaveData->_loadButtonDisabledSrcs[i] : Common::Rect()));

		_saveButtons[i].reset(new UI::Button(1, _background._drawSurface,
			_loadSaveData->_saveButtonDownSrcs[i], _loadSaveData->_saveButtonDests[i],
			hasHighlights ? _loadSaveData->_saveButtonHighlightSrcs[i] : Common::Rect(),
			hasHighlights ? _loadSaveData->_saveButtonDisabledSrcs[i] : Common::Rect()));

		_cancelButtonOverlays[i].reset(new RenderObject(2, _background._drawSurface,
			_loadSaveData->_cancelButtonSrcs[i], _loadSaveData->_cancelButtonDests[i]));

		_loadButtons[i]->init();
		_saveButtons[i]->init();
		_cancelButtonOverlays[i]->init();
	}

	// Load exit button
	_exitButton.reset(new UI::Button(3, _background._drawSurface,
			_loadSaveData->_doneButtonDownSrc, _loadSaveData->_doneButtonDest,
			hasHighlights ? _loadSaveData->_doneButtonHighlightSrc : Common::Rect(),
			hasHighlights ? _loadSaveData->_doneButtonDisabledSrc : Common::Rect()));

	// Load Cancel button that activates when typing a filename
	// Note: this is only responsible for the hover/mouse down/disabled graphic;
	// the graphics that replace the Save buttons with Cancel are their own RenderObject.
	// We also make sure this has an invalid position until we need it.
	Common::Rect pos = _loadSaveData->_cancelButtonDests[0];
	pos.moveTo(-500, 0);
	_cancelButton.reset(new UI::Button(3, _background._drawSurface,
		_loadSaveData->_cancelButtonDownSrc, Common::Rect(),
		_loadSaveData->_cancelButtonHighlightSrc, _loadSaveData->_cancelButtonDisabledSrc));

	// Load the blinking cursor graphic that appears while typing a filename
	_blinkingCursorOverlay._drawSurface.create(_loadSaveData->_blinkingCursorSrc.width(),
		_loadSaveData->_blinkingCursorSrc.height(),
		g_nancy->_graphics->getScreenPixelFormat());
	_blinkingCursorOverlay.setTransparent(true);
	_blinkingCursorOverlay.setVisible(false);
	_blinkingCursorOverlay._drawSurface.clear(_blinkingCursorOverlay._drawSurface.getTransparentColor());
	_blinkingCursorOverlay._drawSurface.transBlitFrom(_highlightFont->getImageSurface(), _loadSaveData->_blinkingCursorSrc,
		Common::Point(), g_nancy->_graphics->getTransColor());

	// Load the "Your game has been saved" popup graphic
	if (!_loadSaveData->_gameSavedPopup.empty()) {
		g_nancy->_resource->loadImage(_loadSaveData->_gameSavedPopup, _successOverlay._drawSurface);
		Common::Rect destBounds = Common::Rect(0,0, _successOverlay._drawSurface.w, _successOverlay._drawSurface.h);
		destBounds.moveTo(640 / 2 - destBounds.width() / 2,
			480 / 2 - destBounds.height() / 2);
		_successOverlay.moveTo(destBounds);
		_successOverlay.setVisible(false);
	}

	registerGraphics();

	_state = kRun;
	_enteringNewState = true;
}

void LoadSaveMenu_V1::run() {
	if (_enteringNewState) {
		// State has changed, revert all relevant objects to an appropriate state
		for (uint i = 0; i < _textboxes.size(); ++i) {
			writeToTextbox(i, _filenameStrings[i], Nancy::State::Scene::hasInstance() ? _baseFont : _disabledFont);

			// Set load button state depending on whether there exists a save in the corresponding slot
			// Save buttons are always active
			_loadButtons[i]->setDisabled(_saveExists[i] == false);
			_saveButtons[i]->setDisabled(!Nancy::State::Scene::hasInstance());
			_cancelButtonOverlays[i]->setVisible(false);

			_loadButtons[i]->_isClicked = false;
			_saveButtons[i]->_isClicked = false;
		}

		if (_cancelButton) {
			_cancelButton->_isClicked = false;
			_cancelButton->setDisabled(true);
			_cancelButton->moveTo(Common::Point(-500, 0));
		}

		_blinkingCursorOverlay.setVisible(false);
		_exitButton->setDisabled(false);
		_enteredString.clear();
		_successOverlay.setVisible(false);

		_selectedSave = -1;
		_enteringNewState = false;
	}

	// Handle input
	NancyInput input = g_nancy->_input->getInput();

	for (uint i = 0; i < _loadButtons.size(); ++i) {
		_loadButtons[i]->handleInput(input);

		if (_loadButtons[i]->_isClicked) {
			if (_saveExists[i]) {
				_state = kLoad;
				_enteringNewState = true;
				_selectedSave = i;
				g_nancy->_sound->playSound("BULS");
			} else if (!_loadSaveData->_saveButtonHighlightSrcs.size()) {
				// TVD and nancy1 buttons get pushed and play error sound
				_loadButtons[i]->setVisible(true);
				g_nancy->_sound->playSound("BUDE");
				_enteringNewState = true;
			}

			return;
		}
	}

	for (uint i = 0; i < _saveButtons.size(); ++i) {
		_saveButtons[i]->handleInput(input);

		if (_saveButtons[i]->_isClicked) {
			if (Nancy::State::Scene::hasInstance()) {
				_state = kSave;
				_enteringNewState = true;
				_selectedSave = i;
				g_nancy->_sound->playSound("BULS");
			} else if (!_loadSaveData->_saveButtonHighlightSrcs.size()) {
				// TVD and nancy1 buttons get pushed and play error sound
				_saveButtons[i]->setVisible(true);
				g_nancy->_sound->playSound("BUDE");
				_enteringNewState = true;
			}

			return;
		}
	}

	// Handle textbox hovering
	bool hoversOverTextbox = false;
	for (int i = 0; i < (int)_textboxes.size(); ++i) {
		if (_textboxes[i]->getScreenPosition().contains(input.mousePos)) {
			if (Nancy::State::Scene::hasInstance()) {
				hoversOverTextbox = true;
				if (_selectedSave != i) {
					if (_selectedSave != -1) {
						writeToTextbox(_selectedSave, _filenameStrings[_selectedSave], _baseFont);
					}

					_selectedSave = i;
					writeToTextbox(_selectedSave, _filenameStrings[_selectedSave], _highlightFont);
				}

				if (input.input & NancyInput::kLeftMouseButtonUp) {
					_state = kEnterFilename;
					_enteringNewState = true;
					g_nancy->_sound->playSound("BUOK");
					_selectedSave = i;
					return;
				}

				break;
			} else if (!_loadSaveData->_saveButtonHighlightSrcs.size()) {
				if (input.input & NancyInput::kLeftMouseButtonUp) {
					// TVD and nancy1 textboxes play error sound when no Scene is active
					g_nancy->_sound->playSound("BUDE");
				}
			}
		}
	}

	if (!hoversOverTextbox && _selectedSave != -1) {
		writeToTextbox(_selectedSave, _filenameStrings[_selectedSave], _baseFont);
		_selectedSave = -1;
	}

	// Check Done button
	if (_exitButton) {
		_exitButton->handleInput(input);

		if (_exitButton->_isClicked) {
			_state = kStop;
			g_nancy->_sound->playSound("BUOK");
			return;
		}
	}
}

void LoadSaveMenu_V1::enterFilename() {
	if (_enteringNewState) {
		// State has changed, revert all relevant objects to an appropriate state
		if (_cancelButton) {
			_cancelButton->setDisabled(false);
			_cancelButton->moveTo(_loadSaveData->_cancelButtonDests[_selectedSave]);
		}

		for (int i = 0; i < (int)_textboxes.size(); ++i) {
			bool sel = i == _selectedSave;
			writeToTextbox(i, sel ? Common::String() : _filenameStrings[i], sel ? _highlightFont : _disabledFont);
			_loadButtons[i]->setDisabled(true);

			if (i != _selectedSave) {
				_saveButtons[i]->setDisabled(true);
			}
		}

		_exitButton->setDisabled(true);
		_cancelButtonOverlays[_selectedSave]->setVisible(true);

		// Set up blinking cursor (doesn't blink in TVD)
		Common::Rect tbPosition = _textboxes[_selectedSave]->getScreenPosition();
		Common::Rect cursorRect = _blinkingCursorOverlay._drawSurface.getBounds();
		cursorRect.moveTo(tbPosition.left, tbPosition.bottom - _blinkingCursorOverlay._drawSurface.h + _loadSaveData->_fontYOffset);
		_blinkingCursorOverlay.moveTo(cursorRect);
		_blinkingCursorOverlay.setVisible(true);
		_nextBlink = g_nancy->getTotalPlayTime() + _loadSaveData->_blinkingTimeDelay;
		_enteringNewState = false;
		g_nancy->_input->setVKEnabled(true);
	}

	LoadSaveMenu::enterFilename();

	// Handle input
	NancyInput input = g_nancy->_input->getInput();

	bool enterKeyPressed = false;
	if (input.otherKbdInput.size()) {
		uint16 textWidthInPixels = writeToTextbox(_selectedSave, _enteredString, _highlightFont);
		Common::Rect tbPosition = _textboxes[_selectedSave]->getScreenPosition();
		Common::Rect lastCursorPosition = _blinkingCursorOverlay.getScreenPosition();
		_blinkingCursorOverlay.moveTo(Common::Point(tbPosition.left + textWidthInPixels,
			lastCursorPosition.top));

		if (	input.otherKbdInput.back().keycode == Common::KEYCODE_RETURN ||
				input.otherKbdInput.back().keycode == Common::KEYCODE_KP_ENTER) {
			enterKeyPressed = true;
		}
	}

	if (_state != kEnterFilename) {
		return;
	}

	_cancelButton->handleInput(input);
	if (_cancelButton->_isClicked) {
		_state = kRun;
		_enteringNewState = true;
		g_nancy->_sound->playSound("BULS");
		g_nancy->_input->setVKEnabled(false);
		return;
	}

	_saveButtons[_selectedSave]->handleInput(input);
	if (_saveButtons[_selectedSave]->_isClicked || enterKeyPressed) {
		_state = kSave;
		_enteringNewState = true;
		g_nancy->_sound->playSound("BULS");
		g_nancy->_input->setVKEnabled(false);
		return;
	}
}

void LoadSaveMenu_V1::save() {
	auto *sdlg = GetEngineData(SDLG);

	if (sdlg && sdlg->dialogs.size() > 1) {
		// nancy6 added a "Do you want to overwrite this save" dialog.
		// First, check if we are actually overwriting
		SaveStateDescriptor desc = g_nancy->getMetaEngine()->querySaveMetaInfos(ConfMan.getActiveDomainName().c_str(), _selectedSave + 1);
		if (desc.isValid()) {
			if (!ConfMan.hasKey("sdlg_return", Common::ConfigManager::kTransientDomain)) {
				// Request the dialog
				ConfMan.setInt("sdlg_id", 1, Common::ConfigManager::kTransientDomain);
				_destroyOnExit = false;
				g_nancy->setState(NancyState::kSaveDialog);
				return;
			} else {
				// Dialog has returned
				g_nancy->_graphics->suppressNextDraw();
				_destroyOnExit = true;
				uint ret = ConfMan.getInt("sdlg_return", Common::ConfigManager::kTransientDomain);
				ConfMan.removeKey("sdlg_return", Common::ConfigManager::kTransientDomain);
				switch (ret) {
				case 1 :
					// "No" keeps us in the LoadSave state but doesn't save
					_state = kRun;
					return;
				case 2 :
					// "Cancel" returns to the main menu
					g_nancy->setState(NancyState::kMainMenu);
					return;
				default:
					// "Yes" actually saves
					break;
				}
			}
		}
	}

	// Improvement: not providing a name doesn't result in the
	// savefile being named "--- Empty ---" or "Nothing Saved Here".
	// Instead, we use ScummVM's built-in save name generator

	// This does not apply to nancy7, where a default name is provided in
	// the LOAD chunk, and has a number appended to the end

	Common::String finalDesc = _enteredString;
	if (!finalDesc.size()) {
		if (_loadSaveData->_defaultSaveNamePrefix.size()) {
			if (_filenameStrings[_selectedSave].equals(_loadSaveData->_emptySaveText)) {
				uint suffixNum = 1;
				for (int i = 1; i < g_nancy->getMetaEngine()->getMaximumSaveSlot(); ++i) {
					if (i == _selectedSave + 1) {
						continue;
					}

					SaveStateDescriptor desc = g_nancy->getMetaEngine()->querySaveMetaInfos(ConfMan.getActiveDomainName().c_str(), i);
					if (desc.getDescription().substr(0, _loadSaveData->_defaultSaveNamePrefix.size()).equals(Common::U32String(_loadSaveData->_defaultSaveNamePrefix))) {
						if (desc.getDescription().substr(_loadSaveData->_defaultSaveNamePrefix.size(), 1).asUint64() == suffixNum) {
							++suffixNum;
						} else {
							break;
						}
					}
				}

				finalDesc = _loadSaveData->_defaultSaveNamePrefix + ('0' + suffixNum);
			} else {
				finalDesc = _filenameStrings[_selectedSave];
			}
		} else {
			if (!_filenameStrings[_selectedSave].equals(g_nancy->getStaticData().emptySaveText)) {
				finalDesc = _filenameStrings[_selectedSave];
			}
		}
	}

	g_nancy->saveGameState(_selectedSave + 1, finalDesc, false);

	// Feed the new name back into the list of saves
	SaveStateDescriptor desc = g_nancy->getMetaEngine()->querySaveMetaInfos(ConfMan.getActiveDomainName().c_str(), _selectedSave + 1);
	if (desc.isValid()) {
		_filenameStrings[_selectedSave] = desc.getDescription();
	}

	if (_successOverlay._drawSurface.empty()) {
		_state = kRun;
		_enteringNewState = true;
	} else {
		_state = kSuccess;
		_enteringNewState = true;
	}

	_saveExists[_selectedSave] = true;
	g_nancy->_hasJustSaved = true;
}

int LoadSaveMenu_V1::scummVMSaveSlotToLoad() const {
	return _selectedSave + 1;
}

enum { kInputTextboxIndex = -2 };

void LoadSaveMenu_V2::registerGraphics() {
	LoadSaveMenu::registerGraphics();

	_background1.registerGraphics();
	_background2.registerGraphics();

	if (_loadButton) {
		_loadButton->registerGraphics();
	}

	if (_saveButton) {
		_saveButton->registerGraphics();
	}

	if (_exitButton) {
		_exitButton->registerGraphics();
	}

	if (_pageUpButton) {
		_pageUpButton->registerGraphics();
	}

	if (_pageDownButton) {
		_pageDownButton->registerGraphics();
	}

	if (_inputTextbox) {
		_inputTextbox->registerGraphics();
	}

	_blinkingCursorOverlay.registerGraphics();
	_successOverlay.registerGraphics();

	g_nancy->_graphics->redrawAll();
}

void LoadSaveMenu_V2::init() {
	_loadSaveData = GetEngineData(LOAD);
	assert(_loadSaveData);

	_background1.init(_loadSaveData->_image1Name);
	_background2.init(_loadSaveData->_image2Name);

	_baseFont = g_nancy->_graphics->getFont(_loadSaveData->_mainFontID);

	if (_loadSaveData->_highlightFontID != -1) {
		_highlightFont = g_nancy->_graphics->getFont(_loadSaveData->_highlightFontID);
	} else {
		_highlightFont = _baseFont;
	}

	if (_loadSaveData->_disabledFontID != -1) {
		_disabledFont = g_nancy->_graphics->getFont(_loadSaveData->_disabledFontID);
	} else {
		_disabledFont = _baseFont;
	}

	_sortedSavesList = g_nancy->getMetaEngine()->listSaves(ConfMan.getActiveDomainName().c_str());
	filterAndSortSaveStates();

	_textboxes.resize(_loadSaveData->_textboxBounds.size());
	for (uint i = 0; i < _textboxes.size(); ++i) {
		// Load textbox objects
		RenderObject *newTb = new RenderObject(5);
		_textboxes[i].reset(newTb);
		const Common::Rect &bounds = _loadSaveData->_textboxBounds[i];
		newTb->_drawSurface.create(bounds.width(), bounds.height(), g_nancy->_graphics->getScreenPixelFormat());
		newTb->_drawSurface.clear(g_nancy->_graphics->getTransColor());
		newTb->moveTo(bounds);
		newTb->setTransparent(true);
		newTb->setVisible(true);
		newTb->init();
	}

	if (!_loadSaveData->_inputTextboxBounds.isEmpty()) {
		_inputTextbox.reset(new RenderObject(5));
		const Common::Rect &bounds = _loadSaveData->_inputTextboxBounds;
		_inputTextbox->_drawSurface.create(bounds.width(), bounds.height(), g_nancy->_graphics->getScreenPixelFormat());
		_inputTextbox->_drawSurface.clear(g_nancy->_graphics->getTransColor());
		_inputTextbox->moveTo(bounds);
		_inputTextbox->setTransparent(true);
		_inputTextbox->setVisible(true);
		_inputTextbox->init();
	}

	_filenameStrings.resize(_loadSaveData->_textboxBounds.size());
	_saveExists.resize(_filenameStrings.size(), false);

	// Five buttons total
	g_nancy->_resource->loadImage(_loadSaveData->_imageButtonsName, _buttonsImage);

	_saveButton.reset(new UI::Button(1, _buttonsImage,
			_loadSaveData->_pressedButtonSrcs[0],
			_loadSaveData->_buttonDests[0],
			_loadSaveData->_highlightedButtonSrcs[0],
			_loadSaveData->_disabledButtonSrcs[0],
			_loadSaveData->_unpressedButtonSrcs[0]));
	_pageUpButton.reset(new UI::Button(1, _buttonsImage,
			_loadSaveData->_pressedButtonSrcs[1],
			_loadSaveData->_buttonDests[1],
			_loadSaveData->_highlightedButtonSrcs[1],
			_loadSaveData->_disabledButtonSrcs[1],
			_loadSaveData->_unpressedButtonSrcs[1]));
	_pageDownButton.reset(new UI::Button(1, _buttonsImage,
			_loadSaveData->_pressedButtonSrcs[2],
			_loadSaveData->_buttonDests[2],
			_loadSaveData->_highlightedButtonSrcs[2],
			_loadSaveData->_disabledButtonSrcs[2],
			_loadSaveData->_unpressedButtonSrcs[2]));
	_loadButton.reset(new UI::Button(1, _buttonsImage,
			_loadSaveData->_pressedButtonSrcs[3],
			_loadSaveData->_buttonDests[3],
			_loadSaveData->_highlightedButtonSrcs[3],
			_loadSaveData->_disabledButtonSrcs[3],
			_loadSaveData->_unpressedButtonSrcs[3]));
	_exitButton.reset(new UI::Button(1, _buttonsImage,
			_loadSaveData->_pressedButtonSrcs[4],
			_loadSaveData->_buttonDests[4],
			_loadSaveData->_highlightedButtonSrcs[4],
			_loadSaveData->_disabledButtonSrcs[4],
			_loadSaveData->_unpressedButtonSrcs[4]));

	// Load the blinking cursor graphic that appears while typing a filename
	if (!_loadSaveData->_blinkingCursorSrc.isEmpty()) {
		_blinkingCursorOverlay._drawSurface.create(_loadSaveData->_blinkingCursorSrc.width(),
			_loadSaveData->_blinkingCursorSrc.height(),
			g_nancy->_graphics->getScreenPixelFormat());
		_blinkingCursorOverlay.setTransparent(true);
		_blinkingCursorOverlay.setVisible(false);
		_blinkingCursorOverlay._drawSurface.clear(_blinkingCursorOverlay._drawSurface.getTransparentColor());
		_blinkingCursorOverlay._drawSurface.transBlitFrom(_highlightFont->getImageSurface(), _loadSaveData->_blinkingCursorSrc,
			Common::Point(), g_nancy->_graphics->getTransColor());
	} else {
		Common::Rect bounds = _highlightFont->getBoundingBox('-');
		_blinkingCursorOverlay._drawSurface.create(bounds.width() + 2, bounds.height(),
			g_nancy->_graphics->getScreenPixelFormat());
		_blinkingCursorOverlay.setTransparent(true);
		_blinkingCursorOverlay.setVisible(false);
		_blinkingCursorOverlay._drawSurface.clear(_blinkingCursorOverlay._drawSurface.getTransparentColor());
		_highlightFont->drawChar(_blinkingCursorOverlay._drawSurface.surfacePtr(), '-', 2, 0, 0);
	}

	// Load the "Your game has been saved" popup graphic
	if (!_loadSaveData->_gameSavedPopup.empty()) {
		g_nancy->_resource->loadImage(_loadSaveData->_gameSavedPopup, _successOverlay._drawSurface);
		Common::Rect destBounds = Common::Rect(0,0, _successOverlay._drawSurface.w, _successOverlay._drawSurface.h);
		destBounds.moveTo(640 / 2 - destBounds.width() / 2,
			480 / 2 - destBounds.height() / 2);
		_successOverlay.moveTo(destBounds);
		_successOverlay.setVisible(false);
	}

	registerGraphics();

	_state = kRun;
	_enteringNewState = true;
}

void LoadSaveMenu_V2::run() {
	if (_enteringNewState) {
		// State has changed, revert all relevant objects to an appropriate state
		goToPage(_currentPage);
		for (uint i = 0; i < _textboxes.size(); ++i) {
			writeToTextbox(i, _filenameStrings[i], Nancy::State::Scene::hasInstance() ? _baseFont : _disabledFont);
		}

		if (_currentPage == 0) {
			_saveButton->setDisabled(!Nancy::State::Scene::hasInstance());
			_saveButton->_isClicked = false;
			_inputTextbox->setVisible(true);
			_textboxes[0]->setVisible(false);
		} else {
			_saveButton->setDisabled(true);
			_saveButton->setVisible(false);
			_inputTextbox->setVisible(false);
			_textboxes[0]->setVisible(true);
		}

		_loadButton->setDisabled(_selectedSave == -1);
		_loadButton->_isClicked = false;

		_blinkingCursorOverlay.setVisible(false);
		_exitButton->setDisabled(false);
		_successOverlay.setVisible(false);

		_hoveredSave = -1;
		_enteringNewState = false;
	}

	// Handle input
	NancyInput input = g_nancy->_input->getInput();

	_loadButton->handleInput(input);

	if (_loadButton->_isClicked) {
		uint index = _selectedSave;
		if (_saveExists[index]) {
			_state = kLoad;
			_enteringNewState = true;
			_selectedSave = index;
			g_nancy->_sound->playSound("BULS");
		}

		return;
	}

	_saveButton->handleInput(input);

	if (_saveButton->_isClicked) {
		if (Nancy::State::Scene::hasInstance()) {
			_state = kSave;
			_enteringNewState = true;
			g_nancy->_sound->playSound("BULS");
		}

		return;
	}

	if (_pageUpButton) {
		_pageUpButton->handleInput(input);
		if (_pageUpButton->_isClicked) {
			--_currentPage;
			g_nancy->_sound->playSound("BUOK");
			_enteringNewState = true;
			_pageUpButton->_isClicked = false;
			return;
		}
	}

	if (_pageDownButton) {
		_pageDownButton->handleInput(input);
		if (_pageDownButton->_isClicked) {
			++_currentPage;
			g_nancy->_sound->playSound("BUOK");
			_enteringNewState = true;
			_pageDownButton->_isClicked = false;
			return;
		}
	}

	// Handle textbox hovering
	bool hoversOverTextbox = false;
	for (int i = 0; i < (int)_textboxes.size(); ++i) {
		uint i2 = (_currentPage == 0 ? i + 1 : i);
		if (i2 >= _textboxes.size()) {
			break;
		}

		if (_textboxes[i2]->getScreenPosition().contains(input.mousePos)) {
			hoversOverTextbox = true;
			if (_hoveredSave != i) {
				if (_hoveredSave != -1 && _hoveredSave != _selectedSave) {
					if (_hoveredSave == kInputTextboxIndex) {
						writeToInputTextbox(_baseFont);
					} else {
						writeToTextbox(_hoveredSave, _filenameStrings[_hoveredSave], _baseFont);
					}
				}

				_hoveredSave = i;
				writeToTextbox(_hoveredSave, _filenameStrings[_hoveredSave], _highlightFont);
			}

			if (input.input & NancyInput::kLeftMouseButtonUp && (_filenameStrings[_hoveredSave].size())) {
				if (_selectedSave != -1) {
					writeToTextbox(_selectedSave, _filenameStrings[_selectedSave], _baseFont);
				}

				_loadButton->setDisabled(false);

				_hoveredSave = -1;
				_selectedSave = i;

				return;
			}

			break;
		}
	}

	if (Nancy::State::Scene::hasInstance() && _currentPage == 0 &&
			_inputTextbox && _inputTextbox->getScreenPosition().contains(input.mousePos)) {
		hoversOverTextbox = true;

		if (_hoveredSave != kInputTextboxIndex && _hoveredSave != -1) {
			writeToTextbox(_hoveredSave, _filenameStrings[_hoveredSave], _baseFont);
		}

		if (_selectedSave != -1) {
			writeToTextbox(_selectedSave, _filenameStrings[_selectedSave], _baseFont);
		}

		_hoveredSave = kInputTextboxIndex;
		writeToInputTextbox(_highlightFont);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_state = kEnterFilename;
			_enteringNewState = true;

			return;
		}
	}

	if (!hoversOverTextbox && _hoveredSave != -1 && _hoveredSave != _selectedSave) {
		if (_hoveredSave == kInputTextboxIndex) {
			writeToInputTextbox(_baseFont);
		} else {
			writeToTextbox(_hoveredSave, _filenameStrings[_hoveredSave], _baseFont);
		}
		_hoveredSave = -1;
	}

	// Check Done button
	if (_exitButton) {
		_exitButton->handleInput(input);

		if (_exitButton->_isClicked) {
			_state = kStop;
			g_nancy->_sound->playSound("BUOK");
			return;
		}
	}
}

void LoadSaveMenu_V2::enterFilename() {
	if (_enteringNewState) {
		// State has changed, revert all relevant objects to an appropriate state
		_hoveredSave = -1;
		_loadButton->setDisabled(true);

		Common::Rect tbPosition = _inputTextbox->getScreenPosition();
		Common::Rect cursorRect = _blinkingCursorOverlay._drawSurface.getBounds();
		cursorRect.moveTo(tbPosition.left + _highlightFont->getStringWidth(_enteredString),
			tbPosition.bottom - _blinkingCursorOverlay._drawSurface.h + _loadSaveData->_fontYOffset);
		cursorRect.translate(0, cursorRect.height() / 2 - 1);
		_blinkingCursorOverlay.moveTo(cursorRect);
		_blinkingCursorOverlay.setVisible(true);
		_nextBlink = g_nancy->getTotalPlayTime() + _loadSaveData->_blinkingTimeDelay;
		_enteringNewState = false;
		g_nancy->_input->setVKEnabled(true);
	}

	LoadSaveMenu::enterFilename();

	// Handle input
	NancyInput input = g_nancy->_input->getInput();

	bool enterKeyPressed = false;
	if (input.otherKbdInput.size()) {
		uint16 textWidthInPixels = writeToInputTextbox(_highlightFont);
		Common::Rect tbPosition = _inputTextbox->getScreenPosition();
		Common::Rect lastCursorPosition = _blinkingCursorOverlay.getScreenPosition();
		_blinkingCursorOverlay.moveTo(Common::Point(tbPosition.left + textWidthInPixels,
			lastCursorPosition.top));

		if (	input.otherKbdInput.back().keycode == Common::KEYCODE_RETURN ||
				input.otherKbdInput.back().keycode == Common::KEYCODE_KP_ENTER) {
			enterKeyPressed = true;
		}
	}

	if (_state != kEnterFilename) {
		return;
	}

	if (_pageUpButton) {
		_pageUpButton->handleInput(input);
		if (_pageUpButton->_isClicked) {
			--_currentPage;
			_state = kRun;
			_enteringNewState = true;
			_pageUpButton->_isClicked = false;
			return;
		}
	}

	if (_pageDownButton) {
		_pageDownButton->handleInput(input);
		if (_pageDownButton->_isClicked) {
			// Redraw input textbox so it's not highlighted when user goes back up
			writeToInputTextbox(_baseFont);

			++_currentPage;
			_state = kRun;
			_enteringNewState = true;
			_pageDownButton->_isClicked = false;
			return;
		}
	}

	_saveButton->handleInput(input);
	if (_saveButton->_isClicked || enterKeyPressed) {
		_state = kSave;
		_enteringNewState = true;
		g_nancy->_sound->playSound("BULS");
		g_nancy->_input->setVKEnabled(false);
		return;
	}

	// Handle hovering over other saves
	bool hoversOverTextbox = false;
	for (int i = 0; i < (int)_textboxes.size(); ++i) {
		uint i2 = i + 1;
		if (i2 >= _textboxes.size()) {
			break;
		}

		if (_textboxes[i2]->getScreenPosition().contains(input.mousePos)) {
			hoversOverTextbox = true;
			if (_hoveredSave != i) {
				if (_hoveredSave != -1) {
					writeToTextbox(_hoveredSave, _filenameStrings[_hoveredSave], _baseFont);
				}

				_hoveredSave = i;
				writeToTextbox(_hoveredSave, _filenameStrings[_hoveredSave], _highlightFont);
			}

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				writeToInputTextbox(_baseFont);
				_state = kRun;
				_enteringNewState = true;

				_hoveredSave = -1;
				_selectedSave = i;

				return;
			}

			break;
		}
	}

	if (!hoversOverTextbox && _hoveredSave != -1 && _hoveredSave != _selectedSave) {
		writeToTextbox(_hoveredSave, _filenameStrings[_hoveredSave], _baseFont);
		_hoveredSave = -1;
	}
}

void LoadSaveMenu_V2::save() {
	Common::String finalDesc = _enteredString;

	// Look for a state with a matching name and overwrite it
	bool foundMatch = false;
	for (auto &save : _sortedSavesList) {
		if (save.getDescription() == finalDesc) {
			foundMatch = true;
			_selectedSave = save.getSaveSlot();
			break;
		}
	}

	if (!foundMatch) {
		// No match, place in the lowest free slot
		_selectedSave = 1;
		bool shouldContinue = false;
		do {
			shouldContinue = false;
			for (auto &save : _sortedSavesList) {
				if (save.getSaveSlot() == _selectedSave) {
					++_selectedSave;
					shouldContinue = true;
					break;
				}
			}
		} while (shouldContinue);
	}

	g_nancy->saveGameState(_selectedSave, finalDesc, false);

	ConfMan.setInt("display_slot", _selectedSave, Common::ConfigManager::kTransientDomain); // Used to load the save name

	if (_successOverlay._drawSurface.empty()) {
		_state = kRun;
		_enteringNewState = true;
	} else {
		_state = kSuccess;
		_enteringNewState = true;
	}

	if ((int)_saveExists.size() < _selectedSave) {
		_saveExists.resize(_selectedSave + 1, false);
	}

	_saveExists[_selectedSave] = true;
	g_nancy->_hasJustSaved = true;
}

void LoadSaveMenu_V2::success() {
	LoadSaveMenu::success();

	if (g_nancy->getTotalPlayTime() > _nextBlink) {
		_selectedSave = 0;
	}
}

int LoadSaveMenu_V2::scummVMSaveSlotToLoad() const {
	uint orderedSaveID = _currentPage * _filenameStrings.size() + _selectedSave;
	if (_currentPage != 0) {
		// First page has one save less
		orderedSaveID -= 1;
	}

	return _sortedSavesList[orderedSaveID].getSaveSlot();
}

uint16 LoadSaveMenu_V2::writeToTextbox(int textboxID, const Common::String &text, const Font *font) {
	assert(font);

	if (_currentPage == 0) {
		// Page one has one textbox less, right at the top.
		// We simply disable it and adjust the indexing here and in the hovering code
		++textboxID;
	}

	if (textboxID >= (int)_textboxes.size()) {
		return 0;
	}

	return LoadSaveMenu::writeToTextbox(textboxID, text, font);
}

uint16 LoadSaveMenu_V2::writeToInputTextbox(const Font *font) {
	assert(font);

	_inputTextbox->_drawSurface.clear(g_nancy->_graphics->getTransColor());
	Common::Point destPoint(_loadSaveData->_fontXOffset, _loadSaveData->_fontYOffset + _inputTextbox->_drawSurface.h - font->getFontHeight());
	font->drawString(&_inputTextbox->_drawSurface, _enteredString,
			destPoint.x, destPoint.y, _inputTextbox->_drawSurface.w, 0);
	_inputTextbox->setVisible(true);

	return font->getStringWidth(_enteredString);
}

struct SaveStateDescriptorSaveTimeComparator {
	bool operator()(const SaveStateDescriptor &x, const SaveStateDescriptor &y) const {
		// Compare the date/time strings. This is valid since they only
		// contain digits and the - and : characters. The comparison
		// makes sure that saves are listed from newest to oldest.
		int dateCompare = x.getSaveDate().compareToIgnoreCase(y.getSaveDate());
		if (dateCompare) {
			return dateCompare > 0;
		} else {
			return x.getSaveTime().compareToIgnoreCase(y.getSaveTime()) > 0;
		}
	}
};

void LoadSaveMenu_V2::filterAndSortSaveStates() {
	if (!_sortedSavesList.size()) {
		return;
	}

	// Assumes the autosave slot is 0
	if (_sortedSavesList[0].isAutosave() && _sortedSavesList[0].isValid()) {
		_sortedSavesList.erase(_sortedSavesList.begin());
	}

	// Clear second chance saves
	for (auto *save = _sortedSavesList.begin(); save != _sortedSavesList.end(); ++save) {
		if (save->getDescription() == "SECOND CHANCE") {
			save = _sortedSavesList.erase(save) - 1;
		}
	}

	Common::sort(_sortedSavesList.begin(), _sortedSavesList.end(), SaveStateDescriptorSaveTimeComparator());
}

void LoadSaveMenu_V2::extractSaveNames(uint pageID) {
	if (!_sortedSavesList.size()) {
		// No saves yet, just load the default name into the input box
		_enteredString = _loadSaveData->_emptySaveText;
		writeToInputTextbox(_baseFont);

		return;
	}

	// First, empty all save names
	for (uint i = 0; i < _filenameStrings.size(); ++i) {
		_filenameStrings[i].clear();
		_saveExists[i] = false;
	}

	uint firstSaveID, lastSaveID;

	// The first page has one textbox less
	uint numTextboxes = (_currentPage == 0 ? _filenameStrings.size() - 1 : _filenameStrings.size());
	firstSaveID = pageID == 0 ? 0 : (_filenameStrings.size() - 1 + (pageID - 1) * _filenameStrings.size());
	lastSaveID = MIN<uint>(_sortedSavesList.size() - 1, firstSaveID + numTextboxes - 1);

	for (uint i = firstSaveID; i <= lastSaveID; ++i) {
		int onScreenSaveID = i - firstSaveID;
		_saveExists[onScreenSaveID] = true;
		_filenameStrings[onScreenSaveID] = _sortedSavesList[i].getDescription();
	}

	// Load the top textbox showing the name of the last save made
	if (_enteredString.empty() && Nancy::State::Scene::hasInstance()) {
		bool textboxSet = false;

		if (ConfMan.hasKey("display_slot", Common::ConfigManager::kTransientDomain)) {
			int slot = ConfMan.getInt("display_slot", Common::ConfigManager::kTransientDomain);

			for (uint i = 0; i < _sortedSavesList.size(); ++i) {
				if (_sortedSavesList[i].getSaveSlot() == slot) {
					_enteredString = _sortedSavesList[i].getDescription();
					writeToInputTextbox(_baseFont);
					textboxSet = true;
					break;
				}
			}
		}

		if (!textboxSet) {
			_enteredString = _loadSaveData->_emptySaveText;
			writeToInputTextbox(_baseFont);
		}
	}
}

void LoadSaveMenu_V2::goToPage(uint pageID) {
	if (pageID == 0) {
		_background1.setVisible(true);
		_background2.setVisible(false);
	} else {
		_background1.setVisible(false);
		_background2.setVisible(true);
	}

	extractSaveNames(pageID);
	_currentPage = pageID;
	_selectedSave = -1;

	int numSaves = _sortedSavesList.size();

	if (!_pageUpButton || !_pageDownButton)
		return;

	if (numSaves > (int)((_filenameStrings.size() - 1) + pageID * _filenameStrings.size())) {
		_pageDownButton->setDisabled(false);
		_pageDownButton->setVisible(true);
	} else {
		_pageDownButton->setDisabled(true);
		_pageDownButton->setVisible(false);
	}

	if (pageID == 0) {
		_pageUpButton->setDisabled(true);
		_pageUpButton->setVisible(false);
	} else {
		_pageUpButton->setDisabled(false);
		_pageUpButton->setVisible(true);
	}

	_loadButton->setDisabled(true);
}

void LoadSaveMenu_V2::reloadSaves() {
	_sortedSavesList = g_nancy->getMetaEngine()->listSaves(ConfMan.getActiveDomainName().c_str());
	filterAndSortSaveStates();
	extractSaveNames(0);
}

} // End of namespace State
} // End of namespace Nancy
