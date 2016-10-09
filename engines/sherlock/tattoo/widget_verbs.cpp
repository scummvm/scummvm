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

#include "sherlock/tattoo/widget_verbs.h"
#include "sherlock/tattoo/tattoo_fixed_text.h"
#include "sherlock/tattoo/tattoo_scene.h"
#include "sherlock/tattoo/tattoo_user_interface.h"
#include "sherlock/tattoo/tattoo_people.h"
#include "sherlock/tattoo/tattoo.h"

namespace Sherlock {

namespace Tattoo {

WidgetVerbs::WidgetVerbs(SherlockEngine *vm) : WidgetBase(vm) {
	_selector = _oldSelector = -1;
	_outsideMenu = false;
}

void WidgetVerbs::load(bool objectsOn) {
	Events &events = *_vm->_events;
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	Talk &talk = *_vm->_talk;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	Common::Point mousePos = events.mousePos();
	bool isWatson = false;

	if (talk._talkToAbort)
		return;

	ui._activeObj = ui._bgFound;
	_outsideMenu = false;
	_verbCommands.clear();
	_selector = _oldSelector = -1;

	// Check if we need to show options for the highlighted object
	if (objectsOn) {
		// Set the verb list accordingly, depending on the target being a
		// person or an object
		if (ui._personFound) {
			TattooPerson &person = people[ui._activeObj - 1000];

			if (!scumm_strnicmp(person._npcName.c_str(), "WATS", 4))
				isWatson = true;


			if (scumm_strnicmp(person._examine.c_str(), "_EXIT", 5))
				_verbCommands.push_back(FIXED(Look));

			_verbCommands.push_back(FIXED(Talk));

			// Add any extra active verbs from the NPC's verb list
			for (int idx = 0; idx < 2; ++idx) {
				if (!person._use[idx]._verb.empty() && !person._use[idx]._verb.hasPrefix(" ") &&
						(person._use[idx]._target.empty() || person._use[idx]._target.hasPrefix(" "))) {
					_verbCommands.push_back(person._use[idx]._verb);
				}
			}
		} else {
			if (!scumm_strnicmp(ui._bgShape->_name.c_str(), "WATS", 4))
				// Looking at Watson
				isWatson = true;

			if (scumm_strnicmp(ui._bgShape->_examine.c_str(), "_EXIT", 5))
				// It's not an exit, so include Look as an option
				_verbCommands.push_back(FIXED(Look));

			if (ui._bgShape->_aType == PERSON)
				_verbCommands.push_back(FIXED(Talk));

			// Add any extra active verbs from the object's verb list
			for (int idx = 0; idx < 6; ++idx) {
				UseType &use = ui._bgShape->_use[idx];
				if (!use._verb.empty() && !use._verb.hasPrefix(" ") && !use._verb.hasPrefix("*") &&
					(use._target.empty() || use._target.hasPrefix("*") || use._target.hasPrefix(" "))) {
					_verbCommands.push_back(use._verb);
				}
			}
		}
	}

	// If clicked on Watson, have Journal as an option
	if (isWatson)
		_verbCommands.push_back(FIXED(Journal));

	// Add the system commands
	_verbCommands.push_back(FIXED(Inventory));
	_verbCommands.push_back(FIXED(Options));

	// Figure out the needed width to show the commands
	int width = 0;
	for (uint idx = 0; idx < _verbCommands.size(); ++idx)
		width = MAX(width, _surface.stringWidth(_verbCommands[idx]));
	width += _surface.widestChar() * 2 + 6;
	int height = (_surface.fontHeight() + 7) * _verbCommands.size() + 3;

	// Set the bounds
	_bounds = Common::Rect(width, height);
	_bounds.moveTo(mousePos.x - _bounds.width() / 2, mousePos.y - _bounds.height() / 2);

	// Render the window on the internal surface
	render();
}

void WidgetVerbs::render() {
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	ImageFile &images = *ui._interfaceImages;

	// Create the drawing surface
	_surface.create(_bounds.width(), _bounds.height());
	_surface.clear(TRANSPARENCY);

	// Draw basic background
	makeInfoArea();

	// Draw the verb commands and the lines separating them
	for (uint idx = 0; idx < _verbCommands.size(); ++idx) {
		_surface.writeString(_verbCommands[idx], Common::Point((_bounds.width() - _surface.stringWidth(_verbCommands[idx])) / 2,
			(_surface.fontHeight() + 7) * idx + 5), INFO_TOP);

		if (idx < (_verbCommands.size() - 1)) {
			_surface.hLine(3, (_surface.fontHeight() + 7) * (idx + 1), _bounds.width() - 4, INFO_TOP);
			_surface.hLine(3, (_surface.fontHeight() + 7) * (idx + 1) + 1, _bounds.width() - 4, INFO_MIDDLE);
			_surface.hLine(3, (_surface.fontHeight() + 7) * (idx + 1) + 2, _bounds.width() - 4, INFO_BOTTOM);

			_surface.SHtransBlitFrom(images[4], Common::Point(0, (_surface.fontHeight() + 7) * (idx + 1) - 1));
			_surface.SHtransBlitFrom(images[5], Common::Point(_bounds.width() - images[5]._width,
				(_surface.fontHeight() + 7) * (idx + 1) - 1));
		}
	}
}

void WidgetVerbs::handleEvents() {
	Events &events = *_vm->_events;
	FixedText &fixedText = *_vm->_fixedText;
	People &people = *_vm->_people;
	TattooScene &scene = *(TattooScene *)_vm->_scene;
	Talk &talk = *_vm->_talk;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	Common::Point mousePos = events.mousePos();

	Common::String strLook = fixedText.getText(kFixedText_Look);
	Common::String strTalk = fixedText.getText(kFixedText_Talk);
	Common::String strJournal = fixedText.getText(kFixedText_Journal);

	checkTabbingKeys(_verbCommands.size());

	// Highlight verb display as necessary
	highlightVerbControls();

	// Flag if the user has started pressing the button with the cursor outsie the menu
	if (events._firstPress && !_bounds.contains(mousePos))
		_outsideMenu = true;

	// See if they released the mouse button
	if (events._released || events._rightReleased) {
		// See if they want to close the menu (they clicked outside of the menu)
		if (!_bounds.contains(mousePos)) {
			if (_outsideMenu) {
				if (events._rightReleased) {
					// Change to the item (if any) that was right-clicked on, and re-draw the verb menu
					ui._bgFound = scene.findBgShape(mousePos);
					ui._personFound = ui._bgFound >= 1000;
					ui._bgShape = ui._personFound || ui._bgFound == -1 ? nullptr : &scene._bgShapes[ui._bgFound];

					bool noDesc = false;
					if (ui._personFound) {
						if (people[ui._bgFound - 1000]._description.empty() || people[ui._bgFound - 1000]._description.hasPrefix(" "))
							noDesc = true;
					} else if (ui._bgFound != -1) {
						if (ui._bgShape->_description.empty() || ui._bgShape->_description.hasPrefix(" "))
							noDesc = true;
					} else {
						noDesc = true;
					}

					// Call the Routine to turn on the Commands for this Object
					load(!noDesc);
				} else {
					// Close the window and clear the events
					banishWindow();
					events.clearEvents();

					// Reset the active UI mode
					ui._menuMode = scene._labTableScene ? LAB_MODE : STD_MODE;
				}
			}
		} else if (_bounds.contains(mousePos) && _selector != -1) {
			// Mouse is within the menu
			// Erase the menu
			banishWindow();
			events.clearEvents();

			// See if they are activating the Look Command
			if (!_verbCommands[_selector].compareToIgnoreCase(strLook)) {
				ui._bgFound = ui._activeObj;
				if (ui._activeObj >= 1000) {
					ui._personFound = true;
				} else {
					ui._personFound = false;
					ui._bgShape = &scene._bgShapes[ui._activeObj];
				}

				ui.lookAtObject();

			} else if (!_verbCommands[_selector].compareToIgnoreCase(strTalk)) {
				// Talk command is being activated
				talk.initTalk(ui._activeObj);
				ui._activeObj = -1;

			} else if (!_verbCommands[_selector].compareToIgnoreCase(strJournal)) {
				ui.doJournal();

				// See if we're in a Lab Table scene
				ui._menuMode = scene._labTableScene ? LAB_MODE : STD_MODE;
			} else if (_selector >= ((int)_verbCommands.size() - 2)) {
				switch (_selector - (int)_verbCommands.size() + 2) {
				case 0:
					// Inventory
					ui.doInventory(2);
					break;

				case 1:
					// Options
					ui.doControls();
					break;

				default:
					ui._menuMode = scene._labTableScene ? LAB_MODE : STD_MODE;
					break;
				}
			} else {
				// If they have selected anything else, process it
				people[HOLMES].gotoStand();

				if (ui._activeObj < 1000) {
					for (int idx = 0; idx < 6; ++idx) {
						if (!_verbCommands[_selector].compareToIgnoreCase(scene._bgShapes[ui._activeObj]._use[idx]._verb)) {
							// See if they are Picking this object up
							if (!scene._bgShapes[ui._activeObj]._use[idx]._target.compareToIgnoreCase("*PICKUP"))
								ui.pickUpObject(ui._activeObj);
							else
								ui.checkAction(scene._bgShapes[ui._activeObj]._use[idx], ui._activeObj);
						}
					}
				} else {
					for (int idx = 0; idx < 2; ++idx) {
						if (!_verbCommands[_selector].compareToIgnoreCase(people[ui._activeObj - 1000]._use[idx]._verb))
							ui.checkAction(people[ui._activeObj - 1000]._use[idx], ui._activeObj);
					}
				}

				ui._activeObj = -1;
				if (ui._menuMode != MESSAGE_MODE) {
					// See if we're in a Lab Table Room
					ui._menuMode = scene._labTableScene ? LAB_MODE : STD_MODE;
				}
			}
		}
	} else if (ui._keyState.keycode == Common::KEYCODE_ESCAPE) {
		// User closing the menu with the ESC key
		banishWindow();
		ui._menuMode = scene._labTableScene ? LAB_MODE : STD_MODE;
	}
}

void WidgetVerbs::highlightVerbControls() {
	Events &events = *_vm->_events;
	Screen &screen = *_vm->_screen;
	Common::Point mousePos = events.mousePos();

	// Get highlighted verb
	_selector = -1;
	Common::Rect bounds = _bounds;
	bounds.grow(-3);
	if (bounds.contains(mousePos))
		_selector = (mousePos.y - bounds.top) / (screen.fontHeight() + 7);

	// See if a new verb is being pointed at
	if (_selector != _oldSelector) {
		// Redraw the verb list
		for (int idx = 0; idx < (int)_verbCommands.size(); ++idx) {
			byte color = (idx == _selector) ? (byte)COMMAND_HIGHLIGHTED : (byte)INFO_TOP;
			_surface.writeString(_verbCommands[idx], Common::Point((_bounds.width() - screen.stringWidth(_verbCommands[idx])) / 2,
				(screen.fontHeight() + 7) * idx + 5), color);
		}

		_oldSelector = _selector;
	}
}

} // End of namespace Tattoo

} // End of namespace Sherlock
