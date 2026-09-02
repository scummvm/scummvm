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

#ifndef MOHAWK_ZOOMBINI_PAGES_DIALOG_BASE_H
#define MOHAWK_ZOOMBINI_PAGES_DIALOG_BASE_H

#include "mohawk/zoombini_page.h"

namespace Mohawk {

/**
 * Common implementation for modal Zoombini dialogs.
 *
 * A dialog is still a @ref ZoombiniPage, but it owns the screen-space rules
 * needed to present a modal surface over the page below it. The base class
 * captures the covered screen before the dialog is drawn and restores the
 * dialog result to its caller through @ref ZoombiniDialog::getResult().
 *
 * Derived dialogs provide their feature callbacks and decide whether the
 * result is a yes/no answer, a save/load action, or no result at all.
 */
class ZoombiniDialog : public ZoombiniPage {
public:
	/** Create a dialog for the supplied page type. */
	ZoombiniDialog(MohawkEngine_Zoombini *vm, ZoombiniPageType pageType);
	/** Release the captured background and all page-owned resources. */
	~ZoombiniDialog() override;

	/** Select the dialog background and capture the covered page surface. */
	void setBackgroundBitmap() override;
	/** Dispatch dialog-owned keys, then apply the original Ctrl+Q fallback. */
	ZmbEventHandleResult onKeyDown(const Common::KeyState &kbd, bool kbdRepeat) override;

	/**
	 * Return the result selected by the dialog.
	 *
	 * @return The current result, or @ref ZoombiniDialogResult::kNone when the
	 *         dialog has not produced one.
	 */
	ZoombiniDialogResult getResult() { return _dialogResult; }

protected:
	/** Semantic action represented by the common dialog completion keys. */
	enum DialogKeyAction {
		kDialogKeyNone,
		kDialogKeyAccept,
		kDialogKeyCancel
	};

	/**
	 * Classify Return, keypad Enter, and Escape without choosing a button.
	 * Concrete dialogs retain ownership of action mapping and dialog state.
	 */
	static DialogKeyAction classifyDialogKey(const Common::KeyState &kbd);

	/** Reset every long-button state owned by the active dialog. */
	void resetLongButtonStates();

	/** Shared long-button state storage used by concrete modal dialogs. */
	Common::StableMap<uint32, ButtonState> _longButtonStateMap;

	/**
	 * Yes/No button result of the dialog.
	 * If the dialog does not produce a result, it will be kNone, and will be ignored.
	 */
	ZoombiniDialogResult _dialogResult = ZoombiniDialogResult::kNone;

private:
	/** Snapshot of the page surface covered by the modal dialog. */
	Graphics::Surface _capturedBackScreen;
};

} // End of namespace Mohawk

#endif
