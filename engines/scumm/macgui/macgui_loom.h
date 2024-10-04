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

#ifndef SCUMM_MACGUI_MACGUI_LOOM_H
#define SCUMM_MACGUI_MACGUI_LOOM_H

#include "common/events.h"
#include "common/rect.h"
#include "common/str.h"

namespace Scumm {

class MacGuiImpl;

class MacLoomGui : public MacGuiImpl {
public:
	MacLoomGui(ScummEngine *vm, const Common::Path &resourceFile);
	~MacLoomGui();

	const Common::String name() const { return "Loom"; }

	bool handleEvent(Common::Event event);

	const Graphics::Font *getFontByScummId(int32 id);

	void setupCursor(int &width, int &height, int &hotspotX, int &hotspotY, int &animate);

	void resetAfterLoad();
	void update(int delta);

	void runDraftsInventory();

protected:
	bool getFontParams(FontId fontId, int &id, int &size, int &slant) const;

	bool handleMenu(int id, Common::String &name);

	void runAboutDialog();
	bool runOpenDialog(int &saveSlotToHandle);
	bool runSaveDialog(int &saveSlotToHandle, Common::String &name);
	bool runOptionsDialog();

	void readStrings() override;

private:
	Graphics::Surface *_practiceBox = nullptr;
	Common::Point _practiceBoxPos;
	int _practiceBoxNotes;
};

} // End of namespace Scumm
#endif
