/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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
 */

#include "engines/myst3/cursor.h"
#include "engines/myst3/menu.h"
#include "engines/myst3/myst3.h"
#include "engines/myst3/variables.h"

#include "common/events.h"
#include "common/savefile.h"

namespace Myst3 {

Menu::Menu(Myst3Engine *vm) :
	_vm(vm) {
}

Menu::~Menu() {
}

void Menu::updateMainMenu(uint16 action) {
	switch (action) {
	case 1: {
			// New game
			int16 choice = 1;

			// If a game is playing, ask if wants to save
			if (_vm->_vars->getMenuSavedAge() != 0) {
				choice = _vm->openDialog(1080);
			}

			if (choice == 0) {
				// Go to save screen
				_vm->_vars->setMenuSaveBack(1);
				_vm->_vars->setMenuSaveAction(6);
				goToNode(300);
			} else if (choice == 1) {
				// New game
				goToNode(98);
			}
		}
		break;
	case 2: {
			// Load game
			int16 choice = 1;

			// If a game is playing, ask if wants to save
			if (_vm->_vars->getMenuSavedAge() != 0) {
				choice = _vm->openDialog(1060);
			}

			if (choice == 0) {
				// Go to save screen
				_vm->_vars->setMenuSaveBack(1);
				_vm->_vars->setMenuSaveAction(3);
				goToNode(300);
			} else if (choice == 1) {
				// Load game screen
				_vm->_vars->setMenuLoadBack(1);
				goToNode(200);
			}
		}
		break;
	case 3:
		// Go to save screen
		_vm->_vars->setMenuSaveBack(1);
		_vm->_vars->setMenuSaveAction(1);
		goToNode(300);
		break;
	case 4:
		// Settings
		_vm->_vars->setMenuOptionsBack(1);
		_vm->runScriptsFromNode(599, 0, 0);
		break;
	case 5: {
			// Asked to quit
			int16 choice = 1;

			// If a game is playing, ask if wants to save
			if (_vm->_vars->getMenuSavedAge() != 0) {
				choice = _vm->openDialog(1070);
			}

			if (choice == 0) {
				// Go to save screen
				_vm->_vars->setMenuSaveBack(1);
				_vm->_vars->setMenuSaveAction(5);
				goToNode(300);
			} else if (choice == 1) {
				// Quit
				_vm->setShouldQuit();
			}
		}
		break;
	default:
		warning("Menu action %d is not implemented", action);
		break;
	}
}

void Menu::goToNode(uint16 node) {
	if (_vm->_vars->getMenuSavedAge() == 0 && _vm->_vars->getLocationRoom() != 901) {
		// Entering menu, save current location
		_vm->_vars->setMenuSavedAge(_vm->_vars->getLocationAge());
		_vm->_vars->setMenuSavedRoom(_vm->_vars->getLocationRoom());
		_vm->_vars->setMenuSavedNode(_vm->_vars->getLocationNode());
	}

	_vm->_vars->setLocationNextAge(9);
	_vm->_vars->setLocationNextRoom(901);
	_vm->goToNode(node, 2);
}

Dialog::Dialog(Myst3Engine *vm, uint id):
	_vm(vm),
	_texture(0),
	_frameToDisplay(0),
	_previousframe(0) {
	const DirectorySubEntry *buttonsDesc = _vm->getFileDescription("DLGB", 1000, 0, DirectorySubEntry::kMetadata);
	const DirectorySubEntry *movieDesc = _vm->getFileDescription("DLOG", id, 0, DirectorySubEntry::kDialogMovie);
	const DirectorySubEntry *countDesc = _vm->getFileDescription("DLGI", id, 0, DirectorySubEntry::kMetadata);

	// Retrieve button count
	_buttonCount = countDesc->getMiscData(0);
	assert(_buttonCount <= 3);

	// Load available buttons
	for (uint i = 0; i < 3; i++) {
		uint32 left = buttonsDesc->getMiscData(i * 4);
		uint32 top = buttonsDesc->getMiscData(i * 4 + 1);
		uint32 width = buttonsDesc->getMiscData(i * 4 + 2);
		uint32 height = buttonsDesc->getMiscData(i * 4 + 3);
		_buttons[i] = Common::Rect(width, height);
		_buttons[i].translate(left, top);
	}

	// Load the movie
	_movieStream = movieDesc->getData();
	_bink.loadStream(_movieStream, Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24));

