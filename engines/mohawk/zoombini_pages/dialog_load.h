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

#ifndef MOHAWK_ZOOMBINI_PAGES_DIALOG_LOAD_H
#define MOHAWK_ZOOMBINI_PAGES_DIALOG_LOAD_H

#include "mohawk/zoombini_pages/dialog_saveload_base.h"

namespace Mohawk {

/** Load dialog with save-entry selection and removal. */
class ZoombiniDialogLoad : public ZoombiniDialogSaveLoad {
public:
	/** Create the load page, optionally replacing Cancel with the New Game command. */
	ZoombiniDialogLoad(MohawkEngine_Zoombini *vm, bool newGameMode);
	/** Load the load frame, scrolling and command SCRBs. */
	void loadFeatures() override;
	/** Draw the title and save entries with the current selection highlighted. */
	void dialogFrame_onPostRender(ZmbFeature *feature) override;
	/** Select a save entry and accept a double-click on the current selection. */
	ZmbEventHandleResult dialogFrame_onLButtonDown(ZmbFeature *feature, const Common::Point &absPos, const Common::Point &relPos) override;
	/** Select or remove a save entry through the configured keyboard shortcuts. */
	ZmbEventHandleResult dialogFrame_onKeyDown(ZmbFeature *feature, const Common::KeyState &kbd, bool kbdRepeat) override;

private:
	/** Load the selected journey and queue its saved page after a successful load. */
	void confirm() override;
	/** Keep the selected save row within the visible list. */
	void clampLoadSelection();

	/** Top coordinate of save rows in the load dialog. */
	static constexpr uint16 _saveEntryLoadModeTop = 110;
	/** Index of the selected save entry; selection begins on the first row. */
	int32 _saveEntrySelectedIdx = 0;
	/** Frame at which the last save entry was clicked. */
	uint32 _lastSaveEntryClickedFrame = 0;
};

} // End of namespace Mohawk

#endif
