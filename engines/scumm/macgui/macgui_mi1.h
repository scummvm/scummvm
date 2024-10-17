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

#ifndef SCUMM_MACGUI_MACGUI_MI1_H
#define SCUMM_MACGUI_MACGUI_MI1_H

#include "common/events.h"
#include "common/rect.h"
#include "common/str.h"

namespace Scumm {

class MacGuiImpl;

class MacMI1Gui : public MacGuiImpl {
public:
	MacMI1Gui(ScummEngine *vm, const Common::Path &resourceFile);
	~MacMI1Gui() {}

	const Common::String name() const { return "game"; }
	int getNumColors() const override { return 256; }

	bool handleEvent(Common::Event event);

	const Graphics::Font *getFontByScummId(int32 id);

	void setupCursor(int &width, int &height, int &hotspotX, int &hotspotY, int &animate);

	void resetAfterLoad();
	void update(int delta) {}

protected:
	bool getFontParams(FontId fontId, int &id, int &size, int &slant) const;

	bool handleMenu(int id, Common::String &name);

	void runAboutDialog();
	bool runOpenDialog(int &saveSlotToHandle);
	bool runSaveDialog(int &saveSlotToHandle, Common::String &name);
	bool runOptionsDialog();

private:
	struct AboutPage {
		const TextLine *text;
		int drawArea;
		uint32 delayMs;
	};

	uint _roughProgress = 0;
	bool _roughWarned = false;

	void runAboutDialogMI1(MacDialogWindow *window);
	void runAboutDialogMI2(MacDialogWindow *window);
	void runAboutDialogIndy4(MacDialogWindow *window);

	void drawShadow(Graphics::Surface *s, int x, int y, int h, Common::Pair<int, int> *drawData);
};

} // End of namespace Scumm
#endif
