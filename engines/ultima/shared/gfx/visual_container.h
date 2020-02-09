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

#ifndef ULTIMA_SHARED_GFX_VISUAL_CONTAINER_H
#define ULTIMA_SHARED_GFX_VISUAL_CONTAINER_H

#include "ultima/shared/gfx/visual_item.h"
#include "ultima/shared/core/message_target.h"

namespace Ultima {
namespace Shared {
namespace Gfx {

class VisualContainer : public VisualItem {
	DECLARE_MESSAGE_MAP;
	bool MouseButtonDownMsg(CMouseButtonDownMsg *msg);
	bool MouseButtonUpMsg(CMouseButtonUpMsg *msg);
	bool MouseMoveMsg(CMouseMoveMsg *msg);
	bool MouseDoubleClickMsg(CMouseDoubleClickMsg *msg);
	bool MouseWheelMsg(CMouseWheelMsg *msg);
	bool MouseDragMsg(CMouseDragMsg *msg);
private:
	VisualItem *_mouseFocusItem;
private:
	/**
	 * Called to handle mouse messagaes on the view
	 */
	VisualItem *handleMouseMsg(CMouseMsg *msg);
public:
	CLASSDEF;
	VisualContainer(TreeItem *parent = nullptr) : VisualItem(parent), _mouseFocusItem(nullptr) {}
	VisualContainer(const Rect &r, TreeItem *parent = nullptr) :
		VisualItem(r, parent), _mouseFocusItem(nullptr) {}
	VisualContainer(const Common::String &name, TreeItem *parent = nullptr) :
		VisualItem(name, parent), _mouseFocusItem(nullptr) {}
	VisualContainer(const Common::String &name, const Rect &r, TreeItem *parent = nullptr) :
		VisualItem(name, r, parent), _mouseFocusItem(nullptr) {}
	~VisualContainer() override {}

	/**
	 * Draws the container by iterating through each child and letting it draw
	 */
	void draw() override;

	/**
	 * Flags the item as being changed, requiring a redraw
	 */
	void setDirty(bool dirty = true) override;

	/**
	 * Returns true if the item is dirty, requiring a redraw
	 */
	bool isDirty() const override;
};

} // End of namespace Gfx
} // End of namespace Shared
} // End of namespace Ultima

#endif
