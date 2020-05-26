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
	if (_handleSpecificNumOfTopLayers <= 0) {
		for (Common::Array<UIComponent*>::iterator component = _components.begin(); component != _components.end(); ++component) {
			(*component)->handleMouseMove(mouseX, mouseY);
		}
	} else {
		int countOfTopLayersToHandle = _handleSpecificNumOfTopLayers;
		Common::Array<UIComponent*>::iterator component = _components.end();
		do {
			--component;
			--countOfTopLayersToHandle;
			(*component)->handleMouseMove(mouseX, mouseY);
		} while (component != _components.begin() && countOfTopLayersToHandle != 0);
	}
}

void UIContainer::handleMouseDown(bool alternateButton) {
	if (_handleSpecificNumOfTopLayers <= 0) {
		for (Common::Array<UIComponent*>::iterator component = _components.begin(); component != _components.end(); ++component) {
			(*component)->handleMouseDown(alternateButton);
		}
	} else {
		int countOfTopLayersToHandle = _handleSpecificNumOfTopLayers;
		Common::Array<UIComponent*>::iterator component = _components.end();
		do {
			--component;
			--countOfTopLayersToHandle;
			(*component)->handleMouseDown(alternateButton);
		} while (component != _components.begin() && countOfTopLayersToHandle != 0);
	}
}

void UIContainer::handleMouseUp(bool alternateButton) {
	if (_handleSpecificNumOfTopLayers <= 0) {
		for (Common::Array<UIComponent*>::iterator component = _components.begin(); component != _components.end(); ++component) {
			(*component)->handleMouseUp(alternateButton);
		}
	} else {
		int countOfTopLayersToHandle = _handleSpecificNumOfTopLayers;
		Common::Array<UIComponent*>::iterator component = _components.end();
		do {
			--component;
			--countOfTopLayersToHandle;
			(*component)->handleMouseUp(alternateButton);
		} while (component != _components.begin() && countOfTopLayersToHandle != 0);
	}
}

void UIContainer::handleMouseScroll(int direction) {
	if (_handleSpecificNumOfTopLayers <= 0) {
		for (Common::Array<UIComponent*>::iterator component = _components.begin(); component != _components.end(); ++component) {
			(*component)->handleMouseScroll(direction);
		}
	} else {
		int countOfTopLayersToHandle = _handleSpecificNumOfTopLayers;
		Common::Array<UIComponent*>::iterator component = _components.end();
		do {
			--component;
			--countOfTopLayersToHandle;
			(*component)->handleMouseScroll(direction);
		} while (component != _components.begin() && countOfTopLayersToHandle != 0);
	}
}

void UIContainer::handleKeyUp(const Common::KeyState &kbd) {
	if (_handleSpecificNumOfTopLayers <= 0) {
		for (Common::Array<UIComponent*>::iterator component = _components.begin(); component != _components.end(); ++component) {
			(*component)->handleKeyUp(kbd);
		}
	} else {
		int countOfTopLayersToHandle = _handleSpecificNumOfTopLayers;
		Common::Array<UIComponent*>::iterator component = _components.end();
		do {
			--component;
			--countOfTopLayersToHandle;
			(*component)->handleKeyUp(kbd);
		} while (component != _components.begin() && countOfTopLayersToHandle != 0);
	}
}

void UIContainer::handleKeyDown(const Common::KeyState &kbd) {
	if (_handleSpecificNumOfTopLayers <= 0) {
		for (Common::Array<UIComponent*>::iterator component = _components.begin(); component != _components.end(); ++component) {
			(*component)->handleKeyDown(kbd);
		}
	} else {
		int countOfTopLayersToHandle = _handleSpecificNumOfTopLayers;
		Common::Array<UIComponent*>::iterator component = _components.end();
		do {
			--component;
			--countOfTopLayersToHandle;
			(*component)->handleKeyDown(kbd);
		} while (component != _components.begin() && countOfTopLayersToHandle != 0);

	}
}

void UIContainer::add(UIComponent *component) {
	_components.push_back(component);
}

void UIContainer::clear() {
	_components.clear();
	_handleSpecificNumOfTopLayers = 0;
}

void UIContainer::setHandleSpecificNumOfTopLayers(int count) {
	_handleSpecificNumOfTopLayers = count;
}


} // End of namespace BladeRunner
