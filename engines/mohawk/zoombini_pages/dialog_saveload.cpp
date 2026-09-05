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

#include "mohawk/console.h"
#include "mohawk/mohawk.h"
#include "mohawk/sound.h"
#include "mohawk/video.h"

#include "mohawk/zoombini.h"
#include "mohawk/zoombini_graphics.h"
#include "mohawk/zoombini_pages/dialog_saveload.h"
#include "mohawk/zoombini_random.h"
#include "mohawk/zoombini_sound.h"
#include "mohawk/zoombini_state.h"

namespace Mohawk {

ZoombiniDialogSaveLoad::ZoombiniDialogSaveLoad(MohawkEngine_Zoombini *vm, SaveLoadMode mode) : ZoombiniDialog(vm, ZoombiniPageType::kDialogSaveLoad),
																							   _mode(mode) {
	ZoombiniText::Key yesKey = ZoombiniText::kNone;
	ZoombiniText::Key noKey = ZoombiniText::kDialogButtonCancel;

	switch (_mode) {
	case kSaveMode:
		_titleKey = ZoombiniText::kDialogTitleSave;
		_titleRect = Common::Rect(0x00FA, 0x0039, 0x0198, 0x0055);
		yesKey = ZoombiniText::kDialogButtonSave;

		_saveInputText = _vm->_state->getActiveSaveName();
		_saveInputCursorPos = _saveInputText.size();
		break;
	case kLoadOrNewMode:
		// The red Load dialog button uses the localized dialog label.
		// @ref ZoombiniText::kNewGame is the executable-sourced default name for an unsaved game instead.
		noKey = ZoombiniText::kDialogButtonNewGame;
		// Fall through.
	case kLoadMode:
		_titleKey = ZoombiniText::kDialogTitleLoad;
		_titleRect = Common::Rect(0x00FA, 0x0043, 0x0198, 0x005F);
		yesKey = ZoombiniText::kDialogButtonLoad;
		break;
	default:
		error("saveload: invalid ZoombiniMsgBoxType: %u", static_cast<uint32>(_mode));
		break;
	}

	ZmbResource soundResId = ZmbResource(ZmbResource::kSystem, kSysResSound0999_ButtonSFX);
	_scrollButtonStateMap[kSaveLoadDialogButton01_ScrollUp] = ButtonState(soundResId, 0, 2, kSystemShape0001_16_SaveLoadScrollUpButtonNormal, kSystemShape0001_17_SaveLoadScrollUpButtonPressed);
	_scrollButtonStateMap[kSaveLoadDialogButton02_ScrollDown] = ButtonState(soundResId, 1, 3, kSystemShape0001_18_SaveLoadScrollDownButtonNormal, kSystemShape0001_19_SaveLoadScrollDownButtonPressed);
	_longButtonStateMap[kSaveLoadDialogButton03_Okay] = ButtonState(yesKey, soundResId, 0, 2, kSystemShape0001_12_LongGreenButtonNormal, kSystemShape0001_13_LongGreenButtonPressed);
	_longButtonStateMap[kSaveLoadDialogButton04_Cancel] = ButtonState(noKey, soundResId, 1, 3, kSystemShape0001_14_LongRedButtonNormal, kSystemShape0001_15_LongRedButtonPressed);
}

ZoombiniDialogSaveLoad::~ZoombiniDialogSaveLoad() {
}

void ZoombiniDialogSaveLoad::loadFeatures() {
	int16 frameScrb;
	int16 scrollButtonsScrb;
	int16 longButtonsScrb;
	if (_mode == kSaveMode) {
		frameScrb = kSysResScrb0007_DialogSave;
		scrollButtonsScrb = kSysResScrb0008_DialogSave;
		longButtonsScrb = kSysResScrb0009_DialogSave;
	} else {
		frameScrb = kSysResScrb0004_DialogLoad;
		scrollButtonsScrb = kSysResScrb0005_DialogLoad;
		longButtonsScrb = kSysResScrb0006_DialogLoad;
	}

	// Load SCRBs
	ZmbFeature::EventHooks hooksDialogFrame;
	hooksDialogFrame.setPostRenderFunc(static_cast<ZmbFeature::OnPostRenderFunc>(&ZoombiniDialogSaveLoad::dialogFrame_onPostRender));
	hooksDialogFrame.setLButtonDownFunc(static_cast<ZmbFeature::OnLButtonDownFunc>(&ZoombiniDialogSaveLoad::dialogFrame_onLButtonDown));
	hooksDialogFrame.setKeyDownFunc(static_cast<ZmbFeature::OnKeyDownFunc>(&ZoombiniDialogSaveLoad::dialogFrame_onKeyDown));
	_dialogFrameFeature = loadScrbFeature(ZmbResource(ZmbResource::kSystem, kSysResShapeBitmap0001_Dialog), frameScrb, 0,
										  ZmbFeature::FLAG_04000000_OVERLAY | ZmbFeature::FLAG_00001000_TOPMOST,
										  hooksDialogFrame);

	for (Common::StableMap<uint32, ButtonState>::iterator it = _scrollButtonStateMap.begin(); it != _scrollButtonStateMap.end(); it++)
		it->second.reset();
	ZmbFeature::EventHooks hooksScrollButtons;
	hooksScrollButtons.setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniDialogSaveLoad::scrollButtons_onPreRenderShape));
	hooksScrollButtons.setPostRenderFunc(static_cast<ZmbFeature::OnPostRenderFunc>(&ZoombiniDialogSaveLoad::scrollButtons_onPostRender));
	hooksScrollButtons.setWheelUpFunc(static_cast<ZmbFeature::OnWheelUpFunc>(&ZoombiniDialogSaveLoad::scrollButtons_onWheelUp));
	hooksScrollButtons.setWheelDownFunc(static_cast<ZmbFeature::OnWheelDownFunc>(&ZoombiniDialogSaveLoad::scrollButtons_onWheelDown));
	hooksScrollButtons.setLButtonDownFunc(static_cast<ZmbFeature::OnLButtonDownFunc>(&ZoombiniDialogSaveLoad::scrollButtons_onLButtonDown));
	hooksScrollButtons.setKeyDownFunc(static_cast<ZmbFeature::OnKeyDownFunc>(&ZoombiniDialogSaveLoad::scrollButtons_onKeyDown));
	loadScrbFeature(ZmbResource(ZmbResource::kSystem, kSysResShapeBitmap0001_Dialog), scrollButtonsScrb, 11, ZmbFeature::FLAG_04000000_OVERLAY, hooksScrollButtons);

	resetLongButtonStates();
	ZmbFeature::EventHooks hooksLongButtons;
	hooksLongButtons.setPreRenderShapeFunc(static_cast<ZmbFeature::OnPreRenderShapeFunc>(&ZoombiniDialogSaveLoad::longButtons_onPreRenderShape));
	hooksLongButtons.setPostRenderFunc(static_cast<ZmbFeature::OnPostRenderFunc>(&ZoombiniDialogSaveLoad::longButtons_onPostRender));
	hooksLongButtons.setLButtonDownFunc(static_cast<ZmbFeature::OnLButtonDownFunc>(&ZoombiniDialogSaveLoad::longButtons_onLButtonDown));
	hooksLongButtons.setKeyDownFunc(static_cast<ZmbFeature::OnKeyDownFunc>(&ZoombiniDialogSaveLoad::longButtons_onKeyDown));
	_longButtonsFeature = loadScrbFeature(ZmbResource(ZmbResource::kSystem, kSysResShapeBitmap0001_Dialog), longButtonsScrb, 13,
										  ZmbFeature::FLAG_04000000_OVERLAY,
										  hooksLongButtons);
}

