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

#ifndef SCUMM_MACGUI_MACGUI_V6_H
#define SCUMM_MACGUI_MACGUI_V6_H

#include "common/events.h"
#include "common/rect.h"
#include "common/str.h"

namespace Scumm {

class MacGuiImpl;

class MacV6Gui : public MacGuiImpl {
private:
	Common::String _gameName;

	Graphics::Surface *_backupScreen;
	byte *_backupPalette;

	int _screenSaveLevel = 0;

public:
	MacV6Gui(ScummEngine *vm, const Common::Path &resourceFile);
	~MacV6Gui();

	bool initialize() override;
	bool readStrings() override;

	const Common::String name() const override { return _gameName; }
	int getNumColors() const override { return 256; }

	// See setMacGuiColors()
	uint32 getBlack() const override { return 255; }
	uint32 getWhite() const override { return 254; }

	bool handleEvent(Common::Event event) override;

	const Graphics::Font *getFontByScummId(int32 id) override;

	void setupCursor(int &width, int &height, int &hotspotX, int &hotspotY, int &animate) override;

	void resetAfterLoad() override;
	void update(int delta) override {}

	void updateThumbnail(MacDialogWindow *window, Common::Rect drawArea, int saveSlot);

protected:
	bool getFontParams(FontId fontId, int &id, int &size, int &slant) const override;

	bool handleMenu(int id, Common::String &name) override;

	void saveScreen() override;
	void restoreScreen() override;

  	void onMenuOpen() override;
	void onMenuClose() override;

	void drawDottedFrame(MacDialogWindow *window, Common::Rect bounds, int x1, int x2);

	MacGuiImpl::MacImageSlider *addSlider(MacDialogWindow *window, int x, int y, int width, int minValue, int maxValue, int primaryMarkings = 4);

	void setVolume(int type, int volume);

	void runAboutDialog() override;
	bool runOpenDialog(int &saveSlotToHandle) override;
	bool runSaveDialog(int &saveSlotToHandle, Common::String &saveName) override;
	bool runOptionsDialog() override;
	bool runQuitDialog() override;
	bool runRestartDialog() override;
};

} // End of namespace Scumm

#endif
