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

#include "common/translation.h"
#include "gui/saveload.h"
#include "sherlock/tattoo/widget_files.h"
#include "sherlock/tattoo/tattoo.h"
#include "sherlock/tattoo/tattoo_fixed_text.h"
#include "sherlock/tattoo/tattoo_scene.h"
#include "sherlock/tattoo/tattoo_user_interface.h"

namespace Sherlock {

namespace Tattoo {

#define FILES_LINES_COUNT 5

WidgetFiles::WidgetFiles(SherlockEngine *vm, const Common::String &target) :
		SaveManager(vm, target), WidgetBase(vm), _vm(vm) {
	_fileMode = SAVEMODE_NONE;
	_selector = _oldSelector = -1;
}

void WidgetFiles::show(SaveMode mode) {
	Events &events = *_vm->_events;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	Common::Point mousePos = events.mousePos();

	if (_vm->_showOriginalSavesDialog) {
		// Render and display the file dialog
		_fileMode = mode;
		ui._menuMode = FILES_MODE;
		_selector = _oldSelector = -1;
		_scroll = true;
		createSavegameList();

		// Set up the display area
		_bounds = Common::Rect(SHERLOCK_SCREEN_WIDTH * 2 / 3, (_surface.fontHeight() + 1) *
			(FILES_LINES_COUNT + 1) + 17);
		_bounds.moveTo(mousePos.x - _bounds.width() / 2, mousePos.y - _bounds.height() / 2);

		// Create the surface and render its contents
		_surface.create(_bounds.width(), _bounds.height());
		render(RENDER_ALL);

		summonWindow();
		ui._menuMode = FILES_MODE;
	} else if (mode == SAVEMODE_LOAD) {
		showScummVMRestoreDialog();
	} else {
		showScummVMSaveDialog();
	}
}

void WidgetFiles::showScummVMSaveDialog() {
	GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"), true);

	int slot = dialog->runModalWithCurrentTarget();
	if (slot >= 0) {
		Common::String desc = dialog->getResultString();

		if (desc.empty()) {
			// create our own description for the saved game, the user didn't enter it
			desc = dialog->createDefaultSaveDescription(slot);
		}

		_vm->saveGameState(slot, desc);
	}

	close();
	delete dialog;
}

void WidgetFiles::showScummVMRestoreDialog() {
	GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Restore game:"), _("Restore"), false);
	int slot = dialog->runModalWithCurrentTarget();
	close();
	delete dialog;

	if (slot >= 0) {
		_vm->loadGameState(slot);
	}
}

void WidgetFiles::render(FilesRenderMode mode) {
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	ImageFile &images = *ui._interfaceImages;
	byte color;

	if (mode == RENDER_ALL) {
		_surface.clear(TRANSPARENCY);
		makeInfoArea();

		switch (_fileMode) {
		case SAVEMODE_LOAD:
			_surface.writeString(FIXED(LoadGame),
				Common::Point((_surface.width() - _surface.stringWidth(FIXED(LoadGame))) / 2, 5), INFO_TOP);
			break;

		case SAVEMODE_SAVE:
			_surface.writeString(FIXED(SaveGame),
				Common::Point((_surface.width() - _surface.stringWidth(FIXED(SaveGame))) / 2, 5), INFO_TOP);
			break;

		default:
			break;
		}

		_surface.hLine(3, _surface.fontHeight() + 7, _surface.width() - 4, INFO_TOP);
		_surface.hLine(3, _surface.fontHeight() + 8, _surface.width() - 4, INFO_MIDDLE);
		_surface.hLine(3, _surface.fontHeight() + 9, _surface.width() - 4, INFO_BOTTOM);
		_surface.SHtransBlitFrom(images[4], Common::Point(0, _surface.fontHeight() + 6));
		_surface.SHtransBlitFrom(images[5], Common::Point(_surface.width() - images[5]._width, _surface.fontHeight() + 6));

		int xp = _surface.width() - BUTTON_SIZE - 6;
		_surface.vLine(xp, _surface.fontHeight() + 10, _bounds.height() - 4, INFO_TOP);
		_surface.vLine(xp + 1, _surface.fontHeight() + 10, _bounds.height() - 4, INFO_MIDDLE);
		_surface.vLine(xp + 2, _surface.fontHeight() + 10, _bounds.height() - 4, INFO_BOTTOM);
		_surface.SHtransBlitFrom(images[6], Common::Point(xp - 1, _surface.fontHeight() + 8));
		_surface.SHtransBlitFrom(images[7], Common::Point(xp - 1, _bounds.height() - 4));
	}

	int xp = _surface.stringWidth("00.") + _surface.widestChar() + 5;
	int yp = _surface.fontHeight() + 14;

	for (int idx = _savegameIndex; idx < (_savegameIndex + FILES_LINES_COUNT); ++idx) {
		if (idx == _selector && mode != RENDER_ALL)
			color = COMMAND_HIGHLIGHTED;
		else
			color = INFO_TOP;

		if (mode == RENDER_NAMES_AND_SCROLLBAR)
			_surface.fillRect(Common::Rect(4, yp, _surface.width() - BUTTON_SIZE - 9, yp + _surface.fontHeight()), TRANSPARENCY);

		Common::String numStr = Common::String::format("%d.", idx + 1);
		_surface.writeString(numStr, Common::Point(_surface.widestChar(), yp), color);
		_surface.writeString(_savegames[idx], Common::Point(xp, yp), color);

		yp += _surface.fontHeight() + 1;
	}

	// Draw the Scrollbar if neccessary
	if (mode != RENDER_NAMES)
		drawScrollBar(_savegameIndex, FILES_LINES_COUNT, _savegames.size());
}

void WidgetFiles::handleEvents() {
	Events &events = *_vm->_events;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	Common::Point mousePos = events.mousePos();
	Common::KeyState keyState = ui._keyState;

	// Handle scrollbar events
	ScrollHighlight oldHighlight = ui._scrollHighlight;
	handleScrollbarEvents(_savegameIndex, FILES_LINES_COUNT, _savegames.size());

	int oldScrollIndex = _savegameIndex;
	handleScrolling(_savegameIndex, FILES_LINES_COUNT, _savegames.size());

	// See if the mouse is pointing at any filenames in the window
	if (Common::Rect(_bounds.left, _bounds.top + _surface.fontHeight() + 14,
			_bounds.right - BUTTON_SIZE - 5, _bounds.bottom - 5).contains(mousePos)) {
		_selector = (mousePos.y - _bounds.top - _surface.fontHeight() - 14) / (_surface.fontHeight() + 1) +
			_savegameIndex;
	} else {
		_selector = -1;
	}

	// Check for the Tab key
	if (keyState.keycode == Common::KEYCODE_TAB) {
		// If the mouse is not over any of the filenames, move the mouse so that it points to the first one
		if (_selector == -1) {
			events.warpMouse(Common::Point(_bounds.right - BUTTON_SIZE - 20,
				_bounds.top + _surface.fontHeight() * 2 + 8));
		} else {
			// See if we're doing Tab or Shift Tab
			if (keyState.flags & Common::KBD_SHIFT) {
				// We're doing Shift Tab
				if (_selector == _savegameIndex)
					_selector = _savegameIndex + 4;
				else
					--_selector;
			} else {
				// We're doing Tab
				++_selector;
				if (_selector >= _savegameIndex + 5)
					_selector = _savegameIndex;
			}

			events.warpMouse(Common::Point(mousePos.x, _bounds.top + _surface.fontHeight() * 2
				+ 8 + (_selector - _savegameIndex) * (_surface.fontHeight() + 1)));
		}
	}

	// Only redraw the window if the the scrollbar position has changed
	if (ui._scrollHighlight != oldHighlight || oldScrollIndex != _savegameIndex || _selector != _oldSelector)
		render(RENDER_NAMES_AND_SCROLLBAR);
	_oldSelector = _selector;

	if (events._firstPress && !_bounds.contains(mousePos))
		_outsideMenu = true;

	if (events._released || events._rightReleased || keyState.keycode == Common::KEYCODE_ESCAPE) {
		ui._scrollHighlight = SH_NONE;

		if (_outsideMenu && !_bounds.contains(mousePos)) {
			close();
		} else {
			_outsideMenu = false;

			if (_selector != -1) {
				if (_fileMode == SAVEMODE_LOAD) {
					// We're in Load Mode
					_vm->loadGameState(_selector);
				} else if (_fileMode == SAVEMODE_SAVE) {
					// We're in Save Mode
					if (getFilename())
						_vm->saveGameState(_selector, _savegames[_selector]);
					close();
				}
			}
		}
	}
}

bool WidgetFiles::getFilename() {
	Events &events = *_vm->_events;
	TattooScene &scene = *(TattooScene *)_vm->_scene;
	Screen &screen = *_vm->_screen;
	Talk &talk = *_vm->_talk;
	int index = 0;
	int done = 0;
	bool blinkFlag = false;
	int blinkCountdown = 0;
	int cursorColor = 192;
	byte color, textColor;
	bool insert = true;

	assert(_selector != -1);
	Common::Point pt(_surface.stringWidth("00.") + _surface.widestChar() + 5,
		_surface.fontHeight() + 14 + (_selector - _savegameIndex) * (_surface.fontHeight() + 1));

	Common::String numStr = Common::String::format("%d.", _selector + 1);
	_surface.writeString(numStr, Common::Point(_surface.widestChar(), pt.y), COMMAND_HIGHLIGHTED);

	Common::String filename = _savegames[_selector];

	if (isSlotEmpty(_selector)) {
		index = 0;
		_surface.fillRect(Common::Rect(pt.x, pt.y, _bounds.right - BUTTON_SIZE - 9, pt.y + _surface.fontHeight() - 1), TRANSPARENCY);
		filename = "";
	} else {
		index = filename.size();
		_surface.writeString(filename, pt, COMMAND_HIGHLIGHTED);
		pt.x = _surface.stringWidth("00.") + _surface.stringWidth(filename) + _surface.widestChar() + 5;
	}

	do {
		scene.doBgAnim();

		if (talk._talkToAbort)
			return false;

		char currentChar = (index == (int)filename.size()) ? ' ' : filename[index];
		Common::String charString = Common::String::format("%c", currentChar);
		int width = screen.charWidth(currentChar);

		// Wait for keypress
		while (!events.kbHit()) {
			events.pollEventsAndWait();
			events.setButtonState();

			scene.doBgAnim();

			if (talk._talkToAbort)
				return false;

			if (--blinkCountdown <= 0) {
				blinkCountdown = 3;
				blinkFlag = !blinkFlag;
				if (blinkFlag) {
					textColor = 236;
					color = cursorColor;
				} else {
					textColor = COMMAND_HIGHLIGHTED;
					color = TRANSPARENCY;
				}

				_surface.fillRect(Common::Rect(pt.x, pt.y, pt.x + width, pt.y + _surface.fontHeight()), color);
				if (currentChar != ' ')
					_surface.writeString(charString, pt, textColor);
			}
			if (_vm->shouldQuit())
				return false;
		}

		Common::KeyState keyState = events.getKey();
		if (keyState.keycode == Common::KEYCODE_BACKSPACE && index > 0) {
			pt.x -= _surface.charWidth(filename[index - 1]);
			--index;

			if (insert) {
				filename.deleteChar(index);
			} else {
				filename.setChar(' ', index);
			}

			_surface.fillRect(Common::Rect(pt.x, pt.y, _surface.width() - BUTTON_SIZE - 9, pt.y + _surface.fontHeight() - 1), TRANSPARENCY);
			_surface.writeString(filename.c_str() + index, pt, COMMAND_HIGHLIGHTED);

		} else if ((keyState.keycode == Common::KEYCODE_LEFT && index > 0)
				|| (keyState.keycode == Common::KEYCODE_RIGHT && index < 49 && pt.x < (_bounds.right - BUTTON_SIZE - 20))
				|| (keyState.keycode == Common::KEYCODE_HOME && index > 0)
				|| (keyState.keycode == Common::KEYCODE_END)) {
			_surface.fillRect(Common::Rect(pt.x, pt.y, pt.x + width, pt.y + _surface.fontHeight()), TRANSPARENCY);
			if (currentChar)
				_surface.writeString(charString, pt, COMMAND_HIGHLIGHTED);

			switch (keyState.keycode) {
			case Common::KEYCODE_LEFT:
				pt.x -= _surface.charWidth(filename[index - 1]);
				--index;
				break;

			case Common::KEYCODE_RIGHT:
				pt.x += _surface.charWidth(filename[index]);
				++index;
				break;

			case Common::KEYCODE_HOME:
				pt.x = _surface.stringWidth("00.") + _surface.widestChar() + 5;
				index = 0;
				break;

			case Common::KEYCODE_END:
				pt.x = _surface.stringWidth("00.") + _surface.stringWidth(filename) + _surface.widestChar() + 5;
				index = filename.size();

				while (filename[index - 1] == ' ' && index > 0) {
					pt.x -= _surface.charWidth(filename[index - 1]);
					--index;
				}
				break;

			default:
				break;
			}
		} else if (keyState.keycode == Common::KEYCODE_INSERT) {
			insert = !insert;
			if (insert)
				cursorColor = 192;
			else
				cursorColor = 200;

		} else if (keyState.keycode == Common::KEYCODE_DELETE) {
			filename.deleteChar(index);

			_surface.fillRect(Common::Rect(pt.x, pt.y, _bounds.right - BUTTON_SIZE - 9, pt.y + _surface.fontHeight() - 1), TRANSPARENCY);
			_surface.writeString(filename + index, pt, COMMAND_HIGHLIGHTED);

		} else  if (keyState.keycode == Common::KEYCODE_RETURN) {
			done = 1;

		} else if (keyState.keycode == Common::KEYCODE_ESCAPE) {
			_selector = -1;
			render(RENDER_NAMES_AND_SCROLLBAR);
			done = -1;
		}

		if ((keyState.ascii >= ' ') && (keyState.ascii <= 'z') && (index < 50)) {
			if (pt.x + _surface.charWidth(keyState.ascii) < _surface.w - BUTTON_SIZE - 20) {
				if (insert)
					filename.insertChar(keyState.ascii, index);
				else
					filename.setChar(keyState.ascii, index);

				_surface.fillRect(Common::Rect(pt.x, pt.y, _bounds.width() - BUTTON_SIZE - 9,
					pt.y + _surface.fontHeight() - 1), TRANSPARENCY);
				_surface.writeString(filename.c_str() + index, pt, COMMAND_HIGHLIGHTED);
				pt.x += _surface.charWidth(keyState.ascii);
				++index;
			}
		}
	} while (!done && !_vm->shouldQuit());

	scene.doBgAnim();

	if (talk._talkToAbort)
		return false;

	if (done == 1)
		_savegames[_selector] = filename;

	return done == 1;
}

Common::Rect WidgetFiles::getScrollBarBounds() const {
	Common::Rect scrollRect(BUTTON_SIZE, _bounds.height() - _surface.fontHeight() - 16);
	scrollRect.moveTo(_bounds.width() - BUTTON_SIZE - 3, _surface.fontHeight() + 13);

	return scrollRect;
}

} // End of namespace Tattoo

} // End of namespace Sherlock
