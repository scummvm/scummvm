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

#ifndef GRAPHICS_MACGUI_MACWIDGET_H
#define GRAPHICS_MACGUI_MACWIDGET_H

#include "common/array.h"
#include "common/events.h"
#include "common/rect.h"

namespace Common {
	struct Event;
}

namespace Graphics {

class ManagedSurface;

class MacWidget {
	friend class MacEditableText;

public:
	MacWidget(MacWidget *parent, int x, int y, int w, int h, bool focusable);
	virtual ~MacWidget();

	/**
	 * Accessor method for the complete dimensions of the widget.
	 * @return Dimensions of the widget relative to the parent's position.
	 */
	const Common::Rect &getDimensions() { return _dims; }

	bool isFocusable() { return _focusable; }
	virtual void setActive(bool active) = 0;

	/**
	 * Method for marking the widget for redraw.
	 * @param dirty True if the widget needs to be redrawn.
	 */
	void setDirty(bool dirty) { _contentIsDirty = dirty; }

	virtual bool draw(ManagedSurface *g, bool forceRedraw = false) = 0;
	virtual bool processEvent(Common::Event &event) = 0;
	virtual bool hasAllFocus() = 0;

	virtual void setDimensions(const Common::Rect &r) {
		_dims = r;
	}

	Common::Point getAbsolutePos();
	MacWidget *findEventHandler(Common::Event &event, int dx, int dy);

	void removeWidget(MacWidget *child, bool del = true);

protected:
	bool _focusable;
	bool _contentIsDirty;

	Common::Rect _dims;

public:
	MacWidget *_parent;
	Common::Array<MacWidget *> _children;
};

} // End of namespace Graphics

#endif
