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

#include "engines/stark/gfx/renderentry.h"
#include "engines/stark/gfx/driver.h"

#include "engines/stark/visual/actor.h"
#include "engines/stark/visual/image.h"
#include "engines/stark/visual/smacker.h"
#include "engines/stark/visual/visual.h"

// TODO: Refactor this logic elsewhere
#include "engines/stark/resources/item.h"
#include "engines/stark/resources/anim.h"
#include "engines/stark/resources/pattable.h"

namespace Stark {
namespace Gfx {

RenderEntry::RenderEntry(Resources::Object *owner, const Common::String &name) :
	_visual(nullptr),
	_name(name),
	_owner(owner),
	_direction3D(0.0),
	_sortKey(0.0) {
}

void RenderEntry::render(Driver *gfx) {
	if (!_visual) {
		// warning("No visual for render entry '%s'", _name.c_str());
		return;
	}

	VisualImageXMG *imageXMG = _visual->get<VisualImageXMG>();
	if (imageXMG) {
		imageXMG->render(_position);
	}

	VisualActor *actor = _visual->get<VisualActor>();
	if (actor) {
		actor->render(gfx, _position3D, _direction3D);
	}

	VisualSmacker *smacker = _visual->get<VisualSmacker>();
	if (smacker) {
		smacker->render(_position);
	}
}

void RenderEntry::setVisual(Visual *visual) {
	_visual = visual;
}

void RenderEntry::setPosition(const Common::Point &position) {
	_position = position;
}

void RenderEntry::setPosition3D(const Math::Vector3d &position, float direction) {
	_position3D = position;
	_direction3D = direction;
}

void RenderEntry::setSortKey(float sortKey) {
	_sortKey = sortKey;
}

bool RenderEntry::compare(const RenderEntry *x, const RenderEntry *y) {
	return x->_sortKey < y->_sortKey;
}

bool RenderEntry::containsPoint(Common::Point point) {
	return indexForPoint(point) != -1;
}

int RenderEntry::indexForPoint(Common::Point point) {
	// TODO: This doesn't consider 3D at all.
	// TODO: This is just a quick fix, we still need to calculate the position, after any scaling and 3D transforms.
	// TODO: We more or less ignore Y for now, since all we consider is the position-point.

	// HACK: Since we lack Subtype2
	if (getOwner() && (getOwner()->getType() == Resources::Type::kItem || getOwner()->getType() == Resources::Type::kAnim)) {
		point.x -= _position.x;
		point.y -= _position.y;
		point.y -= Gfx::Driver::kTopBorderHeight; // Adjust for the top part.
		if (getOwner()->getType() == Resources::Type::kItem) {
			Resources::Item *item = (Resources::Item*)getOwner();
			int index = item->indexForPoint(point);
			// HACK For subtype 2, we get a PAT-index that is NOT inside the item.
			if (getOwner()->getSubType() == Resources::Item::kItemSub2) {
				return index;
			}
			if (index == -1) {
				return -1;
			} else {
				Resources::PATTable *table = item->findChildWithIndex<Resources::PATTable>(index);
				// Ignore any uninteractable Items
				// this should not be done when handling UI-buttons, as they have 0 actions.
				// For now that special case is handled in the Owner == Anim type below,
				// but in practice it should be done by way of checking for ItemSub2
				if (!table) {
					warning("Item %s has no PAT Table", item->getName().c_str());
				}
				if (!table || table->getNumActions() == 0) {
					return -1;
				}
			}
			return index;
		// TODO: This is probably not necessary after introducing SubType2, but we keep it for the statics for now.
		} else if (getOwner()->getType() == Resources::Type::kAnim) { // HACK Until we get Subtype2
			Resources::Anim *anim = (Resources::Anim*)getOwner();
			return anim->indexForPoint(point);
		}
	}
	return -1;
}

} // End of namespace Gfx
} // End of namespace Stark
