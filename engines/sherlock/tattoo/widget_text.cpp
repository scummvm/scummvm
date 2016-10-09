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

#include "sherlock/tattoo/widget_text.h"
#include "sherlock/tattoo/tattoo_people.h"
#include "sherlock/tattoo/tattoo_scene.h"
#include "sherlock/tattoo/tattoo_user_interface.h"
#include "sherlock/tattoo/tattoo.h"

namespace Sherlock {

namespace Tattoo {

WidgetText::WidgetText(SherlockEngine *vm) : WidgetBase(vm) {
}

void WidgetText::load(const Common::String &str, int speaker) {
	Screen &screen = *_vm->_screen;
	Talk &talk = *_vm->_talk;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	Common::StringArray lines;

	int width = SHERLOCK_SCREEN_WIDTH / 3;
	int height;

	for (;;) {
		splitLines(str, lines, width - _surface.widestChar() * 2, 100);
		height = (screen.fontHeight() + 1) * lines.size() + 9;

		if ((width - _surface.widestChar() * 2 > height * 3 / 2) || (width - _surface.widestChar() * 2
				> SHERLOCK_SCREEN_WIDTH * 3 / 4))
			break;

		width += (width / 4);
	}

	// See if it's only a single line long
	if (height == _surface.fontHeight() + 10) {
		width = _surface.widestChar() * 2 + 6;

		const char *strP = str.c_str();
		while (*strP && (*strP < talk._opcodes[OP_SWITCH_SPEAKER] || *strP == talk._opcodes[OP_NULL]))
			width += _surface.charWidth(*strP++);
	}

	_bounds = Common::Rect(width, height);

	if (speaker == -1) {
		// No speaker specified, so center window on look position
		_bounds.translate(ui._lookPos.x - width / 2, ui._lookPos.y - height / 2);
	} else {
		// Speaker specified, so center the window above them
		centerWindowOnSpeaker(speaker);
	}

	render(str);
}

void WidgetText::centerWindowOnSpeaker(int speaker) {
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	TattooScene &scene = *(TattooScene *)_vm->_scene;
	Common::Point pt;

	speaker &= 0x7f;
	bool flag = _vm->readFlags(FLAG_PLAYER_IS_HOLMES);
	if (people[HOLMES]._type == CHARACTER && ((speaker == HOLMES && flag) || (speaker == WATSON && !flag))) {
		// Place the window centered above the player
		pt.x = people[HOLMES]._position.x / FIXED_INT_MULTIPLIER - _bounds.width() / 2;

		int scaleVal = scene.getScaleVal(people[HOLMES]._position);
		if (scaleVal == SCALE_THRESHOLD) {
			pt.x += people[HOLMES].frameWidth() / 2;
			pt.y = people[HOLMES]._position.y / FIXED_INT_MULTIPLIER - people[HOLMES].frameHeight()
				- _bounds.height() - _surface.fontHeight();
		} else {
			pt.x += people[HOLMES]._imageFrame->sDrawXSize(scaleVal) / 2;
			pt.y = people[HOLMES]._position.y / FIXED_INT_MULTIPLIER - people[HOLMES]._imageFrame->sDrawYSize(scaleVal)
				- _bounds.height() - _surface.fontHeight();
		}
	} else {
		pt.y = -1;

		// Check each NPC to see if they are the one that is talking
		for (int idx = 1; idx < MAX_CHARACTERS; ++idx) {
			// WORKAROUND: Fixes an original game bug where the positioning for Watson's dialogs
			// during conversations at the Park Lake lake scene is in the incorrect position
			if (speaker == 1 && scene._currentScene == 30)
				continue;

			if (people[idx]._type == CHARACTER) {
				if (!scumm_strnicmp(people[idx]._npcName.c_str(), people._characters[speaker]._portrait, 4)) {
					// Place the window above the player
					pt.x = people[idx]._position.x / FIXED_INT_MULTIPLIER - _bounds.width() / 2;

					int scaleVal = scene.getScaleVal(people[idx]._position);
					if (scaleVal == SCALE_THRESHOLD) {
						pt.x += people[idx].frameWidth() / 2;
						pt.y = people[idx]._position.y / FIXED_INT_MULTIPLIER - people[idx].frameHeight()
							- _bounds.height() - _surface.fontHeight();
					}
					else {
						pt.x += people[idx]._imageFrame->sDrawXSize(scaleVal) / 2;
						pt.y = people[idx]._position.y / FIXED_INT_MULTIPLIER - people[idx]._imageFrame->sDrawYSize(scaleVal)
							- _bounds.height() - _surface.fontHeight();
					}

					if (pt.y < 0)
						pt.y = 0;
					break;
				}
			}
		}

		if (pt.y == -1) {
			for (uint idx = 0; idx < scene._bgShapes.size(); ++idx) {
				Object &obj = scene._bgShapes[idx];

				if (obj._type == ACTIVE_BG_SHAPE && !scumm_strnicmp(obj._name.c_str(), people._characters[speaker]._portrait, 4)) {
					// Place the window centered above the character
					pt.x = obj._position.x - _bounds.width() / 2;
					pt.y = obj._position.y - _bounds.height() - _surface.fontHeight();
					if (pt.y < 0)
						pt.y = 0;
					if (obj._scaleVal == SCALE_THRESHOLD)
						pt.x += obj.frameWidth() / 2;
					else
						pt.x += obj._imageFrame->sDrawXSize(obj._scaleVal) / 2;

					break;
				}
			}
		}

		if (pt.y == -1) {
			pt.x = SHERLOCK_SCREEN_WIDTH / 2 - _bounds.width() / 2;
			pt.y = SHERLOCK_SCREEN_HEIGHT / 2 - _bounds.height() / 2;
		}
	}

	_bounds.moveTo(pt);
}

void WidgetText::render(const Common::String &str) {
	Common::StringArray lines;
	_remainingText = splitLines(str, lines, _bounds.width() - _surface.widestChar() * 2,
		_bounds.height() / (_surface.fontHeight() + 1));

	// Allocate a surface for the window
	_surface.create(_bounds.width(), _bounds.height());
	_surface.clear(TRANSPARENCY);

	// Form the background for the new window
	makeInfoArea();

	int yp = 5;
	for (int lineNum = 0; yp < (_bounds.height() - _surface.fontHeight() / 2); ++lineNum) {
		_surface.writeString(lines[lineNum], Common::Point(_surface.widestChar(), yp), INFO_TOP);
		yp += _surface.fontHeight() + 1;
	}
}

/*----------------------------------------------------------------*/

WidgetMessage::WidgetMessage(SherlockEngine *vm) : WidgetBase(vm) {
	_menuCounter = 0;
}

void WidgetMessage::load(const Common::String &str, int time) {
	Events &events = *_vm->_events;
	Common::Point mousePos = events.mousePos();
	_menuCounter = time;

	// Set up the bounds for the dialog to be a single line
	_bounds = Common::Rect(_surface.stringWidth(str) + _surface.widestChar() * 2 + 6, _surface.fontHeight() + 10);
	_bounds.moveTo(mousePos.x - _bounds.width() / 2, mousePos.y - _bounds.height() / 2);

	// Allocate a surface for the window
	_surface.create(_bounds.width(), _bounds.height());
	_surface.clear(TRANSPARENCY);

	// Form the background for the new window and write the line of text
	makeInfoArea();
	_surface.writeString(str, Common::Point(_surface.widestChar() + 3, 5), INFO_TOP);
}

void WidgetMessage::handleEvents() {
	Events &events = *_vm->_events;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	WidgetBase::handleEvents();

	--_menuCounter;

	// Check if a mouse or keypress has occurred, or the display counter has expired
	if (events._pressed || events._released || events._rightPressed || events._rightReleased ||
			ui._keyState.keycode || !_menuCounter) {
		// Close the window
		banishWindow();

		// Reset cursor and switch back to standard mode
		events.setCursor(ARROW);
		events.clearEvents();
		ui._key = -1;
		ui._oldBgFound = -1;
		ui._menuMode = STD_MODE;
	}
}

} // End of namespace Tattoo

} // End of namespace Sherlock