void ZoombiniDialogSaveLoad::onEveryFrame() {
	if (_mode != kSaveMode)
		return;

	uint32 cursorBlinkDelta = _currentFrameTime - _saveInputCursorLastBlinkTimeMs;
	if (MohawkEngine_Zoombini::kTextCursorBlinkFrameTimeMs <= cursorBlinkDelta) {
		_saveInputCursorLastBlinkTimeMs = _currentFrameTime;
		_saveInputCursorVisible = !_saveInputCursorVisible;
		markDialogFrameDirty();
	}
}

ZmbEventHandleResult ZoombiniDialogSaveLoad::onImeComposition(const Common::ImeComposition &composition) {
	if (_mode != kSaveMode)
		return ZmbEventHandleResult::kPassthrough;

	Common::U32String compositionText;
	uint32 compositionCursorPos = 0;
	if (composition.state == Common::ImeComposition::kCompositing) {
		compositionText = composition.text;
		if (composition.start < 0)
			compositionCursorPos = compositionText.size();
		else
			compositionCursorPos = MIN<uint32>(static_cast<uint32>(composition.start), compositionText.size());
	}
	if (!compositionText.empty()) {
		Common::U32String displayedText = _saveInputText;
		displayedText.insertString(compositionText, _saveInputCursorPos);
		if (!ZmbRosterEntry::checkSaveNameSize(_vm, displayedText)) {
			compositionText.clear();
			compositionCursorPos = 0;
		}
	}

	if (_saveInputCompositionText == compositionText && _saveInputCompositionCursorPos == compositionCursorPos)
		return ZmbEventHandleResult::kConsumed;

	_saveInputCompositionText = compositionText;
	_saveInputCompositionCursorPos = compositionCursorPos;
	_saveInputCursorVisible = true;
	_saveInputCursorLastBlinkTimeMs = _currentFrameTime;
	markDialogFrameDirty();
	return ZmbEventHandleResult::kConsumed;
}

