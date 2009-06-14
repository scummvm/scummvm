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

#include "asylum/state.h"

namespace Asylum {

State::State(AsylumEngine *vm) {
	_vm     = vm;
	_resMgr = _vm->getResourceManager();

	_mouseX  = 0;
	_mouseY  = 0;
}

void State::handleEvent(Common::Event *event, bool doUpdate) {
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


///////////////
// MenuState //
///////////////

MenuState::MenuState(AsylumEngine *vm): State(vm) {
	_activeIcon         = -1;
	_previousActiveIcon = -1;
	_curIconFrame       = 0;
	_curMouseCursor     = 0;
	_cursorStep         = 1;

	_resPack = new ResourcePack("res.001");
	_musPack = new ResourcePack("mus.005");

	// Load the graphics palette
	byte *pal = _resPack->getResource(17)->data + 32;
	_vm->getScreen()->setPalette(pal);

	// Copy the background to the back buffer
	GraphicResource *bg = _resMgr->getGraphic(1, 0, 1);
	_vm->getScreen()->copyToBackBuffer(bg->data, 0, 0, bg->width, bg->height);

	_resMgr->loadCursor(1, 2, 0);

	ResourceEntry *musicResource = _musPack->getResource(0);
	_vm->getSound()->playMusic(musicResource->data, musicResource->size);

    // TODO just some proof-of-concept of text drawing
    _text = new Text(_vm);
    _text->loadFont(0x80010010);
}

MenuState::~MenuState() {
	delete _musPack;
	delete _resPack;
    delete _text;
}

void MenuState::update() {
    // TODO just some proof-of-concept of text drawing
    _text->drawChar('C');

	// TODO: Just some proof-of concept to change icons here for now
	if (_mouseY >= 20 && _mouseY <= 20 + 48) {
		// Top row
		for (int i = 0; i <= 5; i++) {
			int curX = 40 + i * 100;
			if (_mouseX >= curX && _mouseX <= curX + 55) {
				GraphicResource *res = _resMgr->getGraphic(1, i + 4, _curIconFrame);
				_vm->_system->copyRectToScreen(res->data, res->width, curX, 20, res->width, res->height);

				// Cycle icon frame
				// Icon animations have 15 frames, 0-14
				_curIconFrame++;
				if (_curIconFrame == 15)
					_curIconFrame = 0;

				_activeIcon = i;

				// Play creepy voice
				if (!_vm->getSound()->isSfxActive() && _activeIcon != _previousActiveIcon) {
					// TODO: This should be moved to a sound-related class
					ResourceEntry *sfxResource = _resPack->getResource(i + 44);
					_vm->getSound()->playMusic(sfxResource->data, sfxResource->size);

					_previousActiveIcon = _activeIcon;
				}

				break;
			}
		}
	} else if (_mouseY >= 400 && _mouseY <= 400 + 48) {
		// Bottom row
		for (int i = 0; i <= 5; i++) {
			int curX = 40 + i * 100;
			if (_mouseX >= curX && _mouseX <= curX + 55) {
				int iconNum = i + 10;

				// The last 2 icons are swapped
				if (iconNum == 14)
					iconNum = 15;
				else if (iconNum == 15)
					iconNum = 14;

				_activeIcon = i + 6;

				// HACK: the credits icon has less frames (0 - 9). Currently, there's no way to find the number
				// of frames with the current resource manager implementation, so we just hardcode it here
				if (_activeIcon == 10 && _curIconFrame >= 10)
					_curIconFrame = 0;

				GraphicResource *res = _resMgr->getGraphic(1, iconNum, _curIconFrame);
				_vm->_system->copyRectToScreen(res->data, res->width, curX, 400, res->width, res->height);

				// Cycle icon frame
				// Icon animations have 15 frames, 0-14
				_curIconFrame++;
				if (_curIconFrame == 15)
					_curIconFrame = 0;

				// Play creepy voice
				if (!_vm->getSound()->isSfxActive() && _activeIcon != _previousActiveIcon) {
					// TODO: This should be moved to a sound-related class
					ResourceEntry *sfxResource = _resPack->getResource(iconNum + 40);
					_vm->getSound()->playMusic(sfxResource->data, sfxResource->size);

					_previousActiveIcon = _activeIcon;
				}

				break;
			}
		}
	} else {
		// No selection
		_previousActiveIcon = _activeIcon = -1;
	}

	// Eyes animation
	// Get the appropriate eye resource depending on the mouse position
	int eyeResource = kEyesFront;

	if (_mouseX <= 200) {
		if (_mouseY <= 160)
			eyeResource = kEyesTopLeft;
		else if (_mouseY > 160 && _mouseY <= 320)
			eyeResource = kEyesLeft;
		else
			eyeResource = kEyesBottomLeft;
	} else if (_mouseX > 200 && _mouseX <= 400) {
		if (_mouseY <= 160)
			eyeResource = kEyesTop;
		else if (_mouseY > 160 && _mouseY <= 320)
			eyeResource = kEyesFront;
		else
			eyeResource = kEyesBottom;
	} else if (_mouseX > 400) {
		if (_mouseY <= 160)
			eyeResource = kEyesTopRight;
		else if (_mouseY > 160 && _mouseY <= 320)
			eyeResource = kEyesRight;
		else
			eyeResource = kEyesBottomRight;
	}
	// TODO: kEyesCrossed state

	GraphicResource *res = _resMgr->getGraphic(1, 1, eyeResource);
	_vm->getScreen()->copyRectToScreenWithTransparency(res->data, 265, 230, res->width, res->height);

	updateCursor();
}

void MenuState::updateCursor() {
	_curMouseCursor += _cursorStep;
	if (_curMouseCursor == 0)
		_cursorStep = 1;
	if (_curMouseCursor == 6)
		_cursorStep = -1;

	_resMgr->loadCursor(1, 2, _curMouseCursor);
}

} // end of namespace Asylum
