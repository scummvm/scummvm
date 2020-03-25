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

#ifndef MUTATIONOFJB_WIDGET_H
#define MUTATIONOFJB_WIDGET_H

#include "common/scummsys.h"
#include "common/rect.h"

namespace Common {
struct Event;
}

namespace Graphics {
class ManagedSurface;
}

namespace MutationOfJB {

class GuiScreen;

class Widget {
public:
	enum {
		DIRTY_NONE = 0,
		DIRTY_ALL = 0xFFFFFFFF
	};

	Widget(GuiScreen &gui, const Common::Rect &area) : _gui(gui), _area(area), _id(0), _visible(true), _enabled(true), _dirtyBits(DIRTY_NONE) {}
	virtual ~Widget() {}

	int getId() const;
	void setId(int id);

	bool isVisible() const;
	void setVisible(bool visible);

	bool isEnabled() const;
	void setEnabled(bool enabled);

	Common::Rect getArea() const;
	void setArea(const Common::Rect &area);

	bool isDirty() const;
	void markDirty(uint32 dirtyBits = DIRTY_ALL);
	void update(Graphics::ManagedSurface &);

	virtual void handleEvent(const Common::Event &) {}
protected:
	virtual void draw(Graphics::ManagedSurface &) = 0;

	GuiScreen &_gui;
	Common::Rect _area;
	int _id;
	bool _visible;
	bool _enabled;
	uint32 _dirtyBits;
};

}

#endif