Common::Rect ZoombiniDialogSaveLoad::getSaveEntryBaseRect() {
	Common::Rect saveEntryRect = Common::Rect(_saveEntryLeft, 0, _saveEntryRight, 0);
	if (_mode == kSaveMode) {
		saveEntryRect.top = _saveEntrySaveModeTop;
		saveEntryRect.bottom = _saveEntrySaveModeTop + _saveEntryHeight;
	} else {
		saveEntryRect.top = _saveEntryLoadModeTop;
		saveEntryRect.bottom = _saveEntryLoadModeTop + _saveEntryHeight;
	}
	return saveEntryRect;
}

void ZoombiniDialogSaveLoad::clampLoadSelection() {
	int32 saveCount = _vm->_state->_r.getEntryCount();
	if (saveCount <= 0) {
		_saveEntrySelectedIdx = -1;
		_saveEntryBaseIdx = 0;
		return;
	}

	if (saveCount <= _saveEntrySelectedIdx)
		_saveEntrySelectedIdx = saveCount - 1;

	_saveEntryBaseIdx = CLIP<int32>(_saveEntryBaseIdx, 0, MAX<int32>(saveCount - SAVESLOTS_PER_SCREEN, 0));
	if (_saveEntrySelectedIdx < 0)
		return;

	if (_saveEntrySelectedIdx < _saveEntryBaseIdx) {
		_saveEntryBaseIdx = _saveEntrySelectedIdx;
	} else if (_saveEntryBaseIdx + SAVESLOTS_PER_SCREEN <= _saveEntrySelectedIdx) {
		_saveEntryBaseIdx = _saveEntrySelectedIdx - (SAVESLOTS_PER_SCREEN - 1);
	}
}

void ZoombiniDialogSaveLoad::removeLeadingSaveInputSpaces() {
	while (!_saveInputText.empty() && _saveInputText[0] == U' ') {
		_saveInputText.deleteChar(0);
		if (0 < _saveInputCursorPos)
			_saveInputCursorPos -= 1;
	}
}

void ZoombiniDialogSaveLoad::markDialogFrameDirty() {
	if (_dialogFrameFeature)
		_dialogFrameFeature->setNeedsRedraw(true);
}

Common::U32String ZoombiniDialogSaveLoad::getDisplayedSaveInputText() const {
	Common::U32String displayedText = _saveInputText;
	if (!_saveInputCompositionText.empty())
		displayedText.insertString(_saveInputCompositionText, _saveInputCursorPos);
	return displayedText;
}

