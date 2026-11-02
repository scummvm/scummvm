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

#include "mohawk/zoombini_pages/dialog_load.h"
#include "mohawk/zoombini.h"
#include "mohawk/zoombini_graphics.h"
#include "mohawk/zoombini_state.h"

namespace Mohawk {

ZoombiniDialogLoad::ZoombiniDialogLoad(MohawkEngine_Zoombini *vm, bool newGameMode)
	: ZoombiniDialogSaveLoad(vm, ZoombiniPageType::kDialogLoad, ZoombiniText::kDialogTitleLoad, Common::Rect(0x00FA, 0x0043, 0x0198, 0x005F),
							 ZoombiniText::kDialogButtonLoad, ZoombiniText::kDialogButtonCancel, _saveEntryLoadModeTop) {
	// The red Load dialog button uses the localized dialog label.
	// @ref ZoombiniText::kNewGame is the executable-sourced default name for an unsaved game instead.
	if (newGameMode)
		_longButtonStateMap[kSaveLoadDialogButton04_Cancel]._textKey = ZoombiniText::kDialogButtonNewGame;
}

void ZoombiniDialogLoad::loadFeatures() {
	loadSaveLoadFeatures(kSysResScrb0004_DialogLoad, kSysResScrb0005_DialogLoad, kSysResScrb0006_DialogLoad);
}

void ZoombiniDialogLoad::dialogFrame_onPostRender(ZmbFeature *feature) {
	(void)feature;

	drawDialogTitle();
	drawSaveEntries(ZoombiniGraphics::kColor2D_Black, _saveEntrySelectedIdx);
}

ZmbEventHandleResult ZoombiniDialogLoad::dialogFrame_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos) {
	(void)feature;
	(void)relPos;

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

ZmbEventHandleResult ZoombiniDialogLoad::dialogFrame_onKeyDown(ZmbFeature *feature, const Common::KeyState &kbd, bool kbdRepeat) {
	(void)feature;
	(void)kbdRepeat;

	ZmbEventHandleResult result = ZmbEventHandleResult::kPassthrough;
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

	if (result == ZmbEventHandleResult::kConsumed)
		markDialogFrameDirty();

	return result;
}

void ZoombiniDialogLoad::clampLoadSelection() {
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

void ZoombiniDialogLoad::confirm() {
	if (_saveEntrySelectedIdx < 0 || _vm->_state->_r.getEntryCount() <= _saveEntrySelectedIdx)
		return;

	if (!_vm->_state->loadGame(_saveEntrySelectedIdx)) {
		if (!_vm->_state->wasLastLoadCancelled())
			_vm->openMsgBoxDialog(_vm->_text->getLocalizedString(ZoombiniText::kSvmDialogBodyCannotLoadSave));
		return;
	}

	_vm->getActivePage()->close();
	_vm->setNextPage(_vm->_state->getCurrentState().getCurrentPageType());

	_dialogResult = ZoombiniDialogResult::kYes;
	close();
}

} // End of namespace Mohawk
