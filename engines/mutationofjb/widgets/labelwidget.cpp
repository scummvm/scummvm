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

#include "mutationofjb/widgets/labelwidget.h"

#include "mutationofjb/assets.h"
#include "mutationofjb/game.h"
#include "mutationofjb/gamedata.h"

namespace MutationOfJB {

LabelWidget::LabelWidget(GuiScreen &gui, const Common::Rect &area) :
	Widget(gui, area),
	_backgroundColor(0x00) {}

uint8 LabelWidget::getBackgroundColor() const {
	return _backgroundColor;
}

void LabelWidget::setBackgroundColor(uint8 color) {
	if (_backgroundColor == color)
		return;

	_backgroundColor = color;
	markDirty();
}

const Common::String &LabelWidget::getText() const {
	return _text;
}

void LabelWidget::setText(const Common::String &text) {
	if (_text == text)
		return;

	_text = text;
	markDirty();
}

void LabelWidget::draw(Graphics::ManagedSurface &surface) {
	surface.fillRect(_area, _backgroundColor);
	_gui.getGame().getAssets().getSystemFont().drawString(&surface, _text, _area.left, _area.top, _area.width(), LIGHTGRAY, Graphics::kTextAlignCenter);
}

}
