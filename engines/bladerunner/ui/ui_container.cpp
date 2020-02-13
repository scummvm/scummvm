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

#include "bladerunner/ui/ui_container.h"

#include "common/keyboard.h"

namespace BladeRunner {

void UIContainer::draw(Graphics::Surface &surface) {
	for (Common::Array<UIComponent*>::iterator component = _components.begin(); component != _components.end(); ++component) {
		(*component)->draw(surface);
	}
}

void UIContainer::handleMouseMove(int mouseX, int mouseY) {
	for (Common::Array<UIComponent*>::iterator component = _components.begin(); component != _components.end(); ++component) {
		(*component)->handleMouseMove(mouseX, mouseY);
	}
}

void UIContainer::handleMouseDown(bool alternateButton) {
	for (Common::Array<UIComponent*>::iterator component = _components.begin(); component != _components.end(); ++component) {
		(*component)->handleMouseDown(alternateButton);
	}
}

void UIContainer::handleMouseUp(bool alternateButton) {
	for (Common::Array<UIComponent*>::iterator component = _components.begin(); component != _components.end(); ++component) {
		(*component)->handleMouseUp(alternateButton);
	}
}

void UIContainer::handleMouseScroll(int direction) {
	for (Common::Array<UIComponent*>::iterator component = _components.begin(); component != _components.end(); ++component) {
		(*component)->handleMouseScroll(direction);
	}
}

void UIContainer::handleKeyUp(const Common::KeyState &kbd) {
	for (Common::Array<UIComponent*>::iterator component = _components.begin(); component != _components.end(); ++component) {
		(*component)->handleKeyUp(kbd);
	}
}

void UIContainer::handleKeyDown(const Common::KeyState &kbd) {
	for (Common::Array<UIComponent*>::iterator component = _components.begin(); component != _components.end(); ++component) {
		(*component)->handleKeyDown(kbd);
	}
}

void UIContainer::add(UIComponent *component) {
	_components.push_back(component);
}

void UIContainer::clear() {
	_components.clear();
}

} // End of namespace BladeRunner
