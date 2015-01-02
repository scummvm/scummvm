/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef STARK_GFX_RENDER_ENTRY_H
#define STARK_GFX_RENDER_ENTRY_H

#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"

namespace Stark {

class GfxDriver;
class Resource;
class Visual;

class RenderEntry {
public:
	RenderEntry(Resource *owner, const Common::String &name);
	virtual ~RenderEntry() {};

	void update(uint32 delta); // TODO: Remove
	void render(GfxDriver *gfx);

	void setVisual(Visual *visual);
	void setPosition(const Common::Point &position);

protected:
	Common::String _name;
	Resource *_owner;

	Visual *_visual;
	Common::Point _position;
};

typedef Common::Array<RenderEntry *> RenderEntryArray;

} // End of namespace Stark

#endif // STARK_GFX_RENDER_ENTRY_H
