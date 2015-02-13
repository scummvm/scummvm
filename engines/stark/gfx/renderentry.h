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

#include "math/vector3d.h"

namespace Stark {

namespace Resources {
class Object;
}

class GfxDriver;
class Visual;

class RenderEntry {
public:
	RenderEntry(Resources::Object *owner, const Common::String &name);
	virtual ~RenderEntry() {};

	void render(GfxDriver *gfx);

	void setVisual(Visual *visual);
	void setPosition(const Common::Point &position);
	void setPosition3D(const Math::Vector3d &position, float direction);
	void setSortKey(float sortKey);

	/** Compare two render entries by their sort keys */
	static bool compare(const RenderEntry *x, const RenderEntry *y);

protected:
	Common::String _name;
	Resources::Object *_owner;

	Visual *_visual;
	Common::Point _position;
	Math::Vector3d _position3D;
	float _direction3D;
	float _sortKey;
};

typedef Common::Array<RenderEntry *> RenderEntryArray;

} // End of namespace Stark

#endif // STARK_GFX_RENDER_ENTRY_H