void ZoombiniDialogSaveLoad::dialogFrame_onPostRender(ZmbFeature *feature) {
	(void)feature;

	ZoombiniGraphics::ScreenKind screenKind = ZoombiniGraphics::kShapeScreen;

	{ // [Text Render] Dialog Title
		ZoombiniGraphics::TextConf titleConf;
		titleConf._fontUsage = ZoombiniFontUsage::kFontTitle;
		titleConf._hAlign = Graphics::kTextAlignCenter;
		titleConf._vAlign = Graphics::kTextAlignCenter;
		titleConf._outlineEffect = true;
		titleConf._outlinePalette = ZoombiniGraphics::kColor0E_VeryLightGray;
		titleConf._textPalette = ZoombiniGraphics::kColor2D_Black;
		titleConf._wordWrap = false;
		_vm->_gfx->drawText(screenKind, _titleKey, _titleRect, titleConf);
	}

	Common::Rect saveEntryRect = getSaveEntryBaseRect();
	uint32 textPalette = 0;

	if (_mode == kSaveMode) {
		// [Text Render] TextBox Caption
		ZoombiniGraphics::TextConf titleConf;
		titleConf._fontUsage = ZoombiniFontUsage::kFontTitle;
		titleConf._wordWrap = false;
		_vm->_gfx->drawText(screenKind, ZoombiniText::kDialogTitleSaveAs, _saveAsCaptionRect, titleConf);

		// [Text Render] Text input box content
		ZoombiniGraphics::TextConf inputConf;
		inputConf._fontUsage = ZoombiniFontUsage::kFontTitle;
		inputConf._textPalette = ZoombiniGraphics::kColor2D_Black;
		inputConf._wordWrap = false;
		Common::U32String displayedText = getDisplayedSaveInputText();
		if (!displayedText.empty()) {
			_vm->_gfx->drawText(screenKind, displayedText, _saveTextBoxRect, inputConf);
		}

		// Draw the cursor
		if (_saveInputCursorVisible) {
			uint32 displayedCursorPos = _saveInputCursorPos + _saveInputCompositionCursorPos;
			Common::U32String cursorStr = displayedText.substr(0, displayedCursorPos);

			int16 fontHeight = _vm->_gfx->getFontHeight(inputConf);
			int16 textOffsetY = MAX(0, (_saveTextBoxRect.height() - fontHeight) / 2);
			int cursorX = _vm->_gfx->getTextWidth(cursorStr, inputConf);

			Common::Point cursorTop = Common::Point(_saveTextBoxRect.left + cursorX, _saveTextBoxRect.top + textOffsetY);
			Common::Point cursorBottom = Common::Point(_saveTextBoxRect.left + cursorX, _saveTextBoxRect.bottom - textOffsetY);
			_vm->_gfx->drawLine(screenKind, cursorTop, cursorBottom, ZoombiniGraphics::kColor2D_Black);
		}

		// [Text Render] SaveGame List
		textPalette = ZoombiniGraphics::kColor0D_LightGray;
	} else {
		// [Text Render] LoadGame List
		textPalette = ZoombiniGraphics::kColor2D_Black;
	}

	// [Text Render] SaveEntry List (Up to 8 in one screen)
	for (int32 slotIdx = _saveEntryBaseIdx; slotIdx < _saveEntryBaseIdx + SAVESLOTS_PER_SCREEN && slotIdx < _vm->_state->_r.getEntryCount(); slotIdx++) {
		const ZmbRosterEntry &roster = _vm->_state->_r.getEntry(slotIdx);
		const Common::U32String &saveName = roster.getSaveName(_vm);

		ZoombiniGraphics::TextConf entryConf;
		entryConf._fontUsage = ZoombiniFontUsage::kFontTitle;
		entryConf._textPalette = textPalette;
		entryConf._wordWrap = false;
		if (_mode != kSaveMode && _saveEntrySelectedIdx == slotIdx) {
			entryConf._outlineEffect = true;
			entryConf._outlinePalette = ZoombiniGraphics::kColor2D_Black;
			entryConf._textPalette = ZoombiniGraphics::kColor22_LimeGreen;
		}

		_vm->_gfx->drawText(screenKind, saveName, saveEntryRect, entryConf);

		saveEntryRect.top += 20;
		saveEntryRect.bottom += 20;
	}
}

ZmbEventHandleResult ZoombiniDialogSaveLoad::dialogFrame_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos) {
	(void)feature;
	(void)relPos;

	if (_mode == kSaveMode)
		return ZmbEventHandleResult::kPassthrough;

	// Load dialogs only
	Common::Rect saveEntryRect = getSaveEntryBaseRect();
	for (int32 rowIdx = 0; rowIdx < SAVESLOTS_PER_SCREEN; rowIdx++) {
		if (saveEntryRect.contains(absPos)) {
			const int32 previousSelectedIdx = _saveEntrySelectedIdx;
			const int32 slotIdx = _saveEntryBaseIdx + rowIdx;
			if (slotIdx < _vm->_state->_r.getEntryCount())
				_saveEntrySelectedIdx = slotIdx;
			else
				_saveEntrySelectedIdx = -1;
			markDialogFrameDirty();
			if (0 <= _saveEntrySelectedIdx && previousSelectedIdx == _saveEntrySelectedIdx &&
				_currentFrameCounter - _lastSaveEntryClickedFrame <= MohawkEngine_Zoombini::kDoubleClickFrameRate) { // Double-click
				_longButtonStateMap[kSaveLoadDialogButton03_Okay].press(_vm, _longButtonsFeature, _currentFrameCounter);
			}
			_lastSaveEntryClickedFrame = _currentFrameCounter;
			return ZmbEventHandleResult::kConsumed;
		}

		saveEntryRect.top += 20;
		saveEntryRect.bottom += 20;
	}

	return ZmbEventHandleResult::kPassthrough;
}

