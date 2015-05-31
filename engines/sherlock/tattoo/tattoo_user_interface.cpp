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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "sherlock/tattoo/tattoo_user_interface.h"
#include "sherlock/tattoo/tattoo_scene.h"
#include "sherlock/tattoo/tattoo.h"

namespace Sherlock {

namespace Tattoo {

TattooUserInterface::TattooUserInterface(SherlockEngine *vm): UserInterface(vm) {
	_menuBuffer = nullptr;
	_invMenuBuffer = nullptr;
	_tagBuffer = nullptr;
	_invGraphic = nullptr;
	_scrollSize = _scrollSpeed = 0;
}

void TattooUserInterface::initScrollVars() {
	_scrollSize = 0;
	_currentScroll.x = _currentScroll.y = 0;
	_targetScroll.x = _targetScroll.y = 0;
}

void TattooUserInterface::handleInput() {
	// TODO
	_vm->_events->pollEventsAndWait();
}

void TattooUserInterface::drawInterface(int bufferNum) {
	TattooScene &scene = *(TattooScene *)_vm->_scene;
	Screen &screen = *_vm->_screen;
	TattooEngine &vm = *(TattooEngine *)_vm;
	
	if (_invMenuBuffer != nullptr) {
		Common::Rect r = _invMenuBounds;
		r.grow(-3);
		r.translate(-_currentScroll.x, 0);
		_grayAreas.clear();
		_grayAreas.push_back(r);

		drawGrayAreas();
		screen._backBuffer1.transBlitFrom(*_invMenuBuffer, Common::Point(_invMenuBounds.left, _invMenuBounds.top));
	}

	if (_menuBuffer != nullptr) {
		Common::Rect r = _menuBounds;
		r.grow(-3);
		r.translate(-_currentScroll.x, 0);
		_grayAreas.clear();
		_grayAreas.push_back(r);

		drawGrayAreas();
		screen._backBuffer1.transBlitFrom(*_menuBuffer, Common::Point(_invMenuBounds.left, _invMenuBounds.top));
	}

	// See if we need to draw a Text Tag floating with the cursor
	if (_tagBuffer != nullptr)
		screen._backBuffer1.transBlitFrom(*_tagBuffer, Common::Point(_tagBounds.left, _tagBounds.top));

	// See if we need to draw an Inventory Item Graphic floating with the cursor
	if (_invGraphic != nullptr)
		screen._backBuffer1.transBlitFrom(*_invGraphic, Common::Point(_invGraphicBounds.left, _invGraphicBounds.top));

	if (vm._creditsActive)
		vm.drawCredits();

	// Bring the widgets to the screen
	if (scene._mask != nullptr)
		screen._flushScreen = true;

	//if (screen._flushScreen)
	//	screen.blockMove(_currentScroll.x);

}

void TattooUserInterface::doBgAnimRestoreUI() {
	TattooScene &scene = *((TattooScene *)_vm->_scene);
	Screen &screen = *_vm->_screen;

	// If _oldMenuBounds was set, then either a new menu has been opened or the current menu has been closed.
	// Either way, we need to restore the area where the menu was displayed
	if (_oldMenuBounds.width() > 0)
		screen._backBuffer1.blitFrom(screen._backBuffer2, Common::Point(_oldMenuBounds.left, _oldMenuBounds.top),
			_oldMenuBounds);

	if (_oldInvMenuBounds.width() > 0)
		screen._backBuffer1.blitFrom(screen._backBuffer2, Common::Point(_oldInvMenuBounds.left, _oldInvMenuBounds.top),
			_oldInvMenuBounds);

	if (_menuBuffer != nullptr)
		screen._backBuffer1.blitFrom(screen._backBuffer2, Common::Point(_menuBounds.left, _menuBounds.top), _menuBounds);
	if (_invMenuBuffer != nullptr)
		screen._backBuffer1.blitFrom(screen._backBuffer2, Common::Point(_invMenuBounds.left, _invMenuBounds.top), _invMenuBounds);

	// If there is a Text Tag being display, restore the area underneath it
	if (_oldTagBounds.width() > 0)
		screen._backBuffer1.blitFrom(screen._backBuffer2, Common::Point(_oldTagBounds.left, _oldTagBounds.top), 
			_oldTagBounds);

	// If there is an Inventory being shown, restore the graphics underneath it
	if (_oldInvGraphicBounds.width() > 0)
		screen._backBuffer1.blitFrom(screen._backBuffer2, Common::Point(_oldInvGraphicBounds.left, _oldInvGraphicBounds.top), 
			_oldInvGraphicBounds);

	// If a canimation is active, restore the graphics underneath it
	if (scene._activeCAnim._images != nullptr)
		screen.restoreBackground(scene._activeCAnim._oldBounds);

	// If a canimation just ended, remove it's graphics from the backbuffer
	if (scene._activeCAnim._removeBounds.width() > 0)
		screen.restoreBackground(scene._activeCAnim._removeBounds);
}

void TattooUserInterface::doScroll() {
	Screen &screen = *_vm->_screen;
	int oldScroll = _currentScroll.x;

	// If we're already at the target scroll position, nothing needs to be done
	if (_targetScroll.x == _currentScroll.x)
		return;

	screen._flushScreen = true;
	if (_targetScroll.x > _currentScroll.x) {
		_currentScroll.x += _scrollSpeed;
		if (_currentScroll.x > _targetScroll.x)
			_currentScroll.x = _targetScroll.x;
	} else if (_targetScroll.x < _currentScroll.x) {
		_currentScroll.x -= _scrollSpeed;
		if (_currentScroll.x < _targetScroll.x)
			_currentScroll.x = _targetScroll.x;
	}

	if (_menuBuffer != nullptr)
		_menuBounds.translate(_currentScroll.x - oldScroll, 0);
	if (_invMenuBuffer != nullptr)
		_invMenuBounds.translate(_currentScroll.x - oldScroll, 0);
}

void TattooUserInterface::drawGrayAreas() {
	// TODO
}

} // End of namespace Tattoo

} // End of namespace Sherlock
