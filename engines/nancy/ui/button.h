/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NANCY_UI_BUTTON_H
#define NANCY_UI_BUTTON_H

#include "engines/nancy/renderobject.h"

namespace Nancy {

struct NancyInput;

namespace UI {

class Button : public RenderObject {
public:
	Button(uint16 zOrder, Graphics::ManagedSurface &surface,
			const Common::Rect &clickSrcBounds,
			const Common::Rect &destBounds,
			const Common::Rect &hoverSrcBounds = Common::Rect(),
			const Common::Rect &disabledSrcBounds = Common::Rect());
	virtual ~Button() = default;

	void handleInput(NancyInput &input);

	void setDisabled(bool disabled);

	Graphics::ManagedSurface &surf;
	Common::Rect _clickSrc;
	Common::Rect _hoverSrc;
	Common::Rect _disabledSrc;
	
	bool _isClicked;
	bool _isDisabled;
};

class Toggle : public RenderObject {
public:
	Toggle(uint16 zOrder, Graphics::ManagedSurface &surface, Common::Rect &srcRect, Common::Rect &destRect);
	virtual ~Toggle() = default;

	void handleInput(NancyInput &input);
	void setState(bool toggleState);

	Graphics::ManagedSurface &surf;
	Common::Rect _clickSrc;

	bool _stateChanged;
	bool _toggleState;
};

} // End of namespace UI
} // End of namespace Nancy

#endif // NANCY_UI_BUTTON_H