ZmbEventHandleResult ZoombiniDialogSaveLoad::dialogFrame_onKeyDown(ZmbFeature *feature, const Common::KeyState &kbd, bool kbdRepeat) {
	(void)feature;
	(void)kbdRepeat;

	ZmbEventHandleResult result = ZmbEventHandleResult::kPassthrough;
	if (_mode == kSaveMode) {
		// [SaveDialog] TextBox for SaveDialog
		switch (kbd.keycode) {
		case Common::KEYCODE_BACKSPACE:
			if (0 < _saveInputCursorPos) {
				_saveInputText.deleteChar(_saveInputCursorPos - 1);
				_saveInputCursorPos -= 1;
				result = ZmbEventHandleResult::kConsumed;
			}
			break;
		case Common::KEYCODE_DELETE:
			if (!_vm->useEnhancedKbdShortcuts())
				break;
			if (_saveInputCursorPos < _saveInputText.size()) {
				_saveInputText.deleteChar(_saveInputCursorPos);
				result = ZmbEventHandleResult::kConsumed;
			}
			break;
		case Common::KEYCODE_LEFT:
			if (0 < _saveInputCursorPos) {
				_saveInputCursorPos -= 1;
				result = ZmbEventHandleResult::kConsumed;
			}
			break;
		case Common::KEYCODE_RIGHT:
			if (_saveInputCursorPos < _saveInputText.size()) {
				_saveInputCursorPos += 1;
				result = ZmbEventHandleResult::kConsumed;
			}
			break;
		case Common::KEYCODE_HOME:
			if (!_vm->useEnhancedKbdShortcuts())
				break;
			_saveInputCursorPos = 0;
			result = ZmbEventHandleResult::kConsumed;
			break;
		case Common::KEYCODE_END:
			if (!_vm->useEnhancedKbdShortcuts())
				break;
			_saveInputCursorPos = _saveInputText.size();
			result = ZmbEventHandleResult::kConsumed;
			break;
		default: // Handle typing of printable characters
			// When Windows IME is compositing characters, no keyboard event is produced.
			// When Windows IME has finished compositing characters, keyboard event with KEYCODE_INVALID is produced.
			// @ref Common::KEYCODE_DELETE passes this check, but it has been handled above.
			if ((Common::KEYCODE_EXCLAIM <= kbd.keycode && kbd.keycode <= Common::KEYCODE_TILDE) ||
				kbd.keycode == Common::KEYCODE_INVALID ||
				(Common::KEYCODE_KP0 <= kbd.keycode && kbd.keycode <= Common::KEYCODE_KP9 && (kbd.flags & Common::KBD_NUM)) ||
				(Common::KEYCODE_KP_PERIOD <= kbd.keycode && kbd.keycode <= Common::KEYCODE_KP_EQUALS && kbd.keycode != Common::KEYCODE_KP_ENTER)) {
				result = saveTextBox_handleTyping(kbd);
			}
			break;
		}
	} else {
		// [LoadDialog]
		// - Ctrl+R opens the remove-game confirmation overlay in every release.
		//   TLC v2.0 provides this shortcut in-game. ScummVM also offers it to
		//   v1.x, whose Windows releases may rely on the external ZoomUser utility.
		// - Delete aliases Ctrl+R when enhanced keyboard shortcuts are enabled.
		// - Up/Down Arrow
		//   * Enhanced shortcut mode enabled: Select previous/next save entry
		//   * Enhanced shortcut mode disabled: Scroll one page up/down
		const bool removeSaveShortcut =
			(kbd.keycode == Common::KEYCODE_r && kbd.hasFlags(Common::KBD_CTRL)) ||
			(kbd.keycode == Common::KEYCODE_DELETE && _vm->useEnhancedKbdShortcuts());
		if (removeSaveShortcut) {
			if (0 <= _saveEntrySelectedIdx && _saveEntrySelectedIdx < _vm->_state->_r.getEntryCount()) {
				ZoombiniDialogResult dialogResult = _vm->openMsgBoxDialog(ZoombiniMsgBoxType::kAskRemoveSave);
				if (dialogResult == ZoombiniDialogResult::kYes)
					_vm->_state->deleteGameAndShiftRoster(_saveEntrySelectedIdx);
				clampLoadSelection();
			}
			result = ZmbEventHandleResult::kConsumed;
		} else {
			if (!_vm->useEnhancedKbdShortcuts())
				return result;

			switch (getKeyboardNavDirection(kbd)) {
			case KBD_NAV_UP:
				_saveEntrySelectedIdx = MAX<int32>(0, _saveEntrySelectedIdx - 1);
				result = ZmbEventHandleResult::kConsumed;
				break;
			case KBD_NAV_DOWN:
				_saveEntrySelectedIdx = MIN<int32>(_saveEntrySelectedIdx + 1, MAX<int32>(_vm->_state->_r.getEntryCount() - 1, 0));
				result = ZmbEventHandleResult::kConsumed;
				break;
			default:
				return result;
			}
		}

		clampLoadSelection();
	}

	if (result == ZmbEventHandleResult::kConsumed)
		removeLeadingSaveInputSpaces();
	if (result == ZmbEventHandleResult::kConsumed)
		markDialogFrameDirty();

	return result;
}

