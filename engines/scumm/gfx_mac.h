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

#ifndef SCUMM_GFX_MAC_H
#define SCUMM_GFX_MAC_H

class OSystem;

namespace Graphics {
class Surface;
class Font;
}

namespace Scumm {

class ScummEngine;

class MacIndy3Gui {
public:
	MacIndy3Gui(OSystem *system, ScummEngine *vm);
	~MacIndy3Gui();

	bool isActive();
	void resetAfterLoad();

	void update();

	void handleEvent(Common::Event &event);

private:
	OSystem *_system = nullptr;
	ScummEngine *_vm = nullptr;
	Graphics::Surface *_macScreen = nullptr;
	const Graphics::Font *_fonts[3];

	bool _visible = false;

	struct Widget {
		Common::Rect bounds;
		int verbid = 0;
		int verbslot = -1;
		byte *text = nullptr;
		int timer = 0;
		bool visible = false;
		bool enabled = false;
		bool redraw = false;
		bool kill = false;
	};

	Widget _widgets[29];

	struct InventorySlot {
		Common::Rect bounds;
		byte *name = nullptr;
		int slot = -1;
		int obj = -1;
		int timer = 0;
		bool redraw = false;
	};

	InventorySlot _inventorySlots[6];
	int _inventoryOffset = 0;

	void initWidget(int n, int verbid, int x, int y, int width, int height);
	void resetWidget(Widget *w);
	void initInventorySlot(int n, int x, int y, int width, int height);
	void resetInventorySlot(InventorySlot *s);

	void clear();
	void show();
	void hide();

	void updateTimers();
	void updateButtons(bool &keepGuiAlive, bool &inventoryIsActive);
	void updateInventorySlots();

	void drawButtons();
	void drawInventorySlots();

	void drawWidget(Widget *w);
	void undrawWidget(Widget *w);
	void drawButton(Widget *w);
	void drawInventoryWidget(Widget *w);
	void drawInventoryArrow(int arrowX, int arrowY, bool highlighted, bool flipped);
	void drawInventorySlot(InventorySlot *slot);

	// Primitives
	void fill(Common::Rect r);
	void drawShadowBox(Common::Rect r);
	void drawShadowFrame(Common::Rect r, byte shadowColor, byte fillColor);

	void copyRectToScreen(Common::Rect r);
};

} // End of namespace Scumm

#endif
