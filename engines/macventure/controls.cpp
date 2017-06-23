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

/*
 * Based on
 * WebVenture (c) 2010, Sean Kasun
 * https://github.com/mrkite/webventure, http://seancode.com/webventure/
 *
 * Used with explicit permission from the author
 */

#include "macventure/gui.h"

namespace MacVenture {
CommandButton::CommandButton() {
	_gui = NULL;
}

CommandButton::CommandButton(ControlData data, Gui *g) {
	_data = data;
	_gui = g;
	_selected = false;
}

void CommandButton::draw(Graphics::ManagedSurface &surface) const {

	uint colorFill = _selected ? kColorBlack : kColorWhite;
	uint colorText = _selected ? kColorWhite : kColorBlack;

	surface.fillRect(_data.bounds, colorFill);
	surface.frameRect(_data.bounds, kColorBlack);

	if (_data.titleLength > 0) {
		const Graphics::Font &font = _gui->getCurrentFont();
		Common::String title(_data.title);
		font.drawString(
			&surface,
			title,
			_data.bounds.left,
			_data.bounds.top,
			_data.bounds.right - _data.bounds.left,
			colorText,
			Graphics::kTextAlignCenter);
	}
}

bool CommandButton::isInsideBounds(const Common::Point point) const {
	return _data.bounds.contains(point);
}

const ControlData &CommandButton::getData() const {
	return _data;
}

void CommandButton::select() {
	_selected = true;
}

void CommandButton::unselect() {
	_selected = false;
}

bool CommandButton::isSelected() {
	return _selected;
}
}
