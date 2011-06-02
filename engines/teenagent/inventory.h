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
 */

#ifndef TEENAGENT_INVENTORY_H
#define TEENAGENT_INVENTORY_H

#include "teenagent/surface.h"
#include "teenagent/animation.h"
#include "common/events.h"
#include "common/array.h"
#include "teenagent/objects.h"

namespace TeenAgent {

struct InventoryObject;
class TeenAgentEngine;

class Inventory {
public:
	Inventory(TeenAgentEngine *engine);
	~Inventory();

	void render(Graphics::Surface *surface, int delta);

	void clear();
	void reload();
	void add(byte item);
	bool has(byte item) const;
	void remove(byte item);

	void activate(bool a) { _active = a; }
	bool active() const { return _active; }

	bool processEvent(const Common::Event &event);

	InventoryObject *selectedObject() { return _selectedObj; }
	void resetSelectedObject() { _selectedObj = NULL; }

private:
	TeenAgentEngine *_engine;
	Surface _background;
	byte *_items;
	uint _offset[93];

	Common::Array<InventoryObject> _objects;
	byte *_inventory;

	struct Item {
		Animation _animation;
		Surface _surface;
		Rect _rect;
		bool _hovered;

		Item() : _hovered(false) {}
		void free();
		void load(Inventory *inventory, uint item_id);
		void backgroundEffect(Graphics::Surface *s);
		void render(Inventory *inventory, uint item_id, Graphics::Surface *surface, int delta);
	};

	Item _graphics[24];

	bool _active;
	Common::Point _mouse;

	bool tryObjectCallback(InventoryObject *obj);

	InventoryObject *_hoveredObj;
	InventoryObject *_selectedObj;
};

} // End of namespace TeenAgent

#endif
