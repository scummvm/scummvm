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

#include "common/rect.h"

namespace Common {
	struct Event;
}

namespace Graphics {

class BaseMacWindow;
class ManagedSurface;

class MacWidget {
	friend class MacEditableText;

public:
	MacWidget(int w, int h, bool focusable);
	virtual ~MacWidget() {}

	const Common::Rect &getDimensions() { return _dims; }
	bool isFocusable() { return _focusable; }
	virtual void setActive(bool active) = 0;
	void setDirty(bool dirty) { _contentIsDirty = dirty; }
	virtual bool draw(ManagedSurface *g, bool forceRedraw = false) = 0;
	virtual bool processEvent(Common::Event &event) = 0;
	virtual bool hasAllFocus() = 0;
	void setParent(BaseMacWindow *parent) { _parent = parent; }

protected:
	bool _focusable;
	bool _contentIsDirty;

	Common::Rect _dims;

public:
	BaseMacWindow *_parent;
};

} // End of namespace Graphics

#endif
