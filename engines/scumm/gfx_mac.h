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

	struct GuiButtonData {
		int x;
		int y;
		int w;
		int h;
	};

	// This list has been arranged so that most of the standard verbs
	// have the same index as their verb number in the game. The rest
	// have been arranged tastefully.
	//
	// I think 101-106 are inventory items.

	const GuiButtonData _buttons[21] = {
		{  67, 292, 348, 18 }, // 100: Sentence line
		{ 137, 312,  68, 18 }, // 1: Open
		{ 137, 332,  68, 18 }, // 2: Close
		{  67, 352,  68, 18 }, // 3: Give
		{ 277, 332,  68, 18 }, // 4: Turn on
		{ 277, 352,  68, 18 }, // 5: Turn off
		{  67, 312,  68, 18 }, // 6: Push
		{  67, 332,  68, 18 }, // 7: Pull
		{ 277, 312,  68, 18 }, // 8: Use
		{ 137, 352,  68, 18 }, // 9: Look at
		{ 207, 312,  68, 18 }, // 10: Walk to
		{ 207, 332,  68, 18 }, // 11: Pick up
		{ 207, 352,  68, 18 }, // 12: What is
		{ 347, 312,  68, 18 }, // 13: Talk
		{ 347, 332,  68, 18 }, // 32: Travel
		{  67, 292, 507, 18 }, // 120: Conversation 1
		{  67, 312, 507, 18 }, // 121: Conversation 2
		{  67, 332, 507, 18 }, // 122: Conversation 3
		{  67, 352, 507, 18 }, // 123: Conversation 4
		{  67, 352, 151, 18 }, // Conversation 5
		{ 423, 352, 151, 18 }  // Conversation 6
	};

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
