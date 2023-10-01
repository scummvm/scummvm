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
	void update();
	void handleEvent(Common::Event &event);

private:
	OSystem *_system;
	ScummEngine *_vm;
	Graphics::Surface *_macScreen;
	const Graphics::Font *_fonts[3];

	bool _visible = false;

	struct Widget {
		int slot;
		int x;
		int y;
		int width;
		int height;
		int timer;
		bool visible;
		bool enabled;
		bool redraw;
		bool kill;
	};

	Widget _widgets[21];

	void initWidget(int n, int x, int y, int w, int h);
	void clear();
	void show();
	void hide();
	void fill(Common::Rect r);
	void drawButton(int n, byte *text, bool enabled, bool pressed);
	void drawInventoryWidget();
	void drawInventoryArrow(int arrowX, int arrowY, bool highlighted, bool flipped);
	void drawInventoryArrowUp(bool highlight);
	void drawInventoryArrowDown(bool highlight);
	void drawInventoryScrollbar();
	void drawInventoryText(int slot, char *text, bool highlighted);
};

} // End of namespace Scumm

#endif