ZmbEventHandleResult ZoombiniDialogSaveLoad::saveTextBox_handleTyping(const Common::KeyState &kbd) {
	ZmbEventHandleResult result = ZmbEventHandleResult::kPassthrough;

	// The key event's ASCII field contains the Unicode code point of the typed key.
	char32_t uch = kbd.ascii;

	// ASCII range - filter out control characters
	if (uch < 128 && !Common::isPrint(kbd.ascii))
		return result;

	if (!_saveInputCompositionText.empty()) {
		_saveInputCompositionText.clear();
		_saveInputCompositionCursorPos = 0;
		result = ZmbEventHandleResult::kConsumed;
	}

	// Check if adding this character would exceed maximum save name length
	// Non-ASCII characters may take more than 1 byte in the target code page
	Common::U32String newInputText = _saveInputText;
	newInputText.insertChar(kbd.ascii, _saveInputCursorPos);
	if (ZmbRosterEntry::checkSaveNameSize(_vm, newInputText)) {
		_saveInputText = newInputText;
		_saveInputCursorPos += 1;
		result = ZmbEventHandleResult::kConsumed;
	}

	// SDL emits committed IME text as key events with Unicode code points.
	return result;
}

void ZoombiniDialogSaveLoad::scrollButtons_onPreRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots) {
	(void)hsGroup;

	genericButton_selectShapes(feature, hotspots, _scrollButtonStateMap);
}

void ZoombiniDialogSaveLoad::scrollButtons_onPostRender(ZmbFeature *feature) {
	// [Post-Animation Events]
	genericButton_action(feature, _scrollButtonStateMap, static_cast<ZoombiniPage::OnButtonActionFunc>(&ZoombiniDialogSaveLoad::scrollButtons_onButtonAction));
}

void ZoombiniDialogSaveLoad::scrollButtons_onButtonAction(ZmbFeature *feature, uint32 bsIdx, ButtonState &bs) {
	(void)feature;
	(void)bs;

	int32 oldSaveEntryBaseIdx = _saveEntryBaseIdx;

	switch (bsIdx) {
	case kSaveLoadDialogButton01_ScrollUp:
		_saveEntryBaseIdx -= SAVESLOTS_PER_SCREEN;
		break;
	case kSaveLoadDialogButton02_ScrollDown:
		_saveEntryBaseIdx += SAVESLOTS_PER_SCREEN;
		break;
	default:
		error("saveload: invalid saveload dialog button event(%u)", bsIdx);
		break;
	}

	// Keep @ref ZoombiniDialogSaveLoad::_saveEntryBaseIdx within the visible save-entry range.
	// @ref ZmbRosterFile::getEntryCount() defines the upper bound.
	_saveEntryBaseIdx = CLIP<int32>(_saveEntryBaseIdx, 0, MAX<int32>(_vm->_state->_r.getEntryCount() - SAVESLOTS_PER_SCREEN, 0));
	if (oldSaveEntryBaseIdx != _saveEntryBaseIdx)
		markDialogFrameDirty();
}

