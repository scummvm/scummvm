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

#ifndef HODJNPODJ_GFX_SCROLLBAR_H
#define HODJNPODJ_GFX_SCROLLBAR_H

#include "bagel/hodjnpodj/events.h"
#include "bagel/hodjnpodj/gfx/gfx_surface.h"
#include "bagel/hodjnpodj/gfx/button.h"

namespace Bagel {
namespace HodjNPodj {

class ScrollBar : public UIElement {
private:
	int _minValue = 0;
	int _maxValue = 9;
	int _value = 5;
	bool _isDragging = false;

	void drawSquare(GfxSurface &s, const Common::Rect &r);
	void drawArrow(GfxSurface &s, const Common::Rect &r, bool leftArrow);
	Common::Rect getThumbRect() const;
	int getIndexFromX(int xp) const;

public:
	ScrollBar(const Common::String &name, UIElement *uiParent = nullptr) :
		UIElement(name, uiParent) {}
	ScrollBar(const Common::String &name,
			const Common::Rect &bounds, UIElement *uiParent = nullptr) :
			UIElement(name, uiParent) {
		setBounds(bounds);
	}

	void draw() override;
	bool msgMouseDown(const MouseDownMessage &msg) override;
	bool msgMouseUp(const MouseUpMessage &msg) override;
	bool msgMouseMove(const MouseMoveMessage &msg) override;
	bool msgMouseLeave(const MouseLeaveMessage &msg) override;

	void setScrollRange(int nMinPos, int nMaxPos,
		bool bRedraw = true);
	void setScrollPos(int value);
};

} // namespace HodjNPodj
} // namespace Bagel

#endif
