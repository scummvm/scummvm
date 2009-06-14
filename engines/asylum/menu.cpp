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

#include "asylum/menu.h"
#include "asylum/resourcepack.h"
#include "asylum/graphics.h"

namespace Asylum {

MainMenu::MainMenu(Screen *screen, Sound *sound): _screen(screen), _sound(sound) {
	_mouseX  = 0;
	_mouseY  = 0;
	_activeIcon         = -1;
	_previousActiveIcon = -1;
	_curIconFrame       = 0;
	_curMouseCursor     = 0;
	_cursorStep         = 1;

	_resPack = new ResourcePack(1);
	_musPack = new ResourcePack("mus.005");

	// Load the graphics palette
	_screen->setPalette(_resPack, 17);

	// Copy the background to the back buffer

	GraphicResource *bgResource = new GraphicResource(_resPack, 0);
	GraphicFrame *bg = bgResource->getFrame(1);
	_screen->copyToBackBuffer((byte *)bg->surface.pixels, 0, 0, bg->surface.w, bg->surface.h);
	delete bgResource;

	// Initialize eye animation
	_eyeResource = new GraphicResource(_resPack, 1);

	// Initialize mouse cursor
	_cursorResource = new GraphicResource(_resPack, 2);
	_screen->setCursor(_cursorResource, 0);
	_screen->showCursor();

	_iconResource = 0;

	_sound->playMusic(_musPack, 0);

    // TODO just some proof-of-concept of text drawing
    _text = new Text(_screen);
    _text->loadFont(_resPack, 0x80010010);
}

MainMenu::~MainMenu() {
    delete _text;
	delete _iconResource;
	delete _eyeResource;
	delete _cursorResource;
	delete _musPack;
	delete _resPack;
}

void MainMenu::handleEvent(Common::Event *event, bool doUpdate) {
	_ev = event;

	// handle common event assignment tasks
	// before bubbling to the subclass' virtual
	// update method
	if (_ev->type == Common::EVENT_MOUSEMOVE) {
		_mouseX = _ev->mouse.x;
		_mouseY = _ev->mouse.y;
	}

	if (doUpdate)
		update();
}

void MainMenu::update() {
	int rowId = 0;

	// Eyes animation
	// Get the appropriate eye resource depending on the mouse position
	int eyeFrameNum = kEyesFront;

	if (_mouseX <= 200) {
		if (_mouseY <= 160)
			eyeFrameNum = kEyesTopLeft;
		else if (_mouseY > 160 && _mouseY <= 320)
			eyeFrameNum = kEyesLeft;
		else
			eyeFrameNum = kEyesBottomLeft;
	} else if (_mouseX > 200 && _mouseX <= 400) {
		if (_mouseY <= 160)
			eyeFrameNum = kEyesTop;
		else if (_mouseY > 160 && _mouseY <= 320)
			eyeFrameNum = kEyesFront;
		else
			eyeFrameNum = kEyesBottom;
	} else if (_mouseX > 400) {
		if (_mouseY <= 160)
			eyeFrameNum = kEyesTopRight;
		else if (_mouseY > 160 && _mouseY <= 320)
			eyeFrameNum = kEyesRight;
		else
			eyeFrameNum = kEyesBottomRight;
	}
	// TODO: kEyesCrossed state

	GraphicFrame *eyeFrame = _eyeResource->getFrame(eyeFrameNum);
	_screen->copyRectToScreenWithTransparency((byte *)eyeFrame->surface.pixels, eyeFrame->x, eyeFrame->y, eyeFrame->surface.w, eyeFrame->surface.h);

	updateCursor();

	if (_mouseY >= 20 && _mouseY <= 20 + 48) {
		rowId = 0; // Top row
	} else if (_mouseY >= 400 && _mouseY <= 400 + 48) {
		rowId = 1; // Bottom row
	} else {
		// No row selected
		_previousActiveIcon = _activeIcon = -1;
		return;
	}

	// Icon animation
	for (int i = 0; i <= 5; i++) {
		int curX = 40 + i * 100;
		if (_mouseX >= curX && _mouseX <= curX + 55) {
			int iconNum = i + 6 * rowId;
			_activeIcon = iconNum;

			// The last 2 icons are swapped
			if (iconNum == 11)
				iconNum = 10;
			else if (iconNum == 10)
				iconNum = 11;

			// Get the current icon animation
			if (!_iconResource || _activeIcon != _previousActiveIcon) {
				delete _iconResource;
				_iconResource = new GraphicResource(_resPack, iconNum + 4);
			}

			GraphicFrame *iconFrame = _iconResource->getFrame(MIN<int>(_iconResource->getFrameCount() - 1, _curIconFrame));
			_screen->copyRectToScreenWithTransparency((byte *)iconFrame->surface.pixels, iconFrame->x, iconFrame->y, iconFrame->surface.w, iconFrame->surface.h);

			// Cycle icon frame
			_curIconFrame++;
			if (_curIconFrame >= _iconResource->getFrameCount())
				_curIconFrame = 0;

			// Show text
            _text->drawResTextCentered(iconFrame->x - 20, iconFrame->y + 50, _text->getResTextWidth(iconNum + 1309), iconNum + 1309);
			
			// Play creepy voice
			if (!_sound->isSfxActive() && _activeIcon != _previousActiveIcon) {
				_sound->playSfx(_resPack, iconNum + 44);
				_previousActiveIcon = _activeIcon;
			}

			break;
		}
	}
}

void MainMenu::updateCursor() {
	_curMouseCursor += _cursorStep;
	if (_curMouseCursor == 0)
		_cursorStep = 1;
	if (_curMouseCursor == _cursorResource->getFrameCount() - 1)
		_cursorStep = -1;

	_screen->setCursor(_cursorResource, _curMouseCursor);
}

} // end of namespace Asylum