ZmbEventHandleResult ZoombiniDialogSaveLoad::scrollButtons_onWheelUp(ZmbFeature *feature, const Common::Point &absPos) {
	(void)absPos;

	scrollButtons_onButtonAction(feature, kSaveLoadDialogButton01_ScrollUp, _scrollButtonStateMap[kSaveLoadDialogButton01_ScrollUp]);
	return ZmbEventHandleResult::kConsumed;
}

ZmbEventHandleResult ZoombiniDialogSaveLoad::scrollButtons_onWheelDown(ZmbFeature *feature, const Common::Point &absPos) {
	(void)absPos;

	scrollButtons_onButtonAction(feature, kSaveLoadDialogButton02_ScrollDown, _scrollButtonStateMap[kSaveLoadDialogButton02_ScrollDown]);
	return ZmbEventHandleResult::kConsumed;
}

ZmbEventHandleResult ZoombiniDialogSaveLoad::scrollButtons_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos) {
	(void)relPos;

	return genericButton_onLButtonDown(feature, absPos, _scrollButtonStateMap);
}

ZmbEventHandleResult ZoombiniDialogSaveLoad::scrollButtons_onKeyDown(ZmbFeature *feature, const Common::KeyState &kbd, bool kbdRepeat) {
	(void)kbdRepeat;

	ZmbEventHandleResult result = ZmbEventHandleResult::kPassthrough;
	if (kbd.hasFlags(0)) {
		switch (getKeyboardNavDirection(kbd)) {
		case KBD_NAV_PAGEUP: // PgUp - enhanced shortcut: scroll one page up
			if (!_vm->useEnhancedKbdShortcuts())
				return result;
			if (!_scrollButtonStateMap[kSaveLoadDialogButton01_ScrollUp].isAnimating())
				_scrollButtonStateMap[kSaveLoadDialogButton01_ScrollUp].press(_vm, feature, _currentFrameCounter);
			result = ZmbEventHandleResult::kConsumed;
			break;
		case KBD_NAV_UP: // Up Arrow
			// Enhanced shortcut mode enabled: Select previous save entry
			// Enhanced shortcut mode disabled: Scroll one page up
			if (isLoadDialog() && _vm->useEnhancedKbdShortcuts())
				return result;
			if (!_scrollButtonStateMap[kSaveLoadDialogButton01_ScrollUp].isAnimating())
				_scrollButtonStateMap[kSaveLoadDialogButton01_ScrollUp].press(_vm, feature, _currentFrameCounter);
			result = ZmbEventHandleResult::kConsumed;
			break;
		case KBD_NAV_PAGEDOWN: // PgDn - enhanced shortcut: scroll one page down
			if (!_vm->useEnhancedKbdShortcuts())
				return result;
			if (!_scrollButtonStateMap[kSaveLoadDialogButton02_ScrollDown].isAnimating())
				_scrollButtonStateMap[kSaveLoadDialogButton02_ScrollDown].press(_vm, feature, _currentFrameCounter);
			result = ZmbEventHandleResult::kConsumed;
			break;
		case KBD_NAV_DOWN: // Down Arrow
			// Enhanced shortcut mode enabled: Select next save entry
			// Enhanced shortcut mode disabled: Scroll one page down
			if (isLoadDialog() && _vm->useEnhancedKbdShortcuts())
				return result;
			if (!_scrollButtonStateMap[kSaveLoadDialogButton02_ScrollDown].isAnimating())
				_scrollButtonStateMap[kSaveLoadDialogButton02_ScrollDown].press(_vm, feature, _currentFrameCounter);
			result = ZmbEventHandleResult::kConsumed;
			break;
		default:
			break;
		}
	}
	return result;
}

void ZoombiniDialogSaveLoad::longButtons_onPreRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots) {
	(void)hsGroup;

	genericButton_selectShapes(feature, hotspots, _longButtonStateMap);
}

void ZoombiniDialogSaveLoad::longButtons_onPostRender(ZmbFeature *feature) {
	// [Post-Animation Events]
	genericButton_action(feature, _longButtonStateMap, static_cast<ZoombiniPage::OnButtonActionFunc>(&ZoombiniDialogSaveLoad::longButtons_onButtonAction));

	// [Text Render] Long Button Descriptions
	genericButton_textRender(feature, _longButtonStateMap, Graphics::kTextAlignCenter);
}