	const Graphics::Surface *frame = _bink.decodeNextFrame();
	_texture = _vm->_gfx->createTexture(frame);
}

Dialog::~Dialog() {
	_vm->_gfx->freeTexture(_texture);
}

void Dialog::draw() {
	if (_frameToDisplay != _previousframe) {
		_bink.seekToFrame(_frameToDisplay);

		const Graphics::Surface *frame = _bink.decodeNextFrame();
		_texture->update(frame);

		_previousframe = _frameToDisplay;
	}

	Common::Rect textureRect = Common::Rect(_texture->width, _texture->height);
	_vm->_gfx->drawTexturedRect2D(getPosition(), textureRect, _texture);
}

Common::Rect Dialog::getPosition() {
	Common::Rect screenRect = Common::Rect(_texture->width, _texture->height);
	screenRect.translate((Renderer::kOriginalWidth - _texture->width) / 2,
			(Renderer::kOriginalHeight - _texture->height) / 2);
	return screenRect;
}

int16 Dialog::update() {
	// Process events
	Common::Event event;
	while (_vm->_system->getEventManager()->pollEvent(event)) {
		// Check for "Hard" quit"
		if (event.type == Common::EVENT_QUIT) {
			_vm->setShouldQuit();
			return -2;
		} else if (event.type == Common::EVENT_MOUSEMOVE) {
			// Compute local mouse coordinates
			_vm->_cursor->updatePosition(event.relMouse);
			Common::Rect position = getPosition();
			Common::Point localMouse = _vm->_cursor->getPosition();
			localMouse.x -= position.left;
			localMouse.y -= position.top;

			// Display the frame corresponding to the hovered button
			for (uint i = 0; i < _buttonCount; i++) {
				if (_buttons[i].contains(localMouse)) {
					_frameToDisplay = i + 1;
					return -1;
				}
			}

			// No hovered button
			_frameToDisplay = 0;
		} else if (event.type == Common::EVENT_LBUTTONDOWN) {
			return _frameToDisplay - 1;
		} else if (event.type == Common::EVENT_KEYDOWN) {
			switch (event.kbd.keycode) {
			case Common::KEYCODE_ESCAPE:
				return -2;
				break;
			default:
				break;
			}
		}
	}

	return -1;
}

void Menu::loadMenuOpen() {
	_saveLoadFiles = _vm->getSaveFileManager()->listSavefiles("*.m3s");
	_vm->_vars->setMenuSaveLoadCurrentPage(0);
	saveLoadUpdateVars();
}

void Menu::saveLoadUpdateVars() {
	int16 page = _vm->_vars->getMenuSaveLoadCurrentPage();

	// Go back one page if the last element of the last page was removed
	if (page && (7 * page > (int)_saveLoadFiles.size() - 1))
		page--;
	_vm->_vars->setMenuSaveLoadCurrentPage(page);

	// Set up pagination
	bool canGoLeft = (_saveLoadFiles.size() > 7) && page;
	bool canGoRight = (_saveLoadFiles.size() > 7) && (7 * (page + 1) < (int)_saveLoadFiles.size());

	_vm->_vars->setMenuSaveLoadPageLeft(canGoLeft);
	_vm->_vars->setMenuSaveLoadPageRight(canGoRight);

	// Enable items
	uint16 itemsOnPage = _saveLoadFiles.size() % 7;

	if (itemsOnPage == 0 && _saveLoadFiles.size() != 0)
		itemsOnPage = 7;
	if (canGoRight)
		itemsOnPage = 7;

	for (uint i = 0; i < 7; i++)
		_vm->_vars->set(1354 + i, i < itemsOnPage);
}

void Menu::loadMenuSelect(uint16 item) {
	_vm->_vars->setMenuSaveLoadSelectedItem(item);

	// TODO: Refresh miniature
	// TODO: Selecting twice loads item
}

} /* namespace Myst3 */
