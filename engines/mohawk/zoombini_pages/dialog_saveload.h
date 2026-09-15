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

#ifndef MOHAWK_ZOOMBINI_PAGES_DIALOG_SAVELOAD_H
#define MOHAWK_ZOOMBINI_PAGES_DIALOG_SAVELOAD_H

#include "mohawk/zoombini_pages/dialog_base.h"

namespace Mohawk {

/**
 * Save/load dialog page.
 *
 * The mode selects whether the dialog writes a new save, loads an existing
 * save, or offers both loading and starting a new game. Save entries are
 * presented in a scrollable list, while the text-box callback handles the
 * restricted filename input used by the save path.
 */
class ZoombiniDialogSaveLoad : public ZoombiniDialog {
public:
	/** Which operation the dialog exposes to the player. */
	enum SaveLoadMode {
		/** Save the current journey. */
		kSaveMode,
		/** Load an existing journey. */
		kLoadMode,
		/** Load an existing journey or start a new one. */
		kLoadOrNewMode,
	};

	/** Create a save/load dialog in @p mode. */
	ZoombiniDialogSaveLoad(MohawkEngine_Zoombini *vm, SaveLoadMode mode);
	/** Release save entries, text input, and dialog features. */
	~ZoombiniDialogSaveLoad() override;

	/** Return true when this instance writes a save entry. */
	bool isSaveDialog() const { return _mode == kSaveMode; }
	/** Return true when this instance loads or offers a new game. */
	bool isLoadDialog() const { return _mode != kSaveMode; }

	/** Load save-entry rows, buttons, and text-box features. */
	void loadFeatures() override;
	/** Advance save-entry animations and delayed actions. */
	void onEveryFrame() override;
	/** Update or cancel the uncommitted IME composition shown in the save-name field. */
	ZmbEventHandleResult onImeComposition(const Common::ImeComposition &composition) override;
	/** Enable native IME composition only for the save-name variant. */
	bool wantsImeComposition() const override { return _mode == kSaveMode; }

	/** Return the screen rectangle occupied by a save entry row. */
	Common::Rect getSaveEntryBaseRect();

	/** Draw the save/load frame and visible entry rows. */
	void dialogFrame_onPostRender(ZmbFeature *feature);
	/** Handle selection of a save entry or dialog command. */
	ZmbEventHandleResult dialogFrame_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos);
	/** Handle keyboard selection and text-box focus. */
	ZmbEventHandleResult dialogFrame_onKeyDown(ZmbFeature *feature, const Common::KeyState &kbd, bool kbdRepeat);
	/** Insert, delete, or move the cursor for save-name input. */
	ZmbEventHandleResult saveTextBox_handleTyping(const Common::KeyState &kbd);

	/** Update scroll-button shapes before drawing the list. */
	void scrollButtons_onPreRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots);
	/** Restore the save/load frame after scroll-button rendering. */
	void scrollButtons_onPostRender(ZmbFeature *feature);
	/** Apply a scroll-button action. */
	void scrollButtons_onButtonAction(ZmbFeature *feature, uint32 bsIdx, ButtonState &bs);
	/** Scroll the save list toward older entries. */
	ZmbEventHandleResult scrollButtons_onWheelUp(ZmbFeature *feature, const Common::Point &absPos);
	/** Scroll the save list toward newer entries. */
	ZmbEventHandleResult scrollButtons_onWheelDown(ZmbFeature *feature, const Common::Point &absPos);
	/** Handle pointer input for save-list scrolling. */
	ZmbEventHandleResult scrollButtons_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos);
	/** Handle keyboard input for save-list scrolling. */
	ZmbEventHandleResult scrollButtons_onKeyDown(ZmbFeature *feature, const Common::KeyState &kbd, bool kbdRepeat);

	/** Update long-button shapes before drawing dialog commands. */
	void longButtons_onPreRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots);
	/** Restore the save/load frame after long-button rendering. */
	void longButtons_onPostRender(ZmbFeature *feature);
	/** Apply an okay/cancel or load/new command. */
	void longButtons_onButtonAction(ZmbFeature *feature, uint32 bsIdx, ButtonState &bs);
	/** Handle pointer input for okay/cancel and load/new commands. */
	ZmbEventHandleResult longButtons_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos);
	/** Handle keyboard input for okay/cancel and load/new commands. */
	ZmbEventHandleResult longButtons_onKeyDown(ZmbFeature *feature, const Common::KeyState &kbd, bool kbdRepeat);

private:
	/** Keep the selected save row within the visible list. */
	void clampLoadSelection();
	/** Remove ASCII spaces exposed at the start of the editable save name. */
	void removeLeadingSaveInputSpaces();
	/** Mark the dialog frame and save rows for redraw. */
	void markDialogFrameDirty();
	/** Return the text currently displayed in the save-name field. */
	Common::U32String getDisplayedSaveInputText() const;

	/** Operation exposed by this dialog instance. */
	SaveLoadMode _mode;
	/** Feature that renders the frame and save-entry rows. */
	ZmbFeature *_dialogFrameFeature = nullptr;
	/** Feature that renders the long command buttons. */
	ZmbFeature *_longButtonsFeature = nullptr;

	/** Title rectangle selected for the current release and mode. */
	Common::Rect _titleRect;
	/** Localized title key selected for the current mode. */
	ZoombiniText::Key _titleKey;
	/** Index of the first save entry displayed in the list. */
	int32 _saveEntryBaseIdx = 0;
	/** Index of the currently selected save entry; load modes begin on the first row. */
	int32 _saveEntrySelectedIdx = 0;
	/** Frame at which the last save entry was clicked. */
	uint32 _lastSaveEntryClickedFrame = 0;

	// MapRect & MapSave data
	/** Fixed indexes of the save-list and command buttons. */
	enum SaveLoadDialogButtonIdx : uint32 {
		/** Scroll toward older save entries. */
		kSaveLoadDialogButton01_ScrollUp = 0,
		/** Scroll toward newer save entries. */
		kSaveLoadDialogButton02_ScrollDown = 1,
		/** Confirm the selected save or command. */
		kSaveLoadDialogButton03_Okay = 2,
		/** Cancel and close the dialog. */
		kSaveLoadDialogButton04_Cancel = 3,
	};

	/** Number of save rows visible at once. */
	static constexpr int32 SAVESLOTS_PER_SCREEN = 8;

	/** Pressed and enabled state of the save-list scroll buttons. */
	Common::StableMap<uint32, ButtonState> _scrollButtonStateMap;

	/**
	 * Dialog layout rectangles are instance members because Common::Rect requires
	 * runtime construction and ScummVM prohibits global C++ constructors.
	 */
	/** Rectangle containing the save-as caption. */
	const Common::Rect _saveAsCaptionRect = Common::Rect(0x00BB, 0x010F, 0x017C, 0x0122);
	/** Rectangle containing the editable save-name field. */
	const Common::Rect _saveTextBoxRect = Common::Rect(0x00C0, 0x012A, 0x01C8, 0x013C);
	/** Left edge of a save-entry row. */
	static constexpr uint16 _saveEntryLeft = 192;
	/** Right edge of a save-entry row. */
	static constexpr uint16 _saveEntryRight = 405;
	/** Top coordinate of save rows in save mode. */
	static constexpr uint16 _saveEntrySaveModeTop = 94;

	// Text input for SaveDialog
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
	/** Top coordinate of save rows in load mode. */
	static constexpr uint16 _saveEntryLoadModeTop = 110;
	/** Height of one save-entry row. */
	static constexpr uint16 _saveEntryHeight = 20;
};

} // End of namespace Mohawk

#endif