void ZoombiniDialogSaveLoad::longButtons_onButtonAction(ZmbFeature *feature, uint32 bsIdx, ButtonState &bs) {
	(void)feature;
	(void)bs;

	switch (bsIdx) {
	case kSaveLoadDialogButton03_Okay:
		if (_mode == kSaveMode) {
			Common::U32String saveInputText = getDisplayedSaveInputText();
			while (!saveInputText.empty() && saveInputText[saveInputText.size() - 1] == U' ')
				saveInputText.deleteChar(saveInputText.size() - 1);
			_saveInputText = saveInputText;
			_saveInputCompositionText.clear();
			_saveInputCompositionCursorPos = 0;
			_saveInputCursorPos = _saveInputText.size();
			// Do nothing when the savename is empty
			if (saveInputText.empty()) {
				markDialogFrameDirty();
				return;
			}
			if (!ZmbRosterEntry::isSaveNameEncodingValid(saveInputText, _vm->_text->getExeCodePage())) {
				_vm->openMsgBoxDialog(_vm->_text->getLocalizedString(ZoombiniText::kSvmDialogBodyCannotSaveNameEncoding));
				return;
			}

			// Search for existing save name in the selected slot
			int slot = _vm->_state->searchSaveSlotByName(saveInputText);
			if (slot < 0) { // New save name - check for available slots
				slot = _vm->_state->getAvailableSaveSlot();
				if (slot < 0) { // No available slots
					_vm->openMsgBoxDialog(ZoombiniMsgBoxType::kAlertCannotSaveMoreGames);
					return;
				} else if (!_vm->_state->saveNewGame(saveInputText)) {
					return;
				}
			} else { // Existing save name - Ask for confirmation to overwrite
				// Append the typed name to the localized prefix, followed by the closing quote and question mark.
				// This preserves locale-specific spacing following the opening quote.
				Common::U32String message = _vm->_text->getLocalizedString(ZoombiniText::kDialogBodyReplaceGame);
				message += saveInputText;
				message += U"\"?";
				ZoombiniDialogResult dialogResult = _vm->openConfirmMsgBoxDialog(message);
				if (dialogResult != ZoombiniDialogResult::kYes || !_vm->_state->saveGame(slot))
					return;
			}

			_dialogResult = ZoombiniDialogResult::kYes;
			close();
		} else {
			if (_saveEntrySelectedIdx < 0 || _vm->_state->_r.getEntryCount() <= _saveEntrySelectedIdx)
				return;

			if (!_vm->_state->loadGame(_saveEntrySelectedIdx)) {
				if (!_vm->_state->wasLastLoadCancelled())
					_vm->openMsgBoxDialog(_vm->_text->getLocalizedString(ZoombiniText::kSvmDialogBodyCannotLoadSave));
				break;
			}

			_vm->getActivePage()->close();
			_vm->setNextPage(_vm->_state->getCurrentState().getCurrentPageType());

			_dialogResult = ZoombiniDialogResult::kYes;
			close();
		}
		break;
	case kSaveLoadDialogButton04_Cancel:
		_dialogResult = ZoombiniDialogResult::kNo;
		close();
		break;
	default:
		error("saveload: invalid saveload dialog button event(%u)", bsIdx);
		break;
	}
}

ZmbEventHandleResult ZoombiniDialogSaveLoad::longButtons_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos) {
	(void)relPos;

	return genericButton_onLButtonDown(feature, absPos, _longButtonStateMap);
}

ZmbEventHandleResult ZoombiniDialogSaveLoad::longButtons_onKeyDown(ZmbFeature *feature, const Common::KeyState &kbd, bool kbdRepeat) {
	(void)kbdRepeat;

	ZmbEventHandleResult result = ZmbEventHandleResult::kPassthrough;
	switch (classifyDialogKey(kbd)) {
	case kDialogKeyCancel:
		if (!_longButtonStateMap[kSaveLoadDialogButton04_Cancel].isAnimating())
			_longButtonStateMap[kSaveLoadDialogButton04_Cancel].press(_vm, feature, _currentFrameCounter);
		result = ZmbEventHandleResult::kConsumed;
		break;
	case kDialogKeyAccept:
		if (!_longButtonStateMap[kSaveLoadDialogButton03_Okay].isAnimating())
			_longButtonStateMap[kSaveLoadDialogButton03_Okay].press(_vm, feature, _currentFrameCounter);
		result = ZmbEventHandleResult::kConsumed;
		break;
	case kDialogKeyNone:
		break;
	}
	return result;
}

} // End of namespace Mohawk
