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
 * $URL$
 * $Id$
 */

#ifndef TEENAGENT_INVENTORY_H
#define TEENAGENT_INVENTORY_H

#include "teenagent/surface.h"
#include "teenagent/animation.h"
#include "common/events.h"
#include "teenagent/objects.h"

namespace TeenAgent {

struct InventoryObject;
class TeenAgentEngine;

class Inventory {
public:
	void init(TeenAgentEngine *engine);
	void render(Graphics::Surface *surface);

	void clear();
	void add(byte item);
	bool has(byte item) const;
	void remove(byte item);

	void activate(bool a) { _active = a; }
	bool active() const { return _active; }

	bool processEvent(const Common::Event &event);

	InventoryObject *selectedObject() { return selected_obj; }
	void resetSelectedObject() { selected_obj = NULL; }

private:
	TeenAgentEngine *_engine;
	Surface background;
	Common::SeekableReadStream *items;
	uint16 offset[92];

	byte *objects;
	byte *inventory;
	struct Item {
		Animation animation;
		Surface surface;
		Rect rect;
		bool hovered;

		Item() : hovered(false) {}
		void free();
		void render(Inventory *inventory, InventoryObject *obj, Graphics::Surface *surface);
	} graphics[24];

	bool _active;
	Common::Point mouse;
	int hovered;

	InventoryObject *hovered_obj, *selected_obj;
};

} // End of namespace TeenAgent

#endif
