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

#include "math/ray.h"
#include "math/vector3d.h"

namespace Stark {

class Visual;
class VisualImageXMG;
class VisualText;

namespace Resources {
class ItemVisual;
}

namespace Gfx {

struct LightEntry {
	enum Type {
		kAmbient     = 0,
		kPoint       = 1,
		kDirectional = 2,
		kSpot        = 4
	};

	Type type;
	Math::Vector3d color;
	Math::Vector3d position;
	Math::Vector3d direction;
	Math::Angle innerConeAngle;
	Math::Angle outerConeAngle;
	float falloffNear;
	float falloffFar;
};

typedef Common::Array<LightEntry *> LightEntryArray;

class RenderEntry {
public:
	RenderEntry(Resources::ItemVisual *owner, const Common::String &name);
	virtual ~RenderEntry() {}

	void render(const LightEntryArray &lights = LightEntryArray());

	void setVisual(Visual *visual);
	void setPosition(const Common::Point &position);
	void setPosition3D(const Math::Vector3d &position, float direction);
	void setSortKey(float sortKey);
	void setClickable(bool clickable);

	/** Gets the position */
	Common::Point getPosition() const { return _position; }

	/** Gets the owner-object */
	Resources::ItemVisual *getOwner() const { return _owner; }

	/** Gets the entry's name */
	const Common::String &getName() const { return _name; }

	/** Obtain the underlying image visual, if any */
	VisualImageXMG *getImage() const;

	/** Obtain the underlying text visual, if any */
	VisualText *getText() const;

	/**
	 * Mouse picking test for 2D items
	 *
	 * @param position game window coordinates to test
	 * @param relativePosition successful hit item relative coordinates
	 * @param cursorRect cursor rectangle to be used to test small world items
	 * @return successful hit
	 */
	bool containsPoint(const Common::Point &position, Common::Point &relativePosition, const Common::Rect &cursorRect) const;

	/** Mouse picking test for 3D items */
	bool intersectRay(const Math::Ray &ray) const;

	/** Compare two render entries by their sort keys */
	static bool compare(const RenderEntry *x, const RenderEntry *y);

	/**
	 * Compute the 2D screen space bounding rect for the item,
	 * in original game view coordinates.
	 */
	Common::Rect getBoundingRect() const;

protected:
	Common::String _name;
	Resources::ItemVisual *_owner;

	Visual *_visual;
	Common::Point _position;
	Math::Vector3d _position3D;
	float _direction3D;
	float _sortKey;
	bool _clickable;
};

typedef Common::Array<RenderEntry *> RenderEntryArray;

} // End of namespace Gfx
} // End of namespace Stark

#endif // STARK_GFX_RENDER_ENTRY_H
