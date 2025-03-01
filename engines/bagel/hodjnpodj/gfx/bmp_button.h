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

#ifndef HODJNPODJ_GFX_BMP_BUTTON_H
#define HODJNPODJ_GFX_BMP_BUTTON_H

#include "graphics/managed_surface.h"
#include "bagel/hodjnpodj/events.h"

namespace Bagel {
namespace HodjNPodj {

class BmpButton : public UIElement {
private:
	Graphics::ManagedSurface _base;
	Graphics::ManagedSurface _selected;
	Graphics::ManagedSurface _focused;
	Graphics::ManagedSurface _disabled;

	void loadBitmap(const char *name, Graphics::ManagedSurface &field);

public:
	BmpButton(const Common::String &name, UIElement *parent,
		const Common::Rect &r) :
		UIElement(name, parent) {
		setBounds(r);
	}

	void loadBitmaps(const char *base, const char *selected,
		const char *focused, const char *disabled);
	void clear();

	void draw() override;
};

} // namespace HodjNPodj
} // namespace Bagel

#endif
