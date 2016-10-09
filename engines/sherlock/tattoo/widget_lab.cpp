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

#include "sherlock/tattoo/widget_lab.h"
#include "sherlock/tattoo/tattoo_fixed_text.h"
#include "sherlock/tattoo/tattoo_user_interface.h"
#include "sherlock/tattoo/tattoo.h"

namespace Sherlock {

namespace Tattoo {

WidgetLab::WidgetLab(SherlockEngine *vm) : WidgetBase(vm) {
	_labObject = nullptr;
}

void WidgetLab::summonWindow() {
	WidgetBase::summonWindow();
	_labObject = nullptr;
}

void WidgetLab::handleEvents() {
	Events &events = *_vm->_events;
	Scene &scene = *_vm->_scene;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	Common::Point mousePos = events.mousePos();

	WidgetBase::handleEvents();

	// Handle drawing tooltips. If the user is dragging a lab item, display a tooltip for using the item
	// on another. Otherwise, fall back on showing standard tooltips
	if (events.getCursor() == INVALID_CURSOR)
		displayLabNames();
	else
		ui.displayObjectNames();

	// See if they've released a mouse button to do an action
	if (events._released || events._rightReleased) {
		// See if the mouse was released in an exit/arrow zone (ie. the "Exit" button)
		ui._exitZone = -1;
		if (ui._arrowZone != -1 && events._released)
			ui._exitZone = ui._arrowZone;

		// Turn any current tooltip off
		if (ui._arrowZone == -1 || events._rightReleased)
			ui._tooltipWidget.setText("");

		bool noDesc = false;
		if (ui._bgFound != -1) {
			if (ui._bgShape->_description.hasPrefix(" ") || ui._bgShape->_description.empty())
				noDesc = true;
		} else {
			noDesc = true;
		}

		events.setCursor(ARROW);

		if (events._rightReleased) {
			// If the player is dragging an object around, restore it to its previous location and reset the cursor
			if (_labObject) {
				_labObject->toggleHidden();

				// Toggle any other objects (like shadows) tied to this object
				for (int idx = 0; idx < 6; ++idx) {
					if (!_labObject->_use[idx]._target.compareToIgnoreCase("Toggle")) {
						for (int nameNum = 0; nameNum < 4; ++nameNum)
							scene.toggleObject(_labObject->_use[idx]._names[nameNum]);
					}
				}

				events.setCursor(ARROW);
			}

			// Show the command list for this object
			ui._verbsWidget.load(!noDesc);
		} else if (!noDesc) {
			// The player has released on an object, see if they had an object selected
			// that will be used with this new object
			if (_labObject) {
				// See if the dragged object can be used with the new object
				for (int idx = 0; idx < 6; ++idx) {
					// See if the name of the dragged object is in any of the Target
					// fields of the verbs for the new object
					if (!_labObject->_name.compareToIgnoreCase(ui._bgShape->_use[idx]._target.c_str())) {
						// This object can be used, so use it
						ui.checkAction(ui._bgShape->_use[idx], ui._bgFound);
						ui._activeObj = -1;
					}
				}

				// Restore the dragged object to its previous location
				_labObject->toggleHidden();

				// Toggle any other objects (like shadows) tied to this object
				for (int idx = 0; idx < 6; ++idx) {
					if (!_labObject->_use[idx]._target.compareToIgnoreCase("Toggle")) {
						for (int nameNum = 0; nameNum < 4; ++nameNum)
							scene.toggleObject(_labObject->_use[idx]._names[nameNum]);
					}
				}
			} else if (!ui._bgShape->_name.compareToIgnoreCase("Exit")) {
				// Execute the Exit button's script, which will leave the scene
				ui.lookAtObject();
			}
		} else {
			// The player has released the mouse while NOT over an object. If theu were dragging an object
			// around with the mouse, restore it to its previous location and reset the cursor
			if (_labObject) {
				_labObject->toggleHidden();

				// Toggle any other objects (like shadows) tied to this object
				for (int idx = 0; idx < 6; ++idx) {
					if (!_labObject->_use[idx]._target.compareToIgnoreCase("Toggle")) {
						for (int nameNum = 0; nameNum < 4; ++nameNum)
							scene.toggleObject(_labObject->_use[idx]._names[nameNum]);
					}
				}
			}
		}

		_labObject = nullptr;
		ui._tooltipWidget._offsetY = 0;
	} else if (events._pressed && !_labObject) {
		// If the mouse is over an object and the object is not SOLID, then we need to pick this object
		// up so the player can move it around
		if (ui._bgFound != -1) {
			// Check if the object is set as SOLID, you can't pick up Solid items
			if (ui._bgShape->_aType != SOLID && ui._bgShape->_type != NO_SHAPE) {
				// Save a reference to the object about to be dragged
				_labObject = ui._bgShape;

				// Set the mouse cursor to the object
				Graphics::Surface &img = _labObject->_imageFrame->_frame;
				Common::Point cursorOffset = mousePos - _labObject->_position;
				events.setCursor(ARROW, cursorOffset, img);
				ui._tooltipWidget._offsetY = cursorOffset.y;

				// Hide this object until they are done with it (releasing it)
				_labObject->toggleHidden();

				// Toggle any other objects (like shadows) tied to this object
				for (int idx = 0; idx < 6; ++idx) {
					if (!_labObject->_use[idx]._target.compareToIgnoreCase("Toggle")) {
						for (int nameNum = 0; nameNum < 4; ++nameNum)
							scene.toggleObject(_labObject->_use[idx]._names[nameNum]);
					}
				}
			}
		}
	}
}

void WidgetLab::displayLabNames() {
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;

	// See if thay are pointing at a different object and we need to change the tooltip
	if (ui._bgFound != ui._oldBgFound) {
		// See if there is a new object to be displayed
		if (ui._bgFound == -1) {
			ui._tooltipWidget.setText("");
		} else {
			Common::String str = Common::String::format("%s %s %s %s", FIXED(Use), _labObject->_description.c_str(),
				FIXED(With), ui._bgShape->_description.c_str());

			// Make sure that the Object has a name
			if (!ui._bgShape->_description.empty() && !ui._bgShape->_description.hasPrefix(" ")) {
				ui._tooltipWidget.setText(str);
			} else {
				ui._tooltipWidget.setText("");
			}
		}
	}

	ui._oldArrowZone = ui._arrowZone;
}

} // End of namespace Tattoo

} // End of namespace Sherlock
