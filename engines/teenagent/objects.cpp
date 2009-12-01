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

#include "common/debug.h"
#include "common/stream.h"
#include "teenagent/objects.h"
#include "teenagent/resources.h"

namespace TeenAgent {

void Rect::load(byte * src) {
	_base = src;
	Common::MemoryReadStream ins(src, 8);
	left = ins.readUint16LE();
	top = ins.readUint16LE();
	right = ins.readUint16LE();
	bottom = ins.readUint16LE();
}

void Rect::save() const {
	assert(_base != NULL);
	Common::MemoryWriteStream outs(_base, 8);
	outs.writeUint16LE(left);
	outs.writeUint16LE(top);
	outs.writeUint16LE(right);
	outs.writeUint16LE(bottom);
}

void Rect::render(Graphics::Surface *surface, uint8 color) const {
	surface->hLine(left, bottom, right, color);
	surface->vLine(left, bottom, top, color);
	surface->hLine(left, top, right, color);
	surface->vLine(right, bottom, top, color);
}


void Object::load(byte * src) {
	_base = src;

	id = *src++;

	rect.load(src); 
	src += 8;
	actor_rect.load(src); 
	src += 8;

	actor_orientation = *src++;
	enabled = *src++;
	name = (const char *)src;
	description = parse_description((const char *)src);
}

void Object::save() const {
	assert(_base != NULL);

	rect.save();
	actor_rect.save();
	_base[17] = actor_orientation;
	_base[18] = enabled;
}

void Object::setName(const Common::String &new_name) {
	assert(_base != 0);
	strcpy((char *)(_base + 19), new_name.c_str());
	name = new_name;
}

void Object::dump() const {
	debug(0, "object: %u %u [%u,%u,%u,%u], actor: [%u,%u,%u,%u], orientation: %u, name: %s", id, enabled,
		rect.left, rect.top, rect.right, rect.bottom,
		actor_rect.left, actor_rect.top, actor_rect.right, actor_rect.bottom,
		actor_orientation, name.c_str()
		);
}

Common::String Object::parse_description(const char *name) {
	const char *desc = name + strlen(name) + 1;
	if (*desc == 0)
		return Common::String();

	Common::String result;

	while (*desc != 1 && *desc != 0) {
		Common::String line;
		while (*desc != 1 && *desc != 0) {
			//debug(0, "%02x ", *desc);
			line += *desc++;
		}

		if (line.empty())
			break;

		++desc;
		result += line;
		result += '\n';
	}
	if (!result.empty())
		result.deleteLastChar();
	else
		result = "Cool.";
	return result;
}

void InventoryObject::load(byte *src) {
	_base = src;
	id = *src++;
	animated = *src++;
	name = (const char *)src;
	description = Object::parse_description((const char *)src);
}

void UseHotspot::load(byte *src) {
	Common::MemoryReadStream in(src, 9);
	inventory_id = in.readByte();
	object_id = in.readByte();
	orientation = in.readByte();
	actor_x = in.readUint16LE();
	actor_y = in.readUint16LE();
	callback = in.readUint16LE();
}

void UseHotspot::dump() const {
	debug(0, 
		"hotspot: inv_id: %02x, obj_id: %02x, orientation?: %02x, actor position: (%d,%d), callback: %04x", 
		inventory_id, object_id, orientation, actor_x, actor_y, callback
	);
}

void Walkbox::dump() const {
	debug(0, "walkbox %02x %02x [%d, %d, %d, %d] top: %u, right: %u, bottom: %u, left: %u",
		type, orientation,
		rect.left, rect.right, rect.top, rect.bottom,
		top_side_hint, right_side_hint, bottom_side_hint, left_side_hint);
}

void Walkbox::load(byte *src) {
	_base = src;

	type = *src++;
	orientation = *src++;
	rect.load(src);
	src += 8;
	top_side_hint = *src++;
	right_side_hint = *src++;
	bottom_side_hint = *src++;
	left_side_hint = *src++;
}

void Walkbox::save() const {
	assert(_base != NULL);
	_base[0] = type;
	_base[1] = orientation;
	rect.save();
}


} // End of namespace TeenAgent
