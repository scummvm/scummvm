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

#include "mutationofjb/widgets/buttonwidget.h"
#include "common/events.h"
#include "graphics/managed_surface.h"

namespace MutationOfJB {

ButtonWidget::ButtonWidget(GuiScreen &gui, const Common::Rect &area, const Graphics::Surface &normalSurface, const Graphics::Surface &pressedSurface) :
	Widget(gui, area),
	_normalSurface(normalSurface),
	_pressedSurface(pressedSurface),
	_callback(nullptr),
	_pressed(false) {}

void ButtonWidget::setCallback(ButtonWidgetCallback *callback) {
	_callback = callback;
}

void ButtonWidget::handleEvent(const Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_LBUTTONDOWN: {
		const int16 x = event.mouse.x;
		const int16 y = event.mouse.y;
		if (_area.contains(x, y)) {
			_pressed = true;
			markDirty();
		}
		break;
	}
	case Common::EVENT_LBUTTONUP: {
		if (_pressed) {
			_pressed = false;
			markDirty();
			if (_callback) {
				_callback->onButtonClicked(this);
			}
		}
		break;
	}
	default:
		break;
	}
}

void ButtonWidget::draw(Graphics::ManagedSurface &surface) {
	surface.blitFrom(_pressed ? _pressedSurface : _normalSurface, Common::Point(_area.left, _area.top));
}

}
