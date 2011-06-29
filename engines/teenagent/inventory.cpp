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

#include "common/memstream.h"
#include "common/ptr.h"
#include "common/textconsole.h"

#include "teenagent/inventory.h"
#include "teenagent/resources.h"
#include "teenagent/objects.h"
#include "teenagent/teenagent.h"
#include "teenagent/scene.h"

namespace TeenAgent {

Inventory::Inventory(TeenAgentEngine *engine) {
	_engine = engine;
	_active = false;

	FilePack varia;
	varia.open("varia.res");

	{
		Common::ScopedPtr<Common::SeekableReadStream> s(varia.getStream(3));
		if (!s)
			error("no inventory background");
		debug(0, "loading inventory background...");
		_background.load(s, Surface::kTypeOns);
	}

	uint32 items_size = varia.getSize(4);
	if (items_size == 0)
		error("invalid inventory items size");
	debug(0, "loading items, size: %u", items_size);
	_items = new byte[items_size];
	varia.read(4, _items, items_size);

	byte offsets = _items[0];
	assert(offsets == 92);
	for (byte i = 0; i < offsets; ++i) {
		_offset[i] = READ_LE_UINT16(_items + i * 2 + 1);
	}
	_offset[92] = items_size;

	Resources *res = Resources::instance();
	for (byte i = 0; i <= 92; ++i) {
		InventoryObject io;
		uint16 obj_addr = res->dseg.get_word(0xc4a4 + i * 2);
		if (obj_addr != 0)
			io.load(res->dseg.ptr(obj_addr));
		_objects.push_back(io);
	}

	_inventory = res->dseg.ptr(0xc48d);

	for (int y = 0; y < 4; ++y)
		for (int x = 0; x < 6; ++x) {
			int i = y * 6 + x;
			_graphics[i]._rect.left = 28 + 45 * x - 1;
			_graphics[i]._rect.top = 23 + 31 * y - 1;
			_graphics[i]._rect.right = _graphics[i]._rect.left + 40;
			_graphics[i]._rect.bottom = _graphics[i]._rect.top + 26;
		}

	varia.close();
	_hoveredObj = _selectedObj = NULL;
}

Inventory::~Inventory() {
	delete[] _items;
}

bool Inventory::has(byte item) const {
	for (int i = 0; i < 24; ++i) {
		if (_inventory[i] == item)
			return true;
	}
	return false;
}

void Inventory::remove(byte item) {
	debug(0, "removing %u from inventory", item);
	int i;
	for (i = 0; i < 24; ++i) {
		if (_inventory[i] == item) {
			break;
		}
	}
	for (; i < 23; ++i) {
		_inventory[i] = _inventory[i + 1];
		_graphics[i].free();
	}
	_inventory[23] = 0;
	_graphics[23].free();
}

void Inventory::clear() {
	debug(0, "clearing inventory");
	for (int i = 0; i < 24; ++i) {
		_inventory[i] = 0;
		_graphics[i].free();
	}
}

void Inventory::reload() {
	for (int i = 0; i < 24; ++i) {
		_graphics[i].free();
		uint item = _inventory[i];
		if (item != 0)
			_graphics[i].load(this, item);
	}
}

void Inventory::add(byte item) {
	if (has(item))
		return;
	debug(0, "adding %u to inventory", item);
	for (int i = 0; i < 24; ++i) {
		if (_inventory[i] == 0) {
			_inventory[i] = item;
			return;
		}
	}
	error("no room for item %u", item);
}

bool Inventory::tryObjectCallback(InventoryObject *obj) {
	byte id = obj->id;
	uint i = 0;
	for (byte *table = Resources::instance()->dseg.ptr(0xBB6F + 3); table[0] != 0 && i < 7; table += 3, ++i) {
		if (table[0] == id) {
			resetSelectedObject();
			activate(false);
			if (_engine->processCallback(READ_LE_UINT16(table + 1)))
				return true;
		}
	}
	return false;
}

bool Inventory::processEvent(const Common::Event &event) {
	Resources *res = Resources::instance();

	switch (event.type) {
	case Common::EVENT_MOUSEMOVE:

		if (!_active) {
			if (event.mouse.y < 5)
				activate(true);
			_mouse = event.mouse;
			return false;
		}

		if (event.mouse.x < 17 || event.mouse.x >= 303 || (event.mouse.y - _mouse.y > 0 && event.mouse.y >= 153)) {
			activate(false);
			_mouse = event.mouse;
			return false;
		}

		_mouse = event.mouse;
		_hoveredObj = NULL;

		for (int i = 0; i < 24; ++i) {
			byte item = _inventory[i];
			if (item == 0)
				continue;

			_graphics[i]._hovered = _graphics[i]._rect.in(_mouse);
			if (_graphics[i]._hovered)
				_hoveredObj = &_objects[item];
		}
		return true;

	case Common::EVENT_LBUTTONDOWN: {
		//check combine
		if (!_active)
			return false;

		if (_hoveredObj == NULL)
			return true;

		debug(0, "lclick on %u:%s", _hoveredObj->id, _hoveredObj->name.c_str());

		if (_selectedObj == NULL) {
			if (tryObjectCallback(_hoveredObj))
				return true;
			//activate(false);
			int w = res->font7.render(NULL, 0, 0, _hoveredObj->description, 0xd1);
			_engine->scene->displayMessage(_hoveredObj->description, 0xd1, Common::Point((320 - w) / 2, 162));
			return true;
		}

		int id1 = _selectedObj->id;
		int id2 = _hoveredObj->id;
		if (id1 == id2)
			return true;

		debug(0, "combine(%u, %u)!", id1, id2);
		byte *table = res->dseg.ptr(0xC335);
		while (table[0] != 0 && table[1] != 0) {
			if (
			    (id1 == table[0] && id2 == table[1]) ||
			    (id2 == table[0] && id1 == table[1])
			) {
				byte new_obj = table[2];
				if (new_obj != 0) {
					remove(id1);
					remove(id2);
					debug(0, "adding object %u", new_obj);
					add(new_obj);
					_engine->playSoundNow(69);
				}
				uint16 msg = READ_LE_UINT16(table + 3);
				_engine->displayMessage(msg);
				activate(false);
				resetSelectedObject();
				return true;
			}
			table += 5;
		}
		_engine->displayMessage(0xc3e2);
		activate(false);
		resetSelectedObject();
		return true;
	}

	case Common::EVENT_RBUTTONDOWN:
		if (!_active)
			return false;

		if (_hoveredObj != NULL) {
			debug(0, "rclick object %u:%s", _hoveredObj->id, _hoveredObj->name.c_str());
			if (_hoveredObj->id != 51 && tryObjectCallback(_hoveredObj)) //do not process callback for banknote on r-click
				return true;
		}

		_selectedObj = _hoveredObj;
		if (_selectedObj)
			debug(0, "selected object %s", _selectedObj->name.c_str());
		return true;

	case Common::EVENT_KEYDOWN:
		if (_active && event.kbd.keycode == Common::KEYCODE_ESCAPE) {
			activate(false);
			return true;
		}
		if (event.kbd.keycode == Common::KEYCODE_RETURN) { //triangle button on psp
			activate(!_active);
			return true;
		}
		return false;

	case Common::EVENT_LBUTTONUP:
	case Common::EVENT_RBUTTONUP:
		return _active;

	default:
		return false;
	}
}


void Inventory::Item::free() {
	_animation.free();
	_surface.free();
}

void Inventory::Item::backgroundEffect(Graphics::Surface *s) {
	uint w = _rect.right - _rect.left, h = _rect.bottom - _rect.top;
	byte *line = (byte *)s->getBasePtr(_rect.left, _rect.top);
	for(uint y = 0; y < h; ++y, line += s->pitch) {
		byte *dst = line;
		for(uint x = 0; x < w; ++x, ++dst) {
			*dst = (*dst == 232)? 214: 224;
		}
	}
}

void Inventory::Item::load(Inventory *inventory, uint item_id) {
	InventoryObject *obj = &inventory->_objects[item_id];
	if (obj->animated) {
		if (_animation.empty()) {
			debug(0, "loading item %d from offset %x", obj->id, inventory->_offset[obj->id - 1]);
			Common::MemoryReadStream s(inventory->_items + inventory->_offset[obj->id - 1], inventory->_offset[obj->id] - inventory->_offset[obj->id - 1]);
			_animation.load(&s, Animation::kTypeInventory);
		}
	} else {
		if (_surface.empty()) {
			debug(0, "loading item %d from offset %x", obj->id, inventory->_offset[obj->id - 1]);
			Common::MemoryReadStream s(inventory->_items + inventory->_offset[obj->id - 1], inventory->_offset[obj->id] - inventory->_offset[obj->id - 1]);
			_surface.load(&s, Surface::kTypeOns);
		}
	}
}

void Inventory::Item::render(Inventory *inventory, uint item_id, Graphics::Surface *dst, int delta) {
	InventoryObject *obj = &inventory->_objects[item_id];
	Resources *res = Resources::instance();

	backgroundEffect(dst);
	_rect.render(dst, _hovered ? 233 : 234);
	load(inventory, item_id);
	if (obj->animated) {
		if (_hovered) {
			Surface *s = _animation.currentFrame(delta);
			if (_animation.currentIndex() == 0)
				s = _animation.currentFrame(1); //force index to be 1 here
			if (s != NULL)
				s->render(dst, _rect.left + 1, _rect.top + 1);
		} else {
			Surface *s = _animation.firstFrame();
			if (s != NULL)
				s->render(dst, _rect.left + 1, _rect.top + 1);
		}
	} else {
		_surface.render(dst, _rect.left + 1, _rect.top + 1);
	}

	Common::String name;
	if (inventory->_selectedObj) {
		name = inventory->_selectedObj->name;
		name += " & ";
	}
	if (inventory->_selectedObj != inventory->_hoveredObj)
		name += obj->name;

	if (_hovered && inventory->_engine->scene->getMessage().empty()) {
		int w = res->font7.render(NULL, 0, 0, name, 0xd1, true);
		res->font7.render(dst, (320 - w) / 2, 180, name, 0xd1, true);
	}
}

void Inventory::render(Graphics::Surface *surface, int delta) {
	if (!_active)
		return;

	_background.render(surface);

	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 6; x++) {
			int idx = x + 6 * y;
			byte item = _inventory[idx];
			if (item == 0)
				continue;

			//debug(0, "%d,%d -> %u", x0, y0, item);
			_graphics[idx].render(this, item, surface, delta);
		}
	}
}

} // End of namespace TeenAgent
