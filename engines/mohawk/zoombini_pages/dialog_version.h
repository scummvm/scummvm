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

#ifndef MOHAWK_ZOOMBINI_PAGES_DIALOG_VERSION_H
#define MOHAWK_ZOOMBINI_PAGES_DIALOG_VERSION_H

#include "mohawk/zoombini_pages/dialog_base.h"

namespace Mohawk {

/** Original release information, dismissed by any key or mouse button. */
class ZoombiniDialogVersion : public ZoombiniDialog {
public:
	/** Create the release-information panel over the current page. */
	explicit ZoombiniDialogVersion(MohawkEngine_Zoombini *vm);
	/** Capture the page and draw the centered panel into the dialog background. */
	void setBackgroundBitmap() override;
	/** The static panel does not require any resource-backed features. */
	void loadFeatures() override {}
	/** Keep currently playing sounds running while the panel waits for input. */
	bool pausesSounds() const override { return false; }
	/** Consume a key and close the panel without invoking dialog shortcuts. */
	ZmbEventHandleResult onKeyDown(const Common::KeyState &kbd, bool kbdRepeat) override;
	/** Consume a left click anywhere on the screen and close the panel. */
	ZmbEventHandleResult onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) override;
	/** Consume a middle click and close the panel. */
	ZmbEventHandleResult onMButtonDown(const Common::Point &absPos, const Common::Point &relPos) override;
	/** Consume a right click and dismiss the same input-wait panel. */
	ZmbEventHandleResult onRButtonDown(const Common::Point &absPos, const Common::Point &relPos) override;
};

} // End of namespace Mohawk

#endif
