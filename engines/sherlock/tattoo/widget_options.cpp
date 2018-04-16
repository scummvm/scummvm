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

#include "sherlock/tattoo/widget_options.h"
#include "sherlock/tattoo/tattoo.h"
#include "sherlock/tattoo/tattoo_fixed_text.h"
#include "sherlock/tattoo/tattoo_scene.h"
#include "sherlock/tattoo/tattoo_user_interface.h"

namespace Sherlock {

namespace Tattoo {

WidgetOptions::WidgetOptions(SherlockEngine *vm) : WidgetBase(vm) {
	_midiSliderX = _digiSliderX = 0;
	_selector = _oldSelector = -1;
}

void WidgetOptions::load() {
	Events &events = *_vm->_events;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	_centerPos = events.mousePos();

	render();

	summonWindow();
	ui._menuMode = OPTION_MODE;
}

void WidgetOptions::handleEvents() {
	TattooEngine &vm = *(TattooEngine *)_vm;
	Events &events = *_vm->_events;
	Music &music = *_vm->_music;
	Screen &screen = *_vm->_screen;
	Sound &sound = *_vm->_sound;
	Talk &talk = *_vm->_talk;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	Common::Point mousePos = events.mousePos();

	if (talk._talkToAbort) {
		sound.stopSound();
		return;
	}

	// Flag if they started pressing outside the window
	if (events._firstPress && !_bounds.contains(mousePos))
		_outsideMenu = true;

	if (events.kbHit()) {
		ui._keyState = events.getKey();

		// Emulate a mouse release if Enter or Space Bar is pressed
		if (ui._keyState.keycode == Common::KEYCODE_RETURN || ui._keyState.keycode == Common::KEYCODE_SPACE) {
			events._pressed  = events._oldButtons = false;
			events._released = true;
		} else if (ui._keyState.keycode == Common::KEYCODE_ESCAPE) {
			close();
			return;
		} else {
			checkTabbingKeys(11);
		}
	}

	// Check highlighting the various controls
	if (_bounds.contains(mousePos)) {
		_selector = (mousePos.y - _bounds.top) / (_surface.fontHeight() + 7);

		// If one of the sliders has been selected, & the mouse is not pressed, reset the selector to -1
		if ((_selector == 3 || _selector == 6) && !events._pressed)
			_selector = -1;
	} else {
		_selector = -1;
		if (_outsideMenu && (events._released || events._rightReleased)) {
			events.clearEvents();
			close();
			return;
		}
	}

	// If the selected control has changed, redraw the dialog contents
	if (_selector != _oldSelector)
		render(OP_CONTENTS);
	_oldSelector = _selector;

	// Adjust the Volume Sliders (if neccessary) here
	switch (_selector) {
	case 3: {
		// Set Music Volume
		_midiSliderX = mousePos.x - _bounds.left;
		if (_midiSliderX < _surface.widestChar())
			_midiSliderX = _surface.widestChar();
		else
			if (_midiSliderX > _bounds.width() - _surface.widestChar())
				_midiSliderX = _bounds.width() - _surface.widestChar();

		int newVolume = (_midiSliderX - _surface.widestChar()) * 255 / (_bounds.width() - _surface.widestChar() * 2);
		if (newVolume != music._musicVolume) {
			music.setMusicVolume(newVolume);
			vm.saveConfig();
		}

		render(OP_NAMES);
		break;
	}

	case 6: {
		// Set Digitized Volume
		_digiSliderX = mousePos.x - _bounds.left;
		if (_digiSliderX < _surface.widestChar())
			_digiSliderX = _surface.widestChar();
		else if (_digiSliderX > _bounds.width() - _surface.widestChar())
			_digiSliderX = _bounds.width() - _surface.widestChar();

		int newVolume = (_digiSliderX - _surface.widestChar()) * 255 / (_bounds.width() - _surface.widestChar() * 2);
		if (newVolume != sound._soundVolume) {
			sound.setVolume(newVolume);
			vm.saveConfig();
		}

		render(OP_NAMES);
		break;
	}

	default:
		break;
	}

	// Option selected
	if (events._released || events._rightReleased) {
		events.clearEvents();
		_outsideMenu = false;
		int temp = _selector;
		_selector = -1;

		switch (temp) {
		case 0:
			// Load Game
			close();
			ui.loadGame();
			break;

		case 1:
			// Save Game
			close();
			ui.saveGame();
			break;

		case 2:
			// Toggle Music
			music._musicOn = !music._musicOn;
			if (!music._musicOn)
				music.stopMusic();
			else
				music.startSong();

			render(OP_NAMES);
			vm.saveConfig();
			break;

		case 4:
			// Toggle Sound Effects
			sound.stopSound();
			sound._digitized = !sound._digitized;

			render(OP_NAMES);
			vm.saveConfig();
			break;

		case 5:
			// Toggle Voices
			sound._speechOn = !sound._speechOn;

			render(OP_NAMES);
			vm.saveConfig();
			break;

		case 7:
			// Toggle Text Windows
			vm._textWindowsOn = !vm._textWindowsOn;

			render(OP_NAMES);
			vm.saveConfig();
			break;

		case 8: {
			// New Font Style
			int fontNumber = screen.fontNumber() + 1;
			if (fontNumber == 7)
				fontNumber = 0;
			screen.setFont(fontNumber);

			render(OP_ALL);
			vm.saveConfig();
			break;
		}

		case 9:
			// Toggle Transparent Menus
			vm._transparentMenus = !vm._transparentMenus;

			render(OP_NAMES);
			vm.saveConfig();
			break;

		case 10:
			// Quit
			banishWindow();
			ui.doQuitMenu();
			break;

		default:
			break;
		}

		_oldSelector = -1;
	}
}

void WidgetOptions::render(OptionRenderMode mode) {
	TattooEngine &vm = *(TattooEngine *)_vm;
	Music &music = *_vm->_music;
	Sound &sound = *_vm->_sound;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	ImageFile &images = *ui._interfaceImages;
	const char *const OFF_ON[2] = { FIXED(Off), FIXED(On) };

	// Draw the border if necessary
	if (mode == OP_ALL) {
		// Set bounds for the dialog
		Common::String widestString = Common::String::format("%s %s", FIXED(TransparentMenus), FIXED(Off));
		_bounds = Common::Rect(_surface.stringWidth(widestString) + _surface.widestChar() * 2 + 6,
			(_surface.fontHeight() + 7) * 11 + 3);
		_bounds.moveTo(_centerPos.x - _bounds.width() / 2, _centerPos.y - _bounds.height() / 2);

		// Get slider positions
		_midiSliderX = music._musicVolume * (_bounds.width() - _surface.widestChar() * 2) / 255 + _surface.widestChar();
		_digiSliderX = sound._soundVolume * (_bounds.width() - _surface.widestChar() * 2) / 255 + _surface.widestChar();

		// Setup the dialog
		_surface.create(_bounds.width(), _bounds.height());
		_surface.clear(TRANSPARENCY);
		makeInfoArea();

		// Draw the lines separating options in the dialog
		int yp = _surface.fontHeight() + 7;
		for (int idx = 0; idx < 7; ++idx) {
			_surface.SHtransBlitFrom(images[4], Common::Point(0, yp - 1));
			_surface.SHtransBlitFrom(images[5], Common::Point(_surface.width() - images[5]._width, yp - 1));
			_surface.hLine(3, yp, _surface.width() - 4, INFO_TOP);
			_surface.hLine(3, yp + 1, _surface.width() - 4, INFO_MIDDLE);
			_surface.hLine(3, yp + 2, _surface.width() - 4, INFO_BOTTOM);

			yp += _surface.fontHeight() + 7;
			if (idx == 1)
				yp += _surface.fontHeight() + 7;
			else if (idx == 2)
				yp += (_surface.fontHeight() + 7) * 2;
		}
	}

	// Now go through and display all the items that can be highlighted
	for (int idx = 0, yp = 5; idx < 11; ++idx, yp += _surface.fontHeight() + 7) {
		if (mode == OP_ALL || idx == _selector || idx == _oldSelector) {
			if (mode == OP_NAMES)
				_surface.fillRect(Common::Rect(4, yp, _surface.width() - 5, yp + _surface.fontHeight() - 1), TRANSPARENCY);
			byte color = (idx == _selector) ? COMMAND_HIGHLIGHTED : INFO_TOP;
			Common::String str;

			switch (idx) {
			case 0:
				str = FIXED(LoadGame);
				break;

			case 1:
				str = FIXED(SaveGame);
				break;

			case 2:
				str = Common::String::format("%s %s", FIXED(Music), OFF_ON[music._musicOn]);
				break;

			case 3: {
				int num = (_surface.fontHeight() + 4) & 0xfe;
				int sliderY = yp + num / 2 - 8;

				_surface.fillRect(Common::Rect(4, sliderY - (num - 6) / 2, _surface.width() - 5,
					sliderY - (num - 6) / 2 + num - 1), TRANSPARENCY);
				_surface.fillRect(Common::Rect(_surface.widestChar(), sliderY + 2,
					_surface.width() - _surface.widestChar() - 1, sliderY + 3), INFO_MIDDLE);
				drawDialogRect(Common::Rect(_surface.widestChar(), sliderY, _surface.width() - _surface.widestChar(), sliderY + 6));

				_surface.fillRect(Common::Rect(_midiSliderX - 1, sliderY - (num - 6) / 2 + 2,
					_midiSliderX + 1, sliderY - (num - 6) / 2 + num - 3), INFO_MIDDLE);
				drawDialogRect(Common::Rect(_midiSliderX - 3, sliderY - (num - 6) / 2,
					_midiSliderX + 4, sliderY - (num - 6) / 2 + num));

				if (_midiSliderX - 4 > _surface.widestChar())
					_surface.fillRect(Common::Rect(_midiSliderX - 4, sliderY, _midiSliderX - 4, sliderY + 4), INFO_BOTTOM);
				if (_midiSliderX + 4 < _surface.width() - _surface.widestChar())
					_surface.fillRect(Common::Rect(_midiSliderX + 4, sliderY, _midiSliderX + 4, sliderY + 4), INFO_BOTTOM);
				break;
			}

			case 4:
				str = Common::String::format("%s %s", FIXED(SoundEffects), OFF_ON[sound._digitized]);
				break;

			case 5:
				str = Common::String::format("%s %s", FIXED(Voices), OFF_ON[sound._speechOn]);
				break;

			case 6: {
				int num = (_surface.fontHeight() + 4) & 0xfe;
				int sliderY = yp + num / 2 - 8;

				_surface.fillRect(Common::Rect(4, sliderY - (num - 6) / 2, _surface.width() - 5,
					sliderY - (num - 6) / 2 + num - 1), TRANSPARENCY);
				_surface.fillRect(Common::Rect(_surface.widestChar(), sliderY + 2, _surface.width() - _surface.widestChar() - 1,
					sliderY + 3), INFO_MIDDLE);
				drawDialogRect(Common::Rect(_surface.widestChar(), sliderY, _surface.width() - _surface.widestChar(), sliderY + 6));
				_surface.fillRect(Common::Rect(_digiSliderX - 1, sliderY - (num - 6) / 2 + 2, _digiSliderX + 1,
					sliderY - (num - 6) / 2 + num - 3), INFO_MIDDLE);
				drawDialogRect(Common::Rect(_digiSliderX - 3, sliderY - (num - 6) / 2, _digiSliderX + 4,
					sliderY - (num - 6) / 2 + num));
				if (_digiSliderX - 4 > _surface.widestChar())
					_surface.fillRect(Common::Rect(_digiSliderX - 4, sliderY, _digiSliderX - 4, sliderY + 4), INFO_BOTTOM);
				if (_digiSliderX + 4 < _surface.width() - _surface.widestChar())
					_surface.fillRect(Common::Rect(_digiSliderX + 4, sliderY, _digiSliderX + 4, sliderY + 4), INFO_BOTTOM);
				break;
			}

			case 7:
				if (!sound._voices) {
					color = INFO_BOTTOM;
					str = Common::String::format("%s %s", FIXED(TextWindows), FIXED(On));
				} else {
					str = Common::String::format("%s %s", FIXED(TextWindows), OFF_ON[vm._textWindowsOn]);
				}
				break;

			case 8:
				str = FIXED(ChangeFont);
				break;

			case 9:
				str = Common::String::format("%s %s", FIXED(TransparentMenus), OFF_ON[vm._transparentMenus]);
				break;

			case 10:
				str = FIXED(Quit);
				break;

			default:
				break;
			}

			// Unless we're doing one of the Slider Controls, print the text for the line
			if (idx != 3 && idx != 6) {
				int xp = (_surface.width() - _surface.stringWidth(str)) / 2;
				_surface.writeString(str, Common::Point(xp, yp), color);
			}
		}
	}
}

} // End of namespace Tattoo

} // End of namespace Sherlock
