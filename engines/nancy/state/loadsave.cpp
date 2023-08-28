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

namespace Common {
DECLARE_SINGLETON(Nancy::State::LoadSaveMenu);
}

namespace Nancy {
namespace State {

LoadSaveMenu::~LoadSaveMenu() {
	for (auto *tb : _textboxes) {
		delete tb;
	}

	for (auto *button : _loadButtons) {
		delete button;
	}

	for (auto *button : _saveButtons) {
		delete button;
	}

	for (auto *overlay : _cancelButtonOverlays) {
		delete overlay;
	}

	delete _exitButton;
	delete _cancelButton;
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
	case kStop:
		stop();
		break;
	}

	g_nancy->_cursorManager->setCursorType(CursorManager::kNormalArrow);
}

void LoadSaveMenu::onStateEnter(const NancyState::NancyState prevState) {
	registerGraphics();
}

bool LoadSaveMenu::onStateExit(const NancyState::NancyState nextState) {
	return true;
}

void LoadSaveMenu::registerGraphics() {
	_background.registerGraphics();

	for (auto *button : _loadButtons) {
		button->registerGraphics();
	}

	for (auto *button : _saveButtons) {
		button->registerGraphics();
	}

	for (auto *overlay : _cancelButtonOverlays) {
		overlay->registerGraphics();
	}

	for (auto *tb : _textboxes) {
		tb->registerGraphics();
	}

	if (_exitButton) {
		_exitButton->registerGraphics();
	}

	if (_cancelButton) {
		_cancelButton->registerGraphics();
	}

	_blinkingCursorOverlay.registerGraphics();
	_successOverlay.registerGraphics();
}

void LoadSaveMenu::init() {
	_loadSaveData = g_nancy->_loadSaveData;
	assert(_loadSaveData);

	_background.init(_loadSaveData->_imageName);

	_baseFont = g_nancy->_graphicsManager->getFont(_loadSaveData->_mainFontID);

	if (_loadSaveData->_highlightFontID != -1) {
		_highlightFont = g_nancy->_graphicsManager->getFont(_loadSaveData->_highlightFontID);
		_disabledFont = g_nancy->_graphicsManager->getFont(_loadSaveData->_disabledFontID);
	} else {
		_highlightFont = _disabledFont = _baseFont;
	}

	_filenameStrings.resize(_loadSaveData->_textboxBounds.size());
	_saveExists.resize(_filenameStrings.size(), false);
	_textboxes.resize(_filenameStrings.size());
	for (uint i = 0; i < _textboxes.size(); ++i) {
		// Load textbox objects
		RenderObject *newTb = new RenderObject(5);
		_textboxes[i] = newTb;
		Common::Rect &bounds = _loadSaveData->_textboxBounds[i];
		newTb->_drawSurface.create(bounds.width(), bounds.height(), g_nancy->_graphicsManager->getScreenPixelFormat());
		newTb->_drawSurface.clear(g_nancy->_graphicsManager->getTransColor());
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
			_filenameStrings[i] = g_nancy->getStaticData().emptySaveText;
		}
	}

	bool hasHighlights = _loadSaveData->_loadButtonHighlightSrcs.size();

	_loadButtons.resize(_textboxes.size());
	_saveButtons.resize(_textboxes.size());
	_cancelButtonOverlays.resize(_textboxes.size());
	for (uint i = 0; i < _loadButtons.size(); ++i) {
		// Load Save and Load buttons, and Cancel overlays
		_loadButtons[i] = new UI::Button(1, _background._drawSurface,
			_loadSaveData->_loadButtonDownSrcs[i], _loadSaveData->_loadButtonDests[i],
			hasHighlights ? _loadSaveData->_loadButtonHighlightSrcs[i] : Common::Rect(),
			hasHighlights ? _loadSaveData->_loadButtonDisabledSrcs[i] : Common::Rect());

		_saveButtons[i] = new UI::Button(1, _background._drawSurface,
			_loadSaveData->_saveButtonDownSrcs[i], _loadSaveData->_saveButtonDests[i],
			hasHighlights ? _loadSaveData->_saveButtonHighlightSrcs[i] : Common::Rect(),
			hasHighlights ? _loadSaveData->_saveButtonDisabledSrcs[i] : Common::Rect());

		_cancelButtonOverlays[i] = new RenderObject(2, _background._drawSurface,
			_loadSaveData->_cancelButtonSrcs[i], _loadSaveData->_cancelButtonDests[i]);

		_loadButtons[i]->init();
		_saveButtons[i]->init();
		_cancelButtonOverlays[i]->init();
	}

	// Load exit button
	_exitButton = new UI::Button(3, _background._drawSurface,
			_loadSaveData->_doneButtonDownSrc, _loadSaveData->_doneButtonDest,
			hasHighlights ? _loadSaveData->_doneButtonHighlightSrc : Common::Rect(),
			hasHighlights ? _loadSaveData->_doneButtonDisabledSrc : Common::Rect());

