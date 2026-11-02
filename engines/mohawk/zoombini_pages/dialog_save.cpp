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

#include "mohawk/zoombini_pages/dialog_save.h"
#include "mohawk/zoombini.h"
#include "mohawk/zoombini_graphics.h"
#include "mohawk/zoombini_state.h"

namespace Mohawk {

ZoombiniDialogSave::ZoombiniDialogSave(MohawkEngine_Zoombini *vm)
	: ZoombiniDialogSaveLoad(vm, ZoombiniPageType::kDialogSave, ZoombiniText::kDialogTitleSave, Common::Rect(0x00FA, 0x0039, 0x0198, 0x0055),
							 ZoombiniText::kDialogButtonSave, ZoombiniText::kDialogButtonCancel, _saveEntrySaveModeTop) {
	_saveTextBoxRect = _vm->getRelInfo().getLayoutAdjustment(ZmbReleaseInfo::LayoutKind::kSaveNameInput).apply(_saveTextBoxRect);
	_saveInputText = _vm->_state->getActiveSaveName();
	_saveInputCursorPos = _saveInputText.size();
}

void ZoombiniDialogSave::loadFeatures() {
	loadSaveLoadFeatures(kSysResScrb0007_DialogSave, kSysResScrb0008_DialogSave, kSysResScrb0009_DialogSave);
}

void ZoombiniDialogSave::onPreTickFrame() {
	uint32 cursorBlinkDelta = _currentFrameTime - _saveInputCursorLastBlinkTimeMs;
	if (MohawkEngine_Zoombini::kTextCursorBlinkFrameTimeMs <= cursorBlinkDelta) {
		_saveInputCursorLastBlinkTimeMs = _currentFrameTime;
		_saveInputCursorVisible = !_saveInputCursorVisible;
		markDialogFrameDirty();
	}
}

#if ZOOMBINI_ENABLE_IME_COMPOSITION
ZmbEventHandleResult ZoombiniDialogSave::onImeComposition(const Common::ImeComposition &composition) {
	if (composition.state == Common::ImeComposition::kComplete && !composition.text.empty()) {
		_saveInputCompositionText.clear();
		_saveInputCompositionCursorPos = 0;
		for (char32_t character : composition.text) {
			if (saveTextBox_handleTyping(character) == ZmbEventHandleResult::kConsumed)
				removeLeadingSaveInputSpaces();
		}
		_saveInputCursorVisible = true;
		_saveInputCursorLastBlinkTimeMs = _currentFrameTime;
		markDialogFrameDirty();
		return ZmbEventHandleResult::kConsumed;
	}

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
#endif

void ZoombiniDialogSave::removeLeadingSaveInputSpaces() {
	while (!_saveInputText.empty() && _saveInputText[0] == U' ') {
		_saveInputText.deleteChar(0);
		if (0 < _saveInputCursorPos)
			_saveInputCursorPos -= 1;
	}
}

Common::U32String ZoombiniDialogSave::getDisplayedSaveInputText() const {
	Common::U32String displayedText = _saveInputText;
	if (!_saveInputCompositionText.empty())
		displayedText.insertString(_saveInputCompositionText, _saveInputCursorPos);
	return displayedText;
}

void ZoombiniDialogSave::dialogFrame_onPostRender(ZmbFeature *feature) {
	(void)feature;

	ZoombiniGraphics::ScreenKind screenKind = ZoombiniGraphics::kShapeScreen;
	drawDialogTitle();

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

	drawSaveEntries(ZoombiniGraphics::kColor0D_LightGray);
}

ZmbEventHandleResult ZoombiniDialogSave::dialogFrame_onKeyDown(ZmbFeature *feature, const Common::KeyState &kbd, bool kbdRepeat) {
	(void)feature;
	(void)kbdRepeat;

	ZmbEventHandleResult result = ZmbEventHandleResult::kPassthrough;
	// [SaveDialog] TextBox for SaveDialog
	Common::KeyCode editingKey = kbd.keycode;
	if (kbd.hasFlags(Common::KBD_CTRL) && editingKey == Common::KEYCODE_h)
		editingKey = Common::KEYCODE_BACKSPACE;
	switch (editingKey) {
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
		// Accept key-event text when the backend is not delivering composition commits.
		// @ref Common::KEYCODE_DELETE passes this check, but it has been handled above.
		if ((Common::KEYCODE_EXCLAIM <= kbd.keycode && kbd.keycode <= Common::KEYCODE_TILDE) ||
			kbd.keycode == Common::KEYCODE_INVALID ||
			(Common::KEYCODE_KP0 <= kbd.keycode && kbd.keycode <= Common::KEYCODE_KP9 && (kbd.flags & Common::KBD_NUM)) ||
			(Common::KEYCODE_KP_PERIOD <= kbd.keycode && kbd.keycode <= Common::KEYCODE_KP_EQUALS && kbd.keycode != Common::KEYCODE_KP_ENTER)) {
			result = saveTextBox_handleTyping(kbd.ascii);
		}
		break;
	}

	if (result == ZmbEventHandleResult::kConsumed) {
#if ZOOMBINI_ENABLE_IME_COMPOSITION
		g_system->cancelImeComposition();
		_saveInputCompositionText.clear();
		_saveInputCompositionCursorPos = 0;
#endif
		removeLeadingSaveInputSpaces();
		markDialogFrameDirty();
	}

	return result;
}

ZmbEventHandleResult ZoombiniDialogSave::saveTextBox_handleTyping(char32_t character) {
	ZmbEventHandleResult result = ZmbEventHandleResult::kPassthrough;

	// Key events and committed IME text share the same character insertion rules.
	// ASCII range - filter out control characters
	if (character < 128 && !Common::isPrint(character))
		return result;

	if (!_saveInputCompositionText.empty()) {
		_saveInputCompositionText.clear();
		_saveInputCompositionCursorPos = 0;
		result = ZmbEventHandleResult::kConsumed;
	}

	// Check if adding this character would exceed maximum save name length
	// Non-ASCII characters may take more than 1 byte in the target code page
	Common::U32String newInputText = _saveInputText;
	newInputText.insertChar(character, _saveInputCursorPos);
	if (ZmbRosterEntry::checkSaveNameSize(_vm, newInputText)) {
		_saveInputText = newInputText;
		_saveInputCursorPos += 1;
		result = ZmbEventHandleResult::kConsumed;
	}

	return result;
}

void ZoombiniDialogSave::confirm() {
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
}

} // End of namespace Mohawk
