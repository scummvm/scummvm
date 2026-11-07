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

#ifndef MOHAWK_ZOOMBINI_PAGES_DIALOG_SAVE_H
#define MOHAWK_ZOOMBINI_PAGES_DIALOG_SAVE_H

#include "mohawk/zoombini_pages/dialog_saveload_base.h"

namespace Mohawk {

/** Save dialog with editable, release-encoded journey names. */
class ZoombiniDialogSave : public ZoombiniDialogSaveLoad {
public:
	/** Create the save page and initialize its name from the active journey. */
	ZoombiniDialogSave(MohawkEngine_Zoombini *vm);
	/** Load the save frame, scrolling and command SCRBs. */
	void loadFeatures() override;
	/** Advance the save-name cursor blink timer. */
	void onPreTickFrame() override;
#if ZOOMBINI_ENABLE_IME_COMPOSITION
	/** Update, commit, or cancel IME composition in the save-name field. */
	ZmbEventHandleResult onImeComposition(const Common::ImeComposition &composition) override;
	/** Enable native IME composition for the save-name field. */
	bool wantsImeComposition() const override { return true; }
#endif

	/** Draw the title, editable save name, cursor and existing save entries. */
	void dialogFrame_onPostRender(ZmbFeature *feature) override;
	/** Edit the save-name text and update its cursor. */
	ZmbEventHandleResult dialogFrame_onKeyDown(ZmbFeature *feature, const Common::KeyState &kbd, bool kbdRepeat) override;
	/** Insert one printable Unicode character into the save-name field. */
	ZmbEventHandleResult saveTextBox_handleTyping(char32_t character);

private:
	/** Validate the edited name, confirm replacements and save the current journey. */
	void confirm() override;
	/** Remove ASCII spaces exposed at the start of the editable save name. */
	void removeLeadingSaveInputSpaces();
	/** Return committed text with the active IME composition inserted at the cursor. */
	Common::U32String getDisplayedSaveInputText() const;

	/** Top coordinate of save rows in the save dialog. */
	static constexpr uint16 _saveEntrySaveModeTop = 94;
	/**
	 * Dialog layout rectangles are instance members because Common::Rect requires
	 * runtime construction and ScummVM prohibits global C++ constructors.
	 */
	/** Rectangle containing the save-as caption. */
	const Common::Rect _saveAsCaptionRect = Common::Rect(0x00BB, 0x010F, 0x017C, 0x0122);
	/** Rectangle containing the editable save-name field, adjusted for the release. */
	Common::Rect _saveTextBoxRect = Common::Rect(0x00C0, 0x012A, 0x01C8, 0x013C);
	/** Committed text in the save-name field. */
	Common::U32String _saveInputText;
	/** Uncommitted native-IME composition text. */
	Common::U32String _saveInputCompositionText;
	/** Cursor position within the active IME composition. */
	uint32 _saveInputCompositionCursorPos = 0;
	/** Cursor position within the committed save-name text. */
	uint32 _saveInputCursorPos = 0;
	/** Last timestamp used to blink the save-name cursor. */
	uint32 _saveInputCursorLastBlinkTimeMs = 0;
	/** Whether the save-name cursor is currently visible. */
	bool _saveInputCursorVisible = true;
	/** Maximum serialized byte length accepted for a save name. */
	static constexpr uint32 MAX_SAVENAME_BYTES = 22;
};

} // End of namespace Mohawk

#endif
