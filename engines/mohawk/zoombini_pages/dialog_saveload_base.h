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

#ifndef MOHAWK_ZOOMBINI_PAGES_DIALOG_SAVELOAD_BASE_H
#define MOHAWK_ZOOMBINI_PAGES_DIALOG_SAVELOAD_BASE_H

#include "mohawk/zoombini_pages/dialog_base.h"

namespace Mohawk {

/**
 * Shared frame, save-entry list, scrolling and command buttons for save/load dialogs.
 *
 * Used by @ref Mohawk::ZoombiniDialogSave and @ref Mohawk::ZoombiniDialogLoad.
 * The list displays saved-game names from the current target's @ref Mohawk::ZmbRosterFile.
 * Save shows existing names for reference while editing a save name; Load highlights the entry to load.
 *
 * Derived dialogs have to handle text editing, entry selection and the confirmed operation.
 */
class ZoombiniDialogSaveLoad : public ZoombiniDialog {
public:
	/** Release the shared dialog features. */
	~ZoombiniDialogSaveLoad() override;

	/** Return the first visible save-name row rectangle for drawing and Load-dialog hit testing. */
	Common::Rect getSaveEntryBaseRect();

	/** Draw the title, visible entries and any dialog-specific controls. */
	virtual void dialogFrame_onPostRender(ZmbFeature *feature) = 0;
	/** Leave frame clicks unhandled unless a derived dialog selects entries. */
	virtual ZmbEventHandleResult dialogFrame_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos);
	/** Handle dialog-specific text editing or save-entry selection. */
	virtual ZmbEventHandleResult dialogFrame_onKeyDown(ZmbFeature *feature, const Common::KeyState &kbd, bool kbdRepeat) = 0;

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

protected:
	/** Initialize the title and list layout. */
	ZoombiniDialogSaveLoad(MohawkEngine_Zoombini *vm, ZoombiniPageType pageType, ZoombiniText::Key titleKey, const Common::Rect &titleRect, ZoombiniText::Key yesKey, ZoombiniText::Key noKey, uint16 saveEntryTop);
	/**
	 * Create the frame, scroll buttons and command buttons when the concrete dialog loads its features.
	 * Bind frame drawing and input to the derived dialog, using its supplied system SCRBs.
	 */
	void loadSaveLoadFeatures(int16 frameScrb, int16 scrollButtonsScrb, int16 longButtonsScrb);
	/** Draw the localized title. */
	void drawDialogTitle();
	/**
	 * Draw up to 8 saved-game names starting at @ref Mohawk::ZoombiniDialogSaveLoad::_saveEntryBaseIdx.
	 * Called by the Save and Load frame post-render hooks whenever the frame is redrawn.
	 * Save draws an unselected reference list; Load highlights its selected entry.
	 *
	 * @param textPalette Text color for unselected names.
	 * @param selectedIdx Zero-based index in the full roster, independent of the scroll position.
	 *                    Set to -1 to disable highlighting, as the Save dialog does.
	 */
	void drawSaveEntries(uint32 textPalette, int32 selectedIdx = -1);
	/** Mark the dialog frame and save rows for redraw. */
	void markDialogFrameDirty();
	/** Perform the derived dialog's confirmed save or load operation. */
	virtual void confirm() = 0;

	/** Feature that renders the long command buttons. */
	ZmbFeature *_longButtonsFeature = nullptr;
	/** Zero-based roster index of the first visible save name; scrolling moves this 8-row viewport. */
	int32 _saveEntryBaseIdx = 0;

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

private:
	/** Feature that renders the frame and save-entry rows. */
	ZmbFeature *_dialogFrameFeature = nullptr;
	/** Title rectangle adjusted for the current release. */
	Common::Rect _titleRect;
	/** Localized title key supplied by the derived dialog. */
	ZoombiniText::Key _titleKey;
	/** Top coordinate of the first visible save-entry row. */
	const uint16 _saveEntryTop;
	/** Pressed and enabled state of the save-list scroll buttons. */
	Common::StableMap<uint32, ButtonState> _scrollButtonStateMap;

	/** Left edge of a save-entry row. */
	static constexpr uint16 _saveEntryLeft = 192;
	/** Right edge of a save-entry row. */
	static constexpr uint16 _saveEntryRight = 405;
	/** Height of one save-entry row. */
	static constexpr uint16 _saveEntryHeight = 20;
};

} // End of namespace Mohawk

#endif
