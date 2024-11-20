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

#ifndef SCUMM_MACGUI_MACGUI_H
#define SCUMM_MACGUI_MACGUI_H

#include "common/events.h"
#include "common/str.h"

namespace Graphics {
class Font;
struct Surface;
}

namespace Scumm {

class ScummEngine;
class Actor;
class MacGuiImpl;

class MacGui {
	friend class ScummEngine;

private:
	MacGuiImpl *_impl = nullptr;

public:
	MacGui(ScummEngine *vm, const Common::Path &resourceFile);
	~MacGui();

	int getNumColors() const;

	bool initialize();
	void reset();
	void update(int delta);
	void updateWindowManager();

	void resetAfterLoad();
	bool handleEvent(Common::Event event);

	void setupCursor(int &width, int &height, int &hotspotX, int &hotspotY, int &animate);

	void setPaletteDirty();

	const Graphics::Font *getFontByScummId(int32 id);

	void drawBanner(char *message);
	void undrawBanner();

	bool runQuitDialog();
	bool runRestartDialog();

	// Indiana Jones and the Last Crusade
	bool isVerbGuiActive() const;

	Graphics::Surface *textArea() const;
	void clearTextArea();
	void initTextAreaForActor(Actor *a, byte color);
	void printCharToTextArea(int chr, int x, int y, int color);

	// Loom
	void runDraftsInventory();
};

} // End of namespace Scumm
#endif
