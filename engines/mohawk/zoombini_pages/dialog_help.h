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

#ifndef MOHAWK_ZOOMBINI_PAGES_DIALOG_HELP_H
#define MOHAWK_ZOOMBINI_PAGES_DIALOG_HELP_H

#include "common/hashmap.h"

#include "mohawk/zoombini_pages/dialog_base.h"

namespace Mohawk {

/**
 * Page-specific help dialog.
 *
 * The dialog receives the page whose help is being displayed, loads the
 * localized body paragraphs for that page, and exposes previous/next/okay
 * controls.
 *
 * In v2.0US, help voice playback follows the currently visible paragraph.
 */
class ZoombiniDialogHelp : public ZoombiniDialog {
public:
	/** Create help for @p forPage. */
	ZoombiniDialogHelp(MohawkEngine_Zoombini *vm, ZoombiniPageType forPage);
	/** Stop help audio and release help features. */
	~ZoombiniDialogHelp() override;

	/** Load the localized title, body, controls, and voice features. */
	void loadFeatures() override;

protected:
	/** Page whose localized help content is displayed. */
	ZoombiniPageType _forPageType;
	/** Whether the current Help heading shows Picker's version-update request. */
	bool _showPickerHeaderEasterEgg = false;
	/** Whether Picker's update body remains visible until Help navigation. */
	bool _showPickerUpdateHelpBody = false;
	/** Paragraph count in the normal Help STRL used for navigation bounds. */
	uint32 _normalHelpBodyCount = 0;
	/** Localized body paragraphs for the selected page. */
	Common::Array<Common::U32String> _pageHelpBodyStrs;
	/** Index of the currently visible help paragraph. */
	uint32 _pageHelpBodyIdx = 0;
	/** STRL resource containing the current help voice lines. */
	int16 _helpStrlResId = 0;
	/** Queued help voice identifier, or zero when none is queued. */
	uint32 _helpSoundQueue = 0;

	/** Adjust help shapes and localized text before the feature is drawn. */
	void helpDialog_onPreRenderShape(ZmbFeature *feature, ZmbHotspotGroup *hsGroup, Common::Array<ZmbHotspot> &hotspots);
	/** Restore the dialog surface after help content is rendered. */
	void helpDialog_onPostRender(ZmbFeature *feature);
	/** React to completion of a help button animation. */
	void helpDialog_onPostAnimation(ZmbFeature *feature, uint32 bsIdx, ButtonState &bs);
	/** Return the text rectangle for a help button label. */
	Common::Rect helpDialog_getButtonTextRect(ZmbFeature *feature, uint32 bsIdx, ButtonState &bs, const Common::Rect &buttonRect);
	/** Return the title rectangle for the current release layout. */
	const Common::Rect &helpDialog_getTitleRect() const;
	/** Return the heading rectangle for the current release layout. */
	const Common::Rect &helpDialog_getHeadRect() const;
	/** Return the body rectangle for the current release layout. */
	const Common::Rect &helpDialog_getBodyRect() const;
	/** Handle pointer navigation within the help dialog. */
	ZmbEventHandleResult helpDialog_onMouseLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos);
	/** Return the clickable rectangle for a help button. */
	Common::Rect helpDialog_getButtonClickRect(ZmbFeature *feature, uint32 bsIdx, ButtonState &bs, const Common::Rect &buttonRect);
	/** Handle keyboard navigation within the help dialog. */
	ZmbEventHandleResult helpDialog_onKeyDown(ZmbFeature *feature, const Common::KeyState &kbd, bool kbdRepeat);
	/** Enable or disable previous/next controls for the current paragraph. */
	void updateNavigationButtons();
	/** Stop the currently queued help voice. */
	void stopHelpVoice();
	/** Queue the voice associated with the current help paragraph. */
	void playHelpVoice();
	/** Load the initial Help body, including Picker's pending update body. */
	void loadHelpStrings();
	/** Load the page's normal localized Help STRL resource. */
	void loadRegularHelpStrings();

	/** Base Help STRL resource ID keyed by the page whose Help dialog is open. */
	Common::HashMap<ZoombiniPageType, SystemResourceId> _helpStrlMap;

	// MapRect & MapSave data
	enum HelpDialogButtonIdx : uint32 {
		/** Previous help paragraph. */
		kHelpDialogButton01_Prev = 0,
		/** Next help paragraph. */
		kHelpDialogButton02_Next = 1,
		/** Close the help dialog. */
		kHelpDialogButton03_Okay = 2,
	};

	/** Pressed and enabled state of the previous, next, and okay buttons. */
	Common::StableMap<uint32, ButtonState> _helpDialogButtonStateMap;
	/**
	 * Immutable dialog layout rectangles are instance members because Common::Rect
	 * requires runtime construction and ScummVM prohibits global C++ constructors.
	 */
	/** Standard help-dialog title rectangle. */
	const Common::Rect _helpDialogTitleRect = Common::Rect(0x0104, 0x003E, 0x0174, 0x005B);
	/** TLC help-dialog title rectangle. */
	const Common::Rect _helpDialogTlcTitleRect = Common::Rect(0x0104, 0x0035, 0x0174, 0x0052);
	/** Click rectangles for the three help-dialog buttons. */
	const Common::Rect _helpDialogButtonRects[3] = {
		Common::Rect(0x009A, 0x013F, 0x00F5, 0x0162),
		Common::Rect(0x0108, 0x013F, 0x0165, 0x0162),
		Common::Rect(0x01BB, 0x013F, 0x01E2, 0x0162),
	};
	/** Text rectangles for the localized help button labels. */
	const Common::Rect _helpDialogTextRects[2] = {
		Common::Rect(0x00AD, 0x013F, 0x00F0, 0x0162),
		Common::Rect(0x010F, 0x013F, 0x0152, 0x0162),
	};
	/** Standard help-dialog heading rectangle. */
	const Common::Rect _helpDialogHeadRect = Common::Rect(0x00AA, 0x0063, 0x01DB, 0x0074);
	/** TLC help-dialog heading rectangle. */
	const Common::Rect _helpDialogTlcHeadRect = Common::Rect(0x00AA, 0x005A, 0x01DB, 0x006B);
	/** Standard help-dialog body rectangle. */
	const Common::Rect _helpDialogBodyRect = Common::Rect(0x00AA, 0x0079, 0x01DB, 0x0127);
	/** TLC help-dialog body rectangle. */
	const Common::Rect _helpDialogTlcBodyRect = Common::Rect(0x00AA, 0x0070, 0x01DB, 0x0127);
};

} // End of namespace Mohawk

#endif
