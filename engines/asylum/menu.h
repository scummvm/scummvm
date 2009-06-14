/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef ASYLUM_MENU_H_
#define ASYLUM_MENU_H_

#include "common/events.h"

#include "asylum/asylum.h"
#include "asylum/resourcepack.h"
#include "asylum/graphics.h"
#include "asylum/text.h"

namespace Asylum {

class Screen;
class Sound;
class Text;

class MainMenu {
public:
	MainMenu(Screen *screen, Sound *sound);
	~MainMenu();

	void handleEvent(Common::Event *event, bool doUpdate);

private:

	enum EyesAnimation {
		kEyesFront       = 0,
		kEyesLeft        = 1,
		kEyesRight       = 2,
		kEyesTop         = 3,
		kEyesBottom      = 4,
		kEyesTopLeft     = 5,
		kEyesTopRight    = 6,
		kEyesBottomLeft  = 7,
		kEyesBottomRight = 8,
		kEyesCrossed     = 9
	};

	Screen *_screen;
	Sound *_sound;
	Common::Event *_ev;

	int _mouseX;
	int _mouseY;
	int _activeIcon;
	int _previousActiveIcon;
	int _curIconFrame;
	int _curMouseCursor;
	int _cursorStep;
	ResourcePack *_resPack;
	ResourcePack *_textPack;
	ResourcePack *_musPack;
	GraphicResource *_cursorResource;
	GraphicResource *_eyeResource;
	GraphicResource *_iconResource;
    Text *_text;

	void updateCursor();

	void update();
}; // end of class MainMenu

} // end of namespace Asylum

#endif
