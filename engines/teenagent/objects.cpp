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
 * $URL: https://www.switchlink.se/svn/teen/objects.h $
 * $Id: objects.h 172 2009-08-11 08:06:58Z megath $
 */

#include "objects.h"
#include "common/debug.h"

using namespace TeenAgent;

void Rect::render(Graphics::Surface *surface, uint8 color) const {
	surface->hLine(left, bottom, right, color);
	surface->vLine(left, bottom, top, color);
	surface->hLine(left, top, right, color);
	surface->vLine(right, bottom, top, color);
}

void Walkbox::dump() {
	debug(0, "walkbox %02x %02x [%d, %d, %d, %d] %02x %02x %02x %02x  ", unk00, unk01, 
		rect.left, rect.right, rect.top, rect.bottom, 
		unk0a, unk0b, unk0c, unk0d);
}

void Object::dump() {
	debug(0, "object: %u %u [%u,%u,%u,%u], actor: [%u,%u,%u,%u], orientation: %u, name: %s", id, enabled, 
		rect.left, rect.top, rect.right, rect.bottom, 
		actor_rect.left, actor_rect.top, actor_rect.right, actor_rect.bottom, 
		actor_orientation, name
	); 
}

Common::String Object::description(const char *name) {
	const char * desc = name + strlen(name) + 1;
	if (*desc == 0)
		return Common::String();
		
	Common::String result;

	while(*desc != 1 && *desc != 0) {
		Common::String line;
		while(*desc != 1 && *desc != 0) {
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


Common::String Object::description() const {
	return description(name);
}

Common::String InventoryObject::description() const {
	return Object::description(name);
}
