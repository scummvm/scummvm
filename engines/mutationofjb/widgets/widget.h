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

#include <common/scummsys.h>
#include <common/rect.h>

namespace Common {
class Event;
}

namespace Graphics {
class ManagedSurface;
}

namespace MutationOfJB {

class Gui;

class Widget {
public:
	Widget(Gui &gui, const Common::Rect &area) : _gui(gui), _area(area), _id(0), _dirty(true) {}
	virtual ~Widget() {}

	int getId() const;
	void setId(int id);

	bool isDirty() const;
	void markDirty();
	void update(Graphics::ManagedSurface &);

	virtual void handleEvent(const Common::Event &) {}
protected:
	virtual void _draw(Graphics::ManagedSurface &) = 0;

	Gui &_gui;
	Common::Rect _area;
	int _id;
	bool _dirty;
};

}

#endif

