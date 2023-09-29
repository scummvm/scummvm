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
	
	void handleEvent(Common::Event &event);
	void clear();
	void drawButton(int n, bool pressed);

private:
	OSystem *_system;
	ScummEngine *_vm;
	Graphics::Surface *_macScreen;
	const Graphics::Font *_fonts[3];

	struct GuiButtonData {
		int x;
		int y;
		int w;
		int h;
	};

	const GuiButtonData _buttons[20] = {
		{  67, 292, 348, 18 }, // Sentence line
		{  67, 312,  68, 18 }, // Push
		{  67, 332,  68, 18 }, // Pull
		{  67, 352,  68, 18 }, // Give
		{ 137, 312,  68, 18 }, // Open
		{ 137, 332,  68, 18 }, // Close
		{ 137, 352,  68, 18 }, // Look at
		{ 207, 312,  68, 18 }, // Walk to
		{ 207, 332,  68, 18 }, // Pick up
		{ 207, 352,  68, 18 }, // What is
		{ 277, 312,  68, 18 }, // Use
		{ 277, 332,  68, 18 }, // Turn on
		{ 277, 352,  68, 18 }, // Turn off
		{ 347, 312,  68, 18 }, // Talk
		{ 347, 332,  68, 18 }, // Travel
		{  67, 292, 507, 18 }, // Conversation 1
		{  67, 312, 507, 18 }, // Conversation 2
		{  67, 332, 507, 18 }, // Conversation 3
		{  67, 352, 151, 18 }, // Conversation 4
		{ 423, 352, 151, 18 }  // Conversation 5
	};
};

} // End of namespace Scumm

#endif
