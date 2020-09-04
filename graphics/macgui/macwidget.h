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
#include "graphics/managed_surface.h"

namespace Common {
	struct Event;
}

namespace Graphics {

class ManagedSurface;
class MacWindowManager;

class MacWidget {

public:
	MacWidget(MacWidget *parent, int x, int y, int w, int h, MacWindowManager *wm, bool focusable, uint16 border = 0, uint16 gutter = 0, uint16 shadow = 0, uint fgcolor = 0, uint bgcolor= 0xff);
	virtual ~MacWidget();

	/**
	 * Accessor method for the complete dimensions of the widget.
	 * @return Dimensions of the widget relative to the parent's position.
	 */
	const Common::Rect &getDimensions() { return _dims; }

	/**
	 * Method for indicating whether the widget is active or inactive.
	 * Used by the WM to handle focus on windows, etc.
	 * @param active Desired state of the widget.
	 */
	virtual void setActive(bool active);

	/**
	 * Method for marking the widget for redraw.
	 * @param dirty True if the widget needs to be redrawn.
	 */
	void setDirty(bool dirty) { _contentIsDirty = dirty; }

	virtual bool needsRedraw() { return _contentIsDirty; }

	virtual bool draw(ManagedSurface *g, bool forceRedraw = false);
	virtual bool draw(bool forceRedraw = false);
	virtual void blit(ManagedSurface *g, Common::Rect &dest);
	virtual bool processEvent(Common::Event &event);
	virtual bool hasAllFocus() { return _active; }
	virtual bool isEditable() { return _editable; }

	virtual void setColors(uint32 fg, uint32 bg);

	virtual void setDimensions(const Common::Rect &r) {
		_dims = r;
	}

	Common::Point getAbsolutePos();
	MacWidget *findEventHandler(Common::Event &event, int dx, int dy);

	void removeWidget(MacWidget *child, bool del = true);

	Graphics::ManagedSurface *getSurface() { return _composeSurface; }

protected:
	uint16 _border;
	uint16 _gutter;
	uint16 _shadow;

	uint32 _fgcolor, _bgcolor;

	Graphics::ManagedSurface *_composeSurface;

	bool _contentIsDirty;

public:
	bool _focusable;
	bool _active;
	bool _editable;
	uint _priority;

	Common::Rect _dims;

	MacWindowManager *_wm;
	MacWidget *_parent;
	Common::Array<MacWidget *> _children;
};

} // End of namespace Graphics

#endif