	// Load Cancel button that activates when typing a filename
	// Note: this is only responsible for the hover/mouse down/disabled graphic;
	// the graphics that replace the Save buttons with Cancel are their own RenderObject.
	// We also make sure this has an invalid position until we need it.
	Common::Rect pos = _loadSaveData->_cancelButtonDests[0];
	pos.moveTo(-500, 0);
	_cancelButton = new UI::Button(3, _background._drawSurface,
		_loadSaveData->_cancelButtonDownSrc, Common::Rect(),
		_loadSaveData->_cancelButtonHighlightSrc, _loadSaveData->_cancelButtonDisabledSrc);
	
	// Load the blinking cursor graphic that appears while typing a filename
	_blinkingCursorOverlay._drawSurface.create(_loadSaveData->_blinkingCursorSrc.width(),
		_loadSaveData->_blinkingCursorSrc.height(),
		g_nancy->_graphicsManager->getScreenPixelFormat());
	_blinkingCursorOverlay.setTransparent(true);
	_blinkingCursorOverlay._drawSurface.blitFrom(_highlightFont->getImageSurface(), _loadSaveData->_blinkingCursorSrc, Common::Point());
	_blinkingCursorOverlay.setVisible(false);

	// Load the "Your game has been saved" popup graphic
	if (_loadSaveData->_gameSavedPopup.size()) {
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

void LoadSaveMenu::run() {
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

void LoadSaveMenu::enterFilename() {
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
	}

	// Perform cursor blinking
	uint32 gameTime = g_nancy->getTotalPlayTime();
	if (_loadSaveData->_blinkingTimeDelay != 0 && gameTime > _nextBlink) {
		_blinkingCursorOverlay.setVisible(!_blinkingCursorOverlay.isVisible());
		_nextBlink = gameTime + _loadSaveData->_blinkingTimeDelay;
	}

	// Handle input
	NancyInput input = g_nancy->_input->getInput();

	// Improvement: we allow the enter key to sumbit
	bool enterKeyPressed = false;
	for (uint i = 0; i < input.otherKbdInput.size(); ++i) {
		Common::KeyState &key = input.otherKbdInput[i];
		if (key.keycode == Common::KEYCODE_BACKSPACE) {
			if (_enteredString.size()) {
				_enteredString.deleteLastChar();
			}
		} else if (key.keycode == Common::KEYCODE_RETURN) {
			enterKeyPressed = true;
		} else if (Common::isAlnum(key.ascii) || Common::isSpace(key.ascii)) {
			_enteredString += key.ascii;
		}

		uint16 textWidthInPixels = writeToTextbox(_selectedSave, _enteredString, _highlightFont);
		Common::Rect tbPosition = _textboxes[_selectedSave]->getScreenPosition();
		_blinkingCursorOverlay.moveTo(Common::Point(tbPosition.left + textWidthInPixels,
			tbPosition.bottom - _blinkingCursorOverlay._drawSurface.h + _loadSaveData->_fontYOffset));
	}
	
	_cancelButton->handleInput(input);
	if (_cancelButton->_isClicked) {
		_state = kRun;
		_enteringNewState = true;
		g_nancy->_sound->playSound("BULS");
		return;
	}

	_saveButtons[_selectedSave]->handleInput(input);
	if (_saveButtons[_selectedSave]->_isClicked || enterKeyPressed) {
		_state = kSave;
		_enteringNewState = true;
		g_nancy->_sound->playSound("BULS");
		return;
	}
}

void LoadSaveMenu::save() {
	// Improvement: not providing a name doesn't result in the
	// savefile being named "--- Empty ---" or "Nothing Saved Here".
	// Instead, we use ScummVM's built-in save name generator
	g_nancy->saveGameState(_selectedSave + 1, _enteredString.size() ? _enteredString :
		_filenameStrings[_selectedSave].equals(g_nancy->getStaticData().emptySaveText) ? Common::String() : _filenameStrings[_selectedSave], false);

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
}

void LoadSaveMenu::load() {
	if (Nancy::State::Scene::hasInstance()) {
		Nancy::State::Scene::destroy();
	}

	ConfMan.setInt("save_slot", _selectedSave + 1, Common::ConfigManager::kTransientDomain);

	_state = kStop;
	_enteringNewState = true;
}

void LoadSaveMenu::success() {
	// The original engine still lets the cursor blink in the background, but implementing that is completely unnecessary
	if (_enteringNewState) {
		_nextBlink = g_nancy->getTotalPlayTime() + 2000; // Hardcoded
		_successOverlay.setVisible(true);
		_enteringNewState = false;
	}

	if (g_nancy->getTotalPlayTime() > _nextBlink) {
		_state = kRun;
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

uint16 LoadSaveMenu::writeToTextbox(uint textboxID, const Common::String &text, const Font *font) {
	assert(font);

	_textboxes[textboxID]->_drawSurface.clear(g_nancy->_graphicsManager->getTransColor());
	Common::Point destPoint(_loadSaveData->_fontXOffset, _loadSaveData->_fontYOffset + _textboxes[textboxID]->_drawSurface.h - font->getFontHeight());
	font->drawString(&_textboxes[textboxID]->_drawSurface, text, destPoint.x, destPoint.y, _textboxes[textboxID]->_drawSurface.w, 0);
	_textboxes[textboxID]->setVisible(true);

	return font->getStringWidth(text);
}

} // End of namespace State
} // End of namespace Nancy
