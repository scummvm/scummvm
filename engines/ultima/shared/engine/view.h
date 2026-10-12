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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ULTIMA_SHARED_ENGINE_VIEW_H
#define ULTIMA_SHARED_ENGINE_VIEW_H

#include "graphics/views/view.h"
#include "ultima/shared/engine/events.h"

namespace Ultima {
namespace Shared {

/**
 * Base class for the Ultima engines' views. Adds handling of
 * the Ultima-specific messages on top of the common View
 */
class View : public Graphics::Views::View {
	ULTIMA_CUSTOM_MESSAGES(Graphics::Views::View)
protected:
	/**
	 * Called for a key pressed with Alt held, which games can give a
	 * meaning to
	 * @returns		True if the key was handled
	 */
	virtual bool msgHotkey(Common::KeyCode key) {
		return false;
	}

public:
	View(const Common::String &name, Graphics::Views::UIElement *uiParent) : Graphics::Views::View(name, uiParent) {
	}
	View(const Common::String &name) : Graphics::Views::View(name) {
	}
	~View() override {
	}

	/**
	 * Returns a surface for drawing the element
	 */
	Graphics::Views::GfxSurface getSurface(bool innerBounds = false) const {
		auto s = Graphics::Views::View::getSurface(innerBounds);
		s.setColor(15);		// EGA white color
		return s;
	}

	bool msgKeypress(const Graphics::Views::KeypressMessage &msg) override;
};

} // namespace Shared
} // namespace Ultima

#endif
