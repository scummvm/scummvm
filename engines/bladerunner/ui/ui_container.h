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

#ifndef BLADERUNNER_UI_CONTAINER_H
#define BLADERUNNER_UI_CONTAINER_H

#include "bladerunner/ui/ui_component.h"

#include "common/array.h"

namespace BladeRunner {

class UIComponent;

class UIContainer : public UIComponent {
	Common::Array<UIComponent*> _components;

	int _handleSpecificNumOfTopLayers;

public:
	UIContainer(BladeRunnerEngine *vm) : UIComponent(vm) {
		_handleSpecificNumOfTopLayers = 0;
	}

	void draw(Graphics::Surface &surface) override;

	void handleMouseMove(int mouseX, int mouseY) override;
	void handleMouseDown(bool alternateButton) override;
	void handleMouseUp(bool alternateButton) override;
	void handleMouseScroll(int direction) override; // Added by ScummVM team
	void handleKeyUp(const Common::KeyState &kbd) override;
	void handleKeyDown(const Common::KeyState &kbd) override;

	void add(UIComponent *component);
	void clear();

	void setHandleSpecificNumOfTopLayers(int count);
};


} // End of namespace BladeRunner

#endif
