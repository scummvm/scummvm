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

#include "sherlock/tattoo/widget_files.h"
#include "sherlock/tattoo/tattoo.h"
#include "sherlock/tattoo/tattoo_fixed_text.h"
#include "sherlock/tattoo/tattoo_scene.h"
#include "sherlock/tattoo/tattoo_user_interface.h"

namespace Sherlock {

namespace Tattoo {

WidgetFiles::WidgetFiles(SherlockEngine *vm) : WidgetBase(vm) {
}

void WidgetFiles::show(SaveMode mode) {
	Events &events = *_vm->_events;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	Common::Point mousePos = events.mousePos();

	_fileMode = mode;

	// Set up the display area
	_bounds = Common::Rect(_surface.stringWidth(FIXED(AreYouSureYou)) + _surface.widestChar() * 2,
		(_surface.fontHeight() + 7) * 4);
	_bounds.moveTo(mousePos.x - _bounds.width() / 2, mousePos.y - _bounds.height() / 2);

	// Create the surface
	_surface.create(_bounds.width(), _bounds.height());
	_surface.fill(TRANSPARENCY);
	makeInfoArea();


	ui._menuMode = FILES_MODE;
	summonWindow();
}

void WidgetFiles::handleEvents() {
	//Events &events = *_vm->_events;

}

void WidgetFiles::close() {
	TattooScene &scene = *(TattooScene *)_vm->_scene;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;

	banishWindow();
	ui._menuMode = scene._labTableScene ? LAB_MODE : STD_MODE;
}

} // End of namespace Tattoo

} // End of namespace Sherlock
